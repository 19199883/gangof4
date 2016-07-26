/// 系统内部使用的数据类型定义

#ifndef _MY_EXCHANGE_DATATYPE_INNER_H_
#define  _MY_EXCHANGE_DATATYPE_INNER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
#include <string.h>

#include "my_exchange_utility.h"
#include "my_trade_tunnel_api.h"


struct MYExchangeConfigData
{
    // 通道的配置（通道id对应通道配置文件）

    // MYExchange内部的配置

    // relation of Strategy id and tunnel id
    // 模型在多个账号间均衡下单，每个账号有最大下单数，可能需要拆单
    // 一个账号，可能对应多个物理通道（多个账号间进行均衡，可能不需要多个物理通道了；或者一个账号对应多个物理通道的处理，放在通道内）
};

struct OrderInfoInEx;
typedef std::set<SerialNoType> MYSerialNoSet;
typedef std::map<SerialNoType, SerialNoType> MYSerialNoMap;
typedef std::set<OrderInfoInEx *> MYOrderSet;

// TODO 后续需要考虑修改成POD类型
struct OrderInfoInEx
{
    SerialNoType serial_no;
    StockCodeType stock_code;
    EntrustNoType entrust_no;
    char entrust_status;
    char direction;
    char open_close;
    char speculator;
    char order_kind;
    char order_type;
    VolumeType volume;
    double limit_price;
    VolumeType volume_remain; // 剩余量-没有成交的部分
    int error_no;

    // TODO: imprvoment
    char exchange;

    // 根据客户报单请求，生产报单对象
    OrderInfoInEx(const T_PlaceOrder *order)
    {
        serial_no = order->serial_no;
        memcpy(stock_code, order->stock_code, sizeof(StockCodeType));
        entrust_no = 0;
        entrust_status = BUSSINESS_DEF::ENTRUST_STATUS_REPORDED;
        direction = order->direction;
        open_close = order->open_close;
        speculator = order->speculator;
        order_kind = order->order_kind;
        order_type = order->order_type;
        volume = order->volume;
        limit_price = order->limit_price;
        volume_remain = volume;
        error_no = TUNNEL_ERR_CODE::RESULT_SUCCESS;
        exchange = order->exchange_type;
    }

    // 根据未成交量，创建补单
    OrderInfoInEx(OrderInfoInEx *original_order, SerialNoType new_serial_no, VolumeType rescue_vol)
    {
        serial_no = new_serial_no;
        memcpy(stock_code, original_order->stock_code, sizeof(StockCodeType));
        entrust_no = 0;
        entrust_status = BUSSINESS_DEF::ENTRUST_STATUS_REPORDED;
        direction = original_order->direction;
        open_close = original_order->open_close;
        speculator = original_order->speculator;
        order_kind = original_order->order_kind;
        order_type = original_order->order_type;
        volume = rescue_vol;
        limit_price = original_order->limit_price;
        volume_remain = volume;
        error_no = TUNNEL_ERR_CODE::RESULT_SUCCESS;
    }

    T_PlaceOrder ToPlaceOrder() const
    {
        T_PlaceOrder po;

        po.serial_no = serial_no;
        memcpy(po.stock_code, stock_code, sizeof(StockCodeType));
        po.limit_price = limit_price;
        po.direction = direction;
        po.open_close = open_close;
        po.speculator = speculator;
        po.volume = volume;
        po.order_kind = order_kind;
        po.order_type = order_type;

        // TODO: improvement
        po.exchange_type = exchange;

        return po;
    }

    T_CancelOrder ToCancelOrder(SerialNoType cancel_serial_no) const
        {
        T_CancelOrder co;

        co.serial_no = cancel_serial_no;		// 序列号
        memcpy(co.stock_code, stock_code, sizeof(StockCodeType));
        co.direction = direction;
        co.open_close = open_close;
        co.speculator = speculator;
        co.volume = volume;
        co.limit_price = limit_price;
        co.entrust_no = entrust_no;
        co.org_serial_no = serial_no;	// 待撤报单的序列号

        return co;
    }


    // status changed，回报
    inline T_OrderReturn CreateOrderReturn()
    {
        T_OrderReturn order_return;

        order_return.serial_no = serial_no;
        memcpy(order_return.stock_code, stock_code, sizeof(StockCodeType));
        order_return.entrust_no = entrust_no;
        order_return.entrust_status = entrust_status;
        order_return.direction = direction;
        order_return.open_close = open_close;
        order_return.speculator = speculator;
        order_return.volume = volume;           // 报单的原始手数
        order_return.limit_price = limit_price;
        order_return.volume_remain = volume_remain;   // 剩余未成交的手数

        return order_return;
    }

    // inner matched
    inline T_TradeReturn CreateTradeReturn(int matched_volume)
    {
        return CreateTradeReturn(matched_volume, this->limit_price, 0);
    }

    // create trade return message by matched infomation
    inline T_TradeReturn CreateTradeReturn(int matched_volume, double match_price, int business_no)
    {
        T_TradeReturn n_trade_rtn;
        n_trade_rtn.serial_no = serial_no;
        n_trade_rtn.entrust_no = entrust_no;
        n_trade_rtn.business_volume = matched_volume;
        n_trade_rtn.business_price = match_price;
        n_trade_rtn.business_no = business_no;
        memcpy(n_trade_rtn.stock_code, stock_code, sizeof(StockCodeType));
        n_trade_rtn.direction = direction;
        n_trade_rtn.open_close = open_close;

        return n_trade_rtn;
    }
};


struct PositionInfoInEX
{
    int long_position;
    int short_position;

    // position of yestoday
    int long_pos_yd;
    int short_pos_yd;
    double long_cost_yd;
    double short_cost_yd;

    // open volume and cost
    int long_open_vol;
    double long_open_cost;
    int short_open_vol;
    double short_open_cost;

    // close volume and cost
    int long_close_vol;
    double long_close_cost;
    int short_close_vol;
    double short_close_cost;

    // TODO: Improvement
   	int today_buy_volume; /* 今天的总买量 */
   	double today_aver_price_buy; /* 今天的买平均价格 */
   	int today_sell_volume; /* 今天的总卖量 */
   	double today_aver_price_sell; /* 今天卖平均价格 */

   	// TODO: imprvoment
   	char exchange;

    PositionInfoInEX()
    {
        long_position = 0;
        short_position = 0;

        long_pos_yd = 0;
        short_pos_yd = 0;
        long_cost_yd = 0;
        short_cost_yd = 0;

        long_open_vol = 0;
        long_open_cost = 0;
        short_open_vol = 0;
        short_open_cost = 0;

        long_close_vol = 0;
        long_close_cost = 0;
        short_close_vol = 0;
        short_close_cost = 0;

        today_buy_volume= 0;
        today_aver_price_buy = 0;
        today_sell_volume = 0;
        today_aver_price_sell = 0;
    }
};

typedef std::vector<OrderInfoInEx *> OrderCollection;
// use map for reserve sort
typedef std::map<SerialNoType, OrderInfoInEx*> OrderDatas;
typedef std::map<SerialNoType, T_CancelOrder> PendingCancelOrders;

// 不同合约的仓位（多账号时，需要按照合约和账户来统计仓位）
//typedef std::unordered_map<std::string, PositionInfoInEX> PositionDataOfCode;
typedef std::map<std::string, PositionInfoInEX> PositionDataOfCode;

namespace OrderUtil
{
static inline T_OrderRespond CreateOrderRespond(int error_no, SerialNoType serial_no, EntrustNoType entrust_no, char entrust_status)
{
    T_OrderRespond order_respond;

    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = entrust_no;
    order_respond.entrust_status = entrust_status;
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;

    return order_respond;
}

static inline T_CancelRespond CreateCancelRespond(int error_no, SerialNoType serial_no, EntrustNoType entrust_no)
{
    T_CancelRespond cancel_respond;

    memset(&cancel_respond, 0, sizeof(cancel_respond));
    cancel_respond.entrust_no = entrust_no;
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = BUSSINESS_DEF::ENTRUST_STATUS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = BUSSINESS_DEF::ENTRUST_STATUS_ERROR;

    return cancel_respond;
}
}
#endif  //
