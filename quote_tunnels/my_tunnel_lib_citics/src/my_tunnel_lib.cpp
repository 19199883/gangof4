
#include <string>
#include <boost/thread.hpp>
#include "boost/lexical_cast.hpp"

#include "qtm_with_code.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "check_schedule.h"
#include "config_data.h"
#include "trade_log_util.h"

#include "hs_trade_interface.h"
#include "hs_field_convert.h"
#include "my_protocol_packager.h"

#ifdef MY_LOG_DEBUG
#undef MY_LOG_DEBUG
#endif
#define MY_LOG_DEBUG(format, args...) my_cmn::my_log::instance(NULL)->log(my_cmn::LOG_MOD_QUOTE, my_cmn::LOG_PRI_DEBUG, format, ##args)

using namespace std;
using namespace my_cmn;

static void InitOnce()
{
    static volatile bool s_have_init = false;
    static boost::mutex s_init_sync;

    if (s_have_init)
    {
        return;
    }
    else
    {
        boost::mutex::scoped_lock lock(s_init_sync);
        if (s_have_init)
        {
            return;
        }
        std::string log_file_name = "my_tunnel_lib_" + my_cmn::GetCurrentDateTimeString();
        (void) my_log::instance(log_file_name.c_str());
        MY_LOG_INFO("start init tunnel library.");

        // initialize tunnel monitor
        qtm_init(TYPE_TCA);

        s_have_init = true;
    }
}

enum RespondDataType
{
    kPlaceOrderRsp = 1,
    kCancelOrderRsp,
    kQryPositionRsp,
    kQryTradeDetailRsp,
};

static boost::mutex rsp_sync;
static boost::condition_variable rsp_con;
void MYTunnel::RespondHandleThread(MYTunnel *ptunnel)
{
    std::vector<std::pair<int, void *> > rsp_tmp;

    while (true)
    {
        boost::unique_lock<boost::mutex> lock(rsp_sync);
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
                case RespondDataType::kQryPositionRsp:
                    {
                    T_PositionReturn *p = (T_PositionReturn *) v.second;
                    if (ptunnel->QryPosReturnHandler_) ptunnel->QryPosReturnHandler_(p);
                    delete p;
                }
                    break;
                case RespondDataType::kQryTradeDetailRsp:
                    {
                    T_TradeDetailReturn *p = (T_TradeDetailReturn *) v.second;
                    if (ptunnel->QryTradeDetailReturnHandler_) ptunnel->QryTradeDetailReturnHandler_(p);
                    delete p;
                }
                    break;
                default:
                    MY_LOG_ERROR("unknown type of respond message: %d", v.first);
                    break;
            }
        }

        rsp_tmp.clear();
    }
}

void MYTunnel::SendRespondAsync(int rsp_type, void *rsp)
{
    {
        boost::unique_lock<boost::mutex> lock(rsp_sync);
        pending_rsp_.push_back(std::make_pair(rsp_type, rsp));
    }
    rsp_con.notify_one();
}

MYTunnel::MYTunnel(const std::string &provider_config_file)
{
    trade_inf_ = NULL;
    InitOnce();

    MY_LOG_INFO("create MYTunnel object with configure file: %s", provider_config_file.c_str());

    //TunnelConfigData cfg;
    lib_cfg_ = new TunnelConfigData();
    lib_cfg_->Load(provider_config_file);

    exchange_code_ = lib_cfg_->Logon_config().exch_code.c_str()[0];
    user_id_ = lib_cfg_->Logon_config().clientid;
    tunnel_info_.account = lib_cfg_->Logon_config().investorid;

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "citics_hs_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::INIT);

    // init tunnel log object
    LogUtil::Start("my_tunnel_lib_citics", lib_cfg_->App_cfg().share_memory_key);

    int provider_type = lib_cfg_->App_cfg().provider_type;
    if (provider_type == TunnelProviderType::PROVIDERTYPE_CITICS_HS)
    {
        InitInf(*lib_cfg_);
    }
    else
    {
        MY_LOG_ERROR("not support tunnel provider type(%d), please check config file.", provider_type);
    }

    // init respond thread
    std::thread rsp_thread(&MYTunnel::RespondHandleThread, this);
    rsp_thread.detach();
}

bool MYTunnel::InitInf(const TunnelConfigData &cfg)
{
    // 连接服务
    MY_LOG_INFO("prepare to start citics tunnel server.");

    const ComplianceCheckParam &param = cfg.Compliance_check_param();
    ComplianceCheck_Init(
        tunnel_info_.account.c_str(),
        param.cancel_warn_threshold,
        param.cancel_upper_limit,
        param.max_open_of_speculate,
        param.max_open_of_arbitrage,
        param.max_open_of_total);

    char init_msg[127];
    sprintf(init_msg, "%s: Init compliance check", tunnel_info_.account.c_str());
    update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::OPEN_ORDER_LIMIT, QtmComplianceState::INIT_COMPLIANCE_CHECK, init_msg);
    update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::INIT_COMPLIANCE_CHECK, init_msg);
    trade_inf_ = new CiticsHsTradeInf(cfg, tunnel_info_);
    return true;
}

void MYTunnel::PlaceOrder(const T_PlaceOrder *pPlaceOrder)
{
    MY_LOG_DEBUG("MYExchange::PlaceOrder serial_no=%ld;code=%s;dir=%c;oc=%c;volume=%ld;price=%02f",
        pPlaceOrder->serial_no, pPlaceOrder->stock_code, pPlaceOrder->direction, pPlaceOrder->open_close, pPlaceOrder->volume,
        pPlaceOrder->limit_price);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnPlaceOrder(pPlaceOrder, tunnel_info_);
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    if (p_tunnel)
    {
        // first, place the order into local cache
        p_tunnel->hs_trade_context_.SaveOriginalRequestData(pPlaceOrder->serial_no, *pPlaceOrder);
        //	it's passed to check compliance, so send the order to counter system
        process_result = p_tunnel->PlaceOrder(pPlaceOrder);
    }
    else
    {
        MY_LOG_ERROR("not support tunnel, check configure file");
    }	// end if (p_tunnel){

    if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        MY_LOG_WARN("place order failed, result=%d", process_result);
    }
}

void MYTunnel::CancelOrder(const T_CancelOrder *pCancelOrder)
{
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);

    MY_LOG_DEBUG("MYExchange::CancelOrder serial_no=%ld;code=%s;entrust_no=%ld;org_serial_no=%ld",
        pCancelOrder->serial_no, pCancelOrder->stock_code, pCancelOrder->entrust_no, pCancelOrder->org_serial_no);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnCancelOrder(pCancelOrder, tunnel_info_);

    if (p_tunnel != NULL)
    {
        // first, place the order into local cache
        p_tunnel->hs_trade_context_.SaveCancelRequest(pCancelOrder->serial_no, *pCancelOrder);
        process_result = p_tunnel->CancelOrder(pCancelOrder);
    }
}

void MYTunnel::QueryPosition(const T_QryPosition *pQryPosition)
{
    LogUtil::OnQryPosition(pQryPosition, tunnel_info_);
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    T_PositionReturn *pret = new T_PositionReturn();
    if (p_tunnel == NULL)
    {
        MY_LOG_ERROR("not support tunnel when query position, please check configure file");

        pret->error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        LogUtil::OnPositionRtn(pret, tunnel_info_);
        SendRespondAsync(RespondDataType::kQryPositionRsp, pret);
        return;
    }
    else
    {
        int qry_result = p_tunnel->QueryPosition(pQryPosition);
        if (qry_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
        {
            pret->error_no = qry_result;
            LogUtil::OnPositionRtn(pret, tunnel_info_);
        }
    }
}

void MYTunnel::QueryTradeDetail(const T_QryTradeDetail *pQryParam)
{
    LogUtil::OnQryTradeDetail(pQryParam, tunnel_info_);
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    T_TradeDetailReturn *pret = new T_TradeDetailReturn();
    if (!p_tunnel)
    {
        MY_LOG_ERROR("not support tunnel when query trade detail, please check configure file");

        pret->error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        LogUtil::OnTradeDetailRtn(pret, tunnel_info_);
        SendRespondAsync(RespondDataType::kQryTradeDetailRsp, pret);
        return;
    }

    int qry_result = p_tunnel->QueryTradeDetail(pQryParam);
    if (qry_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        pret->error_no = qry_result;
        LogUtil::OnTradeDetailRtn(pret, tunnel_info_);
        SendRespondAsync(RespondDataType::kQryTradeDetailRsp, pret);
    }
}

void MYTunnel::SetCallbackHandler(std::function<void(const T_OrderRespond *)> callback_handler)
{
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    if (!p_tunnel)
    {
        MY_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    OrderRespond_call_back_handler_ = callback_handler;
}

void MYTunnel::SetCallbackHandler(std::function<void(const T_CancelRespond *)> callback_handler)
{
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    if (!p_tunnel)
    {
        MY_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    CancelRespond_call_back_handler_ = callback_handler;
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_OrderReturn *)> callback_handler)
{
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    if (!p_tunnel)
    {
        MY_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_TradeReturn *)> callback_handler)
{
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    if (!p_tunnel)
    {
        MY_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
{
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    if (!p_tunnel)
    {
        MY_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryPosReturnHandler_ = handler;
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler)
{
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    if (!p_tunnel)
    {
        MY_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }

    QryOrderDetailReturnHandler_ = NULL;
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
{
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    if (!p_tunnel)
    {
        MY_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryTradeDetailReturnHandler_ = handler;
}

MYTunnel::~MYTunnel()
{
    CiticsHsTradeInf * p_tunnel = static_cast<CiticsHsTradeInf *>(trade_inf_);
    if (p_tunnel)
    {
        delete p_tunnel;
        trade_inf_ = NULL;
    }
    qtm_finish();
}

//empty, normal ctp can't support quote now 20150313
// 询价
void MYTunnel::ReqForQuoteInsert(const T_ReqForQuote *p)
{
    // LogUtil::OnReqForQuote(p);
}
///报价录入请求
void MYTunnel::ReqQuoteInsert(const T_InsertQuote *p)
{
    // LogUtil::OnInsertQuote(p);
}
///报价操作请求
void MYTunnel::ReqQuoteAction(const T_CancelQuote *p)
{
    // LogUtil::OnCancelQuote(p);
}

// added for market making
void MYTunnel::SetCallbackHandler(std::function<void(const T_RtnForQuote *)> handler)
{
    //TNL_LOG_ERROR("not support tunnel when SetCallbackHandler(T_RtnForQuote), please check configure file");
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_RspOfReqForQuote *)> handler)
{
    //TNL_LOG_ERROR("not support tunnel when SetCallbackHandler(T_RspOfReqForQuote), please check configure file");
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_InsertQuoteRespond *)> handler)
{
    //TNL_LOG_ERROR("not support tunnel when SetCallbackHandler(T_InsertQuoteRespond), please check configure file");
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_CancelQuoteRespond *)> handler)
{
    // TNL_LOG_ERROR("not support tunnel when SetCallbackHandler(T_CancelQuoteRespond), please check configure file");
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_QuoteReturn *)> handler)
{
    //TNL_LOG_ERROR("not support tunnel when SetCallbackHandler(T_QuoteReturn), please check configure file");
}
void MYTunnel::SetCallbackHandler(std::function<void(const T_QuoteTrade *)> handler)
{
    //TNL_LOG_ERROR("not support tunnel when SetCallbackHandler(T_QuoteTrade), please check configure file");
}

std::string MYTunnel::GetClientID()
{
    return tunnel_info_.account;;
}

void MYTunnel::QueryContractInfo(const T_QryContractInfo *pQryParam)
{

}

void MYTunnel::SetCallbackHandler(std::function<void(const T_ContractInfoReturn *)> handler)
{

}

MYTunnelInterface *CreateTradeTunnel(const std::string &tunnel_config_file)
{
    return new MYTunnel(tunnel_config_file);
}

void DestroyTradeTunnel(MYTunnelInterface * p)
{
    if (p) delete p;
}
