#ifndef ESUNNY_TRADE_CONTEXT_H_
#define ESUNNY_TRADE_CONTEXT_H_

#include <pthread.h>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "TapTradeAPI.h"
#include "trade_data_type.h"
#include "my_trade_tunnel_data_type.h"
#include "my_tunnel_lib.h"
#include "my_cmn_util_funcs.h"
#include "tunnel_cmn_utility.h"

struct EsunnyOrderInfo
{
    T_PlaceOrder po;

    mutable EntrustNoType entrust_no;
    mutable char entrust_status;
    mutable VolumeType volume_remain;  // 剩余未成交的手数
    mutable VolumeType volume_total_matched;  // 已成交的手数

    // fields for cancel order
    mutable TAPISTR_20 order_no;
    mutable TAPICHAR server_flag;

    inline bool IsTerminated() const
    {
        return entrust_status == MY_TNL_OS_ERROR
            || entrust_status == MY_TNL_OS_WITHDRAWED
            || (entrust_status == MY_TNL_OS_COMPLETED && volume_total_matched >= po.volume);
    }

    EsunnyOrderInfo(const T_PlaceOrder &po_)
    :po(po_)
    {
        entrust_no = 0;
        entrust_status = MY_TNL_OS_UNREPORT;
        volume_remain = po.volume;
        volume_total_matched = 0;
        memset(order_no, 0, sizeof(order_no));
        server_flag = 0;
    }

};

typedef std::unordered_map<SerialNoType, const EsunnyOrderInfo *> SNToEsunnyOrderInfoMap;
typedef std::unordered_map<std::string, const EsunnyOrderInfo *> OrderNoToEsunnyOrderInfoMap;
typedef std::unordered_map<TAPIUINT32, const EsunnyOrderInfo *> SessionIDToEsunnyOrderInfoMap;
typedef std::unordered_map<TAPIUINT32, SerialNoType> SessionIDToSerialNoMap;

class ESUNNYTradeContext
{
public:
    ESUNNYTradeContext();

    // order id to order
    void SaveOrderInfo(TAPIUINT32 session_id, const EsunnyOrderInfo *order_info);
    const EsunnyOrderInfo * GetOrderInfoBySN(SerialNoType sn);
    const EsunnyOrderInfo * GetOrderInfoByOrderNo(const std::string &order_no);
    const EsunnyOrderInfo * UpdateOrderNoAndGetOrderInfo(TAPIUINT32 session_id, const char *order_no, char server_flag);

    void SavePendingOrder(const T_PlaceOrder *order_info)
    {
        std::lock_guard<std::mutex> lock(order_mutex_);
        po_tmp_ = order_info;
    }

    // cancel serial no to session id(request id)
    void SaveCancelSnOfSessionID(TAPIUINT32 session_id, SerialNoType c_sn);
    SerialNoType GetSnBySessionID(TAPIUINT32 session_id);

private:
    // object for maintain place order requests
    std::mutex order_mutex_;
    SNToEsunnyOrderInfoMap sn_to_order_;
    OrderNoToEsunnyOrderInfoMap orderno_to_order_;
    SessionIDToEsunnyOrderInfoMap sessionid_to_order_;
    const T_PlaceOrder *po_tmp_;

    // object for maintain cancel requests
    std::mutex cancel_mutex_;
    SessionIDToSerialNoMap sessionid_to_cancel_sn_;
};

struct EsunnyQuoteInfo
{
	T_InsertQuote iq;

    mutable EntrustNoType entrust_no;
    mutable char entrust_status;
    mutable VolumeType buy_volume_remain;  // 买剩余未成交的手数
    mutable VolumeType buy_volume_total_matched;  // 买已成交的手数
    mutable VolumeType sell_volume_remain; // 卖剩余未成交的手数
    mutable VolumeType sell_volume_total_matched;  // 卖已成交的手数

    // fields for cancel order
    mutable TAPISTR_20 order_no;
    mutable TAPICHAR server_flag;

    inline bool IsTerminated() const
    {
        return entrust_status == MY_TNL_OS_ERROR
            || entrust_status == MY_TNL_OS_WITHDRAWED
            || (entrust_status == MY_TNL_OS_COMPLETED &&
            		buy_volume_total_matched >= iq.buy_volume &&
					sell_volume_total_matched >= iq.sell_volume);
    }

    inline bool BuyIsTerminated() const
    {
    	return buy_volume_remain <= 0;
    }

    inline bool SellIsTerminated() const
    {
    	return sell_volume_remain <= 0;
    }

    EsunnyQuoteInfo(const T_InsertQuote &iq_)
    :iq(iq_)
    {
        entrust_no = 0;
        entrust_status = MY_TNL_OS_UNREPORT;
        buy_volume_remain = iq.buy_volume;
        buy_volume_total_matched = 0;
        sell_volume_remain = iq.sell_volume;
        sell_volume_total_matched = 0;
        memset(order_no, 0, sizeof(order_no));
        server_flag = 0;
    }
};

typedef std::unordered_map<SerialNoType, const EsunnyQuoteInfo *> SNToEsunnyQuoteInfoMap;
typedef std::unordered_map<std::string, const EsunnyQuoteInfo *> OrderNoToEsunnyQuoteInfoMap;
typedef std::unordered_map<TAPIUINT32, const EsunnyQuoteInfo *> SessionIDToEsunnyQuoteInfoMap;
typedef std::unordered_map<TAPIUINT32, SerialNoType> SessionIDToSerialNoMap;

class ESUNNYQuoteContext
{
public:
	ESUNNYQuoteContext();

    // order id to order
    void SaveQuoteInfo(TAPIUINT32 session_id, const EsunnyQuoteInfo *quote_info);
    const EsunnyQuoteInfo * GetQuoteInfoBySN(SerialNoType sn);
    const EsunnyQuoteInfo * GetQuoteInfoByOrderNo(const std::string &quote_no);
    const EsunnyQuoteInfo * UpdateQuoteNoAndGetQuoteInfo(TAPIUINT32 session_id, const char *quote_no, char server_flag);

    void SavePendingOrder(const T_InsertQuote *quote_info)
    {
        std::lock_guard<std::mutex> lock(quote_mutex_);
        iq_tmp_ = quote_info;
    }

    // cancel serial no to session id(request id)
    void SaveCancelOrForQuoteSnOfSessionID(TAPIUINT32 session_id, SerialNoType c_sn);
    SerialNoType GetSnBySessionID(TAPIUINT32 session_id);

private:
    // object for maintain place order requests
    std::mutex quote_mutex_;
    SNToEsunnyQuoteInfoMap sn_to_quote_;
    OrderNoToEsunnyQuoteInfoMap orderno_to_quote_;
    SessionIDToEsunnyQuoteInfoMap sessionid_to_quote_;
    const T_InsertQuote *iq_tmp_;

    // object for maintain cancel requests
    std::mutex cancel_mutex_;
    SessionIDToSerialNoMap sessionid_to_cancel_sn_;
};

#endif
