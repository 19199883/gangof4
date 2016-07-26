#include "sgit_trade_interface.h"

#include <dirent.h>
#include <stdlib.h>
#include <iomanip>
#include <algorithm>

#include "sgit_struct_convert.h"
#include "check_schedule.h"
#include "sgit_data_formater.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "qtm_with_code.h"

using namespace std;

void SgitTradeSpi::ReportErrorState(int api_error_no, const std::string &error_msg)
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

SgitTradeSpi::SgitTradeSpi(const TunnelConfigData &cfg)
    : cfg_(cfg)
{
    connected_ = false;
    logoned_ = false;
    memset(trade_day, 0, sizeof(trade_day));

    in_init_state_ = true;
    memset(trade_day, 0, sizeof(trade_day));

    max_order_ref_ = 0;

    trade_context_.InitOrderRef(cfg_.App_cfg().orderref_prefix_id);
    tunnel_initializer_ = NULL;

    // 从配置解析参数
    ParseConfig();

    // qtm init
    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "sgit_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

    //Create Directory
    char cur_path[256];
    char full_path[256];
    getcwd(cur_path, sizeof(cur_path));
    sprintf(full_path, "%s/flow_dir_%s_%02d/", cur_path, user_.c_str(), cfg_.App_cfg().orderref_prefix_id);

    // check whether dir exist
    if (opendir(full_path) == NULL)
    {
        mkdir(full_path, 0755);
    }

    // create
    api_ = CSgitFtdcTraderApi::CreateFtdcTraderApi(full_path);
    api_->RegisterSpi(this);

    // subscribe
    api_->SubscribePrivateTopic(Sgit_TERT_RESTART);
    api_->SubscribePublicTopic(Sgit_TERT_QUICK);

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
    api_->Init(false);
}

SgitTradeSpi::~SgitTradeSpi(void)
{
    if (api_)
    {
        api_->RegisterSpi(NULL);
        api_->Release();
        api_ = NULL;
    }
}

void SgitTradeSpi::ParseConfig()
{
    // 用户密码
    user_ = cfg_.Logon_config().clientid;
    pswd_ = cfg_.Logon_config().password;
    tunnel_info_.account = cfg_.Logon_config().investorid;
    user_id_len_ = user_.length();

    exchange_code_ = cfg_.Logon_config().exch_code[0];
}

bool SgitTradeSpi::IsMineReturn(const char *user_id)
{
    return memcmp(user_id, user_.c_str(), user_id_len_) == 0;
}

void SgitTradeSpi::OnFrontConnected()
{
    TNL_LOG_WARN("OnFrontConnected.");
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::CONNECT_SUCCESS);
    connected_ = true;

    CSgitFtdcReqUserLoginField login_data;
    memset(&login_data, 0, sizeof(CSgitFtdcReqUserLoginField));
    strncpy(login_data.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TSgitFtdcBrokerIDType));
    strncpy(login_data.UserID, user_.c_str(), sizeof(TSgitFtdcUserIDType));
    strncpy(login_data.Password, pswd_.c_str(), sizeof(TSgitFtdcPasswordType));

    // 成功登录后，断开不再重新登录
    if (in_init_state_)
    {
        api_->ReqUserLogin(&login_data, 0);
    }
}

void SgitTradeSpi::ReqLogin()
{
    // wait
    sleep(3);

    CSgitFtdcReqUserLoginField login_data;
    memset(&login_data, 0, sizeof(CSgitFtdcReqUserLoginField));
    strncpy(login_data.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TSgitFtdcBrokerIDType));
    strncpy(login_data.UserID, user_.c_str(), sizeof(TSgitFtdcUserIDType));
    strncpy(login_data.Password, pswd_.c_str(), sizeof(TSgitFtdcPasswordType));

    api_->ReqUserLogin(&login_data, 0);
}

void SgitTradeSpi::OnFrontDisconnected(int nReason)
{
    connected_ = false;

    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
    TNL_LOG_WARN("OnFrontDisconnected, nReason: %d", nReason);
}

void SgitTradeSpi::OnHeartBeatWarning(int nTimeLapse)
{
    TNL_LOG_WARN("OnHeartBeatWarning, nTimeLapse: %d", nTimeLapse);
}

void SgitTradeSpi::OnRspUserLogin(CSgitFtdcRspUserLoginField *pf, CSgitFtdcRspInfoField *rsp, int req_id,
bool is_last)
{
    TNL_LOG_INFO("OnRspUserLogin: requestid = %d, last_flag=%d %s%s",
        req_id, is_last,
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());

    // logon success
    if (rsp == NULL || rsp->ErrorID == 0)
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_SUCCESS);
        // set session parameters
        max_order_ref_ = atoi(pf->MaxOrderRef);
        if (max_order_ref_ < 1000)
        {
            max_order_ref_ = 1000;
        }

        // 设置到交易环境变量中
        trade_context_.SetOrderRef(max_order_ref_);

        const char *td = api_->GetTradingDay();
        strncpy(trade_day, td, sizeof(trade_day));

        tunnel_initializer_ = new TunnelInitializer(cfg_, api_);
        logoned_ = true;
        in_init_state_ = false;
        api_->Ready();
        tunnel_initializer_->StartInit();

        if (TunnelIsReady())
        {
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
        }
    }
    else
    {
        ReportErrorState(rsp->ErrorID, rsp->ErrorMsg);
        int standard_error_no = cfg_.GetStandardErrorNo(rsp->ErrorID);
        TNL_LOG_WARN("OnRspUserLogin, external errorid: %d; Internal errorid: %d", rsp->ErrorID, standard_error_no);

        // 重新登陆
        std::thread req_login(&SgitTradeSpi::ReqLogin, this);
        req_login.detach();
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_FAIL);
    }
}

void SgitTradeSpi::OnRspUserLogout(CSgitFtdcUserLogoutField *pf, CSgitFtdcRspInfoField *rsp, int req_id,
bool is_last)
{
    try
    {
        ReportErrorState(rsp->ErrorID, rsp->ErrorMsg);
        logoned_ = false;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_OUT);

        TNL_LOG_WARN("OnRspUserLogout: requestid = %d, last_flag=%d %s%s",
            req_id, is_last,
            DatatypeFormater::ToString(pf).c_str(),
            DatatypeFormater::ToString(rsp).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspUserLogout.");
    }
}

void SgitTradeSpi::OnRspSettlementInfoConfirm(CSgitFtdcSettlementInfoConfirmField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    try
    {
        TNL_LOG_DEBUG("OnRspSettlementInfoConfirm: requestid = %d, last_flag=%d %s%s",
            req_id, is_last,
            DatatypeFormater::ToString(pf).c_str(),
            DatatypeFormater::ToString(rsp).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspSettlementInfoConfirm.");
    }
}

void SgitTradeSpi::OnRspQrySettlementInfo(CSgitFtdcSettlementInfoField *pSettlementInfo, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    try
    {
        TNL_LOG_DEBUG("OnRspQrySettlementInfo: requestid = %d, last_flag=%d %s%s",
            req_id, is_last,
            DatatypeFormater::ToString(pSettlementInfo).c_str(),
            DatatypeFormater::ToString(rsp).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspQrySettlementInfo.");
    }
}

void SgitTradeSpi::OnRspQrySettlementInfoConfirm(CSgitFtdcSettlementInfoConfirmField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    try
    {
        TNL_LOG_DEBUG("OnRspQrySettlementInfoConfirm: requestid = %d, last_flag=%d %s%s",
            req_id, is_last,
            DatatypeFormater::ToString(pf).c_str(),
            DatatypeFormater::ToString(rsp).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspQrySettlementInfoConfirm.");
    }
}

void SgitTradeSpi::OnRspError(CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    try
    {
        TNL_LOG_WARN("OnRspError: requestid = %d, last_flag=%d \n%s",
            req_id, is_last, DatatypeFormater::ToString(rsp).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspError.");
    }
}

void SgitTradeSpi::OnRtnOrder(CSgitFtdcOrderField *pf, CSgitFtdcRspInfoField *rsp)
{
    // TODO output return message for test
    TNL_LOG_DEBUG("OnRtnOrder - %s\n%s", DatatypeFormater::ToString(pf).c_str(), DatatypeFormater::ToString(rsp).c_str());

    try
    {
        if (!TunnelIsReady())
        {
            if (tunnel_initializer_) tunnel_initializer_->RecvRtnOrder(pf, rsp);

            TNL_LOG_INFO("OnRtnOrder when tunnel not ready.");
            return;
        }

        if (!pf)
        {
            TNL_LOG_WARN("OnRtnOrder, pOrder is null.");
            return;
        }

        if (!IsMineReturn(pf->UserID))
        {
            TNL_LOG_INFO("OnRtnOrder, not mine order, UserID:%s.", pf->UserID);
            return;
        }

        int standard_error_no = 0;
        int api_err_no = 0;

        if (rsp)
        {
            ReportErrorState(rsp->ErrorID, rsp->ErrorMsg);
            api_err_no = rsp->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
        }
        TNL_LOG_INFO("OnRtnOrder, api_err: %d; my_err: %d; OrderRef: %s", api_err_no, standard_error_no, pf->OrderRef);

        OrderRefDataType order_ref = atoll(pf->OrderRef);
        const SgitOrderInfo *p = trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            if (p->IsTerminated())
            {
                TNL_LOG_WARN("OnRtnOrder - after terminated, OrderRef: %s", pf->OrderRef);
                return;
            }

            EntrustNoType e_n = 0;
            if (pf->OrderSysID[0] != '\0')
            {
                e_n = atoll(pf->OrderSysID);
                memcpy(p->sys_order_id, pf->OrderSysID, sizeof(p->sys_order_id));
            }

            T_OrderRespond order_rsp;
            StructConvertor::OrderRespond(standard_error_no, p->po.serial_no, e_n, order_rsp);
            p->entrust_status = order_rsp.entrust_status;

            if (standard_error_no != 0)
            {
                // 报单失败，报告合规检查
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    order_ref,
                    exchange_code_,
                    p->po.stock_code,
                    p->po.volume,
                    p->po.speculator,
                    p->po.open_close,
                    p->po.order_type);
            }

            // 应答
            if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&order_rsp);
            LogUtil::OnOrderRespond(&order_rsp, tunnel_info_);

            if (standard_error_no == TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                // 委托回报
                T_OrderReturn order_return;
                StructConvertor::OrderReturn(pf, p, order_return);
                order_return.entrust_status = p->entrust_status;
                if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
                LogUtil::OnOrderReturn(&order_return, tunnel_info_);
            }
        }
        else
        {
            TNL_LOG_INFO("OnRtnOrder - can't get original place order info of order ref: %s", pf->OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRtnOrder - unknown exception.");
    }
}

void SgitTradeSpi::OnRtnTrade(CSgitFtdcTradeField *pf)
{
    // TODO output return message for test
    TNL_LOG_DEBUG("OnRtnTrade - \n%s", DatatypeFormater::ToString(pf).c_str());

    try
    {
        if (!TunnelIsReady())
        {
            if (tunnel_initializer_) tunnel_initializer_->RecvRtnTrade(pf);

            TNL_LOG_INFO("OnRtnTrade when tunnel not ready.");
            return;
        }
        if (!pf)
        {
            TNL_LOG_WARN("OnRtnTrade, pTrade is null.");
            return;
        }

        if (!IsMineReturn(pf->UserID))
        {
            TNL_LOG_INFO("OnRtnTrade, not mine order, UserID:%s.", pf->UserID);
            return;
        }

        OrderRefDataType order_ref = atoll(pf->OrderLocalID);
        const SgitOrderInfo *p = trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            p->entrust_status = MY_TNL_OS_PARTIALCOM;
            p->volume_remain -= pf->Volume;

            if (0 == p->volume_remain)
            {
                p->entrust_status = MY_TNL_OS_COMPLETED;
                //全成，报告合规检查
                ComplianceCheck_OnOrderFilled(
                    tunnel_info_.account.c_str(),
                    order_ref);
            }

            // 委托回报
            T_OrderReturn order_return;
            StructConvertor::OrderReturn(pf, p, order_return);
            order_return.entrust_status = p->entrust_status;
            if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
            LogUtil::OnOrderReturn(&order_return, tunnel_info_);

            // 成交
            T_TradeReturn trade_return;
            StructConvertor::TradeReturn(pf, p, trade_return);
            if (TradeReturn_call_back_handler_) TradeReturn_call_back_handler_(&trade_return);
            LogUtil::OnTradeReturn(&trade_return, tunnel_info_);
        }
        else
        {
            TNL_LOG_INFO("OnRtnTrade can't get original place order info of order ref: %s", pf->OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRtnTrade - unknown exception.");
    }
}

// 报单拒绝
void SgitTradeSpi::OnRspOrderInsert(CSgitFtdcInputOrderField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        if (!pf)
        {
            TNL_LOG_WARN("OnRspOrderInsert, pInputOrder is null.");
            return;
        }
        if (rsp)
        {
            ReportErrorState(rsp->ErrorID, rsp->ErrorMsg);
            api_err_no = rsp->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
        }
        TNL_LOG_WARN("OnRspOrderInsert, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            api_err_no, standard_error_no, pf->OrderRef);

        OrderRefDataType order_ref = atoll(pf->OrderRef);
        const SgitOrderInfo *p = trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            T_OrderRespond pOrderRespond;
            StructConvertor::OrderRespond(standard_error_no, p->po.serial_no, 0l, pOrderRespond);

            if (standard_error_no != 0)
            {
                // 报单失败，报告合规检查
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    order_ref,
                    exchange_code_,
                    p->po.stock_code,
                    pf->VolumeTotalOriginal,
                    p->po.speculator,
                    p->po.open_close,
                    p->po.order_type);
            }

            // 应答
            if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&pOrderRespond);
            LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
        }
        else
        {
            TNL_LOG_INFO("OnRspOrderInsert - can't get original place order info of order ref: %s", pf->OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRspOrderInsert - unknown exception.");
    }

    TNL_LOG_DEBUG("OnRspOrderInsert: requestid = %d, last_flag=%d %s%s",
        req_id, is_last,
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());
}

void SgitTradeSpi::OnErrRtnOrderInsert(CSgitFtdcInputOrderField *pf, CSgitFtdcRspInfoField *rsp)
{
    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        if (!pf)
        {
            TNL_LOG_WARN("OnErrRtnOrderInsert, pInputOrder is null.");
            return;
        }
        if (rsp)
        {
            ReportErrorState(rsp->ErrorID, rsp->ErrorMsg);
            api_err_no = rsp->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
        }
        TNL_LOG_WARN("OnErrRtnOrderInsert, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            api_err_no, standard_error_no, pf->OrderRef);

        OrderRefDataType order_ref = atoll(pf->OrderRef);
        const SgitOrderInfo *p = trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            T_OrderRespond order_rsp;
            StructConvertor::OrderRespond(standard_error_no, p->po.serial_no, 0l, order_rsp);

            if (standard_error_no != 0)
            {
                // 报单失败，报告合规检查
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    order_ref,
                    exchange_code_,
                    p->po.stock_code,
                    pf->VolumeTotalOriginal,
                    p->po.speculator,
                    p->po.open_close,
                    p->po.order_type);
            }

            // 应答
            if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&order_rsp);
            LogUtil::OnOrderRespond(&order_rsp, tunnel_info_);
        }
        else
        {
            TNL_LOG_INFO("OnErrRtnOrderInsert - can't get original place order info of order ref: %s", pf->OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnErrRtnOrderInsert - unknown exception.");
    }

    TNL_LOG_DEBUG("OnErrRtnOrderInsert:  %s%s",
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());
}

// 撤单拒绝
void SgitTradeSpi::OnRspOrderAction(CSgitFtdcInputOrderActionField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    // TODO output respond message when test
    TNL_LOG_DEBUG("OnRspOrderAction: requestid = %d, last_flag=%d %s%s",
        req_id, is_last,
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());

    try
    {
        if (!TunnelIsReady())
        {
            if (tunnel_initializer_) tunnel_initializer_->RecvRspOrderAction(pf, rsp);

            TNL_LOG_INFO("OnRspOrderAction when cancel unterminated orders.");
            return;
        }
        int standard_error_no = 0;
        int api_err_no = 0;
        if (!pf)
        {
            TNL_LOG_WARN("OnRspOrderAction, pInputOrderAction is null.");
            return;
        }
        if (rsp)
        {
            ReportErrorState(rsp->ErrorID, rsp->ErrorMsg);
            api_err_no = rsp->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
        }

        TNL_LOG_WARN("OnRspOrderAction, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            api_err_no, standard_error_no, pf->OrderRef);

        OrderRefDataType order_ref = atoll(pf->OrderRef);
        const SgitOrderInfo *p = trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            SerialNoType cancel_sn = 0;
            if (trade_context_.PopCancelSnOfOrder(order_ref, cancel_sn))
            {
                T_CancelRespond cancel_respond;
                StructConvertor::CancelRespond(standard_error_no, cancel_sn, atoll(p->sys_order_id), cancel_respond);

                // 应答
                if (CancelRespond_call_back_handler_) CancelRespond_call_back_handler_(&cancel_respond);
                LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("OnRspOrderAction - cancel order: %ld from outside.", p->po.serial_no);
            }

            if (api_err_no != 0)
            {
                // 撤单拒绝，报告合规检查
                ComplianceCheck_OnOrderCancelFailed(
                    tunnel_info_.account.c_str(),
                    pf->InstrumentID,
                    order_ref);
            }
            else
            {
                ComplianceCheck_OnOrderCanceled(
                    tunnel_info_.account.c_str(),
                    p->po.stock_code,
                    order_ref,
                    exchange_code_,
                    p->volume_remain,
                    p->po.speculator,
                    p->po.open_close);

                // 委托回报
                T_OrderReturn order_return;
                memset(&order_return, 0, sizeof(order_return));
                order_return.entrust_no = atoll(p->sys_order_id);
                order_return.serial_no = p->po.serial_no;
                strncpy(order_return.stock_code, p->po.stock_code, sizeof(order_return.stock_code));
                order_return.direction = p->po.direction;
                order_return.open_close = p->po.open_close;
                order_return.speculator = p->po.speculator;
                order_return.volume = p->po.volume;
                order_return.limit_price = p->po.limit_price;

                order_return.volume_remain = p->volume_remain;
                order_return.entrust_status = MY_TNL_OS_WITHDRAWED;

                if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
                LogUtil::OnOrderReturn(&order_return, tunnel_info_);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRspOrderAction - unknown exception.");
    }
}

void SgitTradeSpi::OnErrRtnOrderAction(CSgitFtdcOrderActionField *pf, CSgitFtdcRspInfoField *rsp)
{
    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        if (!pf)
        {
            TNL_LOG_WARN("OnErrRtnOrderAction, pOrderAction is null.");
            return;
        }
        if (rsp)
        {
            ReportErrorState(rsp->ErrorID, rsp->ErrorMsg);
            api_err_no = rsp->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
        }

        TNL_LOG_WARN("OnErrRtnOrderAction, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            api_err_no, standard_error_no, pf->OrderRef);

        OrderRefDataType order_ref = atoll(pf->OrderRef);
        const SgitOrderInfo *p = trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            SerialNoType cancel_sn = 0;
            if (trade_context_.PopCancelSnOfOrder(order_ref, cancel_sn))
            {
                T_CancelRespond cancel_respond;
                StructConvertor::CancelRespond(standard_error_no, cancel_sn, atoll(p->sys_order_id), cancel_respond);

                // 应答
                if (CancelRespond_call_back_handler_) CancelRespond_call_back_handler_(&cancel_respond);
                LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("OnErrRtnOrderAction - cancel order: %ld from outside.", p->po.serial_no);
            }

            if (api_err_no != 0)
            {
                // 撤单拒绝，报告合规检查
                ComplianceCheck_OnOrderCancelFailed(
                    tunnel_info_.account.c_str(),
                    pf->InstrumentID,
                    order_ref);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspOrderAction.");
    }

    TNL_LOG_DEBUG("OnErrRtnOrderAction:  %s%s",
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());
}

void SgitTradeSpi::OnRspQryOrder(CSgitFtdcOrderField* pf, CSgitFtdcRspInfoField* rsp, int req_id, bool is_last)
{
    TNL_LOG_DEBUG("OnRspQryOrder: requestid = %d, last_flag=%d %s %s",
        req_id, is_last,
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());
}

void SgitTradeSpi::OnRspQryTrade(CSgitFtdcTradeField* pf, CSgitFtdcRspInfoField* rsp, int req_id, bool is_last)
{
    TNL_LOG_DEBUG("OnRspQryTrade: requestid = %d, last_flag=%d %s%s",
        req_id, is_last,
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());
}

///请求查询投资者持仓响应
void SgitTradeSpi::OnRspQryInvestorPosition(CSgitFtdcInvestorPositionField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    if (tunnel_initializer_)
    {
        tunnel_initializer_->RecvRspQryInvestorPosition(pf, rsp, req_id, is_last);
    }
    else
    {
        TNL_LOG_DEBUG("OnRspQryInvestorPosition: requestid = %d, last_flag=%d %s%s",
            req_id, is_last,
            DatatypeFormater::ToString(pf).c_str(),
            DatatypeFormater::ToString(rsp).c_str());
    }
}

void SgitTradeSpi::OnRspQryInvestorPositionDetail(CSgitFtdcInvestorPositionDetailField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    TNL_LOG_DEBUG("OnRspQryInvestorPositionDetail: requestid = %d, last_flag=%d %s%s",
        req_id, is_last,
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());
}

void SgitTradeSpi::OnRspQryInstrument(CSgitFtdcInstrumentField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    if (tunnel_initializer_)
    {
        tunnel_initializer_->RecvRspQryInstrument(pf, rsp, req_id, is_last);
    }
    else
    {
        TNL_LOG_DEBUG("OnRspQryInstrument: requestid = %d, last_flag=%d %s%s",
            req_id, is_last,
            DatatypeFormater::ToString(pf).c_str(),
            DatatypeFormater::ToString(rsp).c_str());
    }
}

void SgitTradeSpi::OnRspQryTradingAccount(CSgitFtdcTradingAccountField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last)
{
    if (tunnel_initializer_) tunnel_initializer_->RecvRspQryTradingAccount(pf, rsp, req_id, is_last);
}
