//  TODO: wangying x1 status: done

#ifndef FIELD_CONVERT_H_
#define FIELD_CONVERT_H_

#include "trade_data_type.h"
#include "X1FtdcTraderApi.h"
#include <string>
#include <iomanip>
#include <string.h>

class X1FieldConvert
{
public:
    // 转换成X1字段取值
    inline static int GetX1BuySell(const char direction);
    inline static int GetX1OCFlag(const char open_close);
    inline static int GetX1HedgeType(const char hedge_flag);
    inline static int GetX1PriceType(const char price_type);
    inline static int GetX1OrderProperty(const char order_type);
    inline static const char * ExchCodeToExchName(const char cExchCode);

    // 从X1转换会MY协议取值
    inline static char EntrustStatusTrans(short OrderStatus);
    inline static char EntrustTbFlagTrans(int HedgeFlag);
    inline static char PriceTypeTrans(char PriceType);
    inline static char GetMYBuySell(short direction);
    inline static char GetMYOpenClose(int open_close);

    // 格式化
    inline static void SysOrderIDToX1Format(const std::string &my_orderid, char * const &xspeed_orderid);

};

int X1FieldConvert::GetX1BuySell(const char direction)
{
    if (direction == MY_TNL_D_BUY)
    {
        return X1_FTDC_SPD_BUY;
    }
    else
    {
        return X1_FTDC_SPD_SELL;
    }
}

int X1FieldConvert::GetX1OCFlag(const char open_close)
{
    switch (open_close)
    {
        case MY_TNL_D_OPEN:
            return X1_FTDC_SPD_OPEN;

        case MY_TNL_D_CLOSE:
            return X1_FTDC_SPD_CLOSE;

        case MY_TNL_D_CLOSEYESTERDAY:
            return X1_FTDC_SPD_CLOSE;

        case MY_TNL_D_CLOSETODAY:
            return X1_FTDC_SPD_CLOSETODAY;
    }

    return -1;
}

int X1FieldConvert::GetX1HedgeType(const char hedge_flag)
{
    if (hedge_flag == MY_TNL_HF_SPECULATION)
    {
        return X1_FTDC_SPD_SPECULATOR;
    }
    else if (hedge_flag == MY_TNL_HF_ARBITRAGE)
    {
        return X1_FTDC_SPD_ARBITRAGE;
    }
    else if (hedge_flag == MY_TNL_HF_HEDGE)
    {
        return X1_FTDC_SPD_HEDGE;
    }

    return -1;
}

int X1FieldConvert::GetX1PriceType(const char price_type)
{
    if (price_type == MY_TNL_OPT_ANY_PRICE)
    {
        return X1_FTDC_MKORDER;
    }
    return X1_FTDC_LIMITORDER;
}

int X1FieldConvert::GetX1OrderProperty(const char order_property)
{
    if (order_property == MY_TNL_HF_NORMAL)
    {
        return X1_FTDC_SP_NON;
    }
    else if (order_property == MY_TNL_HF_FAK)
    {
        return X1_FTDC_SP_FAK;
    }
    else if(order_property == MY_TNL_HF_FOK)
    {
        return X1_FTDC_SP_FOK;
    }

    return X1_FTDC_SP_NON;
}

const char * X1FieldConvert::ExchCodeToExchName(const char cExchCode)
{
    if (cExchCode == MY_TNL_EC_SHFE)
    {
        return X1_FTDC_EXCHANGE_SHFE;
    }
    else if (cExchCode == MY_TNL_EC_DCE)
    {
        return X1_FTDC_EXCHANGE_DCE;
    }
    else if (cExchCode == MY_TNL_EC_CZCE)
    {
        return X1_FTDC_EXCHANGE_CZCE;
    }
    else if (cExchCode == MY_TNL_EC_CFFEX)
    {
        return X1_FTDC_EXCHANGE_CFFEX;
    }
    else
    {
        return " ";
    }
}

//将X1的委托状态转换为统一的委托状态
char X1FieldConvert::EntrustStatusTrans(short OrderStatus)
{
    switch (OrderStatus)
    {
        //撤单成功 -->  已经撤销
        case X1_FTDC_SPD_CANCELED:        
        case X1_FTDC_SPD_PARTIAL_CANCELED:
            return MY_TNL_OS_WITHDRAWED;

        //全部成交 --> 全部成交
        case X1_FTDC_SPD_FILLED:
            return MY_TNL_OS_COMPLETED;

        //部分成交还在队列中 -->  部分成交
        //成交成功    --> 部分成交
        case X1_FTDC_SPD_PARTIAL:        
            return MY_TNL_OS_PARTIALCOM;

        //未成交还在队列中  -->  已经报入
        //未成交不在队列中  -->  已经报入
        case X1_FTDC_SPD_IN_QUEUE:
        case X1_FTDC_SPD_IN_CANCELING:
        case X1_FTDC_SPD_PLACED:
            return MY_TNL_OS_REPORDED;

        //错误、废单错误 -->  场内拒绝
        case X1_FTDC_SPD_ERROR:
            return MY_TNL_OS_ERROR;

        //报单的初始状态，表示单子刚刚开始，尚未报到柜台 --> 等待发出
        //柜台已接收，但尚未到交易所   --> 等待发出        
        case X1_FTDC_SPD_TRIGGERED:
            return MY_TNL_OS_REPORDED;
    }

    return MY_TNL_OS_ERROR;
}

//将X1的投机、套保标志转换为统一的委托状态
char X1FieldConvert::EntrustTbFlagTrans(int HedgeFlag)
{
    switch (HedgeFlag)
    {

        case X1_FTDC_SPD_HEDGE: //套保
            return MY_TNL_HF_HEDGE;

        case X1_FTDC_SPD_SPECULATOR: //投机
            return MY_TNL_HF_SPECULATION;

        case X1_FTDC_SPD_ARBITRAGE: //套利
            return MY_TNL_HF_ARBITRAGE;

        default:
            return -1;
    }
}

char X1FieldConvert::PriceTypeTrans(char PriceType)
{
    switch (PriceType)
    {
        case X1_FTDC_MKORDER: ///任意价
            return MY_TNL_OPT_ANY_PRICE;

        case X1_FTDC_LIMITORDER: ///限价
            return MY_TNL_OPT_LIMIT_PRICE;

        default:
            return 'n';
    }
}

char X1FieldConvert::GetMYBuySell(short direction)
{
    if (direction == X1_FTDC_SPD_BUY)
        return MY_TNL_D_BUY;
    else
        return MY_TNL_D_SELL;
}

char X1FieldConvert::GetMYOpenClose(int oc)
{
    if (oc == X1_FTDC_SPD_OPEN)
        return MY_TNL_D_OPEN;
    else
        return MY_TNL_D_CLOSE;
}

// ctp对接中金所/上期/大连，不足12位时，前面补空格，达到12位
void X1FieldConvert::SysOrderIDToX1Format(const std::string &my_orderid, char * const &xspeed_orderid)
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
