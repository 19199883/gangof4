#include "order_data.h"
#include "my_exchange_utility.h"
#include <algorithm>

OrderCollection MYOrderDataManager::GetSelfTradeOrders(const OrderInfoInEx *place_order)
{
    OrderCollection self_trade_orders;

    std::lock_guard < std::mutex > lock(mutex_order_);
    for (OrderInfoInEx * v : unterminated_orders_)
    {
        // 相同合约，方向相反，处于可撤状态的报单
        if (OrderUtil::CanBeCancel(v->entrust_status)
            && !v->is_inner_matched
            && (v->order_type != MY_TNL_HF_FOK && v->order_type != MY_TNL_HF_FAK)
            && v->volume_remain > 0
            && place_order->direction != v->direction
            && v->transform_orders.empty()
            && strcmp(place_order->stock_code, v->stock_code) == 0)
        {
            // 价格优于挂单
            if ((place_order->direction == MY_TNL_D_BUY && place_order->limit_price >= v->limit_price)
                || (place_order->direction == MY_TNL_D_SELL && place_order->limit_price <= v->limit_price))
            {
                self_trade_orders.push_back(v);
            }

            // market order
            else if (v->order_kind == MY_TNL_OPT_LIMIT_PRICE
                && place_order->order_kind == MY_TNL_OPT_ANY_PRICE)
            {
                self_trade_orders.push_back(v);
            }
        }
    }

    // 新的报单号优先，排序
    if (place_order->direction == MY_TNL_D_BUY)
    {
        std::sort(self_trade_orders.begin(), self_trade_orders.end(), SortSellOrdersForInnerMatch());
    }
    else
    {
        std::sort(self_trade_orders.begin(), self_trade_orders.end(), SortBuyOrdersForInnerMatch());
    }

    return self_trade_orders;
}

void MYOrderDataManager::GetTransformedOrders(OrderInfoInEx * p, OrderCollection &transformed_orders)
{
    std::lock_guard < std::mutex > lock(mutex_order_);

    for (MYOrderSet::value_type v : p->transform_orders)
    {
        GetTransformedOrdersImp(v, transformed_orders);
    }
}

void MYOrderDataManager::GetTransformedOrdersImp(OrderInfoInEx * order, OrderCollection &transformed_orders)
{
    transformed_orders.push_back(order);
    for (MYOrderSet::value_type v : order->transform_orders)
    {
        GetTransformedOrdersImp(v, transformed_orders);
    }
}

// VolumeType MYOrderDataManager::GetTransformRemainVolume(const std::string& code, char dir)
// {
//     VolumeType total = 0;
//     std::lock_guard < std::mutex > lock(mutex_order_);
//     for (OrderDatas::value_type &v : order_datas_)
//     {
//         // 相同合约，方向相反，变换成open，但没有成交的量，才是可平仓量的一部分
//         if (v.second->is_transform
//             && v.second->volume_remain > 0
//             && v.second->direction != dir
//             && v.second->open_close == MY_TNL_D_OPEN
//             && !OrderUtil::IsTerminated(v.second->entrust_status)
//             && v.second->transform_orders.empty()
//             && strcmp(code.c_str(), v.second->stock_code) == 0)
//         {
//             total += v.second->volume_remain;
//         }
//     }
//     return total;
// }
// VolumeType MYOrderDataManager::GetOpenRemainVolume(const std::string& code, char dir)
// {
//     VolumeType total = 0;
//     std::lock_guard < std::mutex > lock(mutex_order_);
//     for (OrderDatas::value_type &v : order_datas_)
//     {
//         if (v.second->volume_remain > 0
//             && v.second->direction == dir
//             && v.second->open_close == MY_TNL_D_OPEN
//             && !OrderUtil::IsTerminated(v.second->entrust_status)
//             && v.second->transform_orders.empty()
//             && strcmp(code.c_str(), v.second->stock_code) == 0)
//         {
//             total += v.second->volume_remain;
//         }
//     }
//     return total;
// }

bool MYOrderDataManager::HaveSelfTradeOrderInPendingCancel(const OrderInfoInEx *place_order)
{
//     MYSerialNoSet pending_cancel_orders;
//     {
//         std::lock_guard < std::mutex > lock(mutex_cancel_);
//         for (PendingCancelOrders::value_type &v : pending_cancel_orders_)
//         {
//             pending_cancel_orders.insert(v.second.org_serial_no);
//         }
//     }
//     if (pending_cancel_orders.empty()) return false;
// 
//     {
//         std::lock_guard < std::mutex > lock(mutex_order_);
//         for (OrderDatas::value_type &v : order_datas_)
//         {
//             // 相同合约，方向相反，处于可撤状态的报单
//             if (pending_cancel_orders.find(v.first) != pending_cancel_orders.end()
//                 && v.second->volume_remain > 0
//                 && (v.second->order_type != MY_TNL_HF_FOK && v.second->order_type != MY_TNL_HF_FAK)
//                 && place_order->direction != v.second->direction
//                 && v.second->transform_orders.empty()
//                 && strcmp(place_order->stock_code, v.second->stock_code) == 0)
//             {
//                 // 价格优于挂单
//                 if ((place_order->direction == MY_TNL_D_BUY && place_order->limit_price >= v.second->limit_price)
//                     || (place_order->direction == MY_TNL_D_SELL && place_order->limit_price <= v.second->limit_price))
//                 {
//                     EX_LOG_INFO("maybe match with: %ld, but can't cancel it now.", v.first);
//                     return true;
//                 }
//             }
//         }
//     }

    return false;
}

OrderInfoInEx * MYOrderDataManager::UpdateUpperOrdersImp(OrderInfoInEx * order, VolumeType volume_changed)
{
    OrderInfoInEx** the_ord;
    char new_status = order->entrust_status;
    EntrustNoType entrust_no = order->entrust_no;

    // 终结状态,或者有成交,要上传上层
    bool all_terminate = OrderUtil::IsTerminated(new_status);
    bool update_entrust_no = false;
    OrderInfoInEx *updated_order = order;
    if (all_terminate || volume_changed > 0 || order->entrust_no != 0)
    {
        OrderInfoInEx *upper_order = order;

        while (upper_order->super_serial_no != 0)
        {
            EX_LOG_DEBUG("UpdateUpperOrdersImp, super_serial_no: %ld", upper_order->super_serial_no);

            the_ord = m_order_hash.GetData(upper_order->super_serial_no);
            if (the_ord != NULL)
            {
                upper_order = *the_ord;
            }
            else
            {
                EX_LOG_ERROR("GetOrder failed, order serial no: %ld", upper_order->super_serial_no);
                upper_order = NULL;
                break;
            }

            // 原始报单没有委托号的时候，使用当前的委托号，仅标识报出成功
            update_entrust_no = false;
            if (upper_order->entrust_no == 0 && entrust_no != 0)
            {
                upper_order->entrust_no = entrust_no;
                update_entrust_no = true;
                updated_order = upper_order;
            }

            // 如果所有变换单都终结了，将自身改成终结
            if (all_terminate)
            {
                for (MYOrderSet::value_type v : upper_order->transform_orders)
                {
                    if (!OrderUtil::IsTerminated(v->entrust_status))
                    {
                        all_terminate = false;
                        break;
                    }
                }
                if (all_terminate && upper_order->entrust_status != MY_TNL_OS_ERROR)
                {
                    upper_order->entrust_status = new_status;
                    updated_order = upper_order;
                    unterminated_orders_.erase(updated_order);
                }
            }

            // 剩余量的变换，要上传到顶层
            if (volume_changed > 0)
            {
                EX_LOG_DEBUG("super_serial_no: %ld, volume_remain = %ld - %ld", upper_order->super_serial_no,
                    upper_order->volume_remain, volume_changed);
                upper_order->volume_remain -= volume_changed;
                if (upper_order->volume_remain > 0 && !OrderUtil::IsTerminated(upper_order->entrust_status))
                {
                    upper_order->entrust_status = MY_TNL_OS_PARTIALCOM;
                }
                updated_order = upper_order;
            }
            // added on 20141205, 全成时，又有剩余量，修正成已撤（对因达到限额，只能部分报出的单，会出现该场景）
            if (upper_order->volume_remain > 0 && upper_order->entrust_status == MY_TNL_OS_COMPLETED)
            {
                upper_order->entrust_status = MY_TNL_OS_WITHDRAWED;
            }

            // update nothing
            if (!all_terminate && volume_changed == 0 && !update_entrust_no)
            {
                break;
            }
        }
        // fix bug on 20141226: cancled should be last return message;
        if (volume_changed > 0 && updated_order
            && updated_order->volume_remain > 0
            && updated_order->entrust_status == MY_TNL_OS_WITHDRAWED)
        {
            // 记录补已撤标志
            updated_order->entrust_status = MY_TNL_OS_PARTIALCOM;
            updated_order->need_rescue_canced_state = true;
            updated_order->following_matched_volume = volume_changed;
        }
    }

    return updated_order;
}
