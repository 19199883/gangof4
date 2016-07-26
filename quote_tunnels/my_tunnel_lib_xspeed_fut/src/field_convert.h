#ifndef FIELD_CONVERT_H_
#define FIELD_CONVERT_H_

#include "trade_data_type.h"
#include "DFITCTraderApi.h"
#include <string>
#include <iomanip>
#include <string.h>

class XSpeedFieldConvert
{
public:
    // 转换成XSpeed字段取值
    inline static int GetXSpeedBuySell(const char direction);
    inline static int GetXSpeedOCFlag(const char open_close);
    inline static int GetXSpeedHedgeType(const char hedge_flag);
    inline static int GetXSpeedPriceType(const char price_type);
    inline static int GetXSpeedOrderProperty(const char order_type);
    inline static const char * ExchCodeToExchName(const char cExchCode);

    // 从XSpeed转换会MY协议取值
    inline static char EntrustStatusTrans(short OrderStatus);
    inline static char EntrustTbFlagTrans(int HedgeFlag);
    inline static char PriceTypeTrans(char PriceType);
    inline static char GetMYBuySell(short direction);
    inline static char GetMYOpenClose(int open_close);

    // 格式化
    inline static void SysOrderIDToXSpeedFormat(const std::string &my_orderid, char * const &xspeed_orderid);

};

int XSpeedFieldConvert::GetXSpeedBuySell(const char direction)
{
    if (direction == MY_TNL_D_BUY)
    {
        return DFITC_SPD_BUY;
    }
    else
    {
        return DFITC_SPD_SELL;
    }
}

int XSpeedFieldConvert::GetXSpeedOCFlag(const char open_close)
{
    switch (open_close)
    {
        case MY_TNL_D_OPEN:
            return DFITC_SPD_OPEN;

        case MY_TNL_D_CLOSE:
            return DFITC_SPD_CLOSE;

        case MY_TNL_D_CLOSEYESTERDAY:
            return DFITC_SPD_CLOSE;

        case MY_TNL_D_CLOSETODAY:
            return DFITC_SPD_CLOSETODAY;
    }

    return DFITC_SPD_OPEN;
}

int XSpeedFieldConvert::GetXSpeedHedgeType(const char hedge_flag)
{
    if (hedge_flag == MY_TNL_HF_SPECULATION)
    {
        return DFITC_SPD_SPECULATOR;
    }
    else if (hedge_flag == MY_TNL_HF_ARBITRAGE)
    {
        return DFITC_SPD_ARBITRAGE;
    }
    else if (hedge_flag == MY_TNL_HF_HEDGE)
    {
        return DFITC_SPD_HEDGE;
    }

    return DFITC_SPD_SPECULATOR;
}

int XSpeedFieldConvert::GetXSpeedPriceType(const char price_type)
{
    if (price_type == MY_TNL_OPT_ANY_PRICE)
    {
        return DFITC_MKORDER;
    }
    return DFITC_LIMITORDER;
}

int XSpeedFieldConvert::GetXSpeedOrderProperty(const char order_property)
{
    if (order_property == MY_TNL_HF_NORMAL)
    {
        return DFITC_SP_NON;
    }
    else if (order_property == MY_TNL_HF_FAK)
    {
        return DFITC_SP_FAK;
    }
    else if(order_property == MY_TNL_HF_FOK)
    {
        return DFITC_SP_FOK;
    }

    return DFITC_SP_NON;
}

const char * XSpeedFieldConvert::ExchCodeToExchName(const char cExchCode)
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
        return " ";
    }
}

//将XSpeed的委托状态转换为统一的委托状态
char XSpeedFieldConvert::EntrustStatusTrans(short OrderStatus)
{
    switch (OrderStatus)
    {
        //撤单成功 -->  已经撤销
        case DFITC_SPD_CANCELED:
        case DFITC_SPD_SUCCESS_CANCELED:
        case DFITC_SPD_PARTIAL_CANCELED:
            return MY_TNL_OS_WITHDRAWED;

        //全部成交 --> 全部成交
        case DFITC_SPD_FILLED:
            return MY_TNL_OS_COMPLETED;

        //部分成交还在队列中 -->  部分成交
        //成交成功    --> 部分成交
        case DFITC_SPD_PARTIAL:
        case DFITC_SPD_SUCCESS_FILLED:
            return MY_TNL_OS_PARTIALCOM;

        //未成交还在队列中  -->  已经报入
        //未成交不在队列中  -->  已经报入
        case DFITC_SPD_IN_QUEUE:
        case DFITC_SPD_IN_CANCELING:
        case DFITC_SPD_PLACED:
            return MY_TNL_OS_REPORDED;

        //错误、废单错误 -->  场内拒绝
        case DFITC_SPD_ERROR:
            return MY_TNL_OS_ERROR;

        //报单的初始状态，表示单子刚刚开始，尚未报到柜台 --> 等待发出
        //柜台已接收，但尚未到交易所   --> 等待发出
        case DFITC_SPD_STARTED:
        case DFITC_SPD_TRIGGERED:
            return MY_TNL_OS_REPORDED;
    }

    return MY_TNL_OS_ERROR;
}

//将XSpeed的投机、套保标志转换为统一的委托状态
char XSpeedFieldConvert::EntrustTbFlagTrans(int HedgeFlag)
{
    switch (HedgeFlag)
    {

        case DFITC_SPD_HEDGE: //套保
            return MY_TNL_HF_HEDGE;

        case DFITC_SPD_SPECULATOR: //投机
            return MY_TNL_HF_SPECULATION;

        case DFITC_SPD_ARBITRAGE: //套利
            return MY_TNL_HF_ARBITRAGE;

        default:
            return MY_TNL_HF_SPECULATION;
    }
}

char XSpeedFieldConvert::PriceTypeTrans(char PriceType)
{
    switch (PriceType)
    {
        case DFITC_MKORDER: ///任意价
            return MY_TNL_OPT_ANY_PRICE;

        case DFITC_LIMITORDER: ///限价
            return MY_TNL_OPT_LIMIT_PRICE;

        default:
            return 'n';
    }
}

char XSpeedFieldConvert::GetMYBuySell(short direction)
{
    if (direction == DFITC_SPD_BUY)
        return MY_TNL_D_BUY;
    else
        return MY_TNL_D_SELL;
}

char XSpeedFieldConvert::GetMYOpenClose(int oc)
{
    if (oc == DFITC_SPD_OPEN)
        return MY_TNL_D_OPEN;
    else
        return MY_TNL_D_CLOSE;
}

// ctp对接中金所/上期/大连，不足12位时，前面补空格，达到12位
void XSpeedFieldConvert::SysOrderIDToXSpeedFormat(const std::string &my_orderid, char * const &xspeed_orderid)
{
    std::size_t sysid_len = my_orderid.size();
    std::size_t start_pos = 0;
    if (sysid_len < 12)
    {
        start_pos = 12 - sysid_len;
        memset(xspeed_orderid, ' ', start_pos);
    }

    memcpy(xspeed_orderid + start_pos, my_orderid.c_str(), sysid_len + 1);
}

#endif // FIELD_CONVERT_H_
