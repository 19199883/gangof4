#ifndef CTP_TRADE_CONTEXT_H_
#define CTP_TRADE_CONTEXT_H_

#include "ThostFtdcUserApiStruct.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#include "trade_data_type.h"

struct CTPQuoteInfo
{
    long serial_no;                 // 下单流水号
    int front_id;
    int session_id;
    mutable long entrust_no;
    mutable char status;
    std::string cancel_serial_no;
    std::string Cancel_serial_no() const
    {
        std::size_t pos = cancel_serial_no.find(",");
        if (pos != std::string::npos)
        {
            return cancel_serial_no.substr(0, pos);
        }

        return cancel_serial_no;
    }

    CTPQuoteInfo(long s_no, int f_id, int s_id)
        : serial_no(s_no),
            front_id(f_id),
            session_id(s_id),
            entrust_no(0),
            status(MY_TNL_OS_UNREPORT)
    {
    }
};

typedef std::unordered_map<OrderRefDataType, CTPQuoteInfo> QuoteInfoOfQuoteRef;

class CTPTradeContext
{
public:
    CTPTradeContext();

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

    void SaveOrderSysIDOfSerialNo(long sn, long id);
    long GetOrderSysIDOfSerialNo(long sn);

    bool CheckAndSetHaveHandleInsertErr(long sn);
    bool CheckAndSetHaveHandleActionErr(long sn);

    // added for mm
    OrderRefDataType GetNewOrderRefForInsertQuote();
    void SaveForquoteSerialNoOfOrderRef(OrderRefDataType order_ref, long sn);
    long GetForquoteSerialNoOfOrderRef(OrderRefDataType order_ref);

    void SaveQuoteInfo(OrderRefDataType quote_ref, const CTPQuoteInfo &quote_info);
    const CTPQuoteInfo * GetQuoteInfoByOrderRef(OrderRefDataType order_ref);
    void UpdateCancelInfoOfQuoteRef(OrderRefDataType order_ref, const std::string &c_s_no);
    OrderRefDataType GetQuoteRefBySerialNo(long serial_no);

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

    // added for mm
    std::mutex forquote_orderref_to_serialno_mutex_;
    OrderRefToSerialNoMap forquote_orderref_to_serialno_;

    std::mutex quote_mutex_;
    QuoteInfoOfQuoteRef quote_ref_to_quote_info_;
    SerialNoToOrderRefMap serialno_to_quote_ref_;
};

#endif // TRADE_CONTEXT_H_
