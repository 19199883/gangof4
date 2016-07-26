#pragma once

#include <pthread.h>
#include <strings.h>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <mutex>
#include "SgitFtdcUserApiStruct.h"
#include "my_tunnel_lib.h"
#include "trade_data_type.h"
#include "my_cmn_util_funcs.h"
#include "my_trade_tunnel_data_type.h"

struct SgitOrderInfo
{
    OrderRefDataType order_ref;
    T_PlaceOrder po;


    mutable char entrust_status;
    mutable VolumeType volume_remain;               // 剩余未成交的手数
    mutable TSgitFtdcOrderSysIDType sys_order_id;   // order id in exchange

    bool IsTerminated() const
    {
        return entrust_status == MY_TNL_OS_ERROR
            || entrust_status == MY_TNL_OS_COMPLETED;
    }

    SgitOrderInfo(OrderRefDataType ref, const T_PlaceOrder &po_)
        : order_ref(ref), po(po_)
    {
        sys_order_id[0] = '\0';
        entrust_status = MY_TNL_OS_UNREPORT;
        volume_remain = po.volume;
    }
};

typedef std::unordered_map<SerialNoType, const SgitOrderInfo *> SNToSgitOrderInfoMap;
typedef std::unordered_map<OrderRefDataType, const SgitOrderInfo *> OrderRefToSgitOrderInfoMap;

typedef std::unordered_map<OrderRefDataType, std::list<SerialNoType> > OrderRefToCancelSns;

class TradeContext
{
public:
    TradeContext();

    // 报单引用和请求ID的维护
    void InitOrderRef(OrderRefDataType orderref_prefix);
    void SetOrderRef(OrderRefDataType cur_max_order_ref);
    OrderRefDataType GetNewOrderRef();
    int GetRequestID();

    // sn/报单引用,和报单原始信息的映射
    void SaveOrderInfo(const SgitOrderInfo *order_info);
    const SgitOrderInfo * GetOrderInfoBySN(SerialNoType sn);
    const SgitOrderInfo * GetOrderInfoByOrderRef(OrderRefDataType order_ref);

    // cancel sn, and order ref
    void AddCancelSnForOrder(OrderRefDataType ref, SerialNoType cnl_sn);
    bool PopCancelSnOfOrder(OrderRefDataType ref, SerialNoType &cnl_sn);

private:
    std::mutex sync_lock_;
    OrderRefDataType cur_max_order_ref_;
    int cur_req_id_;

    std::mutex order_lock_;
    SNToSgitOrderInfoMap sn_to_order_;
    OrderRefToSgitOrderInfoMap orderref_to_order_;

    std::mutex cancel_lock_;
    OrderRefToCancelSns orderref_to_cancel_sns_;
};
