#ifndef _MY_EXCHANGE_ORDER_REQ_H_
#define  _MY_EXCHANGE_ORDER_REQ_H_

#include "my_exchange_datatype_inner.h"
#include "my_exchange_utility.h"
#include "config_data.h"

class MYExchange;
class MYOrderDataManager;
class MYPositionDataManager;

class MYOrderReq
{
public:
	MYOrderReq(MYExchange * p_my_exchange, MYOrderDataManager *p_order_manager,
        MYPositionDataManager *p_position_manager, const MYExConfigData &cfg)
		:p_my_exchange_(p_my_exchange), p_order_manager_(p_order_manager), p_position_manager_(p_position_manager),
		 cfg_(cfg)
	{
		cancel_first = false;
        change_oc_flag = cfg_.Position_policy().change_oc_flag;
	}

	~MYOrderReq(){}

	void PlaceOrderImp(OrderInfoInEx *place_order);
	void CancelOrderImp(const T_CancelOrder *cancel_order);

    void InsertQuoteImp(QuoteInfoInEx *p);
    void CancelQuoteImp(const T_CancelQuote *p);

private:
	// forbid copy
	MYOrderReq(const MYOrderReq &);
	MYOrderReq & operator=(const MYOrderReq &);

	bool cancel_first;
	bool change_oc_flag;

	// associated object
	MYExchange * p_my_exchange_;
	MYOrderDataManager *p_order_manager_;
	MYPositionDataManager *p_position_manager_;

	MYExConfigData cfg_;

	// 自成交核查及处理
	// 1. 撤销未成单（价格差的优先，新报的优先），尽量使未成量 >= 报单量
	// 2. 撤单关联本次的报单
	// 3. 发出撤单，记录撤单标记（该单不再进入自成交搜索，报单进入 ENTRUST_STATUS_WITHDRAWING 状态）
	// 4. 本次的报单，关联上撤单列表
	// 5. 如果未成量 < 报单量，先报出剩余部分，本次报单关联新报单
	//SelfTradeResult HandleSelfTradeOrders(OrderInfoInEx *place_order);

	// 优先使用内部仓位，报单变换开平后报出
	void HandleTransform(OrderInfoInEx *place_order);

	// 检查是否需要报出撤单，需要则返回所有叶节点报单
	bool CheckAndGetTransformedOrders(const T_CancelOrder * cancel_info, OrderCollection &transformed_orders);

	// 自成交撮合前，先低一个价位到市场撮合
    //void SendToMarketBeforeInnerMatch(double best_price, double tick_price, OrderInfoInEx* place_order);
};

#endif
