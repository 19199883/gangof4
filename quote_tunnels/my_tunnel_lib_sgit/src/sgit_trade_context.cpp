#include "sgit_trade_context.h"

#include "config_data.h"
#include "tunnel_cmn_utility.h"

using namespace std;

TradeContext::TradeContext()
{
    cur_req_id_ = 0;
    cur_max_order_ref_ = 0;
}

//set cur_max_order_ref_
void TradeContext::InitOrderRef(OrderRefDataType orderref_prefix)
{
    cur_max_order_ref_ = orderref_prefix * 10000000;
}

// 设置报单引用（如果和前置建立多个会话时，取最大的，简化程序中报单引用的单调递增管理）
void TradeContext::SetOrderRef(OrderRefDataType cur_max_order_ref)
{
    std::lock_guard<std::mutex> lock(sync_lock_);
    if (cur_max_order_ref_ < cur_max_order_ref)
    {
        cur_max_order_ref_ = cur_max_order_ref;
    }
}

OrderRefDataType TradeContext::GetNewOrderRef()
{
    std::lock_guard<std::mutex> lock(sync_lock_);
    ++cur_max_order_ref_;

    return cur_max_order_ref_;
}

int TradeContext::GetRequestID()
{
    std::lock_guard<std::mutex> lock(sync_lock_);
    return ++cur_req_id_;
}

void TradeContext::SaveOrderInfo(const SgitOrderInfo* order_info)
{
    std::lock_guard<std::mutex> lock(order_lock_);
    sn_to_order_.insert(std::make_pair(order_info->po.serial_no, order_info));
    bool insert_result = orderref_to_order_.insert(std::make_pair(order_info->order_ref, order_info)).second;
    if (!insert_result)
    {
        TNL_LOG_WARN("duplicate order ref when InsertOrder, order_ref:%lld.", order_info->order_ref);
    }
}

const SgitOrderInfo* TradeContext::GetOrderInfoBySN(SerialNoType sn)
{
    std::lock_guard<std::mutex> lock(order_lock_);

    SNToSgitOrderInfoMap::const_iterator cit = sn_to_order_.find(sn);
    if (cit != sn_to_order_.end())
    {
        return cit->second;
    }

    return NULL;
}

const SgitOrderInfo* TradeContext::GetOrderInfoByOrderRef(OrderRefDataType order_ref)
{
    std::lock_guard<std::mutex> lock(order_lock_);

    OrderRefToSgitOrderInfoMap::const_iterator cit = orderref_to_order_.find(order_ref);
    if (cit != orderref_to_order_.end())
    {
        return cit->second;
    }

    return NULL;
}

void TradeContext::AddCancelSnForOrder(OrderRefDataType ref, SerialNoType cnl_sn)
{
    std::lock_guard<std::mutex> lock(cancel_lock_);

    OrderRefToCancelSns::iterator it = orderref_to_cancel_sns_.find(ref);
    if (it == orderref_to_cancel_sns_.end())
    {
        it = orderref_to_cancel_sns_.insert(std::make_pair(ref, std::list<SerialNoType>())).first;
    }
    it->second.push_back(cnl_sn);
}

bool TradeContext::PopCancelSnOfOrder(OrderRefDataType ref, SerialNoType& cnl_sn)
{
    std::lock_guard<std::mutex> lock(cancel_lock_);

    OrderRefToCancelSns::iterator it = orderref_to_cancel_sns_.find(ref);
    if (it != orderref_to_cancel_sns_.end() && !it->second.empty())
    {
        cnl_sn = it->second.front();
        it->second.pop_front();
        return true;
    }

    return false;
}
