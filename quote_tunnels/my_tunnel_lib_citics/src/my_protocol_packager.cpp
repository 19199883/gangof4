#include "my_protocol_packager.h"

#include <sstream>
#include <stdlib.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "hs_field_convert.h"
#include "config_data.h"
//#include "dce_trade_context.h"
#include "hs_trade_context.h"

void CITTCsUFTPacker::OrderRespond(const int error_no, const long serial_no,
		long entrust_no,const char internal_status, T_OrderRespond &order_respond,char ex)
{
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = entrust_no;
    order_respond.entrust_status = internal_status;
    order_respond.serial_no = serial_no;
    order_respond.error_no = error_no;
    order_respond.exchange_type = ex;
}

void CITTCsUFTPacker::CancelRespond(const int error_no, const long serial_no,
		long entrust_no, const char inter_status,T_CancelRespond &cancle_order)
{
    memset(&cancle_order, 0, sizeof(cancle_order));
    cancle_order.entrust_no = entrust_no;
    cancle_order.serial_no = serial_no;
    cancle_order.error_no = error_no;
    cancle_order.entrust_status = inter_status;
}


void CITTCsUFTPacker::OrderReturn(const T_InternalOrder *req,T_OrderReturn &order_return,char ex)
{
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = req->entrust_no;
    order_return.entrust_status = req->status;
    strcpy(order_return.stock_code, req->stock_code);
    order_return.volume_remain = req->volume - req->acc_vol;
    order_return.serial_no = req->serial_no;
    order_return.direction = req->direction;
    order_return.volume = req->volume;
    order_return.limit_price = req->limit_price;
    order_return.exchange_type = ex;
}

void CITTCsUFTPacker::TradeReturn(const T_InternalOrder *rsp, T_TradeReturn &trade_return,char ex)
{
    memset(&trade_return, 0, sizeof(trade_return));
    trade_return.business_price = rsp->last_px;
    trade_return.business_volume = rsp->last_vol;
    trade_return.entrust_no = rsp->entrust_no;
    trade_return.serial_no = rsp->serial_no;

    strcpy(trade_return.stock_code, rsp->stock_code);
    trade_return.direction = rsp->direction;
    trade_return.exchange_type = ex;
}
