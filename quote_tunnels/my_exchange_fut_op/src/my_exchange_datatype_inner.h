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
#include "my_trade_tunnel_struct.h"

enum SelfTradeResult
{
    HoldUp,			// 拦截 （报单量太小，挂单量太大时，拦截报单）
    NoSelfTrade,	// 无自成交，自行报单
    SelfTrade,		// 内部撮合，需等待撤单完成后报出余量
};

struct MYExchangeConfigData
{
    // 通道的配置（通道id对应通道配置文件）

    // MYExchange内部的配置

    // relation of Strategy id and tunnel id
    // 模型在多个账号间均衡下单，每个账号有最大下单数，可能需要拆单
    // 一个账号，可能对应多个物理通道（多个账号间进行均衡，可能不需要多个物理通道了；或者一个账号对应多个物理通道的处理，放在通道内）
};

struct OrderInfoInEx;
//typedef std::unordered_set<SerialNoType> MYSerialNoSet;
//typedef std::unordered_map<SerialNoType, SerialNoType> MYSerialNoMap;
//typedef std::unordered_set<OrderInfo *> MYOrderSet;
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
    char exchange_id;

    VolumeType volume;
    double limit_price;
    VolumeType volume_remain; // 剩余量-没有成交的部分
    int error_no;
    bool is_transform;  // 是否变换单
    bool wait_real_cancel; // in waiting cancel status
    bool need_rescue_canced_state;
    bool is_inner_matched;
    int following_matched_volume;

    bool is_prepare_for_inner_match;
    bool have_process_remain_flag;
    bool prev_is_fak_to_prepare_inner_match;
    double prev_price_of_sendto_market;

    // 报单，可能是一个报单的补单、开平反向单，需要记录上层报单号；0表示本身是原始报单；
    SerialNoType super_serial_no;

    // 变换单集合；（一个报单，可能对应一个开平反向的单，以及一个补量的单）
    MYOrderSet transform_orders;
    inline bool HasTransform()
    {
        return !transform_orders.empty();
    }

    // 内部撮合的报单集合；（一个报单，可能因自成交对应多个撤单，全部撤单回报后，才能尝试补单）
    MYSerialNoSet inner_matched_orders;
    inline bool InInnerMatching() const
    {
        return !inner_matched_orders.empty();
    }

    // 根据客户报单请求，生产报单对象
    OrderInfoInEx(const T_PlaceOrder *order)
    {
        serial_no = order->serial_no;
        memcpy(stock_code, order->stock_code, sizeof(StockCodeType));
        entrust_no = 0;
        entrust_status = MY_TNL_OS_REPORDED;
        direction = order->direction;
        open_close = order->open_close;
        speculator = order->speculator;
        order_kind = order->order_kind;
        order_type = order->order_type;
        exchange_id = order->exchange_type;

        volume = order->volume;
        limit_price = order->limit_price;
        volume_remain = volume;
        error_no = TUNNEL_ERR_CODE::RESULT_SUCCESS;
        is_transform = false;
        wait_real_cancel = false;
        need_rescue_canced_state = false;
        is_inner_matched = false;
        following_matched_volume = 0;

        is_prepare_for_inner_match = false;
        have_process_remain_flag = false;
        prev_is_fak_to_prepare_inner_match = false;
        prev_price_of_sendto_market = 0;

        // 用于关联其它报单的序列号
        super_serial_no = 0;
    }

    // 根据未成交量，创建补单
    OrderInfoInEx(OrderInfoInEx *original_order, SerialNoType new_serial_no, VolumeType rescue_vol, bool transform,
        bool remain_of_fak = false, double lp_fak = 0)
    {
        serial_no = new_serial_no;
        memcpy(stock_code, original_order->stock_code, sizeof(StockCodeType));
        entrust_no = 0;
        entrust_status = MY_TNL_OS_REPORDED;
        direction = original_order->direction;
        open_close = original_order->open_close;
        speculator = original_order->speculator;
        order_kind = original_order->order_kind;
        order_type = original_order->order_type;
        exchange_id = original_order->exchange_id;

        volume = rescue_vol;
        limit_price = original_order->limit_price;
        volume_remain = volume;
        error_no = TUNNEL_ERR_CODE::RESULT_SUCCESS;
        is_transform = transform;
        wait_real_cancel = false;
        need_rescue_canced_state = false;
        is_inner_matched = false;
        following_matched_volume = 0;
        if (is_transform)
        {
            if (open_close == MY_TNL_D_OPEN)
            {
                open_close = MY_TNL_D_CLOSE;
            }
            else
            {
                open_close = MY_TNL_D_OPEN;
            }
        }

        // 关联本报单的序列号
        super_serial_no = original_order->serial_no;

        is_prepare_for_inner_match = original_order->is_prepare_for_inner_match;
        have_process_remain_flag = false;
        prev_is_fak_to_prepare_inner_match = remain_of_fak;
        prev_price_of_sendto_market = lp_fak;

        // 源报单管理本补单
        original_order->transform_orders.insert(this);
    }

    void CreatePlaceOrder(T_PlaceOrder& po) const
    {
        po.serial_no = serial_no;
        memcpy(po.stock_code, stock_code, sizeof(StockCodeType));
        po.limit_price = limit_price;
        po.direction = direction;
        po.open_close = open_close;
        po.speculator = speculator;
        po.volume = volume;
        po.order_kind = order_kind;
        po.order_type = order_type;

        po.exchange_type = exchange_id;
    }

    void CreateCancelOrder(T_CancelOrder& co, SerialNoType cancel_serial_no) const
    {
        co.serial_no = cancel_serial_no;		// 序列号
        memcpy(co.stock_code, stock_code, sizeof(StockCodeType));
        co.direction = direction;
        co.open_close = open_close;
        co.speculator = speculator;
        co.volume = volume;
        co.limit_price = limit_price;
        co.entrust_no = entrust_no;
        co.org_serial_no = serial_no;	// 待撤报单的序列号
        co.exchange_type = exchange_id;
    }

//     inline void RecordMatchOneOrder(SerialNoType serial_no, VolumeType plan_match_volume)
//     {
//         inner_matched_orders.insert(serial_no);
//     }
//     inline void FinishMatchOneOrder(SerialNoType serial_no, VolumeType real_match_volume)
//     {
//         inner_matched_orders.erase(serial_no);
//     }
    inline bool AllMatched()
    {
        return inner_matched_orders.empty();
    }

    // status changed，回报
    inline void CreateOrderReturn(T_OrderReturn& order_return)
    {
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
        order_return.exchange_type = exchange_id;
    }

    // inner matched
    inline void CreateTradeReturn(T_TradeReturn& rtn, int matched_volume)
    {
        CreateTradeReturn(rtn, matched_volume, this->limit_price, 0);
    }

    // create trade return message by matched infomation
    inline void CreateTradeReturn(T_TradeReturn& n_trade_rtn, int matched_volume, double match_price, int business_no)
    {
        n_trade_rtn.serial_no = serial_no;
        n_trade_rtn.entrust_no = entrust_no;
        n_trade_rtn.business_volume = matched_volume;
        n_trade_rtn.business_price = match_price;
        n_trade_rtn.business_no = business_no;
        memcpy(n_trade_rtn.stock_code, stock_code, sizeof(StockCodeType));
        n_trade_rtn.direction = direction;
        n_trade_rtn.open_close = open_close;

        n_trade_rtn.exchange_type = exchange_id;
    }

    void UpdateInnerMatchedVolume(VolumeType business_volume)
    {
        volume_remain -= business_volume;
        if (volume_remain == 0)
        {
            entrust_status = MY_TNL_OS_COMPLETED;
        }
        else
        {
            entrust_status = MY_TNL_OS_PARTIALCOM;
        }
    }

    void SetInWaitingCancelStatus()
    {
        EX_LOG_DEBUG("SetInWaitingCancelStatus - serial_no=%ld", serial_no);
        wait_real_cancel = true;
    }
    void ClearInWaitingCancelStatus()
    {
        EX_LOG_DEBUG("ClearInWaitingCancelStatus - serial_no=%ld", serial_no);
        wait_real_cancel = false;
    }
    bool IsInWaitingCancelStatus()
    {
        return wait_real_cancel;
    }
};

struct SortBuyOrdersForInnerMatch
{
    // buy orders, higher price is better
    bool operator()(const OrderInfoInEx * r, const OrderInfoInEx * l)
    {
        if (r->limit_price < l->limit_price) return false;
        if (r->limit_price > l->limit_price) return true;

        // same price,newer is better
        return r->entrust_no > l->entrust_no;
    }
};

struct SortSellOrdersForInnerMatch
{
    // sell orders, lower price is better
    bool operator()(const OrderInfoInEx * r, const OrderInfoInEx * l)
    {
        if (r->limit_price > l->limit_price) return false;
        if (r->limit_price < l->limit_price) return true;

        // same price,newer is better
        return r->entrust_no > l->entrust_no;
    }
};

struct PositionInfoInEX
{
    int long_position;          // 多仓(平仓时已经预扣)
    int short_position;         // 空仓(平仓时已经预扣)
    int long_freeze;
    int short_freeze;

    int position_in_myex;   // 在MYExchange内部撮合的仓位（多空总是一致的，记录一个即可）

    // 变换成开仓的盘口量. 实时维护，以提高平仓控制处理性能
    int long_pending_volume_trans_to_open;
    int short_pending_volume_trans_to_open;
    // 开仓的盘口量. 实时维护，以提高开仓控制处理性能
    int long_pending_volume_of_open;
    int short_pending_volume_of_open;

    // position of yesterday
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

    // 当前多仓 = long_position + long_freeze
    // 当前多仓成本（不考虑合约乘数） = long_cost_yd + long_open_cost - short_close_cost
    // 空仓同上

    PositionInfoInEX()
    {
        long_position = 0;
        short_position = 0;
        long_freeze = 0;
        short_freeze = 0;
        position_in_myex = 0;

        long_pending_volume_trans_to_open = 0;
        short_pending_volume_trans_to_open = 0;
        long_pending_volume_of_open = 0;
        short_pending_volume_of_open = 0;

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
    }
};

// 报价对象(MYExchange内部)
struct QuoteInfoInEx
{
    T_InsertQuote data;
    EntrustNoType quote_entrust_no;
    EntrustNoType buy_entrust_no;
    EntrustNoType sell_entrust_no;
    char quote_status;
    char buy_status;
    char sell_status;
    VolumeType buy_remain;
    VolumeType sell_remain;

    QuoteInfoInEx(const T_InsertQuote *p)
        : data(*p), quote_entrust_no(0), buy_entrust_no(0), sell_entrust_no(0)
    {
        quote_status = MY_TNL_OS_UNREPORT;
        buy_status = MY_TNL_OS_UNREPORT;
        sell_status = MY_TNL_OS_UNREPORT;
        buy_remain = data.buy_volume;
        sell_remain = data.sell_volume;
    }

    T_QuoteReturn CreateQuoteRtn(char dir)
    {
        T_QuoteReturn quote_rtn;

        quote_rtn.serial_no = data.serial_no;
        memcpy(quote_rtn.stock_code, data.stock_code, sizeof(StockCodeType));
        quote_rtn.entrust_no = quote_entrust_no;
        quote_rtn.entrust_status = quote_status;
        quote_rtn.direction = dir;
        if (dir == MY_TNL_D_BUY)
        {
            quote_rtn.open_close = data.buy_open_close;
            quote_rtn.speculator = data.buy_speculator;
            quote_rtn.volume = data.buy_volume;           // 报单的原始手数
            quote_rtn.limit_price = data.buy_limit_price;
            quote_rtn.volume_remain = buy_remain;   // 剩余未成交的手数
        }
        else
        {
            quote_rtn.open_close = data.sell_open_close;
            quote_rtn.speculator = data.sell_speculator;
            quote_rtn.volume = data.sell_volume;           // 报单的原始手数
            quote_rtn.limit_price = data.sell_limit_price;
            quote_rtn.volume_remain = sell_remain;   // 剩余未成交的手数
        }

        return quote_rtn;
    }
};

typedef std::vector<OrderInfoInEx *> OrderCollection;
// use map for reserve sort
//typedef std::map<SerialNoType, OrderInfoInEx*> OrderDatas;
typedef std::map<SerialNoType, T_CancelOrder> PendingCancelOrders;

typedef std::map<SerialNoType, QuoteInfoInEx*> QuoteDatas;

// 不同合约的仓位（多账号时，需要按照合约和账户来统计仓位）
//typedef std::unordered_map<std::string, PositionInfoInEX> PositionDataOfCode;
typedef std::map<std::string, PositionInfoInEX> PositionDataOfCode;

namespace OrderUtil
{
    inline static void CreateOrderRespond(T_OrderRespond& order_respond,
                                   int error_no,
                                   SerialNoType serial_no,
                                   EntrustNoType entrust_no,
                                   char entrust_status)
    {
        memset(&order_respond, 0, sizeof(order_respond));
        order_respond.entrust_no = entrust_no;
        order_respond.entrust_status = entrust_status;
        order_respond.serial_no = serial_no;
        order_respond.error_no = error_no;
    }

    inline static void CreateCancelRespond(T_CancelRespond& cancel_respond,
                                    int error_no,
                                    SerialNoType serial_no,
                                    EntrustNoType entrust_no)
    {
        memset(&cancel_respond, 0, sizeof(cancel_respond));
        cancel_respond.entrust_no = entrust_no;
        cancel_respond.serial_no = serial_no;
        cancel_respond.error_no = error_no;

        // 需要回报撤单状态，成功为已报，失败为拒绝
        cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
        if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
    }

    inline static void CreateInsertQuoteRsp(T_InsertQuoteRespond& rsp,
                                     int error_no,
                                     long serial_no,
                                     long entrust_no,
                                     char entrust_status)
    {
        memset(&rsp, 0, sizeof(rsp));
        rsp.entrust_no = entrust_no;
        rsp.entrust_status = entrust_status;
        rsp.serial_no = serial_no;
        rsp.error_no = error_no;
    }

    inline static void CreateCancelQuoteRespond(T_CancelQuoteRespond& cancel_respond, int error_no, long serial_no, long entrust_no, char entrust_status)
    {
        memset(&cancel_respond, 0, sizeof(cancel_respond));
        cancel_respond.entrust_no = entrust_no;
        cancel_respond.entrust_status = entrust_status;
        cancel_respond.serial_no = serial_no;
        cancel_respond.error_no = error_no;
    }
}
#endif  //
