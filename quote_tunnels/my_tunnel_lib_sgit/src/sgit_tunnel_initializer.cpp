#include "sgit_tunnel_initializer.h"
#include "qtm_with_code.h"

using namespace std;

TunnelInitializer::TunnelInitializer(const TunnelConfigData& cfg, CSgitFtdcTraderApi *api)
{
    api_ = api;
    elapase_seconds_ = 0;
    user_ = cfg.Logon_config().clientid;
    tunnel_info_.account = cfg.Logon_config().investorid;
    user_id_len_ = user_.length();

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "sgit_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

    exchange_code_ = cfg.Logon_config().exch_code[0];

    const char *td = api_->GetTradingDay();
    strncpy(trade_day, td, sizeof(trade_day));
    TNL_LOG_INFO("GetTradingDay:%s", trade_day);

    finished_ = false;
    finish_query_account_ = false;
    finish_query_contract_ = false;
    finish_query_position_ = false;

    finish_cancel_active_orders_ = true;
    if (cfg.Initial_policy_param().cancel_orders_at_start)
    {
        finish_cancel_active_orders_ = false;
    }

    finish_stats_cancel_times_ = false;
    if (cfg.Compliance_check_param().init_cancel_times_at_start == 0)
    {
        finish_stats_cancel_times_ = true;
    }

    available_fund_ = 0;
}

TunnelInitializer::~TunnelInitializer(void)
{
}

void TunnelInitializer::StartInit()
{
    std::thread init_thread(&TunnelInitializer::TaskSchedule, this);
    init_thread.detach();
}

void TunnelInitializer::TaskSchedule()
{
    // rebuild flow, to get all orders
    ResetTimer();
    while (!IsTimeout(3))
    {
        sleep(1);
        IncTimer();
    }

    TNL_LOG_INFO("have get all orders, size:%d", orders_.size());

    // cancel active orders
    if (!finish_cancel_active_orders_)
    {
        TNL_LOG_INFO("try to cancel all active order.");
        if (NoActiveOrder())
        {
            finish_cancel_active_orders_ = true;
        }
        else
        {
            CancelAllActiveOrders();
            ResetTimer();
            while (!IsTimeout(10))
            {
                sleep(1);
                IncTimer();

                if (NoActiveOrder())
                {
                    TNL_LOG_INFO("cancel active order finished, no active order now");
                    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::HANDLED_UNTERMINATED_ORDERS_SUCCESS);
                    finish_cancel_active_orders_ = true;
                    break;
                }
            }

            if (!finish_cancel_active_orders_)
            {
                TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::HANDLED_UNTERMINATED_ORDERS_SUCCESS);
                finish_cancel_active_orders_ = true;
                TNL_LOG_WARN("cancel active order finished, because time out");
            }
        }
    }

    // stats cancel times of each contract
    if (!finish_stats_cancel_times_)
    {
        std::lock_guard<std::mutex> lock(sync_lock_);
        for (std::map<OrderRefDataType, OrderDetail>::value_type& v : orders_)
        {
            if (v.second.entrust_status == MY_TNL_OS_WITHDRAWED)
            {
                std::map<std::string, int>::iterator it = cancel_times_of_contract.find(v.second.stock_code);
                if (it == cancel_times_of_contract.end())
                {
                    cancel_times_of_contract.insert(std::make_pair(v.second.stock_code, 1));
                }
                else
                {
                    ++it->second;
                }
            }
        }

        for (std::map<std::string, int>::iterator it = cancel_times_of_contract.begin();
            it != cancel_times_of_contract.end(); ++it)
        {
            ComplianceCheck_SetCancelTimes(tunnel_info_.account.c_str(), it->first.c_str(), it->second);
        }
        cancel_times_of_contract.clear();
        finish_stats_cancel_times_ = true;
    }

    // buffer orders
    {
        std::lock_guard<std::mutex> lock(sync_lock_);
        for (std::map<OrderRefDataType, OrderDetail>::value_type& v : orders_)
        {
            od_buffer_.push_back(v.second);
        }
    }

    // query contract
    QueryContractInfo();
    ResetTimer();
    while (!IsTimeout(10))
    {
        sleep(1);
        IncTimer();

        if (finish_query_contract_)
        {
            TNL_LOG_INFO("query contract finished");
            break;
        }
    }

    if (!finish_query_contract_)
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_CONTRACT_SUCCESS);
        finish_query_contract_ = true;
        TNL_LOG_WARN("query contract finished, because time out");
    }

    // query position
    QueryPosition();
    ResetTimer();
    while (!IsTimeout(10))
    {
        sleep(1);
        IncTimer();

        if (finish_query_position_)
        {
            TNL_LOG_INFO("query position finished");
            break;
        }
    }

    if (!finish_query_position_)
    {
        finish_query_position_ = true;
        TNL_LOG_WARN("query position finished, because time out");
    }

    finished_ = true;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
}

void TunnelInitializer::RecvRtnOrder(CSgitFtdcOrderField* pf, CSgitFtdcRspInfoField* rsp)
{
    ResetTimer();
    if (IsErrRespond(rsp))
    {
        TNL_LOG_INFO("RecvRtnOrder - error order,ErrorID:%d, ErrorMsg:%s", rsp->ErrorID, rsp->ErrorMsg);
        return;
    }
    if (pf && IsMineReturn(pf->UserID) && IsTodayOperation(pf->GTDDate))
    {
        AddOrder(*pf);
    }
}

void TunnelInitializer::RecvRtnTrade(CSgitFtdcTradeField* pf)
{
    ResetTimer();
    if (pf && IsMineReturn(pf->UserID))
    {
        if (UpdateOrderMatchedVolume(atoll(pf->OrderLocalID), pf->Volume))
        {
            AddTrade(*pf);
        }
    }
}

void TunnelInitializer::RecvRspOrderAction(CSgitFtdcInputOrderActionField* pf, CSgitFtdcRspInfoField* rsp)
{
    ResetTimer();
    if (IsErrRespond(rsp))
    {
        TNL_LOG_INFO("RecvRspOrderAction - error order,ErrorID:%d, ErrorMsg:%s", rsp->ErrorID, rsp->ErrorMsg);
        return;
    }
    if (pf && IsMineReturn(pf->UserID))
    {
        (void) UpdateOrderStatus(atoll(pf->OrderRef), MY_TNL_OS_WITHDRAWED);
    }
}

void TunnelInitializer::RecvRspQryTradingAccount(CSgitFtdcTradingAccountField* pf, CSgitFtdcRspInfoField* rsp, int req_id, bool is_last)
{
    if (finish_query_account_)
    {
//        int error_id = 0;
//        if (rsp && rsp->ErrorID != 0)
//        {
//            error_id = rsp->ErrorID;
//        }
//
//        if (pf)
//        {
//            TNL_LOG_DEBUG("OnRspQryTradingAccount: requestid:%d, error_id:%d, last_flag:%d, "
//                "Account:%s,PreBalance:%.2f,Available:%.2f,Balance:%.2f,CurrMargin:%.2f,CloseProfit:%.2f,PositionProfit:%.2f",
//                req_id, error_id, is_last,
//                pf->AccountID, pf->PreBalance, pf->Available, pf->Balance, pf->CurrMargin, pf->CloseProfit, pf->PositionProfit);
//        }
//        else
//        {
//            TNL_LOG_DEBUG("OnRspQryTradingAccount: requestid:%d, error_id:%d, last_flag:%d", req_id, error_id, is_last);
//        }
        return;
    }

    TNL_LOG_DEBUG("OnRspQryTradingAccount: requestid = %d, last_flag=%d %s%s",
        req_id, is_last,
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());

    if (pf)
    {
        available_fund_ = pf->Available;
    }

    if (is_last)
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_ACCOUNT_SUCCESS);
        finish_query_account_ = true;
    }
}

void TunnelInitializer::CancelAllActiveOrders()
{
    std::vector<std::pair<OrderRefDataType, OrderDetail> > unterminated_orders_t;
    {
        std::lock_guard<std::mutex> lock(sync_lock_);
        for (std::map<OrderRefDataType, OrderDetail>::value_type& v : orders_)
        {
            if (!IsTerminateStatus(v.second.entrust_status))
            {
                unterminated_orders_t.push_back(std::make_pair(v.first, v.second));
            }
        }
    }
    TNL_LOG_INFO("CancelAllActiveOrders, %d active order", unterminated_orders_t.size());

    // 遍历 unterminated_orders 间隔20ms（流控要求），发送撤单请求
    for (const std::pair<OrderRefDataType, OrderDetail> &v : unterminated_orders_t)
    {
        CSgitFtdcInputOrderActionField action_field = CreatCancelParam(v.first, v.second);
        int ret = api_->ReqOrderAction(&action_field, 0);
        TNL_LOG_INFO("in CancelUnterminatedOrders, ReqOrderAction return %d", ret);
        usleep(20 * 1000);
    }
}

CSgitFtdcInputOrderActionField TunnelInitializer::CreatCancelParam(OrderRefDataType order_ref, const OrderDetail& order_field)
{
    CSgitFtdcInputOrderActionField cancle_order;
    memset(&cancle_order, 0, sizeof(cancle_order));

    cancle_order.ExchangeID[0] = FieldConvert::GetApiExID(exchange_code_);

    std::map<OrderRefDataType, CSgitFtdcOrderField>::iterator it = original_orders_.find(order_ref);
    if (it != original_orders_.end())
    {
        memcpy(cancle_order.OrderSysID, it->second.OrderSysID, sizeof(cancle_order.OrderSysID));
    }
    snprintf(cancle_order.OrderRef, sizeof(cancle_order.OrderRef), "%012lld", order_ref);
    memcpy(cancle_order.BrokerID, broker_id_.c_str(), sizeof(cancle_order.BrokerID));
    memcpy(cancle_order.InvestorID, tunnel_info_.account.c_str(), sizeof(cancle_order.InvestorID));

    strncpy(cancle_order.UserID, user_.c_str(), sizeof(cancle_order.UserID));
    strncpy(cancle_order.InstrumentID, order_field.stock_code, sizeof(cancle_order.InstrumentID));

    cancle_order.OrderActionRef = 0;
    cancle_order.ActionFlag = Sgit_FTDC_AF_Delete;

    return cancle_order;
}

void TunnelInitializer::RecvRspQryInstrument(CSgitFtdcInstrumentField* pf, CSgitFtdcRspInfoField* rsp, int req_id, bool is_last)
{
    ResetTimer();
    // respond error
    if (rsp && rsp->ErrorID != 0)
    {
        // TODO try query again

        TNL_LOG_WARN("RecvRspQryInstrument, return error, ErrorID:%d, ErrorMsg:%s", rsp->ErrorID, rsp->ErrorMsg);
        return;
    }

    TNL_LOG_DEBUG("OnRspQryInstrument: requestid = %d, last_flag=%d", req_id, is_last);

    // respond error
    if (rsp && rsp->ErrorID != 0)
    {
        return;
    }

    if (pf)
    {
        ContractInfo ci;
        strncpy(ci.stock_code, pf->InstrumentID, sizeof(ci.stock_code));
        memcpy(ci.TradeDate, trade_day, sizeof(ci.TradeDate));
        memcpy(ci.ExpireDate, pf->ExpireDate, sizeof(ci.ExpireDate));
        ci_buffer_.push_back(ci);
    }

    if (is_last)
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_CONTRACT_SUCCESS);
        finish_query_contract_ = true;
        TNL_LOG_DEBUG("finish OnRspQryInstrument, data size:%d", ci_buffer_.size());
    }
}

void TunnelInitializer::RecvRspQryInvestorPosition(CSgitFtdcInvestorPositionField* pf, CSgitFtdcRspInfoField* rsp, int req_id, bool is_last)
{
    ResetTimer();
    T_PositionReturn ret;

    // respond error
    if (rsp && rsp->ErrorID != 0)
    {
        // TODO try query again

        TNL_LOG_WARN("OnRspQryInvestorPosition, return error, ErrorID:%d, ErrorMsg:%s", rsp->ErrorID, rsp->ErrorMsg);
        return;
    }

    // empty positioon
    if (pf)
    {
        PositionDetail pos;
        strncpy(pos.stock_code, pf->InstrumentID, sizeof(pos.stock_code));

        pos.direction = MY_TNL_D_BUY;
        if (pf->PosiDirection == Sgit_FTDC_PD_Long)
        {
            pos.direction = MY_TNL_D_BUY;
        }
        else if (pf->PosiDirection == Sgit_FTDC_PD_Short)
        {
            pos.direction = MY_TNL_D_SELL;
        }
        else if (pf->PosiDirection == Sgit_FTDC_PD_Net)
        {
            if (pos.position < 0) pos.direction = MY_TNL_D_SELL;
        }
        else
        {
            TNL_LOG_WARN("OnRspQryInvestorPosition, return unrecognisable direction:0x%02x", pf->PosiDirection);
        }
        pos.position = pf->Position;
        pos.position_avg_price = 0;
        if (pos.position > 0) pos.position_avg_price = pf->PositionCost;

        pos.yestoday_position = pf->YdPosition;
        pos.yd_position_avg_price = 0;
        if (pos.yestoday_position > 0) pos.yd_position_avg_price = pf->PositionCost;

        if (pos.position > 0 || pos.yestoday_position > 0)
        {
            pos_buffer_.push_back(pos);
        }
    }

    if (is_last)
    {
        finish_query_position_ = true;
    }

    TNL_LOG_DEBUG("OnRspQryInvestorPosition: requestid = %d, last_flag=%d %s%s",
        req_id, is_last,
        DatatypeFormater::ToString(pf).c_str(),
        DatatypeFormater::ToString(rsp).c_str());
}

void TunnelInitializer::QueryPosition()
{
    // query instrument
    CSgitFtdcQryInvestorPositionField qry_param;
    memset(&qry_param, 0, sizeof(qry_param));
    ///经纪公司编号
    strncpy(qry_param.BrokerID, broker_id_.c_str(), sizeof(qry_param.BrokerID));
    ///投资者编号
    strncpy(qry_param.InvestorID, tunnel_info_.account.c_str(), sizeof(qry_param.InvestorID));

    while (true)
    {
        int ret = api_->ReqQryInvestorPosition(&qry_param, 0);
        TNL_LOG_INFO("QryPosition, return: %d", ret);

        if (ret == 0)
        {
            break;
        }
        sleep(1);
    }
}

PositionDetail TunnelInitializer::PackageAvailableFunc()
{
    // insert available fund
    PositionDetail ac;
    memset(&ac, 0, sizeof(ac));
    strcpy(ac.stock_code, "#CASH");
    ac.direction = MY_TNL_D_BUY;
    ac.position = (long) available_fund_;

    return ac;
}

void TunnelInitializer::QueryContractInfo()
{
    // query instrument
    CSgitFtdcQryInstrumentField qry_param;
    memset(&qry_param, 0, sizeof(qry_param));
    qry_param.ExchangeID[0] = FieldConvert::GetApiExID(exchange_code_);

    while (true)
    {
        int ret = api_->ReqQryInstrument(&qry_param, 0);
        TNL_LOG_INFO("ReqQryInstrument, return: %d", ret);

        if (ret == 0)
        {
            break;
        }
        sleep(1);
    }
}
