#ifndef XELE_TRADE_CONTEXT_H_
#define XELE_TRADE_CONTEXT_H_

#include "CXeleFtdcUserApiStruct.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#include "trade_data_type.h"
#include "my_tunnel_lib.h"

struct XeleOriginalReqInfo
{
    long serial_no;                 // 下单流水号
    char exchange_code;             // 交易所代码
    char buy_sell_flag;             // 买卖标志
    char hedge_type;                // 投机套保标识
    char open_close_flag;           // 开平标识
    char order_type;                // 报单类型；0：普通；1：FAK
    int order_volum;                // 委托数量
    char order_Kind;                // 委托类型
    std::string stock_code;         // 合约代码
    OrderRefDataType original_order_ref;   // 下单 local id

    XeleOriginalReqInfo(const long s_no,
        char exch_code, char buy_sell_flg, char hedge_flag, char oc_flag, char order_t,
        int o_vlm, char o_kind, const std::string &s_code, OrderRefDataType po_ref = 0)
        : serial_no(s_no),
            exchange_code(exch_code),
            buy_sell_flag(buy_sell_flg),
            hedge_type(hedge_flag),
            open_close_flag(oc_flag),
            order_type(order_t),
            order_volum(o_vlm), order_Kind(o_kind), stock_code(s_code), original_order_ref(po_ref)
    {
    }
};
typedef std::unordered_map<OrderRefDataType, XeleOriginalReqInfo> XeleOrderRefToRequestInfoMap;
typedef XeleOrderRefToRequestInfoMap::iterator XeleOrderRefToRequestInfoMapIt;
typedef XeleOrderRefToRequestInfoMap::const_iterator XeleOrderRefToRequestInfoMapCit;

// 报单引用和请求ID的维护
class XeleTradeContext
{
public:
    XeleTradeContext();

    // 报单引用和请求ID的维护
    void InitOrderRef(OrderRefDataType orderref_prefix);
    void SetOrderRef(OrderRefDataType cur_max_order_ref);
    OrderRefDataType GetNewOrderRef();
    int GetRequestID();

    // 报单引用和报单原始信息的映射
    void SaveSerialNoToOrderRef(OrderRefDataType order_ref, const XeleOriginalReqInfo &order_info);
    const XeleOriginalReqInfo * GetOrderInfoByOrderRef(OrderRefDataType order_ref);
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
    XeleOrderRefToRequestInfoMap orderref_to_req_;
    SerialNoToOrderRefMap serialno_to_orderref_;

    // order status
    std::mutex status_mutex_;
    OrderRefToOrderStatusMap orderref_to_status_;

    std::mutex err_handle_mutex_;
    SerialNoSet handled_err_insert_orders_;
    SerialNoSet handled_err_action_orders_;

    std::mutex fak_mutex_;
    std::unordered_set<OrderRefDataType> finished_order_ref_;

    // order sys id collection, for identify whether a order is placed by the tunnel
    std::mutex sysid_mutex_;
    std::unordered_set<EntrustNoType> order_sys_ids_;
};

inline void XeleTradeContext::AddNewOrderSysID(EntrustNoType sysid)
{
    std::lock_guard<std::mutex> lock(sysid_mutex_);
    order_sys_ids_.insert(sysid);
}

inline bool XeleTradeContext::IsSysIDOfThisTunnel(EntrustNoType sysid)
{
    std::lock_guard<std::mutex> lock(sysid_mutex_);
    return order_sys_ids_.find(sysid) != order_sys_ids_.end();
}

#endif // TRADE_CONTEXT_H_