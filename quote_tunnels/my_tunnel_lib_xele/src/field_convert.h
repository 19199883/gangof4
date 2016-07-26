#ifndef FIELD_CONVERT_H_
#define FIELD_CONVERT_H_

#include <string>
#include <iomanip>
#include <string.h>

#include "trade_data_type.h"
#include "CXeleFtdcUserApiDataType.h"

class XeleFieldConvert
{
public:
    // 转换成api字段取值
    inline static char GetApiHedgeType(const char hedge_flag);
    inline static char GetApiPriceType(const char price_type);
    inline static char GetApiTimeCondition(const char price_type);
    inline static void SysOrderIDToCounterFormat(const std::string &my_orderid, char * const &ctp_orderid);

    // 从api转换MY协议取值
    inline static char GetMYEntrustStatus(char OrderStatus);
    inline static char GetMYHedgeType(char HedgeFlag);
    inline static char GetMYPriceType(char PriceType);
};

/*************************************************************
 FEMAS
 **************************************************************/
char XeleFieldConvert::GetApiHedgeType(const char hedge_flag)
{
    if (hedge_flag == MY_TNL_HF_SPECULATION)
    {
        return XELE_FTDC_HF_Speculation;
    }
    else if (hedge_flag == MY_TNL_HF_HEDGE)
    {
        return XELE_FTDC_HF_Arbitrage;
    }
    else if (hedge_flag == MY_TNL_HF_ARBITRAGE)
    {
        return XELE_FTDC_HF_Hedge;
    }

    return XELE_FTDC_HF_Speculation;
}

char XeleFieldConvert::GetApiPriceType(const char price_type)
{
    if (price_type == MY_TNL_OPT_ANY_PRICE)
    {
        return XELE_FTDC_OPT_AnyPrice;
    }
    return XELE_FTDC_OPT_LimitPrice;
}

char XeleFieldConvert::GetApiTimeCondition(const char order_type)
{
    if (order_type == MY_TNL_HF_FAK || order_type == MY_TNL_HF_FOK)
    {
        return XELE_FTDC_TC_IOC;
    }
    return XELE_FTDC_TC_GFD;
}

//将FEMAS的委托状态转换为统一的委托状态
char XeleFieldConvert::GetMYEntrustStatus(char OrderStatus)
{
    switch (OrderStatus)
    {
        //全部成交 --> 全部成交
        case XELE_FTDC_OST_AllTraded:
            return MY_TNL_OS_COMPLETED;

            //部分成交还在队列中 -->  部分成交
        case XELE_FTDC_OST_PartTradedQueueing:
            return MY_TNL_OS_PARTIALCOM;

            //部分成交不在队列中 -->  部成部撤
        case XELE_FTDC_OST_PartTradedNotQueueing:
            return MY_TNL_OS_WITHDRAWED;

            //未成交还在队列中  -->  已经报入
        case XELE_FTDC_OST_NoTradeQueueing:
            return MY_TNL_OS_REPORDED;

            //未成交不在队列中  -->  已经报入
        case XELE_FTDC_OST_NoTradeNotQueueing:
            return MY_TNL_OS_REPORDED;

            //撤单 -->  已经撤销
        case XELE_FTDC_OST_Canceled:
            return MY_TNL_OS_WITHDRAWED;

        default:
            return MY_TNL_OS_ERROR;
    }

    //错误委托
    return MY_TNL_OS_ERROR;
}

//将CTP的投机、套保标志转换为统一的委托状态
char XeleFieldConvert::GetMYHedgeType(char HedgeFlag)
{
    switch (HedgeFlag)
    {
        case XELE_FTDC_HF_Hedge:            //套保
            return MY_TNL_HF_ARBITRAGE;

        case XELE_FTDC_HF_Speculation:            //投机
            return MY_TNL_HF_SPECULATION;

        case XELE_FTDC_HF_Arbitrage:            //套利
            return MY_TNL_HF_HEDGE;

        default:
            return MY_TNL_HF_SPECULATION;
    }
}

char XeleFieldConvert::GetMYPriceType(char PriceType)
{
    switch (PriceType)
    {
        case XELE_FTDC_OPT_AnyPrice: ///任意价
            return MY_TNL_OPT_ANY_PRICE;
        case XELE_FTDC_OPT_LimitPrice: ///限价
            return MY_TNL_OPT_LIMIT_PRICE;
        case XELE_FTDC_OPT_BestPrice: ///最优价
            return MY_TNL_OPT_BEST_PRICE;
        default:
            return 'n';
    }
}

void XeleFieldConvert::SysOrderIDToCounterFormat(const std::string &my_orderid, char * const &ctp_orderid)
{
    std::size_t sysid_len = my_orderid.size();
    std::size_t start_pos = 0;
    if (sysid_len < 12)
    {
        start_pos = 12 - sysid_len;
        memset(ctp_orderid, ' ', start_pos);
    }

    memcpy(ctp_orderid + start_pos, my_orderid.c_str(), sysid_len + 1);
}

#endif // FIELD_CONVERT_H_
