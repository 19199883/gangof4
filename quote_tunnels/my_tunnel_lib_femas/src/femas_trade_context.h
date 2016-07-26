#ifndef FEMAS_TRADE_CONTEXT_H_
#define FEMAS_TRADE_CONTEXT_H_

#include "USTPFtdcUserApiStruct.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "trade_data_type.h"

// 报单引用和请求ID的维护
class FEMASTradeContext
{
public:
    FEMASTradeContext();

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

    // 报单status
    void SaveOrderRefToStatus(OrderRefDataType order_ref, const OrderStatusInTunnel &status);
    StatusCheckResult CheckAndUpdateStatus(OrderRefDataType order_ref, FemasRespondType rt,
        char new_status, int new_matched_volume);

    bool CheckAndSetHaveHandleInsertErr(long sn);
    bool CheckAndSetHaveHandleActionErr(long sn);

    // fak
    void SetFinishedOrderOfFAK(OrderRefDataType r);
    bool IsFinishedOrderOfFAK(OrderRefDataType r);

    // order sys id
    inline void AddNewOrderSysID(EntrustNoType sysid);
    inline bool IsSysIDOfThisTunnel(EntrustNoType sysid);

private:
    // order ref and request id dispatch
    std::mutex sync_mutex_;
    OrderRefDataType cur_max_order_ref_;
    int cur_req_id_;

    // 报单引用到原始报单信息的映射表
    std::mutex ref_mutex_;
    OrderRefToRequestInfoMap orderref_to_req_;
    SerialNoToOrderRefMap serialno_to_orderref_;

    // order status
    std::mutex status_mutex_;
    OrderRefToOrderStatusMap orderref_to_status_;

    std::mutex err_handle_mutex_;
    SerialNoSet handled_err_insert_orders_;
    SerialNoSet handled_err_action_orders_;

    std::mutex fak_mutex_;
    std::set<OrderRefDataType> finished_order_ref_;

    // order sys id collection, for identify whether a order is placed by the tunnel
    std::mutex sysid_mutex_;
    std::set<EntrustNoType> order_sys_ids_;
};

inline void FEMASTradeContext::AddNewOrderSysID(EntrustNoType sysid)
{
    std::lock_guard<std::mutex> lock(sysid_mutex_);
    order_sys_ids_.insert(sysid);
}

inline bool FEMASTradeContext::IsSysIDOfThisTunnel(EntrustNoType sysid)
{
    std::lock_guard<std::mutex> lock(sysid_mutex_);
    return order_sys_ids_.find(sysid) != order_sys_ids_.end();
}

#endif // TRADE_CONTEXT_H_