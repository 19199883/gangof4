#pragma once

#include "SgitFtdcUserApiStruct.h"

#include "trade_data_type.h"
#include "sgit_trade_context.h"
#include "config_data.h"
#include "my_tunnel_lib.h"
#include "sgit_trade_context.h"

class StructConvertor
{
public:

    static void OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
        OrderRefDataType new_order_ref, CSgitFtdcInputOrderField &insert_order);

    static void OrderRespond(int error_no, SerialNoType serial_no, long entrust_no, T_OrderRespond &order_respond);

    static void CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req,
        OrderRefDataType order_ref, OrderRefDataType org_order_ref, const SgitOrderInfo * p_order,
        CSgitFtdcInputOrderActionField &cancle_order);

    static void CancelRespond(int error_no, SerialNoType serial_no, const char* entrust_no, T_CancelRespond &cancle_order);

    static void CancelRespond(int error_no, SerialNoType serial_no, long entrust_no, T_CancelRespond &cancle_order);

    static void OrderReturn(const CSgitFtdcOrderField *rsp, const SgitOrderInfo *p_req, T_OrderReturn &order_return);
    static void OrderReturn(const CSgitFtdcTradeField *rsp, const SgitOrderInfo *p_req, T_OrderReturn &order_return);

    static void TradeReturn(const CSgitFtdcTradeField *rsp, const SgitOrderInfo *p_req, T_TradeReturn &trade_return);
};
