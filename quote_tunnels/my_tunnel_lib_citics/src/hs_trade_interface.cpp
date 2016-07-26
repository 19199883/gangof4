#include "hs_trade_interface.h"

#include <stdlib.h>
#include <iomanip>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "qtm_with_code.h"
#include "check_schedule.h"
#include "hs_util.h"
#include "my_protocol_packager.h"

#ifdef MY_LOG_DEBUG
#undef MY_LOG_DEBUG
#endif
#define MY_LOG_DEBUG(format, args...) my_cmn::my_log::instance(NULL)->log(my_cmn::LOG_MOD_QUOTE, my_cmn::LOG_PRI_DEBUG, format, ##args)

using namespace std;


void CiticsHsTradeInf::ReportErrorState(int api_error_no, const std::string &error_msg)
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

void CiticsHsTradeInf::SetNecessaryParam(HSHLPHANDLE HlpHandle)
{
    //设置除登陆外所有报文都需要送的参数。
    CITICs_HsHlp_BeginParam(HlpHandle);
    CITICs_HsHlp_SetValue(HlpHandle, "client_id", client_id);
    CITICs_HsHlp_SetValue(HlpHandle, "fund_account", this->tunnel_info_.account.c_str());
    CITICs_HsHlp_SetValue(HlpHandle, "sysnode_id", sysnode_id);
    CITICs_HsHlp_SetValue(HlpHandle, "identity_type", "2");
    CITICs_HsHlp_SetValue(HlpHandle, "op_branch_no", branch_no);
    CITICs_HsHlp_SetValue(HlpHandle, "branch_no", branch_no);
    CITICs_HsHlp_SetValue(HlpHandle, "op_station", op_station);
    CITICs_HsHlp_SetValue(HlpHandle, "op_entrust_way", op_entrust_way);
    CITICs_HsHlp_SetValue(HlpHandle, "password_type", password_type);
    CITICs_HsHlp_SetValue(HlpHandle, "password", acc_pswd.c_str());
    CITICs_HsHlp_SetValue(HlpHandle, "asset_prop", asset_prop);
    CITICs_HsHlp_SetValue(HlpHandle, "user_token", user_token);
}

CiticsHsTradeInf::CiticsHsTradeInf(const TunnelConfigData &cfg, const Tunnel_Info &tunnel_info)
    : cfg_(cfg), tunnel_info_(tunnel_info)
{
    connected_ = false;
    logoned_ = false;
    hs_handle_ = NULL;
    rtn_handle_ = NULL;

    have_handled_unterminated_orders_ = true;
    if (cfg_.Initial_policy_param().cancel_orders_at_start)
    {
        have_handled_unterminated_orders_ = false;
    }

    // 从配置解析参数
    if (!ParseConfig())
    {
        return;
    }

    Init();

    //	this session for asynchronous and subscribed messages
    Connect(hs_handle_);
    Login(hs_handle_);

    //	this session for place and cancel requests
    Connect(rtn_handle_);
    Login(rtn_handle_);

    // subscribe and start thread to process order return
    SubscribeOrderReturn();
    boost::thread(&CiticsHsTradeInf::ProcessAsynAndSubscribedMessage, this);

    QryStockHolderInfo();
    this->InitTradeDetail();
}

CiticsHsTradeInf::~CiticsHsTradeInf(void)
{
    if (hs_handle_)
    {
        CITICs_HsHlp_DisConnect(hs_handle_);
        CITICs_HsHlp_Exit(hs_handle_);
    }
    if (rtn_handle_)
    {
        UnsubscribeOrderReturn();
        CITICs_HsHlp_DisConnect(rtn_handle_);
        CITICs_HsHlp_Exit(rtn_handle_);
    }
}

bool CiticsHsTradeInf::ParseConfig()
{
    tunnel_info_.account = cfg_.Logon_config().clientid;
    acc_pswd = cfg_.Logon_config().password;
    if (cfg_.Logon_config().front_addrs.size() > 0)
    {
        server_address = cfg_.Logon_config().front_addrs.front();
    }
    strncpy(op_entrust_way, cfg_.Logon_config().op_entrust_way.c_str(), sizeof(op_entrust_way));
    strncpy(identity_type, cfg_.Logon_config().identity_type.c_str(), sizeof(identity_type));
    strncpy(password_type, cfg_.Logon_config().password_type.c_str(), sizeof(password_type));

    return true;
}

int CiticsHsTradeInf::PlaceOrder(const T_PlaceOrder* p)
{
    if (!TunnelIsReady())
    {
        MY_LOG_WARN("Tunnel can't process requests when it is NOT still ready.");
        return ProcessRejectResponse(TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE, p);
    }

    boost::mutex::scoped_lock lock(op_sync_);

    char sz_value[64];
    CITICs_HsHlp_BeginParam(hs_handle_);
    SetNecessaryParam(hs_handle_);
    // Improvement: add a field(exchange_type) into T_PlaceOrder
    string hsexc = HSFieldConvert::Convert2UtfExchange(p->exchange_type);
    string tmp = stock_accounts_[hsexc[0]];
    CITICs_HsHlp_SetValue(hs_handle_, "stock_account", stock_accounts_[hsexc[0]].c_str());
    CITICs_HsHlp_SetValue(hs_handle_, "exchange_type", hsexc.c_str());
    CITICs_HsHlp_SetValue(hs_handle_, "stock_code", p->stock_code);
    sprintf(sz_value, "%ld", p->volume);
    CITICs_HsHlp_SetValue(hs_handle_, "entrust_amount", sz_value);
    sprintf(sz_value, "%.3f", p->limit_price);
    CITICs_HsHlp_SetValue(hs_handle_, "entrust_price", sz_value);
    string bs = HSFieldConvert::ConvertUftBS(p->direction);
    CITICs_HsHlp_SetValue(hs_handle_, "entrust_bs", bs.c_str());
    CITICs_HsHlp_SetValue(hs_handle_, "entrust_prop", "0");
    CITICs_HsHlp_SetValue(hs_handle_, "batch_no", "0");
    CITICs_HsHlp_SetValue(hs_handle_, "entrust_type", "0");

    int ret = CITICs_HsHlp_BizCallAndCommit(hs_handle_, HSUtil::func_id_placeorder, NULL);
    if (ret != 0)
    {
        HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_id_placeorder, ret);
        char error_msg[512] = { 0 };
        CITICs_HsHlp_GetErrorMsg(hs_handle_, &ret, error_msg);
        ReportErrorState(ret, error_msg);
        return ProcessCommitFailure(ret, p);
    }
    else
    {
        this->ProcessSuccessfulSynResponse(hs_handle_, p);
        return TUNNEL_ERR_CODE::RESULT_SUCCESS;
    }
}

int CiticsHsTradeInf::ProcessRejectResponse(int err, const T_PlaceOrder* p)
{
    T_OrderRespond pOrderRespond;
    CITTCsUFTPacker::OrderRespond(0, p->serial_no, 0,
        FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR, pOrderRespond, p->exchange_type);
    if (OrderRespond_call_back_handler_)
    {
        OrderRespond_call_back_handler_(&pOrderRespond);
    }
    LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);

    return err;
}

void CiticsHsTradeInf::ProcessSuccessfulSynResponse(HSHLPHANDLE HlpHandle, const T_PlaceOrder *p)
{
    char entrust_no_str[512];
    CITICs_HsHlp_GetValue(HlpHandle, "entrust_no", entrust_no_str);
    int entrust_no = atoi(entrust_no_str);

    MY_LOG_DEBUG("order serial_no=%ld;entrust_no=%d", p->serial_no, entrust_no);

    hs_trade_context_.MapEntrustNo2ClOrdId(entrust_no, (int) (p->serial_no));
    T_OrderRespond pOrderRespond;
    CITTCsUFTPacker::OrderRespond(0, p->serial_no, entrust_no,
        FUNC_CONST_DEF::CONST_ENTRUST_STATUS_REPORDED, pOrderRespond, p->exchange_type);
    if (OrderRespond_call_back_handler_)
    {
        OrderRespond_call_back_handler_(&pOrderRespond);
    }
    LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
}

int CiticsHsTradeInf::ProcessCommitFailure(int err, const T_PlaceOrder* p)
{
    HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_id_placeorder, err);
    char error_msg[512] = { 0 };
    CITICs_HsHlp_GetErrorMsg(hs_handle_, &err, error_msg);
    ReportErrorState(err, error_msg);

    int std_err = cfg_.GetStandardErrorNo(err);
    MY_LOG_ERROR("place order failed, ErrorCode=%d; OrderRef: %ld ", err, p->serial_no);

    T_OrderRespond pOrderRespond;
    CITTCsUFTPacker::OrderRespond(0, p->serial_no, 0,
        FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR, pOrderRespond, p->exchange_type);
    if (OrderRespond_call_back_handler_)
    {
        OrderRespond_call_back_handler_(&pOrderRespond);
    }
    LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);

    return std_err;
}

int CiticsHsTradeInf::CancelOrder(const T_CancelOrder* pCancelOrder)
{
    if (!TunnelIsReady())
    {
        MY_LOG_WARN("tunnel is NOT still ready, so it can't process requests now.");
        return ProcessRejectResponse(TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE, pCancelOrder);;
    }

    boost::mutex::scoped_lock lock(op_sync_);

    int ret = CancelOrderInternal(pCancelOrder);
    if (ret != 0)
    {
        HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_id_cancelorder, ret);
        char error_msg[512] = { 0 };
        CITICs_HsHlp_GetErrorMsg(hs_handle_, &ret, error_msg);
        ReportErrorState(ret, error_msg);
        return ProcessSynCommitFailure(ret, pCancelOrder);
    }
    else
    {
        T_CancelRespond cancle_order;
        cancle_order.serial_no = pCancelOrder->serial_no;
        cancle_order.entrust_no = pCancelOrder->entrust_no;
        cancle_order.error_no = TUNNEL_ERR_CODE::RESULT_SUCCESS;
        cancle_order.entrust_status = FUNC_CONST_DEF::CONST_ENTRUST_STATUS_REPORDED;

        if (CancelRespond_call_back_handler_) {
            CancelRespond_call_back_handler_(&cancle_order);
        }

        LogUtil::OnCancelRespond(&cancle_order, tunnel_info_);
        return TUNNEL_ERR_CODE::RESULT_SUCCESS;
    }
}

int CiticsHsTradeInf::CancelOrderInternal(const T_CancelOrder* pCancelOrder)
{
    MY_LOG_DEBUG("CancelOrderInternal begin: entrust_no[%ld]", pCancelOrder->entrust_no);

    CITICs_HsHlp_BeginParam(hs_handle_);
    SetNecessaryParam(hs_handle_);
    string hsexc = HSFieldConvert::Convert2UtfExchange(pCancelOrder->exchange_type);
    string tmp = stock_accounts_[hsexc[0]];
    CITICs_HsHlp_SetValue(hs_handle_, "stock_account", stock_accounts_[hsexc[0]].c_str());
    CITICs_HsHlp_SetValue(hs_handle_, "exchange_type", hsexc.c_str());
    CITICs_HsHlp_SetValue(hs_handle_, "batch_flag", "0");
    char szEntrustNo[8];
    sprintf(szEntrustNo, "%08ld", pCancelOrder->entrust_no);
    CITICs_HsHlp_SetValue(hs_handle_, "entrust_no", szEntrustNo);
    int ret = CITICs_HsHlp_BizCallAndCommit(hs_handle_, HSUtil::func_id_cancelorder, NULL);

    return ret;
}

int CiticsHsTradeInf::ProcessRejectResponse(int err, const T_CancelOrder* pCancelOrder)
{
    T_CancelRespond cancel_respond;
    CITTCsUFTPacker::CancelRespond(err, pCancelOrder->serial_no, 0,
        FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR, cancel_respond);
    if (CancelRespond_call_back_handler_) {
        CancelRespond_call_back_handler_(&cancel_respond);
    }
    LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);

    return err;
}

int CiticsHsTradeInf::ProcessSynCommitFailure(int err, const T_CancelOrder* pCancelOrder)
{
    //	notify compliance checker to roll back
    HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_id_cancelorder, err);
    char error_msg[512] = { 0 };
    CITICs_HsHlp_GetErrorMsg(hs_handle_, &err, error_msg);
    ReportErrorState(err, error_msg);

    int std_err = cfg_.GetStandardErrorNo(err);
    MY_LOG_ERROR("cancel order failed, ErrorCode=%d; OrderRef: %ld ", err, pCancelOrder->serial_no);

    T_CancelRespond cancel_respond;
    CITTCsUFTPacker::CancelRespond(err, pCancelOrder->serial_no, 0,
        FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR, cancel_respond);

    if (CancelRespond_call_back_handler_) {
        CancelRespond_call_back_handler_(&cancel_respond);
    }

    LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);

    return std_err;
}

int CiticsHsTradeInf::QueryPosition(const T_QryPosition* pQryPosition)
{
    T_PositionReturn ret;
    ret.error_no = TUNNEL_ERR_CODE::RESULT_SUCCESS;

    if (!TunnelIsReady())
    {
        ret.error_no = -1;
        QryPosReturnHandler_(&ret);

        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        MY_LOG_WARN("tunnel is NOT still ready, so it can't process requests now.");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }

    boost::mutex::scoped_lock lock(op_sync_);

    //	fund query
    CITICs_HsHlp_BeginParam(hs_handle_);
    SetNecessaryParam(hs_handle_);
    CITICs_HsHlp_SetValue(hs_handle_, "money_type", " ");
    char szValue[320];
    int err = CITICs_HsHlp_BizCallAndCommit(hs_handle_, HSUtil::func_client_fund_all_qry, NULL);
    if (err != 0)
    {
        ret.error_no = -1;
        QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_client_fund_all_qry, err);
        char error_msg[512] = { 0 };
        CITICs_HsHlp_GetErrorMsg(hs_handle_, &err, error_msg);
        ReportErrorState(err, error_msg);
        int standard_error_no = cfg_.GetStandardErrorNo(err);
        MY_LOG_ERROR("func_client_fund_all_qry execution failed, ErrorCode=%d", err);
        return standard_error_no;
    }
    else
    {
        PositionDetail long_pos;
        memset(&long_pos, 0, sizeof(PositionDetail));
        strcpy(long_pos.stock_code, "#CASH");
        CITICs_HsHlp_GetValue(hs_handle_, "enable_balance", szValue);
        long_pos.direction = FUNC_CONST_DEF::CONST_ENTRUST_BUY;
        long_pos.position = atol(szValue);
        long_pos.position_avg_price = 1;
        long_pos.exchange_type = '1';

        ret.datas.push_back(long_pos);
    }

    bool bCompleted = false;
    char position_str[32] =
    { 0 };
    position_str[0] = ' ';
    while (!bCompleted)
    {
        CITICs_HsHlp_BeginParam(hs_handle_);
        SetNecessaryParam(hs_handle_);
        CITICs_HsHlp_SetValue(hs_handle_, "exchange_type", " ");
        CITICs_HsHlp_SetValue(hs_handle_, "stock_code", " ");
        CITICs_HsHlp_SetValue(hs_handle_, "query_mode", "1");
        CITICs_HsHlp_SetValue(hs_handle_, "position_str", position_str);
        char request_num[8] =
        { 0 };
        sprintf(request_num, "%d", PAGE_SIZE);
        CITICs_HsHlp_SetValue(hs_handle_, "request_num", request_num);
        int err = CITICs_HsHlp_BizCallAndCommit(hs_handle_, HSUtil::func_secu_stock_qry, NULL);
        if (err != 0)
        {
            ret.error_no = -1;
            QryPosReturnHandler_(&ret);
            LogUtil::OnPositionRtn(&ret, tunnel_info_);
            HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_secu_stock_qry, err);
            char error_msg[512] = { 0 };
            CITICs_HsHlp_GetErrorMsg(hs_handle_, &err, error_msg);
            ReportErrorState(err, error_msg);
            int standard_error_no = cfg_.GetStandardErrorNo(err);
            MY_LOG_ERROR("query stock position failed, ErrorCode=%d", err);
            return standard_error_no;
        }
        else
        {
            bCompleted = BuildInternalPosObj(hs_handle_, ret, position_str);
        }
    }	// END while(cursor!=-1 && cursor==PAGE_SIZE){

    if (NULL != QryPosReturnHandler_) QryPosReturnHandler_(&ret);
    LogUtil::OnPositionRtn(&ret, tunnel_info_);
    //HSUtil::ShowAnsData("query stock position", hs_handle_);
    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}

void CiticsHsTradeInf::fill_today_traded_info(PositionDetail &pos_detail)
{
    if (0 == TradeDetail_cache_.error_no)
    {
        vector<TradeDetail>::iterator it = TradeDetail_cache_.datas.begin();
        vector<TradeDetail>::iterator end = TradeDetail_cache_.datas.end();
        for (; it != end; ++it)
        {
            if (0 == strcmp(it->stock_code, pos_detail.stock_code))
            {
                if (it->direction == FUNC_CONST_DEF::CONST_ENTRUST_BUY)
                {
                    pos_detail.today_buy_volume += it->trade_volume;
                }
                else
                {
                    pos_detail.yestoday_position += it->trade_volume;
                    pos_detail.today_sell_volume += it->trade_volume;
                } // end if(it->direction==FUNC_CONST_DEF::CONST_ENTRUST_BUY){
            } // end if(0 == strcmp(it->stock_code,pos_detail.stock_code){
        } // end for(; it!=end; ++it){
    }
}

bool CiticsHsTradeInf::BuildInternalPosObj(HSHLPHANDLE HlpHandle, T_PositionReturn& ret, char *position_str)
{
    bool bCompleted = false;
    ret.error_no = 0;
    char sz_value[512];
    char code_value[512];
    char ex_value[512];

    int iRow = 0;
    iRow = CITICs_HsHlp_GetRowCount(HlpHandle);
    if (iRow > 0)
    {
        for (int i = 0; i < iRow; i++)
        {
            if (0 == CITICs_HsHlp_GetNextRow(HlpHandle))
            {
                HSUtil::ShowOneRowData("query stock position", hs_handle_, i);

                CITICs_HsHlp_GetValue(HlpHandle, "stock_code", code_value);

                // long position
                PositionDetail long_pos;
                memset(&long_pos, 0, sizeof(PositionDetail));
                strcpy(long_pos.stock_code, code_value);
                CITICs_HsHlp_GetValue(hs_handle_, "exchange_type", ex_value);
                long_pos.exchange_type = HSFieldConvert::ConvertFromUtfExchange(ex_value);
                CITICs_HsHlp_GetValue(HlpHandle, "position_str", position_str);
                if (iRow == PAGE_SIZE)
                {
                    bCompleted = false;
                }
                else
                {
                    bCompleted = true;
                }
                long_pos.direction = FUNC_CONST_DEF::CONST_ENTRUST_BUY;
                // today position
                CITICs_HsHlp_GetValue(HlpHandle, "current_amount", sz_value);
                long_pos.position = atoi(sz_value);
                CITICs_HsHlp_GetValue(HlpHandle, "cost_price", sz_value);
                long_pos.position_avg_price = atof(sz_value);
                // previous days position
                CITICs_HsHlp_GetValue(HlpHandle, "enable_amount", sz_value);
                long enable_amount = atoi(sz_value);
                long_pos.yestoday_position = enable_amount;
                long_pos.yd_position_avg_price = 0;
                fill_today_traded_info(long_pos);
                ret.datas.push_back(long_pos);

                // short position
                PositionDetail short_pos;
                memset(&short_pos, 0, sizeof(PositionDetail));
                strcpy(short_pos.stock_code, code_value);
                short_pos.exchange_type = HSFieldConvert::ConvertFromUtfExchange(ex_value);
                short_pos.direction = FUNC_CONST_DEF::CONST_ENTRUST_SELL;
                short_pos.position = 0;
                short_pos.position_avg_price = 0;
                short_pos.yestoday_position = 0;
                short_pos.yd_position_avg_price = 0;
                ret.datas.push_back(short_pos);
            }	// end if (0 == CITICs_HsHlp_GetNextRow(hs_handle_, MsgCtrl))
        }	// end for (int i = 0; i < iRow; i++)
    }
    else
    {
        bCompleted = true;
    }	// end if(iRow > 0){

    return bCompleted;
}

void CiticsHsTradeInf::InitTradeDetail()
{
    boost::mutex::scoped_lock lock(op_sync_);

    bool bCompleted = false;
    char position_str[32] =
    { 0 };
    position_str[0] = ' ';
    while (!bCompleted)
    {
        CITICs_HsHlp_BeginParam(hs_handle_);
        SetNecessaryParam(hs_handle_);
        CITICs_HsHlp_SetValue(hs_handle_, "exchange_type", " ");
        CITICs_HsHlp_SetValue(hs_handle_, "stock_code", " ");
        CITICs_HsHlp_SetValue(hs_handle_, "serial_no", "0");
        CITICs_HsHlp_SetValue(hs_handle_, "query_type", "1");
        CITICs_HsHlp_SetValue(hs_handle_, "query_mode", "2");
        CITICs_HsHlp_SetValue(hs_handle_, "position_str", position_str);
        char request_num[8] =
        { 0 };
        sprintf(request_num, "%d", PAGE_SIZE);
        CITICs_HsHlp_SetValue(hs_handle_, "request_num", request_num);
        CITICs_HsHlp_SetValue(hs_handle_, "etf_flag", "1");
        int err = CITICs_HsHlp_BizCallAndCommit(hs_handle_, HSUtil::func_secu_realtime_qry, NULL);
        if (err != 0)
        {
            HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_secu_stock_qry, err);
            char error_msg[512] = { 0 };
            CITICs_HsHlp_GetErrorMsg(hs_handle_, &err, error_msg);
            ReportErrorState(err, error_msg);
            int standard_error_no = cfg_.GetStandardErrorNo(err);
            MY_LOG_ERROR("query stock trade failed, ErrorCode=%d", err);
            return;
        }
        else
        {
            bCompleted = BuildTradeDetailObj(hs_handle_, TradeDetail_cache_, position_str);
        } // end if (err != 0) {
    }	// end
}

int CiticsHsTradeInf::QueryTradeDetail(const T_QryTradeDetail* pQryParam)
{
    T_TradeDetailReturn ret;
    if (!TunnelIsReady())
    {
        ret.error_no = -1;
        //if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        MY_LOG_WARN("tunnel is NOT still ready, so it can't process requests now.");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }

    boost::mutex::scoped_lock lock(op_sync_);

    bool bCompleted = false;
    char position_str[32] =
    { 0 };
    position_str[0] = ' ';
    while (!bCompleted)
    {
        CITICs_HsHlp_BeginParam(hs_handle_);
        SetNecessaryParam(hs_handle_);
        CITICs_HsHlp_SetValue(hs_handle_, "exchange_type", " ");
        CITICs_HsHlp_SetValue(hs_handle_, "stock_code", " ");
        CITICs_HsHlp_SetValue(hs_handle_, "serial_no", "0");
        CITICs_HsHlp_SetValue(hs_handle_, "query_type", "1");
        CITICs_HsHlp_SetValue(hs_handle_, "query_mode", "2");
        CITICs_HsHlp_SetValue(hs_handle_, "position_str", position_str);
        char request_num[8] =
        { 0 };
        sprintf(request_num, "%d", PAGE_SIZE);
        CITICs_HsHlp_SetValue(hs_handle_, "request_num", request_num);
        CITICs_HsHlp_SetValue(hs_handle_, "etf_flag", "1");
        int err = CITICs_HsHlp_BizCallAndCommit(hs_handle_, HSUtil::func_secu_realtime_qry, NULL);
        if (err != 0)
        {
            HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_secu_stock_qry, err);
            char error_msg[512] = { 0 };
            CITICs_HsHlp_GetErrorMsg(hs_handle_, &err, error_msg);
            ReportErrorState(err, error_msg);
            int standard_error_no = cfg_.GetStandardErrorNo(err);
            MY_LOG_ERROR("query stock trade failed, ErrorCode=%d", err);
            return standard_error_no;
        }
        else
        {
            bCompleted = BuildTradeDetailObj(hs_handle_, ret, position_str);
        } // end if (err != 0) {
    }	// end while(!bCompleted){

    if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
    LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    //HSUtil::ShowAnsData("query stock trade", hs_handle_);
    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}

bool CiticsHsTradeInf::BuildTradeDetailObj(HSHLPHANDLE HlpHandle, T_TradeDetailReturn& ret, char *position_str)
{
    bool bCompleted = false;
    ret.error_no = 0;
    char sz_value[512];

    int iRow = CITICs_HsHlp_GetRowCount(hs_handle_);
    if (iRow > 0)
    {
        for (int i = 0; i < iRow; i++)
        {
            if (0 == CITICs_HsHlp_GetNextRow(hs_handle_))
            {
                HSUtil::ShowOneRowData("query stock trade", hs_handle_, i);

                if (iRow == PAGE_SIZE)
                {
                    bCompleted = false;
                }
                else
                {
                    bCompleted = true;
                }

                CITICs_HsHlp_GetValue(hs_handle_, "real_type", sz_value);
                char real_type = sz_value[0];
                if (real_type != HSFieldConvert::REAL_TYPE_PLACE) continue;

                TradeDetail detail;
                memset(&detail, 0, sizeof(PositionDetail));

                CITICs_HsHlp_GetValue(HlpHandle, "position_str", position_str);

                CITICs_HsHlp_GetValue(hs_handle_, "stock_code", sz_value);
                strcpy(detail.stock_code, sz_value);
                CITICs_HsHlp_GetValue(hs_handle_, "entrust_no", sz_value);
                detail.entrust_no = atoi(sz_value);
                CITICs_HsHlp_GetValue(hs_handle_, "entrust_bs", sz_value);
                detail.direction = HSFieldConvert::ConvertDirFromHS(sz_value[0]);
                CITICs_HsHlp_GetValue(hs_handle_, "business_price", sz_value);
                detail.trade_price = atof(sz_value);
                CITICs_HsHlp_GetValue(hs_handle_, "business_amount", sz_value);
                detail.trade_volume = atoi(sz_value);
                CITICs_HsHlp_GetValue(hs_handle_, "business_time", sz_value);
                //strncpy(detail.trade_time, atoi(sz_value));

                ret.datas.push_back(detail);
            }	// end if (0 == CITICs_HsHlp_GetNextRow(hs_handle_, MsgCtrl))
        }	// end for (int i = 0; i < iRow; i++)
        //ret.datas.swap(td_buffer_);
    }
    else
    {
        bCompleted = true;
    }	// end if(iRow > 0){

    return bCompleted;
}

bool CiticsHsTradeInf::IsOrderTerminate(char status)
{
    return status == status == HSFieldConvert::kHsEntrustStatus_Filled
        || status == HSFieldConvert::kHsEntrustStatus_Discarded;
}

void CiticsHsTradeInf::Init()
{
    //根据交易所规定，op_station需要严格按照下列规则填写
    strcat(op_station, "TYJR-"); //默认前缀，请不要修改
    strcat(op_station, cfg_.Logon_config().client_name.c_str());
    strcat(op_station, " IP.");
    strcat(op_station, cfg_.Logon_config().local_ip.c_str());
    strcat(op_station, " MAC.");
    strcat(op_station, cfg_.Logon_config().mac_code.c_str());
    MY_LOG_INFO("CiticsHsTradeInf::Init() %s ", op_station);

    int ret = CITICs_HsHlp_LoadConfig(&hs_config_, "Hsconfig.ini");
    if (ret != 0)
    {
        MY_LOG_ERROR("load config file failed[Hsconfig.ini], ErrorCdoe=%d", ret);
        return;
    }
}

void CiticsHsTradeInf::Connect(HSHLPHANDLE &handle)
{
    char szMsg[512] =
    { 0 };
    int ret = CITICs_HsHlp_Init(&handle, hs_config_);
    if (ret != 0)
    {
        CITICs_HsHlp_GetErrorMsg(handle, NULL, szMsg);
        MY_LOG_ERROR("init connect handler failed, ErrorCode=%d, ErrorMsg=%s",
            ret, szMsg);
        return;
    }

    /// 连接服务器
    ret = CITICs_HsHlp_ConnectServer(handle);
    if (ret != 0)
    {
        CITICs_HsHlp_GetErrorMsg(handle, NULL, szMsg);
        MY_LOG_ERROR("connect failed, ErrorCode=%d, ErrorMsg=%s", ret, szMsg);
        return;
    }
    else
    {
        MY_LOG_INFO("Connect successful.");
        connected_ = true;
    }
}

void CiticsHsTradeInf::Login(HSHLPHANDLE &handle)
{
    if (!connected_)
        return;

    int iRet;
    MY_LOG_INFO("prepare to login");

    //备注：密码多次送错会冻结账户，十分钟后再试即可。
    CITICs_HsHlp_BeginParam(handle);
    CITICs_HsHlp_SetValue(handle, "identity_type", "2");
    CITICs_HsHlp_SetValue(handle, "password_type", "1");
    CITICs_HsHlp_SetValue(handle, "input_content", "1");
    CITICs_HsHlp_SetValue(handle, "op_entrust_way", op_entrust_way);
    CITICs_HsHlp_SetValue(handle, "password", acc_pswd.c_str());
    CITICs_HsHlp_SetValue(handle, "account_content", tunnel_info_.account.c_str());
    CITICs_HsHlp_SetValue(handle, "op_station", op_station);
    iRet = CITICs_HsHlp_BizCallAndCommit(handle, HSUtil::func_id_login, NULL);
    if (iRet != 0)
    {
        HSUtil::ShowErrMsg(handle, HSUtil::func_id_login, iRet);
        char error_msg[512] = { 0 };
        CITICs_HsHlp_GetErrorMsg(hs_handle_, &iRet, error_msg);
        ReportErrorState(iRet, error_msg);
        return;
    }

    //将登陆的返回值保存下来，以后所有的报文都需要
    CITICs_HsHlp_GetValue(handle, "client_id", client_id);
    CITICs_HsHlp_GetValue(handle, "user_token", user_token);
    CITICs_HsHlp_GetValue(handle, "branch_no", branch_no);
    CITICs_HsHlp_GetValue(handle, "asset_prop", asset_prop);
    CITICs_HsHlp_GetValue(handle, "sysnode_id", sysnode_id);

    int iRow, iCol;
    char szKey[64], szValue[512];
    iRow = CITICs_HsHlp_GetRowCount(handle);
    iCol = CITICs_HsHlp_GetColCount(handle);
    for (int i = 0; i < iRow; i++)
    {
        if (0 == CITICs_HsHlp_GetNextRow(handle))
        {
            for (int j = 0; j < iCol; j++)
            {
                CITICs_HsHlp_GetValue(handle, "fund_account", szValue);
                string fund_acc = szValue;
                if (fund_acc == this->tunnel_info_.account)
                {
                    CITICs_HsHlp_GetValue(handle, "stock_account", szValue);
                    string stock_account = szValue;
                    CITICs_HsHlp_GetValue(handle, "exchange_type", szValue);
                    char exchange_type = szValue[0];
                    stock_accounts_[exchange_type] = stock_account;
                }
            }	// end for (int j = 0; j < iCol; j++){
        }	// end if (0 == CITICs_HsHlp_GetNextRow(handle)){
    }	// end for (int i = 0; i < iRow; i++){

    HSUtil::ShowAnsData("login", handle);

    logoned_ = true;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_SUCCESS);
}

void CiticsHsTradeInf::QryStockHolderInfo()
{
    if (!logoned_) return;

    int iRet;
    MY_LOG_INFO("prepare to query stock holder's information");
    CITICs_HsHlp_BeginParam(hs_handle_);
    SetNecessaryParam(hs_handle_);
    CITICs_HsHlp_SetValue(hs_handle_, "fund_account", this->tunnel_info_.account.c_str());
    iRet = CITICs_HsHlp_BizCallAndCommit(hs_handle_, HSUtil::func_id_qry_stockholder, NULL);
    if (iRet != 0)
    {
        HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_id_qry_stockholder, iRet);
        char error_msg[512] = { 0 };
        CITICs_HsHlp_GetErrorMsg(hs_handle_, &iRet, error_msg);
        ReportErrorState(iRet, error_msg);
        return;
    }
    HSUtil::ShowAnsData("query stock holder's information", hs_handle_);
    CITICs_HsHlp_GetValue(hs_handle_, "stock_account", stock_account);

    MY_LOG_INFO("prepare to query account information");
    CITICs_HsHlp_BeginParam(hs_handle_);
    SetNecessaryParam(hs_handle_);

    iRet = CITICs_HsHlp_BizCallAndCommit(hs_handle_,
        HSUtil::func_client_fund_all_qry, NULL);
    if (iRet)
    {
        HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_client_fund_all_qry, iRet);
        char error_msg[512] = { 0 };
        CITICs_HsHlp_GetErrorMsg(hs_handle_, &iRet, error_msg);
        ReportErrorState(iRet, error_msg);
        return;
    }
    HSUtil::ShowAnsData("query account information", hs_handle_);

    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_CONTRACT_SUCCESS);

    // start thread for cancel unterminated orders
    if (!have_handled_unterminated_orders_)
    {
        CancelUnterminatedOrders();
    } else {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
    }
}

int CiticsHsTradeInf::SubscribeOrderReturn()
{
    MY_LOG_INFO("SubscribeOrderReturn");
    CITICs_HsHlp_BeginParam(rtn_handle_);
    CITICs_HsHlp_SetValue(rtn_handle_, "issue_type", "12");
    char szAccInfo[256];
    string acc_info = branch_no;
    acc_info += "~";
    acc_info += tunnel_info_.account;
    strcpy(szAccInfo, acc_info.c_str());
    CITICs_HsHlp_SetValue(rtn_handle_, "acc_info", szAccInfo);

    int iRet = CITICs_HsHlp_BizCallAndCommit(rtn_handle_, HSUtil::func_id_subscribe_match_rtn, NULL, BIZCALL_SUBSCRIBE, NULL);
    if (iRet != 0)
    {
        HSUtil::ShowErrMsg(rtn_handle_, HSUtil::func_id_subscribe_match_rtn, iRet);
        char error_msg[512] = { 0 };
        CITICs_HsHlp_GetErrorMsg(hs_handle_, &iRet, error_msg);
        ReportErrorState(iRet, error_msg);
        MY_LOG_ERROR("subscribe order return failed.");
        return iRet;
    }
    HSUtil::ShowAnsData("subscribe", rtn_handle_);
    return iRet;
}

void CiticsHsTradeInf::UnsubscribeOrderReturn()
{
    MY_LOG_INFO("UnsubscribeOrderReturn");
    CITICs_HsHlp_BeginParam(hs_handle_);
    CITICs_HsHlp_SetValue(rtn_handle_, "branch_no", branch_no);
    CITICs_HsHlp_SetValue(rtn_handle_, "issue_type", "12");
    CITICs_HsHlp_SetValue(rtn_handle_, "acc_info", tunnel_info_.account.c_str());
    int iRet = CITICs_HsHlp_BizCallAndCommit(rtn_handle_,
        HSUtil::func_id_cancelsubscribe_match_rtn, NULL, BIZCALL_SUBSCRIBE,
        NULL);
    if (iRet != 0)
    {
        HSUtil::ShowErrMsg(rtn_handle_, HSUtil::func_id_subscribe_match_rtn, iRet);
        char error_msg[512] = { 0 };
        CITICs_HsHlp_GetErrorMsg(hs_handle_, &iRet, error_msg);
        ReportErrorState(iRet, error_msg);
        MY_LOG_ERROR("unsubscribe order return failed.");
        return;
    }
    HSUtil::ShowAnsData("unsubscribe", rtn_handle_);
}

void CiticsHsTradeInf::resubscribe()
{
    while (true)
    {
        if (SubscribeOrderReturn() == 0)
            break;
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void CiticsHsTradeInf::ProcessAsynAndSubscribedMessage()
{
    MY_LOG_INFO("start thread which processes asynchronous messages and subscribed messages.");

    MSG_CTRL *MsgCtrl = new MSG_CTRL();
    while (true)
    {
        if (CITICs_HsHlp_QueueMsgCount(rtn_handle_) > 0)
        {
            MsgCtrl->nIssueType = 0;
            MsgCtrl->nReqNo = 0;
            MsgCtrl->nFuncID = 0;
            if (CITICs_HsHlp_QueueGetMsg(rtn_handle_, MsgCtrl, 0) == 0)
            {
                if (MsgCtrl->nFlags != 0)
                {
                    MY_LOG_ERROR("CITICs_HsHlp_QueueGetMsg get exception msg, nFlags=%d, msg=%s.",
                        MsgCtrl->nFlags, MsgCtrl->szErrorInfo);
                    resubscribe();
                }
                else if (MsgCtrl->nFlags == 0 && MsgCtrl->HAsyncMsg != NULL)
                {
                    if (MsgCtrl->nIssueType > 0)
                    {
                        ProcessSubscribedMessage(rtn_handle_, MsgCtrl);
                    }
                }	// end else if (MsgCtrl->nFlags == 0 && MsgCtrl->HAsyncMsg != NULL) {
                //HSUtil::ShowAnsData("receive asynchronous response", rtn_handle_,MsgCtrl);
                CITICs_HsHlp_QueueEraseMsg(rtn_handle_, MsgCtrl);
            }
            else
            {
                // TODO: log
            }
        }	// end for (std::map<int,long>::iterator it=asyn_msg_reqnos_.begin(); it!=asyn_msg_reqnos_.end(); ++it){
    }	// end while (true) {
}

void CiticsHsTradeInf::ProcessAcceptedSubscribedMessage(
    HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, T_PlaceOrder *p)
{
    char sz_value[512];
    CITICs_HsHlp_GetValue(hs_handle_, "entrust_no", sz_value, lpMsg_Ctrl);
    int entrust_no = atoi(sz_value);
    T_OrderRespond pOrderRespond;
    CITTCsUFTPacker::OrderRespond(0, p->serial_no, entrust_no, FUNC_CONST_DEF::CONST_ENTRUST_STATUS_REPORDED,
        pOrderRespond, p->exchange_type);
    if (OrderRespond_call_back_handler_) {
        OrderRespond_call_back_handler_(&pOrderRespond);
    }
    LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
}

void CiticsHsTradeInf::ProcessAcceptedCancelSubscribedMessage(T_InternalOrder *p)
{
    char sz_value[512];

    ComplianceCheck_OnOrderPendingCancel(cfg_.Logon_config().clientid.c_str(), p->serial_no);

    T_CancelRespond cancel_respond;
    CITTCsUFTPacker::CancelRespond(0, p->serial_no, p->entrust_no, p->status, cancel_respond);
    if (CancelRespond_call_back_handler_) {
        CancelRespond_call_back_handler_(&cancel_respond);
    }
    LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
}

void CiticsHsTradeInf::ProcessRejectedCancelSubscribedMessage(
    HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, T_InternalOrder *p)
{
    char sz_value[512];
    CITICs_HsHlp_GetValue(hs_handle_, "extern_code", sz_value, lpMsg_Ctrl);
    int err = atoi(sz_value);
    int std_err = cfg_.GetStandardErrorNo(err);

    CITICs_HsHlp_GetValue(hs_handle_, "entrust_no", sz_value, lpMsg_Ctrl);
    int entrust_no = atoi(sz_value);

//	 ComplianceCheck_OnOrderCancelFailed(cfg_.Logon_config().clientid.c_str(),p->stock_code);

    T_CancelRespond cancel_respond;
    CITTCsUFTPacker::CancelRespond(std_err, p->serial_no, entrust_no,
        p->status, cancel_respond);
    if (CancelRespond_call_back_handler_) {
        CancelRespond_call_back_handler_(&cancel_respond);
    }
    LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
}

void CiticsHsTradeInf::ProcessSubscribedCancelMessage(HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, const int &rpt_no)
{
    char sz_value[512];
    T_InternalOrder *ord = hs_trade_context_.GetInternalOrdByRptNo(rpt_no);
    if (ord != NULL)
    {
        CITICs_HsHlp_GetValue(hs_handle_, "real_status", sz_value, lpMsg_Ctrl);
        char my_status = HSFieldConvert::RealtStatusTransForCancel(sz_value[0], ord->acc_vol, ord->volume);
        ord->status = my_status;
        switch (my_status)
        {
            case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_WITHDRAWING:	//等待撤除
                ProcessAcceptedCancelSubscribedMessage(ord);
                break;
            case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR:		//错误委托
                ProcessRejectedCancelSubscribedMessage(HlpHandle, lpMsg_Ctrl, ord);
                break;
            case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_DISABLE:		//场内拒绝
                ProcessRejectedCancelSubscribedMessage(HlpHandle, lpMsg_Ctrl, ord);
                break;
            case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_WITHDRAWED:	//已经撤销
                ProcessCancelledStatusMessage(ord);
                break;
            case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_PARTIALWD:	//部成部撤
                ProcessCancelledStatusMessage(ord);
                break;
        }	// end switch(my_status){

    }
    else
    {
        MY_LOG_INFO("Can't find the order whose report_no is: %d", rpt_no);
    }	// end if(ord != NULL){
}

void CiticsHsTradeInf::ProcessOrderStatusData(T_InternalOrder *p)
{
    T_OrderReturn order_return;
    CITTCsUFTPacker::OrderReturn(p, order_return, p->exchange_type);
    if (OrderReturn_call_back_handler_) {
        OrderReturn_call_back_handler_(&order_return);
    }
    LogUtil::OnOrderReturn(&order_return, tunnel_info_);
}

void CiticsHsTradeInf::ProcessRejectedStatusMessage(HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, T_InternalOrder *p)
{
    //	notify Compliance Checker to roll back
    ComplianceCheck_OnOrderInsertFailed(
        cfg_.Logon_config().clientid.c_str(),
        p->serial_no,
        p->exchange_type,
        p->stock_code,
        p->volume,
        p->speculator,
        p->open_close,
        p->order_type);

    //	call back requester
    MY_LOG_WARN("place order failed, result=%d", lpMsg_Ctrl->nErrorNo);

    char sz_value[512];
    CITICs_HsHlp_GetValue(hs_handle_, "extern_code", sz_value, lpMsg_Ctrl);
    int err = atoi(sz_value);
    int std_err = cfg_.GetStandardErrorNo(err);
    T_OrderRespond ord_rep;
    CITTCsUFTPacker::OrderRespond(std_err, p->serial_no, 0, p->status, ord_rep, p->exchange_type);
    if (OrderRespond_call_back_handler_) {
        OrderRespond_call_back_handler_(&ord_rep);
    }

    MY_LOG_DEBUG("order rejected:  \n%ld", p->serial_no);

    LogUtil::OnOrderRespond(&ord_rep, tunnel_info_);
}

void CiticsHsTradeInf::ProcessAcceptedPlaceSubscribedMessage(T_InternalOrder *p)
{
    T_OrderReturn order_return;
    CITTCsUFTPacker::OrderReturn(p, order_return, p->exchange_type);
    if (OrderReturn_call_back_handler_)
    {
        OrderReturn_call_back_handler_(&order_return);
    }
    else
    {
        MY_LOG_INFO("OrderReturn_call_back_handler_ didn't set callback ");
    }

    LogUtil::OnOrderReturn(&order_return, tunnel_info_);
}

void CiticsHsTradeInf::ProcessFilledStatusMessage(T_InternalOrder *p)
{
    ComplianceCheck_OnOrderFilled(cfg_.Logon_config().clientid.c_str(), p->serial_no);

    T_OrderReturn order_return;
    CITTCsUFTPacker::OrderReturn(p, order_return, p->exchange_type);
    if (OrderReturn_call_back_handler_)
    {
        OrderReturn_call_back_handler_(&order_return);
    }
    else
    {
        MY_LOG_INFO("OrderReturn_call_back_handler_ didn't set callback ");
    }

    LogUtil::OnOrderReturn(&order_return, tunnel_info_);
}

void CiticsHsTradeInf::ProcessFilledDataMessage(T_InternalOrder *p)
{
    T_TradeReturn trade_return;
    CITTCsUFTPacker::TradeReturn(p, trade_return, p->exchange_type);
    if (TradeReturn_call_back_handler_)
    {
        TradeReturn_call_back_handler_(&trade_return);
    }
    else
    {
        MY_LOG_INFO("TradeReturn_call_back_handler__ didn't set callback ");
    }

    LogUtil::OnTradeReturn(&trade_return, tunnel_info_);
}
void CiticsHsTradeInf::ProcessPartialFilledStatusMessage(T_InternalOrder *p)
{
    T_OrderReturn order_return;
    CITTCsUFTPacker::OrderReturn(p, order_return, p->exchange_type);
    if (OrderReturn_call_back_handler_) {
        OrderReturn_call_back_handler_(&order_return);
    }
    LogUtil::OnOrderReturn(&order_return, tunnel_info_);
}

void CiticsHsTradeInf::ProcessCancelledStatusMessage(T_InternalOrder *p)
{
//	 ComplianceCheck_OnOrderCanceled(
//			cfg_.Logon_config().clientid.c_str(),
//			p->serial_no,
//			p->exchange_type,
//			p->volume - p->acc_vol,
//			p->speculator,
//			p->open_close);

    T_OrderReturn order_return;
    CITTCsUFTPacker::OrderReturn(p, order_return, p->exchange_type);
    if (OrderReturn_call_back_handler_) {
        OrderReturn_call_back_handler_(&order_return);
    }
    LogUtil::OnOrderReturn(&order_return, tunnel_info_);
}

void CiticsHsTradeInf::ProcessSubscribedPlaceMessage(HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, const int rpt_no)
{
    char sz_value[512];
    char status_value[512];
    T_InternalOrder *ord = hs_trade_context_.GetInternalOrdByRptNo(rpt_no);
    if (ord != NULL)
    {
        if (ord->entrust_no == 0)
        {
            ord->entrust_no = rpt_no;
        }

        // real_status
        CITICs_HsHlp_GetValue(hs_handle_, "real_status", status_value, lpMsg_Ctrl);

        MY_LOG_DEBUG("cl_ord_id:%ld; status:%c;", ord->serial_no, status_value[0]);
        if (status_value[0] != ' ')
        {
            // business_amount
            CITICs_HsHlp_GetValue(hs_handle_, "business_amount", sz_value, lpMsg_Ctrl);
            long last_vol = stol(sz_value);
            if (0 != last_vol)
            {
                ord->acc_vol += stol(sz_value);
                ord->last_vol = stol(sz_value);
                // business_price
                CITICs_HsHlp_GetValue(hs_handle_, "business_price", sz_value, lpMsg_Ctrl);
                ord->last_px = stod(sz_value);
            }

            ord->status = HSFieldConvert::RealtStatusTransForPlace(status_value[0], ord->acc_vol, ord->volume);

            MY_LOG_DEBUG("cl_ord_id:%ld; status:%c; acc_vol:%ld; volume:%ld", ord->serial_no, ord->status, ord->acc_vol, ord->volume);
            switch (ord->status)
            {
                case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_REPORDED: 	//已经报入
                    ProcessAcceptedPlaceSubscribedMessage(ord);
                    break;
                case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_PARTIALCOM:	//部分成交
                    ProcessPartialFilledStatusMessage(ord);
                    this->ProcessFilledDataMessage(ord);
                    break;
                case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_COMPLETED:	//全部成交
                    ProcessFilledStatusMessage(ord);
                    this->ProcessFilledDataMessage(ord);
                    break;
                case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR:		//错误委托
                    ProcessRejectedStatusMessage(HlpHandle, lpMsg_Ctrl, ord);
                    break;
                case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_DISABLE:		//场内拒绝
                    ProcessRejectedStatusMessage(HlpHandle, lpMsg_Ctrl, ord);
                    break;
                case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_WITHDRAWED:	//已经撤销
                    ProcessCancelledStatusMessage(ord);
                    break;
                case FUNC_CONST_DEF::CONST_ENTRUST_STATUS_PARTIALWD:	//部成部撤
                    ProcessCancelledStatusMessage(ord);
                    break;
            }
        }	// end if(sz_value[0] != " "){

    }
    else
    {
        MY_LOG_INFO("Can't find the order whose report_no is: %d", rpt_no);
    }	// end if(ord != NULL){
}

void CiticsHsTradeInf::ProcessSubscribedMessage(HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl)
{
    int iRow = CITICs_HsHlp_GetRowCount(hs_handle_, lpMsg_Ctrl);
    char sz_value[512];
    for (int i = 0; i < iRow; i++)
    {
        if (0 == CITICs_HsHlp_GetNextRow(hs_handle_, lpMsg_Ctrl))
        {
            HSUtil::ShowOneRowData("receive asynchronous response", rtn_handle_, lpMsg_Ctrl, i);

            CITICs_HsHlp_GetValue(hs_handle_, "entrust_no", sz_value, lpMsg_Ctrl);
            int entrust_no = atoi(sz_value);
            CITICs_HsHlp_GetValue(hs_handle_, "real_type", sz_value, lpMsg_Ctrl);
            char real_type = sz_value[0];
            if (real_type == HSFieldConvert::REAL_TYPE_PLACE)
            {
                ProcessSubscribedPlaceMessage(hs_handle_, lpMsg_Ctrl, entrust_no);
            }
            else if (real_type == HSFieldConvert::REAL_TYPE_CANCEL)
            {
                ProcessSubscribedCancelMessage(hs_handle_, lpMsg_Ctrl, entrust_no);
            }
        }	// end if (0 == CITICs_HsHlp_GetNextRow(hs_handle_, MsgCtrl)) {
    }	// end for (int i = 0; i < iRow; i++) {
}

int CiticsHsTradeInf::CancelUnterminatedOrders()
{
    int ret = 0;

    boost::mutex::scoped_lock lock(op_sync_);

    MY_LOG_DEBUG("CancelUnterminatedOrders execution begins...");

    vector<T_CancelOrder> cancelOrders;
    int err = CiticsHsTradeInf::QueryCancelableOrders(cancelOrders);
    if (0 == err)
    {
        int count = cancelOrders.size();
        MY_LOG_DEBUG("CancelUnterminatedOrders: count of cancelable order:%d", count);
        if (count > 0)
        {
            vector<T_CancelOrder>::iterator it = cancelOrders.begin();
            vector<T_CancelOrder>::iterator end = cancelOrders.end();
            for (; it != end; ++it)
            {
                T_CancelOrder tmp = *it;
                int code = CancelOrderInternal(&tmp);
                if (0 == code)
                {
                    MY_LOG_DEBUG("CancelOrderInternal succeeded: entrust_no[%ld]", tmp.entrust_no);
                }
                else
                {
                    HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_id_cancelorder, code);
                    char error_msg[512] = { 0 };
                    CITICs_HsHlp_GetErrorMsg(hs_handle_, &code, error_msg);
                    ReportErrorState(code, error_msg);
                    int std_err = cfg_.GetStandardErrorNo(code);
                    ret = std_err;
                    MY_LOG_ERROR("cancel order failed, ErrorCode=%d; OrderRef: %ld ", std_err, tmp.serial_no);
                    break;
                }
            } // end for(; it!=end; ++it){
        }
    }
    else
    {
        ret = err;
        MY_LOG_ERROR("CancelUnterminatedOrders: error %d occured when QueryCancelableOrders executed.", err);
    }	// end if (0==err){

    if (0 == ret)
    {
        have_handled_unterminated_orders_ = true;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
    }

    MY_LOG_DEBUG("CancelUnterminatedOrders execution completed.");

    return ret;
}

int CiticsHsTradeInf::QueryCancelableOrders(vector<T_CancelOrder> &cancelOrders)
{
    bool bCompleted = false;
    char position_str[32] =
    { 0 };
    position_str[0] = ' ';
    while (!bCompleted)
    {
        CITICs_HsHlp_BeginParam(hs_handle_);
        SetNecessaryParam(hs_handle_);
        CITICs_HsHlp_SetValue(hs_handle_, "entrust_no", 0);
        CITICs_HsHlp_SetValue(hs_handle_, "stock_account", " ");
        CITICs_HsHlp_SetValue(hs_handle_, "position_str", position_str);
        char request_num[8] =
        { 0 };
        sprintf(request_num, "%d", PAGE_SIZE);
        CITICs_HsHlp_SetValue(hs_handle_, "request_num", request_num);

        int err = CITICs_HsHlp_BizCallAndCommit(hs_handle_, HSUtil::func_secu_en_with_ent_qry, NULL);
        if (err != 0)
        {
            HSUtil::ShowErrMsg(hs_handle_, HSUtil::func_secu_en_with_ent_qry, err);
            char error_msg[512] = { 0 };
            CITICs_HsHlp_GetErrorMsg(hs_handle_, &err, error_msg);
            ReportErrorState(err, error_msg);
            int standard_error_no = cfg_.GetStandardErrorNo(err);
            MY_LOG_ERROR("QueryCancelableOrders failed, ErrorCode=%d", err);
            return standard_error_no;
        }
        else
        {
            bCompleted = BuildTradeDetailObj(hs_handle_, cancelOrders, position_str);
        } // end if (err != 0) {
    }	// end while(!bCompleted){

    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}

bool CiticsHsTradeInf::BuildTradeDetailObj(HSHLPHANDLE HlpHandle, vector<T_CancelOrder>& cancelOrders, char *position_str)
{
    bool bCompleted = false;
    char sz_value[512];

    int iRow = CITICs_HsHlp_GetRowCount(hs_handle_);
    if (iRow > 0)
    {
        for (int i = 0; i < iRow; i++)
        {
            if (0 == CITICs_HsHlp_GetNextRow(hs_handle_))
            {
                HSUtil::ShowOneRowData("QueryCancelableOrders", hs_handle_, i);

                T_CancelOrder cancelOrd;
                memset(&cancelOrd, 0, sizeof(T_CancelOrder));

                CITICs_HsHlp_GetValue(HlpHandle, "position_str", position_str);
                if (iRow == PAGE_SIZE)
                {
                    bCompleted = false;
                }
                else
                {
                    bCompleted = true;
                }
                CITICs_HsHlp_GetValue(hs_handle_, "stock_code", sz_value);
                strcpy(cancelOrd.stock_code, sz_value);
                CITICs_HsHlp_GetValue(hs_handle_, "report_no", sz_value);
                cancelOrd.entrust_no = atoi(sz_value);
                CITICs_HsHlp_GetValue(hs_handle_, "entrust_bs", sz_value);
                cancelOrd.direction = HSFieldConvert::ConvertDirFromHS(sz_value[0]);

                CITICs_HsHlp_GetValue(hs_handle_, "exchange_type", sz_value);
                cancelOrd.exchange_type = HSFieldConvert::ConvertFromUtfExchange(sz_value);

                cancelOrders.push_back(cancelOrd);
            }	// end if (0 == CITICs_HsHlp_GetNextRow(hs_handle_, MsgCtrl))
        }	// end for (int i = 0; i < iRow; i++)
        //ret.datas.swap(td_buffer_);
    }
    else
    {
        bCompleted = true;
    }	// end if(iRow > 0){

    return bCompleted;
}
