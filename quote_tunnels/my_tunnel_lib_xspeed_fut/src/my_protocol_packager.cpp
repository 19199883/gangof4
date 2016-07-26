#include "my_protocol_packager.h"

#include <sstream>
#include <stdlib.h>

#include "field_convert.h"
#include "config_data.h"
#include "xspeed_trade_context.h"

void XSpeedPacker::OrderRequest(const TunnelConfigData& cfg, const T_PlaceOrder* req, OrderRefDataType new_order_ref,
    DFITCInsertOrderField& insert_order)
{
    memset(&insert_order, 0, sizeof(DFITCInsertOrderField));
    strncpy(insert_order.accountID, cfg.Logon_config().clientid.c_str(), sizeof(DFITCAccountIDType));
    insert_order.localOrderID = new_order_ref;
    strncpy(insert_order.instrumentID, req->stock_code, sizeof(DFITCInstrumentIDType));
    insert_order.insertPrice = req->limit_price;
    insert_order.orderAmount = req->volume;
    insert_order.buySellType = XSpeedFieldConvert::GetXSpeedBuySell(req->direction);
    insert_order.openCloseType = XSpeedFieldConvert::GetXSpeedOCFlag(req->open_close);
    insert_order.speculator = XSpeedFieldConvert::GetXSpeedHedgeType(req->speculator);
    insert_order.orderType = XSpeedFieldConvert::GetXSpeedPriceType(req->order_kind);
    insert_order.orderProperty = XSpeedFieldConvert::GetXSpeedOrderProperty(req->order_type);

    insert_order.insertType = DFITC_BASIC_ORDER;        //委托类别,默认为普通订单

    insert_order.instrumentType = DFITC_OPT_TYPE;      //合约类型, 可选值：期货、期权，默认期权
    if (strlen(insert_order.instrumentID) < 10) insert_order.instrumentType = DFITC_COMM_TYPE;
}

void XSpeedPacker::CancelRequest(const TunnelConfigData& cfg, const T_CancelOrder* req,
    OrderRefDataType org_order_ref, DFITCCancelOrderField& cancle_order)
{
    memset(&cancle_order, 0, sizeof(DFITCCancelOrderField));
    strncpy(cancle_order.accountID, cfg.Logon_config().clientid.c_str(), sizeof(DFITCAccountIDType));
    strncpy(cancle_order.instrumentID, req->stock_code, sizeof(DFITCInstrumentIDType));
    cancle_order.localOrderID = org_order_ref;
    //cancle_order.spdOrderID = req->entrust_no;
    cancle_order.spdOrderID = -1;
}

void XSpeedPacker::QuoteRequest(const TunnelConfigData& cfg, const T_InsertQuote* p, OrderRefDataType new_order_ref,
    DFITCQuoteInsertField& insert_quote_req)
{
    memset(&insert_quote_req, 0, sizeof(insert_quote_req));

    strncpy(insert_quote_req.accountID, cfg.Logon_config().clientid.c_str(), sizeof(insert_quote_req.accountID));
    strncpy(insert_quote_req.instrumentID, p->stock_code, sizeof(insert_quote_req.instrumentID));
    strncpy(insert_quote_req.quoteID, p->for_quote_id, sizeof(insert_quote_req.quoteID));
    insert_quote_req.localOrderID = new_order_ref;
    insert_quote_req.bOpenCloseType = XSpeedFieldConvert::GetXSpeedOCFlag(p->buy_open_close);
    insert_quote_req.bSpeculator = XSpeedFieldConvert::GetXSpeedHedgeType(p->buy_speculator);
    insert_quote_req.bInsertPrice = p->buy_limit_price;
    insert_quote_req.bOrderAmount = p->buy_volume;
    insert_quote_req.sOpenCloseType = XSpeedFieldConvert::GetXSpeedOCFlag(p->sell_open_close);
    insert_quote_req.sSpeculator = XSpeedFieldConvert::GetXSpeedHedgeType(p->sell_speculator);
    insert_quote_req.sInsertPrice = p->sell_limit_price;
    insert_quote_req.sOrderAmount = p->sell_volume;

    insert_quote_req.instrumentType = DFITC_OPT_TYPE;      //合约类型, 可选值：期货、期权，默认期权
    if (strlen(insert_quote_req.instrumentID) < 10) insert_quote_req.instrumentType = DFITC_COMM_TYPE;
}

void XSpeedPacker::CancelQuoteRequest(const TunnelConfigData& cfg, const T_CancelQuote* req,
    OrderRefDataType org_quote_ref, DFITCCancelOrderField& cancle_order)
{
    memset(&cancle_order, 0, sizeof(DFITCCancelOrderField));
    strncpy(cancle_order.accountID, cfg.Logon_config().clientid.c_str(), sizeof(DFITCAccountIDType));
    strncpy(cancle_order.instrumentID, req->stock_code, sizeof(DFITCInstrumentIDType));
    cancle_order.localOrderID = org_quote_ref;
    //cancle_order.spdOrderID = req->entrust_no;
    cancle_order.spdOrderID = -1;
}

void XSpeedPacker::OrderRespond(int error_no, long serial_no, long entrust_no, char entrust_status, T_OrderRespond& rsp)
{
    memset(&rsp, 0, sizeof(T_OrderRespond));
    rsp.serial_no = serial_no;
    rsp.error_no = error_no;
    rsp.entrust_no = entrust_no;
    rsp.entrust_status = entrust_status;
}

void XSpeedPacker::CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond& cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(T_CancelRespond));
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;
    cancel_respond.entrust_no = entrust_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void XSpeedPacker::OrderReturn(const DFITCOrderRtnField* rsp, const XSpeedOrderInfo* p_req, T_OrderReturn& order_return)
{
    memset(&order_return, 0, sizeof(order_return));

    order_return.serial_no = p_req->serial_no;
    memcpy(order_return.stock_code, p_req->stock_code, sizeof(order_return.stock_code));
    order_return.entrust_no = p_req->entrust_no;
    order_return.entrust_status = p_req->entrust_status;
    order_return.direction = p_req->direction;
    order_return.open_close = p_req->open_close;
    order_return.speculator = p_req->speculator;
    order_return.volume = p_req->volume;
    order_return.limit_price = p_req->limit_price;
    order_return.volume_remain = p_req->volume_remain;
}

void XSpeedPacker::OrderReturn(const DFITCMatchRtnField* rsp, const XSpeedOrderInfo* p_req, T_OrderReturn& order_return)
{
    memset(&order_return, 0, sizeof(order_return));

    order_return.serial_no = p_req->serial_no;
    memcpy(order_return.stock_code, p_req->stock_code, sizeof(order_return.stock_code));
    order_return.entrust_no = p_req->entrust_no;
    order_return.entrust_status = p_req->entrust_status;
    order_return.direction = p_req->direction;
    order_return.open_close = p_req->open_close;
    order_return.speculator = p_req->speculator;
    order_return.volume = p_req->volume;
    order_return.limit_price = p_req->limit_price;
    order_return.volume_remain = p_req->volume_remain;
}

void XSpeedPacker::OrderReturn(const DFITCOrderCanceledRtnField* rsp, const XSpeedOrderInfo* p_req, T_OrderReturn& order_return)
{
    memset(&order_return, 0, sizeof(order_return));

    order_return.serial_no = p_req->serial_no;
    memcpy(order_return.stock_code, p_req->stock_code, sizeof(order_return.stock_code));
    order_return.entrust_no = p_req->entrust_no;
    order_return.entrust_status = p_req->entrust_status;
    order_return.direction = p_req->direction;
    order_return.open_close = p_req->open_close;
    order_return.speculator = p_req->speculator;
    order_return.volume = p_req->volume;
    order_return.limit_price = p_req->limit_price;
    order_return.volume_remain = p_req->volume_remain;
}

void XSpeedPacker::TradeReturn(const DFITCMatchRtnField* rsp, const XSpeedOrderInfo* p_req, T_TradeReturn& trade_return)
{
    memset(&trade_return, 0, sizeof(trade_return));

    trade_return.serial_no = p_req->serial_no;
    trade_return.entrust_no = p_req->entrust_no;
    trade_return.business_volume = rsp->matchedAmount;
    trade_return.business_price = rsp->matchedPrice;
    trade_return.business_no = atoi(rsp->matchID);

    memcpy(trade_return.stock_code, p_req->stock_code, sizeof(trade_return.stock_code));
    trade_return.direction = p_req->direction;
    trade_return.open_close = p_req->open_close;
}

void XSpeedPacker::QuoteRespond(int error_no, long serial_no, long entrust_no, char entrust_status, T_InsertQuoteRespond& rsp)
{
    memset(&rsp, 0, sizeof(rsp));
    rsp.serial_no = serial_no;
    rsp.error_no = error_no;
    rsp.entrust_no = entrust_no;
    rsp.entrust_status = entrust_status;
}

void XSpeedPacker::CancelQuoteRespond(int error_no, long serial_no, long entrust_no, T_CancelQuoteRespond& crsp)
{
    memset(&crsp, 0, sizeof(crsp));
    crsp.serial_no = serial_no;
    crsp.error_no = error_no;
    crsp.entrust_no = entrust_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    crsp.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) crsp.entrust_status = MY_TNL_OS_ERROR;
}

void XSpeedPacker::QuoteReturn(char dir, const char *instrument_id, const XSpeedQuoteInfo *p_req, T_QuoteReturn &rtn)
{
    memset(&rtn, 0, sizeof(rtn));

    rtn.entrust_no = p_req->entrust_no;
    memcpy(rtn.stock_code, instrument_id, sizeof(rtn.stock_code));
    rtn.serial_no = p_req->serial_no;
    rtn.direction = dir;

    if (dir == MY_TNL_D_BUY)
    {
        rtn.entrust_status = p_req->buy_entrust_status;
        rtn.open_close = p_req->buy_open_close;
        rtn.speculator = p_req->buy_speculator;
        rtn.volume = p_req->buy_volume;
        rtn.limit_price = p_req->buy_limit_price;
        rtn.volume_remain = p_req->buy_volume_remain;
    }
    else
    {
        rtn.entrust_status = p_req->sell_entrust_status;
        rtn.open_close = p_req->sell_open_close;
        rtn.speculator = p_req->sell_speculator;
        rtn.volume = p_req->sell_volume;
        rtn.limit_price = p_req->sell_limit_price;
        rtn.volume_remain = p_req->sell_volume_remain;
    }
}

