#include "hs_trade_context.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "config_data.h"
#include "tunnel_cmn_utility.h"
#include "my_tunnel_lib.h"

HSTradeContext::HSTradeContext()
{
}

void HSTradeContext::MapEntrustNo2ClOrdId(int rpt_no,int cl_ord_id)
{
	report_no_2_cl_ord_id_table_[rpt_no] = cl_ord_id;
}

T_PlaceOrder * HSTradeContext::GetOriginalRequestData(int cl_ord_id)
{
    boost::mutex::scoped_lock lock(order_mutex_);
    ClOrdIdOrdMapping::iterator cit = cl_ord_id_order_table_.find(cl_ord_id);
    if (cit != cl_ord_id_order_table_.end())
    {
        return &cit->second;
    }
    return NULL;
}

void HSTradeContext::SaveOriginalRequestData(const int &cl_ord_id, const T_PlaceOrder &req)
{
    boost::mutex::scoped_lock lock(order_mutex_);

    ClOrdIdOrdMapping::iterator it = cl_ord_id_order_table_.find(cl_ord_id);
    if (it == cl_ord_id_order_table_.end()){
    	cl_ord_id_order_table_.insert(std::make_pair(cl_ord_id, req));

    	T_InternalOrder int_ord;
    	int_ord.serial_no = req.serial_no;
    	strcpy(int_ord.stock_code,req.stock_code);
    	int_ord.limit_price = req.limit_price;
    	int_ord.direction = req.direction;
    	int_ord.volume = req.volume;
    	int_ord.order_kind = req.order_kind;
    	int_ord.order_type = req.order_type;
    	int_ord.exchange_type = req.exchange_type;
    	cl_ord_id_internal_order_table_.insert(std::make_pair(cl_ord_id, int_ord));
    }else{
        MY_LOG_WARN("duplicate cl_ord_id: %d", cl_ord_id);
    }

}


T_InternalOrder * HSTradeContext::GetInternalOrdByRptNo(const int rpt_no)
{
    boost::mutex::scoped_lock lock(order_mutex_);
    ClOrdIdReportNoMappingT::const_iterator cit = report_no_2_cl_ord_id_table_.find(rpt_no);
    if (cit != report_no_2_cl_ord_id_table_.end())
    {
        return &cl_ord_id_internal_order_table_[cit->second];
    }
    return NULL;
}

T_InternalOrder * HSTradeContext::GetInternalOrdByClOrdId(const int cl_ord_id)
{
     return &cl_ord_id_internal_order_table_[cl_ord_id];
}

void HSTradeContext::SaveCancelRequest(const int cl_ord_id, const T_CancelOrder &req)
{
    boost::mutex::scoped_lock lock(order_mutex_);

    ClCancelIdOrdMapping::iterator it = cl_cancel_id_order_table_.find(cl_ord_id);
    if (it == cl_cancel_id_order_table_.end()){
    	cl_cancel_id_order_table_.insert(std::make_pair(cl_ord_id, req));
    }else{
        MY_LOG_WARN("duplicate cl_ord_id: %d", cl_ord_id);
    }
}

T_CancelOrder * HSTradeContext::GetCancelRequestByRptNo(const int &rpt_no)
{
    boost::mutex::scoped_lock lock(order_mutex_);

    ClOrdIdReportNoMappingT::const_iterator cit = report_no_2_cl_ord_id_table_.find(rpt_no);
    if (cit != report_no_2_cl_ord_id_table_.end())
    {
        return &(cl_cancel_id_order_table_[cit->second]);
    }
    return NULL;
}
