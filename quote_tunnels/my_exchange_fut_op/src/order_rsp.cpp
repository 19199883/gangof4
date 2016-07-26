#include "order_rsp.h"
#include "my_exchange_utility.h"
#include "order_data.h"
#include "position_data.h"
#include "order_req.h"
#include "my_exchange.h"

void MYOrderRsp::OrderRespondImp(bool from_myex, const T_OrderRespond * p)
{
    return OrderRspProcess(from_myex, false, p->error_no, p->serial_no, p->entrust_no, p->entrust_status);
}

void MYOrderRsp::OrderReturnImp(bool from_myex, const T_OrderReturn * p)
{
    return OrderRspProcess(from_myex, true, 0, p->serial_no, p->entrust_no, p->entrust_status, p->volume_remain);
}

void MYOrderRsp::ResendTerminateStatusAfterTradeReturn(OrderInfoInEx* order, VolumeType volume_matched)
{
    if (order->need_rescue_canced_state)
    {
        if (order->following_matched_volume == volume_matched)
        {
            order->need_rescue_canced_state = false;
            order->following_matched_volume = 0;
            order->entrust_status = MY_TNL_OS_WITHDRAWED;

            T_OrderReturn order_return;
            order->CreateOrderReturn(order_return);
            p_my_exchange_->SendOrderReturn(&order_return, 0); // volume matched have reported before trade reture, so here is 0
        }
        else
        {
            order->following_matched_volume -= volume_matched;
        }
    }
}

void MYOrderRsp::TradeReturnImp(bool from_myex, const T_TradeReturn * t_rtn)
{
    EX_LOG_DEBUG("MYOrderRsp::TradeReturnImp, serial_no=%ld", t_rtn->serial_no);

    // 实际报单的回报，直接返回
    if (p_order_manager_->IsOriginalSerialNo(t_rtn->serial_no))
    {
        p_my_exchange_->SendTradeReturn(t_rtn);
    }
    else
    {
        // 查找原始报单，回报成交
        SerialNoType serial_no = t_rtn->serial_no;
        OrderInfoInEx *order = p_order_manager_->GetOrder(serial_no);
        while ((order != NULL) && (order->super_serial_no != 0))
        {
            order = p_order_manager_->GetOrder(order->super_serial_no);
        }

        // 成交量合并到最顶层
        if (order != NULL)
        {
            // 发送合并的成交回报
            T_TradeReturn ntr;
            order->CreateTradeReturn(ntr, t_rtn->business_volume, t_rtn->business_price, t_rtn->business_no);
            p_my_exchange_->SendTradeReturn(&ntr);

            ResendTerminateStatusAfterTradeReturn(order, t_rtn->business_volume);
        }
        else
        {
            EX_LOG_ERROR("MYOrderRsp::TradeReturnImp, can't get original order of serial_no=%d", serial_no);
        }
    }

    // 更新仓位
    // commented on 20141202: update when receive order return
//    if (!from_myex)
//    {
//        p_position_manager_->UpdateOutterPosition(t_rtn->stock_code, t_rtn->direction, t_rtn->open_close, t_rtn->business_volume);
//    }
}

void MYOrderRsp::QuoteRespondImp(const T_InsertQuoteRespond * p)
{
    QuoteInfoInEx *quote = p_order_manager_->UpdateAndGetQuote(p->serial_no, p->entrust_status, p->entrust_no);
    if (!quote)
    {
        EX_LOG_ERROR("MYOrderRsp::QuoteRespondImp, quote not found, serial_no=%ld", p->serial_no);
        return;
    }

    // 如果被终结，freez position and pending volume要回滚 (2 dir)
    if (OrderUtil::IsTerminated(p->entrust_status))
    {
        p_position_manager_->RollBackPendingVolume(quote->data.stock_code,
            MY_TNL_D_BUY, quote->data.buy_open_close, quote->data.buy_volume);

        p_position_manager_->RollBackPendingVolume(quote->data.stock_code,
            MY_TNL_D_SELL, quote->data.sell_open_close, quote->data.sell_volume);
    }

    // send respond
    p_my_exchange_->SendInsertQuoteRespond(p, quote->data.stock_code, quote->buy_remain, quote->sell_remain);
}

void MYOrderRsp::QuoteReturnImp(const T_QuoteReturn * p)
{
    // don't update now, need check new matched volume
    QuoteInfoInEx *quote = p_order_manager_->GetQuote(p->serial_no);
    if (!quote)
    {
        EX_LOG_ERROR("MYOrderRsp::QuoteReturnImp, order not found, serial_no=%ld", p->serial_no);
        return;
    }

    VolumeType volume_matched = 0;
    if (p->direction == MY_TNL_D_BUY && p->volume_remain < quote->buy_remain)
    {
        volume_matched = quote->buy_remain - p->volume_remain;
        p_position_manager_->UpdateOutterPosition(p->stock_code, p->direction, p->open_close, volume_matched, false);
    }
    else if (p->direction == MY_TNL_D_SELL && p->volume_remain < quote->sell_remain)
    {
        volume_matched = quote->sell_remain - p->volume_remain;
        p_position_manager_->UpdateOutterPosition(p->stock_code, p->direction, p->open_close, volume_matched, false);
    }
    p_order_manager_->UpdateQuote(p->serial_no, p->direction, p->entrust_status, p->entrust_no, p->volume_remain);

    // 如果被终结，freez position and pending volume要回滚
    if (OrderUtil::IsTerminated(p->entrust_status))
    {
        p_position_manager_->RollBackPendingVolume(quote->data.stock_code, p->direction, p->open_close, p->volume_remain);
    }

    // send respond
    p_my_exchange_->SendQuoteReturn(p, volume_matched);
}

void MYOrderRsp::QuoteTradeImp(const T_QuoteTrade * p)
{
    EX_LOG_DEBUG("MYOrderRsp::QuoteTradeImp, serial_no=%ld", p->serial_no);

    // 直接返回
    p_my_exchange_->SendQuoteTradeReturn(p);
}

void MYOrderRsp::InnerMatch(SerialNoType new_matched_serial_no,
    SerialNoType cancel_order_serial_no,
    VolumeType canceled_volume)
{
    std::lock_guard < std::mutex > lock(mutex_inner_match_);

    EX_LOG_INFO("MYOrderRsp::InnerMatch new_matched_serial_no=%ld;cancel_order_serial_no=%ld;canceled_volume=%ld",
        new_matched_serial_no, cancel_order_serial_no, canceled_volume);
    OrderInfoInEx* matched_order = p_order_manager_->GetOrder(new_matched_serial_no);

    // 删除内部撮合关联关系
    p_order_manager_->RemoveOrderInnerMatchOrderRelation(cancel_order_serial_no, matched_order);

    // 没有撮合成交的量 fix bug in 20141113, maybe all terminated, should rescue
    if (canceled_volume == 0)
    {
        CheckAndRescueMatchedOrder(matched_order);
        return;
    }

    // volume remain of new place order
    VolumeType new_order_remain = 0;
    if (!OrderUtil::IsTerminated(matched_order->entrust_status))
    {
        new_order_remain = matched_order->volume_remain;
    }

    // 撮合成交的量
    VolumeType inner_matched_volume = 0;
    double match_price = 0;
    if (canceled_volume <= new_order_remain)
    {
        // 被撤量 <= 新报单的剩余量；被撤单全成，新报单考虑补单
        inner_matched_volume = canceled_volume;
    }
    else
    {
        // 被撤量 > 新报单的剩余量；新报单全成，被撤单要补单
        inner_matched_volume = new_order_remain;
    }
    OrderInfoInEx * canceled_order = p_order_manager_->GetOrder(cancel_order_serial_no);
    if (!canceled_order) return;

    // update matched volume and order status
    match_price = canceled_order->limit_price;
    if (inner_matched_volume > 0)
    {
        matched_order->UpdateInnerMatchedVolume(inner_matched_volume);
        canceled_order->UpdateInnerMatchedVolume(inner_matched_volume);

        if (canceled_order->is_transform)
        {
            p_position_manager_->InnerMatchTransferOrder(canceled_order->stock_code,
                canceled_order->open_close, inner_matched_volume);
        }
    }

    // 1. 被撤单-内部撮合 (需要修改所有子单已经终结的回报流程,报出成交回报后，应该再次修正状态，补报终结状态)
    if (inner_matched_volume > 0)
    {
        OrderInfoInEx *top_cancel_order = p_order_manager_->UpdateUpperOrders(canceled_order, inner_matched_volume);
        if (top_cancel_order && top_cancel_order->super_serial_no == 0)
        {
            // 造成了顶层报单的状态更新，需要回报
            // 1. 被撤单回报；
            T_OrderReturn order_return;
            top_cancel_order->CreateOrderReturn(order_return);

            p_my_exchange_->SendOrderReturn(&order_return, inner_matched_volume);

            // 2. 被撤单成交回报
            T_TradeReturn inner_trade;
            top_cancel_order->CreateTradeReturn(inner_trade, inner_matched_volume);
            p_my_exchange_->SendTradeReturn(&inner_trade);

            // rescue a terminated status
            ResendTerminateStatusAfterTradeReturn(top_cancel_order, inner_matched_volume);
        }
    }

    // 2.  被撤单补单
    if (canceled_order->IsInWaitingCancelStatus())
    {
        canceled_order->ClearInWaitingCancelStatus();
        OrderInfoInEx *top_cancel_order = p_order_manager_->UpdateOrder(canceled_order->serial_no,
            MY_TNL_OS_WITHDRAWED);
        if (top_cancel_order && top_cancel_order->super_serial_no == 0)
        {
            // 造成了顶层报单的状态更新，需要回报
            T_OrderReturn order_return;
            top_cancel_order->CreateOrderReturn(order_return);

            p_my_exchange_->SendOrderReturn(&order_return, 0);
        }
    }
    else if (canceled_order->volume_remain > 0)
    {
        OrderInfoInEx* rescue_order = new OrderInfoInEx(canceled_order,
            p_order_manager_->GetNewSerialNo(canceled_order->serial_no),
            canceled_order->volume_remain,
            false);
        // fix bug on 20150108 需要根据原单是否变换单，恢复开平方向
        if (canceled_order->is_transform)
        {
            if (rescue_order->open_close == MY_TNL_D_OPEN)
            {
                rescue_order->open_close = MY_TNL_D_CLOSE;
            }
            else
            {
                rescue_order->open_close = MY_TNL_D_OPEN;
            }
        }
        p_order_manager_->AddOrder(rescue_order);
        p_order_req_->PlaceOrderImp(rescue_order);
    }

    // 3. 报单内部撮合
    if (inner_matched_volume > 0)
    {
        OrderInfoInEx *top_matched_order = p_order_manager_->UpdateUpperOrders(matched_order, inner_matched_volume);

        if (top_matched_order && top_matched_order->super_serial_no == 0)
        {
            // 造成了顶层报单的状态更新，需要回报
            // 1. 报单状态回报；
            T_OrderReturn order_return;
            top_matched_order->CreateOrderReturn(order_return);

            p_my_exchange_->SendOrderReturn(&order_return, inner_matched_volume);

            // 2. 成交回报
            T_TradeReturn inner_trade;
            top_matched_order->CreateTradeReturn(inner_trade, inner_matched_volume);
            inner_trade.business_price = match_price;
            p_my_exchange_->SendTradeReturn(&inner_trade);

            // rescue a terminated status
            ResendTerminateStatusAfterTradeReturn(top_matched_order, inner_matched_volume);
        }
    }
    // 4. 新报单的剩余部分，如果自成交撤单全部回报了，需要补单
    CheckAndRescueMatchedOrder(matched_order);

    // 更新内部撮合仓位
    p_position_manager_->UpdateInnerPosition(canceled_order, matched_order, inner_matched_volume);
}

void MYOrderRsp::CheckAndRescueMatchedOrder(OrderInfoInEx* matched_order)
{
    // 新报单的剩余部分，如果自成交撤单全部回报了，需要补单
    if (matched_order->AllMatched())
    {
        if (matched_order->IsInWaitingCancelStatus())
        {
            matched_order->ClearInWaitingCancelStatus();
            OrderInfoInEx *top_matched_order = p_order_manager_->UpdateOrder(matched_order->serial_no,
                MY_TNL_OS_WITHDRAWED);
            if (top_matched_order && top_matched_order->super_serial_no == 0)
            {
                // 造成了顶层报单的状态更新，需要回报
                T_OrderReturn order_return;
                top_matched_order->CreateOrderReturn(order_return);
                p_my_exchange_->SendOrderReturn(&order_return, 0);
            }
        }
        else if (matched_order->volume_remain > 0)
        {
            OrderInfoInEx* rescue_order = new OrderInfoInEx(matched_order,
                p_order_manager_->GetNewSerialNo(matched_order->serial_no),
                matched_order->volume_remain, false);
            p_order_manager_->AddOrder(rescue_order);
            p_order_req_->PlaceOrderImp(rescue_order);
        }
    }
}

void MYOrderRsp::UpdateAndReport(bool from_myex,
                                bool from_order_return,
                                int error_no,
                                SerialNoType serial_no,
                                EntrustNoType entrust_no,
                                char entrust_status,
                                VolumeType volume_remain)
{
    VolumeType matched_volume = 0;

    // update order status, entrust_no, and volume_remain
    OrderInfoInEx *top_updated_order = p_order_manager_->UpdateOrder(serial_no, entrust_status, entrust_no, volume_remain,
        matched_volume);

    if (top_updated_order && top_updated_order->super_serial_no == 0)
    {
        // 造成了顶层报单的状态更新，需要回报
        if (from_order_return)
        {
            T_OrderReturn order_return;
            top_updated_order->CreateOrderReturn(order_return);

            p_my_exchange_->SendOrderReturn(&order_return, matched_volume);
        }
        else
        {
            T_OrderRespond order_rsp;
            OrderUtil::CreateOrderRespond(order_rsp, error_no, top_updated_order->serial_no,
                top_updated_order->entrust_no, top_updated_order->entrust_status);

            p_my_exchange_->SendOrderRespond(&order_rsp, top_updated_order->stock_code, top_updated_order->direction,
                top_updated_order->volume_remain);
        }
    }
}

void MYOrderRsp::OrderRspProcess(bool from_myex,
                                bool from_order_return,
                                int error_no,
                                SerialNoType serial_no,
                                EntrustNoType entrust_no,
                                char entrust_status,
                                VolumeType volume_remain /* = -1 */)
{
    OrderInfoInEx *order = p_order_manager_->GetOrder(serial_no);
    if (NULL == order)
    {
        EX_LOG_ERROR("MYOrderRsp::OrderRspProcess, order not found, serial_no=%ld", serial_no);
        return;
    }

    if (-1 == volume_remain)
    {
        volume_remain = order->volume_remain;
    }

    /* the order_return is the order_return just before a trade_return */
    if (!from_myex && (volume_remain < order->volume_remain))
    {
        VolumeType volume_matched = order->volume_remain - volume_remain;
        p_position_manager_->UpdateOutterPosition(order->stock_code, order->direction, order->open_close, volume_matched,
            order->is_transform);
    }

    // 如果被终结，freez position and pending volume要回滚
    if (!from_myex
        && (volume_remain > 0)
        && OrderUtil::IsTerminated(entrust_status))
    {
        p_position_manager_->RollBackPendingVolume(order->stock_code, order->direction, order->open_close, volume_remain);
    }

    // 变换单终结，要回滚内部仓位（变换时已经预扣）
    if (volume_remain > 0
        && order->is_transform
        && OrderUtil::IsTerminated(entrust_status))
    {
        p_position_manager_->RollBackTransformVolume(order->stock_code, order->direction, order->open_close, volume_remain);
    }

    // 自成交预处理，市场撮合剩下的单重新发出
//     if (order->is_prepare_for_inner_match
//         && volume_remain > 0
//         && OrderUtil::IsTerminated(entrust_status))
//     {
//         if (order->have_process_remain_flag)
//         {
//             EX_LOG_WARN("order %ld receive terminate flag after process remain volume", serial_no);
//             return;
//         }
//         if (order->super_serial_no != 0)
//         {
//             OrderInfoInEx *supper_order = p_order_manager_->GetOrder(order->super_serial_no);
//             while (supper_order && supper_order->is_prepare_for_inner_match)
//             {
//                 supper_order->have_process_remain_flag = true;
//                 supper_order->entrust_status = entrust_status;
//                 supper_order->volume_remain = volume_remain;
//                 supper_order = p_order_manager_->GetOrder(supper_order->super_serial_no);
//             }
//             if (!supper_order)
//             {
//                 EX_LOG_ERROR("MYOrderRsp::OrderRspProcess, order not found, serial_no=%ld", order->super_serial_no);
//             }
//             else
//             {
//                 OrderInfoInEx* rescue_order = new OrderInfoInEx(supper_order,
//                     p_order_manager_->GetNewSerialNo(supper_order->serial_no),
//                     volume_remain, false, true, order->limit_price);
//                 // fix bug on 20150108 需要根据原单是否变换单，恢复开平方向
//                 if (supper_order->is_transform)
//                 {
//                     if (rescue_order->open_close == MY_TNL_D_OPEN)
//                     {
//                         rescue_order->open_close = MY_TNL_D_CLOSE;
//                     }
//                     else
//                     {
//                         rescue_order->open_close = MY_TNL_D_OPEN;
//                     }
//                 }
//                 order->have_process_remain_flag = true;
//                 order->entrust_status = entrust_status;
//                 order->volume_remain = volume_remain;
//                 p_order_manager_->AddOrder(rescue_order);
//                 p_order_req_->PlaceOrderImp(rescue_order);
//             }
//         }
//         else
//         {
//             EX_LOG_ERROR("MYOrderRsp::OrderRspProcess, pre_inner_match order no supper serial_no, %ld", serial_no);
//         }
// 
//         return;
//     }

    // 内部撮合的挂单终结，需要完成撮合动作
//     SerialNoType new_matched_serial_no;
//     if (OrderUtil::IsTerminated(entrust_status)
//         && p_order_manager_->IsOrderSerialNoForInnerMatch(serial_no, new_matched_serial_no))
//     {
//         if (order->entrust_no == 0)
//         {
//             p_order_manager_->RemoveFromPendingCancel(serial_no);
//         }
// 
//         if (volume_remain == 0)
//         {
//             UpdateAndReport(from_myex, from_order_return, error_no, serial_no, entrust_no, entrust_status, volume_remain);
//         }
//         InnerMatch(new_matched_serial_no, order->serial_no, volume_remain);
//         return;
//     }

    // 内部撮合的盘口挂单,因原来没有委托号在等待撤单列表中，需发出撤单
//     T_CancelOrder co;
//     if ((entrust_no != 0) 
//         && (order->entrust_no == 0) 
//         && (p_order_manager_->GetAndRemovePendingCancel(order, co, entrust_no)))
//     {
//         p_my_exchange_->CancelOrderToTunnel(&co);
// 
//         UpdateAndReport(from_myex, from_order_return, error_no, serial_no, entrust_no, entrust_status, volume_remain);
// 
//         p_order_manager_->UpdateOrder(serial_no, MY_TNL_OS_WITHDRAWING);
// 
//         if (cancel_first && order->IsInWaitingCancelStatus())
//         {
//             OrderInfoInEx *new_order_inner_match = p_order_manager_->GetNewOrderOfInnerMatch(serial_no);
// 
//             if (new_order_inner_match)
//             {
//                 // 清除撮合关系
//                 p_order_manager_->RemoveOrderInnerMatchOrderRelation(serial_no, new_order_inner_match);
// 
//                 // 对手单没有内部撮合了，需要报出
//                 if (!new_order_inner_match->InInnerMatching() && new_order_inner_match->volume_remain > 0)
//                 {
//                     OrderInfoInEx* rescue_order = new OrderInfoInEx(new_order_inner_match,
//                         p_order_manager_->GetNewSerialNo(new_order_inner_match->serial_no),
//                         new_order_inner_match->volume_remain, false);
//                     p_order_manager_->AddOrder(rescue_order);
//                     p_order_req_->PlaceOrderImp(rescue_order);
//                 }
//             }
//             order->ClearInWaitingCancelStatus();
//         }
//         return;
//     }

    // update order status,entrust_no, and volume_remain
    UpdateAndReport(from_myex, from_order_return, error_no, serial_no, entrust_no, entrust_status, volume_remain);
}
