#include "xele_trade_context.h"

#include "config_data.h"
#include "tunnel_cmn_utility.h"
#include "trade_data_type.h"

using namespace std;

XeleTradeContext::XeleTradeContext()
{
    cur_req_id_ = 0;
    cur_max_order_ref_ = 0;
}

//set cur_max_order_ref_
void XeleTradeContext::InitOrderRef(OrderRefDataType orderref_prefix)
{
    cur_max_order_ref_ = (9900 + orderref_prefix) * 10000000;
}

// 设置报单引用（如果和前置建立多个会话时，取最大的，简化程序中报单引用的单调递增管理）
void XeleTradeContext::SetOrderRef(OrderRefDataType cur_max_order_ref)
{
    lock_guard<mutex> lock(sync_mutex_);
    if (cur_max_order_ref_ < cur_max_order_ref)
    {
        cur_max_order_ref_ = cur_max_order_ref;
    }
}

OrderRefDataType XeleTradeContext::GetNewOrderRef()
{
    lock_guard<mutex> lock(sync_mutex_);
    ++cur_max_order_ref_;

    return cur_max_order_ref_;
}

int XeleTradeContext::GetRequestID()
{
    lock_guard<mutex> lock(sync_mutex_);
    return ++cur_req_id_;
}

void XeleTradeContext::SaveSerialNoToOrderRef(OrderRefDataType order_ref, const XeleOriginalReqInfo &order_info)
{
    lock_guard<mutex> lock(ref_mutex_);

    XeleOrderRefToRequestInfoMapIt ref_it = orderref_to_req_.find(order_ref);
    if (ref_it == orderref_to_req_.end())
    {
        orderref_to_req_.insert(std::make_pair(order_ref, order_info));
    }
    else
    {
        TNL_LOG_WARN("duplicate order ref: %ld", order_ref);
    }

    SerialNoToOrderRefMapIt it = serialno_to_orderref_.find(order_info.serial_no);
    if (it != serialno_to_orderref_.end())
    {
        // 相同的流水号，可能交易程序配置错误
        it->second = order_ref;
        TNL_LOG_WARN("duplicate serial no: %ld", order_info.serial_no);
    }
    else
    {
        serialno_to_orderref_.insert(std::make_pair(order_info.serial_no, order_ref));
    }
}

const XeleOriginalReqInfo * XeleTradeContext::GetOrderInfoByOrderRef(OrderRefDataType order_ref)
{
    lock_guard<mutex> lock(ref_mutex_);
    XeleOrderRefToRequestInfoMapCit cit = orderref_to_req_.find(order_ref);
    if (cit != orderref_to_req_.end())
    {
        return &cit->second;
    }
    return NULL;
}

OrderRefDataType XeleTradeContext::GetOrderRefBySerialNo(long serial_no)
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

void XeleTradeContext::SaveOrderRefToStatus(OrderRefDataType order_ref, const OrderStatusInTunnel& status)
{
    lock_guard<mutex> lock(status_mutex_);

    OrderRefToOrderStatusMapIt ref_it = orderref_to_status_.find(order_ref);
    if (ref_it == orderref_to_status_.end())
    {
        orderref_to_status_.insert(std::make_pair(order_ref, status));
    }
    else
    {
        TNL_LOG_WARN("SaveOrderRefToStatus - duplicate order ref: %ld", order_ref);
    }
}

StatusCheckResult XeleTradeContext::CheckAndUpdateStatus(OrderRefDataType order_ref, FemasRespondType rt,
    char new_status, int new_matched_volume)
{
    lock_guard<mutex> lock(status_mutex_);
    OrderRefToOrderStatusMapIt it = orderref_to_status_.find(order_ref);
    if (it != orderref_to_status_.end())
    {
        OrderStatusInTunnel& order_status = it->second;

        switch (rt)
        {
            case FemasRespondType::order_respond:
            {
                // respond，判断状态，如果是初始状态，更新；否则，抛弃；
                if (order_status.status == MY_TNL_OS_UNREPORT)
                {
                    order_status.status = new_status;
                    return StatusCheckResult::none;
                }
                else
                {
                    return StatusCheckResult::abandon;
                }
                break;
            }
            case FemasRespondType::order_return:
            {
                // return，判断状态，如果是初始状态，补发respond；如果已全成，抛弃；否则，同原流程
                if (order_status.status == MY_TNL_OS_UNREPORT)
                {
                    order_status.status = new_status;
                    return StatusCheckResult::fillup_rsp;
                }
                else if(order_status.status == MY_TNL_OS_COMPLETED)
                {
                    return StatusCheckResult::abandon;
                }
                else
                {
                    order_status.status = new_status;
                    return StatusCheckResult::none;
                }

                break;
            }
            case FemasRespondType::trade_return:
            {
                // trade return，判断状态，
                //   如果是初始状态，补发respond
                //   如果不是部成/已成，补发部成（如果成交量达到全成的量，补发全成return）
                if (order_status.status == MY_TNL_OS_UNREPORT)
                {
                    order_status.matched_volume += new_matched_volume;
                    if (order_status.matched_volume == order_status.volume)
                    {
                        order_status.status = MY_TNL_OS_COMPLETED;
                        return StatusCheckResult::fillup_rsp_rtn_c;
                    }
                    else
                    {
                        order_status.status = MY_TNL_OS_PARTIALCOM;
                        return StatusCheckResult::fillup_rsp_rtn_p;
                    }
                }
                else if(order_status.status != MY_TNL_OS_PARTIALCOM
                    && order_status.status != MY_TNL_OS_COMPLETED)
                {
                    order_status.matched_volume += new_matched_volume;
                    if (order_status.matched_volume == order_status.volume)
                    {
                        order_status.status = MY_TNL_OS_COMPLETED;
                        return StatusCheckResult::fillup_rtn_c;
                    }
                    else
                    {
                        order_status.status = MY_TNL_OS_PARTIALCOM;
                        return StatusCheckResult::fillup_rtn_p;
                    }
                }
                else
                {
                    return StatusCheckResult::none;
                }

                break;
            }
            default:
                return StatusCheckResult::none;
        }
    }

    return StatusCheckResult::none;
}

bool XeleTradeContext::CheckAndSetHaveHandleInsertErr(long sn)
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

bool XeleTradeContext::CheckAndSetHaveHandleActionErr(long sn)
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

void XeleTradeContext::SetFinishedOrderOfFAK(OrderRefDataType r)
{
    lock_guard<mutex> lock(fak_mutex_);

    if (finished_order_ref_.find(r) == finished_order_ref_.end())
    {
        finished_order_ref_.insert(r);
    }
    else
    {
        TNL_LOG_WARN("SetFinishedOrderOfFAK - duplicate order ref: %ld", r);
    }
}

bool  XeleTradeContext::IsFinishedOrderOfFAK(OrderRefDataType r)
{
    lock_guard<mutex> lock(fak_mutex_);

    return finished_order_ref_.find(r) != finished_order_ref_.end();
}
