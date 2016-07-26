#ifndef _MY_EXCHANGE_ORDER_RSP_H_
#define  _MY_EXCHANGE_ORDER_RSP_H_

#include "my_exchange_datatype_inner.h"
#include "my_exchange_utility.h"
#include <mutex>

class MYExchange;
class MYOrderDataManager;
class MYPositionDataManager;
class MYOrderReq;

class MYOrderRsp
{
public:
    MYOrderRsp(MYExchange * p_my_exchange, MYOrderDataManager *p_order_manager,
        MYPositionDataManager *p_position_manager, MYOrderReq *p_order_req)
        : p_my_exchange_(p_my_exchange),
            p_order_manager_(p_order_manager),
            p_position_manager_(p_position_manager),
            p_order_req_(p_order_req)
    {
        cancel_first = false;
    }

    ~MYOrderRsp()
    {
    }

    void OrderRespondImp(bool from_myex, const T_OrderRespond * order_rsp);
    void OrderReturnImp(bool from_myex, const T_OrderReturn * order_rtn);
    void TradeReturnImp(bool from_myex, const T_TradeReturn * trade_rtn);

    // added for support market making on 20141218
    void QuoteRespondImp(const T_InsertQuoteRespond * p);
    void QuoteReturnImp(const T_QuoteReturn * p);
    void QuoteTradeImp(const T_QuoteTrade * p);

private:
    // forbid copy
    MYOrderRsp(const MYOrderRsp &);
    MYOrderRsp & operator=(const MYOrderRsp &);

    bool cancel_first;

    void InnerMatch(SerialNoType new_matched_serial_no,
        SerialNoType canceled_serial_no,
        VolumeType canceled_volume);

    void UpdateAndReport(bool from_myex, bool from_order_return, int error_no,
        SerialNoType serial_no, EntrustNoType entrust_no, char entrust_status, VolumeType volume_remain);

    void OrderRspProcess(bool from_myex, bool from_order_return, int error_no,
        SerialNoType serial_no, EntrustNoType entrust_no, char entrust_status, VolumeType volume_remain = -1);

    void CheckAndRescueMatchedOrder(OrderInfoInEx* matched_order);

    void ResendTerminateStatusAfterTradeReturn(OrderInfoInEx* order, VolumeType volume_matched);

    // associated object
    MYExchange * p_my_exchange_;
    MYOrderDataManager *p_order_manager_;
    MYPositionDataManager *p_position_manager_;
    MYOrderReq *p_order_req_;

    // inner match concurrency control
    std::mutex mutex_inner_match_;
};

#endif
