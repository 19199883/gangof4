#pragma once

#include "trade_data_type.h"
#include "my_trade_tunnel_data_type.h"
#include "ZeusingFtdcTraderApi.h"
#include <string>
#include <iomanip>
#include <string.h>

class DataFieldConvert
{
public:
    // 转换成api字段取值
    inline static char GetApiOCFlag(const char exch_code, const char open_close);
    inline static char GetApiHedgeType(const char hedge_flag);
    inline static char GetApiPriceType(const char price_type);
    inline static char GetApiTimeCondition(const char price_type);
    inline static const char * ExchCodeToExchName(const char cExchCode);

    // 从api转换会MY协议取值
    inline static char GetMYEntrustStatus(char OrderSubmitStatus, char OrderStatus);
    inline static char GetMYHedgeFlag(char HedgeFlag);
    inline static char GetMYPriceType(char PriceType);
    inline static char ExchNameToExchCode(const char * ex_name);

    // 格式化
    inline static void FormatOrderSysId(EntrustNoType entrust_no, char * const &order_sysid);

};

char DataFieldConvert::GetApiOCFlag(const char exch_code, const char open_close)
{
    if ((exch_code == MY_TNL_EC_SHFE) &&
        ((open_close == MY_TNL_D_CLOSE) ||
            (open_close == MY_TNL_D_CLOSETODAY)))
    {
        return ZEUSING_FTDC_OF_CloseToday;
    }
    else if ((exch_code == MY_TNL_EC_SHFE)
        && (open_close == MY_TNL_D_CLOSEYESTERDAY))
    {
        return ZEUSING_FTDC_OF_CloseYesterday;
    }

    return open_close;
}

char DataFieldConvert::GetApiHedgeType(const char hedge_flag)
{
    if (hedge_flag == MY_TNL_HF_SPECULATION)
    {
        return ZEUSING_FTDC_HF_Speculation;
    }
    else if (hedge_flag == MY_TNL_HF_ARBITRAGE)
    {
        return ZEUSING_FTDC_HF_Arbitrage;
    }
    else if (hedge_flag == MY_TNL_HF_HEDGE)
    {
        return ZEUSING_FTDC_HF_Hedge;
    }

    return ZEUSING_FTDC_HF_Speculation;
}

char DataFieldConvert::GetApiPriceType(const char price_type)
{
    if (price_type == MY_TNL_OPT_ANY_PRICE)
    {
        return ZEUSING_FTDC_OPT_AnyPrice;
    }
    return ZEUSING_FTDC_OPT_LimitPrice;
}

char DataFieldConvert::GetApiTimeCondition(const char order_type)
{
    if (order_type == MY_TNL_HF_FAK || order_type == MY_TNL_HF_FOK)
    {
        return ZEUSING_FTDC_TC_IOC;
    }
    return ZEUSING_FTDC_TC_GFD;
}

const char * DataFieldConvert::ExchCodeToExchName(const char cExchCode)
{
    if (cExchCode == MY_TNL_EC_SHFE)
    {
        return MY_TNL_EXID_SHFE;
    }
    else if (cExchCode == MY_TNL_EC_DCE)
    {
        return MY_TNL_EXID_DCE;
    }
    else if (cExchCode == MY_TNL_EC_CZCE)
    {
        return MY_TNL_EXID_CZCE;
    }
    else if (cExchCode == MY_TNL_EC_CFFEX)
    {
        return MY_TNL_EXID_CFFEX;
    }
    else
    {
        return "";
    }
}

char DataFieldConvert::ExchNameToExchCode(const char * ex_name)
{
    if (strcmp(ex_name, MY_TNL_EXID_CFFEX) == 0)
        return MY_TNL_EC_CFFEX;

    if (strcmp(ex_name, MY_TNL_EXID_SHFE) == 0)
        return MY_TNL_EC_SHFE;

    if (strcmp(ex_name, MY_TNL_EXID_DCE) == 0)
        return MY_TNL_EC_DCE;

    if (strcmp(ex_name, MY_TNL_EXID_CZCE) == 0)
        return MY_TNL_EC_CZCE;

    return ' ';
}

//将CTP的委托状态转换为统一的委托状态
char DataFieldConvert::GetMYEntrustStatus(char OrderSubmitStatus, char OrderStatus)
{
    if (OrderSubmitStatus == ZEUSING_FTDC_OSS_InsertSubmitted
        || OrderSubmitStatus == ZEUSING_FTDC_OSS_CancelSubmitted
        || OrderSubmitStatus == ZEUSING_FTDC_OSS_ModifySubmitted
        || OrderSubmitStatus == ZEUSING_FTDC_OSS_Accepted)
    {
        switch (OrderStatus)
        {
            //全部成交 --> 全部成交
            case ZEUSING_FTDC_OST_AllTraded:
                return MY_TNL_OS_COMPLETED;

                //部分成交还在队列中 -->  全部成交
            case ZEUSING_FTDC_OST_PartTradedQueueing:
                return MY_TNL_OS_PARTIALCOM;

                //部分成交不在队列中 -->  部成部撤
            case ZEUSING_FTDC_OST_PartTradedNotQueueing:
                return MY_TNL_OS_WITHDRAWED;

                //未成交还在队列中  -->  已经报入
            case ZEUSING_FTDC_OST_NoTradeQueueing:
                return MY_TNL_OS_REPORDED;

                //未成交不在队列中  -->  已经报入
            case ZEUSING_FTDC_OST_NoTradeNotQueueing:
                return MY_TNL_OS_REPORDED;

                //撤单 -->  已经撤销
            case ZEUSING_FTDC_OST_Canceled:
                return MY_TNL_OS_WITHDRAWED;

            default:
                return MY_TNL_OS_REPORDED;
        }
    }
    else if (OrderSubmitStatus == ZEUSING_FTDC_OSS_InsertRejected)
    {
        switch (OrderStatus)
        {
            case ZEUSING_FTDC_OST_Canceled:
                return MY_TNL_OS_WITHDRAWED;
            default:
                //错误委托
                return MY_TNL_OS_ERROR;
        }
    }

    //错误委托
    return MY_TNL_OS_ERROR;
}

//将CTP的投机、套保标志转换为统一的委托状态
char DataFieldConvert::GetMYHedgeFlag(char HedgeFlag)
{
    switch (HedgeFlag)
    {
        //套保
        case ZEUSING_FTDC_ECIDT_Hedge:
            return MY_TNL_HF_HEDGE;
            //投机
        case ZEUSING_FTDC_ECIDT_Speculation:
            return MY_TNL_HF_SPECULATION;
            //套利 20130125 caodl 增加
        case ZEUSING_FTDC_ECIDT_Arbitrage:
            return MY_TNL_HF_ARBITRAGE;
        default:
            return MY_TNL_HF_SPECULATION;
    }
}

char DataFieldConvert::GetMYPriceType(char PriceType)
{
    switch (PriceType)
    {
        case ZEUSING_FTDC_OPT_AnyPrice: ///任意价
            return MY_TNL_OPT_ANY_PRICE;
        case ZEUSING_FTDC_OPT_LimitPrice: ///限价
            return MY_TNL_OPT_LIMIT_PRICE;
        case ZEUSING_FTDC_OPT_BestPrice: ///最优价
            return MY_TNL_OPT_BEST_PRICE;
        default:
            return 'n';
    }
}

// 对接中金所/上期/大连，不足12位时，前面补空格，达到12位
void DataFieldConvert::FormatOrderSysId(EntrustNoType entrust_no, char * const &ctp_orderid)
{
    char my_orderid[64];
    sprintf(my_orderid, "%ld", entrust_no);
    std::size_t sysid_len = strlen(my_orderid);
    std::size_t start_pos = 0;
    if (sysid_len < 12)
    {
        start_pos = 12 - sysid_len;
        memset(ctp_orderid, ' ', start_pos);
    }

    memcpy(ctp_orderid + start_pos, my_orderid, sysid_len + 1);
}
