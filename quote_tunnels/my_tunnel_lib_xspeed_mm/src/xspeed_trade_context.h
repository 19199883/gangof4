#pragma once

#include <string>
#include <list>
#include <mutex>
#include <cstring>

#include "DFITCApiStruct.h"
#include <unordered_map>
#include <unordered_set>

#include "trade_data_type.h"
#include "my_tunnel_lib.h"

struct XSpeedOrderInfo
{
    SerialNoType serial_no;     // 报单序列号
    StockCodeType stock_code;   // 合约代码
    double limit_price;         // 价格
    char direction;             // '0': buy; '1': sell
    char open_close;            // '0': open; '1': close
    char speculator;            // '0': speculation; '1': hedge; '2':arbitrage;
    VolumeType volume;          // 手数
    char order_kind;            // '0': limit price; '1': market price
    char order_type;            // '0': normal; '1': fak;

    char exchange_code;

    //
    mutable EntrustNoType entrust_no;
    mutable char entrust_status;
    mutable VolumeType volume_remain;  // 剩余未成交的手数

    mutable std::list<long> cancel_serial_nos;
    long Pop_cancel_serial_no() const
    {
        if (cancel_serial_nos.empty()) return 0;

        long first_sn = cancel_serial_nos.front();
        cancel_serial_nos.pop_front();
        return first_sn;
    }

    XSpeedOrderInfo(char exch_code, const T_PlaceOrder *p)
    {
        serial_no = p->serial_no;
        std::memcpy(stock_code, p->stock_code, sizeof(stock_code));
        limit_price = p->limit_price;
        direction = p->direction;
        open_close = p->open_close;
        speculator = p->speculator;
        volume = p->volume;
        order_kind = p->order_kind;
        order_type = p->order_type;

        exchange_code = exch_code;

        entrust_no = 0;
        entrust_status = MY_TNL_OS_UNREPORT;
        volume_remain = volume;
    }
};

struct XSpeedQuoteInfo
{
    SerialNoType serial_no;     // 报价序列号

    double buy_limit_price;         // 价格
    VolumeType buy_volume;          // 手数
    char buy_open_close;            // '0': open; '1': close
    char buy_speculator;            // '0': speculation; '1': hedge; '2':arbitrage;

    double sell_limit_price;
    VolumeType sell_volume;
    char sell_open_close;
    char sell_speculator;

    mutable EntrustNoType entrust_no;
    mutable char entrust_status;
    mutable char buy_entrust_status;
    mutable char sell_entrust_status;
    mutable std::list<long> cancel_serial_nos;
    mutable int buy_volume_remain;
    mutable int sell_volume_remain;
    long Pop_cancel_serial_no() const
    {
        if (cancel_serial_nos.empty()) return 0;

        long first_sn = cancel_serial_nos.front();
        cancel_serial_nos.pop_front();
        return first_sn;
    }

    XSpeedQuoteInfo(const T_InsertQuote &d)
        : serial_no(d.serial_no),
          buy_limit_price(d.buy_limit_price),
          buy_volume(d.buy_volume),
          buy_open_close(d.buy_open_close),
          buy_speculator(d.buy_speculator),
          sell_limit_price(d.sell_limit_price),
          sell_volume(d.sell_volume),
          sell_open_close(d.sell_open_close),
          sell_speculator(d.sell_speculator)
    {
        entrust_no = 0;
        entrust_status = MY_TNL_OS_UNREPORT;
        buy_entrust_status = entrust_status;
        sell_entrust_status = entrust_status;
        buy_volume_remain = buy_volume;
        sell_volume_remain = sell_volume;
    }
};

typedef std::map<OrderRefDataType, XSpeedOrderInfo> OrderIDToOrderInfoMap;
typedef std::map<OrderRefDataType, XSpeedQuoteInfo> OrderIDToQuoteInfoMap;

class XSpeedTradeContext
{
public:
    XSpeedTradeContext();

    // 报单引用和请求ID的维护
    void InitOrderRef(OrderRefDataType orderref_prefix);
    void SetOrderRef(OrderRefDataType cur_max_order_ref);
    OrderRefDataType GetNewOrderRef();
    int GetRequestID();

    // 报单引用和报单原始信息的映射
    void SaveOrderInfo(OrderRefDataType order_id, const XSpeedOrderInfo &order_info);
    const XSpeedOrderInfo * GetOrderInfoByLocalID(OrderRefDataType local_id);
    void UpdateCancelInfoOfOrderRef(OrderRefDataType order_ref, long c_s_no);
    OrderRefDataType GetOrderRefBySerialNo(long serial_no);

    // added for mm
    OrderRefDataType GetNewOrderRefForInsertQuote();
    void SaveForquoteSerialNoOfOrderRef(OrderRefDataType order_ref, long sn);
    long GetForquoteSerialNoOfOrderRef(OrderRefDataType order_ref);

    void SaveQuoteInfo(OrderRefDataType quote_ref, const XSpeedQuoteInfo &quote_info);
    const XSpeedQuoteInfo * GetQuoteInfoByOrderRef(OrderRefDataType order_ref);
    void UpdateCancelInfoOfQuoteRef(OrderRefDataType order_ref, long c_s_no);
    OrderRefDataType GetQuoteRefBySerialNo(long serial_no);

private:
    std::mutex sync_mutex_;
    OrderRefDataType cur_max_order_ref_;
    int cur_req_id_;

    // 报单引用到原始报单信息的映射表
    std::mutex order_mutex_;
    OrderIDToOrderInfoMap orderref_to_req_;
    SerialNoToOrderRefMap serialno_to_order_id_;

    // added for mm
    std::mutex forquote_orderref_to_serialno_mutex_;
    OrderRefToSerialNoMap forquote_orderref_to_serialno_;

    std::mutex quote_mutex_;
    OrderIDToQuoteInfoMap quote_ref_to_quote_info_;
    SerialNoToOrderRefMap serialno_to_quote_ref_;
};
