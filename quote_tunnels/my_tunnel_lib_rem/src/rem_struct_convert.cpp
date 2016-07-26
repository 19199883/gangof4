#include "rem_struct_convert.h"
#include "rem_field_convert.h"
#include <string.h>
#include <strings.h>

void RemStructConvert::OrderRespond(int err, long sn, long en_no, char entrust_status, T_OrderRespond& rsp)
{
    bzero(&rsp, sizeof(rsp));
    rsp.entrust_no = en_no;
    rsp.entrust_status = entrust_status;
    rsp.serial_no = sn;
    rsp.error_no = err;
}

void RemStructConvert::CancelRespond(int err, long sn, long en_no, T_CancelRespond& rsp)
{
    bzero(&rsp, sizeof(rsp));
    rsp.entrust_no = en_no;
    rsp.serial_no = sn;
    rsp.error_no = err;

    // 需要回报撤单状态，成功为已报，失败为拒绝
    rsp.entrust_status = MY_TNL_OS_REPORDED;
    if (err != 0) rsp.entrust_status = MY_TNL_OS_ERROR;
}

void RemStructConvert::OrderReturn(const RemReqInfo* req, T_OrderReturn& order_return)
{
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = req->entruct_no;
    order_return.entrust_status = req->order_status;
    order_return.serial_no = req->po.serial_no;

    strncpy(order_return.stock_code, req->po.stock_code, sizeof(order_return.stock_code));
    order_return.direction = req->po.direction;
    order_return.open_close = req->po.open_close;
    order_return.speculator = req->po.speculator;
    order_return.volume = req->po.volume;
    order_return.limit_price = req->po.limit_price;

    order_return.volume_remain = req->volume_remain;
}

void RemStructConvert::TradeReturn(const RemReqInfo *req, const EES_OrderExecutionField* match_info, T_TradeReturn &trade_return)
{
    memset(&trade_return, 0, sizeof(trade_return));

    trade_return.serial_no = req->po.serial_no;
    trade_return.entrust_no = req->entruct_no;
    trade_return.business_volume = match_info->m_Quantity;
    trade_return.business_price = match_info->m_Price;
    trade_return.business_no = match_info->m_ExecutionID;

    memcpy(trade_return.stock_code, req->po.stock_code, sizeof(trade_return.stock_code));
    trade_return.direction = req->po.direction;
    trade_return.open_close = req->po.open_close;
}

void RemStructConvert::ReqOrderInsert(const T_PlaceOrder* p, EES_ClientToken tok, const char *acc, EES_EnterOrderField& req)
{
    bzero(&req, sizeof(req));
    strncpy(req.m_Account, acc, sizeof(req.m_Account));                     ///< 用户代码
    req.m_Side = RemFieldConvert::GetRemSide(p->direction, p->open_close);  ///< 买卖方向
    req.m_Exchange = EES_ExchangeID_cffex;                                  ///< 交易所 (cffex)
    strncpy(req.m_Symbol, p->stock_code, sizeof(req.m_Symbol));             ///< 合约代码
    req.m_SecType = EES_SecType_fut;                                        ///< 交易品种
    req.m_Price = p->limit_price;                                           ///< 价格
    req.m_Qty = p->volume;                                                  ///< 数量
    req.m_ForceCloseReason = EES_ForceCloseType_not_force_close;            ///< 强平原因
    req.m_ClientOrderToken = tok;                                           ///< 整型，必须保证，这次比上次的值大，并不一定需要保证连续
    req.m_Tif = RemFieldConvert::GetRemCondition(p->order_type);            ///< 当需要下FAK/FOK报单时，需要设置为EES_OrderTif_IOC
    req.m_MinQty = 0;                                                       ///< 当需要下FAK/FOK报单时，该值=0：映射交易所的FAK-任意数量；
    if (p->order_type == MY_TNL_HF_FOK) req.m_MinQty = p->volume;
    //req.m_CustomField;                                                    ///< 用户自定义字段，8个字节。用户在下单时指定的值，将会在OnOrderAccept，OnQueryTradeOrder事件中返回
    //req.m_MarketSessionId;                                                ///< 交易所席位代码，从OnResponseQueryMarketSessionId获取合法值，如果填入0或者其他非法值，REM系统将自行决定送单的席位
    req.m_HedgeFlag = RemFieldConvert::GetRemHedgeFlag(p->speculator);      ///< 投机套利标志
}
