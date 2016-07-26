#pragma once

#include "SecurityFtdcUserApiStruct.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#include "trade_data_type.h"

class LTSTradeContext
{
public:
    LTSTradeContext();

    // 报单引用和请求ID的维护
    void InitOrderRef(OrderRefDataType orderref_prefix);
    void SetOrderRef(OrderRefDataType cur_max_order_ref);
    OrderRefDataType GetNewOrderRef();
    int GetRequestID();

    // 报单引用和报单原始信息的映射
    void SaveSerialNoToOrderRef(OrderRefDataType order_ref, const OriginalReqInfo &order_info);
    const OriginalReqInfo * GetOrderInfoByOrderRef(OrderRefDataType order_ref);
    void UpdateCancelOrderRef(OrderRefDataType order_ref, const std::string &c_s_no);
    OrderRefDataType GetOrderRefBySerialNo(long serial_no);

    // 报单是否已应答
    void SetAnsweredPlaceOrder(OrderRefDataType order_ref);
    void SetAnsweredCancelOrder(OrderRefDataType order_ref);
    bool IsAnsweredPlaceOrder(OrderRefDataType order_ref);
    bool IsAnsweredCancelOrder(OrderRefDataType order_ref);

    bool SaveOrderSysIDOfSerialNo(long sn, long id);
    long GetOrderSysIDOfSerialNo(long sn);

    bool CheckAndSetHaveHandleInsertErr(long sn);
    bool CheckAndSetHaveHandleActionErr(long sn);

private:
    std::mutex sync_mutex_;
    OrderRefDataType cur_max_order_ref_;
    int cur_req_id_;

    // 报单引用到原始报单信息的映射表
    std::mutex ref_mutex_;
    OrderRefToRequestInfoMap orderref_to_req_;
    SerialNoToOrderRefMap serialno_to_orderref_;

    std::mutex ans_po_mutex_;
    AnsweredOrderRefSet answered_place_order_refs_;

    std::mutex ans_co_mutex_;
    AnsweredOrderRefSet answered_cancel_order_refs_;

    std::mutex ordersysid_mutex_;
    SerialNoToOrderSysIDMap serialno_to_ordersysid_;

    std::mutex err_handle_mutex_;
    SerialNoSet handled_err_insert_orders_;
    SerialNoSet handled_err_action_orders_;
};
