#include "my_protocol_packager.h"

#include <sstream>
#include <stdlib.h>

#include "field_convert.h"
#include "config_data.h"

void ApiPacker::OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
    OrderRefDataType new_order_ref, CZeusingFtdcInputOrderField &insert_order)
{
    char exch_code = cfg.Logon_config().exch_code.c_str()[0];
    memset(&insert_order, 0, sizeof(insert_order));
    insert_order.RequestID = 0;
    strncpy(insert_order.BrokerID, cfg.Logon_config().brokerid.c_str(), sizeof(TZeusingFtdcBrokerIDType));
    strncpy(insert_order.InvestorID, cfg.Logon_config().clientid.c_str(), sizeof(TZeusingFtdcInvestorIDType));
    strncpy(insert_order.InstrumentID, req->stock_code, sizeof(TZeusingFtdcInstrumentIDType));

    snprintf(insert_order.OrderRef, sizeof(TZeusingFtdcOrderRefType), "%lld", new_order_ref);
    strncpy(insert_order.UserID, cfg.Logon_config().clientid.c_str(), sizeof(TZeusingFtdcInvestorIDType));
    insert_order.OrderPriceType = DataFieldConvert::GetApiPriceType(req->order_kind);
    insert_order.Direction = req->direction;
    insert_order.CombOffsetFlag[0] = DataFieldConvert::GetApiOCFlag(exch_code, req->open_close);
    insert_order.CombHedgeFlag[0] = DataFieldConvert::GetApiHedgeType(req->speculator);

    insert_order.LimitPrice = req->limit_price;
    insert_order.VolumeTotalOriginal = req->volume;

    // 有效期类型
    insert_order.TimeCondition = DataFieldConvert::GetApiTimeCondition(req->order_type);
    // GTD日期
    strcpy(insert_order.GTDDate, "");
    // 成交量类型
    insert_order.VolumeCondition = ZEUSING_FTDC_VC_AV;
    // 最小成交量
    insert_order.MinVolume = 0;
    // 触发条件
    insert_order.ContingentCondition = ZEUSING_FTDC_CC_Immediately;
    // 止损价
    insert_order.StopPrice = 0;
    // 强平原因
    insert_order.ForceCloseReason = ZEUSING_FTDC_FCC_NotForceClose;
    // 自动挂起标志
    insert_order.IsAutoSuspend = 0;
    //用户强平标志
    insert_order.UserForceClose = 0;
}

void ApiPacker::OrderRespond(const int error_no, const long serial_no, const char* entrust_no, const char OrderSubmitStatus,
    const char entrust_status, T_OrderRespond &order_respond)
{
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = atol(entrust_no);
    order_respond.entrust_status = DataFieldConvert::GetMYEntrustStatus(OrderSubmitStatus, entrust_status);
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;
}

void ApiPacker::OrderRespond(const int error_no, const long serial_no, const long entrust_no,
    const short entrust_status, T_OrderRespond &order_respond)
{
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = entrust_no;
    order_respond.entrust_status = 'e';
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;
}

void ApiPacker::CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req,
    OrderRefDataType order_ref, OrderRefDataType org_order_ref,
    const OriginalReqInfo *org_order_info, CZeusingFtdcInputOrderActionField &cancle_order)
{
    memset(&cancle_order, 0, sizeof(cancle_order));
    strncpy(cancle_order.BrokerID, cfg.Logon_config().brokerid.c_str(), sizeof(TZeusingFtdcBrokerIDType));
    strncpy(cancle_order.InvestorID, cfg.Logon_config().clientid.c_str(), sizeof(TZeusingFtdcInvestorIDType));

    cancle_order.OrderActionRef = order_ref;
    cancle_order.Direction = req->direction;

    // 原报单引用
    if (org_order_info)
    {
        snprintf(cancle_order.OrderRef, sizeof(TZeusingFtdcOrderRefType), "%lld", org_order_ref);
        cancle_order.FrontID = org_order_info->front_id;
        cancle_order.SessionID = org_order_info->session_id;
    }

    // 原报单交易所标识
    strncpy(cancle_order.ExchangeID, DataFieldConvert::ExchCodeToExchName(req->exchange_type), sizeof(TZeusingFtdcExchangeIDType));

    if (req->entrust_no != 0)
    {
        DataFieldConvert::FormatOrderSysId(req->entrust_no, cancle_order.OrderSysID);
    }

    cancle_order.ActionFlag = ZEUSING_FTDC_AF_Delete;
    cancle_order.LimitPrice = req->limit_price;
    cancle_order.VolumeChange = 0;

    strncpy(cancle_order.UserID, cfg.Logon_config().clientid.c_str(), sizeof(TZeusingFtdcInvestorIDType));
    strncpy(cancle_order.InstrumentID, req->stock_code, sizeof(TZeusingFtdcInstrumentIDType));

}

void ApiPacker::CancelRespond(int error_no, long serial_no, const char* entrust_no, T_CancelRespond &cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(cancel_respond));
    cancel_respond.entrust_no = atol(entrust_no);
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void ApiPacker::CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond &cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(cancel_respond));
    cancel_respond.entrust_no = entrust_no;
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void ApiPacker::OrderReturn(const CZeusingFtdcOrderField *rsp, const OriginalReqInfo *p_req, T_OrderReturn &order_return)
{
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = atol(rsp->OrderSysID);
    order_return.entrust_status = DataFieldConvert::GetMYEntrustStatus(rsp->OrderSubmitStatus, rsp->OrderStatus);
    order_return.serial_no = p_req->serial_no;

    strncpy(order_return.stock_code, rsp->InstrumentID, sizeof(TZeusingFtdcInstrumentIDType));
    order_return.direction = rsp->Direction;
    order_return.open_close = rsp->CombOffsetFlag[0];
    order_return.speculator = DataFieldConvert::GetMYHedgeFlag(rsp->CombHedgeFlag[0]);
    order_return.volume = rsp->VolumeTotalOriginal;
    order_return.limit_price = rsp->LimitPrice;

    order_return.volume_remain = rsp->VolumeTotal;
}

void ApiPacker::TradeReturn(const CZeusingFtdcTradeField *rsp, const OriginalReqInfo *p_req, T_TradeReturn &trade_return)
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

    strncpy(trade_return.stock_code, rsp->InstrumentID, sizeof(TZeusingFtdcInstrumentIDType));
    trade_return.direction = p_req->buy_sell_flag;
    trade_return.open_close = p_req->open_close_flag;
}
