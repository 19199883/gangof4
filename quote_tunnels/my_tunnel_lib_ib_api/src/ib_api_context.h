#pragma once

#include "my_trade_tunnel_struct.h"
#include <pthread.h>
#include <string>
#include <map>
#include <mutex>
#include <unordered_map>

#include "config_data.h"

#include "Order.h"
#include "Contract.h"

// order information in tunnel
struct TnlOrderInfo
{
    T_PlaceOrder po;

    long order_id;
    mutable TMyTnlOrderStatusType entrust_status; // order status in MY trade system
    mutable VolumeType volume_remain;          // remain volume
    mutable SerialNoType last_cancel_sn;

    TnlOrderInfo(const T_PlaceOrder *po_, long OrdID)
        : po(*po_), order_id(OrdID)
    {
        entrust_status = MY_TNL_OS_UNREPORT;
        volume_remain = po.volume;
        last_cancel_sn = 0;
    }
};

typedef std::unordered_map<long, TnlOrderInfo *> IDToOrderMap;
typedef std::unordered_map<SerialNoType, TnlOrderInfo *> SnToOrderMap;
typedef std::map<std::string, Contract> ContractMap; // use map to keep sort order

class TradeContext
{
public:
    TradeContext();
    int GetNewReqID();

    const Contract * GetContractInfo(const char *po_stockcode);
    std::string GetSymbolOnMarket(const Contract &contract);
    void GetContractInfo(T_ContractInfoReturn &c_ret);

    const TnlOrderInfo * GetOrderByOrderID(long order_id);
    const TnlOrderInfo * GetOrderBySn(SerialNoType order_sn);

    bool CreateNewOrder(const T_PlaceOrder *p, long order_id, Order &order, Contract &contract, T_OrderRespond &order_rsp);
    void BuildOrderRtn(const TnlOrderInfo * p, T_OrderReturn &order_return);
    void BuildTradeRtn(const TnlOrderInfo* p, int filled, double fill_price, T_TradeReturn &trade_return);
    void BuildCancelRsp(const TnlOrderInfo * p, int error_no, T_CancelRespond &rsp);

    char GetExchangeID(const std::string &exchange_name);

private:
    // context variables
    std::mutex context_mutex;
    int req_id_;
    void LoadContractFromFile();
    ContractMap contracts_;

    // object for maintain place order requests
    std::mutex order_mutex;
    IDToOrderMap id_to_order_;
    SnToOrderMap sn_to_order_;

    // variables from config
    long client_id_;
    long perm_id_;
};

