//  TODO: wangying x1 status: done
//
#include "my_protocol_packager.h"

#include <sstream>
#include <stdlib.h>

#include "field_convert.h"
#include "config_data.h"
#include "x1_trade_context.h"

void X1Packer::OrderRequest(const TunnelConfigData& cfg, const T_PlaceOrder* req, OrderRefDataType new_order_ref,
    CX1FtdcInsertOrderField& insert_order)
{
    memset(&insert_order, 0, sizeof(CX1FtdcInsertOrderField));
    strncpy(insert_order.AccountID, cfg.Logon_config().clientid.c_str(), sizeof(TX1FtdcAccountIDType));
    insert_order.LocalOrderID = new_order_ref;
    strncpy(insert_order.InstrumentID, req->stock_code, sizeof(TX1FtdcInstrumentIDType));
    insert_order.InsertPrice = req->limit_price;
    insert_order.OrderAmount = req->volume;
    insert_order.BuySellType = X1FieldConvert::GetX1BuySell(req->direction);
    insert_order.OpenCloseType = X1FieldConvert::GetX1OCFlag(req->open_close);
    insert_order.Speculator = X1FieldConvert::GetX1HedgeType(req->speculator);
    insert_order.OrderType = X1FieldConvert::GetX1PriceType(req->order_kind);
    insert_order.OrderProperty = X1FieldConvert::GetX1OrderProperty(req->order_type);

    insert_order.InsertType = X1_FTDC_BASIC_ORDER;        //委托类别,默认为普通订单

    insert_order.InstrumentType = X1FTDC_INSTRUMENT_TYPE_OPT;      //合约类型, 可选值：期货、期权，默认期权
    if (strlen(insert_order.InstrumentID) < 10) insert_order.InstrumentType = X1FTDC_INSTRUMENT_TYPE_COMM;
}

void X1Packer::CancelRequest(const TunnelConfigData& cfg, const T_CancelOrder* req,
    OrderRefDataType org_order_ref, CX1FtdcCancelOrderField& cancle_order)
{
    memset(&cancle_order, 0, sizeof(CX1FtdcCancelOrderField));
    strncpy(cancle_order.AccountID, cfg.Logon_config().clientid.c_str(), sizeof(TX1FtdcAccountIDType));
    strncpy(cancle_order.InstrumentID, req->stock_code, sizeof(TX1FtdcInstrumentIDType));
    cancle_order.LocalOrderID = org_order_ref;
    //cancle_order.spdOrderID = req->entrust_no;
    cancle_order.X1OrderID = -1;
}

void X1Packer::QuoteRequest(const TunnelConfigData& cfg, const T_InsertQuote* p, OrderRefDataType new_order_ref,
    CX1FtdcQuoteInsertField& insert_quote_req)
{
    memset(&insert_quote_req, 0, sizeof(insert_quote_req));

    strncpy(insert_quote_req.AccountID, cfg.Logon_config().clientid.c_str(), sizeof(insert_quote_req.AccountID));
    strncpy(insert_quote_req.InstrumentID, p->stock_code, sizeof(insert_quote_req.InstrumentID));
    strncpy(insert_quote_req.QuoteID, p->for_quote_id, sizeof(insert_quote_req.QuoteID));
    insert_quote_req.LocalOrderID = new_order_ref;
    insert_quote_req.BuyOpenCloseType = X1FieldConvert::GetX1OCFlag(p->buy_open_close);
    insert_quote_req.BuySpeculator = X1FieldConvert::GetX1HedgeType(p->buy_speculator);
    insert_quote_req.BuyInsertPrice = p->buy_limit_price;
    insert_quote_req.BuyOrderAmount = p->buy_volume;
    insert_quote_req.SellOpenCloseType = X1FieldConvert::GetX1OCFlag(p->sell_open_close);
    insert_quote_req.SellSpeculator = X1FieldConvert::GetX1HedgeType(p->sell_speculator);
    insert_quote_req.SellInsertPrice = p->sell_limit_price;
    insert_quote_req.SellOrderAmount = p->sell_volume;

    insert_quote_req.InstrumentType = X1FTDC_INSTRUMENT_TYPE_OPT;      //合约类型, 可选值：期货、期权，默认期权
    if (strlen(insert_quote_req.InstrumentID) < 10) insert_quote_req.InstrumentType = X1FTDC_INSTRUMENT_TYPE_COMM;
}

void X1Packer::CancelQuoteRequest(const TunnelConfigData& cfg, const T_CancelQuote* req,
    OrderRefDataType org_quote_ref, CX1FtdcCancelOrderField& cancle_order)
{
    memset(&cancle_order, 0, sizeof(CX1FtdcCancelOrderField));
    strncpy(cancle_order.AccountID, cfg.Logon_config().clientid.c_str(), sizeof(TX1FtdcAccountIDType));
    strncpy(cancle_order.InstrumentID, req->stock_code, sizeof(TX1FtdcInstrumentIDType));
    cancle_order.LocalOrderID = org_quote_ref;
    //cancle_order.spdOrderID = req->entrust_no;
    cancle_order.X1OrderID = -1;
}

void X1Packer::OrderRespond(int error_no, long serial_no, long entrust_no, char entrust_status, T_OrderRespond& rsp)
{
    memset(&rsp, 0, sizeof(T_OrderRespond));
    rsp.serial_no = serial_no;
    rsp.error_no = error_no;
    rsp.entrust_no = entrust_no;
    rsp.entrust_status = entrust_status;
}

void X1Packer::CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond& cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(T_CancelRespond));
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;
    cancel_respond.entrust_no = entrust_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void X1Packer::OrderReturn(const CX1FtdcRspPriOrderField* rsp, const X1OrderInfo* p_req, T_OrderReturn& order_return)
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

void X1Packer::OrderReturn(const CX1FtdcRspPriMatchInfoField* rsp, const X1OrderInfo* p_req, T_OrderReturn& order_return)
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

void X1Packer::OrderReturn(const CX1FtdcRspPriCancelOrderField* rsp, const X1OrderInfo* p_req, T_OrderReturn& order_return)
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

void X1Packer::TradeReturn(const CX1FtdcRspPriMatchInfoField* rsp, const X1OrderInfo* p_req, T_TradeReturn& trade_return)
{
    memset(&trade_return, 0, sizeof(trade_return));

    trade_return.serial_no = p_req->serial_no;
    trade_return.entrust_no = p_req->entrust_no;
    trade_return.business_volume = rsp->MatchedAmount;
    trade_return.business_price = rsp->MatchedPrice;
    trade_return.business_no = atoi(rsp->MatchID);

    memcpy(trade_return.stock_code, p_req->stock_code, sizeof(trade_return.stock_code));
    trade_return.direction = p_req->direction;
    trade_return.open_close = p_req->open_close;
}

void X1Packer::QuoteRespond(int error_no, long serial_no, long entrust_no, char entrust_status, T_InsertQuoteRespond& rsp)
{
    memset(&rsp, 0, sizeof(rsp));
    rsp.serial_no = serial_no;
    rsp.error_no = error_no;
    rsp.entrust_no = entrust_no;
    rsp.entrust_status = entrust_status;
}

void X1Packer::CancelQuoteRespond(int error_no, long serial_no, long entrust_no, T_CancelQuoteRespond& crsp)
{
    memset(&crsp, 0, sizeof(crsp));
    crsp.serial_no = serial_no;
    crsp.error_no = error_no;
    crsp.entrust_no = entrust_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    crsp.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) crsp.entrust_status = MY_TNL_OS_ERROR;
}

void X1Packer::QuoteReturn(char dir, const char *instrument_id, const X1QuoteInfo *p_req, T_QuoteReturn &rtn)
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

