#ifndef FIELD_CONVERT_H_
#define FIELD_CONVERT_H_

#include <string>
#include <iomanip>
#include <string.h>

#include "trade_data_type.h"
#include "USTPFtdcUserApiDataType.h"

class FEMASFieldConvert
{
public:
    // 转换成FEMAS字段取值
    inline static char GetFEMASHedgeType(const char hedge_flag);
    inline static char GetFEMASPriceType(const char price_type);
    inline static char GetFEMASTimeCondition(const char price_type);
    inline static char GetFEMASOCFlag(const char exch_code, const char open_close);
    inline static void SysOrderIDToCTPFormat(EntrustNoType my_orderid, char * const &ctp_orderid);

    // 从FEMAS转换MY协议取值
    inline static char EntrustStatusTrans(char OrderStatus);
    inline static char EntrustTbFlagTrans(char HedgeFlag);
    inline static char PriceTypeTrans(char PriceType);
};

/*************************************************************
 FEMAS
 **************************************************************/
char FEMASFieldConvert::GetFEMASHedgeType(const char hedge_flag)
{
    if (hedge_flag == MY_TNL_HF_SPECULATION)
    {
        return USTP_FTDC_CHF_Speculation;
    }
    else if (hedge_flag == MY_TNL_HF_ARBITRAGE)
    {
        return USTP_FTDC_CHF_Arbitrage;
    }
    else if (hedge_flag == MY_TNL_HF_HEDGE)
    {
        return USTP_FTDC_CHF_Hedge;
    }

    return USTP_FTDC_CHF_Speculation;
}

char FEMASFieldConvert::GetFEMASPriceType(const char price_type)
{
    if (price_type == MY_TNL_OPT_ANY_PRICE)
    {
        return USTP_FTDC_OPT_AnyPrice;
    }
    return USTP_FTDC_OPT_LimitPrice;
}

char FEMASFieldConvert::GetFEMASTimeCondition(const char order_type)
{
    if (order_type == MY_TNL_HF_FAK || order_type == MY_TNL_HF_FOK)
    {
        return USTP_FTDC_TC_IOC;
    }
    return USTP_FTDC_TC_GFD;
}

char FEMASFieldConvert::GetFEMASOCFlag(const char exch_code, const char open_close)
{
    if ((exch_code == MY_TNL_EC_SHFE) &&
        ((open_close == MY_TNL_D_CLOSE) ||
            (open_close == MY_TNL_D_CLOSETODAY)))
    {
        return USTP_FTDC_OF_CloseToday;
    }
    else if ((exch_code == MY_TNL_EC_SHFE)
        && (open_close == MY_TNL_D_CLOSEYESTERDAY))
    {
        return USTP_FTDC_OF_Close;
    }

    return open_close;
}

//将FEMAS的委托状态转换为统一的委托状态
char FEMASFieldConvert::EntrustStatusTrans(char OrderStatus)
{
    switch (OrderStatus)
    {
        //全部成交 --> 全部成交
        case USTP_FTDC_OS_AllTraded:
            return MY_TNL_OS_COMPLETED;

            //部分成交还在队列中 -->  部分成交
        case USTP_FTDC_OS_PartTradedQueueing:
            return MY_TNL_OS_PARTIALCOM;

            //部分成交不在队列中 -->  部成部撤
        case USTP_FTDC_OS_PartTradedNotQueueing:
            return MY_TNL_OS_WITHDRAWED;

            //未成交还在队列中  -->  已经报入
        case USTP_FTDC_OS_NoTradeQueueing:
            return MY_TNL_OS_REPORDED;

            //未成交不在队列中  -->  已经报入
        case USTP_FTDC_OS_NoTradeNotQueueing:
            return MY_TNL_OS_REPORDED;

            //撤单 -->  已经撤销
        case USTP_FTDC_OS_Canceled:
            return MY_TNL_OS_WITHDRAWED;

        default:
            return MY_TNL_OS_ERROR;
    }

    //错误委托
    return MY_TNL_OS_ERROR;
}

//将CTP的投机、套保标志转换为统一的委托状态
char FEMASFieldConvert::EntrustTbFlagTrans(char HedgeFlag)
{
    switch (HedgeFlag)
    {
        case USTP_FTDC_CHF_Hedge:            //套保
            return MY_TNL_HF_HEDGE;

        case USTP_FTDC_CHF_Speculation:            //投机
            return MY_TNL_HF_SPECULATION;

        case USTP_FTDC_CHF_Arbitrage:            //套利
            return MY_TNL_HF_ARBITRAGE;

        default:
            return MY_TNL_HF_SPECULATION;
    }
}

char FEMASFieldConvert::PriceTypeTrans(char PriceType)
{
    switch (PriceType)
    {
        case USTP_FTDC_OPT_AnyPrice: ///任意价
            return MY_TNL_OPT_ANY_PRICE;
        case USTP_FTDC_OPT_LimitPrice: ///限价
            return MY_TNL_OPT_LIMIT_PRICE;
        case USTP_FTDC_OPT_BestPrice: ///最优价
            return MY_TNL_OPT_BEST_PRICE;
        default:
            return 'n';
    }
}

void FEMASFieldConvert::SysOrderIDToCTPFormat(EntrustNoType entrust_no, char * const &ctp_orderid)
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

#endif // FIELD_CONVERT_H_
