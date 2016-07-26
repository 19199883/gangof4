#pragma once

#include "trade_data_type.h"
#include "my_trade_tunnel_data_type.h"
#include "SecurityFtdcTraderApi.h"
#include <string>
#include <iomanip>
#include <string.h>

class LTSFieldConvert
{
public:
    // 转换成LTS字段取值
    inline static char GetLTSOCFlag(const char exch_code, const char open_close);
    inline static char GetLTSHedgeType(const char hedge_flag);
    inline static char GetLTSPriceType(const char price_type);
    inline static char GetLTSTimeCondition(const char price_type);
    inline static const char * ExchCodeToExchName(const char cExchCode);

    // 从LTS转换会MY协议取值
    inline static char EntrustStatusTrans(char OrderSubmitStatus, char OrderStatus);
    inline static char EntrustTbFlagTrans(char HedgeFlag);
    inline static char PriceTypeTrans(char PriceType);
    inline static char ExchNameToExchCode(const char * ex_name);

    inline static long GetEntrustNo(const char *order_sys_id);

    // 格式化
    inline static void SysOrderIDToLTSFormat(EntrustNoType entrust_no, char * const &LTS_orderid);
};

char LTSFieldConvert::GetLTSOCFlag(const char exch_code, const char open_close)
{
    if ((exch_code == MY_TNL_EC_SHFE) &&
        ((open_close == MY_TNL_D_CLOSE) ||
            (open_close == MY_TNL_D_CLOSETODAY)))
    {
        return SECURITY_FTDC_OF_CloseToday;
    }
    else if ((exch_code == MY_TNL_EC_SHFE)
        && (open_close == MY_TNL_D_CLOSEYESTERDAY))
    {
        return SECURITY_FTDC_OF_CloseYesterday;
    }

    return open_close;
}

char LTSFieldConvert::GetLTSHedgeType(const char hedge_flag)
{
    if (hedge_flag == MY_TNL_HF_SPECULATION)
    {
        return SECURITY_FTDC_HF_Speculation;
    }
    else if (hedge_flag == MY_TNL_HF_ARBITRAGE)
    {
        return SECURITY_FTDC_HF_Hedge;
    }
    else if (hedge_flag == MY_TNL_HF_HEDGE)
    {
        return SECURITY_FTDC_HF_Hedge;
    }

    return SECURITY_FTDC_HF_Speculation;
}

char LTSFieldConvert::GetLTSPriceType(const char price_type)
{
    if (price_type == MY_TNL_OPT_ANY_PRICE)
    {
        return SECURITY_FTDC_OPT_AnyPrice;
    }
    return SECURITY_FTDC_OPT_LimitPrice;
}

char LTSFieldConvert::GetLTSTimeCondition(const char order_type)
{
    if (order_type == MY_TNL_HF_FAK || order_type == MY_TNL_HF_FOK)
    {
        return SECURITY_FTDC_TC_IOC;
    }
    return SECURITY_FTDC_TC_GFD;
}

const char * LTSFieldConvert::ExchCodeToExchName(const char cExchCode)
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
    else if (cExchCode == MY_TNL_EC_SZEX)
    {
        return MY_TNL_EXID_SZEX;
    }
    else if (cExchCode == MY_TNL_EC_SHEX)
    {
        return MY_TNL_EXID_SHEX;
    }
    else
    {
        return "";
    }
}

char LTSFieldConvert::ExchNameToExchCode(const char * ex_name)
{
    if (strcmp(ex_name, "SZE") == 0)
        return MY_TNL_EC_SZEX;

    if (strcmp(ex_name, "SSE") == 0)
        return MY_TNL_EC_SHEX;

    if (strcmp(ex_name, "CFFEX") == 0)
        return MY_TNL_EC_CFFEX;

    if (strcmp(ex_name, "SHFE") == 0)
        return MY_TNL_EC_SHFE;

    if (strcmp(ex_name, "DCE") == 0)
        return MY_TNL_EC_DCE;

    if (strcmp(ex_name, "CZCE") == 0)
        return MY_TNL_EC_CZCE;

    return ' ';
}

long LTSFieldConvert::GetEntrustNo(const char *order_sys_id)
{
    if (order_sys_id == NULL)
    {
        return 0;
    }

    long en_no = atol(order_sys_id);

    // null:     OrderSysID=
    // sz stock: OrderSysID=N1PK9225399381
    // sh stock: OrderSysID=005001067232769
    if (en_no == 0 && strlen(order_sys_id) > 5)
    {
        en_no = atol(order_sys_id + 4);
    }

    return en_no;
}

//将LTS的委托状态转换为统一的委托状态
char LTSFieldConvert::EntrustStatusTrans(char OrderSubmitStatus, char OrderStatus)
{
    if (OrderSubmitStatus == SECURITY_FTDC_OSS_InsertSubmitted
        || OrderSubmitStatus == SECURITY_FTDC_OSS_CancelSubmitted
        || OrderSubmitStatus == SECURITY_FTDC_OSS_ModifySubmitted
        || OrderSubmitStatus == SECURITY_FTDC_OSS_Accepted)
    {
        switch (OrderStatus)
        {
            //全部成交 --> 全部成交
            case SECURITY_FTDC_OST_AllTraded:
                return MY_TNL_OS_COMPLETED;

                //部分成交还在队列中 -->  全部成交
            case SECURITY_FTDC_OST_PartTradedQueueing:
                return MY_TNL_OS_PARTIALCOM;

                //部分成交不在队列中 -->  部成部撤
            case SECURITY_FTDC_OST_PartTradedNotQueueing:
                return MY_TNL_OS_WITHDRAWED;

                //未成交还在队列中  -->  已经报入
            case SECURITY_FTDC_OST_NoTradeQueueing:
                return MY_TNL_OS_REPORDED;

                //未成交不在队列中  -->  已经报入
            case SECURITY_FTDC_OST_NoTradeNotQueueing:
                return MY_TNL_OS_REPORDED;

                //撤单 -->  已经撤销
            case SECURITY_FTDC_OST_Canceled:
                return MY_TNL_OS_WITHDRAWED;

            default:
                return MY_TNL_OS_REPORDED;
        }
    }
    else if (OrderSubmitStatus == SECURITY_FTDC_OSS_InsertRejected)
    {
        switch (OrderStatus)
        {
            case SECURITY_FTDC_OST_Canceled:
                return MY_TNL_OS_WITHDRAWED;
            default:
                //错误委托
                return MY_TNL_OS_ERROR;
        }
    }

    //错误委托
    return MY_TNL_OS_ERROR;
}

//将LTS的投机、套保标志转换为统一的委托状态
char LTSFieldConvert::EntrustTbFlagTrans(char HedgeFlag)
{
    switch (HedgeFlag)
    {
        //套保
        case SECURITY_FTDC_HF_Hedge:
            return MY_TNL_HF_HEDGE;
            //投机
        case SECURITY_FTDC_HF_Speculation:
            return MY_TNL_HF_SPECULATION;
        default:
            return MY_TNL_HF_SPECULATION;
    }
}

char LTSFieldConvert::PriceTypeTrans(char PriceType)
{
    switch (PriceType)
    {
        case SECURITY_FTDC_OPT_AnyPrice: ///任意价
            return MY_TNL_OPT_ANY_PRICE;
        case SECURITY_FTDC_OPT_LimitPrice: ///限价
            return MY_TNL_OPT_LIMIT_PRICE;
        case SECURITY_FTDC_OPT_BestPrice: ///最优价
            return MY_TNL_OPT_BEST_PRICE;
        default:
            return 'n';
    }
}

// LTS对接中金所/上期/大连，不足12位时，前面补空格，达到12位
void LTSFieldConvert::SysOrderIDToLTSFormat(EntrustNoType entrust_no, char * const &LTS_orderid)
{
    char my_orderid[64];
    sprintf(my_orderid, "%ld", entrust_no);
    std::size_t sysid_len = strlen(my_orderid);
    std::size_t start_pos = 0;
    if (sysid_len < 12)
    {
        start_pos = 12 - sysid_len;
        memset(LTS_orderid, ' ', start_pos);
    }

    memcpy(LTS_orderid + start_pos, my_orderid, sysid_len + 1);
}
