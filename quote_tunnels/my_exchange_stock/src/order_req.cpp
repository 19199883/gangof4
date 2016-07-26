#include "order_req.h"
#include "order_data.h"
#include "position_data.h"
#include "my_exchange.h"

void MYOrderReq::PlaceOrderImp(OrderInfoInEx *place_order)
{
    MY_LOG_DEBUG("MYOrderReq::PlaceOrderImp - %ld", place_order->serial_no);
    T_PlaceOrder po = place_order->ToPlaceOrder();
    p_my_exchange_->PlaceOrderToTunnel(&po);

}

void MYOrderReq::CancelOrderImp(const T_CancelOrder *cancel_info)
{
	 OrderInfoInEx *dest_order = p_order_manager_->GetOrder(cancel_info->org_serial_no);
	if (dest_order!=NULL){
		if (OrderUtil::IsTerminated(dest_order->entrust_status)){
			T_CancelRespond cancel_rsp = OrderUtil::CreateCancelRespond(
					TUNNEL_ERR_CODE::DUPLICATE_ORDER_ACTION_REF,cancel_info->serial_no, 0);
			p_my_exchange_->SendCancelRespond(&cancel_rsp);
		}else{
			p_my_exchange_->CancelOrderToTunnel(cancel_info);
			p_order_manager_->UpdateOrder(cancel_info->org_serial_no, BUSSINESS_DEF::ENTRUST_STATUS_WITHDRAWING);
		}
	}else{
		T_CancelRespond cancel_rsp = OrderUtil::CreateCancelRespond(TUNNEL_ERR_CODE::ORDER_NOT_FOUND, cancel_info->serial_no, 0);
		p_my_exchange_->SendCancelRespond(&cancel_rsp);
	}
}



