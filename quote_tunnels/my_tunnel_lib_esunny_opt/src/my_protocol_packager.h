#ifndef MY_PROTOCOL_PACKAGER_H_
#define MY_PROTOCOL_PACKAGER_H_

#include "TapTradeAPIDataType.h"

#include "trade_data_type.h"
#include "my_tunnel_lib.h"
#include "esunny_trade_context.h"

class ESUNNYPacker
{
public:

    static bool OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req, TapAPINewOrder &new_or);

    static bool QuoteRequest(const TunnelConfigData &cfg, const T_ReqForQuote *req, TapAPINewOrder &quote_or);

    static bool QuoteInsertRequest(const TunnelConfigData &cfg, const T_InsertQuote *req, TapAPINewOrder &quote_or);

    static void OrderRespond(int error_no, long serial_no, long entrust_no, short entrust_status, T_OrderRespond &order_respond);

    static void CancelRequest(const EsunnyOrderInfo *org_order_info, TapAPIOrderCancelReq &co);

    static void CancelRequest(const EsunnyQuoteInfo *org_quote_info, TapAPIOrderCancelReq &co);

    static void CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond &cancle_order);

    static void OrderReturn(const TapAPIOrderInfo *rsp, const EsunnyOrderInfo *p_req, T_OrderReturn &order_return);

    static void TradeReturn(const TapAPIFillInfo *rsp, const EsunnyOrderInfo *p_req, T_TradeReturn &trade_return);

    static  void QuoteRequestRespond(const T_ReqForQuote *rsp, int error_no,  T_RspOfReqForQuote &quote_respond);

    static void QuoteInsertRespond(const T_InsertQuote *rsp, int error_no, T_InsertQuoteRespond &quote_respond);

    static void QuoteActionRespond(const T_CancelQuote *rsp, int error_no, T_CancelQuoteRespond &quote_respond);

    static void QuoteInsertReturn(const TapAPIOrderInfo *rsp, const EsunnyQuoteInfo *p_req, T_QuoteReturn &quote_return, TMyTnlDirectionType direction);

    static void QuoteInsertTrade(const TapAPIFillInfo *rsp, const EsunnyQuoteInfo *p_req, T_QuoteTrade &quote_trade);

    static void ForQuoteRespond(TunnelConfigData &cfg, long serial_no, const TapAPIOrderInfo *rsp, T_RspOfReqForQuote &for_quote_respond);

    static void ForQuoteReturn(const TapAPIReqQuoteNotice *rsp, T_RtnForQuote &for_quote_return);
};

#endif // MY_PROTOCOL_Packer_H_
