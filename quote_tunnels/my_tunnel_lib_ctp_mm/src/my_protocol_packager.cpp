#include "my_protocol_packager.h"

#include <sstream>
#include <stdlib.h>

#include "field_convert.h"
#include "config_data.h"

void CTPPacker::OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
    OrderRefDataType new_order_ref, CThostFtdcInputOrderField &insert_order)
{
    char exch_code = cfg.Logon_config().exch_code.c_str()[0];
    memset(&insert_order, 0, sizeof(insert_order));
    insert_order.RequestID = 0;
    strncpy(insert_order.BrokerID, cfg.Logon_config().brokerid.c_str(), sizeof(TThostFtdcBrokerIDType));
    strncpy(insert_order.InvestorID, cfg.Logon_config().clientid.c_str(), sizeof(TThostFtdcInvestorIDType));
    strncpy(insert_order.InstrumentID, req->stock_code, sizeof(TThostFtdcInstrumentIDType));

    snprintf(insert_order.OrderRef, sizeof(TThostFtdcOrderRefType), "%lld", new_order_ref);
    strncpy(insert_order.UserID, cfg.Logon_config().clientid.c_str(), sizeof(TThostFtdcInvestorIDType));
    insert_order.OrderPriceType = CTPFieldConvert::GetCTPPriceType(req->order_kind);
    insert_order.Direction = req->direction;
    insert_order.CombOffsetFlag[0] = CTPFieldConvert::GetCTPOCFlag(exch_code, req->open_close);
    insert_order.CombHedgeFlag[0] = CTPFieldConvert::GetCTPHedgeType(req->speculator);

    insert_order.LimitPrice = req->limit_price;
    insert_order.VolumeTotalOriginal = req->volume;

    // 有效期类型
    insert_order.TimeCondition = CTPFieldConvert::GetCTPTimeCondition(req->order_type);
    // GTD日期
    strcpy(insert_order.GTDDate, "");
    // 成交量类型
    insert_order.VolumeCondition = THOST_FTDC_VC_AV;
    // 最小成交量
    insert_order.MinVolume = 0;

    if (req->order_type == MY_TNL_HF_FOK)
    {
        insert_order.VolumeCondition = THOST_FTDC_VC_CV;
        insert_order.MinVolume = req->volume;
    }
    // 触发条件
    insert_order.ContingentCondition = THOST_FTDC_CC_Immediately;
    // 止损价
    insert_order.StopPrice = 0;
    // 强平原因
    insert_order.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    // 自动挂起标志
    insert_order.IsAutoSuspend = 0;
    //用户强平标志
    insert_order.UserForceClose = 0;


    strcpy(insert_order.ExchangeID, CTPFieldConvert::ExchCodeToExchName(req->exchange_type));
    strcpy(insert_order.CurrencyID, CTPFieldConvert::GetCurrencyID());
    CTPFieldConvert::GetMacAndIPAddress("eth0", insert_order.MacAddress, insert_order.IPAddress);
}

void CTPPacker::OrderRespond(const int error_no, const long serial_no, const char* entrust_no, const char OrderSubmitStatus,
    const char entrust_status, T_OrderRespond &order_respond)
{
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = atol(entrust_no);
    order_respond.entrust_status = CTPFieldConvert::EntrustStatusTrans(OrderSubmitStatus, entrust_status);
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;
}

void CTPPacker::OrderRespond(const int error_no, const long serial_no, const long entrust_no,
    const short entrust_status, T_OrderRespond &order_respond)
{
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = entrust_no;
    order_respond.entrust_status = 'e';
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;
}

void CTPPacker::CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req,
    OrderRefDataType order_ref, OrderRefDataType org_order_ref,
    const OriginalReqInfo *org_order_info, CThostFtdcInputOrderActionField &cancle_order)
{
    char exch_code = cfg.Logon_config().exch_code.c_str()[0];
    memset(&cancle_order, 0, sizeof(cancle_order));
    strncpy(cancle_order.BrokerID, cfg.Logon_config().brokerid.c_str(), sizeof(TThostFtdcBrokerIDType));
    strncpy(cancle_order.InvestorID, cfg.Logon_config().clientid.c_str(), sizeof(TThostFtdcInvestorIDType));

    cancle_order.OrderActionRef = order_ref;

    // 原报单引用
    if (org_order_info)
    {
        snprintf(cancle_order.OrderRef, sizeof(TThostFtdcOrderRefType), "%lld", org_order_ref);
        cancle_order.FrontID = org_order_info->front_id;
        cancle_order.SessionID = org_order_info->session_id;
    }

    // 原报单交易所标识
    strncpy(cancle_order.ExchangeID, CTPFieldConvert::ExchCodeToExchName(exch_code), sizeof(TThostFtdcExchangeIDType));

    if (req->entrust_no != 0)
    {
        CTPFieldConvert::SysOrderIDToCTPFormat(req->entrust_no, cancle_order.OrderSysID);
    }

    cancle_order.ActionFlag = THOST_FTDC_AF_Delete;
    cancle_order.LimitPrice = 0;
    cancle_order.VolumeChange = 0;

    strncpy(cancle_order.UserID, cfg.Logon_config().clientid.c_str(), sizeof(TThostFtdcInvestorIDType));
    strncpy(cancle_order.InstrumentID, req->stock_code, sizeof(TThostFtdcInstrumentIDType));


    CTPFieldConvert::GetMacAndIPAddress("eth0", cancle_order.MacAddress, cancle_order.IPAddress);
}

void CTPPacker::CancelRespond(int error_no, long serial_no, const char* entrust_no, T_CancelRespond &cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(cancel_respond));
    cancel_respond.entrust_no = atol(entrust_no);
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void CTPPacker::CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond &cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(cancel_respond));
    cancel_respond.entrust_no = entrust_no;
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void CTPPacker::OrderReturn(const CThostFtdcOrderField *rsp, const OriginalReqInfo *p_req, T_OrderReturn &order_return)
{
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = atol(rsp->OrderSysID);
    order_return.entrust_status = CTPFieldConvert::EntrustStatusTrans(rsp->OrderSubmitStatus, rsp->OrderStatus);
    order_return.serial_no = p_req->serial_no;

    strncpy(order_return.stock_code, rsp->InstrumentID, sizeof(TThostFtdcInstrumentIDType));
    order_return.direction = rsp->Direction;
    order_return.open_close = rsp->CombOffsetFlag[0];
    order_return.speculator = CTPFieldConvert::EntrustTbFlagTrans(rsp->CombHedgeFlag[0]);
    order_return.volume = rsp->VolumeTotalOriginal;
    order_return.limit_price = rsp->LimitPrice;

    order_return.volume_remain = rsp->VolumeTotal;
}

void CTPPacker::TradeReturn(const CThostFtdcTradeField *rsp, const OriginalReqInfo *p_req, T_TradeReturn &trade_return)
{
    memset(&trade_return, 0, sizeof(trade_return));
    int id_len = strlen(rsp->TradeID);
    int id_offset = 0;
    if (id_len > 8)
    {
        id_offset = id_len - 8; // get 8 bits at the tail
    }
    trade_return.business_no = atoi(rsp->TradeID + id_offset);
    trade_return.business_price = rsp->Price;
    trade_return.business_volume = rsp->Volume;
    trade_return.entrust_no = atol(rsp->OrderSysID);
    trade_return.serial_no = p_req->serial_no;

    strncpy(trade_return.stock_code, rsp->InstrumentID, sizeof(TThostFtdcInstrumentIDType));
    trade_return.direction = p_req->buy_sell_flag;
    trade_return.open_close = p_req->open_close_flag;
}

void CTPPacker::QuoteReturn(const CThostFtdcOrderField* rsp, const CTPQuoteInfo* p_req, T_QuoteReturn& quote_return)
{
    memset(&quote_return, 0, sizeof(quote_return));
    quote_return.entrust_no = p_req->entrust_no;
    quote_return.entrust_status = CTPFieldConvert::EntrustStatusTrans(rsp->OrderSubmitStatus, rsp->OrderStatus);
    quote_return.serial_no = p_req->serial_no;

    strncpy(quote_return.stock_code, rsp->InstrumentID, sizeof(TThostFtdcInstrumentIDType));
    quote_return.direction = rsp->Direction;
    quote_return.open_close = rsp->CombOffsetFlag[0];
    quote_return.speculator = CTPFieldConvert::EntrustTbFlagTrans(rsp->CombHedgeFlag[0]);
    quote_return.volume = rsp->VolumeTotalOriginal;
    quote_return.limit_price = rsp->LimitPrice;

    quote_return.volume_remain = rsp->VolumeTotal;
}

void CTPPacker::QuoteTrade(const CThostFtdcTradeField* rsp, const CTPQuoteInfo* p_req, T_QuoteTrade& quote_trade)
{
    memset(&quote_trade, 0, sizeof(quote_trade));
    quote_trade.business_no = atoi(rsp->TradeID);
    quote_trade.business_price = rsp->Price;
    quote_trade.business_volume = rsp->Volume;
    quote_trade.entrust_no = atol(rsp->OrderSysID);
    quote_trade.serial_no = p_req->serial_no;

    strncpy(quote_trade.stock_code, rsp->InstrumentID, sizeof(TThostFtdcInstrumentIDType));
    quote_trade.direction = rsp->Direction;
    quote_trade.open_close = rsp->OffsetFlag;
}
