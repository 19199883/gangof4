#ifndef HS_TRADE_CONTEXT_H_
#define HS_TRADE_CONTEXT_H_

#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
#include <boost/thread.hpp>

#include "my_trade_tunnel_api.h"
#include "trade_data_type.h"
#include "my_tunnel_lib.h"


typedef std::map<int, T_PlaceOrder> ClOrdIdOrdMapping;
typedef std::map<int, T_InternalOrder> ClOrdIdInternalOrdMapping;
typedef std::map<int, T_CancelOrder> ClCancelIdOrdMapping;
typedef std::map<int, int> ClOrdIdReportNoMappingT;


class HSTradeContext
{
public:
    HSTradeContext();
    void SaveOriginalRequestData(const int &cl_ord_id, const T_PlaceOrder &req);
    T_PlaceOrder *GetOriginalRequestData(int cl_ord_id);
    T_InternalOrder * GetInternalOrdByRptNo(const int rpt_no);
    T_InternalOrder * GetInternalOrdByClOrdId(const int cl_ord_id);

    void SaveCancelRequest(const int cl_ord_id, const T_CancelOrder &req);
	T_CancelOrder *GetCancelRequest(const int &cl_ord_id);

	T_CancelOrder *GetCancelRequestByRptNo(const int &rpt_no);

    void MapEntrustNo2ClOrdId(int rpt_no,int cl_ord_id);

private:
    // 报单引用到原始报单信息的映射表
    boost::mutex order_mutex_;
    ClOrdIdOrdMapping cl_ord_id_order_table_;
    ClOrdIdInternalOrdMapping cl_ord_id_internal_order_table_;
    ClCancelIdOrdMapping cl_cancel_id_order_table_;
    ClOrdIdReportNoMappingT report_no_2_cl_ord_id_table_;

};

#endif
