#include "my_tunnel_lib.h"

#include <string>
#include <string.h>
#include <mutex>
#include <condition_variable>

#include "my_tunnel_lib.h"
#include "qtm_with_code.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "my_trade_tunnel_struct.h"

#include "config_data.h"
#include "trade_log_util.h"
#include "xspeed_trade_interface.h"
#include "field_convert.h"

#include "my_protocol_packager.h"
#include "check_schedule.h"

#include "time_probe.h"

using namespace std;
using namespace my_cmn;

//#define XSPEED_VER "xspeed_2.5.7_2015.07.28_00"
//#define XSPEED_VER "xspeed_2.5.7_2015.12.24_00"
#define XSPEED_VER "xspeed_2.5.7_2015.12.25_00"

#ifdef PROBE_COST
static int place_order_count = 1;
static int cancel_order_count = 2000;
#endif

std::atomic_int XspeedTunnel::counter_(0);
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
        std::string log_file_name = "my_tunnel_lib_" + my_cmn::GetCurrentDateTimeString();
        (void) my_log::instance(log_file_name.c_str());
        TNL_LOG_INFO("start init xspeed tunnel library. version = %s", XSPEED_VER);

        // initialize tunnel monitor
        qtm_init(TYPE_TCA);
        TIME_PROBER_INIT;
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
//static std::mutex rsp_sync;
//static std::condition_variable rsp_con;
void XspeedTunnel::RespondHandleThread(XspeedTunnel *ptunnel)
{
    MYXSpeedSpi * p_xspeed = static_cast<MYXSpeedSpi *>(ptunnel->trade_inf_);
    if (!p_xspeed)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        exit(0);
    }

    std::mutex &rsp_sync = p_xspeed->rsp_sync;
    std::condition_variable &rsp_con = p_xspeed->rsp_con;
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
            TNL_LOG_DEBUG("RespondHandleThread:%d", v.first);
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
void XspeedTunnel::SendRespondAsync(int rsp_type, void *rsp)
{
    MYXSpeedSpi * p_xspeed = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_xspeed)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        exit(0);
    }
    std::mutex &rsp_sync = p_xspeed->rsp_sync;
    std::condition_variable &rsp_con = p_xspeed->rsp_con;

    {
        std::unique_lock<std::mutex> lock(rsp_sync);
        pending_rsp_.push_back(std::make_pair(rsp_type, rsp));
    }
    rsp_con.notify_one();
}

std::string XspeedTunnel::GetClientID()
{
    return tunnel_info_.account;
}

XspeedTunnel::XspeedTunnel(const std::string &provider_config_file)
{
    trade_inf_ = NULL;
    exchange_code_ = ' ';
    InitOnce();

    std::string cfg_file("my_trade_tunnel.xml");
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

    counter_++;
    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, strlen(qtm_tmp_name));
    int tmp_int  = counter_.load();
    sprintf(qtm_tmp_name, "xspeed_fut_%s_%u_%d", tunnel_info_.account.c_str(), getpid(), tmp_int);
    tunnel_info_.qtm_name = qtm_tmp_name;
    update_state(tunnel_info_.qtm_name.c_str(), TYPE_TCA, QtmState::INIT, GetDescriptionWithState(QtmState::INIT).c_str());
    TNL_LOG_INFO("update_state: name: %s, State: %d, Description: %s.", tunnel_info_.qtm_name.c_str(), QtmState::INIT, GetDescriptionWithState(QtmState::INIT).c_str());

    // start tunnel log object
    LogUtil::Start("my_tunnel_lib_xspeed", lib_cfg_->App_cfg().share_memory_key);

    int provider_type = lib_cfg_->App_cfg().provider_type;
    if (provider_type == TunnelProviderType::PROVIDERTYPE_DCE)
    {
        InitInf(*lib_cfg_);
    }
    else
    {
        TNL_LOG_ERROR("not support tunnel provider type, please check config file.");
        exit(0);
    }

    // init respond thread
    std::thread rsp_thread(&XspeedTunnel::RespondHandleThread, this);
    rsp_thread.detach();
}

bool XspeedTunnel::InitInf(const TunnelConfigData &cfg)
{
    // 连接服务
    TNL_LOG_INFO("prepare to start XSpeed tunnel server.");

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
    trade_inf_ = new MYXSpeedSpi(cfg, tunnel_info_);
    return true;
}

void XspeedTunnel::PlaceOrder(const T_PlaceOrder *pPlaceOrder)
{
#ifdef PROBE_COST
    char tag_name[32];
    memset(tag_name, 0, sizeof(tag_name));
    sprintf(tag_name, "PlaceOrder%d", place_order_count);
    TIME_PROBER_SET_TAG_NAME(place_order_count*10, tag_name);
    TIME_PROBER_BEGIN(place_order_count*10);
    sprintf(tag_name, "PLogUtil%d", place_order_count);
    TIME_PROBER_SET_TAG_NAME(place_order_count*10+1, tag_name);
    TIME_PROBER_BEGIN(place_order_count*10+1);
#endif
    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnPlaceOrder(pPlaceOrder, tunnel_info_);
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);

    OrderRefDataType return_param;
    if (p_tunnel)
    {
        // 获取OrderRef和往柜台报单，需要串行化处理，以免先获取OrderRef的线程，后报单到柜台，造成报单重复错误
        std::lock_guard<std::mutex> lock(p_tunnel->client_sync);

        OrderRefDataType order_ref = p_tunnel->xspeed_trade_context_.GetNewOrderRef();
        TNL_LOG_INFO("serial_no: %ld map order_ref: %lld", pPlaceOrder->serial_no, order_ref);
#ifdef PROBE_COST
    TIME_PROBER_END(place_order_count*10+1);
    sprintf(tag_name, "PComplianceCheck%d", place_order_count);
    TIME_PROBER_SET_TAG_NAME(place_order_count*10+2, tag_name);
    TIME_PROBER_BEGIN(place_order_count*10+2);
#endif
        //struct timespec time_start={0, 0},time_end={0, 0};
        //clock_gettime(CLOCK_REALTIME, &time_start);
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
        //clock_gettime(CLOCK_REALTIME, &time_end);
       // printf("\ntotal: %lluns\n\n", time_end.tv_nsec-time_start.tv_nsec);
#ifdef PROBE_COST
    TIME_PROBER_END(place_order_count*10+2);
    sprintf(tag_name, "PComplianceResultOutput%d", place_order_count);
    TIME_PROBER_SET_TAG_NAME(place_order_count*10+3, tag_name);
    TIME_PROBER_BEGIN(place_order_count*10+3);
#endif

        if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS &&
        	 process_result != TUNNEL_ERR_CODE::OPEN_EQUAL_LIMIT &&
			 process_result != TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
        {
            // 日志
            if (process_result == TUNNEL_ERR_CODE::CFFEX_EXCEED_LIMIT)
            {
            	update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::OPEN_ORDER_LIMIT, QtmComplianceState::OPEN_POSITIONS_EXCEED_LIMITS,
            			GetComplianceDescriptionWithState(QtmComplianceState::OPEN_POSITIONS_EXCEED_LIMITS, tunnel_info_.account.c_str(), pPlaceOrder->stock_code).c_str());
                TNL_LOG_WARN("forbid open because current open volumn: %lld", return_param);
            }
            else if (process_result == TUNNEL_ERR_CODE::POSSIBLE_SELF_TRADE)
            {
                TNL_LOG_WARN("possible trade with order: %lld (order ref)", return_param);
            }
            else if (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
            {
            	update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD,
            			GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD, tunnel_info_.account.c_str(), pPlaceOrder->stock_code).c_str());
                TNL_LOG_WARN("reach the warn threthold of cancel time, forbit open new position.");
            }

        }
        else
        {
        	if (process_result == TUNNEL_ERR_CODE::OPEN_EQUAL_LIMIT) {
        		update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS,
        		            			GetComplianceDescriptionWithState(QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS, tunnel_info_.account.c_str(), pPlaceOrder->stock_code).c_str());
        		TNL_LOG_WARN("equal the warn threthold of open position.");
        	} else if (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT) {
        		update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD,
        		            			GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD, tunnel_info_.account.c_str(), pPlaceOrder->stock_code).c_str());
        		TNL_LOG_WARN("equal the warn threthold of cancel time.");
        	}
#ifdef PROBE_COST
    TIME_PROBER_END(place_order_count*10+3);
    sprintf(tag_name, "PInputOrder%d", place_order_count);
    TIME_PROBER_SET_TAG_NAME(place_order_count*10+4, tag_name);
    TIME_PROBER_BEGIN(place_order_count*10+4);
#endif
            DFITCInsertOrderField counter_req;
            XSpeedPacker::OrderRequest(*lib_cfg_, pPlaceOrder, order_ref, counter_req);
            p_tunnel->xspeed_trade_context_.SaveOrderInfo(order_ref, XSpeedOrderInfo(exchange_code_, pPlaceOrder));
#ifdef PROBE_COST
    TIME_PROBER_END(place_order_count*10+4);
    sprintf(tag_name, "PSendOrder%d", place_order_count);
    TIME_PROBER_SET_TAG_NAME(place_order_count*10+5, tag_name);
    TIME_PROBER_BEGIN(place_order_count*10+5);
#endif
            process_result = p_tunnel->ReqOrderInsert(&counter_req);
#ifdef PROBE_COST
    TIME_PROBER_END(place_order_count*10+5);
    sprintf(tag_name, "PRespondProcess%d", place_order_count);
    TIME_PROBER_SET_TAG_NAME(place_order_count*10+6, tag_name);
    TIME_PROBER_BEGIN(place_order_count*10+6);
#endif
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
        XSpeedPacker::OrderRespond(process_result, pPlaceOrder->serial_no, 0, MY_TNL_OS_ERROR, *pOrderRespond);

        LogUtil::OnOrderRespond(pOrderRespond, tunnel_info_);

        SendRespondAsync(RespondDataType::kPlaceOrderRsp, pOrderRespond);
    }
#ifdef PROBE_COST
    TIME_PROBER_END(place_order_count*10+6);
    TIME_PROBER_END(place_order_count*10);
    place_order_count++;
#endif
}

void XspeedTunnel::CancelOrder(const T_CancelOrder *pCancelOrder)
{
#ifdef PROBE_COST
    char tag_name[32];
    memset(tag_name, 0, sizeof(tag_name));
    sprintf(tag_name, "CancelOrder%d", cancel_order_count-1000);
    TIME_PROBER_SET_TAG_NAME(cancel_order_count*10, tag_name);
    TIME_PROBER_BEGIN(cancel_order_count*10);
    sprintf(tag_name, "CLogUtil%d", cancel_order_count-1000);
    TIME_PROBER_SET_TAG_NAME(cancel_order_count*10+1, tag_name);
    TIME_PROBER_BEGIN(cancel_order_count*10+1);
#endif

    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnCancelOrder(pCancelOrder, tunnel_info_);
#ifdef PROBE_COST
    TIME_PROBER_END(cancel_order_count*10+1);
    sprintf(tag_name, "CComplianceCheck%d", cancel_order_count);
    TIME_PROBER_SET_TAG_NAME(cancel_order_count*10+2, tag_name);
    TIME_PROBER_BEGIN(cancel_order_count*10+2);
#endif
    if (p_tunnel)
    {
        OrderRefDataType org_order_ref = p_tunnel->xspeed_trade_context_.GetOrderRefBySerialNo(pCancelOrder->org_serial_no);

        process_result = ComplianceCheck_TryReqOrderAction(
            tunnel_info_.account.c_str(),
            pCancelOrder->stock_code,
			org_order_ref);
#ifdef PROBE_COST
    TIME_PROBER_END(cancel_order_count*10+2);
    sprintf(tag_name, "CComplianceResultOutput%d", cancel_order_count);
    TIME_PROBER_SET_TAG_NAME(cancel_order_count*10+3, tag_name);
    TIME_PROBER_BEGIN(cancel_order_count*10+3);
#endif
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
    		update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT, QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD,
    		            			GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD, tunnel_info_.account.c_str(), pCancelOrder->stock_code).c_str());
        	TNL_LOG_WARN("cancel time equal threshold");
        }
#ifdef PROBE_COST
    TIME_PROBER_END(cancel_order_count*10+3);
    sprintf(tag_name, "CInputOrder%d", cancel_order_count);
    TIME_PROBER_SET_TAG_NAME(cancel_order_count*10+4, tag_name);
    TIME_PROBER_BEGIN(cancel_order_count*10+4);
#endif
        if ((process_result == TUNNEL_ERR_CODE::RESULT_SUCCESS)
            || (process_result == TUNNEL_ERR_CODE::CANCEL_TIMES_REACH_WARN_THRETHOLD)
			|| (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT))
        {

            DFITCCancelOrderField counter_req;
            XSpeedPacker::CancelRequest(*lib_cfg_, pCancelOrder, org_order_ref, counter_req);
            p_tunnel->xspeed_trade_context_.UpdateCancelInfoOfOrderRef(org_order_ref, pCancelOrder->serial_no);
#ifdef PROBE_COST
    TIME_PROBER_END(cancel_order_count*10+4);
    sprintf(tag_name, "CSendOrder%d", cancel_order_count);
    TIME_PROBER_SET_TAG_NAME(cancel_order_count*10+5, tag_name);
    TIME_PROBER_BEGIN(cancel_order_count*10+5);
#endif
            process_result = p_tunnel->ReqOrderAction(&counter_req);
#ifdef PROBE_COST
    TIME_PROBER_END(cancel_order_count*10+5);
    sprintf(tag_name, "CRespondProcess%d", cancel_order_count);
    TIME_PROBER_SET_TAG_NAME(cancel_order_count*10+6, tag_name);
    TIME_PROBER_BEGIN(cancel_order_count*10+6);
#endif
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
        XSpeedPacker::CancelRespond(process_result, pCancelOrder->serial_no, 0, *cancle_order);
        LogUtil::OnCancelRespond(cancle_order, tunnel_info_);

        SendRespondAsync(RespondDataType::kCancelOrderRsp, cancle_order);
    }
#ifdef PROBE_COST
    TIME_PROBER_END(cancel_order_count*10+6);
    TIME_PROBER_END(cancel_order_count*10);
    cancel_order_count++;
#endif
}

void XspeedTunnel::QueryPosition(const T_QryPosition *pQryPosition)
{
    LogUtil::OnQryPosition(pQryPosition, tunnel_info_);
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    T_PositionReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query position, please check configure file");

        ret.error_no = 2;
        if (QryPosReturnHandler_) QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    DFITCPositionDetailField qry_param;
    memset(&qry_param, 0, sizeof(DFITCPositionField));
    strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));

    int qry_result = p_tunnel->QryPosition(&qry_param);

    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryPosReturnHandler_) QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}
void XspeedTunnel::QueryOrderDetail(const T_QryOrderDetail *pQryParam)
{
    LogUtil::OnQryOrderDetail(pQryParam, tunnel_info_);
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    T_OrderDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query order detail, please check configure file");

        ret.error_no = 2;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }

    DFITCOrderField qry_param;
    memset(&qry_param, 0, sizeof(DFITCOrderField));
    strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));

    int qry_result = p_tunnel->QryOrderDetail(&qry_param);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}
void XspeedTunnel::QueryTradeDetail(const T_QryTradeDetail *pQryParam)
{
    LogUtil::OnQryTradeDetail(pQryParam, tunnel_info_);
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    T_TradeDetailReturn ret;
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query trade detail, please check configure file");

        ret.error_no = 2;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    DFITCMatchField qry_param;
    memset(&qry_param, 0, sizeof(DFITCMatchField));
    strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));

    int qry_result = p_tunnel->QryTradeDetail(&qry_param);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}
void XspeedTunnel::QueryContractInfo(const T_QryContractInfo *p)
{
    LogUtil::OnQryContractInfo(p, tunnel_info_);

    T_ContractInfoReturn ret;
    // TODO for compatible ctp tunnel, return success
    ret.error_no = TUNNEL_ERR_CODE::RESULT_SUCCESS;
    if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);
    LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
}

//added for mm, on 20141218
void XspeedTunnel::ReqForQuoteInsert(const T_ReqForQuote *p)
{
    LogUtil::OnReqForQuote(p, tunnel_info_);
    //xspeed期货接口不支持本接口，向上层返回不支持的操作类型;


    int process_result = TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION;
        

    // 应答包
    T_RspOfReqForQuote *rsp = new T_RspOfReqForQuote();
    rsp->serial_no = p->serial_no;
    rsp->error_no = process_result;
    LogUtil::OnRspOfReqForQuote(rsp, tunnel_info_);

    SendRespondAsync(RespondDataType::kInsertForQuoteRsp, rsp);
	TNL_LOG_ERROR("not support ReqForQuoteInsert.");
}

///报价录入请求
void XspeedTunnel::ReqQuoteInsert(const T_InsertQuote *p)
{
    LogUtil::OnInsertQuote(p, tunnel_info_);
    //xspeed期货接口不支持本接口，向上层返回不支持的操作类型;	

    int process_result = TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION;
        

    // 应答包
    T_InsertQuoteRespond *rsp = new T_InsertQuoteRespond();
    XSpeedPacker::QuoteRespond(process_result, p->serial_no, 0, process_result, *rsp);

    LogUtil::OnInsertQuoteRespond(rsp, tunnel_info_);

    SendRespondAsync(RespondDataType::kInsertQuoteRsp, rsp);
    TNL_LOG_ERROR("not support ReqQuoteInsert.");
}
///报价操作请求
void XspeedTunnel::ReqQuoteAction(const T_CancelQuote *p)
{
    LogUtil::OnCancelQuote(p, tunnel_info_);
    //xspeed期货接口不支持本接口，向上层返回不支持的操作类型;

    // 应答包
    int process_result = TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION;
    T_CancelQuoteRespond *cancel_quote_rsp = new T_CancelQuoteRespond();
    XSpeedPacker::CancelQuoteRespond(process_result, p->serial_no, p->entrust_no, *cancel_quote_rsp);

    LogUtil::OnCancelQuoteRespond(cancel_quote_rsp, tunnel_info_);

    SendRespondAsync(RespondDataType::kCancelQuoteRsp, cancel_quote_rsp);
    TNL_LOG_ERROR("not support ReqQuoteAction.");
}

void XspeedTunnel::SetCallbackHandler(std::function<void(const T_OrderRespond *)> callback_handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    OrderRespond_call_back_handler_ = callback_handler;
}

void XspeedTunnel::SetCallbackHandler(std::function<void(const T_CancelRespond *)> callback_handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
    CancelRespond_call_back_handler_ = callback_handler;
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_OrderReturn *)> callback_handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_TradeReturn *)> callback_handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryPosReturnHandler_ = handler;
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryOrderDetailReturnHandler_ = handler;
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryTradeDetailReturnHandler_ = handler;
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_ContractInfoReturn *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryContractInfoHandler_ = handler;
}


// added for market making
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_RtnForQuote *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);

    // TODO return of ForQuote, ctp will return from quote interface
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_RspOfReqForQuote *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    RspOfReqForQuoteHandler_ = handler;
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_InsertQuoteRespond *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    InsertQuoteRespondHandler_ = handler;
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_CancelQuoteRespond *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    CancelQuoteRespondHandler_ = handler;
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_QuoteReturn *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_QuoteTrade *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when SetCallbackHandler, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
}

XspeedTunnel::~XspeedTunnel()
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (p_tunnel)
    {
        delete p_tunnel;
        trade_inf_ = NULL;
    }
    TIME_PROBER_EXIT;
    qtm_finish();
    LogUtil::Stop();
}

MYTunnelInterface *CreateTradeTunnel(const std::string &tunnel_config_file)
{
    return new XspeedTunnel(tunnel_config_file);
}
void DestroyTradeTunnel(MYTunnelInterface * p)
{
    if (p) delete p;
}
