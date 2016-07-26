#include "my_tunnel_lib.h"

#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "qtm_with_code.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "config_data.h"
#include "trade_log_util.h"
#include "xspeed_trade_interface.h"
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

void XspeedTunnel::QueryPositionBeforeClose(const std::string &qry_time)
{
	time_t t;
	struct tm *cur_time;
	char second[3];
	char minute[3];
	char hour[3];
	strcpy(second, qry_time.substr(4, 2).c_str());
	strcpy(minute, qry_time.substr(2, 2).c_str());
	strcpy(hour, qry_time.substr(0, 2).c_str());
	struct tm localtime_result;
	while (1) {
		t = time(NULL);
		cur_time = localtime_r(&t, &localtime_result);
		if (cur_time->tm_sec == atoi(second) && cur_time->tm_min == atoi(minute) && cur_time->tm_hour == atoi(hour)) {
			MYXSpeedSpi * p_ctp = static_cast<MYXSpeedSpi *>(trade_inf_);
			p_ctp->StartPositionBackUp();
			sleep(5);
			break;
		} else {
			sleep(1);
		}
	}
}


void XspeedTunnel::RespondHandleThread(XspeedTunnel *ptunnel)
{
    MYXSpeedSpi * p_xspeed = static_cast<MYXSpeedSpi *>(ptunnel->trade_inf_);
    if (!p_xspeed)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_xspeed->rsp_sync;
    std::condition_variable &rsp_con = p_xspeed->rsp_con;

    std::vector<std::pair<int, void *> > rsp_tmp;

    while (true)
    {
        // swap responds synchronized
        {
            std::unique_lock<std::mutex> lock(rsp_sync);
            while (ptunnel->pending_rsp_.empty())
            {
                rsp_con.wait(lock);
            }

            ptunnel->pending_rsp_.swap(rsp_tmp);
        }

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
void XspeedTunnel::SendRespondAsync(int rsp_type, void *rsp)
{
    MYXSpeedSpi * p_xspeed = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_xspeed)
    {
        TNL_LOG_ERROR("tunnel not init in RespondHandleThread.");
        return;
    }

    std::mutex &rsp_sync = p_xspeed->rsp_sync;
    std::condition_variable &rsp_con = p_xspeed->rsp_con;

    {
        std::unique_lock<std::mutex> lock(rsp_sync);
        pending_rsp_.push_back(std::make_pair(rsp_type, rsp));
    }
    rsp_con.notify_one();
}

XspeedTunnel::XspeedTunnel(const std::string &provider_config_file)
{
    trade_inf_ = NULL;
    InitOnce();

    std::string cfg_file("my_trade_tunnel.xml");
    if (!provider_config_file.empty())
    {
        cfg_file = provider_config_file;
    }

    TNL_LOG_INFO("create XspeedTunnel object with configure file: %s", cfg_file.c_str());

    //TunnelConfigData cfg;
    lib_cfg_ = new TunnelConfigData();
    lib_cfg_->Load(cfg_file);
    tunnel_info_.account = lib_cfg_->Logon_config().investorid;

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "xspeed_mm_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::INIT);

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
    }

    query_pos_time_ = lib_cfg_->Initial_policy_param().time_to_query_pos;
    if (query_pos_time_.size() == 6) {
    	std::thread qry_pos(std::bind(&XspeedTunnel::QueryPositionBeforeClose, this, query_pos_time_));
    	qry_pos.detach();
    }

    // init respond thread
    std::thread rsp_thread(&XspeedTunnel::RespondHandleThread, this);
    rsp_thread.detach();
}

std::string XspeedTunnel::GetClientID()
{
    return tunnel_info_.account;
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
    trade_inf_ = new MYXSpeedSpi(cfg);

    return true;
}

void XspeedTunnel::PlaceOrder(const T_PlaceOrder *po)
{
    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnPlaceOrder(po, tunnel_info_);
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);

    OrderRefDataType return_param;
    if (p_tunnel)
    {
        // 获取OrderRef和往柜台报单，需要串行化处理，以免先获取OrderRef的线程，后报单到柜台，造成报单重复错误
        std::lock_guard<std::mutex> lock(p_tunnel->client_sync);

        OrderRefDataType order_ref = p_tunnel->xspeed_trade_context_.GetNewOrderRef();
        TNL_LOG_INFO("serial_no: %ld map order_ref: %lld", po->serial_no, order_ref);

        process_result = ComplianceCheck_TryReqOrderInsert(
            tunnel_info_.account.c_str(),
            order_ref,
            po->exchange_type,
            po->stock_code,
            po->volume,
            po->limit_price,
            po->order_kind,
            po->speculator,
            po->direction,
            po->open_close,
            po->order_type,
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
                        po->stock_code).c_str());
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
                        po->stock_code).c_str());
                TNL_LOG_WARN("reach the warn threthold of cancel time, forbit open new position.");
            }
        }
        else
        {
            if (process_result == TUNNEL_ERR_CODE::OPEN_EQUAL_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                    QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS,
                    GetComplianceDescriptionWithState(QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS, tunnel_info_.account.c_str(),
                        po->stock_code).c_str());
                TNL_LOG_WARN("equal the warn threthold of open position.");
            }
            else if (process_result == TUNNEL_ERR_CODE::CANCEL_EQUAL_LIMIT)
            {
                update_compliance(tunnel_info_.qtm_name.c_str(), tag_compl_type_enum::CANCEL_ORDER_LIMIT,
                    QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD,
                    GetComplianceDescriptionWithState(QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD, tunnel_info_.account.c_str(),
                        po->stock_code).c_str());
                TNL_LOG_WARN("equal the warn threthold of cancel time.");
            }
            DFITCInsertOrderField counter_req;
            XSpeedPacker::OrderRequest(*lib_cfg_, po, order_ref, counter_req);
            p_tunnel->xspeed_trade_context_.SaveOrderInfo(order_ref, XSpeedOrderInfo(po->exchange_type, po));
            process_result = p_tunnel->ReqOrderInsert(&counter_req);

            // 下单
            // 发送失败，即时回滚
            if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    order_ref,
                    po->exchange_type,
                    po->stock_code,
                    po->volume,
                    po->speculator,
                    po->open_close,
                    po->order_type);
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
        XSpeedPacker::OrderRespond(process_result, po->serial_no, 0, MY_TNL_OS_ERROR, *pOrderRespond);

        LogUtil::OnOrderRespond(pOrderRespond, tunnel_info_);

        SendRespondAsync(RespondDataType::kPlaceOrderRsp, pOrderRespond);
    }
}

void XspeedTunnel::CancelOrder(const T_CancelOrder *pCancelOrder)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    LogUtil::OnCancelOrder(pCancelOrder, tunnel_info_);

    if (p_tunnel)
    {
        OrderRefDataType org_order_ref = p_tunnel->xspeed_trade_context_.GetOrderRefBySerialNo(pCancelOrder->org_serial_no);

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
            DFITCCancelOrderField counter_req;
            XSpeedPacker::CancelRequest(*lib_cfg_, pCancelOrder, org_order_ref, counter_req);
            p_tunnel->xspeed_trade_context_.UpdateCancelInfoOfOrderRef(org_order_ref, pCancelOrder->serial_no);
            process_result = p_tunnel->ReqOrderAction(&counter_req);

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

    int qry_result = p_tunnel->QryPosition(pQryPosition);

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

    int qry_result = p_tunnel->QryOrderDetail(pQryParam);
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

    int qry_result = p_tunnel->QryTradeDetail(pQryParam);
    if (qry_result != 0)
    {
        ret.error_no = qry_result;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

void XspeedTunnel::QueryContractInfo(const T_QryContractInfo* pQryParam)
{
    LogUtil::OnQryContractInfo(pQryParam, tunnel_info_);
    T_ContractInfoReturn ret;
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("not support tunnel when query trade detail, please check configure file");

        ret.error_no = 2;
        if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);
        LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
        return;
    }

    // TODO only need for hedge stock strategy
    int qry_result = p_tunnel->QryContractInfo(pQryParam);
	if (qry_result != 0) {
		ret.error_no = qry_result;//TUNNEL_ERR_CODE::RESULT_SUCCESS;
		if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);
		LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
    }
}

//added for mm, on 20141218
void XspeedTunnel::ReqForQuoteInsert(const T_ReqForQuote *p)
{
    LogUtil::OnReqForQuote(p, tunnel_info_);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);

    if (p_tunnel)
    {
        DFITCForQuoteField input_for_quote;
        memset(&input_for_quote, 0, sizeof(input_for_quote));

        OrderRefDataType order_ref = p_tunnel->xspeed_trade_context_.GetNewOrderRef();
        input_for_quote.lRequestID = order_ref;
        strncpy(input_for_quote.accountID, tunnel_info_.account.c_str(), sizeof(input_for_quote.accountID));
        strncpy(input_for_quote.instrumentID, p->stock_code, sizeof(input_for_quote.instrumentID));

        process_result = p_tunnel->ReqForQuoteInsert(&input_for_quote);

        // insert quote success, save original request infomation
        if (process_result == TUNNEL_ERR_CODE::RESULT_SUCCESS)
        {
            p_tunnel->xspeed_trade_context_.SaveForquoteSerialNoOfOrderRef(order_ref, p->serial_no);
        }
    }
    else
    {
        TNL_LOG_ERROR("not support tunnel, check configure file");
    }

    // 请求失败后即时回报，否则，等柜台回报
    if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        TNL_LOG_WARN("insert failed, result=%d", process_result);

        // 应答包
        T_RspOfReqForQuote *rsp = new T_RspOfReqForQuote();
        rsp->serial_no = p->serial_no;
        rsp->error_no = process_result;
        LogUtil::OnRspOfReqForQuote(rsp, tunnel_info_);

        SendRespondAsync(RespondDataType::kInsertForQuoteRsp, rsp);
    }
}

///报价录入请求
void XspeedTunnel::ReqQuoteInsert(const T_InsertQuote *p)
{
    LogUtil::OnInsertQuote(p, tunnel_info_);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);

    if (p_tunnel)
    {
        // 获取OrderRef和往柜台报单，需要串行化处理，以免先获取OrderRef的线程，后报单到柜台，造成报单重复错误
        std::lock_guard<std::mutex> lock(p_tunnel->client_sync);

        OrderRefDataType quote_ref = p_tunnel->xspeed_trade_context_.GetNewOrderRefForInsertQuote();
        TNL_LOG_INFO("insert_quote serial_no: %ld map order_ref: %lld", p->serial_no, quote_ref);

        {
            DFITCQuoteInsertField insert_quote_req;
            XSpeedPacker::QuoteRequest(*lib_cfg_, p, quote_ref, insert_quote_req);

            process_result = p_tunnel->ReqQuoteInsert(&insert_quote_req);

            // insert quote success, save original request infomation
            if (process_result == TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                p_tunnel->xspeed_trade_context_.SaveQuoteInfo(quote_ref, XSpeedQuoteInfo(*p));
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
        TNL_LOG_WARN("insert failed, result=%d", process_result);

        // 应答包
        T_InsertQuoteRespond *rsp = new T_InsertQuoteRespond();
        XSpeedPacker::QuoteRespond(process_result, p->serial_no, 0, MY_TNL_OS_ERROR, *rsp);

        LogUtil::OnInsertQuoteRespond(rsp, tunnel_info_);

        SendRespondAsync(RespondDataType::kInsertQuoteRsp, rsp);
    }
}
///报价操作请求
void XspeedTunnel::ReqQuoteAction(const T_CancelQuote *p)
{
    LogUtil::OnCancelQuote(p, tunnel_info_);

    int process_result = TUNNEL_ERR_CODE::RESULT_FAIL;
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);

    if (p_tunnel)
    {
        OrderRefDataType org_quote_ref = p_tunnel->xspeed_trade_context_.GetQuoteRefBySerialNo(p->org_serial_no);

        // create request object
        DFITCCancelOrderField req;
        XSpeedPacker::CancelQuoteRequest(*lib_cfg_, p, org_quote_ref, req);

        process_result = p_tunnel->ReqQuoteAction(&req);

        if (process_result == TUNNEL_ERR_CODE::RESULT_SUCCESS)
        {
            p_tunnel->xspeed_trade_context_.UpdateCancelInfoOfQuoteRef(org_quote_ref, p->serial_no);
        }
    }
    else
    {
        TNL_LOG_ERROR("not support tunnel when cancel quote, please check configure file");
    }

    if (process_result != TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        // 应答包
        T_CancelQuoteRespond *cancel_quote_rsp = new T_CancelQuoteRespond();
        XSpeedPacker::CancelQuoteRespond(process_result, p->serial_no, p->entrust_no, *cancel_quote_rsp);

        LogUtil::OnCancelQuoteRespond(cancel_quote_rsp, tunnel_info_);

        SendRespondAsync(RespondDataType::kCancelQuoteRsp, cancel_quote_rsp);
    }
}

void XspeedTunnel::SetCallbackHandler(std::function<void(const T_OrderRespond *)> callback_handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
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
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
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
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_TradeReturn *)> callback_handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(callback_handler);
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
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
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
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
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
    QryTradeDetailReturnHandler_ = handler;
}

void XspeedTunnel::SetCallbackHandler(std::function<void(const T_ContractInfoReturn*)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
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
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_RspOfReqForQuote *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
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
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
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
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
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
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
        return;
    }
    p_tunnel->SetCallbackHandler(handler);
}
void XspeedTunnel::SetCallbackHandler(std::function<void(const T_QuoteTrade *)> handler)
{
    MYXSpeedSpi * p_tunnel = static_cast<MYXSpeedSpi *>(trade_inf_);
    if (!p_tunnel)
    {
        TNL_LOG_ERROR("SetCallbackHandler failed because tunnel initial failed, please check configure file");
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
