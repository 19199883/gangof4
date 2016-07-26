﻿#include "ibapi_tunnel_lib.h"

#include <string>
#include <condition_variable>
#include <mutex>

#include "qtm_with_code.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "config_data.h"
#include "ib_api_context.h"
#include "trade_log_util.h"
#include "ib_api_trade_interface.h"
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
    kCancelOrderRsp,
    kInsertForQuoteRsp,
    kInsertQuoteRsp,
    kCancelQuoteRsp,
};

void IBAPITunnel::RespondHandleThread(IBAPITunnel *ptunnel)
{
    MYIBApiTunnel * p_tnl = static_cast<MYIBApiTunnel *>(ptunnel->trade_inf_);
    if (!p_tnl)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_tnl->rsp_sync;
    std::condition_variable &rsp_con = p_tnl->rsp_con;
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
                    if (ptunnel->OrderRespondHandler_) ptunnel->OrderRespondHandler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kCancelOrderRsp:
                    {
                    T_CancelRespond *p = (T_CancelRespond *) v.second;
                    if (ptunnel->CancelRespondHandler_) ptunnel->CancelRespondHandler_(p);
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
void IBAPITunnel::SendRespondAsync(int rsp_type, void *rsp)
{
    MYIBApiTunnel * p_tnl = static_cast<MYIBApiTunnel *>(trade_inf_);
    if (!p_tnl)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_tnl->rsp_sync;
    std::condition_variable &rsp_con = p_tnl->rsp_con;
    {
        std::unique_lock<std::mutex> lock(rsp_sync);
        pending_rsp_.push_back(std::make_pair(rsp_type, rsp));
    }
    rsp_con.notify_one();
}

IBAPITunnel::IBAPITunnel(const std::string &provider_config_file)
{
    trade_inf_ = NULL;
    exchange_code_ = ' ';
    InitOnce();

    std::string cfg_file("my_trade_tunnel.xml");
    if (!provider_config_file.empty())
    {
        cfg_file = provider_config_file;
    }

    TNL_LOG_INFO("create IBAPITunnel object with configure file: %s", cfg_file.c_str());

    //TunnelConfigData cfg;
    lib_cfg_ = new TunnelConfigData();
    lib_cfg_->Load(cfg_file);
    exchange_code_ = lib_cfg_->Logon_config().exch_code.c_str()[0];
    tunnel_info_.account = lib_cfg_->Logon_config().investorid;

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "ibapi_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::INIT);

    // init tunnel log object
    LogUtil::Start("my_tunnel_lib_ib", lib_cfg_->App_cfg().share_memory_key);

    int provider_type = lib_cfg_->App_cfg().provider_type;
    if (provider_type == TunnelProviderType::PROVIDERTYPE_IB_API)
    {
        InitInf(*lib_cfg_);
    }
    else
    {
        TNL_LOG_ERROR("not support tunnel provider type, please check config file.");
    }

    // init respond thread
    rsp_thread_ = new std::thread(&IBAPITunnel::RespondHandleThread, this);
}

std::string IBAPITunnel::GetClientID()
{
    return tunnel_info_.account;
}

bool IBAPITunnel::InitInf(const TunnelConfigData &cfg)
{
    // 连接服务
    TNL_LOG_INFO("prepare to start IB api tunnel.");

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
    trade_inf_ = new MYIBApiTunnel(cfg);
    return true;
}

void IBAPITunnel::PlaceOrder(const T_PlaceOrder *p)
{
    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnPlaceOrder(p, tunnel_info_);
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);

    OrderRefDataType return_param;
    if (p_tunnel)
    {
        process_result = ComplianceCheck_TryReqOrderInsert(
            tunnel_info_.account.c_str(),
            p->serial_no,
            exchange_code_,
            p->stock_code,
            p->volume,
            p->limit_price,
            p->order_kind,
            p->speculator,
            p->direction,
            p->open_close,
            p->order_type,
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
                        p->stock_code).c_str());
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
                        p->stock_code).c_str());
                TNL_LOG_WARN("reach the warn threthold of cancel time, forbit open new position.");
            }
            else if (process_result == TUNNEL_ERR_CODE::CANCEL_REACH_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::CANCEL_TIME_OVER_MAXIMUN,
                    GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_MAXIMUN, tunnel_info_.account.c_str(),
                        p->stock_code).c_str());
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
                        p->stock_code).c_str());
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

            // 下单
            // 发送失败，即时回滚
            if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    p->serial_no,
                    exchange_code_,
                    p->stock_code,
                    p->volume,
                    p->speculator,
                    p->open_close,
                    p->order_type);
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
        T_OrderRespond *p_rsp = new T_OrderRespond();
        memset(p_rsp, 0, sizeof(T_OrderRespond));
        p_rsp->entrust_no = 0;
        p_rsp->entrust_status = MY_TNL_OS_UNREPORT;
        p_rsp->serial_no = p->serial_no;
        p_rsp->error_no = process_result;

        LogUtil::OnOrderRespond(p_rsp, tunnel_info_);

        SendRespondAsync(RespondDataType::kPlaceOrderRsp, p_rsp);
    }
    else    //请求成功也即时回报
    {
        TNL_LOG_WARN("place order failed, result=%d", process_result);

        // 应答包
        T_OrderRespond *p_rsp = new T_OrderRespond();
        memset(p_rsp, 0, sizeof(T_OrderRespond));
        p_rsp->entrust_no = 0;
        p_rsp->entrust_status = MY_TNL_OS_REPORDED;
        p_rsp->serial_no = p->serial_no;
        p_rsp->error_no = process_result;

        LogUtil::OnOrderRespond(p_rsp, tunnel_info_);

        SendRespondAsync(RespondDataType::kPlaceOrderRsp, p_rsp);
    }
}

void IBAPITunnel::CancelOrder(const T_CancelOrder *p)
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnCancelOrder(p, tunnel_info_);

    if (p_tunnel)
    {
        process_result = ComplianceCheck_TryReqOrderAction(
            tunnel_info_.account.c_str(),
            p->stock_code,
            p->org_serial_no);

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
                GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                    p->stock_code).c_str());
            TNL_LOG_WARN("cancel time equal threshold");
        }

        if ((process_result == TUNNEL_ERR_CODE::RESULT_SUCCESS)
            || (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
            || (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT))
        {
            process_result = p_tunnel->ReqOrderAction(p);

            if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                ComplianceCheck_OnOrderCancelFailed(
                    tunnel_info_.account.c_str(),
                    p->stock_code,
                    p->org_serial_no);
            }
            else
            {
                ComplianceCheck_OnOrderPendingCancel(
                    tunnel_info_.account.c_str(),
                    p->org_serial_no);
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
        T_CancelRespond *cancel_respond = new T_CancelRespond();
        memset(cancel_respond, 0, sizeof(T_CancelRespond));
        cancel_respond->entrust_no = p->entrust_no;
        cancel_respond->serial_no = p->serial_no;
        cancel_respond->error_no = process_result;
        cancel_respond->entrust_status = MY_TNL_OS_ERROR;

        LogUtil::OnCancelRespond(cancel_respond, tunnel_info_);

        SendRespondAsync(RespondDataType::kCancelOrderRsp, cancel_respond);
    }
    else
    {
        // 应答包
        T_CancelRespond *cancel_respond = new T_CancelRespond();
        memset(cancel_respond, 0, sizeof(T_CancelRespond));
        cancel_respond->entrust_no = p->entrust_no;
        cancel_respond->serial_no = p->serial_no;
        cancel_respond->error_no = process_result;
        cancel_respond->entrust_status = MY_TNL_OS_WITHDRAWING;

        LogUtil::OnCancelRespond(cancel_respond, tunnel_info_);

        SendRespondAsync(RespondDataType::kCancelOrderRsp, cancel_respond);
    }
}

void IBAPITunnel::QueryPosition(const T_QryPosition *p)
{
    LogUtil::OnQryPosition(p, tunnel_info_);
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
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
void IBAPITunnel::QueryOrderDetail(const T_QryOrderDetail *p)
{
    LogUtil::OnQryOrderDetail(p, tunnel_info_);
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    T_OrderDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query order detail, please check configure file");

        ret.error_no = 2;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }
    // TODO don't need query order in IB, return success directly.
    int qry_result = 0; //p_tunnel->QryOrderDetail(p);
//    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}
void IBAPITunnel::QueryTradeDetail(const T_QryTradeDetail *p)
{
    LogUtil::OnQryTradeDetail(p, tunnel_info_);
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    T_TradeDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query trade detail, please check configure file");

        ret.error_no = 2;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryTradeDetail(p);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

void IBAPITunnel::QueryContractInfo(const T_QryContractInfo *p)
{
    LogUtil::OnQryContractInfo(p, tunnel_info_);
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    T_ContractInfoReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query contract info, please check configure file");

        ret.error_no = 2;
        if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);
        LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
        return;
    }

    int qry_result = p_tunnel->QryContractInfo(ret);
    ret.error_no = qry_result;
    if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);
    LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
}

// 询价
void IBAPITunnel::ReqForQuoteInsert(const T_ReqForQuote *p)
{
    LogUtil::OnReqForQuote(p, tunnel_info_);
}
///报价录入请求
void IBAPITunnel::ReqQuoteInsert(const T_InsertQuote *p)
{
    LogUtil::OnInsertQuote(p, tunnel_info_);
}
///报价操作请求
void IBAPITunnel::ReqQuoteAction(const T_CancelQuote *p)
{
    LogUtil::OnCancelQuote(p, tunnel_info_);
}

void IBAPITunnel::SetCallbackHandler(std::function<void(const T_OrderRespond *)> callback_handler)
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init, SetCallbackHandler failed!");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    OrderRespondHandler_ = callback_handler;
}

void IBAPITunnel::SetCallbackHandler(std::function<void(const T_CancelRespond *)> callback_handler)
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init, SetCallbackHandler failed!");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    CancelRespondHandler_ = callback_handler;
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_OrderReturn *)> callback_handler)
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init, SetCallbackHandler failed!");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_TradeReturn *)> callback_handler)
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init, SetCallbackHandler failed!");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init, SetCallbackHandler failed!");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryPosReturnHandler_ = handler;
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler)
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init, SetCallbackHandler failed!");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryOrderDetailReturnHandler_ = handler;
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init, SetCallbackHandler failed!");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryTradeDetailReturnHandler_ = handler;
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_ContractInfoReturn *)> handler)
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("tunnel not init, SetCallbackHandler failed!");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryContractInfoHandler_ = handler;
}

// added for market making
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_RtnForQuote *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_RtnForQuote)");
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_RspOfReqForQuote *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_RspOfReqForQuote)");
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_InsertQuoteRespond *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_InsertQuoteRespond)");
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_CancelQuoteRespond *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_CancelQuoteRespond)");
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_QuoteReturn *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_QuoteReturn)");
}
void IBAPITunnel::SetCallbackHandler(std::function<void(const T_QuoteTrade *)> handler)
{
    TNL_LOG_WARN("tunnel not support function SetCallbackHandler(T_QuoteTrade)");
}

IBAPITunnel::~IBAPITunnel()
{
    MYIBApiTunnel * p_tunnel = static_cast<MYIBApiTunnel *>(trade_inf_);
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
    return new IBAPITunnel(tunnel_config_file);
}
void DestroyTradeTunnel(MYTunnelInterface * p)
{
    if (p) delete p;
}
