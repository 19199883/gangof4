#include "femas_trade_interface.h"
#include <iomanip>

#include "my_protocol_packager.h"
#include "check_schedule.h"
#include "qtm_with_code.h"
#include "femas_data_formater.h"

using namespace std;
using namespace commonfuncs;

void MyFemasTradeSpi::ReportErrorState(int api_error_no, const std::string &error_msg)
{
    if (api_error_no == 0)
    {
        return;
    }
    if (!cfg_.IsKnownErrorNo(api_error_no))
    {
        char err_msg[127];
        sprintf(err_msg, "api error no: %d, error msg: %s", api_error_no, error_msg.c_str());
        update_state(tunnel_info_.qtm_name.c_str(), TYPE_TCA, QtmState::UNDEFINED_API_ERROR, err_msg);
        TNL_LOG_INFO("update_state: name: %s, State: %d, Description: %s.", tunnel_info_.qtm_name.c_str(), QtmState::UNDEFINED_API_ERROR, err_msg);
    }
}

MyFemasTradeSpi::MyFemasTradeSpi(const TunnelConfigData &cfg)
    : cfg_(cfg)
{
    connected_ = false;
    logoned_ = false;
    in_init_state_ = true;

    // whether it is need to support cancel all order when init
    have_handled_unterminated_orders_ = true;
    if (cfg_.Initial_policy_param().cancel_orders_at_start)
    {
        have_handled_unterminated_orders_ = false;
    }
    finish_qryorder_result_stats_ = false;
    if (cfg_.Compliance_check_param().init_cancel_times_at_start == 0)
    {
        finish_qryorder_result_stats_ = true;
    }

    femas_trade_context_.InitOrderRef(cfg_.App_cfg().orderref_prefix_id);

    // 从配置解析参数
    if (!ParseConfig())
    {
        return;
    }

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "femas_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

    // create
    api_ = CUstpFtdcTraderApi::CreateFtdcTraderApi();
    api_->RegisterSpi(this);

    // subscribe
    api_->SubscribePrivateTopic(USTP_TERT_QUICK);
    api_->SubscribePublicTopic(USTP_TERT_QUICK);

    // set front address list
    for (const std::string &v : cfg_.Logon_config().front_addrs)
    {
        char *addr_tmp = new char[v.size() + 1];
        memcpy(addr_tmp, v.c_str(), v.size() + 1);
        api_->RegisterFront(addr_tmp);
        TNL_LOG_WARN("RegisterFront, addr: %s", addr_tmp);
        delete[] addr_tmp;
    }

    // init
    api_->Init();
}

MyFemasTradeSpi::~MyFemasTradeSpi(void)
{
}

bool MyFemasTradeSpi::ParseConfig()
{
    // 用户密码
    tunnel_info_.account = cfg_.Logon_config().investorid;
    user_ = cfg_.Logon_config().clientid;
    pswd_ = cfg_.Logon_config().password;
    return true;
}

void MyFemasTradeSpi::OnFrontConnected()
{
    TNL_LOG_WARN("OnFrontConnected.");
    connected_ = true;
    //ProcessEngine::ConnectedToCounter();

    // 登录

    CUstpFtdcReqUserLoginField login_data;
    memset(&login_data, 0, sizeof(CUstpFtdcReqUserLoginField));
    strncpy(login_data.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TUstpFtdcBrokerIDType));
    strncpy(login_data.UserID, user_.c_str(), sizeof(TUstpFtdcUserIDType));
    strncpy(login_data.Password, pswd_.c_str(), sizeof(TUstpFtdcPasswordType));

    // 成功登录后，断开不再重新登录
    if (in_init_state_)
    {
        api_->ReqUserLogin(&login_data, 0);
    }
}

void MyFemasTradeSpi::OnFrontDisconnected(int nReason)
{
    connected_ = false;
    logoned_ = false;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
    TNL_LOG_WARN("OnFrontDisconnected, nReason: %d", nReason);
}

void MyFemasTradeSpi::OnHeartBeatWarning(int nTimeLapse)
{
    TNL_LOG_WARN("OnHeartBeatWarning, nTimeLapse: %d", nTimeLapse);
}

void MyFemasTradeSpi::OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    TNL_LOG_INFO("OnRspUserLogin: requestid = %d, last_flag=%d \n%s \n%s",
        nRequestID, bIsLast,
        FEMASDatatypeFormater::ToString(pRspUserLogin).c_str(),
        FEMASDatatypeFormater::ToString(pRspInfo).c_str());

    // logon success
    if (pRspInfo == NULL || pRspInfo->ErrorID == 0)
    {
        // set session parameters

        max_order_ref_ = atoll(pRspUserLogin->MaxOrderLocalID);

        // 设置到交易环境变量中
        femas_trade_context_.SetOrderRef(max_order_ref_);

        logoned_ = true;
        in_init_state_ = false;

        // start thread for cancel unterminated orders
        if (!HaveFinishQueryOrders())
        {
            std::thread qry_order(&MyFemasTradeSpi::QueryAndHandleOrders, this);
            qry_order.detach();
        }
        else
        {
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_SUCCESS);
        }
    }
    else
    {
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        int standard_error_no = cfg_.GetStandardErrorNo(pRspInfo->ErrorID);

        TNL_LOG_WARN("OnRspUserLogin, external errorid: %d; Internal errorid: %d",
            pRspInfo->ErrorID, standard_error_no);

        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_FAIL);
    }
}

void MyFemasTradeSpi::OnRspUserLogout(CUstpFtdcRspUserLogoutField *pRspUserLogout, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    try
    {
        logoned_ = false;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_OUT);
        TNL_LOG_WARN("OnRspUserLogout: requestid = %d, last_flag=%d \n%s \n%s",
            nRequestID, bIsLast,
            FEMASDatatypeFormater::ToString(pRspUserLogout).c_str(),
            FEMASDatatypeFormater::ToString(pRspInfo).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspUserLogout.");
    }
}

void MyFemasTradeSpi::OnRspError(CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    try
    {
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            TNL_LOG_WARN("OnRspError: requestid = %d, last_flag=%d \n%s",
                nRequestID, bIsLast, FEMASDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnRspError ");
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspError.");
    }
}

void Init(CUstpFtdcTradeField* fake_trade_rsp, const CUstpFtdcOrderField * pOrder)
{
    // 仅将要用的字段赋值
    memcpy(fake_trade_rsp->OrderSysID, pOrder->OrderSysID, sizeof(TUstpFtdcOrderSysIDType));
    memcpy(fake_trade_rsp->InstrumentID, pOrder->InstrumentID, sizeof(TUstpFtdcInstrumentIDType));
    fake_trade_rsp->Direction = pOrder->Direction;
    fake_trade_rsp->OffsetFlag = pOrder->OffsetFlag;
    fake_trade_rsp->TradeVolume = pOrder->VolumeTraded;
    fake_trade_rsp->TradePrice = pOrder->LimitPrice;
    memcpy(fake_trade_rsp->TradeID, pOrder->OrderSysID, sizeof(TUstpFtdcTradeIDType));
    fake_trade_rsp->HedgeFlag = pOrder->HedgeFlag;
}

void MyFemasTradeSpi::OnRtnOrder(CUstpFtdcOrderField *pOrder)
{
    try
    {
        if (!HaveFinishQueryOrders())
        {
            TNL_LOG_WARN("OnRtnOrder when tunnel not ready.");
            return;
        }
        if (!pOrder)
        {
            TNL_LOG_WARN("OnRtnOrder, pOrder is null.");
            return;
        }

        const OriginalReqInfo *p = NULL;
        OrderRefDataType order_ref = atoll(pOrder->UserOrderLocalID);

        if (strcmp(pOrder->UserID, pOrder->UserCustom) == 0)
        {
            p = femas_trade_context_.GetOrderInfoByOrderRef(order_ref);
        }

        if (p)
        {
            femas_trade_context_.AddNewOrderSysID(atoll(pOrder->OrderSysID));
            // should adapt response for cffex simulator test; femas only support cffex
            if (p->order_type == MY_TNL_HF_FAK)
            {
                // FAK指令
                OnRtnOrderFak(pOrder, p, order_ref);
            }
            else
            {
                OnRtnOrderNormal(pOrder, p, order_ref);
            }
            TNL_LOG_DEBUG("%s", FEMASDatatypeFormater::ToString(pOrder).c_str());
        }
        else
        {
            TNL_LOG_INFO("%s - place by %s, not mine(%s) in OnRtnOrder", pOrder->UserOrderLocalID, pOrder->UserCustom,
                pOrder->UserID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnOrder.");
    }
}

void MyFemasTradeSpi::OnRtnTrade(CUstpFtdcTradeField *pTrade)
{
    try
    {
        if (!HaveFinishQueryOrders())
        {
            TNL_LOG_WARN("OnRtnTrade when tunnel not ready.");
            return;
        }
        if (!pTrade)
        {
            TNL_LOG_WARN("OnRtnTrade, pTrade is null.");
            return;
        }

        EntrustNoType sysid = atoll(pTrade->OrderSysID);
        OrderRefDataType order_ref = atoll(pTrade->UserOrderLocalID);
        const OriginalReqInfo *p = NULL;
        if (femas_trade_context_.IsSysIDOfThisTunnel(sysid))
        {
            p = femas_trade_context_.GetOrderInfoByOrderRef(order_ref);
        }
        if (p)
        {
            if (!femas_trade_context_.IsFinishedOrderOfFAK(order_ref))
            {
                T_TradeReturn trade_return;
                FEMASPacker::TradeReturn(p, pTrade, trade_return);

                // 成交
                StatusCheckResult check_result = femas_trade_context_.CheckAndUpdateStatus(
                    order_ref,
                    FemasRespondType::trade_return,
                    MY_TNL_OS_PARTIALCOM,
                    0);

                if (check_result == StatusCheckResult::fillup_rsp_rtn_c
                    || check_result == StatusCheckResult::fillup_rsp_rtn_p)
                {
                    HandleFillupRsp(trade_return.entrust_no, p->serial_no);
                }

                if (check_result == StatusCheckResult::fillup_rsp_rtn_c
                    || check_result == StatusCheckResult::fillup_rtn_c)
                {
                    HandleFillupRtn(trade_return.entrust_no, MY_TNL_OS_COMPLETED, p);
                }
                else if (check_result == StatusCheckResult::fillup_rsp_rtn_p
                    || check_result == StatusCheckResult::fillup_rtn_p)
                {
                    HandleFillupRtn(trade_return.entrust_no, MY_TNL_OS_PARTIALCOM, p);
                }

                TradeReturn_call_back_handler_(&trade_return);
                LogUtil::OnTradeReturn(&trade_return, tunnel_info_);
            }
            TNL_LOG_DEBUG("%s", FEMASDatatypeFormater::ToString(pTrade).c_str());
        }
        else
        {
            TNL_LOG_INFO("%s - can't get original order info in OnRtnTrade(sysid:%s)",
                pTrade->UserOrderLocalID, pTrade->OrderSysID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnTrade.");
    }
}

// 报单拒绝
void MyFemasTradeSpi::OnRspOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    int errorid = 0;
    int standard_error_no = 0;
    try
    {
        if (!pInputOrder)
        {
            TNL_LOG_WARN("OnRspOrderInsert, pInputOrder is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            errorid = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(errorid);
        }

        TNL_LOG_WARN("OnRspOrderInsert, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            errorid, standard_error_no, pInputOrder->UserOrderLocalID);

        OrderRefDataType OrderRef = atoll(pInputOrder->UserOrderLocalID);
        const OriginalReqInfo *p = femas_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            T_OrderRespond pOrderRespond;

            if (0 != errorid)
            {
                if (femas_trade_context_.CheckAndSetHaveHandleInsertErr(p->serial_no))
                {
                    // 报单失败，报告合规检查
                    ComplianceCheck_OnOrderInsertFailed(
                        tunnel_info_.account.c_str(),
                        atoll(pInputOrder->UserOrderLocalID),
                        p->exchange_code,
                        pInputOrder->InstrumentID,
                        pInputOrder->Volume,
                        p->hedge_type,
                        p->open_close_flag,
                        p->order_type);

                    FEMASPacker::OrderRespond(standard_error_no, p->serial_no, "0", 'e', pOrderRespond);
                    StatusCheckResult check_result = femas_trade_context_.CheckAndUpdateStatus(
                        atoll(pInputOrder->UserOrderLocalID),
                        FemasRespondType::order_respond,
                        pOrderRespond.entrust_status,
                        0);

                    if (check_result == StatusCheckResult::none)
                    {
                        // 应答
                        OrderRespond_call_back_handler_(&pOrderRespond);
                        LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
                    }
                    else
                    {
                        TNL_LOG_WARN("abandon a respond: %ld", p->serial_no);
                    }
                }
                else
                {
                    TNL_LOG_WARN("rsp err insert have handled, order ref: %s", pInputOrder->UserOrderLocalID);
                }
            }
            else
            {
                femas_trade_context_.AddNewOrderSysID(atoll(pInputOrder->OrderSysID));
                FEMASPacker::OrderRespond(standard_error_no, p->serial_no, pInputOrder->OrderSysID, '3', pOrderRespond);

                StatusCheckResult check_result = femas_trade_context_.CheckAndUpdateStatus(
                    atoll(pInputOrder->UserOrderLocalID),
                    FemasRespondType::order_respond,
                    pOrderRespond.entrust_status,
                    0);

                if (check_result == StatusCheckResult::none)
                {
                    // 应答
                    OrderRespond_call_back_handler_(&pOrderRespond);
                    LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
                }
                else
                {
                    TNL_LOG_WARN("abandon a respond: %ld", p->serial_no);
                }
            }

            TNL_LOG_DEBUG("OnRspOrderInsert: requestid = %d, last_flag=%d \n%s \n%s",
                nRequestID, bIsLast,
                FEMASDatatypeFormater::ToString(pInputOrder).c_str(),
                FEMASDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("%s - can't get original order info in OnRspOrderInsert", pInputOrder->UserOrderLocalID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspOrderInsert.");
    }
}

// 飞马平台撤单响应
void MyFemasTradeSpi::OnRspOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    int errorid = 0;
    int standard_error_no = 0;
    try
    {
        if (!HaveFinishQueryOrders())
        {
            TNL_LOG_WARN("OnRspOrderAction when cancel unterminated orders.");
            return;
        }
        if (!pOrderAction)
        {
            TNL_LOG_WARN("OnRspOrderAction, pInputOrderAction is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            errorid = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(errorid);
        }

        TNL_LOG_WARN("OnRspOrderAction, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            errorid, standard_error_no, pOrderAction->UserOrderActionLocalID);

        OrderRefDataType OrderRef = atoll(pOrderAction->UserOrderActionLocalID);
        const OriginalReqInfo *p = femas_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            T_CancelRespond cancel_respond;

            if (0 != errorid)
            {
                if (femas_trade_context_.CheckAndSetHaveHandleActionErr(p->serial_no))
                {
                    FEMASPacker::CancelRespond(standard_error_no, p->serial_no, "0", cancel_respond);
                    // 撤单拒绝，报告合规检查
                    ComplianceCheck_OnOrderCancelFailed(
                        tunnel_info_.account.c_str(),
                        p->stock_code.c_str(),
                        atoll(pOrderAction->UserOrderLocalID));

                    // 应答
                    CancelRespond_call_back_handler_(&cancel_respond);
                    LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
                }
                else
                {
                    TNL_LOG_WARN("rsp err action have handled, order sn: %ld", p->serial_no);
                }
            }
            else
            {
                FEMASPacker::CancelRespond(standard_error_no, p->serial_no, "0", cancel_respond);

                // 应答
                CancelRespond_call_back_handler_(&cancel_respond);
                LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
            }

            TNL_LOG_DEBUG("OnRspOrderAction: requestid = %d, last_flag=%d \n%s \n%s",
                nRequestID, bIsLast,
                FEMASDatatypeFormater::ToString(pOrderAction).c_str(),
                FEMASDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("%s - can't get original action info in OnRspOrderAction", pOrderAction->UserOrderActionLocalID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspOrderAction.");
    }
}

void MyFemasTradeSpi::OnErrRtnOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo)
{
    int errorid = 0;
    int standard_error_no = 0;
    try
    {
        if (!pInputOrder)
        {
            TNL_LOG_WARN("OnErrRtnOrderInsert, pInputOrder is null.");
            return;
        }
        if (pRspInfo)
        {
            errorid = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(errorid);
        }

        TNL_LOG_WARN("OnErrRtnOrderInsert, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            errorid, standard_error_no, pInputOrder->UserOrderLocalID);

        OrderRefDataType OrderRef = atoll(pInputOrder->UserOrderLocalID);
        const OriginalReqInfo *p = femas_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            T_OrderRespond pOrderRespond;

            if (femas_trade_context_.CheckAndSetHaveHandleInsertErr(p->serial_no))
            {
                // 报单失败，报告合规检查
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    atoll(pInputOrder->UserOrderLocalID),
                    p->exchange_code,
                    pInputOrder->InstrumentID,
                    pInputOrder->Volume,
                    p->hedge_type,
                    p->open_close_flag,
                    p->order_type);

                FEMASPacker::OrderRespond(standard_error_no, p->serial_no, "0", 'e', pOrderRespond);

                StatusCheckResult check_result = femas_trade_context_.CheckAndUpdateStatus(
                    atoll(pInputOrder->UserOrderLocalID),
                    FemasRespondType::order_respond,
                    pOrderRespond.entrust_status,
                    0);
                if (check_result == StatusCheckResult::none)
                {
                    // 应答
                    OrderRespond_call_back_handler_(&pOrderRespond);
                    LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
                }
                else
                {
                    TNL_LOG_WARN("abandon a err insert respond: %ld", p->serial_no);
                }
            }
            else
            {
                TNL_LOG_WARN("rsp err insert have handled, order ref: %s", pInputOrder->UserOrderLocalID);
            }

            TNL_LOG_DEBUG("OnErrRtnOrderInsert:  \n%s \n%s",
                FEMASDatatypeFormater::ToString(pInputOrder).c_str(),
                FEMASDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("%s - place by %s, not mine(%s) in OnErrRtnOrderInsert", pInputOrder->UserOrderLocalID,
                pInputOrder->UserCustom, pInputOrder->UserID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnErrRtnOrderInsert.");
    }
}

void MyFemasTradeSpi::OnErrRtnOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo)
{
    int errorid = 0;
    int standard_error_no = 0;
    try
    {
        if (!pOrderAction)
        {
            TNL_LOG_WARN("OnErrRtnOrderAction, pOrderAction is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            errorid = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(errorid);
        }

        TNL_LOG_WARN("OnErrRtnOrderAction, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            errorid, standard_error_no, pOrderAction->UserOrderActionLocalID);

        OrderRefDataType OrderRef = atoll(pOrderAction->UserOrderActionLocalID);
        const OriginalReqInfo *p = femas_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            T_CancelRespond cancel_respond;

            if (femas_trade_context_.CheckAndSetHaveHandleActionErr(p->serial_no))
            {
                FEMASPacker::CancelRespond(standard_error_no, p->serial_no, "0", cancel_respond);

                // 撤单拒绝，报告合规检查
                ComplianceCheck_OnOrderCancelFailed(
                    tunnel_info_.account.c_str(),
                    p->stock_code.c_str(),
                    atoll(pOrderAction->UserOrderLocalID));

                // 应答
                CancelRespond_call_back_handler_(&cancel_respond);
                LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("rsp err action have handled, order sn: %ld", p->serial_no);
            }

            TNL_LOG_DEBUG("OnErrRtnOrderAction:  \n%s \n%s",
                FEMASDatatypeFormater::ToString(pOrderAction).c_str(),
                FEMASDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("%s - can't get original action info in OnErrRtnOrderAction", pOrderAction->UserOrderActionLocalID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnErrRtnOrderAction.");
    }
}

void MyFemasTradeSpi::OnRtnOrderFak(CUstpFtdcOrderField * pOrder, const OriginalReqInfo * p, OrderRefDataType order_ref)
{
    SerialNoType serial_no = p->serial_no;

    if (pOrder->OrderStatus == USTP_FTDC_OS_Canceled
        && pOrder->VolumeTraded > 0)
    {		// 如果是部成，回报委托/成交/已撤

        // 部成委托回报
        CUstpFtdcOrderField fake_partial_rsp;
        memcpy(&fake_partial_rsp, pOrder, sizeof(CUstpFtdcOrderField));
        fake_partial_rsp.OrderStatus = USTP_FTDC_OS_PartTradedQueueing;
        T_OrderReturn order_return;
        FEMASPacker::OrderReturn(serial_no, &fake_partial_rsp, order_return);

        StatusCheckResult check_result = femas_trade_context_.CheckAndUpdateStatus(
            order_ref,
            FemasRespondType::order_return,
            order_return.entrust_status,
            0);
        if (check_result == StatusCheckResult::fillup_rsp)
        {
            HandleFillupRsp(order_return.entrust_no, serial_no);
        }
        if (check_result != StatusCheckResult::abandon)
        {
            OrderReturn_call_back_handler_(&order_return);
            LogUtil::OnOrderReturn(&order_return, tunnel_info_);
        }
        else
        {
            TNL_LOG_WARN("abandon a return: %ld", p->serial_no);
        }

        // 成交回报
        CUstpFtdcTradeField fake_trade_rsp;
        Init(&fake_trade_rsp, pOrder);
        T_TradeReturn trade_return;
        FEMASPacker::TradeReturn(p, &fake_trade_rsp, trade_return);

        // don't need check and update status
        TradeReturn_call_back_handler_(&trade_return);
        LogUtil::OnTradeReturn(&trade_return, tunnel_info_);

        // 已撤回报
        T_OrderReturn order_cancle_return;
        FEMASPacker::OrderReturn(serial_no, pOrder, order_cancle_return);
        OrderReturn_call_back_handler_(&order_cancle_return);
        LogUtil::OnOrderReturn(&order_cancle_return, tunnel_info_);

        femas_trade_context_.SetFinishedOrderOfFAK(order_ref);

        // 已撤，报告合规检查
        ComplianceCheck_OnOrderCanceled(
            tunnel_info_.account.c_str(),
            p->stock_code.c_str(),
            order_ref,
            p->exchange_code,
            pOrder->VolumeRemain,
            p->hedge_type,
            p->open_close_flag);

    }
    else
    {
        // 全成，无需报告合规检查（FAK报单不进入委托列表）

        // 已撤，报告合规检查
        if (pOrder->OrderStatus == USTP_FTDC_OS_Canceled)
        {
            ComplianceCheck_OnOrderCanceled(
                tunnel_info_.account.c_str(),
                p->stock_code.c_str(),
                order_ref,
                p->exchange_code,
                pOrder->VolumeRemain,
                p->hedge_type,
                p->open_close_flag);
        }

        T_OrderReturn order_return;
        FEMASPacker::OrderReturn(serial_no, pOrder, order_return);

        StatusCheckResult check_result = femas_trade_context_.CheckAndUpdateStatus(
            order_ref,
            FemasRespondType::order_return,
            order_return.entrust_status,
            0);
        if (check_result == StatusCheckResult::fillup_rsp)
        {
            HandleFillupRsp(order_return.entrust_no, serial_no);
        }
        if (check_result != StatusCheckResult::abandon)
        {
            OrderReturn_call_back_handler_(&order_return);
            LogUtil::OnOrderReturn(&order_return, tunnel_info_);
        }
        else
        {
            TNL_LOG_WARN("abandon a return: %ld", p->serial_no);
        }
    }
}

void MyFemasTradeSpi::OnRtnOrderNormal(CUstpFtdcOrderField * pOrder, const OriginalReqInfo * p, OrderRefDataType order_ref)
{
    //PackagerHelper rsp_package(pOrder, p);
    SerialNoType serial_no = p->serial_no;

    // 全成，报告合规检查
    if (pOrder->OrderStatus == USTP_FTDC_OS_AllTraded)
    {
        ComplianceCheck_OnOrderFilled(
            tunnel_info_.account.c_str(),
            order_ref);
    }
    else if (pOrder->OrderStatus == USTP_FTDC_OS_Canceled)
    {
        ComplianceCheck_OnOrderCanceled(
            tunnel_info_.account.c_str(),
            p->stock_code.c_str(),
            order_ref,
            p->exchange_code,
            pOrder->VolumeRemain,
            p->hedge_type,
            p->open_close_flag);
    }

    T_OrderReturn order_return;
    FEMASPacker::OrderReturn(serial_no, pOrder, order_return);

    StatusCheckResult check_result = femas_trade_context_.CheckAndUpdateStatus(
        order_ref,
        FemasRespondType::order_return,
        order_return.entrust_status,
        0);
    if (check_result == StatusCheckResult::fillup_rsp)
    {
        HandleFillupRsp(order_return.entrust_no, serial_no);
    }
    if (check_result != StatusCheckResult::abandon)
    {
        OrderReturn_call_back_handler_(&order_return);
        LogUtil::OnOrderReturn(&order_return, tunnel_info_);
    }
    else
    {
        TNL_LOG_WARN("abandon a return: %ld", p->serial_no);
    }
}

void MyFemasTradeSpi::SetCallbackHandler(std::function<void(const T_OrderRespond *)> callback_handler)
{
    OrderRespond_call_back_handler_ = callback_handler;
}

void MyFemasTradeSpi::SetCallbackHandler(std::function<void(const T_CancelRespond *)> callback_handler)
{
    CancelRespond_call_back_handler_ = callback_handler;
}

void MyFemasTradeSpi::SetCallbackHandler(std::function<void(const T_OrderReturn *)> callback_handler)
{
    OrderReturn_call_back_handler_ = callback_handler;
}

void MyFemasTradeSpi::SetCallbackHandler(std::function<void(const T_TradeReturn *)> callback_handler)
{
    TradeReturn_call_back_handler_ = callback_handler;
}

void MyFemasTradeSpi::HandleFillupRsp(long entrust_no, SerialNoType serial_no)
{
    T_OrderRespond order_respond;
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = entrust_no;
    order_respond.entrust_status = MY_TNL_OS_REPORDED;
    order_respond.serial_no = serial_no;
    order_respond.error_no = 0;
    OrderRespond_call_back_handler_(&order_respond);
    LogUtil::OnOrderRespond(&order_respond, tunnel_info_);
    TNL_LOG_WARN("fill up respond for trade_return: %ld", serial_no);
}

void MyFemasTradeSpi::HandleFillupRtn(long entrust_no, char order_status, const OriginalReqInfo* p)
{
    T_OrderReturn order_return;
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = entrust_no;
    order_return.entrust_status = order_status;
    order_return.serial_no = p->serial_no;

    strncpy(order_return.stock_code, p->stock_code.c_str(), sizeof(TUstpFtdcInstrumentIDType));
    order_return.direction = p->buy_sell_flag;
    order_return.open_close = p->open_close_flag;
    order_return.speculator = p->hedge_type;
    order_return.volume = p->order_volum;
    order_return.limit_price = 0;
    OrderReturn_call_back_handler_(&order_return);
    LogUtil::OnOrderReturn(&order_return, tunnel_info_);
    TNL_LOG_WARN("fill up return for trade_return: %ld", p->serial_no);
}

///报单查询应答
void MyFemasTradeSpi::OnRspQryOrder(CUstpFtdcOrderField *pOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!HaveFinishQueryOrders())
    {
        if (!finish_qryorder_result_stats_)
        {
            std::unique_lock<std::mutex> lock(qry_order_result_stats_sync_);
            if (pOrder && pOrder->OrderStatus == USTP_FTDC_OS_Canceled)
            {
                std::map<std::string, int>::iterator it = cancel_times_of_contract.find(pOrder->InstrumentID);
                if (it == cancel_times_of_contract.end())
                {
                    cancel_times_of_contract.insert(std::make_pair(pOrder->InstrumentID, 1));
                }
                else
                {
                    ++it->second;
                }
            }
            if (pOrder
                && pOrder->OffsetFlag == USTP_FTDC_OF_Open
                && pOrder->HedgeFlag == USTP_FTDC_CHF_Speculation
                && pOrder->VolumeTraded > 0)
            {
                std::string product(pOrder->InstrumentID, 2);
                std::map<std::string, int>::iterator it = open_volume_of_product.find(product);
                if (it == open_volume_of_product.end())
                {
                    open_volume_of_product.insert(std::make_pair(product, 1));
                }
                else
                {
                    it->second += pOrder->VolumeTraded;
                }
            }

            if (bIsLast)
            {
                for (std::map<std::string, int>::iterator it = cancel_times_of_contract.begin();
                    it != cancel_times_of_contract.end(); ++it)
                {
                    ComplianceCheck_SetCancelTimes(tunnel_info_.account.c_str(), it->first.c_str(), it->second);
                }
                cancel_times_of_contract.clear();

                for (std::map<std::string, int>::value_type v : open_volume_of_product)
                {
                    ComplianceCheck_SetOpenVolume(tunnel_info_.account.c_str(), MY_TNL_EC_CFFEX,
                        v.first.c_str(), MY_TNL_HF_SPECULATION, v.second);
                }
                open_volume_of_product.clear();
                finish_qryorder_result_stats_ = true;
            }
        }

        if (!have_handled_unterminated_orders_)
        {
            std::unique_lock<std::mutex> lock(cancel_sync_);
            if (pOrder && !IsOrderTerminate(*pOrder))
            {
                unterminated_orders_.push_back(*pOrder);
            }

            if (bIsLast && unterminated_orders_.empty())
            {
                have_handled_unterminated_orders_ = true;
            }

            if (bIsLast)
            {
                qry_order_finish_cond_.notify_one();
            }
        }

        if (TunnelIsReady())
        {
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
        }

        TNL_LOG_INFO("OnRspQryOrder when cancel unterminated orders or stats cancel times, order: 0X%X, last: %d", pOrder, bIsLast);
        return;
    }

    T_OrderDetailReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        ret.error_no = -1;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }
    TNL_LOG_DEBUG("%s", FEMASDatatypeFormater::ToString(pOrder).c_str());

    // Order detail
    if (pOrder)
    {
        OrderDetail od;
        strncpy(od.stock_code, pOrder->InstrumentID, sizeof(TUstpFtdcInstrumentIDType));
        od.entrust_no = atol(pOrder->OrderSysID);
        od.order_kind = FEMASFieldConvert::PriceTypeTrans(pOrder->OrderPriceType);
        od.direction = pOrder->Direction;
        od.open_close = pOrder->OffsetFlag;
        od.speculator = FEMASFieldConvert::EntrustTbFlagTrans(pOrder->HedgeFlag);
        od.entrust_status = FEMASFieldConvert::EntrustStatusTrans(pOrder->OrderStatus);
        od.limit_price = pOrder->LimitPrice;
        od.volume = pOrder->Volume;
        od.volume_traded = pOrder->VolumeTraded;
        od.volume_remain = pOrder->VolumeRemain;
        od_buffer_.push_back(od);
    }

    if (bIsLast)
    {
        ret.datas.swap(od_buffer_);
        ret.error_no = 0;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}

///成交单查询应答
void MyFemasTradeSpi::OnRspQryTrade(CUstpFtdcTradeField *pTrade, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    T_TradeDetailReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        ret.error_no = -1;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    TNL_LOG_DEBUG("%s", FEMASDatatypeFormater::ToString(pTrade).c_str());

    // trade detail
    if (pTrade)
    {
        TradeDetail td;
        strncpy(td.stock_code, pTrade->InstrumentID, sizeof(TUstpFtdcInstrumentIDType));
        td.entrust_no = atol(pTrade->OrderSysID);
        td.direction = pTrade->Direction;
        td.open_close = pTrade->OffsetFlag;
        td.speculator = FEMASFieldConvert::EntrustTbFlagTrans(pTrade->HedgeFlag);
        td.trade_price = pTrade->TradePrice;
        td.trade_volume = pTrade->TradeVolume;
        strncpy(td.trade_time, pTrade->TradeTime, sizeof(TUstpFtdcTimeType));
        td_buffer_.push_back(td);
    }

    if (bIsLast)
    {
        ret.datas.swap(td_buffer_);
        ret.error_no = 0;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

///投资者持仓查询应答
void MyFemasTradeSpi::OnRspQryInvestorPosition(CUstpFtdcRspInvestorPositionField *pRspInvestorPosition,
    CUstpFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    T_PositionReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        ret.error_no = -1;
        QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    TNL_LOG_DEBUG("%s", FEMASDatatypeFormater::ToString(pRspInvestorPosition).c_str());

    // empty positioon
    if (pRspInvestorPosition)
    {
        PositionDetail pos;
        strncpy(pos.stock_code, pRspInvestorPosition->InstrumentID, sizeof(TUstpFtdcInstrumentIDType));
        pos.direction = pRspInvestorPosition->Direction;
        pos.position = pRspInvestorPosition->Position;
        pos.position_avg_price = 0;
        if (pos.position > 0)
        {
            pos.position_avg_price = pRspInvestorPosition->PositionCost / pos.position / 300;
        }

        pos.yestoday_position = pRspInvestorPosition->YdPosition;
        pos.yd_position_avg_price = 0;
        if (pos.yestoday_position > 0)
        {
            pos.yd_position_avg_price = pRspInvestorPosition->YdPositionCost / pos.yestoday_position / 300;
        }

        if (pos.position > 0 || pos.yestoday_position > 0)
        {
            pos_buffer_.push_back(pos);
        }
    }

    if (bIsLast)
    {
        ret.datas.swap(pos_buffer_);
        ret.error_no = 0;
        QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}

CUstpFtdcOrderActionField
MyFemasTradeSpi::CreatCancelParam(const CUstpFtdcOrderField& order_field)
{
    CUstpFtdcOrderActionField cancle_order;
    OrderRefDataType order_ref = femas_trade_context_.GetNewOrderRef();
    memset(&cancle_order, 0, sizeof(cancle_order));

    memcpy(cancle_order.ExchangeID, order_field.ExchangeID, sizeof(TUstpFtdcExchangeIDType));
    memcpy(cancle_order.OrderSysID, order_field.OrderSysID, sizeof(TUstpFtdcOrderSysIDType));
    memcpy(cancle_order.BrokerID, order_field.BrokerID, sizeof(TUstpFtdcBrokerIDType));
    memcpy(cancle_order.InvestorID, order_field.InvestorID, sizeof(TUstpFtdcInvestorIDType));
    memcpy(cancle_order.UserID, order_field.UserID, sizeof(TUstpFtdcUserIDType));
    memcpy(cancle_order.UserOrderLocalID, order_field.UserOrderLocalID, sizeof(TUstpFtdcUserOrderLocalIDType));
    snprintf(cancle_order.UserOrderActionLocalID, sizeof(cancle_order.UserOrderActionLocalID), "%lld", order_ref);

    cancle_order.ActionFlag = USTP_FTDC_AF_Delete;
    cancle_order.LimitPrice = 0;
    cancle_order.VolumeChange = 0;

    return cancle_order;
}

bool MyFemasTradeSpi::IsOrderTerminate(const CUstpFtdcOrderField& order_field)
{
    return order_field.OrderStatus == USTP_FTDC_OS_AllTraded
        || order_field.OrderStatus == USTP_FTDC_OS_Canceled;
}

void MyFemasTradeSpi::QueryAndHandleOrders()
{
    // query order detail parameter
    CUstpFtdcQryOrderField qry_param;
    memset(&qry_param, 0, sizeof(CUstpFtdcQryOrderField));
    strncpy(qry_param.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TUstpFtdcBrokerIDType));
    // query orders of account
    // strncpy(qry_param.UserID, user_.c_str(), sizeof(TUstpFtdcUserIDType));
    strncpy(qry_param.ExchangeID, MY_TNL_EXID_SHFE, sizeof(TUstpFtdcExchangeIDType));
    strncpy(qry_param.InvestorID, tunnel_info_.account.c_str(), sizeof(TUstpFtdcInvestorIDType));
    //TUstpFtdcOrderSysIDType OrderSysID;
    //strncpy(qry_param.InstrumentID, pQryPosition->stock_code, sizeof(TUstpFtdcInstrumentIDType));

    //超时后没有完成查询，重试。为防止委托单太多，10s都回报不了，每次超时加5s
    int wait_seconds = 10;

    std::vector<CUstpFtdcOrderField> unterminated_orders_t;
    while (!HaveFinishQueryOrders())
    {
        {
            std::unique_lock<std::mutex> lock(qry_order_result_stats_sync_);
            cancel_times_of_contract.clear();
        }
        //主动查询所有报单
        while (true)
        {
            int qry_result = api_->ReqQryOrder(&qry_param, 0);
            if (qry_result != 0)
            {
                // retry if failed, wait some seconds
                sleep(2);
                continue;
            }
            TNL_LOG_INFO("ReqQryOrder for cancel unterminated orders or stats cancel times, return %d", qry_result);

            break;
        }

        //处理未终结报单（撤未终结报单）
        if (!have_handled_unterminated_orders_)
        {
            {
                std::unique_lock<std::mutex> lock(cancel_sync_);
                qry_order_finish_cond_.wait_for(lock, std::chrono::seconds(10));
                unterminated_orders_t.swap(unterminated_orders_);
            }
            //遍历 unterminated_orders 间隔20ms（流控要求），发送撤单请求
            for (const CUstpFtdcOrderField &order_field : unterminated_orders_t)
            {
                CUstpFtdcOrderActionField action_field = CreatCancelParam(order_field);
                int ret = api_->ReqOrderAction(&action_field, 0);
                if (ret != 0)
                {
                    TNL_LOG_WARN("in CancelUnterminatedOrders, ReqOrderAction return %d", ret);
                }
                usleep(20 * 1000);
            }
            unterminated_orders_t.clear();

            //全部发送完毕后，等待 1s ， 判断 handle_flag , 如没有完成，则retry
            sleep(1);
        }
        else if (!finish_qryorder_result_stats_)
        {
            // wait order query result return back
            sleep(wait_seconds);
            wait_seconds += 5;
        }
    }
}

void MyFemasTradeSpi::OnPackageStart(int nTopicID, int nSequenceNo)
{
    // 不能用于识别初始化时的恢复数据，每单个Rtn消息都有开始结束
    //TNL_LOG_INFO("OnPackageStart, nTopicID:%d, nSequenceNo:%d", nTopicID, nSequenceNo);
}

void MyFemasTradeSpi::OnPackageEnd(int nTopicID, int nSequenceNo)
{
    // 不能用于识别初始化时的恢复数据，每单个Rtn消息都有开始结束
    //TNL_LOG_INFO("OnPackageEnd, nTopicID:%d, nSequenceNo:%d", nTopicID, nSequenceNo);
}
