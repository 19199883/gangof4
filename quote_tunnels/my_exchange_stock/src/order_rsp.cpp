#include "order_rsp.h"
#include "my_exchange_utility.h"
#include "order_data.h"
#include "position_data.h"
#include "order_req.h"
#include "my_exchange.h"
#include "my_exchange_datatype.h"

void MYOrderRsp::OrderRespondImp(const T_OrderRespond * p)
{
    return OrderRspProcess(false, p->error_no, p->serial_no, p->entrust_no, p->entrust_status,p->exchange_type);
}

void MYOrderRsp::OrderReturnImp(bool from_myex, const T_OrderReturn * p)
{
    return OrderRspProcess(true, 0, p->serial_no, p->entrust_no, p->entrust_status, p->volume_remain);
}

void MYOrderRsp::TradeReturnImp(bool from_myex, const T_TradeReturn * t_rtn)
{
    MY_LOG_DEBUG("MYOrderRsp::TradeReturnImp, serial_no=%ld", t_rtn->serial_no);

	T_PositionData pos = p_position_manager_->GetPositionInMarketOfCode(t_rtn->stock_code);
	p_my_exchange_->SendTradeReturn(t_rtn, &pos);
}

void MYOrderRsp::UpdateAndReport(
		bool from_order_return,int error_no,SerialNoType serial_no,
		EntrustNoType entrust_no,char entrust_status,VolumeType volume_remain,const T_PositionData *pos)
{
    OrderInfoInEx *top_updated_order = p_order_manager_->UpdateOrder(
    		serial_no,entrust_status, entrust_no, volume_remain);
	if (from_order_return){
		T_OrderReturn order_return = top_updated_order->CreateOrderReturn();
		p_my_exchange_->SendOrderReturn(&order_return, pos);
	}else{
		T_OrderRespond order_rsp = OrderUtil::CreateOrderRespond(
				error_no, top_updated_order->serial_no,top_updated_order->entrust_no, top_updated_order->entrust_status);
		p_my_exchange_->SendOrderRespond(&order_rsp, pos);
	}

}

void MYOrderRsp::OrderRspProcess(bool from_order_return,int error_no,
    SerialNoType serial_no,EntrustNoType entrust_no,char entrust_status, char ex,VolumeType volume_remain /* = -1 */)
{
    OrderInfoInEx *order = p_order_manager_->GetOrder(serial_no);
    if (order!=NULL){
		if (volume_remain == -1) volume_remain = order->volume_remain;

		bool position_changed = false;
		if (volume_remain < order->volume_remain){
			VolumeType volume_matched = order->volume_remain - volume_remain;
			position_changed = true;
			p_position_manager_->UpdateOutterPosition(
					order->stock_code, order->direction,order->open_close, volume_matched,ex);
		}

		T_PositionData pos = p_position_manager_->GetPositionInMarketOfCode(order->stock_code);
		if (position_changed) pos.update_flag = 1;
		UpdateAndReport(from_order_return, error_no, serial_no, entrust_no,entrust_status, volume_remain, &pos);

    }else{
    	MY_LOG_ERROR("MYOrderRsp::OrderRspProcess, order not found, serial_no=%ld", serial_no);
    }
}
