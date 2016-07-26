#include "sgit_struct_convert.h"

#include <sstream>
#include <stdlib.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "sgit_field_convert.h"
#include "config_data.h"
#include "sgit_trade_context.h"

void StructConvertor::OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
    OrderRefDataType new_order_ref, CSgitFtdcInputOrderField &insert_order)
{
    char exch_code = cfg.Logon_config().exch_code.c_str()[0];
    memset(&insert_order, 0, sizeof(insert_order));
    insert_order.RequestID = 0;
    strncpy(insert_order.BrokerID, cfg.Logon_config().brokerid.c_str(), sizeof(insert_order.BrokerID));
    strncpy(insert_order.InvestorID, cfg.Logon_config().investorid.c_str(), sizeof(insert_order.InvestorID));
    strncpy(insert_order.InstrumentID, req->stock_code, sizeof(insert_order.InstrumentID));

    snprintf(insert_order.OrderRef, sizeof(insert_order.OrderRef), "%012lld", new_order_ref);
    strncpy(insert_order.UserID, cfg.Logon_config().clientid.c_str(), sizeof(insert_order.UserID));
    insert_order.OrderPriceType = FieldConvert::GetApiPriceType(req->order_kind);
    insert_order.Direction = req->direction;
    insert_order.CombOffsetFlag[0] = FieldConvert::GetApiOCFlag(exch_code, req->open_close);
    insert_order.CombHedgeFlag[0] = FieldConvert::GetApiHedgeFlag(req->speculator);

    insert_order.LimitPrice = req->limit_price;
    insert_order.VolumeTotalOriginal = req->volume;

    // 有效期类型
    insert_order.TimeCondition = FieldConvert::GetApiTimeCondition(req->order_type);
    // GTD日期
    strcpy(insert_order.GTDDate, "");
    // 成交量类型
    insert_order.VolumeCondition = Sgit_FTDC_VC_AV;
    // 最小成交量
    insert_order.MinVolume = 0;
    // Fill or Kill
    if (req->order_type == MY_TNL_HF_FOK)
    {
        insert_order.VolumeCondition = Sgit_FTDC_VC_CV;
        insert_order.MinVolume = req->volume;
    }
    // 触发条件
    insert_order.ContingentCondition = Sgit_FTDC_CC_Immediately;
    // 止损价
    insert_order.StopPrice = 0;
    // 强平原因
    insert_order.ForceCloseReason = Sgit_FTDC_FCC_NotForceClose;
    // 自动挂起标志
    insert_order.IsAutoSuspend = 0;
    //用户强平标志
    insert_order.UserForceClose = 0;
}

void StructConvertor::OrderRespond(int error_no, SerialNoType serial_no, EntrustNoType entrust_no, T_OrderRespond &order_respond)
{
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = entrust_no;
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;
    if (error_no == 0)
    {
        order_respond.entrust_status = MY_TNL_OS_REPORDED;
    }
    else
    {
        order_respond.entrust_status = MY_TNL_OS_ERROR;
    }
}

void StructConvertor::CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req,
    OrderRefDataType order_ref, OrderRefDataType org_order_ref, const SgitOrderInfo * p_order,
    CSgitFtdcInputOrderActionField &cancle_order)
{
    memset(&cancle_order, 0, sizeof(cancle_order));

    strncpy(cancle_order.BrokerID, cfg.Logon_config().brokerid.c_str(), sizeof(cancle_order.BrokerID));
    strncpy(cancle_order.InvestorID, cfg.Logon_config().investorid.c_str(), sizeof(cancle_order.InvestorID));

    cancle_order.OrderActionRef = order_ref;

    // 原报单引用
    snprintf(cancle_order.OrderRef, sizeof(cancle_order.OrderRef), "%012lld", org_order_ref);

    // 原报单交易所标识
    char exch_code = cfg.Logon_config().exch_code.c_str()[0];
    cancle_order.ExchangeID[0] = FieldConvert::GetApiExID(exch_code);
    memcpy(cancle_order.OrderSysID, p_order->sys_order_id, sizeof(cancle_order.OrderSysID));


    cancle_order.ActionFlag = Sgit_FTDC_AF_Delete;

    strncpy(cancle_order.UserID, cfg.Logon_config().clientid.c_str(), sizeof(cancle_order.UserID));
    strncpy(cancle_order.InstrumentID, req->stock_code, sizeof(cancle_order.InstrumentID));
}

void StructConvertor::CancelRespond(int error_no, SerialNoType serial_no, const char* entrust_no, T_CancelRespond &cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(cancel_respond));
    cancel_respond.entrust_no = atol(entrust_no);
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void StructConvertor::CancelRespond(int error_no, SerialNoType serial_no, EntrustNoType entrust_no, T_CancelRespond &cancel_respond)
{
    memset(&cancel_respond, 0, sizeof(cancel_respond));
    cancel_respond.entrust_no = entrust_no;
    cancel_respond.serial_no = serial_no;
    cancel_respond.error_no = error_no;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    cancel_respond.entrust_status = MY_TNL_OS_REPORDED;
    if (error_no != 0) cancel_respond.entrust_status = MY_TNL_OS_ERROR;
}

void StructConvertor::OrderReturn(const CSgitFtdcOrderField *rsp, const SgitOrderInfo *p_req, T_OrderReturn &order_return)
{
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = atoll(rsp->OrderSysID);
    order_return.serial_no = p_req->po.serial_no;

    strncpy(order_return.stock_code, rsp->InstrumentID, sizeof(order_return.stock_code));
    order_return.direction = p_req->po.direction;
    order_return.open_close = p_req->po.open_close;
    order_return.speculator = p_req->po.speculator;
    order_return.volume = p_req->po.volume;
    order_return.limit_price = p_req->po.limit_price;

    order_return.volume_remain = p_req->volume_remain;
}

void StructConvertor::OrderReturn(const CSgitFtdcTradeField *rsp, const SgitOrderInfo *p_req, T_OrderReturn &order_return)
{
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = atoll(rsp->OrderSysID);
    order_return.serial_no = p_req->po.serial_no;

    strncpy(order_return.stock_code, rsp->InstrumentID, sizeof(order_return.stock_code));
    order_return.direction = p_req->po.direction;
    order_return.open_close = p_req->po.open_close;
    order_return.speculator = p_req->po.speculator;
    order_return.volume = p_req->po.volume;
    order_return.limit_price = p_req->po.limit_price;

    order_return.volume_remain = p_req->volume_remain;
}

void StructConvertor::TradeReturn(const CSgitFtdcTradeField *rsp, const SgitOrderInfo *p_req, T_TradeReturn &trade_return)
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
    trade_return.serial_no = p_req->po.serial_no;

    strncpy(trade_return.stock_code, rsp->InstrumentID, sizeof(trade_return.stock_code));
    trade_return.direction = p_req->po.direction;
    trade_return.open_close = p_req->po.open_close;
}
