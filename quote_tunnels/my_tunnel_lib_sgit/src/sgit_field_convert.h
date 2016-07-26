#pragma once

#include "trade_data_type.h"
#include "SgitFtdcTraderApi.h"
#include <string>
#include <iomanip>
#include <string.h>

class FieldConvert
{
public:
    // 转换成Api字段取值
    inline static char GetApiOCFlag(const char exch_code, const char open_close);
    inline static char GetApiHedgeFlag(const char hedge_flag);
    inline static char GetApiPriceType(const char price_type);
    inline static char GetApiTimeCondition(const char order_type);
    inline static char GetApiExID(const char ex_code);

    // 从Api转换会MY协议取值
    inline static char GetMYEntrustStatus(char OrderSubmitStatus, char OrderStatus);
    inline static char GetMYHedgeFlag(char HedgeFlag);
    inline static char GetMYPriceType(char PriceType);

    // 格式化
    inline static void FormatSysOrderID(const std::string &my_orderid, char * const &sys_orderid);

};

char FieldConvert::GetApiOCFlag(const char exch_code, const char open_close)
{
    if ((exch_code == MY_TNL_EC_SHFE) &&
        ((open_close == MY_TNL_D_CLOSE) ||
            (open_close == MY_TNL_D_CLOSETODAY)))
    {
        return Sgit_FTDC_OF_CloseToday;
    }
    else if ((exch_code == MY_TNL_EC_SHFE)
        && (open_close == MY_TNL_D_CLOSEYESTERDAY))
    {
        return Sgit_FTDC_OF_CloseYesterday;
    }

    return open_close;
}

char FieldConvert::GetApiHedgeFlag(const char hedge_flag)
{
    if (hedge_flag == MY_TNL_HF_SPECULATION)
    {
        return Sgit_FTDC_HF_Speculation;
    }
    else if (hedge_flag == MY_TNL_HF_ARBITRAGE)
    {
        return Sgit_FTDC_HF_Arbitrage;
    }
    else if (hedge_flag == MY_TNL_HF_HEDGE)
    {
        return Sgit_FTDC_HF_Hedge;
    }

    return Sgit_FTDC_HF_Speculation;
}

char FieldConvert::GetApiPriceType(const char price_type)
{
    if (price_type == MY_TNL_OPT_ANY_PRICE)
    {
        return Sgit_FTDC_OPT_AnyPrice;
    }
    return Sgit_FTDC_OPT_LimitPrice;
}

char FieldConvert::GetApiTimeCondition(const char order_type)
{
    if (order_type == MY_TNL_HF_FAK || order_type == MY_TNL_HF_FOK)
    {
        return Sgit_FTDC_TC_IOC;
    }
    return Sgit_FTDC_TC_GFD;
}

char FieldConvert::GetApiExID(const char ex_code)
{
    if (ex_code == MY_TNL_EC_SHFE)
    {
        return Sgit_FTDC_EIDT_SHFE;
    }
    else if (ex_code == MY_TNL_EC_SHGOLD)
    {
        return Sgit_FTDC_EIDT_GOLD;
    }
    else if (ex_code == MY_TNL_EC_DCE)
    {
        return Sgit_FTDC_EIDT_DCE;
    }
    else if (ex_code == MY_TNL_EC_CZCE)
    {
        return Sgit_FTDC_EIDT_CZCE;
    }
    else if (ex_code == MY_TNL_EC_CFFEX)
    {
        return Sgit_FTDC_EIDT_CFFEX;
    }
    else
    {
        return ' ';
    }
}

//将api的委托状态转换为统一的委托状态
char FieldConvert::GetMYEntrustStatus(char OrderSubmitStatus, char OrderStatus)
{
    if (OrderSubmitStatus == Sgit_FTDC_OSS_InsertSubmitted
        || OrderSubmitStatus == Sgit_FTDC_OSS_CancelSubmitted
        || OrderSubmitStatus == Sgit_FTDC_OSS_ModifySubmitted
        || OrderSubmitStatus == Sgit_FTDC_OSS_Accepted)
    {
        switch (OrderStatus)
        {
            //全部成交 --> 全部成交
            case Sgit_FTDC_OST_AllTraded:
                return MY_TNL_OS_COMPLETED;

                //部分成交还在队列中 -->  全部成交
            case Sgit_FTDC_OST_PartTradedQueueing:
                return MY_TNL_OS_PARTIALCOM;

                //部分成交不在队列中 -->  部成部撤
            case Sgit_FTDC_OST_PartTradedNotQueueing:
                return MY_TNL_OS_WITHDRAWED;

                //未成交还在队列中  -->  已经报入
            case Sgit_FTDC_OST_NoTradeQueueing:
                return MY_TNL_OS_REPORDED;

                //未成交不在队列中  -->  已经报入
            case Sgit_FTDC_OST_NoTradeNotQueueing:
                return MY_TNL_OS_REPORDED;

                //撤单 -->  已经撤销
            case Sgit_FTDC_OST_Canceled:
                return MY_TNL_OS_WITHDRAWED;

            default:
                return MY_TNL_OS_REPORDED;
        }
    }
    else if (OrderSubmitStatus == Sgit_FTDC_OSS_InsertRejected)
    {
        switch (OrderStatus)
        {
            case Sgit_FTDC_OST_Canceled:
                return MY_TNL_OS_WITHDRAWED;
            default:
                //错误委托
                return MY_TNL_OS_ERROR;
        }
    }

    //错误委托
    return MY_TNL_OS_ERROR;
}

//将api的投机、套保标志转换为统一的委托状态
char FieldConvert::GetMYHedgeFlag(char HedgeFlag)
{
    switch (HedgeFlag)
    {
        //套保
        case Sgit_FTDC_ECIDT_Hedge:
            return MY_TNL_HF_HEDGE;

        //投机
        case Sgit_FTDC_ECIDT_Speculation:
            return MY_TNL_HF_SPECULATION;

        //套利 20130125 caodl 增加
        case Sgit_FTDC_ECIDT_Arbitrage:
            return MY_TNL_HF_ARBITRAGE;

        default:
            return MY_TNL_HF_SPECULATION;
    }
}

char FieldConvert::GetMYPriceType(char PriceType)
{
    switch (PriceType)
    {
        case Sgit_FTDC_OPT_AnyPrice: ///任意价
            return MY_TNL_OPT_ANY_PRICE;
        case Sgit_FTDC_OPT_LimitPrice: ///限价
            return MY_TNL_OPT_LIMIT_PRICE;
        case Sgit_FTDC_OPT_BestPrice: ///最优价
            return MY_TNL_OPT_BEST_PRICE;
        default:
            return 'n';
    }
}

// 对接中金所/上期/大连，不足12位时，前面补空格，达到12位
void FieldConvert::FormatSysOrderID(const std::string &my_orderid, char * const &sys_orderid)
{
    std::size_t sysid_len = my_orderid.size();
    std::size_t start_pos = 0;
    if (sysid_len < 12)
    {
        start_pos = 12 - sysid_len;
        memset(sys_orderid, ' ', start_pos);
    }

    memcpy(sys_orderid + start_pos, my_orderid.c_str(), sysid_len + 1);
}
