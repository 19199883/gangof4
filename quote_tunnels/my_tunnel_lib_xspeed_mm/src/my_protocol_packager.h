#pragma once

#include "DFITCApiStruct.h"

#include "trade_data_type.h"
#include "my_tunnel_lib.h"
#include "xspeed_trade_context.h"

class XSpeedPacker
{
public:
    // create request object
    static void OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
        OrderRefDataType new_order_ref, DFITCInsertOrderField &insert_order);
    static void CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req,
        OrderRefDataType org_order_ref, DFITCCancelOrderField &cancle_order);
    static void QuoteRequest(const TunnelConfigData &cfg, const T_InsertQuote *req,
        OrderRefDataType new_order_ref, DFITCQuoteInsertField &insert_quote);
    static void CancelQuoteRequest(const TunnelConfigData &cfg, const T_CancelQuote *req,
        OrderRefDataType org_quote_ref, DFITCCancelOrderField &cancle_order);

    // respond
    static void OrderRespond(int error_no, long serial_no, long entrust_no, char entrust_status, T_OrderRespond &rsp);
    static void CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond &crsp);
    static void OrderReturn(const DFITCOrderRtnField *rsp, const XSpeedOrderInfo *p_req, T_OrderReturn &order_return);
    static void OrderReturn(const DFITCMatchRtnField *rsp, const XSpeedOrderInfo *p_req, T_OrderReturn &order_return);
    static void OrderReturn(const DFITCOrderCanceledRtnField *rsp, const XSpeedOrderInfo *p_req, T_OrderReturn &order_return);
    static void TradeReturn(const DFITCMatchRtnField *rsp, const XSpeedOrderInfo *p_req, T_TradeReturn &trade_return);

    // added for mm
    static void QuoteRespond(int error_no, long serial_no, long entrust_no, char entrust_status, T_InsertQuoteRespond &rsp);
    static void CancelQuoteRespond(int error_no, long serial_no, long entrust_no, T_CancelQuoteRespond &crsp);
    static void QuoteReturn(char dir, const char *instrument_id, const XSpeedQuoteInfo *p_req, T_QuoteReturn &rtn);
    static void QuoteTrade(char dir, const DFITCQuoteMatchRtnField *rsp, const XSpeedQuoteInfo *p_req, T_QuoteTrade &quote_trade);
};
