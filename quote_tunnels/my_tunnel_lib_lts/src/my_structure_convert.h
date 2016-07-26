#pragma once

#include "SecurityFtdcTraderApi.h"

#include "trade_data_type.h"
#include "lts_trade_context.h"
#include "my_trade_tunnel_struct.h"
#include "config_data.h"

class LTSPacker
{
public:

    static void OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
        OrderRefDataType new_order_ref, CSecurityFtdcInputOrderField &insert_order);

    static void OrderRespond(int error_no, long serial_no, long entrust_no, T_OrderRespond &order_respond);

    static void OrderRespond(int error_no, long serial_no, long entrust_no, char OrderSubmitStatus,
        char entrust_status, T_OrderRespond &order_respond);

    static void CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req,
        OrderRefDataType order_ref, OrderRefDataType org_order_ref,
        const OriginalReqInfo *org_order_info, CSecurityFtdcInputOrderActionField &cancle_order);

    static void CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond &cancle_order);

    static void OrderReturn(const CSecurityFtdcOrderField *rsp, const OriginalReqInfo *p_req, T_OrderReturn &order_return);

    static void TradeReturn(const CSecurityFtdcTradeField *rsp, const OriginalReqInfo *p_req, T_TradeReturn &trade_return);
};
