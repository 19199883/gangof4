#include "my_structure_convert.h"

#include <sstream>
#include <stdlib.h>

#include "lts_field_convert.h"
#include "config_data.h"

void LTSPacker::OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
    OrderRefDataType new_order_ref, CSecurityFtdcInputOrderField &po)
{
    char exch_code = cfg.Logon_config().exch_code.c_str()[0];
    memset(&po, 0, sizeof(po));
    po.RequestID = 0;
    strncpy(po.BrokerID, cfg.Logon_config().brokerid.c_str(), sizeof(po.BrokerID));
    strncpy(po.InvestorID, cfg.Logon_config().clientid.c_str(), sizeof(po.InvestorID));
    strncpy(po.InstrumentID, req->stock_code, sizeof(po.InstrumentID));
    strncpy(po.ExchangeID, LTSFieldConvert::ExchCodeToExchName(req->exchange_type), sizeof(po.ExchangeID));

    snprintf(po.OrderRef, sizeof(po.OrderRef), "%lld", new_order_ref);
    strncpy(po.UserID, cfg.Logon_config().clientid.c_str(), sizeof(po.UserID));
    po.OrderPriceType = LTSFieldConvert::GetLTSPriceType(req->order_kind);
    po.Direction = req->direction;
    po.CombOffsetFlag[0] = LTSFieldConvert::GetLTSOCFlag(exch_code, req->open_close);
    po.CombHedgeFlag[0] = LTSFieldConvert::GetLTSHedgeType(req->speculator);

    snprintf(po.LimitPrice, sizeof(po.LimitPrice), "%.4f", req->limit_price);
    po.VolumeTotalOriginal = req->volume;

    // 有效期类型
    po.TimeCondition = LTSFieldConvert::GetLTSTimeCondition(req->order_type);
    // GTD日期
    strcpy(po.GTDDate, "");
    // 成交量类型
    po.VolumeCondition = SECURITY_FTDC_VC_AV;
    // 最小成交量
    po.MinVolume = 0;
    // 触发条件
    po.ContingentCondition = SECURITY_FTDC_CC_Immediately;
    // 止损价
    po.StopPrice = 0;
    // 强平原因
    po.ForceCloseReason = SECURITY_FTDC_FCC_NotForceClose;
    // 自动挂起标志
    po.IsAutoSuspend = 0;
    //用户强平标志
    po.UserForceClose = 0;
}

void LTSPacker::OrderRespond(int error_no, long serial_no, long entrust_no, T_OrderRespond &order_respond)
{
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = entrust_no;
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;

    order_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != TUNNEL_ERR_CODE::RESULT_SUCCESS) order_respond.entrust_status = MY_TNL_OS_ERROR;
}

void LTSPacker::OrderRespond(int error_no, long serial_no, long entrust_no, char OrderSubmitStatus,
    char entrust_status, T_OrderRespond &order_respond)
{
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = entrust_no;
    order_respond.entrust_status = LTSFieldConvert::EntrustStatusTrans(OrderSubmitStatus, entrust_status);
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;
}

void LTSPacker::CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req,
    OrderRefDataType order_ref, OrderRefDataType org_order_ref,
    const OriginalReqInfo *org_order_info, CSecurityFtdcInputOrderActionField &cancle_order)
{
    memset(&cancle_order, 0, sizeof(cancle_order));
    strncpy(cancle_order.BrokerID, cfg.Logon_config().brokerid.c_str(), sizeof(TSecurityFtdcBrokerIDType));
    strncpy(cancle_order.InvestorID, cfg.Logon_config().clientid.c_str(), sizeof(TSecurityFtdcInvestorIDType));

    cancle_order.OrderActionRef = order_ref;

    // 原报单引用
    if (org_order_info)
    {
        snprintf(cancle_order.OrderRef, sizeof(TSecurityFtdcOrderRefType), "%lld", org_order_ref);
        cancle_order.FrontID = org_order_info->front_id;
        cancle_order.SessionID = org_order_info->session_id;
    }

    // 原报单交易所标识
    strncpy(cancle_order.ExchangeID, LTSFieldConvert::ExchCodeToExchName(req->exchange_type), sizeof(TSecurityFtdcExchangeIDType));

    cancle_order.ActionFlag = SECURITY_FTDC_AF_Delete;
    cancle_order.LimitPrice = 0;
    cancle_order.VolumeChange = 0;

    strncpy(cancle_order.UserID, cfg.Logon_config().clientid.c_str(), sizeof(cancle_order.UserID));
    strncpy(cancle_order.InstrumentID, req->stock_code, sizeof(cancle_order.InstrumentID));
}

void LTSPacker::CancelRespond(int error_no, long serial_no, long entrust_no, T_CancelRespond &cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(cancel_respond));
    cancel_respond.entrust_no = entrust_no;
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != TUNNEL_ERR_CODE::RESULT_SUCCESS) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void LTSPacker::OrderReturn(const CSecurityFtdcOrderField *rsp, const OriginalReqInfo *p_req, T_OrderReturn &order_return)
{
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = LTSFieldConvert::GetEntrustNo(rsp->OrderSysID);
    order_return.entrust_status = LTSFieldConvert::EntrustStatusTrans(rsp->OrderSubmitStatus, rsp->OrderStatus);
    order_return.serial_no = p_req->serial_no;

    strncpy(order_return.stock_code, rsp->InstrumentID, sizeof(TSecurityFtdcInstrumentIDType));
    order_return.direction = rsp->Direction;
    order_return.open_close = rsp->CombOffsetFlag[0];
    order_return.speculator = LTSFieldConvert::EntrustTbFlagTrans(rsp->CombHedgeFlag[0]);
    order_return.volume = rsp->VolumeTotalOriginal;
    order_return.limit_price = atof(rsp->LimitPrice);

    order_return.volume_remain = rsp->VolumeTotalOriginal - rsp->VolumeTraded;
}

void LTSPacker::TradeReturn(const CSecurityFtdcTradeField *rsp, const OriginalReqInfo *p_req, T_TradeReturn &trade_return)
{
    memset(&trade_return, 0, sizeof(trade_return));
    int id_len = strlen(rsp->TradeID);
    int id_offset = 0;
    if (id_len > 8)
    {
        id_offset = id_len - 8; // get 8 bits at the tail
    }
    trade_return.business_no = atoi(rsp->TradeID + id_offset);
    trade_return.business_price = atof(rsp->Price);
    trade_return.business_volume = rsp->Volume;
    trade_return.entrust_no = LTSFieldConvert::GetEntrustNo(rsp->OrderSysID);
    trade_return.serial_no = p_req->serial_no;

    strncpy(trade_return.stock_code, rsp->InstrumentID, sizeof(TSecurityFtdcInstrumentIDType));
    trade_return.direction = p_req->buy_sell_flag;
    trade_return.open_close = p_req->open_close_flag;
}
