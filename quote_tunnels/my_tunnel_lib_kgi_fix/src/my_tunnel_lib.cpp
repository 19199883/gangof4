#include "my_tunnel_lib.h"
#include "check_schedule.h"
#include "my_cmn_log.h"

// f8 headers
#include <fix8/f8includes.hpp>

#include "KGIfix_types.hpp"
#include "KGIfix_router.hpp"
#include "KGIfix_classes.hpp"

#include "kgi_fix_trade_interface.h"

//using namespace std;
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
        std::lock_guard<std::mutex> lock(s_init_sync);
        if (s_have_init)
        {
            return;
        }
        std::string log_file_name = "kgi_fix_tunnel_log_"
            + my_cmn::GetCurrentDateTimeString();
        (void) my_log::instance(log_file_name.c_str());
        TNL_LOG_INFO("start init tunnel library.");

        // initialize tunnel monitor
        qtm_init(TYPE_TCA);
        TNL_LOG_INFO("qtm_init");

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

void KgiFixTunnel::RespondHandleThread(KgiFixTunnel *ptunnel)
{
    MYFixTrader * p_kgi = static_cast<MYFixTrader *>(ptunnel->trade_inf_);
    if (!p_kgi)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_kgi->rsp_sync;
    std::condition_variable &rsp_con = p_kgi->rsp_con;
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
                    if (ptunnel->OrderRespond_call_back_handler_)
                        ptunnel->OrderRespond_call_back_handler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kCancelOrderRsp:
                {
                    T_CancelRespond *p = (T_CancelRespond *) v.second;
                    if (ptunnel->CancelRespond_call_back_handler_)
                        ptunnel->CancelRespond_call_back_handler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kInsertForQuoteRsp:
                {
                    T_RspOfReqForQuote *p = (T_RspOfReqForQuote *) v.second;
                    if (ptunnel->RspOfReqForQuoteHandler_)
                        ptunnel->RspOfReqForQuoteHandler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kInsertQuoteRsp:
                {
                    T_InsertQuoteRespond *p = (T_InsertQuoteRespond *) v.second;
                    if (ptunnel->InsertQuoteRespondHandler_)
                        ptunnel->InsertQuoteRespondHandler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kCancelQuoteRsp:
                {
                    T_CancelQuoteRespond *p = (T_CancelQuoteRespond *) v.second;
                    if (ptunnel->CancelQuoteRespondHandler_)
                        ptunnel->CancelQuoteRespondHandler_(p);
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
void KgiFixTunnel::SendRespondAsync(int rsp_type, void *rsp)
{
    MYFixTrader * p_esunny = static_cast<MYFixTrader *>(trade_inf_);
    if (!p_esunny)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_esunny->rsp_sync;
    std::condition_variable &rsp_con = p_esunny->rsp_con;

    {
        std::unique_lock<std::mutex> lock(rsp_sync);
        pending_rsp_.push_back(std::make_pair(rsp_type, rsp));
    }
    rsp_con.notify_one();
}

KgiFixTunnel::KgiFixTunnel(const std::string &provider_config_file)
{
    trade_inf_ = NULL;
    exchange_code_ = ' ';
    InitOnce();

    // load config file
    cfg_file = "fix8_settings.xml";
    if (!provider_config_file.empty())
    {
        cfg_file = provider_config_file;
    }

    TNL_LOG_INFO("create KgiFixTunnel object with configure file: %s",
        cfg_file.c_str());

    //TunnelConfigData cfg;
    lib_cfg_ = new TunnelConfigData();
    lib_cfg_->Load(cfg_file);
    exchange_code_ = lib_cfg_->Logon_config().exch_code.c_str()[0];
    tunnel_info_.account = lib_cfg_->Logon_config().investorid;

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "kgi_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;
    update_state(tunnel_info_.qtm_name.c_str(), TYPE_TCA, QtmState::INIT, GetDescriptionWithState(QtmState::INIT).c_str());
    TNL_LOG_INFO("update_state: name: %s, State: %d, Description: %s.", tunnel_info_.qtm_name.c_str(), QtmState::INIT,
        GetDescriptionWithState(QtmState::INIT).c_str());

    // start log output thread
    LogUtil::Start("my_tunnel_lib_kgi_fix",
        lib_cfg_->App_cfg().share_memory_key);

    int provider_type = lib_cfg_->App_cfg().provider_type;
    if (provider_type == TunnelProviderType::PROVIDERTYPE_FIX_KGI)
    {
        InitInf(*lib_cfg_);
    }
    else
    {
        TNL_LOG_ERROR(
            "not support tunnel provider type, please check config file.");
    }

    // init respond thread
    std::thread rsp_thread(&KgiFixTunnel::RespondHandleThread, this);
    rsp_thread.detach();
}

std::string KgiFixTunnel::GetClientID()
{
    return tunnel_info_.account;
}

bool KgiFixTunnel::InitInf(const TunnelConfigData &cfg)
{
    // 连接服务
    TNL_LOG_INFO("prepare to start KGI_FIX tunnel server.");

    const ComplianceCheckParam &param = cfg.Compliance_check_param();
    ComplianceCheck_Init(tunnel_info_.account.c_str(), param.cancel_warn_threshold,
        param.cancel_upper_limit, param.max_open_of_speculate,
        param.max_open_of_arbitrage, param.max_open_of_total,
        param.switch_mask.c_str());

    char init_msg[127];
    sprintf(init_msg, "%s: Init compliance check", tunnel_info_.account.c_str());
    update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::OPEN_ORDER_LIMIT, QtmComplianceState::INIT_COMPLIANCE_CHECK, init_msg);
    update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::INIT_COMPLIANCE_CHECK, init_msg);

    std::string xml_file = "./fix8_settings.xml";
    trade_inf_ = new MYFixTrader(xml_file, tunnel_info_);

    return true;
}

void KgiFixTunnel::PlaceOrder(const T_PlaceOrder *p)
{
    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnPlaceOrder(p, tunnel_info_);
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);

    if (p_tunnel)
    {
        OrderRefDataType return_param;
        //cout << "ComplianceCheck_TryReqOrderInsert  SerialNo: " << p->serial_no << " User: " << user_id_.c_str() << endl;
        process_result = ComplianceCheck_TryReqOrderInsert(tunnel_info_.account.c_str(),
            p->serial_no, exchange_code_, p->stock_code, p->volume,
            p->limit_price, p->order_kind, p->speculator, p->direction,
            p->open_close, p->order_type, &return_param);

        if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS &&
            process_result != TUNNEL_ERR_CODE::OPEN_EQUAL_LIMIT &&
            process_result != TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
        {
            // 日志
            if (process_result == TUNNEL_ERR_CODE::CFFEX_EXCEED_LIMIT)
            {
                TNL_LOG_WARN("forbid open because current open volumn: %lld",
                    return_param);
            }
            else if (process_result == TUNNEL_ERR_CODE::POSSIBLE_SELF_TRADE)
            {
                TNL_LOG_WARN("possible trade with order: %lld (order ref)",
                    return_param);
            }
            else if (process_result
                == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
            {
                TNL_LOG_WARN(
                    "reach the warn threthold of cancel time, forbit open new position.");
            }
        }
        else
        {
            // 下单
            if (process_result == TUNNEL_ERR_CODE::OPEN_EQUAL_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                    QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS,
                    GetComplianceDescriptionWithState(QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS, tunnel_info_.account.c_str(), p->stock_code).c_str());
                TNL_LOG_WARN("equal the warn threthold of open position.");
            }
            else if (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                    QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD,
                    GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                        p->stock_code).c_str());
                TNL_LOG_WARN("equal the warn threthold of cancel time.");
            }

            process_result = p_tunnel->ReqOrderInsert(p);

            // 发送失败，即时回滚
            if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                //cout << "ComplianceCheck_OnOrderInsertFailed SerialNo: " << p->serial_no << " User: " << user_id_.c_str() << endl;
                ComplianceCheck_OnOrderInsertFailed(tunnel_info_.account.c_str(),
                    p->serial_no, exchange_code_, p->stock_code, p->volume,
                    p->speculator, p->open_close, p->order_type);
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
        T_OrderRespond *rsp = new T_OrderRespond();
        memset(rsp, 0, sizeof(T_OrderRespond));
        rsp->serial_no = p->serial_no;
        rsp->error_no = process_result;
        rsp->entrust_no = 0;
        rsp->entrust_status = MY_TNL_OS_ERROR;
        LogUtil::OnOrderRespond(rsp, tunnel_info_);

        SendRespondAsync(RespondDataType::kPlaceOrderRsp, rsp);
    }
}

void KgiFixTunnel::CancelOrder(const T_CancelOrder *p)
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnCancelOrder(p, tunnel_info_);

    if (p_tunnel)
    {
        //cout << "ComplianceCheck_TryReqOrderAction SerialNo: " << p->serial_no << " User: " << user_id_.c_str() << endl;
        process_result = ComplianceCheck_TryReqOrderAction(tunnel_info_.account.c_str(),
            p->stock_code, p->org_serial_no);

        if (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
        {
            update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD,
                GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                    p->stock_code).c_str());
            TNL_LOG_WARN("cancel time approaches threshold");
        }
        if (process_result == TUNNEL_ERR_CODE::CANCEL_REACH_LIMIT)
        {
            update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                QtmComplianceState::CANCEL_TIME_OVER_MAXIMUN,
                GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_MAXIMUN, tunnel_info_.account.c_str(),
                    p->stock_code).c_str());
            TNL_LOG_WARN("cancel time reach limitation");
        }
        if (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
        {
            update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD,
                GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD, tunnel_info_.account.c_str(), p->stock_code).c_str());
            TNL_LOG_WARN("cancel time equal threshold");
        }

        if ((process_result == TUNNEL_ERR_CODE::RESULT_SUCCESS)
            || (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
            || (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT))
        {
            // cancel order
            process_result = p_tunnel->ReqOrderAction(p);

            // hanle fail event
            if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                //cout << " ComplianceCheck_OnOrderCancelFailed SerialNo: " << p->serial_no << " User: " << user_id_.c_str() << endl;
                ComplianceCheck_OnOrderCancelFailed(tunnel_info_.account.c_str(),
                    p->stock_code, p->org_serial_no);
            }
            else
            {
                //cout << "ComplianceCheck_OnOrderPendingCancel SerialNo: " << p->serial_no << " User: " << user_id_.c_str() << endl;
                ComplianceCheck_OnOrderPendingCancel(tunnel_info_.account.c_str(),
                    p->org_serial_no);
            }
        }
    }
    else
    {
        TNL_LOG_ERROR(
            "not support tunnel when cancel order, please check configure file");
    }

    if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS
        && process_result != TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD
        && process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
    {
        // 应答包
        T_CancelRespond *cancle_order = new T_CancelRespond();
        memset(cancle_order, 0, sizeof(T_CancelRespond));
        cancle_order->serial_no = p->serial_no;
        cancle_order->error_no = process_result;
        cancle_order->entrust_no = 0L;

        // 需要回报撤单状态，成功为已报，失败为拒绝
        cancle_order->entrust_status = MY_TNL_OS_REPORDED;
        if (process_result != 0)
        {
            cancle_order->entrust_status = MY_TNL_OS_ERROR;
        }

        LogUtil::OnCancelRespond(cancle_order, tunnel_info_);

        SendRespondAsync(RespondDataType::kCancelOrderRsp, cancle_order);
    }
}

void KgiFixTunnel::QueryPosition(const T_QryPosition *pQryPosition)
{
    LogUtil::OnQryPosition(pQryPosition, tunnel_info_);
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    T_PositionReturn ret;
    memset(&ret, 0, sizeof(ret));
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when query position, please check configure file");

        ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        if (QryPosReturnHandler_)
        {
            QryPosReturnHandler_(&ret);
        }
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryPosition(ret);
    TNL_LOG_DEBUG("QueryPosition qry_result: %d, ret error_no: %d.", qry_result, ret.error_no);
    if (qry_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        ret.error_no = qry_result;
        if (QryPosReturnHandler_)
            QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}
void KgiFixTunnel::QueryOrderDetail(const T_QryOrderDetail *pQryParam)
{
    LogUtil::OnQryOrderDetail(pQryParam, tunnel_info_);
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    T_OrderDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when query order detail, please check configure file");

        ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        if (QryOrderDetailReturnHandler_)
            QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryOrderDetail(pQryParam);
    if (qry_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        ret.error_no = qry_result;
        if (QryOrderDetailReturnHandler_)
            QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}
void KgiFixTunnel::QueryTradeDetail(const T_QryTradeDetail *pQryParam)
{
    LogUtil::OnQryTradeDetail(pQryParam, tunnel_info_);
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    T_TradeDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when query trade detail, please check configure file");

        ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        if (QryTradeDetailReturnHandler_)
            QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryTradeDetail(pQryParam);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryTradeDetailReturnHandler_)
            QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

void KgiFixTunnel::QueryContractInfo(const T_QryContractInfo *p)
{
    LogUtil::OnQryContractInfo(p, tunnel_info_);
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    T_ContractInfoReturn ret;
    memset(&ret, 0, sizeof(ret));
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when query trade detail, please check configure file");

        LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryContractInfo(ret);
    if (qry_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        if (QryContractInfoHandler_)
        {
            QryContractInfoHandler_(&ret);
        }
        LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
    }
}

void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_OrderRespond *)> callback_handler)
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    OrderRespond_call_back_handler_ = callback_handler;
}

void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_CancelRespond *)> callback_handler)
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    CancelRespond_call_back_handler_ = callback_handler;
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_OrderReturn *)> callback_handler)
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_TradeReturn *)> callback_handler)
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_PositionReturn *)> handler)
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryPosReturnHandler_ = handler;
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_OrderDetailReturn *)> handler)
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryOrderDetailReturnHandler_ = handler;
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_TradeDetailReturn *)> handler)
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR(
            "not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryTradeDetailReturnHandler_ = handler;
}

void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_ContractInfoReturn *)> handler)
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init when SetCallbackHandler");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryContractInfoHandler_ = handler;
}

// 新增做市接口, not support now 20151127
// 询价
void KgiFixTunnel::ReqForQuoteInsert(const T_ReqForQuote *p)
{
    LogUtil::OnReqForQuote(p, tunnel_info_);
}
///报价录入请求
void KgiFixTunnel::ReqQuoteInsert(const T_InsertQuote *p)
{
    LogUtil::OnInsertQuote(p, tunnel_info_);
}
///报价操作请求
void KgiFixTunnel::ReqQuoteAction(const T_CancelQuote *p)
{
    LogUtil::OnCancelQuote(p, tunnel_info_);
}

// 新增做市接口
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_RtnForQuote *)> handler)
{
    TNL_LOG_WARN(
        "tunnel not support function SetCallbackHandler(T_RtnForQuote)");
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_RspOfReqForQuote *)> handler)
{
    TNL_LOG_WARN(
        "tunnel not support function SetCallbackHandler(T_RspOfReqForQuote)");
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_InsertQuoteRespond *)> handler)
{
    TNL_LOG_WARN(
        "tunnel not support function SetCallbackHandler(T_InsertQuoteRespond)");
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_CancelQuoteRespond *)> handler)
{
    TNL_LOG_WARN(
        "tunnel not support function SetCallbackHandler(T_CancelQuoteRespond)");
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_QuoteReturn *)> handler)
{
    TNL_LOG_WARN(
        "tunnel not support function SetCallbackHandler(T_QuoteReturn)");
}
void KgiFixTunnel::SetCallbackHandler(
    std::function<void(const T_QuoteTrade *)> handler)
{
    TNL_LOG_WARN(
        "tunnel not support function SetCallbackHandler(T_QuoteTrade)");
}

KgiFixTunnel::~KgiFixTunnel()
{
    MYFixTrader * p_tunnel = static_cast<MYFixTrader *>(trade_inf_);
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
    return new KgiFixTunnel(tunnel_config_file);
}
void DestroyTradeTunnel(MYTunnelInterface * p)
{
    if (p)
    {
        delete p;
    }
}
