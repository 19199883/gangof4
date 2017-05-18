#include "my_protocol_packager.h"

#include <sstream>
#include <stdlib.h>

#include "field_convert.h"
#include "config_data.h"

void XELEPacker::OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
    OrderRefDataType new_order_ref, CXeleFtdcInputOrderField &req_fld)
{
    memset(&req_fld, 0, sizeof(req_fld));

    strcpy(req_fld.OrderSysID, ""); //系统报单编号，填什么内容?
    strncpy(req_fld.ParticipantID, cfg.Logon_config().ParticipantID.c_str(), sizeof(req_fld.ParticipantID));
    strncpy(req_fld.ClientID, cfg.Logon_config().investorid.c_str(), sizeof(req_fld.ClientID));
    strncpy(req_fld.UserID, cfg.Logon_config().clientid.c_str(), sizeof(req_fld.UserID));
    strncpy(req_fld.InstrumentID, req->stock_code, sizeof(req_fld.InstrumentID));
    req_fld.OrderPriceType = XeleFieldConvert::GetApiPriceType(req->order_kind);
    req_fld.Direction = req->direction;
    req_fld.CombOffsetFlag[0] = XeleFieldConvert::GetOCFlag(MY_TNL_EC_SHFE, req->open_close);
    req_fld.CombHedgeFlag[0] = XeleFieldConvert::GetApiHedgeType(req->speculator);
    req_fld.LimitPrice = req->limit_price;
    req_fld.VolumeTotalOriginal = req->volume;

    // 有效期类型
    req_fld.TimeCondition = XeleFieldConvert::GetApiTimeCondition(req->order_type);
    if (req_fld.OrderPriceType == XELE_FTDC_OPT_AnyPrice)
    {
        req_fld.TimeCondition = XELE_FTDC_TC_IOC;
    }
    // GTD日期
    strcpy(req_fld.GTDDate, "");
    // 成交量类型
    req_fld.VolumeCondition = XELE_FTDC_VC_AV;
    // 最小成交量
    req_fld.MinVolume = 0;
    if (req->order_type == MY_TNL_HF_FOK)
    {
        req_fld.VolumeCondition = XELE_FTDC_VC_CV;
        req_fld.MinVolume = req_fld.VolumeTotalOriginal;
    }
    ///触发条件
    req_fld.ContingentCondition = XELE_FTDC_CC_Immediately;
    // 止损价
    req_fld.StopPrice = 0;
    // 强平原因
    req_fld.ForceCloseReason = XELE_FTDC_FCC_NotForceClose;
    ///本地报单编号
    snprintf(req_fld.OrderLocalID, sizeof(TXeleFtdcOrderLocalIDType), "%012lld", new_order_ref);
    // 自动挂起标志
    req_fld.IsAutoSuspend = 0;
}

void XELEPacker::OrderRespond(int error_no, SerialNoType serial_no, const char* entrust_no,
    char entrust_status, T_OrderRespond &order_respond)
{
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = atol(entrust_no);
    order_respond.entrust_status = XeleFieldConvert::GetMYEntrustStatus(entrust_status);
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;
}

void XELEPacker::CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req, OrderRefDataType order_ref,
    OrderRefDataType org_order_ref, CXeleFtdcOrderActionField &req_fld)
{
    memset(&req_fld, 0, sizeof(req_fld));

    // 原报单交易所标识
    snprintf(req_fld.OrderSysID, sizeof(TXeleFtdcOrderSysIDType), "%012ld", req->entrust_no);
    snprintf(req_fld.OrderLocalID, sizeof(TXeleFtdcOrderLocalIDType), "%012lld", org_order_ref);
    req_fld.ActionFlag = XELE_FTDC_AF_Delete;

    strncpy(req_fld.ParticipantID, cfg.Logon_config().ParticipantID.c_str(), sizeof(req_fld.ParticipantID));
    strncpy(req_fld.ClientID, cfg.Logon_config().investorid.c_str(), sizeof(req_fld.ClientID));
    strncpy(req_fld.UserID, cfg.Logon_config().clientid.c_str(), sizeof(req_fld.UserID));

    req_fld.LimitPrice = 0;
    req_fld.VolumeChange = 0;

    snprintf(req_fld.ActionLocalID, sizeof(req_fld.ActionLocalID), "%012lld", order_ref);
}

void XELEPacker::CancelRespond(int error_no, SerialNoType serial_no, const char* entrust_no, T_CancelRespond &cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(cancel_respond));
    cancel_respond.entrust_no = atol(entrust_no);
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void XELEPacker::OrderReturn(SerialNoType serial_no, const CXeleFtdcOrderField *rsp, T_OrderReturn &order_return)
{
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = atol(rsp->OrderSysID);
    order_return.entrust_status = XeleFieldConvert::GetMYEntrustStatus(rsp->OrderStatus);
    order_return.serial_no = serial_no;

    strncpy(order_return.stock_code, rsp->InstrumentID, sizeof(TXeleFtdcInstrumentIDType));
    order_return.direction = rsp->Direction;
    order_return.open_close = rsp->CombOffsetFlag[0];
    order_return.speculator = XeleFieldConvert::GetMYHedgeType(rsp->CombHedgeFlag[0]);
    order_return.volume = rsp->VolumeTotalOriginal;
    order_return.limit_price = rsp->LimitPrice;

    order_return.volume_remain = rsp->VolumeTotal;
}

void XELEPacker::TradeReturn(const XeleOriginalReqInfo *p_req, const CXeleFtdcTradeField *rsp, T_TradeReturn &trade_return)
{
    memset(&trade_return, 0, sizeof(trade_return));
    trade_return.business_no = atoi(rsp->TradeID);
    trade_return.business_price = rsp->Price;
    trade_return.business_volume = rsp->Volume;
    trade_return.entrust_no = atol(rsp->OrderSysID);
    trade_return.serial_no = p_req->serial_no;

    strncpy(trade_return.stock_code, rsp->InstrumentID, sizeof(TXeleFtdcInstrumentIDType));
    trade_return.direction = p_req->buy_sell_flag;
    trade_return.open_close = p_req->open_close_flag;
}
