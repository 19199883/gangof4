#include "rem_trade_interface.h"

#include <stdlib.h>
#include <iomanip>
#include <algorithm>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "my_cmn_util_funcs.h"
#include "tunnel_cmn_utility.h"
#include "rem_struct_convert.h"
#include "check_schedule.h"

#include "qtm_with_code.h"

using namespace std;

void MYRemTradeSpi::ReportErrorState(int api_error_no, const std::string &error_msg)
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

MYRemTradeSpi::MYRemTradeSpi(const TunnelConfigData &cfg)
    : cfg_(cfg)
{
    connected_ = false;
    logoned_ = false;
    in_init_state_ = true;
    available_fund = 0;
    trade_day = 0;
    tunnel_info_.account = cfg_.Logon_config().investorid;
    user_ = cfg_.Logon_config().clientid;
    pswd_ = cfg_.Logon_config().password;
    mac_addr_ = cfg_.Logon_config().mac_address;
    program_name_ = cfg_.App_cfg().program_title;

    //qtm init
    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "rem_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

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
    finish_query_orders_ = false;

    // create
    api_ = CreateEESTraderApi();
    if (api_ == NULL)
    {
        TNL_LOG_WARN("CreateEESTraderApi failed!");
        return;
    }

    // TODO output log when debug, should close before release
    api_->SetLoggerSwitch(true);
    IPAndPortNum ip_port = ParseIPAndPortNum(cfg_.Logon_config().front_addrs.front());
    int ret = api_->ConnServer(ip_port.first.c_str(), ip_port.second, this);
    TNL_LOG_WARN("ConnServer, addr: %s:%d; return:%d", ip_port.first.c_str(), ip_port.second, ret);
}

MYRemTradeSpi::~MYRemTradeSpi(void)
{
    if (api_)
    {
        DestroyEESTraderApi(api_);
    }
}

void MYRemTradeSpi::ReqLogin()
{
    sleep(3);

    int ret = api_->UserLogon(user_.c_str(), pswd_.c_str(), program_name_.c_str(), mac_addr_.c_str());
    TNL_LOG_INFO("MYRemTradeSpi::ReqLogin() - UserLogon, return:%d", ret);
}

void MYRemTradeSpi::QueryAccount()
{
    // query account
    while (true)
    {
        int ret = api_->QueryAccountBP(tunnel_info_.account.c_str(), 0);
        TNL_LOG_INFO("QueryAccountBP, return: %d", ret);

        if (ret == 0)
        {
            break;
        }
        sleep(1);
    }
}

PositionDetail MYRemTradeSpi::PackageAvailableFund()
{
    // insert available fund
    PositionDetail ac;
    memset(&ac, 0, sizeof(ac));
    strcpy(ac.stock_code, "#CASH");
    ac.direction = '0';
    ac.position = (long) available_fund;  // available fund at beginning, shouldn't update after init

    return ac;
}

EES_CancelOrder MYRemTradeSpi::CreatCancelParam(EES_MarketToken market_token)
{
    EES_CancelOrder co;

    bzero(&co, sizeof(co));
    strncpy(co.m_Account, tunnel_info_.account.c_str(), sizeof(co.m_Account));
    co.m_MarketOrderToken = market_token;
    co.m_Quantity = 0;

    return co;
}

void MYRemTradeSpi::OnConnection(ERR_NO errNo, const char* pErrStr)
{
    TNL_LOG_INFO("MYRemTradeSpi::OnConnection, err_no:%d, msg:%s.", errNo, pErrStr);
    connected_ = true;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::CONNECT_SUCCESS);
    // 成功登录后，断开不再重新登录
    if (in_init_state_)
    {
        int ret = api_->UserLogon(user_.c_str(), pswd_.c_str(),  program_name_.c_str(), mac_addr_.c_str());
        TNL_LOG_INFO("UserLogon, return:%d", ret);
    }
}

void MYRemTradeSpi::OnDisConnection(ERR_NO errNo, const char* pErrStr)
{
    connected_ = false;

    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
    TNL_LOG_WARN("MYRemTradeSpi::OnDisConnection, err_no:%d, msg:%s", errNo, pErrStr);
}

void MYRemTradeSpi::OnUserLogon(EES_LogonResponse* pLogon)
{
    TNL_LOG_INFO("MYRemTradeSpi::OnUserLogon: %s", RemStructFormater::ToString(pLogon).c_str());

    // logon success
    if (pLogon && pLogon->m_Result == 0)
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_SUCCESS);
        // save max client token
        trade_context_.SetClientToken(pLogon->m_MaxToken);
        trade_day = pLogon->m_TradingDate;

        logoned_ = true;
        in_init_state_ = false;

        std::thread t_qry_account(&MYRemTradeSpi::QueryAccount, this);
        t_qry_account.detach();
    }
    else
    {
        TNL_LOG_WARN("user login failed, err_no:%d", pLogon->m_Result);

        // 重新登陆
        std::thread t_req_login(&MYRemTradeSpi::ReqLogin, this);
        t_req_login.detach();
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_FAIL);
    }
}

void MYRemTradeSpi::OnQueryUserAccount(EES_AccountInfo* p, bool bFinish)
{
    TNL_LOG_INFO("MYRemTradeSpi::OnQueryUserAccount: %s", RemStructFormater::ToString(p).c_str());
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
void MYRemTradeSpi::OnQueryAccountPosition(const char* pAccount, EES_AccountPosition* pf, int nReqId, bool bFinish)
{
    TNL_LOG_DEBUG("OnQueryAccountPosition: acc=%s, req_id=%d, last_flag=%d \n%s",
        pAccount, nReqId, bFinish,
        RemStructFormater::ToString(pf).c_str());

    T_PositionReturn ret;

    if (!bFinish && pf)
    {
        PositionDetail pos;
        memset(&pos, 0, sizeof(pos));
        strncpy(pos.stock_code, pf->m_Symbol, sizeof(pos.stock_code));
        pos.exchange_type = MY_TNL_EC_CFFEX;

        if (pf->m_PosiDirection == EES_PosiDirection_long)
        {
            pos.direction = MY_TNL_D_BUY;
        }
        else
        {
            pos.direction = MY_TNL_D_SELL;
        }

        // 今仓，当前的仓位
        pos.position = pf->m_TodayQty + pf->m_OvnQty;
        pos.position_avg_price = 0; // can't get cost or price

        // quality of over night
        pos.yestoday_position = pf->m_InitOvnQty;
        pos.yd_position_avg_price = 0; // can't get cost or price

        std::vector<PositionDetail>::iterator it = std::find_if(pos_buffer_.begin(), pos_buffer_.end(), PositionDetailPred(pos));
        if (it == pos_buffer_.end())
        {
            pos_buffer_.push_back(pos);
        }
        else
        {
            it->position += pos.position;
            it->position_avg_price += pos.position_avg_price; // now is total cost
            it->yestoday_position += pos.yestoday_position;
            it->yd_position_avg_price += pos.yd_position_avg_price; // now is total cost
        }
    }

    if (bFinish)
    {
        // calc avg price
        for (PositionDetail &v : pos_buffer_)
        {
            if (v.position > 0) v.position_avg_price = v.position_avg_price / v.position;
            if (v.yestoday_position > 0) v.yd_position_avg_price = v.yd_position_avg_price / v.yestoday_position;
        }

        TNL_LOG_INFO("receive last respond of query position, send to client");
        ret.datas.swap(pos_buffer_);

        ret.datas.insert(ret.datas.begin(), PackageAvailableFund());

        ret.error_no = 0;
        if (QryPosReturnHandler_) QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}

void MYRemTradeSpi::OnQueryAccountBP(const char* pAccount, EES_AccountBP* pf, int nReqId)
{
    TNL_LOG_DEBUG("MYRemTradeSpi::OnQueryAccountBP: acc=%s, req_id=%d\n%s",
        pAccount, nReqId,
        RemStructFormater::ToString(pf).c_str());

    if (pf)
    {
        available_fund = pf->m_AvailableBp;
    }

    // start thread for cancel unterminated orders
    if (!HaveFinishQueryOrders())
    {
        std::thread qry_order_t_(&MYRemTradeSpi::QueryAndHandleOrders, this);
        qry_order_t_.detach();
    }
    else
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
    }
}

void MYRemTradeSpi::OnQuerySymbol(EES_SymbolField* pf, bool bFinish)
{
    T_ContractInfoReturn ret;

    TNL_LOG_DEBUG("MYRemTradeSpi::OnQuerySymbol: last_flag=%d \n%s",
        bFinish,
        RemStructFormater::ToString(pf).c_str());

    // valid symbol object
    if (!bFinish && pf != NULL)
    {
        ContractInfo ci;
        strncpy(ci.stock_code, pf->m_symbol, sizeof(ci.stock_code));
        sprintf(ci.TradeDate, "%08d", trade_day);
        sprintf(ci.ExpireDate, "%08d", pf->m_ExpireDate);
        ci_buffer_.push_back(ci);
    }

    if (bFinish)
    {
        TNL_LOG_DEBUG("finish OnRspQryInstrument, prepare to send to client, data size:%d", ci_buffer_.size());
        ret.datas.swap(ci_buffer_);
        ret.error_no = 0;

        if (QryContractInfoHandler_) QryContractInfoHandler_(&ret);
        LogUtil::OnContractInfoRtn(&ret, tunnel_info_);
    }
}

void MYRemTradeSpi::OnOrderAccept(EES_OrderAcceptField* pAccept)
{
    try
    {
        if (!pAccept)
        {
            TNL_LOG_WARN("OnOrderAccept, pAccept is null.");
            return;
        }

        const RemReqInfo *p = trade_context_.GetReqInfoByClientToken(pAccept->m_ClientOrderToken);
        if (p)
        {
            p->market_token = pAccept->m_MarketOrderToken;
            p->order_status = MY_TNL_OS_REPORDED;
            trade_context_.SaveReqInfoOfMarketToken(p->market_token, p);

            T_OrderRespond rsp;
            RemStructConvert::OrderRespond(0, p->po.serial_no, 0, MY_TNL_OS_REPORDED, rsp);

            if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&rsp);
            LogUtil::OnOrderRespond(&rsp, tunnel_info_);

            TNL_LOG_DEBUG("MYRemTradeSpi::OnOrderAccept - %s", RemStructFormater::ToString(pAccept).c_str());
        }
        else
        {
            TNL_LOG_INFO("MYRemTradeSpi::OnOrderAccept - m_ClientOrderToken(%d) - can't get req info.", pAccept->m_ClientOrderToken);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("MYRemTradeSpi::OnOrderAccept - unknown exception.");
    }
}

void MYRemTradeSpi::OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept)
{
    try
    {
        if (!pAccept)
        {
            TNL_LOG_WARN("OnOrderMarketAccept, pAccept is null.");
            return;
        }

        const RemReqInfo *p = trade_context_.GetReqInfoByMarketToken(pAccept->m_MarketOrderToken);
        if (p)
        {
            p->entruct_no = atol(pAccept->m_MarketOrderId);
            p->order_status = MY_TNL_OS_REPORDED;

            T_OrderReturn rsp;
            RemStructConvert::OrderReturn(p, rsp);

            if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&rsp);
            LogUtil::OnOrderReturn(&rsp, tunnel_info_);

            TNL_LOG_DEBUG("MYRemTradeSpi::OnOrderMarketAccept - %s", RemStructFormater::ToString(pAccept).c_str());
        }
        else
        {
            TNL_LOG_INFO("MYRemTradeSpi::OnOrderMarketAccept - m_MarketOrderId(%lld) - can't get req info.", pAccept->m_MarketOrderId);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("MYRemTradeSpi::OnOrderMarketAccept - unknown exception.");
    }
}

void MYRemTradeSpi::OnOrderReject(EES_OrderRejectField* pReject)
{
    try
    {
    const RemReqInfo *p = trade_context_.GetReqInfoByClientToken(pReject->m_ClientOrderToken);

    if (p)
    {
        if (strlen(pReject->m_GrammerText) != 0)
        {
            ReportErrorState(pReject->m_ReasonCode, pReject->m_GrammerText);
        }
        else
        {
            ReportErrorState(pReject->m_ReasonCode, pReject->m_RiskText);
        }
        int my_err_no = RemFieldConvert::GetMYErrorNo(pReject->m_GrammerResult, pReject->m_RiskResult);
        p->order_status = MY_TNL_OS_ERROR;

        // 报单失败，报告合规检查
        ComplianceCheck_OnOrderInsertFailed(
            tunnel_info_.account.c_str(),
            p->po.serial_no,
            p->po.exchange_type,
            p->po.stock_code,
            p->po.volume,
            p->po.speculator,
            p->po.open_close,
            p->po.order_type);

        T_OrderRespond rsp;
        RemStructConvert::OrderRespond(my_err_no, p->po.serial_no, 0, MY_TNL_OS_ERROR, rsp);
        if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&rsp);
        LogUtil::OnOrderRespond(&rsp, tunnel_info_);

        TNL_LOG_WARN("MYRemTradeSpi::OnOrderReject - reject msg: grammer(%s),risk(%s)", pReject->m_GrammerText, pReject->m_RiskText);
        TNL_LOG_DEBUG("MYRemTradeSpi::OnOrderReject - %s", RemStructFormater::ToString(pReject).c_str());
    }
    else
    {
        TNL_LOG_INFO("MYRemTradeSpi::OnOrderReject - m_ClientOrderToken(%d) - can't get req info.", pReject->m_ClientOrderToken);
    }
    }
    catch (...)
    {
        TNL_LOG_ERROR("MYRemTradeSpi::OnOrderReject - unknown exception.");
    }
}

void MYRemTradeSpi::OnOrderMarketReject(EES_OrderMarketRejectField* pReject)
{
    try
    {
        const RemReqInfo *p = trade_context_.GetReqInfoByMarketToken(pReject->m_MarketOrderToken);

        if (p)
        {
            p->order_status = MY_TNL_OS_ERROR;

            // 报单失败，报告合规检查
            ComplianceCheck_OnOrderInsertFailed(
                tunnel_info_.account.c_str(),
                p->po.serial_no,
                p->po.exchange_type,
				p->po.stock_code,
                p->po.volume,
                p->po.speculator,
                p->po.open_close,
                p->po.order_type);

            T_OrderReturn rsp;
            RemStructConvert::OrderReturn(p, rsp);

            if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&rsp);
            LogUtil::OnOrderReturn(&rsp, tunnel_info_);

            TNL_LOG_WARN("MYRemTradeSpi::OnOrderMarketReject - reject msg: ", pReject->m_ReasonText);
            TNL_LOG_DEBUG("MYRemTradeSpi::OnOrderMarketReject - %s", RemStructFormater::ToString(pReject).c_str());
        }
        else
        {
            TNL_LOG_INFO("MYRemTradeSpi::OnOrderMarketReject - m_MarketOrderToken(%lld) - can't get req info.", pReject->m_MarketOrderToken);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("MYRemTradeSpi::OnOrderMarketReject - unknown exception.");
    }
}

void MYRemTradeSpi::OnOrderExecution(EES_OrderExecutionField* pf)
{
    try
    {
        const RemReqInfo *p = trade_context_.GetReqInfoByMarketToken(pf->m_MarketOrderToken);
        if (p)
        {
            p->order_status = MY_TNL_OS_PARTIALCOM;
            p->volume_remain -= pf->m_Quantity;

            if (p->volume_remain == 0) p->order_status = MY_TNL_OS_COMPLETED;

            // 全成，报告合规检查
            if (p->order_status == MY_TNL_OS_COMPLETED)
            {
                ComplianceCheck_OnOrderFilled(
                    tunnel_info_.account.c_str(),
                    p->po.serial_no);
            }

            // 委托回报
            T_OrderReturn order_return;
            RemStructConvert::OrderReturn(p, order_return);
            if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
            LogUtil::OnOrderReturn(&order_return, tunnel_info_);

            // match回报
            T_TradeReturn trade_return;
            RemStructConvert::TradeReturn(p, pf, trade_return);
            if (TradeReturn_call_back_handler_) TradeReturn_call_back_handler_(&trade_return);
            LogUtil::OnTradeReturn(&trade_return, tunnel_info_);

            TNL_LOG_DEBUG("MYRemTradeSpi::OnOrderExecution - %s", RemStructFormater::ToString(pf).c_str());
        }
        else
        {
            TNL_LOG_INFO("MYRemTradeSpi::OnOrderExecution - m_MarketOrderToken(%lld) - can't get req info.", pf->m_MarketOrderToken);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("MYRemTradeSpi::OnOrderExecution - unknown exception.");
    }
}

void MYRemTradeSpi::OnOrderCxled(EES_OrderCxled* pCxled)
{
    try
    {
        const RemReqInfo *p = trade_context_.GetReqInfoByMarketToken(pCxled->m_MarketOrderToken);

        if (p)
        {
            p->order_status = MY_TNL_OS_WITHDRAWED;

            ComplianceCheck_OnOrderCanceled(
                tunnel_info_.account.c_str(),
                p->po.stock_code,
                p->po.serial_no,
                p->po.exchange_type,
                p->po.volume,
                p->po.speculator,
                p->po.open_close);

            SerialNoType cancel_sn = trade_context_.GetCancelSn(pCxled->m_MarketOrderToken);
            if (cancel_sn != 0)
            {
                T_CancelRespond cancel_rsp;
                RemStructConvert::CancelRespond(0, cancel_sn, 0L, cancel_rsp);

                if (CancelRespond_call_back_handler_) CancelRespond_call_back_handler_(&cancel_rsp);
                LogUtil::OnCancelRespond(&cancel_rsp, tunnel_info_);
            }

            T_OrderReturn rsp;
            RemStructConvert::OrderReturn(p, rsp);

            if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&rsp);
            LogUtil::OnOrderReturn(&rsp, tunnel_info_);

            TNL_LOG_DEBUG("MYRemTradeSpi::OnOrderCxled - %s", RemStructFormater::ToString(pCxled).c_str());
        }
        else
        {
            TNL_LOG_INFO("MYRemTradeSpi::OnOrderCxled - m_MarketOrderToken(%lld) - can't get req info.", pCxled->m_MarketOrderToken);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("MYRemTradeSpi::OnOrderCxled - unknown exception.");
    }
}

void MYRemTradeSpi::OnCxlOrderReject(EES_CxlOrderRej* pReject)
{
    try
    {
        const RemReqInfo *p = trade_context_.GetReqInfoByMarketToken(pReject->m_MarketOrderToken);

        if (p)
        {
            ReportErrorState(pReject->m_ReasonCode, pReject->m_ReasonText);
            // 撤单拒绝，报告合规检查
            ComplianceCheck_OnOrderCancelFailed(
                tunnel_info_.account.c_str(),
                p->po.stock_code,
                p->po.serial_no);

            SerialNoType cancel_sn = trade_context_.GetCancelSn(pReject->m_MarketOrderToken);
            if (cancel_sn != 0)
            {
                int my_err = RemFieldConvert::GetMYErrorNoOfRejectReason(pReject->m_ReasonCode);
                T_CancelRespond cancel_rsp;
                RemStructConvert::CancelRespond(my_err, cancel_sn, 0L, cancel_rsp);

                if (CancelRespond_call_back_handler_) CancelRespond_call_back_handler_(&cancel_rsp);
                LogUtil::OnCancelRespond(&cancel_rsp, tunnel_info_);
            }

            TNL_LOG_DEBUG("MYRemTradeSpi::OnCxlOrderReject - %s", RemStructFormater::ToString(pReject).c_str());
        }
        else
        {
            TNL_LOG_INFO("MYRemTradeSpi::OnCxlOrderReject - m_MarketOrderToken(%lld) - can't get req info.", pReject->m_MarketOrderToken);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("MYRemTradeSpi::OnCxlOrderReject - unknown exception.");
    }
}

void MYRemTradeSpi::OnQueryTradeOrder(const char* pAccount, EES_QueryAccountOrder* pOrder, bool bFinish)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_DEBUG("MYRemTradeSpi::OnQueryTradeOrder: acc=%s, last_flag=%d\n - %s",
            pAccount, bFinish,
            RemStructFormater::ToString(pOrder).c_str());

        if (!finish_query_canceltimes_)
        {
            lock_guard<mutex> lock(stats_canceltimes_sync_);
            if (pOrder && !bFinish && IsClientCanceledOrder(*pOrder))
            {
                ContractCancelTimesMap::iterator it = cancel_times_of_contract.find(pOrder->m_symbol);
                if (it == cancel_times_of_contract.end())
                {
                    cancel_times_of_contract.insert(std::make_pair(pOrder->m_symbol, 1));
                }
                else
                {
                    ++it->second;
                }
            }

            if (bFinish)
            {
                for (ContractCancelTimesMap::iterator it = cancel_times_of_contract.begin();
                    it != cancel_times_of_contract.end(); ++it)
                {
                    ComplianceCheck_SetCancelTimes(tunnel_info_.account.c_str(), it->first.c_str(), it->second);
                }
                TNL_LOG_INFO("MYRemTradeSpi::OnQueryTradeOrder, stats canceltimes finished.");
                cancel_times_of_contract.clear();
                finish_query_canceltimes_ = true;
            }
        }

        if (!finish_query_orders_)
        {
            if (pOrder && !bFinish && IsExecutedOrder(*pOrder))
            {
                executed_orders_.insert(std::make_pair(pOrder->m_MarketOrderToken, *pOrder));
            }
            if (bFinish)
            {
                finish_query_orders_ = true;
            }
        }

        if (!have_handled_unterminated_orders_)
        {
            lock_guard<mutex> lock(cancel_sync_);
            if (pOrder && !bFinish && !IsOrderTerminate(*pOrder))
            {
                unterminated_orders_.push_back(pOrder->m_MarketOrderToken);
            }

            if (bFinish && unterminated_orders_.empty())
            {
                TNL_LOG_INFO("MYRemTradeSpi::OnQueryTradeOrder, terminate all active order finished.");
                have_handled_unterminated_orders_ = true;
            }

            if (bFinish)
            {
                TNL_LOG_INFO("MYRemTradeSpi::OnQueryTradeOrder, unterminated order count:%d.", unterminated_orders_.size());
                qry_order_finish_cond_.notify_one();
            }
        }

        if (TunnelIsReady())
        {
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
        }

        if (pOrder)
        {
            TNL_LOG_INFO("OnQueryTradeOrder when init tunnel, order: %lld, last: %d", pOrder->m_MarketOrderToken, bFinish);
        }
        else
        {
            TNL_LOG_INFO("OnQueryTradeOrder when init tunnel, order(null), last: %d", bFinish);
        }
        return;
    }

    T_OrderDetailReturn ret;

    TNL_LOG_DEBUG("MYRemTradeSpi::OnQueryTradeOrder: acc=%s, last_flag=%d\n - %s",
        pAccount, bFinish,
        RemStructFormater::ToString(pOrder).c_str());

    if (!bFinish)
    {
        OrderDetail od;
        strncpy(od.stock_code, pOrder->m_symbol, sizeof(od.stock_code));
        od.entrust_no = atol(pOrder->m_MarketOrderId);
        od.order_kind = MY_TNL_OPT_LIMIT_PRICE;
        od.direction = RemFieldConvert::GetMYDirection(pOrder->m_SideType);
        od.open_close = RemFieldConvert::GetMYOCflag(pOrder->m_SideType);
        od.speculator = RemFieldConvert::GetMYHedgeFlag(pOrder->m_HedgeFlag);
        od.entrust_status = RemFieldConvert::GetMYStatus(pOrder->m_OrderStatus);
        od.limit_price = pOrder->m_Price;
        od.volume = pOrder->m_Quantity;
        od.volume_traded = pOrder->m_FilledQty;
        od.volume_remain = pOrder->m_Quantity - pOrder->m_FilledQty;
        od_buffer_.push_back(od);
    }

    if (bFinish)
    {
        ret.datas.swap(od_buffer_);
        ret.error_no = 0;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}

void MYRemTradeSpi::OnQueryTradeOrderExec(const char* pAccount, EES_QueryOrderExecution* pTrade, bool bFinish)
{
    TNL_LOG_DEBUG("MYRemTradeSpi::OnQueryTradeOrderExec: acc=%s, last_flag=%d\n - %s",
        pAccount, bFinish,
        RemStructFormater::ToString(pTrade).c_str());

    T_TradeDetailReturn ret;

    if (pTrade && !bFinish)
    {
        OrderOfMTokenMap::iterator it = executed_orders_.find(pTrade->m_MarketOrderToken);
        if (it != executed_orders_.end())
        {
            TradeDetail td;
            strncpy(td.stock_code, it->second.m_symbol, sizeof(td.stock_code));
            td.entrust_no = atol(it->second.m_MarketOrderId);
            td.direction = RemFieldConvert::GetMYDirection(it->second.m_SideType);
            td.open_close = RemFieldConvert::GetMYOCflag(it->second.m_SideType);
            td.speculator = RemFieldConvert::GetMYHedgeFlag(it->second.m_HedgeFlag);
            td.trade_price = pTrade->m_ExecutionPrice;
            td.trade_volume = pTrade->m_ExecutedQuantity;
            struct tm tmResult;
            unsigned int nanoSsec;
            api_->ConvertFromTimestamp(pTrade->m_Timestamp, tmResult, nanoSsec);
            snprintf(td.trade_time, sizeof(td.trade_time), "%02d:%02d:%02d", tmResult.tm_hour, tmResult.tm_min, tmResult.tm_sec);
            td_buffer_.push_back(td);
        }
        else
        {
            TNL_LOG_INFO("MYRemTradeSpi::OnQueryTradeOrderExec, MToken(%lld) can't find order info", pTrade->m_MarketOrderToken);
        }
    }

    if (bFinish)
    {
        ret.datas.swap(td_buffer_);
        ret.error_no = 0;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

void MYRemTradeSpi::OnPostOrder(EES_PostOrder* pPostOrder)
{
    TNL_LOG_DEBUG("MYRemTradeSpi::OnPostOrder - %s",
        RemStructFormater::ToString(pPostOrder).c_str());
}

void MYRemTradeSpi::OnPostOrderExecution(EES_PostOrderExecution* pPostOrderExecution)
{
    TNL_LOG_DEBUG("MYRemTradeSpi::OnPostOrderExecution - %s",
        RemStructFormater::ToString(pPostOrderExecution).c_str());
}

void MYRemTradeSpi::OnQueryMarketSession(EES_ExchangeMarketSession* pMarketSession, bool bFinish)
{
    TNL_LOG_DEBUG("MYRemTradeSpi::OnQueryMarketSession: last_flag=%d\n - %s",
        bFinish,
        RemStructFormater::ToString(pMarketSession).c_str());
}

void MYRemTradeSpi::OnMarketSessionStatReport(EES_MarketSessionId MarketSessionId, bool ConnectionGood)
{
    TNL_LOG_DEBUG("MYRemTradeSpi::OnMarketSessionStatReport: MarketSessionId=%d, ConnectionGood=%d",
        MarketSessionId, ConnectionGood);
}

void MYRemTradeSpi::OnSymbolStatusReport(EES_SymbolStatus* pSymbolStatus)
{
    TNL_LOG_DEBUG("MYRemTradeSpi::OnSymbolStatusReport - %s",
        RemStructFormater::ToString(pSymbolStatus).c_str());
}

void MYRemTradeSpi::OnQuerySymbolStatus(EES_SymbolStatus* pSymbolStatus, bool bFinish)
{
    TNL_LOG_DEBUG("MYRemTradeSpi::OnQuerySymbolStatus: last_flag=%d\n - %s",
        bFinish,
        RemStructFormater::ToString(pSymbolStatus).c_str());
}

bool MYRemTradeSpi::IsOrderTerminate(const EES_QueryAccountOrder& order_field)
{
    return (order_field.m_OrderStatus & EES_OrderStatus_closed) == EES_OrderStatus_closed;
}

bool MYRemTradeSpi::IsClientCanceledOrder(const EES_QueryAccountOrder& order)
{
    return (order.m_OrderStatus & EES_OrderStatus_cxl_requested) == EES_OrderStatus_cxl_requested
        && (order.m_OrderStatus & EES_OrderStatus_cancelled) == EES_OrderStatus_cancelled;
}

bool MYRemTradeSpi::IsExecutedOrder(const EES_QueryAccountOrder& order_field)
{
    return (order_field.m_OrderStatus & EES_OrderStatus_executed) == EES_OrderStatus_executed;
}

void MYRemTradeSpi::QueryAndHandleOrders()
{
//超时后没有完成查询，重试。为防止委托单太多，10s都回报不了，每次超时加5s
    int wait_seconds = 10;

    std::vector<EES_MarketToken> unterminated_orders_t;
    while (!HaveFinishQueryOrders())
    {
        {
            std::unique_lock<std::mutex> lock(stats_canceltimes_sync_);
            cancel_times_of_contract.clear();
        }
        //主动查询所有报单
        while (true)
        {
            int qry_result = api_->QueryAccountOrder(tunnel_info_.account.c_str());
            if (qry_result != 0)
            {
                // retry if failed, wait some seconds
                sleep(2);
                continue;
            }
            TNL_LOG_INFO("QueryAccountOrder for init tunnel, return %d", qry_result);

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
            for (EES_MarketToken &order_field : unterminated_orders_t)
            {
                EES_CancelOrder action_field = CreatCancelParam(order_field);
                int ret = api_->CancelOrder(&action_field);
                if (ret != 0)
                {
                    TNL_LOG_WARN("when QueryAndHandleOrders, CancelOrder return %d", ret);
                }
                usleep(20 * 1000);
            }
            unterminated_orders_t.clear();

            //全部发送完毕后，等待 1s ， 判断 handle_flag , 如没有完成，则retry
            sleep(1);
        }
        else if (!finish_query_canceltimes_ || !finish_query_orders_)
        {
            // wait order query result return back
            sleep(wait_seconds);
            wait_seconds += 5;
        }
    }
}
