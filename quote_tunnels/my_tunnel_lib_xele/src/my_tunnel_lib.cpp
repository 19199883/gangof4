#include "my_tunnel_lib.h"

#include <string>
#include <mutex>

#include "qtm_with_code.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "config_data.h"
#include "trade_log_util.h"
#include "xele_trade_interface.h"
#include "field_convert.h"

#include "my_protocol_packager.h"
#include "check_schedule.h"

using namespace std;
using namespace my_cmn;

void InitOnce()
{
    static volatile bool s_have_init = false;
    static std::mutex s_init_sync;

    if (s_have_init)
    {
        return;
    }
    else
    {
        lock_guard<mutex> lock(s_init_sync);
        if (s_have_init)
        {
            return;
        }
        std::string log_file_name = "my_tunnel_lib_" + my_cmn::GetCurrentDateTimeString();
        (void) my_log::instance(log_file_name.c_str());
        TNL_LOG_INFO("start init tunnel library.");

        // initialize tunnel monitor
        qtm_init(TYPE_TCA);

        s_have_init = true;
    }
}

enum RespondDataType
{
    kPlaceOrderRsp = 1,
    kCancelOrderRsp = 2,
    kInsertForQuoteRsp = 3,
    kInsertQuoteRsp = 4,
    kCancelQuoteRsp = 5,
};

void XeleTunnel::RespondHandleThread(XeleTunnel *ptunnel)
{
    MyXeleTradeSpi * p_xele = static_cast<MyXeleTradeSpi *>(ptunnel->trade_inf_);
    if (!p_xele)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_xele->rsp_sync;
    std::condition_variable &rsp_con = p_xele->rsp_con;

    std::vector<std::pair<int, void *> > rsp_tmp;

    while (true)
    {
        std::unique_lock<mutex> lock(rsp_sync);
        while (ptunnel->pending_rsp_.empty())
        {
            rsp_con.wait(lock);
        }

        ptunnel->pending_rsp_.swap(rsp_tmp);
        for (std::pair<int, void *> &v : rsp_tmp)
        {
            switch (v.first)
            {
                case RespondDataType::kPlaceOrderRsp:
                    {
                    T_OrderRespond *p = (T_OrderRespond *) v.second;
                    if (ptunnel->OrderRespond_call_back_handler_) ptunnel->OrderRespond_call_back_handler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kCancelOrderRsp:
                    {
                    T_CancelRespond *p = (T_CancelRespond *) v.second;
                    if (ptunnel->CancelRespond_call_back_handler_) ptunnel->CancelRespond_call_back_handler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kInsertForQuoteRsp:
                    {
                    T_RspOfReqForQuote *p = (T_RspOfReqForQuote *) v.second;
                    if (ptunnel->RspOfReqForQuoteHandler_) ptunnel->RspOfReqForQuoteHandler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kInsertQuoteRsp:
                    {
                    T_InsertQuoteRespond *p = (T_InsertQuoteRespond *) v.second;
                    if (ptunnel->InsertQuoteRespondHandler_) ptunnel->InsertQuoteRespondHandler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kCancelQuoteRsp:
                    {
                    T_CancelQuoteRespond *p = (T_CancelQuoteRespond *) v.second;
                    if (ptunnel->CancelQuoteRespondHandler_) ptunnel->CancelQuoteRespondHandler_(p);
                    delete p;
                }
                    break;
                default:
                    TNL_LOG_ERROR("unknown type of respond message: %d", v.first);
                    break;
            }
        }

        rsp_tmp.clear();
    }
}
void XeleTunnel::SendRespondAsync(int rsp_type, void *rsp)
{
    MyXeleTradeSpi * p_xele = static_cast<MyXeleTradeSpi *>(trade_inf_);
    if (!p_xele)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_xele->rsp_sync;
    std::condition_variable &rsp_con = p_xele->rsp_con;

    {
        std::unique_lock<std::mutex> lock(rsp_sync);
        pending_rsp_.push_back(std::make_pair(rsp_type, rsp));
    }
    rsp_con.notify_one();
}

XeleTunnel::XeleTunnel(const std::string &provider_config_file)
{
    trade_inf_ = NULL;
    exchange_code_ = ' ';
    InitOnce();

    // load config file
    std::string cfg_file("my_trade_tunnel_xele.xml");
    if (!provider_config_file.empty())
    {
        cfg_file = provider_config_file;
    }

    TNL_LOG_INFO("create MYTunnel object with configure file: %s", cfg_file.c_str());

    //TunnelConfigData cfg;
    lib_cfg_ = new TunnelConfigData();
    lib_cfg_->Load(cfg_file);
    exchange_code_ = lib_cfg_->Logon_config().exch_code.c_str()[0];
    tunnel_info_.account = lib_cfg_->Logon_config().investorid;

    //qtm init
    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "xele_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::INIT);

    // start log output thread
    LogUtil::Start("my_tunnel_lib_xele", lib_cfg_->App_cfg().share_memory_key);

    int provider_type = lib_cfg_->App_cfg().provider_type;
    if (provider_type == TunnelProviderType::PROVIDERTYPE_XELE)
    {
        InitInf(*lib_cfg_);
    }
    else
    {
        TNL_LOG_ERROR("not support tunnel provider type, please check config file.");
    }

    // init respond thread
    rsp_thread_ = new std::thread(&XeleTunnel::RespondHandleThread, this);
}

std::string XeleTunnel::GetClientID()
{
    return tunnel_info_.account;
}

bool XeleTunnel::InitInf(const TunnelConfigData& cfg)
{
    TNL_LOG_INFO("prepare to start ac_xele tunnel.");
    const ComplianceCheckParam &param = cfg.Compliance_check_param();
    ComplianceCheck_Init(
        tunnel_info_.account.c_str(),
        param.cancel_warn_threshold,
        param.cancel_upper_limit,
        param.max_open_of_speculate,
        param.max_open_of_arbitrage,
        param.max_open_of_total,
        param.switch_mask.c_str());

    char init_msg[127];
    sprintf(init_msg, "%s: Init compliance check", tunnel_info_.account.c_str());
    update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::OPEN_ORDER_LIMIT, QtmComplianceState::INIT_COMPLIANCE_CHECK, init_msg);
    update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::INIT_COMPLIANCE_CHECK, init_msg);

    trade_inf_ = new MyXeleTradeSpi(cfg);
    return true;
}

void XeleTunnel::PlaceOrder(const T_PlaceOrder *pPlaceOrder)
{
    OrderRefDataType return_param;
    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;

    LogUtil::OnPlaceOrder(pPlaceOrder, tunnel_info_);

    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);

    if (p_tunnel)
    {
        // 获取OrderRef和往柜台报单，需要串行化处理，以免先获取OrderRef的线程，后报单到柜台，造成报单重复错误
        lock_guard<mutex> lock(p_tunnel->client_sync);

        OrderRefDataType order_ref = p_tunnel->xele_trade_context_.GetNewOrderRef();
        TNL_LOG_DEBUG("serial_no: %ld map order_ref: %012lld", pPlaceOrder->serial_no, order_ref);
        int request_id = p_tunnel->xele_trade_context_.GetRequestID();

        process_result = ComplianceCheck_TryReqOrderInsert(
            tunnel_info_.account.c_str(),
            order_ref,
            exchange_code_,
            pPlaceOrder->stock_code,
            pPlaceOrder->volume,
            pPlaceOrder->limit_price,
            pPlaceOrder->order_kind,
            pPlaceOrder->speculator,
            pPlaceOrder->direction,
            pPlaceOrder->open_close,
            pPlaceOrder->order_type,
            &return_param);

        if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS &&
            process_result != TUNNEL_ERR_CODE::OPEN_EQUAL_LIMIT &&
            process_result != TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
        {
            // 日志
            if (process_result == TUNNEL_ERR_CODE::CFFEX_EXCEED_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::OPEN_ORDER_LIMIT,
                    QtmComplianceState::OPEN_POSITIONS_EXCEED_LIMITS,
                    GetComplianceDescriptionWithState(QtmComplianceState::OPEN_POSITIONS_EXCEED_LIMITS, tunnel_info_.account.c_str(),
                        pPlaceOrder->stock_code).c_str());
                TNL_LOG_WARN("forbid open because current open volumn: %ld", return_param);
            }
            else if (process_result == TUNNEL_ERR_CODE::POSSIBLE_SELF_TRADE)
            {
                TNL_LOG_WARN("possible trade with order: %ld", return_param);
            }
            else if (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                    QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD,
                    GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                        pPlaceOrder->stock_code).c_str());
                TNL_LOG_WARN("reach the warn threthold of cancel time, forbit open new position.");
            }
        }
        else
        {
            if (process_result == TUNNEL_ERR_CODE::OPEN_EQUAL_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                    QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS,
                    GetComplianceDescriptionWithState(QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS, tunnel_info_.account.c_str(), pPlaceOrder->stock_code).c_str());
                TNL_LOG_WARN("equal the warn threthold of open position.");
            }
            else if (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                    QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD,
                    GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                        pPlaceOrder->stock_code).c_str());
                TNL_LOG_WARN("equal the warn threthold of cancel time.");
            }
            CXeleFtdcInputOrderField counter_req;
            XELEPacker::OrderRequest(*lib_cfg_, pPlaceOrder, order_ref, counter_req);
            p_tunnel->xele_trade_context_.SaveSerialNoToOrderRef(order_ref,
                XeleOriginalReqInfo(pPlaceOrder->serial_no,
                    exchange_code_, pPlaceOrder->direction, pPlaceOrder->speculator, pPlaceOrder->open_close,
                    pPlaceOrder->order_type,
                    pPlaceOrder->volume, pPlaceOrder->order_kind, pPlaceOrder->stock_code, order_ref));
            process_result = p_tunnel->ReqOrderInsert(&counter_req, request_id);

            // 下单
            // 发送失败，即时回滚
            if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    order_ref,
                    exchange_code_,
					pPlaceOrder->stock_code,
                    pPlaceOrder->volume,
                    pPlaceOrder->speculator,
                    pPlaceOrder->open_close,
                    pPlaceOrder->order_type);
            }
            else
            {
                p_tunnel->xele_trade_context_.SaveOrderRefToStatus(order_ref,
                    OrderStatusInTunnel(pPlaceOrder->volume));
            }
        }
    }
    else
    {
        TNL_LOG_ERROR("not support tunnel, check configure file");
    }

    // 请求失败后即时回报，否则，等柜台回报
    if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        TNL_LOG_WARN("place order failed, result=%d", process_result);

        // 应答包
        T_OrderRespond *pOrderRespond = new T_OrderRespond();
        XELEPacker::OrderRespond(process_result, pPlaceOrder->serial_no, "0", 'e', *pOrderRespond);

        LogUtil::OnOrderRespond(pOrderRespond, tunnel_info_);

        SendRespondAsync(RespondDataType::kPlaceOrderRsp, pOrderRespond);
    }
}

void XeleTunnel::CancelOrder(const T_CancelOrder *pCancelOrder)
{
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnCancelOrder(pCancelOrder, tunnel_info_);

    if (p_tunnel)
    {
        // 获取OrderRef和往柜台报单，需要串行化处理，以免先获取OrderRef的线程，后报单到柜台，造成报单重复错误
        lock_guard<mutex> lock(p_tunnel->client_sync);

        OrderRefDataType order_ref = p_tunnel->xele_trade_context_.GetNewOrderRef();
        TNL_LOG_DEBUG("cancel_serial_no: %ld map action_ref: %012lld", pCancelOrder->serial_no, order_ref);
        int request_id = p_tunnel->xele_trade_context_.GetRequestID();
        OrderRefDataType org_order_ref = p_tunnel->xele_trade_context_.GetOrderRefBySerialNo(pCancelOrder->org_serial_no);

        process_result = ComplianceCheck_TryReqOrderAction(
            tunnel_info_.account.c_str(),
            pCancelOrder->stock_code,
            org_order_ref);

        if (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
        {
            update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD,
                GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                    pCancelOrder->stock_code).c_str());
            TNL_LOG_WARN("cancel time approaches threshold");
        }
        if (process_result == TUNNEL_ERR_CODE::CANCEL_REACH_LIMIT)
        {
            update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                QtmComplianceState::CANCEL_TIME_OVER_MAXIMUN,
                GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_MAXIMUN, tunnel_info_.account.c_str(),
                    pCancelOrder->stock_code).c_str());
            TNL_LOG_WARN("cancel time reach limitation");
        }
        if (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
        {
            update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD,
                GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD, tunnel_info_.account.c_str(), pCancelOrder->stock_code).c_str());
            TNL_LOG_WARN("cancel time equal threshold");
        }

        if ((process_result == TUNNEL_ERR_CODE::RESULT_SUCCESS)
            || (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
            || (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT))
        {
            CXeleFtdcOrderActionField counter_req;
            XELEPacker::CancelRequest(*lib_cfg_, pCancelOrder, order_ref, org_order_ref, counter_req);
            p_tunnel->xele_trade_context_.SaveSerialNoToOrderRef(order_ref,
                XeleOriginalReqInfo(pCancelOrder->serial_no,
                    exchange_code_, pCancelOrder->direction, pCancelOrder->speculator, pCancelOrder->open_close, 0,
                    pCancelOrder->volume, 0, pCancelOrder->stock_code,
                    org_order_ref));
            process_result = p_tunnel->ReqOrderAction(&counter_req, request_id);

            if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                ComplianceCheck_OnOrderCancelFailed(
                    tunnel_info_.account.c_str(),
                    pCancelOrder->stock_code,
                    org_order_ref);
            }
            else
            {
                ComplianceCheck_OnOrderPendingCancel(
                    tunnel_info_.account.c_str(),
                    org_order_ref);
            }
        }
    }
    else
    {
        TNL_LOG_ERROR("not support tunnel when cancel order, please check configure file");
    }

    // request failed, respond now.
    if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS
        && process_result != TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD
        && process_result != TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
    {
        // 应答包
        T_CancelRespond *cancle_order = new T_CancelRespond();
        XELEPacker::CancelRespond(process_result, pCancelOrder->serial_no, "0", *cancle_order);

        LogUtil::OnCancelRespond(cancle_order, tunnel_info_);

        SendRespondAsync(RespondDataType::kCancelOrderRsp, cancle_order);
    }
}

void XeleTunnel::QueryPosition(const T_QryPosition *pQryPosition)
{
    LogUtil::OnQryPosition(pQryPosition, tunnel_info_);
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    T_PositionReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query position, please check configure file");

        ret.error_no = 2;
        if (QryPosReturnHandler_) QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    const LogonConfig & log_cfg = lib_cfg_->Logon_config();

    // qry client position
    CXeleFtdcQryClientPositionField qry_param;
    memset(&qry_param, 0, sizeof(CXeleFtdcQryClientPositionField));

    strncpy(qry_param.ClientID, log_cfg.investorid.c_str(), sizeof(qry_param.ClientID));
    qry_param.ClientType = log_cfg.ClientType;

    int qry_result = p_tunnel->QryPosition(&qry_param, 0);

//    CXeleFtdcQryPartPositionField qry_param;
//    memset(&qry_param, 0, sizeof(CXeleFtdcQryPartPositionField));
//
//    strncpy(qry_param.PartIDStart, log_cfg.ParticipantID.c_str(), sizeof(qry_param.PartIDStart));
//    strncpy(qry_param.PartIDEnd, log_cfg.ParticipantID.c_str(), sizeof(qry_param.PartIDEnd));
//
//    int qry_result = p_tunnel->QryPositionPart(&qry_param, 0);

    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryPosReturnHandler_) QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}
void XeleTunnel::QueryOrderDetail(const T_QryOrderDetail *pQryParam)
{
    LogUtil::OnQryOrderDetail(pQryParam, tunnel_info_);
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    T_OrderDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query order detail, please check configure file");

        ret.error_no = 2;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }

    CXeleFtdcQryOrderField qry_param;
    memset(&qry_param, 0, sizeof(CXeleFtdcQryOrderField));

    const LogonConfig & log_cfg = lib_cfg_->Logon_config();
    strncpy(qry_param.ClientID, log_cfg.clientid.c_str(), sizeof(qry_param.ClientID));

    int qry_result = p_tunnel->QryOrderDetail(&qry_param, 0);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}
void XeleTunnel::QueryTradeDetail(const T_QryTradeDetail *pQryParam)
{
    LogUtil::OnQryTradeDetail(pQryParam, tunnel_info_);
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    T_TradeDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query trade detail, please check configure file");

        ret.error_no = 2;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    CXeleFtdcQryTradeField qry_param;
    memset(&qry_param, 0, sizeof(CXeleFtdcQryTradeField));

    const LogonConfig & log_cfg = lib_cfg_->Logon_config();
    strncpy(qry_param.ClientID, log_cfg.clientid.c_str(), sizeof(qry_param.ClientID));

    int qry_result = p_tunnel->QryTradeDetail(&qry_param, 0);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

void XeleTunnel::QueryContractInfo(const T_QryContractInfo* pQryParam)
{
    LogUtil::OnQryContractInfo(pQryParam, tunnel_info_);
    T_ContractInfoReturn ret;
    TNL_LOG_ERROR("not support query contract info");

    ret.error_no = TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION;
    if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);

    LogUtil::OnContractInfoRtn(&ret, tunnel_info_);

    return;
}

//empty, xele can't support quote now 20150313
// 询价
void XeleTunnel::ReqForQuoteInsert(const T_ReqForQuote *p)
{
    LogUtil::OnReqForQuote(p, tunnel_info_);
}
///报价录入请求
void XeleTunnel::ReqQuoteInsert(const T_InsertQuote *p)
{
    LogUtil::OnInsertQuote(p, tunnel_info_);
}
///报价操作请求
void XeleTunnel::ReqQuoteAction(const T_CancelQuote *p)
{
    LogUtil::OnCancelQuote(p, tunnel_info_);
}

void XeleTunnel::SetCallbackHandler(std::function<void(const T_OrderRespond *)> callback_handler)
{
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    OrderRespond_call_back_handler_ = callback_handler;
}

void XeleTunnel::SetCallbackHandler(std::function<void(const T_CancelRespond *)> callback_handler)
{
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    CancelRespond_call_back_handler_ = callback_handler;
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_OrderReturn *)> callback_handler)
{
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_TradeReturn *)> callback_handler)
{
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
{
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryPosReturnHandler_ = handler;
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler)
{
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryOrderDetailReturnHandler_ = handler;
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
{
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryTradeDetailReturnHandler_ = handler;
}

void XeleTunnel::SetCallbackHandler(std::function<void(const T_ContractInfoReturn*)> handler)
{
    TNL_LOG_WARN("not support query contract when SetCallbackHandler(T_ContractInfoReturn)");
}

// added for market making
void XeleTunnel::SetCallbackHandler(std::function<void(const T_RtnForQuote *)> handler)
{
    TNL_LOG_WARN("not support mm interface when SetCallbackHandler(T_RtnForQuote)");
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_RspOfReqForQuote *)> handler)
{
    TNL_LOG_WARN("not support mm interface when SetCallbackHandler(T_RspOfReqForQuote)");
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_InsertQuoteRespond *)> handler)
{
    TNL_LOG_WARN("not support mm interface when SetCallbackHandler(T_InsertQuoteRespond)");
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_CancelQuoteRespond *)> handler)
{
    TNL_LOG_WARN("not support mm interface when SetCallbackHandler(T_CancelQuoteRespond)");
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_QuoteReturn *)> handler)
{
    TNL_LOG_WARN("not support mm interface when SetCallbackHandler(T_QuoteReturn)");
}
void XeleTunnel::SetCallbackHandler(std::function<void(const T_QuoteTrade *)> handler)
{
    TNL_LOG_WARN("not support mm interface when SetCallbackHandler(T_QuoteTrade)");
}

XeleTunnel::~XeleTunnel()
{
    MyXeleTradeSpi * p_tunnel = static_cast<MyXeleTradeSpi *>(trade_inf_);
    if (p_tunnel)
    {
        delete p_tunnel;
        trade_inf_ = NULL;
    }

    LogUtil::Stop();
}

MYTunnelInterface *CreateTradeTunnel(const std::string &tunnel_config_file)
{
    return new XeleTunnel(tunnel_config_file);
}
void DestroyTradeTunnel(MYTunnelInterface * p)
{
    if (p) delete p;
}
