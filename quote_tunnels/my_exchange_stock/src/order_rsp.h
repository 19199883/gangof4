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

    void OrderRespondImp(const T_OrderRespond * order_rsp);
    void OrderReturnImp(bool from_myex, const T_OrderReturn * order_rtn);
    void TradeReturnImp(bool from_myex, const T_TradeReturn * trade_rtn);

private:
    // forbid copy
    MYOrderRsp(const MYOrderRsp &);
    MYOrderRsp & operator=(const MYOrderRsp &);

    bool cancel_first;

    void UpdateAndReport( bool from_order_return, int error_no,
        SerialNoType serial_no, EntrustNoType entrust_no, char entrust_status, VolumeType volume_remain,
        const T_PositionData *pos);

    void OrderRspProcess(bool from_order_return, int error_no,
        SerialNoType serial_no, EntrustNoType entrust_no, char entrust_status, char ex,VolumeType volume_remain = -1);

    // associated object
    MYExchange * p_my_exchange_;
    MYOrderDataManager *p_order_manager_;
    MYPositionDataManager *p_position_manager_;
    MYOrderReq *p_order_req_;
};

#endif
