#include "my_protocol_packager.h"

#include <sstream>
#include <stdlib.h>

#include "field_convert.h"
#include "config_data.h"
#include "esunny_trade_context.h"

bool ESUNNYPacker::OrderRequest(const TunnelConfigData& cfg, const T_PlaceOrder* req, TapAPINewOrder& new_or)
{
    const TapAPITradeContractInfo * ci = ESUNNYFieldConvert::GetContractInfo(req->stock_code);
    if (!ci)
    {
        return false;
    }

    memset(&new_or, 0, sizeof(new_or));

    strcpy(new_or.AccountNo, cfg.Logon_config().clientid.c_str());
    strcpy(new_or.ExchangeNo, ci->ExchangeNo);
    new_or.CommodityType = ci->CommodityType;
    strcpy(new_or.CommodityNo, ci->CommodityNo);
    strcpy(new_or.ContractNo, ci->ContractNo1);
    strcpy(new_or.StrikePrice, ci->StrikePrice1);
    new_or.CallOrPutFlag = ci->CallOrPutFlag1;
    strcpy(new_or.ContractNo2, ci->ContractNo2);
    strcpy(new_or.StrikePrice2, ci->StrikePrice2);
    new_or.CallOrPutFlag2 = ci->CallOrPutFlag2;
    new_or.OrderType = ESUNNYFieldConvert::GetESUNNYPriceType(req->order_kind);
    new_or.OrderSource = TAPI_ORDER_SOURCE_ESUNNY_API;
    new_or.TimeInForce = ESUNNYFieldConvert::GetESUNNYTimeCondition(req->order_type);
    strcpy(new_or.ExpireTime, "");
    new_or.IsRiskOrder = APIYNFLAG_NO;
    new_or.OrderSide = ESUNNYFieldConvert::GetESUNNYSide(req->direction);
    new_or.PositionEffect = ESUNNYFieldConvert::GetESUNNYOCFlag(req->open_close);
    new_or.PositionEffect2 = TAPI_PositionEffect_NONE;
    strcpy(new_or.InquiryNo, "");
    new_or.HedgeFlag = ESUNNYFieldConvert::GetESUNNYHedgeType(req->speculator);
    new_or.OrderPrice = req->limit_price;
//    new_or.OrderPrice2;
//    new_or.StopPrice;
    new_or.OrderQty = req->volume;
//    new_or.OrderMinQty;
//    new_or.MinClipSize;
//    new_or.MaxClipSize;
    new_or.RefInt = 0;
    //snprintf(new_or.RefString, sizeof(new_or.RefString), "%d", 0);
    new_or.TacticsType = TAPI_TACTICS_TYPE_NONE;
    new_or.TriggerCondition = TAPI_TRIGGER_CONDITION_NONE;
    new_or.TriggerPriceType = TAPI_TRIGGER_PRICE_NONE;
    new_or.AddOneIsValid = APIYNFLAG_NO;
    new_or.HedgeFlag2 = new_or.HedgeFlag;
    new_or.OrderDeleteByDisConnFlag = APIYNFLAG_NO;
    return true;
}

bool ESUNNYPacker::QuoteRequest(const TunnelConfigData &cfg, const T_ReqForQuote *req, TapAPINewOrder &quote_or)
{
    const TapAPITradeContractInfo * ci = ESUNNYFieldConvert::GetContractInfo(req->stock_code);
    if (!ci)
    {
        return false;
    }

    memset(&quote_or, 0, sizeof(quote_or));
    strcpy(quote_or.AccountNo, cfg.Logon_config().clientid.c_str());
    strcpy(quote_or.ExchangeNo, ci->ExchangeNo);
    strcpy(quote_or.CommodityNo, ci->CommodityNo);
    strcpy(quote_or.ContractNo, ci->ContractNo1);
    strcpy(quote_or.StrikePrice, ci->StrikePrice1);
    quote_or.CommodityType = ci->CommodityType;
    quote_or.CallOrPutFlag = ci->CallOrPutFlag1;
    ;
    quote_or.CallOrPutFlag2 = ci->CallOrPutFlag2;
    quote_or.OrderType = TAPI_ORDER_TYPE_REQQUOT;
    quote_or.OrderSource = TAPI_ORDER_SOURCE_ESUNNY_API;
    quote_or.OrderSide = TAPI_SIDE_NONE;
    quote_or.TimeInForce = TAPI_ORDER_TIMEINFORCE_GFD;
    quote_or.IsRiskOrder = APIYNFLAG_NO;
    quote_or.AddOneIsValid = APIYNFLAG_NO;
    quote_or.OrderDeleteByDisConnFlag = APIYNFLAG_NO;
    quote_or.PositionEffect = TAPI_PositionEffect_NONE;
    quote_or.PositionEffect2 = TAPI_PositionEffect_NONE;
    quote_or.HedgeFlag = TAPI_HEDGEFLAG_NONE;
    quote_or.HedgeFlag2 = TAPI_HEDGEFLAG_NONE;
    quote_or.TacticsType = TAPI_TACTICS_TYPE_NONE;
    quote_or.TriggerCondition = TAPI_TRIGGER_CONDITION_NONE;
    quote_or.TriggerPriceType = TAPI_TRIGGER_PRICE_NONE;
    return true;
}

bool ESUNNYPacker::QuoteInsertRequest(const TunnelConfigData &cfg, const T_InsertQuote *req, TapAPINewOrder &quote_or)
{
    const TapAPITradeContractInfo * ci = ESUNNYFieldConvert::GetContractInfo(req->stock_code);
    if (!ci)
    {
        return false;
    }

    memset(&quote_or, 0, sizeof(quote_or));
    strcpy(quote_or.AccountNo, cfg.Logon_config().clientid.c_str());
    strcpy(quote_or.CommodityNo, ci->CommodityNo);
    strcpy(quote_or.ExchangeNo, ci->ExchangeNo);
    strcpy(quote_or.ContractNo, ci->ContractNo1);
    strcpy(quote_or.ContractNo2, ci->ContractNo2);
    strcpy(quote_or.StrikePrice, ci->StrikePrice1);
    strcpy(quote_or.StrikePrice2, ci->StrikePrice2);
    quote_or.CommodityType = ci->CommodityType;
    quote_or.CallOrPutFlag = ci->CallOrPutFlag1;
    quote_or.CallOrPutFlag2 = ci->CallOrPutFlag2;
    quote_or.OrderType = TAPI_ORDER_TYPE_RSPQUOT;
    quote_or.OrderSource = TAPI_ORDER_SOURCE_ESUNNY_API;
    quote_or.TimeInForce = TAPI_ORDER_TIMEINFORCE_GFD;
    quote_or.OrderSide = TAPI_SIDE_NONE;
    quote_or.OrderPrice = req->buy_limit_price;
    quote_or.OrderPrice2 = req->sell_limit_price;
    quote_or.OrderQty = req->buy_volume;
    quote_or.OrderQty2 = req->sell_volume;
    quote_or.PositionEffect = ESUNNYFieldConvert::GetESUNNYOCFlag(req->buy_open_close);
    quote_or.PositionEffect2 = ESUNNYFieldConvert::GetESUNNYOCFlag(req->sell_open_close);
    quote_or.HedgeFlag = ESUNNYFieldConvert::GetESUNNYHedgeType(req->buy_speculator);
    quote_or.HedgeFlag2 = ESUNNYFieldConvert::GetESUNNYHedgeType(req->sell_speculator);
    strcpy(quote_or.InquiryNo, req->for_quote_id);
    quote_or.TacticsType = TAPI_TACTICS_TYPE_NONE;
    quote_or.TriggerCondition = TAPI_TRIGGER_CONDITION_NONE;
    quote_or.TriggerPriceType = TAPI_TRIGGER_PRICE_NONE;
    quote_or.AddOneIsValid = APIYNFLAG_NO;
    quote_or.OrderDeleteByDisConnFlag = APIYNFLAG_NO;
    quote_or.IsRiskOrder = APIYNFLAG_NO;
    return true;
}

void ESUNNYPacker::OrderRespond(int error_no, long serial_no, long entrust_no, short entrust_status, T_OrderRespond& rsp)
{
    memset(&rsp, 0, sizeof(T_OrderRespond));
    rsp.serial_no = serial_no;
    rsp.error_no = error_no;
    rsp.entrust_no = entrust_no;
    rsp.entrust_status = entrust_status;
}

void ESUNNYPacker::CancelRequest(const EsunnyOrderInfo* org_order_info, TapAPIOrderCancelReq& co)
{
    memset(&co, 0, sizeof(co));

    //co.RefInt = o.RefInt;                                                  ///< 整型参考值
    //memcpy(co.RefString, o.RefString, sizeof(co.RefString));               ///< 字符串参考值
    co.ServerFlag = org_order_info->server_flag;                             ///< 服务器标识
    memcpy(co.OrderNo, org_order_info->order_no, sizeof(co.OrderNo));        ///< 委托编码
}

void ESUNNYPacker::CancelRequest(const EsunnyQuoteInfo* org_quote_info, TapAPIOrderCancelReq& co)
{
    memset(&co, 0, sizeof(co));

    //co.RefInt = o.RefInt;                                                  ///< 整型参考值
    //memcpy(co.RefString, o.RefString, sizeof(co.RefString));               ///< 字符串参考值
    co.ServerFlag = org_quote_info->server_flag;                             ///< 服务器标识
    memcpy(co.OrderNo, org_quote_info->order_no, sizeof(co.OrderNo));        ///< 委托编码
}

void ESUNNYPacker::CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond& cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(T_CancelRespond));
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;
    cancel_respond.entrust_no = entrust_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void ESUNNYPacker::OrderReturn(const TapAPIOrderInfo* rsp, const EsunnyOrderInfo* p_req, T_OrderReturn& order_return)
{
    memset(&order_return, 0, sizeof(order_return));

    order_return.serial_no = p_req->po.serial_no;
    memcpy(order_return.stock_code, p_req->po.stock_code, sizeof(order_return.stock_code));
    order_return.entrust_no = p_req->entrust_no;
    order_return.entrust_status = ESUNNYFieldConvert::GetMYEntrustStatus(rsp->OrderState);
    order_return.direction = p_req->po.direction;
    order_return.open_close = p_req->po.open_close;
    order_return.speculator = p_req->po.speculator;
    order_return.volume = p_req->po.volume;
    order_return.limit_price = p_req->po.limit_price;
    order_return.volume_remain = p_req->volume_remain;
}

void ESUNNYPacker::TradeReturn(const TapAPIFillInfo* rsp, const EsunnyOrderInfo* p_req, T_TradeReturn& trade_return)
{
    memset(&trade_return, 0, sizeof(trade_return));

    trade_return.serial_no = p_req->po.serial_no;
    trade_return.entrust_no = p_req->entrust_no;
    trade_return.business_volume = rsp->MatchQty;
    trade_return.business_price = rsp->MatchPrice;
    int id_len = strlen(rsp->ExchangeMatchNo);
    int id_offset = 0;
    if (id_len > 8)
    {
        id_offset = id_len - 8; // get 8 bits at the tail
    }
    trade_return.business_no = atoi(rsp->ExchangeMatchNo + id_offset);

    memcpy(trade_return.stock_code, p_req->po.stock_code, sizeof(trade_return.stock_code));
    trade_return.direction = p_req->po.direction;
    trade_return.open_close = p_req->po.open_close;
}

void ESUNNYPacker::QuoteRequestRespond(const T_ReqForQuote *rsp, int error_no, T_RspOfReqForQuote &quote_respond)
{
    memset(&quote_respond, 0, sizeof(quote_respond));
    quote_respond.error_no = error_no;
    quote_respond.serial_no = rsp->serial_no;
}

void ESUNNYPacker::QuoteInsertRespond(const T_InsertQuote *rsp, int error_no, T_InsertQuoteRespond &quote_respond)
{
    memset(&quote_respond, 0, sizeof(quote_respond));
    quote_respond.error_no = error_no;
    quote_respond.serial_no = rsp->serial_no;
    quote_respond.entrust_no = 0;

    quote_respond.entrust_status = MY_TNL_OS_ERROR;
    if (error_no == TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        quote_respond.entrust_status = MY_TNL_OS_REPORDED;
    }
}

void ESUNNYPacker::QuoteActionRespond(const T_CancelQuote *rsp, int error_no, T_CancelQuoteRespond &quote_respond)
{
    memset(&quote_respond, 0, sizeof(quote_respond));
    quote_respond.error_no = error_no;
    quote_respond.serial_no = rsp->serial_no;
    quote_respond.entrust_no = 0;

    quote_respond.entrust_status = MY_TNL_OS_ERROR;
    if (error_no == TUNNEL_ERR_CODE::RESULT_SUCCESS)
    {
        quote_respond.entrust_status = MY_TNL_OS_REPORDED;
    }
}

void ESUNNYPacker::QuoteInsertReturn(const TapAPIOrderInfo *rsp, const EsunnyQuoteInfo *p_req, T_QuoteReturn &quote_return,
    TMyTnlDirectionType direction)
{
    memset(&quote_return, 0, sizeof(quote_return));
    quote_return.serial_no = p_req->iq.serial_no;
    strcpy(quote_return.stock_code, p_req->iq.stock_code);
    quote_return.entrust_no = p_req->entrust_no;
    quote_return.entrust_status = ESUNNYFieldConvert::GetMYEntrustStatus(rsp->OrderState);
    quote_return.direction = direction;
    if (direction == MY_TNL_D_BUY)
    {
        quote_return.limit_price = rsp->OrderPrice;
        quote_return.volume = rsp->OrderQty;
        quote_return.volume_remain = p_req->buy_volume_remain - rsp->OrderMatchQty;
        quote_return.open_close = ESUNNYFieldConvert::GetMYOCFlag(rsp->PositionEffect);
        quote_return.speculator = ESUNNYFieldConvert::GetMYHedgeFlag(rsp->HedgeFlag);
    }
    else
    {
        quote_return.limit_price = rsp->OrderPrice2;
        quote_return.volume = rsp->OrderQty;
        quote_return.volume_remain = p_req->sell_volume_remain - rsp->OrderMatchQty2;
        quote_return.open_close = ESUNNYFieldConvert::GetMYOCFlag(rsp->PositionEffect2);
        quote_return.speculator = ESUNNYFieldConvert::GetMYHedgeFlag(rsp->HedgeFlag2);
    }
}

void ESUNNYPacker::QuoteInsertTrade(const TapAPIFillInfo *rsp, const EsunnyQuoteInfo *p_req, T_QuoteTrade &quote_trade)
{
    memset(&quote_trade, 0, sizeof(quote_trade));
    quote_trade.serial_no = p_req->iq.serial_no;
    strcpy(quote_trade.stock_code, p_req->iq.stock_code);
    quote_trade.open_close = ESUNNYFieldConvert::GetMYOCFlag(rsp->PositionEffect);
    quote_trade.direction = ESUNNYFieldConvert::GetMYSide(rsp->MatchSide);
    quote_trade.business_volume = rsp->MatchQty;
    quote_trade.business_price = rsp->MatchPrice;
    quote_trade.business_no = rsp->MatchStreamID;
}

void ESUNNYPacker::ForQuoteRespond(TunnelConfigData &cfg, long serial_no, const TapAPIOrderInfo *rsp, T_RspOfReqForQuote &for_quote_respond)
{
    memset(&for_quote_respond, 0, sizeof(for_quote_respond));
    if (rsp->ErrorCode != TAPIERROR_SUCCEED)
    {
        for_quote_respond.error_no = cfg.GetStandardErrorNo(rsp->ErrorCode);
    }
    for_quote_respond.serial_no = serial_no;
}

void ESUNNYPacker::ForQuoteReturn(const TapAPIReqQuoteNotice *rsp, T_RtnForQuote &for_quote_return)
{
    memset(&for_quote_return, 0, sizeof(rsp));
    std::string stock_tmp = rsp->CommodityNo;
    stock_tmp.append(rsp->ContractNo);
    if (rsp->CallOrPutFlag != TAPI_CALLPUT_FLAG_NONE)
    {
        stock_tmp.append(1, rsp->CallOrPutFlag);
        stock_tmp.append(rsp->StrikePrice);
    }
    strcpy(for_quote_return.stock_code, stock_tmp.c_str());
    strcpy(for_quote_return.for_quote_id, rsp->InquiryNo);
    strncpy(for_quote_return.for_quote_time, rsp->UpdateTime + 11, 9);
}
