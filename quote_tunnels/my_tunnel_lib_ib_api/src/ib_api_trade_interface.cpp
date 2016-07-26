#include "ib_api_trade_interface.h"
#include <string.h>
#include <iostream>
#include <thread>
#include <stdint.h>

#include "Execution.h"
#include "OrderState.h"
#include "CommissionReport.h"

#include "ib_api_context.h"
#include "my_cmn_log.h"
#include "my_cmn_util_funcs.h"
#include "check_schedule.h"
#include "qtm_with_code.h"

using namespace std;
using namespace my_cmn;

inline double Invalid2Nan(double d)
{
    if (d < -999999999 || d > 999999999)
    {
        return -1;
    }

    return d;
}

MYIBApiTunnel::MYIBApiTunnel(const TunnelConfigData& cfg)
    : api_client_(new EPosixClientSocket(this)), cfg_(cfg)
{
    is_connected_ = false;
    run_flag_ = true;
    cancel_active_order_finished_ = true;

    gateway_port_ = 0;
    gateway_clientid_ = 0;
    in_query_flag = false;

    ParseConfig();

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "ibapi_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

    // IB tunnel don't support feature of cancel all active orders
    if (cfg_.Initial_policy_param().cancel_orders_at_start)
    {
        //cancel_active_order_finished_ = false;
        TNL_LOG_WARN("don't support feature of cancel all active orders");
    }

    time_to_query_pos_int = 0;
    query_position_step = 0;
    if (!cfg_.Initial_policy_param().time_to_query_pos.empty())
    {
        time_to_query_pos_int = atoi(cfg_.Initial_policy_param().time_to_query_pos.c_str());
    }

    if (time_to_query_pos_int > 0)
    {
        std::thread thread_tmp(&MYIBApiTunnel::RecordPositionAtSpecTime, this);
        thread_tmp.detach();
    }

    std::thread t_connect(&MYIBApiTunnel::Start, this);
    t_connect.detach();
}

MYIBApiTunnel::~MYIBApiTunnel(void)
{
    run_flag_ = false;
    api_client_->eDisconnect();
}

void MYIBApiTunnel::ParseConfig()
{
    tunnel_info_.account = cfg_.Logon_config().investorid;
    gateway_clientid_ = cfg_.App_cfg().orderref_prefix_id;

    if (!cfg_.Logon_config().front_addrs.empty())
    {
        IPAndPortNum ip_port = ParseIPAndPortNum(cfg_.Logon_config().front_addrs.front());
        gateway_ip_ = ip_port.first;
        gateway_port_ = ip_port.second;
    }
    else
    {
        gateway_ip_ = "127.0.0.1";
        gateway_port_ = 4001;
        TNL_LOG_ERROR("must set front address in config file");
    }
}

void MYIBApiTunnel::Start()
{
    TNL_LOG_INFO("prepare to connect to gateway, Host Address: %s:%d, clientId: %d", gateway_ip_.c_str(), gateway_port_, gateway_clientid_);

    while (run_flag_)
    {
        while (!is_connected_)
        {
            (void) api_client_->eConnect(gateway_ip_.c_str(), gateway_port_, gateway_clientid_, false);
            if (api_client_->isConnected())
            {
                is_connected_ = true;
                TNL_LOG_INFO("connect to gateway successful.");
                break;
            }

            TNL_LOG_WARN("connect to gateway failed, retry after a while.");
            sleep(10);
        }

        if (!cancel_active_order_finished_)
        {
            std::thread qry_order(&MYIBApiTunnel::QryOrdersHander, this);
            qry_order.detach();
        }
        else
        {
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
        }

        while (true)
        {
            api_client_->checkMessages();
            //usleep(100);
            if (!api_client_->isConnected())
            {
                TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
                TNL_LOG_ERROR("connection to IB gateway is lost.");
                is_connected_ = false;
                break;
            }
        }
    }
}

int MYIBApiTunnel::ReqOrderInsert(const T_PlaceOrder* po)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("ReqOrderInsert when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }

    int ret = TUNNEL_ERR_CODE::RESULT_SUCCESS;
    try
    {
        Order order;
        Contract contract;
        T_OrderRespond order_res;

        if (trade_ctx_.CreateNewOrder(po, GetOrderId(), order, contract, order_res))
        {
            api_client_->placeOrder(order.orderId, contract, order);
        }

        //if (OrderRespondHandler_) OrderRespondHandler_(&order_res);

        //LogUtil::OnOrderRespond(&order_res, tunnel_info_);
    }
    catch (...)
    {
        ret = TUNNEL_ERR_CODE::RESULT_FAIL;
    }

    return ret;
}

int MYIBApiTunnel::ReqOrderAction(const T_CancelOrder* co)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("ReqOrderAction when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }

    int ret = TUNNEL_ERR_CODE::RESULT_SUCCESS;
    const TnlOrderInfo *p = trade_ctx_.GetOrderBySn(co->org_serial_no);
    if (p)
    {
        long order_id = p->order_id;
        p->last_cancel_sn = co->serial_no;

        api_client_->cancelOrder(order_id);

        //T_CancelRespond cancel_respond;
        //trade_ctx_.BuildCancelRsp(p, TUNNEL_ERR_CODE::RESULT_SUCCESS, cancel_respond);

        //if (CancelRespondHandler_) CancelRespondHandler_(&cancel_respond);

        //LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
    }
    else
    {
        ret = TUNNEL_ERR_CODE::ORDER_NOT_FOUND;
    }

    return ret;
}

int MYIBApiTunnel::QryPosition(const T_QryPosition* p)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("QryPosition when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    if (in_query_flag)
    {
        TNL_LOG_WARN("QryPosition when tunnel is in query state");
        return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
    }

    int ret = TUNNEL_ERR_CODE::RESULT_SUCCESS;
    try
    {
        in_query_flag = true;
        pos_buffer_.clear();
        api_client_->reqAccountUpdates(true, tunnel_info_.account);
    }
    catch (...)
    {
        ret = TUNNEL_ERR_CODE::RESULT_FAIL;
    }

    return ret;
}

int MYIBApiTunnel::QryOrderDetail(const T_QryOrderDetail* p)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("QryOrderDetail when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    if (in_query_flag)
    {
        TNL_LOG_WARN("QryOrderDetail when tunnel is in query state");
        return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
    }

    int ret = TUNNEL_ERR_CODE::RESULT_SUCCESS;
    try
    {
        in_query_flag = true;
        od_buffer_.clear();
        api_client_->reqOpenOrders();
    }
    catch (...)
    {
        ret = TUNNEL_ERR_CODE::RESULT_FAIL;
    }

    return ret;
}

int MYIBApiTunnel::QryTradeDetail(const T_QryTradeDetail* p)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("QryTradeDetail when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    if (in_query_flag)
    {
        TNL_LOG_WARN("QryTradeDetail when tunnel is in query state");
        return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
    }

    int ret = TUNNEL_ERR_CODE::RESULT_SUCCESS;
    try
    {
        in_query_flag = true;
        td_buffer_.clear();
        int req_id = trade_ctx_.GetNewReqID();
        ExecutionFilter filter;
        api_client_->reqExecutions(req_id, filter);
    }
    catch (...)
    {
        ret = TUNNEL_ERR_CODE::RESULT_FAIL;
    }

    return ret;
}

int MYIBApiTunnel::QryContractInfo(T_ContractInfoReturn &c_ret)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("QryContractInfo when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }

    trade_ctx_.GetContractInfo(c_ret);

    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}

void MYIBApiTunnel::error(const int id, const int errorCode, const IBString errorString)
{
    TNL_LOG_DEBUG("error - id:%d, errorCode:%d, errorString:%s", id, errorCode, errorString.c_str());

    if (id == -1 && errorCode == 1100) // if "Connectivity between IB and TWS has been lost"
    {
        api_client_->eDisconnect();
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
        TNL_LOG_ERROR("Connectivity between IB and TWS has been lost");
        return;
    }

    const TnlOrderInfo *p = trade_ctx_.GetOrderByOrderID(id);
    if (p)
    {
        // terminated order, discard error respond
        if (p->entrust_status == MY_TNL_OS_ERROR || p->entrust_status == MY_TNL_OS_WITHDRAWED || p->entrust_status == MY_TNL_OS_COMPLETED)
        {
            return;
        }

        //error codes that don't need to report
        if (errorCode == 161 || errorCode == 399)
        {
            return;
        }

        T_OrderReturn order_return;
        p->entrust_status = MY_TNL_OS_ERROR;
        if (errorCode == 201)
        {
            p->entrust_status = MY_TNL_OS_ERROR; // order rejected

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
        }
        else if (errorCode == 202)
        {
            p->entrust_status = MY_TNL_OS_WITHDRAWED; // order canceled

            // 已撤，报告合规检查
            ComplianceCheck_OnOrderCanceled(
                tunnel_info_.account.c_str(),
                p->po.stock_code,
                p->po.serial_no,
                p->po.exchange_type,
                p->volume_remain,
                p->po.speculator,
                p->po.open_close);
        }

        trade_ctx_.BuildOrderRtn(p, order_return);

        if (OrderReturnHandler_) OrderReturnHandler_(&order_return);

        LogUtil::OnOrderReturn(&order_return, tunnel_info_);
    }
}

void MYIBApiTunnel::contractDetails(int reqId, const ContractDetails& contractDetails)
{
    TNL_LOG_DEBUG("contractDetails");
}

void MYIBApiTunnel::bondContractDetails(int reqId, const ContractDetails& contractDetails)
{
    TNL_LOG_DEBUG("bondContractDetails");
}

void MYIBApiTunnel::contractDetailsEnd(int reqId)
{
    TNL_LOG_DEBUG("contractDetailsEnd");
}

void MYIBApiTunnel::position(const IBString& account, const Contract& contract, int position, double avgCost)
{
    TNL_LOG_DEBUG("position - account:%s,symbol:%s,position:%d,avgCost:%.4f",
        account.c_str(), contract.symbol.c_str(), position, Invalid2Nan(avgCost));
}

void MYIBApiTunnel::positionEnd()
{
    TNL_LOG_DEBUG("positionEnd");
}

void MYIBApiTunnel::accountSummary(int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency)
{
    TNL_LOG_DEBUG("accountSummary - reqId:%d,account:%s,tag:%s,value:%s,curency:%s",
        reqId, account.c_str(), tag.c_str(), value.c_str(), curency.c_str());
}

void MYIBApiTunnel::accountSummaryEnd(int reqId)
{
    TNL_LOG_DEBUG("accountSummaryEnd - reqId:%d", reqId);
}

void MYIBApiTunnel::execDetails(int reqId, const Contract& c, const Execution& exe)
{
    TNL_LOG_DEBUG("execDetails - reqId:%d, contract(%s-%s-%s-%s-%s)"
        "execution(%s-%d-%d-%d-%s-%s-%d-%.4f-%s-%s)",
        reqId, c.symbol.c_str(), c.exchange.c_str(), c.secType.c_str(), c.currency.c_str(), c.expiry.c_str(),
        exe.acctNumber.c_str(), exe.permId, exe.clientId, exe.orderId,
        exe.orderRef.c_str(), exe.execId.c_str(), exe.shares, exe.price,
        exe.side.c_str(), exe.exchange.c_str());

    std::string symbol_market = trade_ctx_.GetSymbolOnMarket(c);
    if (symbol_market.empty())
    {
        symbol_market = c.symbol;
        TNL_LOG_ERROR("can't get symbol by contract(%s-%s-%s-%s-%s), use IB symbol for position data",
            c.symbol.c_str(), c.exchange.c_str(), c.secType.c_str(), c.currency.c_str(), c.expiry.c_str());
    }

    TradeDetail td;
    strncpy(td.stock_code, symbol_market.c_str(), sizeof(td.stock_code));
    td.entrust_no = exe.orderId;
    td.open_close = MY_TNL_D_OPEN;
    if (exe.side == "BOT")
    {
        td.direction = MY_TNL_D_BUY;
    }
    else if (exe.side == "SLD")
    {
        td.direction = MY_TNL_D_SELL;
        if (c.secType == "STK")
        {
            td.open_close = MY_TNL_D_CLOSE;
        }
    }
    td.speculator = MY_TNL_HF_SPECULATION;
    td.trade_price = exe.price;
    td.trade_volume = exe.shares;
    strncpy(td.trade_time, "", sizeof(td.trade_time));

    td_buffer_.push_back(td);
}

void MYIBApiTunnel::execDetailsEnd(int reqId)
{
    TNL_LOG_DEBUG("execDetailsEnd");

    in_query_flag = false;

    // send trade data to client
    T_TradeDetailReturn ret;
    ret.datas.swap(td_buffer_);
    ret.error_no = 0;

    if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
    LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
}

void MYIBApiTunnel::orderStatus(OrderId orderId, const IBString& status, int filled, int remaining, double avgFillPrice, int permId, int parentId,
    double lastFillPrice, int clientId, const IBString& whyHeld)
{
    TNL_LOG_DEBUG("orderStatus - orderId:%ld, status:%s, filled:%d, remaining:%d, avgFillPrice:%.4f, "
        "permId:%d, parentId:%d, lastFillPrice:%.4f, clientId:%d, whyHeld:%s",
        orderId, status.c_str(), filled, remaining, avgFillPrice,
        permId, parentId, lastFillPrice, clientId, whyHeld.c_str());

    const TnlOrderInfo *p = trade_ctx_.GetOrderByOrderID(orderId);
    if (p)
    {
        // duplicate orderStatus
        if (filled > 0 && p->volume_remain == remaining)
        {
            return;
        }

        // terminated order, discard order status report
        if (p->entrust_status == MY_TNL_OS_ERROR || p->entrust_status == MY_TNL_OS_WITHDRAWED || p->entrust_status == MY_TNL_OS_COMPLETED)
        {
            return;
        }

        // update order
        int filled_this_time = p->volume_remain - remaining;
        p->entrust_status = MY_TNL_OS_REPORDED;
        p->volume_remain = remaining;
        if (filled_this_time > 0) p->entrust_status = MY_TNL_OS_PARTIALCOM;
        if (remaining == 0) p->entrust_status = MY_TNL_OS_COMPLETED;

        if (p->entrust_status == MY_TNL_OS_COMPLETED)
        {
            ComplianceCheck_OnOrderFilled(
                tunnel_info_.account.c_str(),
                p->po.serial_no);
        }

        T_OrderReturn order_return;
        trade_ctx_.BuildOrderRtn(p, order_return);

        if (OrderReturnHandler_) OrderReturnHandler_(&order_return);

        LogUtil::OnOrderReturn(&order_return, tunnel_info_);

        if (filled_this_time > 0)
        {
            T_TradeReturn trade_return;
            trade_ctx_.BuildTradeRtn(p, filled_this_time, lastFillPrice, trade_return);
            if (TradeReturnHandler_) TradeReturnHandler_(&trade_return);
            LogUtil::OnTradeReturn(&trade_return, tunnel_info_);
        }
    }
}

void MYIBApiTunnel::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute)
{
    TNL_LOG_DEBUG("tickPrice");
}

void MYIBApiTunnel::tickSize(TickerId tickerId, TickType field, int size)
{
    TNL_LOG_DEBUG("tickSize");
}

void MYIBApiTunnel::tickOptionComputation(TickerId tickerId, TickType tickType, double impliedVol, double delta, double optPrice, double pvDividend,
    double gamma, double vega, double theta, double undPrice)
{
    TNL_LOG_DEBUG("tickOptionComputation");
}

void MYIBApiTunnel::tickGeneric(TickerId tickerId, TickType tickType, double value)
{
    TNL_LOG_DEBUG("tickGeneric");
}

void MYIBApiTunnel::tickString(TickerId tickerId, TickType tickType, const IBString& value)
{
    TNL_LOG_DEBUG("tickString");
}

void MYIBApiTunnel::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints, double totalDividends,
    int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry)
{
    TNL_LOG_DEBUG("tickEFP");
}

void MYIBApiTunnel::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& order_state)
{
    TNL_LOG_DEBUG("openOrder - orderId:%ld, contract:%s, "
        "order_commission:%.4f, order_commission_currency:%s"
        "order_status:%s, order_warntxt:%s",
        orderId, contract.symbol.c_str(),
        Invalid2Nan(order_state.commission), order_state.commissionCurrency.c_str(),
        order_state.status.c_str(), order_state.warningText.c_str());

    if (!cancel_active_order_finished_)
    {
        if (orderId >= 0 && order_state.status == "Submitted")
        {
            std::unique_lock<std::mutex> lock(qry_order_result_stats_sync_);
            active_orders_.push_back(orderId);
        }
    }
}

void MYIBApiTunnel::openOrderEnd()
{
    TNL_LOG_DEBUG("openOrderEnd");

    if (!cancel_active_order_finished_)
    {
        if (active_orders_.empty())
        {
            cancel_active_order_finished_ = true;
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_CANCEL_TIMES_SUCCESS);
        }
        qry_order_finish_cond_.notify_one();
        return;
    }

    in_query_flag = false;
    T_OrderDetailReturn ret;
    ret.error_no = 0;
    if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
    LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
}

void MYIBApiTunnel::winError(const IBString& str, int lastError)
{
    TNL_LOG_DEBUG("winError - str:%s", str.c_str());
}

void MYIBApiTunnel::connectionClosed()
{
    is_connected_ = false;
    TNL_LOG_DEBUG("connectionClosed");
}

void MYIBApiTunnel::updateAccountValue(const IBString& key, const IBString& val, const IBString& currency, const IBString& accountName)
{
    TNL_LOG_DEBUG("updateAccountValue - key:%s,val:%s,currency:%s,accountName:%s",
        key.c_str(), val.c_str(), currency.c_str(), accountName.c_str());

    if (key == "AvailableFunds")
    {
        available_fund = atof(val.c_str());
        char buf[256];
        sprintf(buf, "AvailableFunds:%s,currency:%s", val.c_str(), currency.c_str());
        fund_info_ = buf;
    }
}

void MYIBApiTunnel::updatePortfolio(const Contract& c, int position, double marketPrice, double marketValue, double averageCost,
    double unrealizedPNL, double realizedPNL, const IBString& accountName)
{
    TNL_LOG_DEBUG("updatePortfolio - c(%s-%s-%s-%s-%s),position:%d,price:%.4f,cost:%.4f,accountName:%s",
        c.symbol.c_str(), c.primaryExchange.c_str(), c.secType.c_str(), c.currency.c_str(), c.expiry.c_str(),
        position, marketPrice, averageCost, accountName.c_str());

    std::string symbol_market = trade_ctx_.GetSymbolOnMarket(c);
    if (symbol_market.empty())
    {
        symbol_market = c.symbol;
        TNL_LOG_ERROR("can't get symbol by contract(%s-%s-%s-%s-%s), use IB symbol for position data",
            c.symbol.c_str(), c.primaryExchange.c_str(), c.secType.c_str(), c.currency.c_str(), c.expiry.c_str());
    }

    PositionDetail pos;
    bzero(&pos, sizeof(pos));
    strncpy(pos.stock_code, symbol_market.c_str(), sizeof(pos.stock_code));
    pos.exchange_type = trade_ctx_.GetExchangeID(c.primaryExchange);
    if (position > 0)
    {
        pos.direction = MY_TNL_D_BUY;
        pos.position = position;
        pos.position_avg_price = averageCost;
    }
    else if (position < 0)
    {
        pos.direction = MY_TNL_D_SELL;
        pos.position = -position;
        pos.position_avg_price = averageCost;
    }
    else
    {
        return;
    }

    pos_buffer_.push_back(pos);
}

void MYIBApiTunnel::updateAccountTime(const IBString& timeStamp)
{
    TNL_LOG_DEBUG("updateAccountTime - timeStamp:%s", timeStamp.c_str());
}

PositionDetail MYIBApiTunnel::PackageAvailableFunc()
{
    // insert available fund
    PositionDetail ac;
    memset(&ac, 0, sizeof(ac));
    strcpy(ac.stock_code, "#CASH");
    ac.direction = '0';
    ac.position = (long) available_fund;
    ac.exchange_type = ' ';

    return ac;
}

void MYIBApiTunnel::accountDownloadEnd(const IBString& accountName)
{
    TNL_LOG_DEBUG("accountDownloadEnd - accountName:%s", accountName.c_str());

    // shutdown account update
    in_query_flag = false;
    api_client_->reqAccountUpdates(false, tunnel_info_.account);

    if (query_position_step == 1)
    {
        pos_buffer_query_on_time_ = pos_buffer_;
        query_position_step = 2;
    }
    else
    {
        // send position to client
        T_PositionReturn ret;
        ret.datas.swap(pos_buffer_);

        // need return fund in stock system
        ret.datas.insert(ret.datas.begin(), PackageAvailableFunc());

        ret.error_no = 0;
        if (QryPosReturnHandler_) QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}

void MYIBApiTunnel::nextValidId(OrderId orderId)
{
    SaveOrderId(orderId);
    TNL_LOG_DEBUG("nextValidId - orderId:%ld", orderId);
}

void MYIBApiTunnel::updateMktDepth(TickerId id, int position, int operation, int side, double price, int size)
{
    TNL_LOG_DEBUG("updateMktDepth");
}

void MYIBApiTunnel::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation, int side, double price, int size)
{
    TNL_LOG_DEBUG("updateMktDepthL2");
}

void MYIBApiTunnel::updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch)
{
    TNL_LOG_DEBUG("updateNewsBulletin");
}

void MYIBApiTunnel::managedAccounts(const IBString& accountsList)
{
    TNL_LOG_DEBUG("managedAccounts - accountsList:%s", accountsList.c_str());
}

void MYIBApiTunnel::receiveFA(faDataType pFaDataType, const IBString& cxml)
{
    TNL_LOG_DEBUG("receiveFA");
}

void MYIBApiTunnel::historicalData(TickerId reqId, const IBString& date, double open, double high, double low, double close, int volume, int barCount,
    double WAP, int hasGaps)
{
    TNL_LOG_DEBUG("historicalData");
}

void MYIBApiTunnel::scannerParameters(const IBString& xml)
{
    TNL_LOG_DEBUG("scannerParameters");
}

void MYIBApiTunnel::scannerData(int reqId, int rank, const ContractDetails& contractDetails, const IBString& distance, const IBString& benchmark,
    const IBString& projection, const IBString& legsStr)
{
    TNL_LOG_DEBUG("scannerData");
}

void MYIBApiTunnel::scannerDataEnd(int reqId)
{
    TNL_LOG_DEBUG("scannerDataEnd");
}

void MYIBApiTunnel::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close, long volume, double wap, int count)
{
    TNL_LOG_DEBUG("realtimeBar");
}

void MYIBApiTunnel::currentTime(long time)
{
    TNL_LOG_DEBUG("currentTime");
}

void MYIBApiTunnel::fundamentalData(TickerId reqId, const IBString& data)
{
    TNL_LOG_DEBUG("fundamentalData");
}

void MYIBApiTunnel::deltaNeutralValidation(int reqId, const UnderComp& underComp)
{
    TNL_LOG_DEBUG("deltaNeutralValidation");
}

void MYIBApiTunnel::tickSnapshotEnd(int reqId)
{
    TNL_LOG_DEBUG("tickSnapshotEnd");
}

void MYIBApiTunnel::marketDataType(TickerId reqId, int marketDataType)
{
    TNL_LOG_DEBUG("marketDataType");
}

void MYIBApiTunnel::commissionReport(const CommissionReport& r)
{
    TNL_LOG_DEBUG("commissionReport, execId:%s, commission:%.4f, yield:%.4f, currency:%s",
        r.execId.c_str(), Invalid2Nan(r.commission), Invalid2Nan(r.yield), r.currency.c_str());
}

void MYIBApiTunnel::verifyMessageAPI(const IBString& apiData)
{
    TNL_LOG_DEBUG("verifyMessageAPI");
}

void MYIBApiTunnel::verifyCompleted(bool isSuccessful, const IBString& errorText)
{
    TNL_LOG_DEBUG("verifyCompleted");
}

void MYIBApiTunnel::displayGroupList(int reqId, const IBString& groups)
{
    TNL_LOG_DEBUG("displayGroupList");
}

void MYIBApiTunnel::displayGroupUpdated(int reqId, const IBString& contractInfo)
{
    TNL_LOG_DEBUG("displayGroupUpdated");
}

void MYIBApiTunnel::QryOrdersHander()
{
    std::vector<long> unterminated_orders_t;
    while (!cancel_active_order_finished_)
    {
        {
            std::unique_lock<std::mutex> lock(qry_order_result_stats_sync_);
            active_orders_.clear();
        }

        //主动查询所有报单
        TNL_LOG_INFO("reqOpenOrders for cancel all active orders");
        api_client_->reqOpenOrders();

        //处理未终结报单（撤未终结报单）
        {
            {
                std::unique_lock<std::mutex> lock(cancel_sync_);
                qry_order_finish_cond_.wait_for(lock, std::chrono::seconds(20));
                unterminated_orders_t.swap(active_orders_);
            }
            //遍历 unterminated_orders，发送撤单请求
            for (const long &order_id : unterminated_orders_t)
            {
                api_client_->cancelOrder(order_id);
                usleep(20 * 1000);
            }
            unterminated_orders_t.clear();

            //全部发送完毕后，等待 1s ， 判断 handle_flag , 如没有完成，则retry
            sleep(1);
        }
    }

    if (TunnelIsReady())
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
    }
}

void MYIBApiTunnel::RecordPositionAtSpecTime()
{
    TNL_LOG_INFO("thread start: query position at specific time.");
    bool is_query_time = false;
    while (true)
    {
        sleep(1);
        int cur_time = my_cmn::GetCurrentTimeInt();
        if (cur_time >= time_to_query_pos_int)
        {
            is_query_time = true;
            break;
        }
    }

    TNL_LOG_INFO("it's time to query: query position at specific time.");
    T_QryPosition qry_param;
    if (is_query_time)
    {
        query_position_step = 1;
        pos_buffer_.clear();
        while (true)
        {
            int ret = QryPosition(&qry_param);
            TNL_LOG_INFO("QryPosition for query position on specific time - return:%d", ret);
            if (ret == 0)
            {
                break;
            }
            sleep(5);
        }
        TNL_LOG_INFO("send query request success: query position at specific time.");

        // wait while finish query
        while (query_position_step != 2)
        {
            sleep(1);
        }

        TNL_LOG_INFO("finish query: query position at specific time.");
        SaveCurrentPosition();
    }

    TNL_LOG_INFO("thread exit: query position at specific time.");
}

void MYIBApiTunnel::SaveCurrentPosition()
{
    // 存储文件名： pos_account_YYYYMMDD.txt
    std::string pos_file_name = "pos_" + tunnel_info_.account + "_" + my_cmn::GetCurrentDateString() + ".txt";
    int ret = access(pos_file_name.c_str(), F_OK);

    bool file_exist = (ret == 0);

    if (file_exist)
    {
        std::string pos_file_name_bak = pos_file_name + "." + my_cmn::GetCurrentDateTimeWithMilisecString();
        rename(pos_file_name.c_str(), pos_file_name_bak.c_str());
        TNL_LOG_WARN("file(%s) exist when SaveCurrentPosition, backup it", pos_file_name.c_str());
    }

    std::ofstream pos_fs(pos_file_name.c_str());
    if (!pos_fs)
    {
        TNL_LOG_ERROR("open file(%s) failed when SaveCurrentPosition", pos_file_name.c_str());
        return;
    }

    int pos_count = 0;
    pos_fs << fund_info_ << std::endl;
    for (PositionDetail v : pos_buffer_query_on_time_)
    {
        if (v.position > 0)
        {
            VolumeType pos_t = v.position;
            if (v.direction == MY_TNL_D_SELL) pos_t = -pos_t;
            // 格式：合约，仓量
            pos_fs << v.stock_code << "," << pos_t << std::endl;

            ++pos_count;
        }
    }

    TNL_LOG_INFO("current position saved, valid position count:%d", pos_count);
}
