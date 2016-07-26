#include "order_req.h"
#include "order_data.h"
#include "position_data.h"
#include "my_exchange.h"

void MYOrderReq::PlaceOrderImp(OrderInfoInEx *place_order)
{
    EX_LOG_DEBUG("MYOrderReq::PlaceOrderImp - %ld", place_order->serial_no);

    // can't support self trade when don't support transform open-close
    if (!change_oc_flag)
    {
        // no transform
        T_PlaceOrder po;
        place_order->CreatePlaceOrder(po);
        p_my_exchange_->PlaceOrderToTunnel(&po);
        return;
    }

    HandleTransform(place_order);
//     // fak/fok, NO self-trade
//     if (place_order->order_type == MY_TNL_HF_FAK || place_order->order_type == MY_TNL_HF_FOK)
//     {
//         return HandleTransform(place_order);
//     }
// 
//     // 自成交处理
//     SelfTradeResult self_trade_res = HandleSelfTradeOrders(place_order);
//     if (self_trade_res == SelfTradeResult::NoSelfTrade)
//     {
//         // 开平变换处理
//         HandleTransform(place_order);
//     }
}

void MYOrderReq::CancelOrderImp(const T_CancelOrder *cancel_info)
{
    OrderCollection transformed_orders;
    if (!CheckAndGetTransformedOrders(cancel_info, transformed_orders))
    {
        return;
    }

    // 对所有变换单，发出撤单请求
    for (OrderInfoInEx *po : transformed_orders)
    {
        if (po->HasTransform())
        {
            // 如果有变换单，撤变换单即可，本身无需处理
            /* In CheckAndGetTransformedOrders, we have recursively retrieve all transformed orders of one order. */
        }
//         else if (po->is_inner_matched)
//         {
//             OrderInfoInEx *new_order_inner_match = p_order_manager_->GetNewOrderOfInnerMatch(po->serial_no);
// 
//             // 在内部撮合状态中(盘口挂单)
//             if (new_order_inner_match)
//             {
//                 po->SetInWaitingCancelStatus();
// 
//                 // 如果不在PendingCancel集合中
//                 if (!p_order_manager_->InPendingCancel(po->serial_no))
//                 {
//                     if (cancel_first)
//                     {
//                         // 清除撮合关系
//                         p_order_manager_->RemoveOrderInnerMatchOrderRelation(po->serial_no, new_order_inner_match);
// 
//                         // 对手单没有内部撮合了，需要报出
//                         if (!new_order_inner_match->InInnerMatching() && new_order_inner_match->volume_remain > 0)
//                         {
//                             OrderInfoInEx* rescue_order = new OrderInfoInEx(new_order_inner_match,
//                                 p_order_manager_->GetNewSerialNo(new_order_inner_match->serial_no),
//                                 new_order_inner_match->volume_remain, false);
//                             p_order_manager_->AddOrder(rescue_order);
//                             PlaceOrderImp(rescue_order);
//                         }
//                     }
//                 }
//             }
// 
//             // 报单在自成交撮合中(报单)
//             if (po->InInnerMatching())
//             {
//                 po->SetInWaitingCancelStatus();
//                 if (cancel_first)
//                 {
//                     p_order_manager_->CancelInnerMatching(po);
//                     OrderInfoInEx *top_updated_order = p_order_manager_->UpdateOrder(po->serial_no,
//                     MY_TNL_OS_WITHDRAWED);
//                     if (top_updated_order && top_updated_order->super_serial_no == 0)
//                     {
//                         // 造成了顶层报单的状态更新，需要回报
//                         T_OrderReturn order_return;
//                         top_updated_order->CreateOrderReturn(order_return);
// 
//                         p_my_exchange_->SendOrderReturn(&order_return, 0);
//                     }
//                 }
//             }
//         }
        else
        {
            if (po->serial_no == cancel_info->org_serial_no)
            {
                /* This means that the org_serial_no order has NO transformed order. */
                // self，直接发出
                p_my_exchange_->CancelOrderToTunnel(cancel_info);
                p_order_manager_->UpdateOrder(cancel_info->org_serial_no, MY_TNL_OS_WITHDRAWING);
            }
            // fix bug 20141118, add refused order to pending cancel list
            else if (!OrderUtil::IsTerminated(po->entrust_status))
            {
                T_CancelOrder new_co;
                po->CreateCancelOrder(new_co, p_order_manager_->GetNewSerialNo(po->serial_no));

                if (!p_order_manager_->ShouldAddToPendingCancel(po, new_co))
                {
                    p_my_exchange_->CancelOrderToTunnel(&new_co);
                    p_order_manager_->UpdateOrder(new_co.org_serial_no, MY_TNL_OS_WITHDRAWING);
                }
                else
                {
                    // 如果报单还没有委托号，要在收到报单响应，获得委托号时处理 handled in ShouldAddToPendingCancel
                }
            }
        }
    }
}
// 
// void MYOrderReq::SendToMarketBeforeInnerMatch(double best_price, double tick_price, OrderInfoInEx* place_order)
// {
//     double new_limit_price = best_price + tick_price; // sell
//     if (place_order->direction == MY_TNL_D_BUY)
//     {
//         new_limit_price = best_price - tick_price;
//     }
//     OrderInfoInEx* pre_innermatch_order = new OrderInfoInEx(place_order, p_order_manager_->GetNewSerialNo(place_order->serial_no),
//         place_order->volume_remain, false);
//     pre_innermatch_order->is_prepare_for_inner_match = true;
//     pre_innermatch_order->limit_price = new_limit_price;
//     pre_innermatch_order->order_kind = MY_TNL_OPT_LIMIT_PRICE;
//     pre_innermatch_order->order_type = MY_TNL_HF_FAK;
//     // fix bug on 20150108 需要根据原单是否变换单，恢复开平方向
//     if (place_order->is_transform)
//     {
//         if (pre_innermatch_order->open_close == MY_TNL_D_OPEN)
//         {
//             pre_innermatch_order->open_close = MY_TNL_D_CLOSE;
//         }
//         else
//         {
//             pre_innermatch_order->open_close = MY_TNL_D_OPEN;
//         }
//     }
//     p_order_manager_->AddOrder(pre_innermatch_order);
//     HandleTransform(pre_innermatch_order);
// }

// SelfTradeResult MYOrderReq::HandleSelfTradeOrders(OrderInfoInEx * place_order)
// {
//     // PendingCancel 中存在自成交的单，拒绝
//     if (p_order_manager_->HaveSelfTradeOrderInPendingCancel(place_order))
//     {
//         EX_LOG_WARN("hold up because there are pending cancel orders, maybe match it.");
//         OrderInfoInEx *top_updated_order = p_order_manager_->UpdateOrder(place_order->serial_no,
//         MY_TNL_OS_ERROR);
//         if (top_updated_order && top_updated_order->super_serial_no == 0)
//         {
//             // 造成了顶层报单的状态更新，需要回报
//             if (place_order->super_serial_no == 0)
//             {
//                 T_OrderRespond order_rsp;
//                 OrderUtil::CreateOrderRespond(order_rsp, TUNNEL_ERR_CODE::POSSIBLE_SELF_TRADE,
//                     place_order->serial_no, 0, MY_TNL_OS_ERROR);
// 
//                 p_my_exchange_->SendOrderRespond(&order_rsp, place_order->stock_code, place_order->direction,
//                     place_order->volume_remain);
//             }
//             else
//             {
//                 T_OrderReturn order_return;
//                 top_updated_order->CreateOrderReturn(order_return);
// 
//                 p_my_exchange_->SendOrderReturn(&order_return, 0);
//             }
//         }
//         return SelfTradeResult::HoldUp;
//     }
// 
//     // 0. 查找自成交挂单
//     OrderCollection self_trade_orders = p_order_manager_->GetSelfTradeOrders(place_order);
//     EX_LOG_INFO("MYOrderReq::HandleSelfTradeOrders - %ld self trade order size(%d)", place_order->serial_no,
//         self_trade_orders.size());
// 
//     if (!self_trade_orders.empty())
//     {
//         // TODO 报单量太小，挂单都很大，直接拒绝 （业务规则还需要细化）
// 
//         // prepare for inner match: match in market first
//         double best_price = self_trade_orders.front()->limit_price;
//         bool farther_price = false;
//         double tick_price = cfg_.GetTickPriceOfContract(place_order->stock_code);
//         if (place_order->prev_is_fak_to_prepare_inner_match)
//         {
// 
//             if (place_order->direction == MY_TNL_D_BUY
//                 && best_price > place_order->limit_price + tick_price + 0.00005)
//             {
//                 farther_price = true;
//             }
//             else if (place_order->direction == MY_TNL_D_SELL
//                 && best_price < place_order->limit_price - tick_price - 0.00005)
//             {
//                 farther_price = true;
//             }
//         }
// 
//         // 报单还没有到市场撮合过，或虽然是市场撮合剩下的单，但盘口价格更远了
//         // 先用低一个tick的价位，报到市场
//         if (!place_order->prev_is_fak_to_prepare_inner_match || farther_price)
//         {
//             SendToMarketBeforeInnerMatch(best_price, tick_price, place_order);
//         }
//         else
//         {
//             VolumeType possible_canceled_volume = 0;
//             double first_price = self_trade_orders.front()->limit_price;
// 
//             // 派生出自成交撮合的报单（全部量放入内部撮合）
//             OrderInfoInEx *innermatch_order = new OrderInfoInEx(place_order,
//                 p_order_manager_->GetNewSerialNo(place_order->serial_no),
//                 place_order->volume_remain,
//                 false);
//             p_order_manager_->AddOrder(innermatch_order);
// 
//             // 1. 撤销未成单（价格好的优先，新报的优先），尽量使未成量 >= 报单量
//             for (OrderInfoInEx * po : self_trade_orders)
//             {
//                 // 自成交撮合不同价位时，需要先到市场撮合
//                 if ((po->direction == MY_TNL_D_BUY && (po->limit_price + 0.00005) < first_price)
//                     || (po->direction == MY_TNL_D_SELL && (po->limit_price - 0.00005) > first_price))
//                 {
//                     break;
//                 }
// 
//                 possible_canceled_volume += po->volume_remain;
//                 T_CancelOrder new_co;
//                 po->CreateCancelOrder(new_co, p_order_manager_->GetNewSerialNo(po->serial_no));
// 
//                 // 2. 撤单关联本次的报单
//                 po->is_inner_matched = true;
//                 p_order_manager_->InsertOrderSerialNoForInnerMatch(new_co.org_serial_no, innermatch_order->serial_no);
//                 p_order_manager_->UpdateOrder(po->serial_no, MY_TNL_OS_INNERMATCHING);
// 
//                 // 3. 发出撤单，记录撤单标记（该单不再进入自成交搜索，报单进入 ENTRUST_STATUS_INNERMATCHING 状态）
//                 if (!p_order_manager_->ShouldAddToPendingCancel(po, new_co))
//                 {
//                     p_my_exchange_->CancelOrderToTunnel(&new_co);
//                 }
//                 else
//                 {
//                     // 如果报单还没有委托号，就因自成交需要撤单，在收到报单响应，获得委托号时处理 handled in ShouldAddToPendingCancel
//                 }
// 
//                 // 4. 本次的报单，关联上撤单列表
//                 innermatch_order->RecordMatchOneOrder(po->serial_no, po->volume_remain);
// 
//                 // 可能撤够了，退出
//                 if (possible_canceled_volume >= innermatch_order->volume_remain)
//                 {
//                     break;
//                 }
//             }
//             // 更新报单状态
//             innermatch_order->is_inner_matched = true;
//             (void) p_order_manager_->UpdateOrder(innermatch_order->serial_no, MY_TNL_OS_INNERMATCHING);
// 
//             // 5. 如果未成量 < 报单量，先报出剩余部分，本次报单关联新报单
//             VolumeType rescue_volume = innermatch_order->volume_remain - possible_canceled_volume;
//             EX_LOG_INFO("possible_canceled_volume: %ld, place volume: %ld", possible_canceled_volume, place_order->volume_remain);
//             if (rescue_volume > 0 && p_order_manager_->NoPendingCancel())
//             {
//                 // 修正内部撮合的量，
//                 innermatch_order->volume -= rescue_volume;
//                 innermatch_order->volume_remain -= rescue_volume;
// 
//                 // 补单
//                 OrderInfoInEx *rescue_order = new OrderInfoInEx(place_order,
//                     p_order_manager_->GetNewSerialNo(place_order->serial_no),
//                     rescue_volume, false);
//                 p_order_manager_->AddOrder(rescue_order);
//                 PlaceOrderImp(rescue_order);
//             }
// 
//             if (possible_canceled_volume > 0 && place_order->super_serial_no == 0)
//             {
//                 T_OrderRespond order_rsp;
//                 OrderUtil::CreateOrderRespond(order_rsp, TUNNEL_ERR_CODE::RESULT_SUCCESS,
//                     place_order->serial_no, 0, MY_TNL_OS_REPORDED);
// 
//                 p_my_exchange_->SendOrderRespond(&order_rsp, place_order->stock_code, place_order->direction,
//                     place_order->volume_remain);
//             }
//         }
//         return SelfTradeResult::SelfTrade;
//     }
// 
//     return SelfTradeResult::NoSelfTrade;
// }

void MYOrderReq::HandleTransform(OrderInfoInEx *p)
{
    // 获取需要转换的量
    VolumeType transform_volume = p_position_manager_->GetTransformVolume(p);
    EX_LOG_INFO("MYOrderReq::HandleTransform, transform_volume=%d", transform_volume);

    if (transform_volume <= 0)
    {
        T_PlaceOrder po;
        p->CreatePlaceOrder(po);
        p_my_exchange_->PlaceOrderToTunnel(&po);
        return;
    }

    VolumeType close_volume = 0;
    VolumeType open_volume = 0;
    bool close_is_transform = false;
    bool open_is_transform = false;
    if (p->open_close == MY_TNL_D_OPEN)
    {
        open_volume = p->volume_remain - transform_volume;
        close_volume = transform_volume;
        close_is_transform = true;
    }
    else
    {
        close_volume = p->volume_remain - transform_volume;
        open_volume = transform_volume;
        open_is_transform = true;
    }

    // fix bug on 20141230, send respond before place order, to avoid respond after tunnel
    if (0 == p->super_serial_no)
    {
        T_OrderRespond order_rsp;
        OrderUtil::CreateOrderRespond(order_rsp, TUNNEL_ERR_CODE::RESULT_SUCCESS, p->serial_no, 0,
        MY_TNL_OS_REPORDED);

        p_my_exchange_->SendOrderRespond(&order_rsp, p->stock_code, p->direction, p->volume_remain);
    }

    // close first
    if (close_volume > 0)
    {
        OrderInfoInEx *close_order = new OrderInfoInEx(p, p_order_manager_->GetNewSerialNo(p->serial_no), close_volume,
            close_is_transform);
        close_order->open_close = MY_TNL_D_CLOSE;
        p_order_manager_->AddOrder(close_order);
        T_PlaceOrder po;
        close_order->CreatePlaceOrder(po);
        p_my_exchange_->PlaceOrderToTunnel(&po);
    }

    // then open
    if (open_volume > 0)
    {
        OrderInfoInEx *open_order = new OrderInfoInEx(p, p_order_manager_->GetNewSerialNo(p->serial_no), open_volume,
            open_is_transform);
        open_order->open_close = MY_TNL_D_OPEN;
        p_order_manager_->AddOrder(open_order);
        T_PlaceOrder po;
        open_order->CreatePlaceOrder(po);
        p_my_exchange_->PlaceOrderToTunnel(&po);

    }
}

/*
 * The function has 2 targets:
 * 1. If something is wrong with the cancel_order, respond immediately.
 * 2. If not, return transformed orders.
 */
bool MYOrderReq::CheckAndGetTransformedOrders(const T_CancelOrder * cancel_info, OrderCollection &transformed_orders)
{
    OrderInfoInEx *org_pl_ord = p_order_manager_->GetOrder(cancel_info->org_serial_no);
    if (NULL == org_pl_ord)
    {
        /* Invalid cancel_order, reject it! */
        T_CancelRespond cancel_rsp;
        OrderUtil::CreateCancelRespond(cancel_rsp, TUNNEL_ERR_CODE::ORDER_NOT_FOUND, cancel_info->serial_no, 0);
        p_my_exchange_->SendCancelRespond(&cancel_rsp);
        return false;
    }

    if (OrderUtil::IsTerminated(org_pl_ord->entrust_status))
    {
        /* Invalid cancel_order, reject it! */
        T_CancelRespond cancel_rsp;
        OrderUtil::CreateCancelRespond(cancel_rsp, TUNNEL_ERR_CODE::DUPLICATE_ORDER_ACTION_REF,
            cancel_info->serial_no, 0);
        p_my_exchange_->SendCancelRespond(&cancel_rsp);
        return false;
    }
    else if (org_pl_ord->HasTransform())
    {
        /* 
         * Because it's a master order of transformed orders(maybe 2), so the tunnel would not respond it. 
         * So, we must respond it here.
         */
        T_CancelRespond cancel_rsp;
        OrderUtil::CreateCancelRespond(cancel_rsp, TUNNEL_ERR_CODE::RESULT_SUCCESS, cancel_info->serial_no, 0);
        p_my_exchange_->SendCancelRespond(&cancel_rsp);
    }
    else
    {
        // 没有变换，也没有内部撮合的单，等通道的回报，此处无需respond
    }

    // 查找该单对应的最终发出去的报单（开平反转、补单等）
    transformed_orders.push_back(org_pl_ord); // 将自己加进入，便于统一处理
    p_order_manager_->GetTransformedOrders(org_pl_ord, transformed_orders);

    return true;
}
void MYOrderReq::InsertQuoteImp(QuoteInfoInEx *p)
{
    EX_LOG_DEBUG("MYOrderReq::InsertQuoteImp - %d", p->data.serial_no);
    p_position_manager_->CheckAndTransformQuote(p);
    p_my_exchange_->InsertQuoteToTunnel(&p->data);
}
void MYOrderReq::CancelQuoteImp(const T_CancelQuote *p)
{
    p_my_exchange_->CancelQuoteToTunnel(p);
}
