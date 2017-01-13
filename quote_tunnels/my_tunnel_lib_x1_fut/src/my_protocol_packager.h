//  TODO: wangying x1 status: done

#ifndef MY_PROTOCOL_PACKAGER_H_
#define MY_PROTOCOL_PACKAGER_H_

#include "X1FtdcApiStruct.h"

#include "trade_data_type.h"
#include "my_tunnel_lib.h"
#include "x1_trade_context.h"

class X1Packer
{
public:
    // create request object
    static void OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
        OrderRefDataType new_order_ref, CX1FtdcInsertOrderField &insert_order);
    static void CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req,
        OrderRefDataType org_order_ref, CX1FtdcCancelOrderField &cancle_order);
    static void QuoteRequest(const TunnelConfigData &cfg, const T_InsertQuote *req,
        OrderRefDataType new_order_ref, CX1FtdcQuoteInsertField &insert_quote);
    static void CancelQuoteRequest(const TunnelConfigData &cfg, const T_CancelQuote *req,
        OrderRefDataType org_quote_ref, CX1FtdcCancelOrderField &cancle_order);

    // respond
    static void OrderRespond(int error_no, long serial_no, long entrust_no, char entrust_status, T_OrderRespond &rsp);
    static void CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond &crsp);
    static void OrderReturn(const CX1FtdcRspPriOrderField *rsp, const X1OrderInfo *p_req, T_OrderReturn &order_return);
    static void OrderReturn(const CX1FtdcRspPriMatchInfoField *rsp, const X1OrderInfo *p_req, T_OrderReturn &order_return);
    static void OrderReturn(const CX1FtdcRspPriCancelOrderField *rsp, const X1OrderInfo *p_req, T_OrderReturn &order_return);
    static void TradeReturn(const CX1FtdcRspPriMatchInfoField *rsp, const X1OrderInfo *p_req, T_TradeReturn &trade_return);

    // added for mm
    static void QuoteRespond(int error_no, long serial_no, long entrust_no, char entrust_status, T_InsertQuoteRespond &rsp);
    static void CancelQuoteRespond(int error_no, long serial_no, long entrust_no, T_CancelQuoteRespond &crsp);
    static void QuoteReturn(char dir, const char *instrument_id, const X1QuoteInfo *p_req, T_QuoteReturn &rtn);

};

#endif // MY_PROTOCOL_Packer_H_
