#include "zeusing_trade_interface.h"

#include <dirent.h>
#include <stdlib.h>
#include <iomanip>
#include <algorithm>

#include "my_protocol_packager.h"
#include "check_schedule.h"
#include "zeusing_data_formater.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "qtm_with_code.h"

using namespace std;

void MYTraderSpi::ReportErrorState(int api_error_no, const std::string &error_msg)
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

MYTraderSpi::MYTraderSpi(const TunnelConfigData &cfg)
    : cfg_(cfg)
{
    connected_ = false;
    logoned_ = false;
    memset(trade_day, 0, sizeof(trade_day));

    in_init_state_ = true;
    memset(trade_day, 0, sizeof(trade_day));
    available_fund = 0;

    // whether it is need to support cancel all order when init
    have_handled_unterminated_orders_ = true;
    if (cfg_.Initial_policy_param().cancel_orders_at_start)
    {
        have_handled_unterminated_orders_ = false;
    }
    finish_query_canceltimes_ = false;
    if (cfg_.Compliance_check_param().init_cancel_times_at_start == 0)
    {
        finish_query_canceltimes_ = true;
    }
    finish_query_contracts_ = false;

    front_id_ = 0;
    session_id_ = 0;
    max_order_ref_ = 0;

    api_trade_context_.InitOrderRef(cfg_.App_cfg().orderref_prefix_id);

    // 从配置解析参数
    if (!ParseConfig())
    {
        return;
    }

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "zeusing_%s_%d_%u", tunnel_info_.account.c_str(), cfg_.App_cfg().orderref_prefix_id, getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

    //Create Directory
    char cur_path[256];
    char full_path[256];
    getcwd(cur_path, sizeof(cur_path));
    sprintf(full_path, "%s/zeusing_flow_dir_%s_%02d/", cur_path, tunnel_info_.account.c_str(), cfg_.App_cfg().orderref_prefix_id);

    // check whether dir exist
    if (opendir(full_path) == NULL)
    {
        mkdir(full_path, 0755);
    }

    // create
    api_ = CZeusingFtdcTraderApi::CreateFtdcTraderApi(full_path);
    api_->RegisterSpi(this);

    // subscribe
    api_->SubscribePrivateTopic(ZEUSING_TERT_QUICK);
    api_->SubscribePublicTopic(ZEUSING_TERT_QUICK);

    // set front address
    for (const std::string &v : cfg.Logon_config().front_addrs)
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

MYTraderSpi::~MYTraderSpi(void)
{
    if (api_)
    {
        api_->RegisterSpi(NULL);
        api_->Release();
        api_ = NULL;
    }
}

bool MYTraderSpi::ParseConfig()
{
    // 用户密码
    tunnel_info_.account = cfg_.Logon_config().investorid;
    pswd_ = cfg_.Logon_config().password;
    return true;
}

void MYTraderSpi::OnFrontConnected()
{
    TNL_LOG_WARN("OnFrontConnected.");
    connected_ = true;
    CZeusingFtdcReqAuthenticateField req;
    memset(&req, 0, sizeof(CZeusingFtdcReqAuthenticateField));
    strncpy(req.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TZeusingFtdcBrokerIDType));
    strncpy(req.UserID, cfg_.Logon_config().clientid.c_str(), sizeof(TZeusingFtdcUserIDType));
    strncpy(req.UserProductInfo, cfg_.Logon_config().UserProductInfo.c_str(), sizeof(TZeusingFtdcProductInfoType));
    strncpy(req.AuthCode, cfg_.Logon_config().AuthCode.c_str(), sizeof(TZeusingFtdcAuthCodeType));

    api_->ReqAuthenticate(&req, 0);
}

void MYTraderSpi::ReqLogin()
{
    // 成功登录后，断开不再重新登录
    if (!in_init_state_)
    {
        return;
    }

    // wait
    sleep(3);
    CZeusingFtdcReqUserLoginField login_data;
    memset(&login_data, 0, sizeof(CZeusingFtdcReqUserLoginField));
    strncpy(login_data.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TZeusingFtdcBrokerIDType));
    strncpy(login_data.UserID, cfg_.Logon_config().clientid.c_str(), sizeof(TZeusingFtdcUserIDType));
    strncpy(login_data.Password, cfg_.Logon_config().password.c_str(), sizeof(TZeusingFtdcPasswordType));

    api_->ReqUserLogin(&login_data, 0);
}

void MYTraderSpi::OnFrontDisconnected(int nReason)
{
    logoned_ = false;
    connected_ = false;

    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
    TNL_LOG_WARN("OnFrontDisconnected, nReason: %d", nReason);
}

void MYTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
    TNL_LOG_WARN("OnHeartBeatWarning, nTimeLapse: %d", nTimeLapse);
}

void MYTraderSpi::OnRspAuthenticate(CZeusingFtdcRspAuthenticateField *pf, CZeusingFtdcRspInfoField *rsp, int req_id, bool last_flag)
{
    if (rsp == NULL || rsp->ErrorID == 0)
    {
        CZeusingFtdcReqUserLoginField login_data;
        memset(&login_data, 0, sizeof(CZeusingFtdcReqUserLoginField));
        strncpy(login_data.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TZeusingFtdcBrokerIDType));
        strncpy(login_data.UserID, cfg_.Logon_config().clientid.c_str(), sizeof(TZeusingFtdcUserIDType));
        strncpy(login_data.Password, cfg_.Logon_config().password.c_str(), sizeof(TZeusingFtdcPasswordType));

        api_->ReqUserLogin(&login_data, 0);
    }
    else
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_FAIL);
        TNL_LOG_ERROR("OnRspAuthenticate return fail, ErrorID:%d", rsp->ErrorID);
    }
}

void MYTraderSpi::OnRspUserLogin(CZeusingFtdcRspUserLoginField *pRspUserLogin, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TNL_LOG_INFO("OnRspUserLogin: requestid = %d, last_flag=%d \n%s \n%s",
        nRequestID, bIsLast,
        DataStructFormater::ToString(pRspUserLogin).c_str(),
        DataStructFormater::ToString(pRspInfo).c_str());

    // logon success
    if (pRspInfo == NULL || pRspInfo->ErrorID == 0)
    {
        // set session parameters
        front_id_ = pRspUserLogin->FrontID;
        session_id_ = pRspUserLogin->SessionID;
        max_order_ref_ = atoi(pRspUserLogin->MaxOrderRef);
        if (max_order_ref_ < 1000)
        {
            max_order_ref_ = 1000;
        }

        // 设置到交易环境变量中
        api_trade_context_.SetOrderRef(max_order_ref_);

        CZeusingFtdcSettlementInfoConfirmField req;
        memset(&req, 0, sizeof(req));
        strncpy(req.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TZeusingFtdcBrokerIDType));
        strncpy(req.InvestorID, tunnel_info_.account.c_str(), sizeof(TZeusingFtdcInvestorIDType));
        int ret = api_->ReqSettlementInfoConfirm(&req, 0);
        TNL_LOG_WARN("ReqSettlementInfoConfirm, return: %d", ret);

        const char *td = api_->GetTradingDay();
        strncpy(trade_day, td, sizeof(trade_day));

        logoned_ = true;
        in_init_state_ = false;

        std::thread t_qry_account(&MYTraderSpi::QueryAccount, this);
        t_qry_account.detach();
    }
    else
    {
        int standard_error_no = cfg_.GetStandardErrorNo(pRspInfo->ErrorID);
        TNL_LOG_WARN("OnRspUserLogin, external errorid: %d; Internal errorid: %d", pRspInfo->ErrorID, standard_error_no);

        // 重新登陆
        std::thread req_login(&MYTraderSpi::ReqLogin, this);
        req_login.detach();
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_FAIL);
    }
}

void MYTraderSpi::QueryAccount()
{
    // query account
    CZeusingFtdcQryTradingAccountField qry_account;
    memset(&qry_account, 0, sizeof(qry_account));
    strncpy(qry_account.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TZeusingFtdcBrokerIDType));
    strncpy(qry_account.InvestorID, tunnel_info_.account.c_str(), sizeof(TZeusingFtdcInvestorIDType));

    while (true)
    {
        int ret = api_->ReqQryTradingAccount(&qry_account, 0);
        TNL_LOG_INFO("ReqQryTradingAccount, return: %d", ret);

        if (ret == 0)
        {
            break;
        }
        sleep(1);
    }
}

void MYTraderSpi::QueryContractInfo()
{
    // query instrument
    CZeusingFtdcQryInstrumentField qry_param;
    memset(&qry_param, 0, sizeof(qry_param));
    const char * ex = DataFieldConvert::ExchCodeToExchName(cfg_.Logon_config().exch_code[0]);
    memcpy(qry_param.ExchangeID, ex, strlen(ex));
//    memcpy(qry_param.InstrumentID, "", strlen(""));
//    memcpy(qry_param.ExchangeInstID, "", strlen(""));
//    memcpy(qry_param.ProductID, "", strlen(""));

    while (true)
    {
        int ret = api_->ReqQryInstrument(&qry_param, 0);
        TNL_LOG_INFO("ReqQryInstrument, return: %d", ret);

        if (ret == 0)
        {
            break;
        }
        sleep(1);
    }
}

void MYTraderSpi::OnRspUserLogout(CZeusingFtdcUserLogoutField *pUserLogout, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    try
    {
        logoned_ = false;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_OUT);

        TNL_LOG_WARN("OnRspUserLogout: requestid = %d, last_flag=%d \n%s \n%s",
            nRequestID, bIsLast,
            DataStructFormater::ToString(pUserLogout).c_str(),
            DataStructFormater::ToString(pRspInfo).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspUserLogout.");
    }
}

void MYTraderSpi::OnRspSettlementInfoConfirm(CZeusingFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
    CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    try
    {
        TNL_LOG_DEBUG("OnRspSettlementInfoConfirm: requestid = %d, last_flag=%d \n%s \n%s",
            nRequestID, bIsLast,
            DataStructFormater::ToString(pSettlementInfoConfirm).c_str(),
            DataStructFormater::ToString(pRspInfo).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspSettlementInfoConfirm.");
    }
}

void MYTraderSpi::OnRspQrySettlementInfo(CZeusingFtdcSettlementInfoField *pSettlementInfo, CZeusingFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    try
    {
        TNL_LOG_DEBUG("OnRspQrySettlementInfo: requestid = %d, last_flag=%d \n%s \n%s",
            nRequestID, bIsLast,
            DataStructFormater::ToString(pSettlementInfo).c_str(),
            DataStructFormater::ToString(pRspInfo).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspQrySettlementInfo.");
    }
}

void MYTraderSpi::OnRspQrySettlementInfoConfirm(CZeusingFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
    CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    try
    {
        TNL_LOG_DEBUG("OnRspQrySettlementInfoConfirm: requestid = %d, last_flag=%d \n%s \n%s",
            nRequestID, bIsLast,
            DataStructFormater::ToString(pSettlementInfoConfirm).c_str(),
            DataStructFormater::ToString(pRspInfo).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspQrySettlementInfoConfirm.");
    }
}

void MYTraderSpi::OnRspError(CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    try
    {
        TNL_LOG_WARN("OnRspError: requestid = %d, last_flag=%d \n%s",
            nRequestID, bIsLast, DataStructFormater::ToString(pRspInfo).c_str());
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspError.");
    }
}

void Init(CZeusingFtdcTradeField* fake_trade_rsp, const CZeusingFtdcOrderField * pOrder)
{
    // 仅将要用的字段赋值
    memcpy(fake_trade_rsp->OrderSysID, pOrder->OrderSysID, sizeof(TZeusingFtdcOrderSysIDType));
    memcpy(fake_trade_rsp->InstrumentID, pOrder->InstrumentID, sizeof(TZeusingFtdcInstrumentIDType));
    fake_trade_rsp->Direction = pOrder->Direction;
    fake_trade_rsp->OffsetFlag = pOrder->CombOffsetFlag[0];
    fake_trade_rsp->Volume = pOrder->VolumeTraded;
    fake_trade_rsp->Price = pOrder->LimitPrice;
    memcpy(fake_trade_rsp->TradeID, pOrder->OrderSysID, sizeof(TZeusingFtdcTradeIDType));
    fake_trade_rsp->HedgeFlag = pOrder->CombHedgeFlag[0];
}

void MYTraderSpi::OnRtnOrder(CZeusingFtdcOrderField *pOrder)
{
    try
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("OnRtnOrder when tunnel not ready.");
            return;
        }
        if (!pOrder)
        {
            TNL_LOG_WARN("OnRtnOrder, pOrder is null.");
            return;
        }

        // check if order is placed by this session
        if (pOrder->FrontID != front_id_ || pOrder->SessionID != session_id_)
        {
            TNL_LOG_INFO("OnRtnOrder - order not of this session, FrontID: %d;SessionID: %d;OrderRef: %s",
                pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef);
            return;
        }

        OrderRefDataType order_ref = atoll(pOrder->OrderRef);
        const OriginalReqInfo *p = api_trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            // Save Order SysID
            long order_sys_id = atol(pOrder->OrderSysID);
            if (order_sys_id != 0)
            {
                api_trade_context_.SaveOrderSysIDOfSerialNo(p->serial_no, order_sys_id);
            }

            // should adapt response for cffex simulator test;
            // dont need adapt in dce.
            if (p->order_type == MY_TNL_HF_FAK && p->exchange_code == MY_TNL_EC_CFFEX)
            {
                // FAK指令
                OnRtnOrderFak(pOrder, p, order_ref);
            }
            else
            {
                OnRtnOrderNormal(pOrder, p, order_ref);
            }

            TNL_LOG_DEBUG(DataStructFormater::ToString(pOrder).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnRtnOrder can't get original place order info of order ref: %s", pOrder->OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnOrder.");
    }
}

void MYTraderSpi::OnRtnTrade(CZeusingFtdcTradeField *pTrade)
{
    try
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("OnRtnTrade when tunnel not ready.");
            return;
        }
        if (!pTrade)
        {
            TNL_LOG_WARN("OnRtnTrade, pTrade is null.");
            return;
        }

        OrderRefDataType order_ref = atoll(pTrade->OrderRef);
        const OriginalReqInfo *p = api_trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            long order_sys_id = atol(pTrade->OrderSysID);
            long original_sys_id = api_trade_context_.GetOrderSysIDOfSerialNo(p->serial_no);
            // not my own order
            if (order_sys_id != original_sys_id)
            {
                TNL_LOG_INFO("OnRtnTrade - order not placed by this session, order_sys_id: %s", pTrade->OrderSysID);
                return;
            }
            // FAK修改，部成的单，在第一个委托回报中报告成交，后续不再回报
            if (p->Cancel_serial_no() == "0")
            {
                // "0"标识已经回报
            }
            else
            {
                // 成交
                T_TradeReturn trade_return;
                ApiPacker::TradeReturn(pTrade, p, trade_return);
                if (trade_rtn_handler_) trade_rtn_handler_(&trade_return);
                LogUtil::OnTradeReturn(&trade_return, tunnel_info_);
            }

            TNL_LOG_DEBUG(DataStructFormater::ToString(pTrade).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnRtnTrade can't get original place order info of order ref: %s", pTrade->OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnTrade.");
    }
}

// 报单拒绝
void MYTraderSpi::OnRspOrderInsert(CZeusingFtdcInputOrderField *pInputOrder, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        if (!pInputOrder)
        {
            TNL_LOG_WARN("OnRspOrderInsert, pInputOrder is null.");
            return;
        }
        if (pRspInfo)
        {
            api_err_no = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
        }
        TNL_LOG_WARN("OnRspOrderInsert, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            api_err_no, standard_error_no, pInputOrder->OrderRef);

        OrderRefDataType OrderRef = atoll(pInputOrder->OrderRef);
        const OriginalReqInfo *p = api_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            if (api_trade_context_.CheckAndSetHaveHandleInsertErr(p->serial_no))
            {
                T_OrderRespond pOrderRespond;
                ApiPacker::OrderRespond(standard_error_no, p->serial_no, "0", '4', '0', pOrderRespond);

                // 报单失败，报告合规检查
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    OrderRef,
                    p->exchange_code,
                    pInputOrder->InstrumentID,
                    pInputOrder->VolumeTotalOriginal,
                    p->hedge_type,
                    p->open_close_flag,
                    p->order_type);

                // 应答只发给请求的客户
                if (order_rsp_handler_) order_rsp_handler_(&pOrderRespond);
                LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("rsp err insert have handled, order ref: %s", pInputOrder->OrderRef);
            }

            TNL_LOG_DEBUG("OnRspOrderInsert: requestid = %d, last_flag=%d \n%s \n%s",
                nRequestID, bIsLast,
                DataStructFormater::ToString(pInputOrder).c_str(),
                DataStructFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("can't get original place order info of order ref: %s", pInputOrder->OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspOrderInsert.");
    }
}

// 撤单拒绝
void MYTraderSpi::OnRspOrderAction(CZeusingFtdcInputOrderActionField *pInputOrderAction, CZeusingFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    try
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("OnRspOrderAction when cancel unterminated orders.");
            return;
        }
        int standard_error_no = 0;
        int api_err_no = 0;
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        if (!pInputOrderAction)
        {
            TNL_LOG_WARN("OnRspOrderAction, pInputOrderAction is null.");
            return;
        }
        if (pRspInfo)
        {
            api_err_no = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
        }

        TNL_LOG_WARN("OnRspOrderAction, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            api_err_no, standard_error_no, pInputOrderAction->OrderRef);

        OrderRefDataType OrderRef = atoll(pInputOrderAction->OrderRef);
        const OriginalReqInfo *p = api_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            long cancel_serial_no = atol(p->Cancel_serial_no().c_str());
            if (cancel_serial_no > 0)
            {
                if (api_trade_context_.CheckAndSetHaveHandleActionErr(cancel_serial_no))
                {
                    T_CancelRespond cancel_respond;
                    ApiPacker::CancelRespond(standard_error_no, cancel_serial_no, 0L, cancel_respond);

                    // 应答只发给请求的客户
                    if (cancel_rsp_handler_) cancel_rsp_handler_(&cancel_respond);
                    LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);

                    // 撤单拒绝，报告合规检查
                    ComplianceCheck_OnOrderCancelFailed(
                        tunnel_info_.account.c_str(),
                        pInputOrderAction->InstrumentID,
                        OrderRef);
                }
                else
                {
                    TNL_LOG_WARN("rsp err action have handled, order sn: %ld", p->serial_no);
                }

                TNL_LOG_DEBUG("OnRspOrderAction: requestid = %d, last_flag=%d \n%s \n%s",
                    nRequestID, bIsLast,
                    DataStructFormater::ToString(pInputOrderAction).c_str(),
                    DataStructFormater::ToString(pRspInfo).c_str());
            }
            else
            {
                // 撤单拒绝，报告合规检查
                ComplianceCheck_OnOrderCancelFailed(
                    tunnel_info_.account.c_str(),
                    pInputOrderAction->InstrumentID,
                    OrderRef);

                TNL_LOG_WARN("cancel order: %ld from outside.", p->serial_no);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspOrderAction.");
    }
}

void MYTraderSpi::OnErrRtnOrderInsert(CZeusingFtdcInputOrderField *pInputOrder, CZeusingFtdcRspInfoField *pRspInfo)
{
    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        if (!pInputOrder)
        {
            TNL_LOG_WARN("OnErrRtnOrderInsert, pInputOrder is null.");
            return;
        }
        if (pRspInfo)
        {
            api_err_no = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
        }
        TNL_LOG_WARN("OnErrRtnOrderInsert, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            api_err_no, standard_error_no, pInputOrder->OrderRef);

        OrderRefDataType OrderRef = atoll(pInputOrder->OrderRef);
        const OriginalReqInfo *p = api_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            if (api_trade_context_.CheckAndSetHaveHandleInsertErr(p->serial_no))
            {
                T_OrderRespond pOrderRespond;
                ApiPacker::OrderRespond(standard_error_no, p->serial_no, "0", '4', '0', pOrderRespond);

                // 报单失败，报告合规检查
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    OrderRef,
                    p->exchange_code,
                    pInputOrder->InstrumentID,
                    pInputOrder->VolumeTotalOriginal,
                    p->hedge_type,
                    p->open_close_flag,
                    p->order_type);

                // 应答只发给请求的客户
                if (order_rsp_handler_) order_rsp_handler_(&pOrderRespond);
                LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("rtn err insert have handled, order ref: %s", pInputOrder->OrderRef);
            }

            TNL_LOG_DEBUG("OnErrRtnOrderInsert:  \n%s \n%s",
                DataStructFormater::ToString(pInputOrder).c_str(),
                DataStructFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("can't get original place order info of order ref: %s", pInputOrder->OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspOrderInsert.");
    }
}

void MYTraderSpi::OnErrRtnOrderAction(CZeusingFtdcOrderActionField *pOrderAction, CZeusingFtdcRspInfoField *pRspInfo)
{
    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        if (!pOrderAction)
        {
            TNL_LOG_WARN("OnErrRtnOrderAction, pOrderAction is null.");
            return;
        }
        if (pRspInfo)
        {
            api_err_no = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
        }

        TNL_LOG_WARN("OnErrRtnOrderAction, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            api_err_no, standard_error_no, pOrderAction->OrderRef);

        OrderRefDataType OrderRef = atoll(pOrderAction->OrderRef);
        const OriginalReqInfo *p = api_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            long cancel_serial_no = atol(p->Cancel_serial_no().c_str());
            if (cancel_serial_no > 0)
            {
                if (api_trade_context_.CheckAndSetHaveHandleActionErr(cancel_serial_no))
                {
                    T_CancelRespond cancel_respond;
                    ApiPacker::CancelRespond(standard_error_no, cancel_serial_no, 0L, cancel_respond);

                    // 应答只发给请求的客户
                    if (cancel_rsp_handler_) cancel_rsp_handler_(&cancel_respond);
                    LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);

                    // 撤单拒绝，报告合规检查
                    ComplianceCheck_OnOrderCancelFailed(
                        tunnel_info_.account.c_str(),
                        pOrderAction->InstrumentID,
                        OrderRef);
                }
                else
                {
                    TNL_LOG_WARN("rtn err action have handled, order sn: %ld", p->serial_no);
                }
            }
            else if (pOrderAction->FrontID == front_id_ && pOrderAction->SessionID == session_id_)
            {
                // 撤单拒绝，报告合规检查
                ComplianceCheck_OnOrderCancelFailed(
                    tunnel_info_.account.c_str(),
                    pOrderAction->InstrumentID,
                    OrderRef);
            }
            else
            {
                TNL_LOG_WARN("cancel order: %ld from outside.", p->serial_no);
            }

            TNL_LOG_DEBUG("OnErrRtnOrderAction:  \n%s \n%s",
                DataStructFormater::ToString(pOrderAction).c_str(),
                DataStructFormater::ToString(pRspInfo).c_str());
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspOrderAction.");
    }
}

void MYTraderSpi::OnRtnOrderFak(CZeusingFtdcOrderField * pOrder, const OriginalReqInfo * p, OrderRefDataType order_ref)
{
    if (p->Cancel_serial_no() == "0")
    {
        // 已完成回报
    }
    else if (pOrder->OrderStatus == ZEUSING_FTDC_OST_Canceled && pOrder->VolumeTraded > 0)
    {
        // 如果是部成，回报委托/成交/已撤

        // 部成委托回报
        CZeusingFtdcOrderField fake_partial_rsp;
        memcpy(&fake_partial_rsp, pOrder, sizeof(CZeusingFtdcOrderField));
        fake_partial_rsp.OrderStatus = ZEUSING_FTDC_OST_PartTradedQueueing;
        T_OrderReturn order_return;
        ApiPacker::OrderReturn(&fake_partial_rsp, p, order_return);
        if (order_rtn_handler_) order_rtn_handler_(&order_return);
        LogUtil::OnOrderReturn(&order_return, tunnel_info_);

        // 成交回报
        CZeusingFtdcTradeField fake_trade_rsp;
        Init(&fake_trade_rsp, pOrder);
        T_TradeReturn trade_return;
        ApiPacker::TradeReturn(&fake_trade_rsp, p, trade_return);
        if (trade_rtn_handler_) trade_rtn_handler_(&trade_return);
        LogUtil::OnTradeReturn(&trade_return, tunnel_info_);

        // 已撤回报
        T_OrderReturn order_cancle_return;
        ApiPacker::OrderReturn(pOrder, p, order_cancle_return);
        if (order_rtn_handler_) order_rtn_handler_(&order_cancle_return);
        LogUtil::OnOrderReturn(&order_cancle_return, tunnel_info_);

        // 已撤，报告合规检查
        ComplianceCheck_OnOrderCanceled(
            tunnel_info_.account.c_str(),
            p->stock_code.c_str(),
            order_ref,
            p->exchange_code,
            pOrder->VolumeTotal,
            p->hedge_type,
            p->open_close_flag);

        // 做标记，再也不用回报了
        api_trade_context_.UpdateCancelOrderRef(order_ref, "0");
    }
    else
    {
        // 全成，无需报告合规检查（FAK报单不进入委托列表）

        // 已撤，报告合规检查
        if (pOrder->OrderStatus == ZEUSING_FTDC_OST_Canceled)
        {
            ComplianceCheck_OnOrderCanceled(
                tunnel_info_.account.c_str(),
                p->stock_code.c_str(),
                order_ref,
                p->exchange_code,
                pOrder->VolumeTotal,
                p->hedge_type,
                p->open_close_flag);
        }

        if (pOrder->ActiveUserID[0] == '\0' && !api_trade_context_.IsAnsweredPlaceOrder(order_ref))
        {
            T_OrderRespond pOrderRespond;
            ApiPacker::OrderRespond(0, p->serial_no, pOrder->OrderSysID, pOrder->OrderSubmitStatus, pOrder->OrderStatus,
                pOrderRespond);
            if (order_rsp_handler_) order_rsp_handler_(&pOrderRespond);
            api_trade_context_.SetAnsweredPlaceOrder(order_ref);
            LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
        }
        else if (pOrder->ActiveUserID[0] != '\0' && !api_trade_context_.IsAnsweredCancelOrder(order_ref))
        {
            long cancel_serial_no = atol(p->Cancel_serial_no().c_str());
            if (cancel_serial_no > 0)
            {
                T_CancelRespond cancel_respond;
                ApiPacker::CancelRespond(0, cancel_serial_no, pOrder->OrderSysID, cancel_respond);
                if (cancel_rsp_handler_) cancel_rsp_handler_(&cancel_respond);
                api_trade_context_.SetAnsweredCancelOrder(order_ref);
                LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
            }
            else
            {
                T_OrderReturn order_return;
                ApiPacker::OrderReturn(pOrder, p, order_return);
                if (order_rtn_handler_) order_rtn_handler_(&order_return);
                LogUtil::OnOrderReturn(&order_return, tunnel_info_);
            }
        }
        else
        {
            T_OrderReturn order_return;
            ApiPacker::OrderReturn(pOrder, p, order_return);
            if (order_rtn_handler_) order_rtn_handler_(&order_return);
            LogUtil::OnOrderReturn(&order_return, tunnel_info_);
        }
    }
}

void MYTraderSpi::OnRtnOrderNormal(CZeusingFtdcOrderField * pOrder, const OriginalReqInfo * p, OrderRefDataType order_ref)
{
    // 全成，报告合规检查
    if (pOrder->OrderStatus == ZEUSING_FTDC_OST_AllTraded)
    {
        ComplianceCheck_OnOrderFilled(
            tunnel_info_.account.c_str(),
            order_ref);
    }

    // 已撤，报告合规检查
    if (pOrder->OrderStatus == ZEUSING_FTDC_OST_Canceled)
    {
        ComplianceCheck_OnOrderCanceled(
            tunnel_info_.account.c_str(),
            p->stock_code.c_str(),
            order_ref,
            p->exchange_code,
            pOrder->VolumeTotal,
            p->hedge_type,
            p->open_close_flag);
    }

    if (pOrder->ActiveUserID[0] == '\0' && !api_trade_context_.IsAnsweredPlaceOrder(order_ref))
    {
        T_OrderRespond pOrderRespond;
        ApiPacker::OrderRespond(0, p->serial_no, pOrder->OrderSysID, pOrder->OrderSubmitStatus, pOrder->OrderStatus, pOrderRespond);
        if (order_rsp_handler_) order_rsp_handler_(&pOrderRespond);
        api_trade_context_.SetAnsweredPlaceOrder(order_ref);
        LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
    }
    else if (pOrder->ActiveUserID[0] != '\0' && !api_trade_context_.IsAnsweredCancelOrder(order_ref))
    {
        long cancel_serial_no = atol(p->Cancel_serial_no().c_str());
        if (cancel_serial_no > 0)
        {
            T_CancelRespond cancel_respond;
            ApiPacker::CancelRespond(0, cancel_serial_no, pOrder->OrderSysID, cancel_respond);
            if (cancel_rsp_handler_) cancel_rsp_handler_(&cancel_respond);
            api_trade_context_.SetAnsweredCancelOrder(order_ref);
            LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
        }
        else
        {
            T_OrderReturn order_return;
            ApiPacker::OrderReturn(pOrder, p, order_return);
            if (order_rtn_handler_) order_rtn_handler_(&order_return);
            LogUtil::OnOrderReturn(&order_return, tunnel_info_);
            TNL_LOG_WARN("cancel order: %ld from outside.", p->serial_no);
        }
    }
    else
    {
        T_OrderReturn order_return;
        ApiPacker::OrderReturn(pOrder, p, order_return);
        if (order_rtn_handler_) order_rtn_handler_(&order_return);
        LogUtil::OnOrderReturn(&order_return, tunnel_info_);
    }
}

void MYTraderSpi::OnRspQryOrder(CZeusingFtdcOrderField* pOrder, CZeusingFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!HaveFinishQueryOrders())
    {
        if (!finish_query_canceltimes_)
        {
            std::unique_lock<std::mutex> lock(stats_canceltimes_sync_);
            if (pOrder && pOrder->OrderStatus == ZEUSING_FTDC_OST_Canceled
                && pOrder->OrderSubmitStatus != ZEUSING_FTDC_OSS_InsertRejected)
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

            if (bIsLast)
            {
                for (std::map<std::string, int>::iterator it = cancel_times_of_contract.begin();
                    it != cancel_times_of_contract.end(); ++it)
                {
                    ComplianceCheck_SetCancelTimes(tunnel_info_.account.c_str(), it->first.c_str(), it->second);
                }
                cancel_times_of_contract.clear();
                finish_query_canceltimes_ = true;
                TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_CANCEL_TIMES_SUCCESS);
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
                TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::HANDLED_UNTERMINATED_ORDERS_SUCCESS);
            }
        }

        if (bIsLast)
        {
            qry_order_finish_cond_.notify_one();
        }

        if (TunnelIsReady())
        {
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
        }

        TNL_LOG_INFO("OnRspQryOrder when cancel unterminated orders, order: %p, last: %d", pOrder, bIsLast);
        return;
    }

    T_OrderDetailReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ret.error_no = -1;
        if (qry_order_rtn_handler_) qry_order_rtn_handler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }

    TNL_LOG_DEBUG("OnRspQryOrder: requestid = %d, last_flag=%d \n%s \n%s",
        nRequestID, bIsLast,
        DataStructFormater::ToString(pOrder).c_str(),
        DataStructFormater::ToString(pRspInfo).c_str());

    // empty Order detail
    if (!pOrder)
    {
        ret.error_no = 0;
        if (qry_order_rtn_handler_) qry_order_rtn_handler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }

    OrderDetail od;
    strncpy(od.stock_code, pOrder->InstrumentID, sizeof(TZeusingFtdcInstrumentIDType));
    od.entrust_no = atol(pOrder->OrderSysID);
    od.order_kind = DataFieldConvert::GetMYPriceType(pOrder->OrderPriceType);
    od.direction = pOrder->Direction;
    od.open_close = pOrder->CombOffsetFlag[0];
    od.speculator = DataFieldConvert::GetMYHedgeFlag(pOrder->CombHedgeFlag[0]);
    od.entrust_status = DataFieldConvert::GetMYEntrustStatus(pOrder->OrderSubmitStatus, pOrder->OrderStatus);
    od.limit_price = pOrder->LimitPrice;
    od.volume = pOrder->VolumeTotalOriginal;
    od.volume_traded = pOrder->VolumeTraded;
    od.volume_remain = pOrder->VolumeTotal;
    od_buffer_.push_back(od);

    if (bIsLast)
    {
        ret.datas.swap(od_buffer_);
        ret.error_no = 0;
        if (qry_order_rtn_handler_) qry_order_rtn_handler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}

void MYTraderSpi::OnRspQryTrade(CZeusingFtdcTradeField* pTrade, CZeusingFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    T_TradeDetailReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ret.error_no = -1;
        if (qry_trade_rtn_handler_) qry_trade_rtn_handler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    TNL_LOG_DEBUG("OnRspQryTrade: requestid = %d, last_flag=%d \n%s \n%s",
        nRequestID, bIsLast,
        DataStructFormater::ToString(pTrade).c_str(),
        DataStructFormater::ToString(pRspInfo).c_str());

    // empty trade detail
    if (!pTrade)
    {
        ret.error_no = 0;
        if (qry_trade_rtn_handler_) qry_trade_rtn_handler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    TradeDetail td;
    strncpy(td.stock_code, pTrade->InstrumentID, sizeof(TZeusingFtdcInstrumentIDType));
    td.entrust_no = atol(pTrade->OrderSysID);
    td.direction = pTrade->Direction;
    td.open_close = pTrade->OffsetFlag;
    td.speculator = DataFieldConvert::GetMYHedgeFlag(pTrade->HedgeFlag);
    td.trade_price = pTrade->Price;
    td.trade_volume = pTrade->Volume;
    strncpy(td.trade_time, pTrade->TradeTime, sizeof(TZeusingFtdcTimeType));
    td_buffer_.push_back(td);

    if (bIsLast)
    {
        ret.datas.swap(td_buffer_);
        ret.error_no = 0;
        if (qry_trade_rtn_handler_) qry_trade_rtn_handler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

namespace
{
struct PositionDetailPred
{
    PositionDetailPred(const PositionDetail &v)
        : v_(v)
    {
    }
    bool operator()(const PositionDetail &l)
    {
        return v_.direction == l.direction && strcmp(v_.stock_code, l.stock_code) == 0;
    }

private:
    const PositionDetail v_;
};
}

///请求查询投资者持仓响应
void MYTraderSpi::OnRspQryInvestorPosition(CZeusingFtdcInvestorPositionField *pf, CZeusingFtdcRspInfoField *rsp, int req_id, bool last)
{
    TNL_LOG_DEBUG("OnRspQryInvestorPosition: requestid = %d, last_flag=%d \n%s \n%s",
        req_id, last,
        DataStructFormater::ToString(pf).c_str(),
        DataStructFormater::ToString(rsp).c_str());
    T_PositionReturn ret;

    // respond error
    if (rsp && rsp->ErrorID != 0)
    {
        ret.error_no = -1;
        if (qry_pos_rtn_handler_) qry_pos_rtn_handler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    if (pf)
    {
        PositionDetail pos;
        memset(&pos, 0, sizeof(pos));
        strncpy(pos.stock_code, pf->InstrumentID, sizeof(pos.stock_code));
        pos.exchange_type = DataFieldConvert::ExchNameToExchCode(pf->ExchangeID);

        int cur_pos = (int) pf->Position;
        int yd_pos = (int) pf->YdPosition;
        if (pf->PosiDirection == ZEUSING_FTDC_PD_Long)
        {
            pos.direction = MY_TNL_D_BUY;
        }
        else if (pf->PosiDirection == ZEUSING_FTDC_PD_Short)
        {
            pos.direction = MY_TNL_D_SELL;
        }
        else if (pf->PosiDirection == ZEUSING_FTDC_PD_Net)
        {
            if (cur_pos < 0 || yd_pos < 0)
            {
                pos.direction = MY_TNL_D_SELL;
                cur_pos = -cur_pos;
                yd_pos = -yd_pos;
            }
            else
            {
                pos.direction = MY_TNL_D_BUY;
            }
        }

        // 今仓，当前的仓位
        if (cur_pos != 0)
        {
            pos.position = cur_pos;
            pos.position_avg_price = pf->OpenCost / pos.position / GetVolumeMultiple(pos.stock_code);
        }
        if (yd_pos != 0)
        {
            pos.yestoday_position = (int) yd_pos;
            pos.yd_position_avg_price = pf->OpenCost / pos.yestoday_position / GetVolumeMultiple(pos.stock_code);
        }

        TNL_LOG_INFO("pos_data %s - pos:%d, yd_pod:%d", pos.stock_code, pos.position, pos.yestoday_position);
        if (pos.position > 0 || pos.yestoday_position > 0)
        {
            pos_buffer_.push_back(pos);
        }
    }

    if (last)
    {
        TNL_LOG_INFO("receive last respond of query position, send to client");
        ret.datas.swap(pos_buffer_);

        // TODO return fund in stock system and option trade
        ret.datas.insert(ret.datas.begin(), PackageAvailableFunc());

        ret.error_no = 0;
        if (qry_pos_rtn_handler_) qry_pos_rtn_handler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}

PositionDetail MYTraderSpi::PackageAvailableFunc()
{
    // insert available fund
    PositionDetail ac;
    memset(&ac, 0, sizeof(ac));
    strcpy(ac.stock_code, "#CASH");
    ac.direction = '0';
    ac.position = (long) available_fund;  // TODO available fund at beginning, don't update after init

    return ac;
}

void MYTraderSpi::OnRspQryInvestorPositionDetail(CZeusingFtdcInvestorPositionDetailField *pf,
    CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TNL_LOG_DEBUG("OnRspQryInvestorPositionDetail: requestid = %d, last_flag=%d \n%s \n%s",
        nRequestID, bIsLast,
        DataStructFormater::ToString(pf).c_str(),
        DataStructFormater::ToString(pRspInfo).c_str());

    T_PositionReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ret.error_no = -1;
        if (qry_pos_rtn_handler_) qry_pos_rtn_handler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    if (pf)
    {
        PositionDetail pos;
        memset(&pos, 0, sizeof(pos));
        strncpy(pos.stock_code, pf->InstrumentID, sizeof(pos.stock_code));
        pos.exchange_type = DataFieldConvert::ExchNameToExchCode(pf->ExchangeID);

        if (pf->Direction == ZEUSING_FTDC_D_Buy)
        {
            pos.direction = MY_TNL_D_BUY;
        }
        else
        {
            pos.direction = MY_TNL_D_SELL;
        }

        // 今仓，当前的仓位
        pos.position = pf->Volume;
        pos.position_avg_price = pf->Volume * pf->OpenPrice; // now is total cost
        if (memcmp(pf->OpenDate, pf->TradingDay, sizeof(pf->OpenDate)) != 0)
        {
            pos.yestoday_position = pf->Volume;
            pos.yd_position_avg_price = pf->Volume * pf->OpenPrice; // now is total cost
        }
        std::vector<PositionDetail>::iterator it = std::find_if(pos_buffer_.begin(), pos_buffer_.end(), PositionDetailPred(pos));
        if (it == pos_buffer_.end())
        {
            if (pos.position > 0 || pos.yestoday_position > 0)
            {
                pos_buffer_.push_back(pos);
            }
        }
        else
        {
            it->position += pos.position;
            it->position_avg_price += pos.position_avg_price; // now is total cost
            it->yestoday_position += pos.yestoday_position;
            it->yd_position_avg_price += pos.yd_position_avg_price; // now is total cost
        }
    }

    if (bIsLast)
    {
        CheckAndSaveYestodayPosition();
        TNL_LOG_INFO("receive last respond of query position, send to client");
        ret.datas.swap(pos_buffer_);

        // TODO need return fund in stock system
        ret.datas.insert(ret.datas.begin(), PackageAvailableFunc());

        ret.error_no = 0;
        if (qry_pos_rtn_handler_) qry_pos_rtn_handler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}

void MYTraderSpi::OnRspQryInstrument(CZeusingFtdcInstrumentField *pf, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    T_ContractInfoReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ret.error_no = -1;
        if (qry_contract_rtn_handler_) qry_contract_rtn_handler_(&ret);
        LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
        return;
    }

    TNL_LOG_DEBUG("OnRspQryInstrument: InstrumentID:%s,ExchangeID:%s,PriceTick:%.4f, last_flag:%d",
        pf->InstrumentID, pf->ExchangeID, pf->PriceTick, bIsLast);
//    TNL_LOG_DEBUG("OnRspQryInstrument: requestid = %d, last_flag=%d \n%s \n%s",
//        nRequestID, bIsLast,
//        DataStructFormater::ToString(pf).c_str(),
//        DataStructFormater::ToString(pRspInfo).c_str());

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        return;
    }

    if (pf)
    {
        ContractInfo ci;
        strncpy(ci.stock_code, pf->InstrumentID, sizeof(ci.stock_code));
        memcpy(ci.TradeDate, trade_day, sizeof(ci.TradeDate));
        memcpy(ci.ExpireDate, pf->ExpireDate, sizeof(ci.ExpireDate));
        ci_buffer_.push_back(ci);

        AddVolumeMultiple(pf->InstrumentID, pf->VolumeMultiple);
    }

    if (bIsLast)
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_CONTRACT_SUCCESS);
        finish_query_contracts_ = true;
        TNL_LOG_DEBUG("finish OnRspQryInstrument, data size:%d", ci_buffer_.size());

        if (!HaveFinishQueryOrders())
        {
            std::thread t_handle_orders(std::bind(&MYTraderSpi::QueryAndHandleOrders, this));
            t_handle_orders.detach();
        }
        else
        {
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_SUCCESS);
        }
    }
}

void MYTraderSpi::OnRspQryTradingAccount(CZeusingFtdcTradingAccountField *pf, CZeusingFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    TNL_LOG_DEBUG("OnRspQryTradingAccount: requestid = %d, last_flag=%d \n%s \n%s",
        nRequestID, bIsLast,
        DataStructFormater::ToString(pf).c_str(),
        DataStructFormater::ToString(pRspInfo).c_str());

    if (pf) available_fund = pf->Available;

    if (bIsLast)
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_ACCOUNT_SUCCESS);
        std::thread t_qry_contract(&MYTraderSpi::QueryContractInfo, this);
        t_qry_contract.detach();
    }
}

CZeusingFtdcInputOrderActionField MYTraderSpi::CreatCancelParam(const CZeusingFtdcOrderField& order_field)
{
    CZeusingFtdcInputOrderActionField cancle_order;
    OrderRefDataType order_ref = api_trade_context_.GetNewOrderRef();
    memset(&cancle_order, 0, sizeof(cancle_order));

    memcpy(cancle_order.ExchangeID, order_field.ExchangeID, sizeof(TZeusingFtdcExchangeIDType));
    memcpy(cancle_order.OrderSysID, order_field.OrderSysID, sizeof(TZeusingFtdcOrderSysIDType));
    memcpy(cancle_order.BrokerID, order_field.BrokerID, sizeof(TZeusingFtdcBrokerIDType));
    memcpy(cancle_order.InvestorID, order_field.InvestorID, sizeof(TZeusingFtdcInvestorIDType));

    cancle_order.OrderActionRef = (int) order_ref;
    cancle_order.ActionFlag = ZEUSING_FTDC_AF_Delete;
    cancle_order.LimitPrice = 0;
    cancle_order.VolumeChange = 0;

    return cancle_order;
}

bool MYTraderSpi::IsOrderTerminate(const CZeusingFtdcOrderField& order_field)
{
    return order_field.OrderStatus == ZEUSING_FTDC_OST_AllTraded
        || order_field.OrderStatus == ZEUSING_FTDC_OST_Canceled;
}

void MYTraderSpi::QueryAndHandleOrders()
{
    // query order detail parameter
    CZeusingFtdcQryOrderField qry_param;
    memset(&qry_param, 0, sizeof(CZeusingFtdcQryOrderField));
    strncpy(qry_param.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TZeusingFtdcBrokerIDType));
    strncpy(qry_param.InvestorID, tunnel_info_.account.c_str(), sizeof(TZeusingFtdcInvestorIDType));

    //超时后没有完成查询，重试。为防止委托单太多，10s都回报不了，每次超时加5s
    int wait_seconds = 10;

    std::vector<CZeusingFtdcOrderField> unterminated_orders_t;
    while (!HaveFinishQueryOrders())
    {
        {
            std::unique_lock<std::mutex> lock(stats_canceltimes_sync_);
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
            TNL_LOG_INFO("ReqQryOrder for (cancel unterminated orders) / (init cancel times), return %d", qry_result);

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

            // 遍历 unterminated_orders 间隔20ms（流控要求），发送撤单请求
            for (const CZeusingFtdcOrderField &order_field : unterminated_orders_t)
            {
                CZeusingFtdcInputOrderActionField action_field = CreatCancelParam(order_field);
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
        else if (!finish_query_canceltimes_)
        {
            // wait order query result return back
            sleep(wait_seconds);
            wait_seconds += 5;
        }
    }
}

//如果已经存储昨仓数据，直接从文件中读取，否则，存储到文件中
void MYTraderSpi::CheckAndSaveYestodayPosition()
{
    // calc avg price
    for (PositionDetail &v : pos_buffer_)
    {
        if (v.position > 0) v.position_avg_price = v.position_avg_price / v.position;
        if (v.yestoday_position > 0) v.yd_position_avg_price = v.yd_position_avg_price / v.yestoday_position;
    }

    // 存储文件名：yd_pos_账号_日期.rec
    std::string yd_pos_file_name = "yd_pos_" + tunnel_info_.account + "_" + std::string(trade_day) + ".rec";
    int ret = access(yd_pos_file_name.c_str(), F_OK);

    bool file_exist = (ret == 0);

    if (file_exist)
    {
        LoadYestodayPositionFromFile(yd_pos_file_name);
    }
    else
    {
        SaveYestodayPositionToFile(yd_pos_file_name);
    }
}

void MYTraderSpi::LoadYestodayPositionFromFile(const std::string &file)
{
    std::ifstream yd_pos_fs(file.c_str());
    if (!yd_pos_fs)
    {
        TNL_LOG_ERROR("open file failed when LoadYestodayPositionFromFile");
        return;
    }
    char r[255];
    while (yd_pos_fs.getline(r, 255))
    {
        std::vector<std::string> fields;
        my_cmn::MYStringSplit(r, fields, ',');

        // 格式：合约，方向，昨仓量，开仓均价
        if (fields.size() != 4)
        {
            TNL_LOG_ERROR("yestoday position data in wrong format, line: %s, file: %s", r, file.c_str());
            continue;
        }
        PositionDetail pos;
        memset(&pos, 0, sizeof(pos));
        strncpy(pos.stock_code, fields[0].c_str(), sizeof(pos.stock_code));
        pos.direction = fields[1][0];
        pos.yestoday_position = atoi(fields[2].c_str());
        pos.yd_position_avg_price = atof(fields[3].c_str());

        std::vector<PositionDetail>::iterator it = std::find_if(pos_buffer_.begin(), pos_buffer_.end(), PositionDetailPred(pos));
        if (it == pos_buffer_.end())
        {
            pos_buffer_.push_back(pos);
        }
        else
        {
            it->yestoday_position = pos.yestoday_position;
            it->yd_position_avg_price = pos.yd_position_avg_price;
        }
    }
}

void MYTraderSpi::SaveYestodayPositionToFile(const std::string &file)
{
    std::ofstream yd_pos_fs(file.c_str());
    if (!yd_pos_fs)
    {
        TNL_LOG_ERROR("open file failed when SaveYestodayPositionToFile");
        return;
    }

    for (PositionDetail v : pos_buffer_)
    {
        if (v.yestoday_position > 0)
        {
            // 格式：合约，方向，昨仓量，开仓均价
            yd_pos_fs << v.stock_code << ","
                << v.direction << ","
                << v.yestoday_position << ","
                << fixed << setprecision(16) << v.yd_position_avg_price << std::endl;
        }
    }
}
