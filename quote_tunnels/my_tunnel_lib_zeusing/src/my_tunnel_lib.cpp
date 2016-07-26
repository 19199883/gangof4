#include "my_tunnel_lib.h"

#include <string>
#include <mutex>
#include <thread>

#include "qtm_with_code.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "config_data.h"
#include "trade_log_util.h"
#include "zeusing_trade_interface.h"
#include "field_convert.h"

#include "my_protocol_packager.h"
#include "check_schedule.h"

using namespace std;
using namespace my_cmn;

static void InitOnce()
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

        qtm_init(TYPE_TCA);

        std::string log_file_name = "my_tunnel_lib_" + my_cmn::GetCurrentDateTimeString();
        (void) my_log::instance(log_file_name.c_str());
        TNL_LOG_INFO("start init tunnel library.");

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

void ZeusingTunnel::RespondHandleThread(ZeusingTunnel *ptunnel)
{
    MYTraderSpi * p_zeusing_tnl = static_cast<MYTraderSpi *>(ptunnel->trade_inf_);
    if (!p_zeusing_tnl)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_zeusing_tnl->rsp_sync;
    std::condition_variable &rsp_con = p_zeusing_tnl->rsp_con;
    std::vector<std::pair<int, void *> > rsp_tmp;

    while (true)
    {
        std::unique_lock<std::mutex> lock(rsp_sync);
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
void ZeusingTunnel::SendRespondAsync(int rsp_type, void *rsp)
{
    MYTraderSpi * p_zeusing_tnl = static_cast<MYTraderSpi *>(trade_inf_);
    if (!p_zeusing_tnl)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_zeusing_tnl->rsp_sync;
    std::condition_variable &rsp_con = p_zeusing_tnl->rsp_con;
    {
        std::unique_lock<std::mutex> lock(rsp_sync);
        pending_rsp_.push_back(std::make_pair(rsp_type, rsp));
    }
    rsp_con.notify_one();
}

ZeusingTunnel::ZeusingTunnel(const std::string &provider_config_file)
{
    trade_inf_ = NULL;
    exchange_code_ = ' ';
    InitOnce();

    std::string cfg_file("my_trade_tunnel_zeusing.xml");
    if (!provider_config_file.empty())
    {
        cfg_file = provider_config_file;
    }

    TNL_LOG_INFO("create ZeusingTunnel object with configure file: %s", cfg_file.c_str());

    //TunnelConfigData cfg;
    lib_cfg_ = new TunnelConfigData();
    lib_cfg_->Load(cfg_file);
    exchange_code_ = lib_cfg_->Logon_config().exch_code.c_str()[0];
    tunnel_info_.account = lib_cfg_->Logon_config().investorid;

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "zeusing_%s_%d_%u", tunnel_info_.account.c_str(), lib_cfg_->App_cfg().orderref_prefix_id, getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

    update_state(tunnel_info_.qtm_name.c_str(), TYPE_TCA, QtmState::INIT, GetDescriptionWithState(QtmState::INIT).c_str());
    TNL_LOG_INFO("update_state: name: %s, State: %d, Description: %s.", tunnel_info_.qtm_name.c_str(), QtmState::INIT,
        GetDescriptionWithState(QtmState::INIT).c_str());

    // init tunnel log object
    LogUtil::Start("my_tunnel_lib_zeusing", lib_cfg_->App_cfg().share_memory_key);

    int provider_type = lib_cfg_->App_cfg().provider_type;
    if (provider_type == TunnelProviderType::PROVIDERTYPE_ZEUSING)
    {
        InitInf(*lib_cfg_);
    }
    else
    {
        TNL_LOG_ERROR("not support tunnel provider type, please check config file.");
    }

    // init respond thread
    std::thread rsp_thread(&ZeusingTunnel::RespondHandleThread, this);
    rsp_thread.detach();
}

std::string ZeusingTunnel::GetClientID()
{
    return tunnel_info_.account;
}

bool ZeusingTunnel::InitInf(const TunnelConfigData &cfg)
{
    // 连接服务
    TNL_LOG_INFO("prepare to start Zeusing tunnel server.");

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
    trade_inf_ = new MYTraderSpi(cfg);
    return true;
}

void ZeusingTunnel::PlaceOrder(const T_PlaceOrder *pPlaceOrder)
{
    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnPlaceOrder(pPlaceOrder, tunnel_info_);
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);

    OrderRefDataType return_param;
    if (p_tunnel)
    {
        // 获取OrderRef和往柜台报单，需要串行化处理，以免先获取OrderRef的线程，后报单到柜台，造成报单重复错误
        lock_guard<mutex> lock(p_tunnel->client_sync);

        OrderRefDataType order_ref = p_tunnel->api_trade_context_.GetNewOrderRef();
        TNL_LOG_DEBUG("serial_no: %ld map order_ref: %lld", pPlaceOrder->serial_no, order_ref);
        int request_id = p_tunnel->api_trade_context_.GetRequestID();

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
                TNL_LOG_INFO("update_compliance:  qtm_name: %s, type: %d, status: %d, des: %s",tunnel_info_.qtm_name.c_str(),
                    tag_compl_type_enum::OPEN_ORDER_LIMIT, QtmComplianceState::OPEN_POSITIONS_EXCEED_LIMITS,
                    GetComplianceDescriptionWithState(QtmComplianceState::OPEN_POSITIONS_EXCEED_LIMITS, tunnel_info_.account.c_str(),
                        pPlaceOrder->stock_code).c_str());
                TNL_LOG_WARN("forbid open because current open volumn: %lld", return_param);
            }
            else if (process_result == TUNNEL_ERR_CODE::POSSIBLE_SELF_TRADE)
            {
                TNL_LOG_WARN("possible trade with order: %lld (order ref)", return_param);
            }
            else if (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                    QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD,
                    GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                        pPlaceOrder->stock_code).c_str());
                TNL_LOG_INFO("update_compliance:  qtm_name: %s, type: %d, status: %d, des: %s",tunnel_info_.qtm_name.c_str(),
                    tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD,
                    GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                        pPlaceOrder->stock_code).c_str());
                TNL_LOG_WARN("reach the warn threthold of cancel time, forbit open new position.");
            }
            else if (process_result == TUNNEL_ERR_CODE::CANCEL_REACH_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::CANCEL_TIME_OVER_MAXIMUN,
                    GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_MAXIMUN, tunnel_info_.account.c_str(),
                        pPlaceOrder->stock_code).c_str());
                TNL_LOG_WARN("reach the maximum of cancel time, forbit open new position.");
            }
        }
        else
        {
            if (process_result == TUNNEL_ERR_CODE::OPEN_EQUAL_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                    QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS,
                    GetComplianceDescriptionWithState(QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS, tunnel_info_.account.c_str(),
                        pPlaceOrder->stock_code).c_str());
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
            CZeusingFtdcInputOrderField counter_req;
            ApiPacker::OrderRequest(*lib_cfg_, pPlaceOrder, order_ref, counter_req);

            // save order_ref of order before request. sometime order respond before it has been saved. modified on 20160422
            p_tunnel->api_trade_context_.SaveSerialNoToOrderRef(order_ref,
                OriginalReqInfo(pPlaceOrder->serial_no,
                    p_tunnel->Front_id(), p_tunnel->Session_id(),
                    exchange_code_, pPlaceOrder->direction, pPlaceOrder->speculator, pPlaceOrder->open_close,
                    pPlaceOrder->order_type,
                    pPlaceOrder->volume, pPlaceOrder->order_kind, pPlaceOrder->stock_code, ""));

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
        ApiPacker::OrderRespond(process_result, pPlaceOrder->serial_no, 0, 0, *pOrderRespond);
        LogUtil::OnOrderRespond(pOrderRespond, tunnel_info_);

        SendRespondAsync(RespondDataType::kPlaceOrderRsp, pOrderRespond);
    }
}

void ZeusingTunnel::CancelOrder(const T_CancelOrder *pCancelOrder)
{
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnCancelOrder(pCancelOrder, tunnel_info_);

    if (p_tunnel)
    {
        // 获取OrderRef和往柜台报单，需要串行化处理，以免先获取OrderRef的线程，后报单到柜台，造成报单重复错误
        lock_guard<mutex> lock(p_tunnel->client_sync);

        OrderRefDataType order_ref = p_tunnel->api_trade_context_.GetNewOrderRef();
        TNL_LOG_INFO("cancel_serial_no: %ld map action_ref: %lld", pCancelOrder->serial_no, order_ref);
        int request_id = p_tunnel->api_trade_context_.GetRequestID();
        OrderRefDataType org_order_ref = p_tunnel->api_trade_context_.GetOrderRefBySerialNo(pCancelOrder->org_serial_no);
        const OriginalReqInfo * org_order_info = p_tunnel->api_trade_context_.GetOrderInfoByOrderRef(org_order_ref);

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
                GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                    pCancelOrder->stock_code).c_str());
            TNL_LOG_WARN("cancel time equal threshold");
        }

        if ((process_result == TUNNEL_ERR_CODE::RESULT_SUCCESS)
            || (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
            || (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT))
        {

            CZeusingFtdcInputOrderActionField counter_req;
            ApiPacker::CancelRequest(*lib_cfg_, pCancelOrder, order_ref, org_order_ref, org_order_info,
                counter_req);

            // save cancel info before request. sometime respond before it has been saved. modified on 20160422
            char cancel_sn[64];
            sprintf(cancel_sn, "%ld", pCancelOrder->serial_no);
            p_tunnel->api_trade_context_.UpdateCancelOrderRef(org_order_ref, cancel_sn);

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

    if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS
        && process_result != TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD
        && process_result != TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
    {
        // 应答包
        T_CancelRespond *cancle_order = new T_CancelRespond();
        ApiPacker::CancelRespond(process_result, pCancelOrder->serial_no, 0L, *cancle_order);
        LogUtil::OnCancelRespond(cancle_order, tunnel_info_);

        SendRespondAsync(RespondDataType::kCancelOrderRsp, cancle_order);
    }
}

void ZeusingTunnel::QueryPosition(const T_QryPosition *p)
{
    LogUtil::OnQryPosition(p, tunnel_info_);
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    T_PositionReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query position, please check configure file");

        ret.error_no = 2;
        if (QryPosReturnHandler_) QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryPosition(p);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryPosReturnHandler_) QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}
void ZeusingTunnel::QueryOrderDetail(const T_QryOrderDetail *pQryParam)
{
    LogUtil::OnQryOrderDetail(pQryParam, tunnel_info_);
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    T_OrderDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query order detail, please check configure file");

        ret.error_no = 2;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryOrderDetail(pQryParam);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}
void ZeusingTunnel::QueryTradeDetail(const T_QryTradeDetail *pQryParam)
{
    LogUtil::OnQryTradeDetail(pQryParam, tunnel_info_);
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    T_TradeDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query trade detail, please check configure file");

        ret.error_no = 2;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryTradeDetail(pQryParam);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

void ZeusingTunnel::QueryContractInfo(const T_QryContractInfo *pQryParam)
{
    LogUtil::OnQryContractInfo(pQryParam, tunnel_info_);
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    T_ContractInfoReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query contract info, please check configure file");

        ret.error_no = 2;
        if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);
        LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryInstrument(pQryParam);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);
        LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
    }
}

//empty, don't need support quote now 20160425
// 询价
void ZeusingTunnel::ReqForQuoteInsert(const T_ReqForQuote *p)
{
    LogUtil::OnReqForQuote(p, tunnel_info_);
}
///报价录入请求
void ZeusingTunnel::ReqQuoteInsert(const T_InsertQuote *p)
{
    LogUtil::OnInsertQuote(p, tunnel_info_);
}
///报价操作请求
void ZeusingTunnel::ReqQuoteAction(const T_CancelQuote *p)
{
    LogUtil::OnCancelQuote(p, tunnel_info_);
}

void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_OrderRespond *)> callback_handler)
{
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init when SetCallbackHandler");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    OrderRespond_call_back_handler_ = callback_handler;
}

void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_CancelRespond *)> callback_handler)
{
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init when SetCallbackHandler");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    CancelRespond_call_back_handler_ = callback_handler;
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_OrderReturn *)> callback_handler)
{
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init when SetCallbackHandler");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_TradeReturn *)> callback_handler)
{
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init when SetCallbackHandler");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
{
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init when SetCallbackHandler");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryPosReturnHandler_ = handler;
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler)
{
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init when SetCallbackHandler");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryOrderDetailReturnHandler_ = handler;
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
{
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init when SetCallbackHandler");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryTradeDetailReturnHandler_ = handler;
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_ContractInfoReturn *)> handler)
{
    TNL_LOG_DEBUG("ZeusingTunnel::SetCallbackHandler - QryContractInfoHandler_");
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init when SetCallbackHandler");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryContractInfoHandler_ = handler;
}

// added for market making
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_RtnForQuote *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_RtnForQuote)");
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_RspOfReqForQuote *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_RspOfReqForQuote)");
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_InsertQuoteRespond *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_InsertQuoteRespond)");
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_CancelQuoteRespond *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_CancelQuoteRespond)");
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_QuoteReturn *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_QuoteReturn)");
}
void ZeusingTunnel::SetCallbackHandler(std::function<void(const T_QuoteTrade *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_QuoteTrade)");
}

ZeusingTunnel::~ZeusingTunnel()
{
    MYTraderSpi * p_tunnel = static_cast<MYTraderSpi *>(trade_inf_);
    if (p_tunnel)
    {
        delete p_tunnel;
        trade_inf_ = NULL;
    }

    qtm_finish();
    LogUtil::Stop();
}

MYTunnelInterface *CreateTradeTunnel(const std::string &tunnel_config_file)
{
    return new ZeusingTunnel(tunnel_config_file);
}
void DestroyTradeTunnel(MYTunnelInterface * p)
{
    if (p) delete p;
}
