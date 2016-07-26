#include "lts_trade_interface.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <algorithm>

#include "my_structure_convert.h"
#include "check_schedule.h"
#include "lts_data_formater.h"
#include "lts_query_interface.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "qtm_with_code.h"

using namespace std;

inline static void TrimTailSpace(char p[], std::size_t len)
{
    for (std::size_t i = 0; i < len; ++i)
    {
        if (p[i] == ' ')
        {
            p[i] = '\0';
            break;
        }
    }
}

void LTSTradeInf::ReportErrorState(int api_error_no, const std::string &error_msg)
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

LTSTradeInf::LTSTradeInf(const TunnelConfigData &cfg)
    : cfg_(cfg)
{
    logoned_ = false;
    query_logoned_ = false;
    memset(trade_day, 0, sizeof(trade_day));

    in_init_state_ = true;
    available_fund = 0;

    tunnel_finish_init_ = false;
    query_finish_init_ = false;
    trade_finish_init_ = false;

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

    time_to_query_pos_int = 0;
    query_position_step = 0;
    if (!cfg_.Initial_policy_param().time_to_query_pos.empty())
    {
        time_to_query_pos_int = atoi(cfg_.Initial_policy_param().time_to_query_pos.c_str());
    }

    if (time_to_query_pos_int > 0)
    {
        std::thread thread_tmp(&LTSTradeInf::RecordPositionAtSpecTime, this);
        thread_tmp.detach();
    }

    lts_trade_context_.InitOrderRef(cfg_.App_cfg().orderref_prefix_id);

    // 从配置解析参数
    ParseConfig();

    char qtm_tmp_name[32];
    sprintf(qtm_tmp_name, "lts_%s_%d_%u", tunnel_info_.account.c_str(), cfg_.App_cfg().orderref_prefix_id, getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

    InitQueryInterface();
    InitTradeInterface();
}

void LTSTradeInf::InitQueryInterface()
{
    //Create Directory
    char cur_path[256];
    char full_path[256];
    getcwd(cur_path, sizeof(cur_path));
    sprintf(full_path, "%s/flow_query_%s_%02d/", cur_path, tunnel_info_.account.c_str(), cfg_.App_cfg().orderref_prefix_id);

    // check whether dir exist
    if (opendir(full_path) == NULL)
    {
        mkdir(full_path, 0755);
    }

    // create
    query_api_ = CSecurityFtdcQueryApi::CreateFtdcQueryApi(full_path);
    if (query_api_ == NULL)
    {
        TNL_LOG_ERROR("Query CreateFtdcQueryApi failed");
        return;
    }
    query_inf_ = new LTSQueryInf(this);
    query_api_->RegisterSpi(query_inf_);

    // set front address
    const std::string &query_front = cfg_.Logon_config().quote_front_addr;
    char *addr_tmp = new char[query_front.size() + 1];
    memcpy(addr_tmp, query_front.c_str(), query_front.size() + 1);
    query_api_->RegisterFront(addr_tmp);
    TNL_LOG_INFO("RegisterQueryFront, addr: %s", addr_tmp);
    delete[] addr_tmp;

    // init
    query_api_->Init();
}
void LTSTradeInf::InitTradeInterface()
{
    //Create Directory
    char cur_path[256];
    char full_path[256];
    getcwd(cur_path, sizeof(cur_path));
    sprintf(full_path, "%s/flow_trade_%s_%02d/", cur_path, tunnel_info_.account.c_str(), cfg_.App_cfg().orderref_prefix_id);

    // check whether dir exist
    if (opendir(full_path) == NULL)
    {
        mkdir(full_path, 0755);
    }

    // create
    api_ = CSecurityFtdcTraderApi::CreateFtdcTraderApi(full_path);
    if (api_ == NULL)
    {
        TNL_LOG_ERROR("Trade CreateFtdcQueryApi failed");
        return;
    }
    api_->RegisterSpi(this);

    // subscribe
    api_->SubscribePrivateTopic(SECURITY_TERT_QUICK);
    api_->SubscribePublicTopic(SECURITY_TERT_QUICK);

    // set front address
    for (const std::string &v : cfg_.Logon_config().front_addrs)
    {
        char *addr_tmp = new char[v.size() + 1];
        memcpy(addr_tmp, v.c_str(), v.size() + 1);
        api_->RegisterFront(addr_tmp);
        TNL_LOG_INFO("RegisterTradeFront, addr: %s", addr_tmp);
        delete[] addr_tmp;
    }

    // init
    api_->Init();
}

LTSTradeInf::~LTSTradeInf(void)
{
    if (query_api_)
    {
        query_api_->RegisterSpi(NULL);
        if (query_inf_) delete query_inf_;
        query_api_->Release();
    }
    if (api_)
    {
        api_->RegisterSpi(NULL);
        api_->Release();
        api_ = NULL;
    }
}

void LTSTradeInf::ParseConfig()
{
    // 用户密码
    tunnel_info_.account = cfg_.Logon_config().investorid;
    pswd_ = cfg_.Logon_config().password;
}

void LTSTradeInf::OnFrontConnected()
{
    TNL_LOG_WARN("OnFrontConnected.");

    CSecurityFtdcAuthRandCodeField pAuthRandCode;
    memset(&pAuthRandCode, 0, sizeof(pAuthRandCode));

    // 成功登录后，断开不再重新登录
    if (in_init_state_)
    {
        int ret = api_->ReqFetchAuthRandCode(&pAuthRandCode, 1);
        TNL_LOG_INFO("call Trade ReqFetchAuthRandCode, return:%d", ret);
    }
}

void LTSTradeInf::OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
    TNL_LOG_INFO("OnRspFetchAuthRandCode: requestid = %d, last_flag=%d \n%s \n%s",
        req_id, last,
        LTSDatatypeFormater::ToString(pf).c_str(),
        LTSDatatypeFormater::ToString(rsp).c_str());

    if (rsp && rsp->ErrorID != 0)
    {
        return;
    }
    memcpy(trade_authcode_, pf->RandCode, sizeof(trade_authcode_));

    CSecurityFtdcReqUserLoginField login_data;
    memset(&login_data, 0, sizeof(CSecurityFtdcReqUserLoginField));
    strncpy(login_data.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TSecurityFtdcBrokerIDType));
    strncpy(login_data.UserID, tunnel_info_.account.c_str(), sizeof(TSecurityFtdcUserIDType));
    strncpy(login_data.Password, pswd_.c_str(), sizeof(TSecurityFtdcPasswordType));
    strncpy(login_data.RandCode, trade_authcode_, sizeof(login_data.RandCode));
    strncpy(login_data.UserProductInfo, cfg_.Logon_config().UserProductInfo.c_str(), sizeof(login_data.UserProductInfo));
    strncpy(login_data.AuthCode, cfg_.Logon_config().AuthCode.c_str(), sizeof(login_data.AuthCode));

    // 成功登录后，断开不再重新登录
    if (in_init_state_)
    {
        int ret = api_->ReqUserLogin(&login_data, 0);
        TNL_LOG_INFO("call trade ReqUserLogin, return:%d", ret);
    }
}

void LTSTradeInf::ReqLogin()
{
    // wait
    sleep(30);

    CSecurityFtdcReqUserLoginField login_data;
    memset(&login_data, 0, sizeof(CSecurityFtdcReqUserLoginField));
    strncpy(login_data.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TSecurityFtdcBrokerIDType));
    strncpy(login_data.UserID, tunnel_info_.account.c_str(), sizeof(TSecurityFtdcUserIDType));
    strncpy(login_data.Password, pswd_.c_str(), sizeof(TSecurityFtdcPasswordType));
    strncpy(login_data.RandCode, trade_authcode_, sizeof(login_data.RandCode));
    strncpy(login_data.UserProductInfo, cfg_.Logon_config().UserProductInfo.c_str(), sizeof(login_data.UserProductInfo));
    strncpy(login_data.AuthCode, cfg_.Logon_config().AuthCode.c_str(), sizeof(login_data.AuthCode));

    int ret = api_->ReqUserLogin(&login_data, 0);
    TNL_LOG_INFO("call trade ReqUserLogin, return:%d", ret);
}

void LTSTradeInf::QueryReqLogin()
{
    // wait
    sleep(30);

    CSecurityFtdcReqUserLoginField login_data;
    memset(&login_data, 0, sizeof(CSecurityFtdcReqUserLoginField));
    strncpy(login_data.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TSecurityFtdcBrokerIDType));
    strncpy(login_data.UserID, tunnel_info_.account.c_str(), sizeof(TSecurityFtdcUserIDType));
    strncpy(login_data.Password, pswd_.c_str(), sizeof(TSecurityFtdcPasswordType));
    strncpy(login_data.RandCode, query_authcode_, sizeof(login_data.RandCode));
    strncpy(login_data.UserProductInfo, cfg_.Logon_config().UserProductInfo.c_str(), sizeof(login_data.UserProductInfo));
    strncpy(login_data.AuthCode, cfg_.Logon_config().AuthCode.c_str(), sizeof(login_data.AuthCode));

    int ret = query_api_->ReqUserLogin(&login_data, 0);
    TNL_LOG_INFO("call query ReqUserLogin, return:%d", ret);
}

void LTSTradeInf::OnFrontDisconnected(int reason)
{
    logoned_ = false;
    tunnel_finish_init_ = false;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
    TNL_LOG_WARN("OnFrontDisconnected, nReason: %d", reason);
}

void LTSTradeInf::OnHeartBeatWarning(int time_lapse)
{
    TNL_LOG_WARN("OnHeartBeatWarning, nTimeLapse: %d", time_lapse);
}

void LTSTradeInf::OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TNL_LOG_INFO("OnRspUserLogin: requestid = %d, last_flag=%d \n%s \n%s",
        nRequestID, bIsLast,
        LTSDatatypeFormater::ToString(pRspUserLogin).c_str(),
        LTSDatatypeFormater::ToString(pRspInfo).c_str());

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
        lts_trade_context_.SetOrderRef(max_order_ref_);

        logoned_ = true;
        in_init_state_ = false;
        SetTradeFinished();
    }
    else
    {
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        int standard_error_no = cfg_.GetStandardErrorNo(pRspInfo->ErrorID);
        TNL_LOG_WARN("OnRspUserLogin, external errorid: %d; Internal errorid: %d", pRspInfo->ErrorID, standard_error_no);

        // 重新登陆
        std::thread req_login(&LTSTradeInf::ReqLogin, this);
        req_login.detach();
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_FAIL);
    }
}

void LTSTradeInf::OnQueryFrontConnected()
{
    TNL_LOG_WARN("OnQueryFrontConnected.");

    CSecurityFtdcAuthRandCodeField pAuthRandCode;
    memset(&pAuthRandCode, 0, sizeof(pAuthRandCode));

    int ret = query_api_->ReqFetchAuthRandCode(&pAuthRandCode, 1);
    TNL_LOG_INFO("call Query ReqFetchAuthRandCode, return:%d", ret);
}

void LTSTradeInf::OnQueryRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
    TNL_LOG_INFO("OnQueryRspFetchAuthRandCode: requestid = %d, last_flag=%d \n%s \n%s",
        req_id, last,
        LTSDatatypeFormater::ToString(pf).c_str(),
        LTSDatatypeFormater::ToString(rsp).c_str());

    if (rsp && rsp->ErrorID != 0)
    {
        ReportErrorState(rsp->ErrorID, rsp->ErrorMsg);
        return;
    }
    CSecurityFtdcReqUserLoginField login_data;
    memcpy(query_authcode_, pf->RandCode, sizeof(trade_authcode_));
    memset(&login_data, 0, sizeof(CSecurityFtdcReqUserLoginField));
    strncpy(login_data.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TSecurityFtdcBrokerIDType));
    strncpy(login_data.UserID, tunnel_info_.account.c_str(), sizeof(TSecurityFtdcUserIDType));
    strncpy(login_data.Password, pswd_.c_str(), sizeof(TSecurityFtdcPasswordType));
    strncpy(login_data.RandCode, query_authcode_, sizeof(login_data.RandCode));
    strncpy(login_data.UserProductInfo, cfg_.Logon_config().UserProductInfo.c_str(), sizeof(login_data.UserProductInfo));
    strncpy(login_data.AuthCode, cfg_.Logon_config().AuthCode.c_str(), sizeof(login_data.AuthCode));

    int ret = query_api_->ReqUserLogin(&login_data, 0);
    TNL_LOG_INFO("call Query ReqUserLogin, return:%d", ret);
}

void LTSTradeInf::OnQueryFrontDisconnected(int reason)
{
    query_logoned_ = false;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
    TNL_LOG_WARN("OnQueryFrontDisconnected, nReason: %d", reason);
}

void LTSTradeInf::OnQueryHeartBeatWarning(int time_lapse)
{
    TNL_LOG_WARN("OnQueryHeartBeatWarning, nTimeLapse: %d", time_lapse);
}

void LTSTradeInf::OnQueryRspError(CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
    try
    {
        TNL_LOG_WARN("OnQueryRspError: requestid = %d, last_flag=%d \n%s",
            req_id, last, LTSDatatypeFormater::ToString(rsp).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnQueryRspError.");
    }
}

void LTSTradeInf::OnQueryRspUserLogin(CSecurityFtdcRspUserLoginField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
    TNL_LOG_INFO("OnQueryRspUserLogin: requestid = %d, last_flag=%d \n%s \n%s",
        req_id, last,
        LTSDatatypeFormater::ToString(pf).c_str(),
        LTSDatatypeFormater::ToString(rsp).c_str());

    // logon success
    if (rsp == NULL || rsp->ErrorID == 0)
    {
        query_logoned_ = true;
        strncpy(trade_day, pf->TradingDay, sizeof(trade_day));

        std::thread t_qry_account(&LTSTradeInf::QueryAccount, this);
        t_qry_account.detach();
    }
    else
    {
        ReportErrorState(rsp->ErrorID, rsp->ErrorMsg);
        int standard_error_no = cfg_.GetStandardErrorNo(rsp->ErrorID);
        TNL_LOG_WARN("OnQueryRspUserLogin, external errorid: %d; Internal errorid: %d", rsp->ErrorID, standard_error_no);

        // 重新登陆
        std::thread req_login(&LTSTradeInf::QueryReqLogin, this);
        req_login.detach();
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_FAIL);
    }
}

void LTSTradeInf::OnQueryRspUserLogout(CSecurityFtdcUserLogoutField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
    try
    {
        query_logoned_ = false;

        TNL_LOG_WARN("OnQueryRspUserLogout: requestid = %d, last_flag=%d \n%s \n%s",
            req_id, last,
            LTSDatatypeFormater::ToString(pf).c_str(),
            LTSDatatypeFormater::ToString(rsp).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnQueryRspUserLogout.");
    }
}

void LTSTradeInf::QueryAccount()
{
    // query account
    CSecurityFtdcQryTradingAccountField qry_account;
    memset(&qry_account, 0, sizeof(qry_account));
    strncpy(qry_account.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TSecurityFtdcBrokerIDType));
    strncpy(qry_account.InvestorID, tunnel_info_.account.c_str(), sizeof(TSecurityFtdcInvestorIDType));

    available_fund = 0;
    while (true)
    {
        int ret = query_api_->ReqQryTradingAccount(&qry_account, 0);
        TNL_LOG_INFO("ReqQryTradingAccount, return: %d", ret);

        if (ret == 0)
        {
            break;
        }
        sleep(1);
    }
}

void LTSTradeInf::QueryContractInfo()
{
    // query instrument
    CSecurityFtdcQryInstrumentField qry_param;
    memset(&qry_param, 0, sizeof(qry_param));
//    memcpy(qry_param.InstrumentID, "", strlen(""));
//    memcpy(qry_param.ExchangeID, "", strlen(""));
//    memcpy(qry_param.ExchangeInstID, "", strlen(""));
//    memcpy(qry_param.ProductID, "", strlen(""));

    while (true)
    {
        int ret = query_api_->ReqQryInstrument(&qry_param, 0);
        TNL_LOG_INFO("ReqQryInstrument, return: %d", ret);

        if (ret == 0)
        {
            break;
        }
        sleep(1);
    }
}

void LTSTradeInf::OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    try
    {
        logoned_ = false;
        tunnel_finish_init_ = false;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_OUT);

        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        TNL_LOG_WARN("OnRspUserLogout: requestid = %d, last_flag=%d \n%s \n%s",
            nRequestID, bIsLast,
            LTSDatatypeFormater::ToString(pUserLogout).c_str(),
            LTSDatatypeFormater::ToString(pRspInfo).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspUserLogout.");
    }
}

void LTSTradeInf::OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    try
    {
        TNL_LOG_WARN("OnRspError: requestid = %d, last_flag=%d \n%s",
            nRequestID, bIsLast, LTSDatatypeFormater::ToString(pRspInfo).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspError.");
    }
}

void LTSTradeInf::OnRtnOrder(CSecurityFtdcOrderField *pOrder)
{
    try
    {
        bool no_need_handle = false;
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("OnRtnOrder when tunnel not ready.");
            no_need_handle = true;
        }
        if (!pOrder)
        {
            TNL_LOG_WARN("OnRtnOrder, pOrder is null.");
            no_need_handle = true;
        }

        // check if order is placed by this session
        if (pOrder->FrontID != front_id_ || pOrder->SessionID != session_id_)
        {
            TNL_LOG_INFO("OnRtnOrder - order not of this session, FrontID: %d;SessionID: %d;OrderRef: %s",
                pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef);
            no_need_handle = true;
        }

        if (no_need_handle)
        {
            return;
        }

        OrderRefDataType order_ref = atoll(pOrder->OrderRef);
        const OriginalReqInfo *p = lts_trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            // Save Order SysID
            long order_sys_id = LTSFieldConvert::GetEntrustNo(pOrder->OrderSysID);
            bool update_sysid_flag = false;
            if (order_sys_id != 0)
            {
                update_sysid_flag = lts_trade_context_.SaveOrderSysIDOfSerialNo(p->serial_no, order_sys_id);
            }

            // 全成，报告合规检查
            if (pOrder->OrderStatus == SECURITY_FTDC_OST_AllTraded)
            {
                ComplianceCheck_OnOrderFilled(
                    tunnel_info_.account.c_str(),
                    order_ref);
            }

            // 已撤，报告合规检查
            if (pOrder->OrderStatus == SECURITY_FTDC_OST_Canceled)
            {
                ComplianceCheck_OnOrderCanceled(
                    tunnel_info_.account.c_str(),
                    p->stock_code.c_str(),
                    order_ref,
                    p->exchange_code,
                    pOrder->VolumeTotalOriginal - pOrder->VolumeTraded,
                    p->hedge_type,
                    p->open_close_flag);
            }

            if (pOrder->ActiveUserID[0] == '\0' && !lts_trade_context_.IsAnsweredPlaceOrder(order_ref))
            {
                T_OrderRespond pOrderRespond;
                LTSPacker::OrderRespond(0, p->serial_no, order_sys_id, pOrder->OrderSubmitStatus, pOrder->OrderStatus, pOrderRespond);
                if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&pOrderRespond);
                lts_trade_context_.SetAnsweredPlaceOrder(order_ref);
                LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
            }
            else if (pOrder->ActiveUserID[0] != '\0' && !lts_trade_context_.IsAnsweredCancelOrder(order_ref))
            {
                long cancel_serial_no = atol(p->Cancel_serial_no().c_str());
                if (cancel_serial_no > 0)
                {
                    T_CancelRespond cancel_respond;
                    LTSPacker::CancelRespond(0, cancel_serial_no, order_sys_id, cancel_respond);
                    if (CancelRespond_call_back_handler_) CancelRespond_call_back_handler_(&cancel_respond);
                    lts_trade_context_.SetAnsweredCancelOrder(order_ref);
                    LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);

                    // to avoid no canceld status report of the order (merge from 258 20160601)
                    if (pOrder->OrderStatus == SECURITY_FTDC_OST_Canceled)
                    {
                        T_OrderReturn order_return;
                        LTSPacker::OrderReturn(pOrder, p, order_return);
                        if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
                        LogUtil::OnOrderReturn(&order_return, tunnel_info_);
                    }
                }
                else
                {
                    T_OrderReturn order_return;
                    LTSPacker::OrderReturn(pOrder, p, order_return);

                    // fillup a reported order return if not have one before filled
                    if (update_sysid_flag &&
                        (order_return.entrust_status == MY_TNL_OS_COMPLETED
                            || order_return.entrust_status == MY_TNL_OS_PARTIALCOM
                            || order_return.entrust_status == MY_TNL_OS_ERROR
                            || order_return.entrust_status == MY_TNL_OS_WITHDRAWED))
                    {
                        TNL_LOG_INFO("OnRtnOrder - fillup a reported return in tunnel before going to patial filled/filled/error/canceled");
                        T_OrderReturn fillup_rtn(order_return);
                        fillup_rtn.entrust_status = MY_TNL_OS_REPORDED;
                        fillup_rtn.volume_remain = fillup_rtn.volume;

                        if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&fillup_rtn);
                        LogUtil::OnOrderReturn(&fillup_rtn, tunnel_info_);
                    }

                    if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
                    LogUtil::OnOrderReturn(&order_return, tunnel_info_);
                    TNL_LOG_WARN("cancel order: %ld from outside.", p->serial_no);
                }
            }
            else
            {
                T_OrderReturn order_return;
                LTSPacker::OrderReturn(pOrder, p, order_return);

                // fillup a reported order return if not have one before filled
                if (update_sysid_flag &&
                    (order_return.entrust_status == MY_TNL_OS_COMPLETED
                        || order_return.entrust_status == MY_TNL_OS_PARTIALCOM
                        || order_return.entrust_status == MY_TNL_OS_ERROR
                        || order_return.entrust_status == MY_TNL_OS_WITHDRAWED))
                {
                    TNL_LOG_INFO("OnRtnOrder - fillup a reported return in tunnel before going to patial filled/filled/error/canceled");
                    T_OrderReturn fillup_rtn(order_return);
                    fillup_rtn.entrust_status = MY_TNL_OS_REPORDED;
                    fillup_rtn.volume_remain = fillup_rtn.volume;

                    if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&fillup_rtn);
                    LogUtil::OnOrderReturn(&fillup_rtn, tunnel_info_);
                }

                if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
                LogUtil::OnOrderReturn(&order_return, tunnel_info_);
            }

            TNL_LOG_DEBUG(LTSDatatypeFormater::ToString(pOrder).c_str());
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

void LTSTradeInf::OnRtnTrade(CSecurityFtdcTradeField *pTrade)
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
        const OriginalReqInfo *p = lts_trade_context_.GetOrderInfoByOrderRef(order_ref);
        if (p)
        {
            long order_sys_id = LTSFieldConvert::GetEntrustNo(pTrade->OrderSysID);
            long original_sys_id = lts_trade_context_.GetOrderSysIDOfSerialNo(p->serial_no);
            // maybe not my own order, shouldn't happen, just record it
            if (order_sys_id != original_sys_id)
            {
                TNL_LOG_WARN("OnRtnTrade - order may not placed by this session, order_sys_id: %s", pTrade->OrderSysID);
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
                LTSPacker::TradeReturn(pTrade, p, trade_return);
                if (TradeReturn_call_back_handler_) TradeReturn_call_back_handler_(&trade_return);
                LogUtil::OnTradeReturn(&trade_return, tunnel_info_);
            }

            TNL_LOG_DEBUG(LTSDatatypeFormater::ToString(pTrade).c_str());
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
void LTSTradeInf::OnRspOrderInsert(CSecurityFtdcInputOrderField *pInputOrder, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    try
    {
        int standard_error_no = 0;
        int lts_err_no = 0;
        if (!pInputOrder)
        {
            TNL_LOG_WARN("OnRspOrderInsert, pInputOrder is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            lts_err_no = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(lts_err_no);
        }
        TNL_LOG_WARN("OnRspOrderInsert, lts_err_code: %d; my_err_code: %d; OrderRef: %s",
            lts_err_no, standard_error_no, pInputOrder->OrderRef);

        OrderRefDataType OrderRef = atoll(pInputOrder->OrderRef);
        const OriginalReqInfo *p = lts_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            if (lts_trade_context_.CheckAndSetHaveHandleInsertErr(p->serial_no))
            {
                T_OrderRespond pOrderRespond;
                LTSPacker::OrderRespond(standard_error_no, p->serial_no, 0, '4', '0', pOrderRespond);

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

                // 应答
                if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&pOrderRespond);
                LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("rsp err insert have handled, order ref: %s", pInputOrder->OrderRef);
            }

            TNL_LOG_DEBUG("OnRspOrderInsert: requestid = %d, last_flag=%d \n%s \n%s",
                nRequestID, bIsLast,
                LTSDatatypeFormater::ToString(pInputOrder).c_str(),
                LTSDatatypeFormater::ToString(pRspInfo).c_str());
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
void LTSTradeInf::OnRspOrderAction(CSecurityFtdcInputOrderActionField *pInputOrderAction, CSecurityFtdcRspInfoField *pRspInfo,
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
        int lts_err_no = 0;
        if (!pInputOrderAction)
        {
            TNL_LOG_WARN("OnRspOrderAction, pInputOrderAction is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            lts_err_no = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(lts_err_no);
        }

        TNL_LOG_WARN("OnRspOrderAction, lts_err_code: %d; my_err_code: %d; OrderRef: %s",
            lts_err_no, standard_error_no, pInputOrderAction->OrderRef);

        OrderRefDataType OrderRef = atoll(pInputOrderAction->OrderRef);
        const OriginalReqInfo *p = lts_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            long cancel_serial_no = atol(p->Cancel_serial_no().c_str());
            if (cancel_serial_no > 0)
            {
                if (lts_trade_context_.CheckAndSetHaveHandleActionErr(cancel_serial_no))
                {
                    T_CancelRespond cancel_respond;
                    LTSPacker::CancelRespond(standard_error_no, cancel_serial_no, 0L, cancel_respond);

                    // 应答
                    if (CancelRespond_call_back_handler_) CancelRespond_call_back_handler_(&cancel_respond);
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
                    LTSDatatypeFormater::ToString(pInputOrderAction).c_str(),
                    LTSDatatypeFormater::ToString(pRspInfo).c_str());
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

void LTSTradeInf::OnErrRtnOrderInsert(CSecurityFtdcInputOrderField *pInputOrder, CSecurityFtdcRspInfoField *pRspInfo)
{
    try
    {
        int standard_error_no = 0;
        int lts_err_no = 0;
        if (!pInputOrder)
        {
            TNL_LOG_WARN("OnErrRtnOrderInsert, pInputOrder is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            lts_err_no = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(lts_err_no);
        }
        TNL_LOG_WARN("OnErrRtnOrderInsert, lts_err_code: %d; my_err_code: %d; OrderRef: %s",
            lts_err_no, standard_error_no, pInputOrder->OrderRef);

        OrderRefDataType OrderRef = atoll(pInputOrder->OrderRef);
        const OriginalReqInfo *p = lts_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            if (lts_trade_context_.CheckAndSetHaveHandleInsertErr(p->serial_no))
            {
                T_OrderRespond pOrderRespond;
                LTSPacker::OrderRespond(standard_error_no, p->serial_no, 0, '4', '0', pOrderRespond);

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

                // 应答
                if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&pOrderRespond);
                LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("rtn err insert have handled, order ref: %s", pInputOrder->OrderRef);
            }

            TNL_LOG_DEBUG("OnErrRtnOrderInsert:  \n%s \n%s",
                LTSDatatypeFormater::ToString(pInputOrder).c_str(),
                LTSDatatypeFormater::ToString(pRspInfo).c_str());
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

void LTSTradeInf::OnErrRtnOrderAction(CSecurityFtdcOrderActionField *pOrderAction, CSecurityFtdcRspInfoField *pRspInfo)
{
    try
    {
        int standard_error_no = 0;
        int lts_err_no = 0;
        if (!pOrderAction)
        {
            TNL_LOG_WARN("OnErrRtnOrderAction, pOrderAction is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            lts_err_no = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(lts_err_no);
        }

        TNL_LOG_WARN("OnErrRtnOrderAction, lts_err_code: %d; my_err_code: %d; OrderRef: %s",
            lts_err_no, standard_error_no, pOrderAction->OrderRef);

        OrderRefDataType OrderRef = atoll(pOrderAction->OrderRef);
        const OriginalReqInfo *p = lts_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            long cancel_serial_no = atol(p->Cancel_serial_no().c_str());
            if (cancel_serial_no > 0)
            {
                if (lts_trade_context_.CheckAndSetHaveHandleActionErr(cancel_serial_no))
                {
                    T_CancelRespond cancel_respond;
                    LTSPacker::CancelRespond(standard_error_no, cancel_serial_no, 0L, cancel_respond);

                    // 应答
                    if (CancelRespond_call_back_handler_) CancelRespond_call_back_handler_(&cancel_respond);
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
                LTSDatatypeFormater::ToString(pOrderAction).c_str(),
                LTSDatatypeFormater::ToString(pRspInfo).c_str());
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspOrderAction.");
    }
}

void LTSTradeInf::OnRspQryOrder(CSecurityFtdcOrderField* pOrder, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
//    TNL_LOG_DEBUG("OnRspQryOrder: requestid = %d, last_flag=%d \n%s \n%s",
//        nRequestID, bIsLast,
//        LTSDatatypeFormater::ToString(pOrder).c_str(),
//        LTSDatatypeFormater::ToString(pRspInfo).c_str());

    TNL_LOG_INFO("OnRspQryOrder, order: %p, last: %d", pOrder, bIsLast);
    if (!HaveFinishQueryOrders())
    {
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
        }

        if (!finish_query_canceltimes_)
        {
            std::unique_lock<std::mutex> lock(stats_canceltimes_sync_);
            if (pOrder && pOrder->OrderStatus == SECURITY_FTDC_OST_Canceled
                && pOrder->OrderSubmitStatus != SECURITY_FTDC_OSS_InsertRejected)
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
                // stats cancle times after terminate all orders, or terminate process will produce new cancel action
                TNL_LOG_INFO("OnRspQryOrder cancel contract size:%d", cancel_times_of_contract.size());
                if (have_handled_unterminated_orders_)
                {
                    for (std::map<std::string, int>::iterator it = cancel_times_of_contract.begin();
                        it != cancel_times_of_contract.end(); ++it)
                    {
                        ComplianceCheck_SetCancelTimes(tunnel_info_.account.c_str(), it->first.c_str(), it->second);
                    }
                    finish_query_canceltimes_ = true;
                }
                cancel_times_of_contract.clear();
            }
        }

        if (bIsLast)
        {
            qry_order_finish_cond_.notify_one();
        }

        if (HaveFinishQueryOrders())
        {
            SetQueryFinished();
        }

        TNL_LOG_INFO("OnRspQryOrder when cancel unterminated orders, order: %p, last: %d", pOrder, bIsLast);
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

    // empty Order detail
    if (!pOrder)
    {
        ret.error_no = 0;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }

    OrderDetail od;
    strncpy(od.stock_code, pOrder->InstrumentID, sizeof(TSecurityFtdcInstrumentIDType));
    od.entrust_no = LTSFieldConvert::GetEntrustNo(pOrder->OrderSysID);
    od.order_kind = LTSFieldConvert::PriceTypeTrans(pOrder->OrderPriceType);
    od.direction = pOrder->Direction;
    od.open_close = pOrder->CombOffsetFlag[0];
    od.speculator = LTSFieldConvert::EntrustTbFlagTrans(pOrder->CombHedgeFlag[0]);
    od.entrust_status = LTSFieldConvert::EntrustStatusTrans(pOrder->OrderSubmitStatus, pOrder->OrderStatus);
    od.limit_price = atof(pOrder->LimitPrice);
    od.volume = pOrder->VolumeTotalOriginal;
    od.volume_traded = pOrder->VolumeTraded;
    od.volume_remain = pOrder->VolumeTotalOriginal - pOrder->VolumeTraded;
    od_buffer_.push_back(od);

    if (bIsLast)
    {
        ret.datas.swap(od_buffer_);
        ret.error_no = 0;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}

void LTSTradeInf::OnRspQryTrade(CSecurityFtdcTradeField* pTrade, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    T_TradeDetailReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ret.error_no = -1;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    TNL_LOG_DEBUG("OnRspQryTrade: requestid = %d, last_flag=%d \n%s \n%s",
        nRequestID, bIsLast,
        LTSDatatypeFormater::ToString(pTrade).c_str(),
        LTSDatatypeFormater::ToString(pRspInfo).c_str());

    // empty trade detail
    if (!pTrade)
    {
        ret.error_no = 0;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    TradeDetail td;
    strncpy(td.stock_code, pTrade->InstrumentID, sizeof(TSecurityFtdcInstrumentIDType));
    td.entrust_no = LTSFieldConvert::GetEntrustNo(pTrade->OrderSysID);
    td.direction = pTrade->Direction;
    td.open_close = pTrade->OffsetFlag;
    td.speculator = LTSFieldConvert::EntrustTbFlagTrans(pTrade->HedgeFlag);
    td.trade_price = atof(pTrade->Price);
    td.trade_volume = pTrade->Volume;
    strncpy(td.trade_time, pTrade->TradeTime, sizeof(TSecurityFtdcTimeType));
    td_buffer_.push_back(td);

    if (bIsLast)
    {
        ret.datas.swap(td_buffer_);
        ret.error_no = 0;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
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

PositionDetail LTSTradeInf::PackageAvailableFunc()
{
    // insert available fund
    PositionDetail ac;
    memset(&ac, 0, sizeof(ac));
    strcpy(ac.stock_code, "#CASH");
    ac.direction = '0';
    ac.position = (long) available_fund;  // TODO available fund at beginning, don't update after init
    ac.exchange_type = ' ';

    return ac;
}

void LTSTradeInf::OnRspQryInvestorPosition(CSecurityFtdcInvestorPositionField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
{
    TNL_LOG_DEBUG("OnRspQryInvestorPositionDetail: requestid = %d, last_flag=%d \n%s \n%s",
        req_id, last,
        LTSDatatypeFormater::ToString(pf).c_str(),
        LTSDatatypeFormater::ToString(rsp).c_str());

    T_PositionReturn ret;

    // respond error
    if (rsp && rsp->ErrorID != 0)
    {
        ret.error_no = -1;
        QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    if (pf)
    {
        PositionDetail pos;
        memset(&pos, 0, sizeof(pos));
        strncpy(pos.stock_code, pf->InstrumentID, sizeof(pos.stock_code));
        pos.exchange_type = LTSFieldConvert::ExchNameToExchCode(pf->ExchangeID);

        int cur_pos = (int) pf->Position;
        int yd_pos = (int) pf->YdPosition;
        if (pf->PosiDirection == SECURITY_FTDC_PD_Long)
        {
            pos.direction = MY_TNL_D_BUY;
        }
        else if (pf->PosiDirection == SECURITY_FTDC_PD_Short)
        {
            pos.direction = MY_TNL_D_SELL;
        }
        else if (pf->PosiDirection == SECURITY_FTDC_PD_Net)
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
            pos.position_avg_price = pf->OpenCost / GetContractMultiple(std::string(pos.stock_code)); // now is total cost
        }
        if (yd_pos != 0)
        {
            pos.yestoday_position = (int) yd_pos;
            pos.yd_position_avg_price = pf->OpenCost / GetContractMultiple(std::string(pos.stock_code)); // now is total cost
        }

        TNL_LOG_INFO("pos_data %s - pos:%d, yd_pod:%d", pos.stock_code, pos.position, pos.yestoday_position);
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

    if (last)
    {
        if (query_position_step == 1)
        {
            pos_buffer_query_on_time_ = pos_buffer_;
            query_position_step = 2;
        }
        else
        {
            CheckAndSaveYestodayPosition();
            TNL_LOG_INFO("receive last respond of query position, send to client");
            ret.datas.swap(pos_buffer_);

            // TODO return fund in stock system and option trade
            ret.datas.insert(ret.datas.begin(), PackageAvailableFunc());

            ret.error_no = 0;
            QryPosReturnHandler_(&ret);
            LogUtil::OnPositionRtn(&ret, tunnel_info_);
        }
    }
}

void LTSTradeInf::OnRspQryInstrument(CSecurityFtdcInstrumentField *pf, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    T_ContractInfoReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ret.error_no = -1;
        if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);
        LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
        TNL_LOG_WARN("OnRspQryInstrument return error, ErrorID:%d", pRspInfo->ErrorID);
        return;
    }

//    //TNL_LOG_DEBUG("OnRspQryInstrument: requestid = %d, last_flag=%d", nRequestID, bIsLast);
//    TNL_LOG_DEBUG("OnRspQryInstrument: requestid = %d, last_flag=%d \n%s \n%s",
//        nRequestID, bIsLast,
//        LTSDatatypeFormater::ToString(pf).c_str(),
//        LTSDatatypeFormater::ToString(pRspInfo).c_str());

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        return;
    }

    // option tunnel, only get option instruments
    if (pf && (pf->InstrumentType == SECURITY_FTDC_IT_CallOptions || pf->InstrumentType == SECURITY_FTDC_IT_PutOptions))
    {
        ContractInfo ci;
        // modify contractname: contract_code#instrumentid on 20160324
        TSecurityFtdcExchangeInstIDType contract_code_tmp;
        memcpy(contract_code_tmp, pf->ExchangeInstID, sizeof(contract_code_tmp));
        TrimTailSpace(contract_code_tmp, sizeof(contract_code_tmp));
        std::string n_tmp(contract_code_tmp);
        n_tmp.append("#");
        n_tmp.append(pf->InstrumentID);
        strncpy(ci.stock_code, n_tmp.c_str(), sizeof(ci.stock_code));
        memcpy(ci.TradeDate, trade_day, sizeof(ci.TradeDate));
        memcpy(ci.ExpireDate, pf->ExpireDate, sizeof(ci.ExpireDate));
        ci_buffer_.push_back(ci);

        std::lock_guard<std::mutex> lock(contract_sync_);
        contract_multiple_[std::string(pf->InstrumentID)] = pf->VolumeMultiple;
    }

    if (bIsLast)
    {
        finish_query_contracts_ = true;
        TNL_LOG_DEBUG("finish OnRspQryInstrument, data size:%d", ci_buffer_.size());

        if (!HaveFinishQueryOrders())
        {
            std::thread t_handle_orders(std::bind(&LTSTradeInf::QueryAndHandleOrders, this));
            t_handle_orders.detach();
        }
        else
        {
            SetQueryFinished();
        }
    }
}

int LTSTradeInf::GetContractMultiple(const std::string &contract)
{
    int m = 1;
    std::lock_guard<std::mutex> lock(contract_sync_);
    auto it = contract_multiple_.find(contract);
    if (it != contract_multiple_.end())
    {
        m = it->second;
    }
    if (m < 1) m = 1;

    return m;
}

void LTSTradeInf::OnRspQryTradingAccount(CSecurityFtdcTradingAccountField *pf, CSecurityFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    TNL_LOG_DEBUG("OnRspQryTradingAccount: requestid = %d, last_flag=%d \n%s \n%s",
        nRequestID, bIsLast,
        LTSDatatypeFormater::ToString(pf).c_str(),
        LTSDatatypeFormater::ToString(pRspInfo).c_str());

    if (pf)
        available_fund += pf->Available;

    if (bIsLast)
    {
        std::thread t_qry_contract(&LTSTradeInf::QueryContractInfo, this);
        t_qry_contract.detach();
    }
}

CSecurityFtdcInputOrderActionField LTSTradeInf::CreatCancelParam(const CSecurityFtdcOrderField& order_field)
{
    CSecurityFtdcInputOrderActionField cancle_order;
    OrderRefDataType order_ref = lts_trade_context_.GetNewOrderRef();
    memset(&cancle_order, 0, sizeof(cancle_order));

    memcpy(cancle_order.ExchangeID, order_field.ExchangeID, sizeof(cancle_order.ExchangeID));
    memcpy(cancle_order.OrderLocalID, order_field.OrderLocalID, sizeof(cancle_order.OrderLocalID));
    memcpy(cancle_order.BrokerID, order_field.BrokerID, sizeof(cancle_order.BrokerID));
    memcpy(cancle_order.InvestorID, order_field.InvestorID, sizeof(cancle_order.InvestorID));

    cancle_order.OrderActionRef = (int) order_ref;
    cancle_order.FrontID = order_field.FrontID;
    cancle_order.SessionID = order_field.SessionID;
    memcpy(cancle_order.OrderRef, order_field.OrderRef, sizeof(cancle_order.OrderRef));
    memcpy(cancle_order.UserID, order_field.UserID, sizeof(cancle_order.UserID));
    memcpy(cancle_order.InstrumentID, order_field.InstrumentID, sizeof(cancle_order.InstrumentID));
    memcpy(cancle_order.OrderLocalID, order_field.OrderLocalID, sizeof(cancle_order.OrderLocalID));
    cancle_order.ActionFlag = SECURITY_FTDC_AF_Delete;
    cancle_order.LimitPrice = 0;
    cancle_order.VolumeChange = 0;

    return cancle_order;
}

bool LTSTradeInf::IsOrderTerminate(const CSecurityFtdcOrderField& order_field)
{
    return order_field.OrderStatus == SECURITY_FTDC_OST_AllTraded
        || order_field.OrderStatus == SECURITY_FTDC_OST_Canceled;
}

void LTSTradeInf::QueryAndHandleOrders()
{
    // query order detail parameter
    CSecurityFtdcQryOrderField qry_param;
    memset(&qry_param, 0, sizeof(CSecurityFtdcQryOrderField));
    strncpy(qry_param.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TSecurityFtdcBrokerIDType));
    strncpy(qry_param.InvestorID, tunnel_info_.account.c_str(), sizeof(TSecurityFtdcInvestorIDType));

    //超时后没有完成查询，重试。为防止委托单太多，10s都回报不了，每次超时加5s
    int wait_seconds = 10;

    std::vector<CSecurityFtdcOrderField> unterminated_orders_t;
    while (!HaveFinishQueryOrders())
    {
        {
            std::unique_lock<std::mutex> lock(stats_canceltimes_sync_);
            cancel_times_of_contract.clear();
        }
        //主动查询所有报单
        while (true)
        {
            int qry_result = query_api_->ReqQryOrder(&qry_param, 0);
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
            for (const CSecurityFtdcOrderField &order_field : unterminated_orders_t)
            {
                CSecurityFtdcInputOrderActionField action_field = CreatCancelParam(order_field);
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
void LTSTradeInf::CheckAndSaveYestodayPosition()
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

void LTSTradeInf::LoadYestodayPositionFromFile(const std::string &file)
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
            for (const std::string &v : fields)
            {
                TNL_LOG_INFO("yd_pos_field:", v.c_str());
            }
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

void LTSTradeInf::SaveYestodayPositionToFile(const std::string &file)
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

void LTSTradeInf::OnRspQryExchange(CSecurityFtdcExchangeField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::OnRspQryInvestor(CSecurityFtdcInvestorField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::OnRspQryTradingCode(CSecurityFtdcTradingCodeField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::OnRspQryMarketRationInfo(CSecurityFtdcMarketRationInfoField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::OnRspQryInstrumentCommissionRate(CSecurityFtdcInstrumentCommissionRateField* pf, CSecurityFtdcRspInfoField* rsp, int req_id,
bool last)
{
}

void LTSTradeInf::OnRspQryETFInstrument(CSecurityFtdcETFInstrumentField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::OnRspQryETFBasket(CSecurityFtdcETFBasketField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::OnRspQryOFInstrument(CSecurityFtdcOFInstrumentField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::OnRspQrySFInstrument(CSecurityFtdcSFInstrumentField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::OnRspQryInstrumentUnitMargin(CSecurityFtdcInstrumentUnitMarginField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::OnRspQryMarketDataStaticInfo(CSecurityFtdcMarketDataStaticInfoField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
}

void LTSTradeInf::RecordPositionAtSpecTime()
{
    TNL_LOG_INFO("thread start: query position at specific time.");
    bool is_query_time = false;
    while (true)
    {
        sleep(1);
        int cur_time = my_cmn::GetCurrentTimeInt();
        if (cur_time >= time_to_query_pos_int)
        {
            is_query_time = true;
            break;
        }
    }

    TNL_LOG_INFO("it's time to query: query position at specific time.");
    if (is_query_time)
    {
        CSecurityFtdcQryInvestorPositionField qry_param;
        memset(&qry_param, 0, sizeof(CSecurityFtdcQryInvestorPositionDetailField));
        ///经纪公司编号
        strncpy(qry_param.BrokerID, cfg_.Logon_config().brokerid.c_str(), sizeof(TSecurityFtdcBrokerIDType));
        ///投资者编号
        strncpy(qry_param.InvestorID, tunnel_info_.account.c_str(), sizeof(TSecurityFtdcInvestorIDType));

        query_position_step = 1;
        pos_buffer_.clear();
        while (true)
        {
            int ret = query_api_->ReqQryInvestorPosition(&qry_param, 0);
            TNL_LOG_INFO("ReqQryInvestorPosition for query position on specific time - return:%d", ret);
            if (ret == 0)
            {
                break;
            }
            sleep(5);
        }
        TNL_LOG_INFO("send query request success: query position at specific time.");

        // wait while finish query
        while (query_position_step != 2)
        {
            sleep(1);
        }

        TNL_LOG_INFO("finish query: query position at specific time.");
        SaveCurrentPosition();
    }

    TNL_LOG_INFO("thread exit: query position at specific time.");
}

void LTSTradeInf::SaveCurrentPosition()
{
    // 存储文件名： pos_account_YYYYMMDD.txt
    std::string pos_file_name = "pos_" + tunnel_info_.account + "_" + my_cmn::GetCurrentDateString() + ".txt";
    int ret = access(pos_file_name.c_str(), F_OK);

    bool file_exist = (ret == 0);

    if (file_exist)
    {
        std::string pos_file_name_bak = pos_file_name + "." + my_cmn::GetCurrentDateTimeWithMilisecString();
        rename(pos_file_name.c_str(), pos_file_name_bak.c_str());
        TNL_LOG_WARN("file(%s) exist when SaveCurrentPosition, backup it", pos_file_name.c_str());
    }

    std::ofstream pos_fs(pos_file_name.c_str());
    if (!pos_fs)
    {
        TNL_LOG_ERROR("open file(%s) failed when SaveCurrentPosition", pos_file_name.c_str());
        return;
    }

    int pos_count = 0;
    for (PositionDetail v : pos_buffer_query_on_time_)
    {
        if (v.position > 0)
        {
            // 格式：合约，方向，昨仓量，开仓均价
            pos_fs << v.stock_code << ","
                << v.direction << ","
                << v.position << std::endl;

            ++pos_count;
        }
    }

    TNL_LOG_INFO("current position saved, valid position count:%d", pos_count);
}
