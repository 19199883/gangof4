#ifndef  MD_TYPE_H
#define  MD_TYPE_H

#include "ac_xele_md_type.h"

///深度行情
struct DepthMarketDataField
{
    ///合约代码
    InstrumentIDType InstrumentID;
    ///最后修改时间
    TimeType         UpdateTime;
    ///最后修改毫秒
    MillisecType     UpdateMillisec;
    ///预留
    Reserved4Type    __reverved4_1;
    ///今开盘
    PriceType        OpenPrice;
    ///最高价
    PriceType        HighestPrice;
    ///最低价
    PriceType        LowestPrice;
    ///今收盘
    PriceType        ClosePrice;
    ///涨停板价
    PriceType        UpperLimitPrice;
    ///跌停板价
    PriceType        LowerLimitPrice;
    ///今结算
    PriceType        SettlementPrice;
    ///今虚实度
    RatioType        CurrDelta;
    ///预留
    Reserved4Type    __reverved4_2;
    ///最新价
    PriceType        LastPrice;
    ///数量
    VolumeType       Volume;
    ///成交金额
    MoneyType        Turnover;
    ///持仓量
    LargeVolumeType  OpenInterest;
    ///预留
    Reserved4Type    __reverved4_3;
    ///申买价一
    PriceType        BidPrice1;
    ///申买量一
    VolumeType       BidVolume1;
    ///申卖价一
    PriceType        AskPrice1;
    ///申卖量一
    VolumeType       AskVolume1;
    ///预留
    Reserved4Type    __reverved4_4;
    ///申买价二
    PriceType        BidPrice2;
    ///申买量二
    VolumeType       BidVolume2;
    ///申买价三
    PriceType        BidPrice3;
    ///申买量三
    VolumeType       BidVolume3;
    ///预留
    Reserved4Type    __reverved4_5;
    ///申卖价二
    PriceType        AskPrice2;
    ///申卖量二
    VolumeType       AskVolume2;
    ///申卖价三
    PriceType        AskPrice3;
    ///申卖量三
    VolumeType       AskVolume3;
    ///预留
    Reserved4Type    __reverved4_6;
    ///申买价四
    PriceType        BidPrice4;
    ///申买量四
    VolumeType       BidVolume4;
    ///申买价五
    PriceType        BidPrice5;
    ///申买量五
    VolumeType       BidVolume5;
    ///预留
    Reserved4Type    __reverved4_7;
    ///申卖价四
    PriceType        AskPrice4;
    ///申卖量四
    VolumeType       AskVolume4;
    ///申卖价五
    PriceType        AskPrice5;
    ///申卖量五
    VolumeType       AskVolume5;
} __attribute__( (packed) );

#endif
