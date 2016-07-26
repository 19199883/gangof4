#pragma once

#include <pthread.h>
#include <string>
#include <list>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "EPosixClientSocket.h"
#include "EWrapper.h"

#include "config_data.h"
#include "trade_data_type.h"
#include "ib_api_context.h"
#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"
#include "my_trade_tunnel_api.h"
#include "my_cmn_util_funcs.h"

class EPosixClientSocket;

class MYIBApiTunnel: public EWrapper
{
public:
    MYIBApiTunnel(const TunnelConfigData &cfg);
    virtual ~MYIBApiTunnel(void);

    // 下发指令接口
    int ReqOrderInsert(const T_PlaceOrder *po);

    //报单操作请求
    int ReqOrderAction(const T_CancelOrder *co);

    // query interfaces
    int QryPosition(const T_QryPosition *p);
    int QryOrderDetail(const T_QryOrderDetail *p);
    int QryTradeDetail(const T_QryTradeDetail *p);
    int QryContractInfo(T_ContractInfoReturn &c_ret);

    void SetCallbackHandler(std::function<void(const T_OrderRespond *)> handler)
    {
        OrderRespondHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_CancelRespond *)> handler)
    {
        CancelRespondHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_OrderReturn *)> handler)
    {
        OrderReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_TradeReturn *)> handler)
    {
        TradeReturnHandler_ = handler;
    }

    void SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
    {
        QryPosReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler)
    {
        QryOrderDetailReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
    {
        QryTradeDetailReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_ContractInfoReturn *)> handler)
    {
        QryContractInfoHandler_ = handler;
    }

    // interface of EWrapper
    virtual void error(const int id, const int errorCode, const IBString errorString);
    virtual void contractDetails(int reqId, const ContractDetails& contractDetails);
    virtual void bondContractDetails(int reqId, const ContractDetails& contractDetails);
    virtual void contractDetailsEnd(int reqId);
    virtual void position(const IBString& account, const Contract& contract, int position, double avgCost);
    virtual void positionEnd();
    virtual void accountSummary(int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency);
    virtual void accountSummaryEnd(int reqId);
    virtual void execDetails(int reqId, const Contract& contract, const Execution& execution);
    virtual void execDetailsEnd(int reqId);
    virtual void orderStatus(OrderId orderId, const IBString &status, int filled,
        int remaining, double avgFillPrice, int permId, int parentId,
        double lastFillPrice, int clientId, const IBString& whyHeld);

    virtual void tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute);
    virtual void tickSize(TickerId tickerId, TickType field, int size);
    virtual void tickOptionComputation(TickerId tickerId, TickType tickType, double impliedVol, double delta,
        double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice);
    virtual void tickGeneric(TickerId tickerId, TickType tickType, double value);
    virtual void tickString(TickerId tickerId, TickType tickType, const IBString& value);
    virtual void tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
        double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry);
    virtual void openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& order_state);
    virtual void openOrderEnd();
    virtual void winError(const IBString &str, int lastError);
    virtual void connectionClosed();
    virtual void updateAccountValue(const IBString& key, const IBString& val, const IBString& currency, const IBString& accountName);
    virtual void updatePortfolio(const Contract& contract, int position,
        double marketPrice, double marketValue, double averageCost,
        double unrealizedPNL, double realizedPNL, const IBString& accountName);
    virtual void updateAccountTime(const IBString& timeStamp);
    virtual void accountDownloadEnd(const IBString& accountName);
    virtual void nextValidId(OrderId orderId);
    virtual void updateMktDepth(TickerId id, int position, int operation, int side,
        double price, int size);
    virtual void updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
        int side, double price, int size);
    virtual void updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch);
    virtual void managedAccounts(const IBString& accountsList);
    virtual void receiveFA(faDataType pFaDataType, const IBString& cxml);
    virtual void historicalData(TickerId reqId, const IBString& date, double open, double high,
        double low, double close, int volume, int barCount, double WAP, int hasGaps);
    virtual void scannerParameters(const IBString &xml);
    virtual void scannerData(int reqId, int rank, const ContractDetails &contractDetails,
        const IBString &distance, const IBString &benchmark, const IBString &projection,
        const IBString &legsStr);
    virtual void scannerDataEnd(int reqId);
    virtual void realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
        long volume, double wap, int count);
    virtual void currentTime(long time);
    virtual void fundamentalData(TickerId reqId, const IBString& data);
    virtual void deltaNeutralValidation(int reqId, const UnderComp& underComp);
    virtual void tickSnapshotEnd(int reqId);
    virtual void marketDataType(TickerId reqId, int marketDataType);
    virtual void commissionReport(const CommissionReport &commissionReport);
    virtual void verifyMessageAPI(const IBString& apiData);
    virtual void verifyCompleted( bool isSuccessful, const IBString& errorText);
    virtual void displayGroupList(int reqId, const IBString& groups);
    virtual void displayGroupUpdated(int reqId, const IBString& contractInfo);

    // inner interfaces need used in outer
    const char *InvestorID()
    {
        return tunnel_info_.account.c_str();
    }
    bool TunnelIsReady()
    {
        return is_connected_ && cancel_active_order_finished_;
    }

    std::mutex rsp_sync;
    std::condition_variable rsp_con;

private:
    void ParseConfig();
    void Start();

    std::function<void(const T_OrderRespond *)> OrderRespondHandler_;
    std::function<void(const T_CancelRespond *)> CancelRespondHandler_;
    std::function<void(const T_OrderReturn *)> OrderReturnHandler_;
    std::function<void(const T_TradeReturn *)> TradeReturnHandler_;

    std::function<void(const T_PositionReturn *)> QryPosReturnHandler_;
    std::function<void(const T_OrderDetailReturn *)> QryOrderDetailReturnHandler_;
    std::function<void(const T_TradeDetailReturn *)> QryTradeDetailReturnHandler_;
    std::function<void(const T_ContractInfoReturn *)> QryContractInfoHandler_;

    // client variables
    std::shared_ptr<EPosixClientSocket> api_client_;
    TradeContext trade_ctx_;
    TunnelConfigData cfg_;
    bool is_connected_;
    bool run_flag_;

    // tunnel config
    Tunnel_Info tunnel_info_;
    std::string gateway_ip_;
    unsigned int gateway_port_;
    int gateway_clientid_;

    // variable share in request thread and respond thread
    std::mutex trade_lock_;
    long cur_order_id_;
    long GetOrderId()
    {
        std::lock_guard<std::mutex> lock(trade_lock_);
        return cur_order_id_++;
    }
    void SaveOrderId(long order_id)
    {
        std::lock_guard<std::mutex> lock(trade_lock_);
        cur_order_id_ = order_id;
    }

    // query result buffers
    PositionDetail PackageAvailableFunc();
    bool in_query_flag;
    double available_fund;
    std::vector<PositionDetail> pos_buffer_;
    std::vector<OrderDetail> od_buffer_;
    std::vector<TradeDetail> td_buffer_;

    // terminate all active orders when init
    bool cancel_active_order_finished_;
    void QryOrdersHander();
    std::mutex cancel_sync_;
    std::mutex qry_order_result_stats_sync_;
    std::condition_variable qry_order_finish_cond_;
    std::vector<long> active_orders_;

    // query and save available fund and position at specific time (A50 need, requirments from zijun)
    int time_to_query_pos_int;
    int query_position_step; // 0:init; 1:start query; 2:finish query
    std::vector<PositionDetail> pos_buffer_query_on_time_;
    std::string fund_info_;
    void RecordPositionAtSpecTime();
    void SaveCurrentPosition();
};
