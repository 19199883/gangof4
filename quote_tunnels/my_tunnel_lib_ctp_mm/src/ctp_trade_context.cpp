#include "ctp_trade_context.h"

#include "config_data.h"
#include "tunnel_cmn_utility.h"

using namespace std;

CTPTradeContext::CTPTradeContext()
{
    cur_req_id_ = 0;
    cur_max_order_ref_ = 0;
}

//set cur_max_order_ref_
void CTPTradeContext::InitOrderRef(OrderRefDataType orderref_prefix)
{
    cur_max_order_ref_ = orderref_prefix * 10000000;
}

// 设置报单引用（如果和前置建立多个会话时，取最大的，简化程序中报单引用的单调递增管理）
void CTPTradeContext::SetOrderRef(OrderRefDataType cur_max_order_ref)
{
    lock_guard<mutex> lock(sync_mutex_);
    if (cur_max_order_ref_ < cur_max_order_ref)
    {
        cur_max_order_ref_ = cur_max_order_ref;
    }
}

OrderRefDataType CTPTradeContext::GetNewOrderRef()
{
    lock_guard<mutex> lock(sync_mutex_);
    ++cur_max_order_ref_;

    return cur_max_order_ref_;
}

OrderRefDataType CTPTradeContext::GetNewOrderRefForInsertQuote()
{
    lock_guard<mutex> lock(sync_mutex_);
    OrderRefDataType base_ref = ++cur_max_order_ref_;
    cur_max_order_ref_ += 2;

    return base_ref;
}

int CTPTradeContext::GetRequestID()
{
    lock_guard<mutex> lock(sync_mutex_);
    return ++cur_req_id_;
}

void CTPTradeContext::SaveSerialNoToOrderRef(OrderRefDataType order_ref, const OriginalReqInfo &order_info)
{
    long serial_no = order_info.serial_no;
    lock_guard<mutex> lock(ref_mutex_);

    OrderRefToRequestInfoMapIt ref_it = orderref_to_req_.find(order_ref);
    if (ref_it == orderref_to_req_.end())
    {
        orderref_to_req_.insert(std::make_pair(order_ref, order_info));
    }
    else
    {
        TNL_LOG_WARN("duplicate order ref: %lld", order_ref);
    }

    SerialNoToOrderRefMapIt it = serialno_to_orderref_.find(serial_no);
    if (it != serialno_to_orderref_.end())
    {
        // 相同的流水号，可能交易程序配置错误
        it->second = order_ref;
        TNL_LOG_WARN("duplicate serial no: %ld", serial_no);
    }
    else
    {
        serialno_to_orderref_.insert(std::make_pair(serial_no, order_ref));
    }
}

const OriginalReqInfo * CTPTradeContext::GetOrderInfoByOrderRef(OrderRefDataType order_ref)
{
    lock_guard<mutex> lock(ref_mutex_);
    OrderRefToRequestInfoMapCit cit = orderref_to_req_.find(order_ref);
    if (cit != orderref_to_req_.end())
    {
        return &cit->second;
    }
    return NULL;
}

void CTPTradeContext::UpdateCancelOrderRef(OrderRefDataType order_ref, const std::string &c_s_no)
{
    lock_guard<mutex> lock(ref_mutex_);
    OrderRefToRequestInfoMapIt it = orderref_to_req_.find(order_ref);
    if (it != orderref_to_req_.end())
    {
        it->second.cancel_serial_no.append(c_s_no + ",");
    }
}

OrderRefDataType CTPTradeContext::GetOrderRefBySerialNo(long serial_no)
{
    OrderRefDataType order_ref = 0;
    lock_guard<mutex> lock(ref_mutex_);
    SerialNoToOrderRefMapCit cit = serialno_to_orderref_.find(serial_no);
    if (cit != serialno_to_orderref_.end())
    {
        order_ref = cit->second;
    }
    else
    {
        TNL_LOG_WARN("can't find order ref of serial no: %ld", serial_no);
    }
    return order_ref;
}

void CTPTradeContext::SetAnsweredPlaceOrder(OrderRefDataType order_ref)
{
    lock_guard<mutex> lock(ans_po_mutex_);
    if (answered_place_order_refs_.find(order_ref) == answered_place_order_refs_.end())
    {
        answered_place_order_refs_.insert(order_ref);
    }
    else
    {
        TNL_LOG_WARN("duplicate answer to order ref: %lld", order_ref);
    }
}

void CTPTradeContext::SetAnsweredCancelOrder(OrderRefDataType order_ref)
{
    {
        lock_guard<mutex> lock(ans_co_mutex_);
        if (answered_cancel_order_refs_.find(order_ref) == answered_cancel_order_refs_.end())
        {
            answered_cancel_order_refs_.insert(order_ref);
        }
        else
        {
            // 对相同报单，会重复撤单，所以，重复应答是正常的
            TNL_LOG_DEBUG("duplicate answer to order ref: %lld", order_ref);
        }
    }
    {
        // 清除撤单流水号
        lock_guard<mutex> lock(ref_mutex_);
        OrderRefToRequestInfoMapIt it = orderref_to_req_.find(order_ref);
        if (it != orderref_to_req_.end())
        {
            std::size_t pos = it->second.cancel_serial_no.find(",");
            if (pos != std::string::npos)
            {
                it->second.cancel_serial_no = it->second.cancel_serial_no.substr(pos + 1);
            }
        }
    }
}

bool CTPTradeContext::IsAnsweredPlaceOrder(OrderRefDataType order_ref)
{
    lock_guard<mutex> lock(ans_po_mutex_);
    return answered_place_order_refs_.find(order_ref) != answered_place_order_refs_.end();
}

bool CTPTradeContext::IsAnsweredCancelOrder(OrderRefDataType order_ref)
{
    lock_guard<mutex> lock(ans_co_mutex_);
    return answered_cancel_order_refs_.find(order_ref) != answered_cancel_order_refs_.end();
}

void CTPTradeContext::SaveOrderSysIDOfSerialNo(long sn, long id)
{
    lock_guard<mutex> lock(ordersysid_mutex_);
    SerialNoToOrderSysIDMap::iterator it = serialno_to_ordersysid_.find(sn);
    if (it == serialno_to_ordersysid_.end())
    {
        serialno_to_ordersysid_.insert(std::make_pair(sn, id));
    }
    else
    {
    }
}

long CTPTradeContext::GetOrderSysIDOfSerialNo(long sn)
{
    lock_guard<mutex> lock(ordersysid_mutex_);
    SerialNoToOrderSysIDMap::iterator it = serialno_to_ordersysid_.find(sn);
    if (it == serialno_to_ordersysid_.end())
    {
        return 0;
    }
    else
    {
        return it->second;
    }
}

bool CTPTradeContext::CheckAndSetHaveHandleInsertErr(long sn)
{
    TNL_LOG_DEBUG("CheckAndSetHaveHandleInsertErr sn: %ld", sn);
    lock_guard<mutex> lock(err_handle_mutex_);
    SerialNoSet::iterator it = handled_err_insert_orders_.find(sn);
    if (it == handled_err_insert_orders_.end())
    {
        handled_err_insert_orders_.insert(sn);
        return true;
    }
    else
    {
        return false;
    }
}

bool CTPTradeContext::CheckAndSetHaveHandleActionErr(long sn)
{
    TNL_LOG_DEBUG("CheckAndSetHaveHandleActionErr sn: %ld", sn);
    lock_guard<mutex> lock(err_handle_mutex_);
    SerialNoSet::iterator it = handled_err_action_orders_.find(sn);
    if (it == handled_err_action_orders_.end())
    {
        handled_err_action_orders_.insert(sn);
        return true;
    }
    else
    {
        return false;
    }
}

void CTPTradeContext::SaveForquoteSerialNoOfOrderRef(OrderRefDataType order_ref, long sn)
{
    lock_guard<mutex> lock(forquote_orderref_to_serialno_mutex_);
    OrderRefToSerialNoMap::iterator it = forquote_orderref_to_serialno_.find(order_ref);
    if (it == forquote_orderref_to_serialno_.end())
    {
        forquote_orderref_to_serialno_.insert(std::make_pair(order_ref, sn));
    }
    else
    {
    }
}

long CTPTradeContext::GetForquoteSerialNoOfOrderRef(OrderRefDataType order_ref)
{
    long sn = 0;
    lock_guard<mutex> lock(forquote_orderref_to_serialno_mutex_);
    OrderRefToSerialNoMap::iterator it = forquote_orderref_to_serialno_.find(order_ref);
    if (it != forquote_orderref_to_serialno_.end())
    {
        sn = it->second;
        forquote_orderref_to_serialno_.erase(it);
    }

    return sn;
}

void CTPTradeContext::SaveQuoteInfo(OrderRefDataType quote_ref, const CTPQuoteInfo &quote_info)
{
    long serial_no = quote_info.serial_no;
    lock_guard<mutex> lock(quote_mutex_);

    QuoteInfoOfQuoteRef::iterator ref_it = quote_ref_to_quote_info_.find(quote_ref);
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

const CTPQuoteInfo* CTPTradeContext::GetQuoteInfoByOrderRef(OrderRefDataType order_ref)
{
    lock_guard<mutex> lock(quote_mutex_);
    QuoteInfoOfQuoteRef::iterator cit = quote_ref_to_quote_info_.find(order_ref);
    if (cit != quote_ref_to_quote_info_.end())
    {
        return &cit->second;
    }
    TNL_LOG_WARN("can't find quote of quote ref: %lld", order_ref);
    return NULL;
}

void CTPTradeContext::UpdateCancelInfoOfQuoteRef(OrderRefDataType order_ref, const std::string& c_s_no)
{
    lock_guard<mutex> lock(quote_mutex_);
    QuoteInfoOfQuoteRef::iterator it = quote_ref_to_quote_info_.find(order_ref);
    if (it != quote_ref_to_quote_info_.end())
    {
        it->second.cancel_serial_no.append(c_s_no + ",");
    }
}

OrderRefDataType CTPTradeContext::GetQuoteRefBySerialNo(long serial_no)
{
    OrderRefDataType quote_ref = 0;
    lock_guard<mutex> lock(quote_mutex_);
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
