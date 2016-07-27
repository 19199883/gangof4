#ifndef  MD_TYPE_H
#define  MD_TYPE_H

#include "ac_xele_md_type.h"

struct MarketData
{
    //行情类型
    MdType Md;
    char dataItem[86];
}__attribute__( (packed) );
///一档行情高频
struct LevelOneMarketDataField
{
    ///预留字段
    Reserved4Type    __reserved_1;
    ///合约代码
    InstrumentIDType InstrumentID;
    ///最后修改时间
    TimeType         UpdateTime;
    ///最后修改毫秒
    MillisecType     UpdateMillisec;
    ///数量
    VolumeType       Volume;
    ///最新价
    PriceType        LastPrice;
    ///成交金额
    MoneyType        Turnover;
    ///持仓量
    LargeVolumeType  OpenInterest;
    ///申买价
    PriceType        BidPrice;
    ///申卖价
    PriceType        AskPrice;
    ///申买量
    VolumeType       BidVolume;
    ///申卖量
    VolumeType       AskVolume;
} __attribute__( (packed) );

///一档行情低频
struct LowLevelOneMarketDataField
{
    ///预留字段
    Reserved4Type    __reserved_1;
    ///合约代码
    InstrumentIDType1 InstrumentID;
    ///最后修改时间
    TimeType         UpdateTime;
    //今开盘
    PriceType        OpenPrice;
    //最高价
    PriceType        HighestPrice;
    //最低价
    PriceType        LowestPrice;
    //今收盘
    PriceType        ClosePrice;
    //涨停板价
    PriceType        UpperLimitPrice;
    //跌停板价
    PriceType        LowerLimitPrice;
    //今结
    PriceType        SettlementPrice;
    //今需实度
    RatioType        CurrDelta;
} __attribute__( (packed) );
///快照
struct QuickStartMarketDataField
{
    ///预留字段
    Reserved4Type    __reserved_1;
    ///合约代码
    InstrumentIDType1 InstrumentID;
    ///最后修改时间
    TimeType         UpdateTime;
    //今开盘
    PriceType        OpenPrice;
    //最高价
    PriceType        HighestPrice;
    //最低价
    PriceType        LowestPrice;
    //今收盘
    PriceType        ClosePrice;
    //涨停板价
    PriceType        UpperLimitPrice;
    //跌停板价
    PriceType        LowerLimitPrice;
    //今结
    PriceType        SettlementPrice;
    //今需实度
    RatioType        CurrDelta;
} __attribute__( (packed) );
///无限深度行情
struct QueryMarketDataField
{
    ///预留字段
    Reserved4Type       __reserved_1;
    ///合约代码
    ExInstrumentIDType  InstrumentID;
    ///买卖方向
    DirectionType       Direction;
    ///最后修改时间
    TimeType            UpdateTime;
    ///最后修改毫秒
    MillisecType        UpdateMillisec;
    ///数量1
    VolumeType          Volume1;
    ///价格1
    PriceType           Price1;
    ///数量2
    VolumeType          Volume2;
    ///价格2
    PriceType           Price2;
    ///数量3
    VolumeType          Volume3;
    ///价格3
    PriceType           Price3;
    ///数量4
    VolumeType          Volume4;
    ///价格4
    PriceType           Price4;
    ///数量5
    VolumeType          Volume5;
    ///价格5
    PriceType           Price5;
} __attribute__( (packed) );

#endif
