//  TODO: wangying x1 status: done
//
#include "x1_trade_context.h"

#include "config_data.h"
#include "tunnel_cmn_utility.h"

X1TradeContext::X1TradeContext()
{
    cur_req_id_ = 0;
    cur_max_order_ref_ = 0;
}

//set cur_max_order_ref_
void X1TradeContext::InitOrderRef(OrderRefDataType orderref_prefix)
{

}

// 设置报单引用
void X1TradeContext::SetOrderRef(OrderRefDataType cur_max_order_ref)
{
	std::lock_guard<std::mutex> lock(sync_mutex_);
    cur_max_order_ref_ = cur_max_order_ref;
}

OrderRefDataType X1TradeContext::GetNewOrderRef()
{
    std::lock_guard<std::mutex> lock(sync_mutex_);
    ++cur_max_order_ref_;

    return cur_max_order_ref_;
}

OrderRefDataType X1TradeContext::GetNewOrderRefForInsertQuote()
{
	std::lock_guard<std::mutex> lock(sync_mutex_);
    OrderRefDataType base_ref = ++cur_max_order_ref_;
    cur_max_order_ref_ += 2;

    return base_ref;
}

int X1TradeContext::GetRequestID()
{
	std::lock_guard<std::mutex> lock(sync_mutex_);
    return ++cur_req_id_;
}

void X1TradeContext::SaveOrderInfo(OrderRefDataType order_id, const X1OrderInfo& order_info)
{
    long serial_no = order_info.serial_no;
    std::lock_guard<std::mutex> lock(order_mutex_);

    OrderIDToOrderInfoMap::iterator ref_it = orderref_to_req_.find(order_id);
    if (ref_it == orderref_to_req_.end())
    {
        orderref_to_req_.insert(std::make_pair(order_id, order_info));
    }
    else
    {
        TNL_LOG_WARN("duplicate order ref: %lld", order_id);
    }

    SerialNoToOrderRefMapIt it = serialno_to_order_id_.find(serial_no);
    if (it != serialno_to_order_id_.end())
    {
        // 相同的流水号，可能交易程序配置错误
        it->second = order_id;
        TNL_LOG_WARN("duplicate order serial no: %ld", serial_no);
    }
    else
    {
        serialno_to_order_id_.insert(std::make_pair(serial_no, order_id));
    }
}

const X1OrderInfo* X1TradeContext::GetOrderInfoByLocalID(OrderRefDataType local_id)
{
	std::lock_guard<std::mutex> lock(order_mutex_);
    OrderIDToOrderInfoMap::iterator cit = orderref_to_req_.find(local_id);
    if (cit != orderref_to_req_.end())
    {
        return &cit->second;
    }
    TNL_LOG_WARN("can't find order of: %lld", local_id);
    return NULL;
}

void X1TradeContext::UpdateCancelInfoOfOrderRef(OrderRefDataType order_ref, long c_s_no)
{
	std::lock_guard<std::mutex> lock(order_mutex_);
    OrderIDToOrderInfoMap::iterator it = orderref_to_req_.find(order_ref);
    if (it != orderref_to_req_.end())
    {
        it->second.cancel_serial_nos.push_back(c_s_no);
    }
}

OrderRefDataType X1TradeContext::GetOrderRefBySerialNo(long serial_no)
{
    OrderRefDataType quote_ref = 0;
    std::lock_guard<std::mutex> lock(order_mutex_);
    SerialNoToOrderRefMapCit cit = serialno_to_order_id_.find(serial_no);
    if (cit != serialno_to_order_id_.end())
    {
        quote_ref = cit->second;
    }
    else
    {
        TNL_LOG_WARN("can't find order ref of serial no: %ld", serial_no);
    }
    return quote_ref;
}

void X1TradeContext::SaveForquoteSerialNoOfOrderRef(OrderRefDataType order_ref, long sn)
{
	std::lock_guard<std::mutex> lock(forquote_orderref_to_serialno_mutex_);
    OrderRefToSerialNoMap::iterator it = forquote_orderref_to_serialno_.find(order_ref);
    if (it == forquote_orderref_to_serialno_.end())
    {
        forquote_orderref_to_serialno_.insert(std::make_pair(order_ref, sn));
    }
    else
    {
    }
}

long X1TradeContext::GetForquoteSerialNoOfOrderRef(OrderRefDataType order_ref)
{
	std::lock_guard<std::mutex> lock(forquote_orderref_to_serialno_mutex_);
    OrderRefToSerialNoMap::iterator it = forquote_orderref_to_serialno_.find(order_ref);
    if (it == forquote_orderref_to_serialno_.end())
    {
        return 0;
    }
    else
    {
        return it->second;
    }
}

void X1TradeContext::SaveQuoteInfo(OrderRefDataType quote_ref, const X1QuoteInfo &quote_info)
{
    long serial_no = quote_info.serial_no;
    std::lock_guard<std::mutex> lock(quote_mutex_);

    OrderIDToQuoteInfoMap::iterator ref_it = quote_ref_to_quote_info_.find(quote_ref);
    if (ref_it == quote_ref_to_quote_info_.end())
    {
        quote_ref_to_quote_info_.insert(std::make_pair(quote_ref, quote_info));
    }
    else
    {
        TNL_LOG_WARN("duplicate quote ref: %lld", quote_ref);
    }

    SerialNoToOrderRefMapIt it = serialno_to_quote_ref_.find(serial_no);
    if (it != serialno_to_quote_ref_.end())
    {
        // 相同的流水号，可能交易程序配置错误
        it->second = quote_ref;
        TNL_LOG_WARN("duplicate quote serial no: %ld", serial_no);
    }
    else
    {
        serialno_to_quote_ref_.insert(std::make_pair(serial_no, quote_ref));
    }
}

const X1QuoteInfo* X1TradeContext::GetQuoteInfoByOrderRef(OrderRefDataType order_ref)
{
	std::lock_guard<std::mutex> lock(quote_mutex_);
    OrderIDToQuoteInfoMap::iterator cit = quote_ref_to_quote_info_.find(order_ref);
    if (cit != quote_ref_to_quote_info_.end())
    {
        return &cit->second;
    }
    TNL_LOG_WARN("can't find quote of quote ref: %lld", order_ref);
    return NULL;
}

void X1TradeContext::UpdateCancelInfoOfQuoteRef(OrderRefDataType order_ref, long c_s_no)
{
	std::lock_guard<std::mutex> lock(quote_mutex_);
    OrderIDToQuoteInfoMap::iterator it = quote_ref_to_quote_info_.find(order_ref);
    if (it != quote_ref_to_quote_info_.end())
    {
        it->second.cancel_serial_nos.push_back(c_s_no);
    }
}

OrderRefDataType X1TradeContext::GetQuoteRefBySerialNo(long serial_no)
{
    OrderRefDataType quote_ref = 0;
    std::lock_guard<std::mutex> lock(quote_mutex_);
    SerialNoToOrderRefMapCit cit = serialno_to_quote_ref_.find(serial_no);
    if (cit != serialno_to_quote_ref_.end())
    {
        quote_ref = cit->second;
    }
    else
    {
        TNL_LOG_WARN("can't find quote ref of serial no: %ld", serial_no);
    }
    return quote_ref;
}
