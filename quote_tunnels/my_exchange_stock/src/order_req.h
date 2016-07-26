#ifndef _MY_EXCHANGE_ORDER_REQ_H_
#define  _MY_EXCHANGE_ORDER_REQ_H_

#include "my_exchange_datatype_inner.h"
#include "my_exchange_utility.h"

class MYExchange;
class MYOrderDataManager;
class MYPositionDataManager;

class MYOrderReq
{
public:
	MYOrderReq(MYExchange * p_my_exchange, MYOrderDataManager *p_order_manager,
		MYPositionDataManager *p_position_manager)
		:p_my_exchange_(p_my_exchange), p_order_manager_(p_order_manager), p_position_manager_(p_position_manager)
	{
		cancel_first = false;
	}

	~MYOrderReq(){}

	void PlaceOrderImp(OrderInfoInEx *place_order);
	void CancelOrderImp(const T_CancelOrder *cancel_order);

private:
	// forbid copy
	MYOrderReq(const MYOrderReq &);
	MYOrderReq & operator=(const MYOrderReq &);

	bool cancel_first;

	// associated object
	MYExchange * p_my_exchange_;
	MYOrderDataManager *p_order_manager_;
	MYPositionDataManager *p_position_manager_;

};

#endif
