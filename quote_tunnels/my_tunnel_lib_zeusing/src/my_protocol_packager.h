#pragma once

#include "ZeusingFtdcUserApiStruct.h"

#include "trade_data_type.h"
#include "zeusing_trade_context.h"
#include "my_trade_tunnel_struct.h"
#include "config_data.h"

class ApiPacker
{
public:

	static void OrderRequest(const TunnelConfigData &cfg,const T_PlaceOrder *req,
        OrderRefDataType new_order_ref, CZeusingFtdcInputOrderField &insert_order);

	static void OrderRespond(const int error_no, const long serial_no, const char* entrust_no, const char OrderSubmitStatus,
                                        const char entrust_status, T_OrderRespond &order_respond);

	static void OrderRespond(const int error_no, const long serial_no, const long entrust_no,
                                        const short entrust_status, T_OrderRespond &order_respond);

	static void CancelRequest(const TunnelConfigData &cfg,const T_CancelOrder *req,
        OrderRefDataType order_ref, OrderRefDataType org_order_ref,
                            const OriginalReqInfo *org_order_info, CZeusingFtdcInputOrderActionField &cancle_order);

	static void CancelRespond(int error_no, long serial_no, const char* entrust_no, T_CancelRespond &cancle_order);

 	static void CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond &cancle_order);

	static void OrderReturn(const CZeusingFtdcOrderField *rsp, const OriginalReqInfo *p_req, T_OrderReturn &order_return);

	static void TradeReturn(const CZeusingFtdcTradeField *rsp, const OriginalReqInfo *p_req, T_TradeReturn &trade_return);
};
