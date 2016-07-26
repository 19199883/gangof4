#if !defined(QUOTE_DATATYPE_ZEUSING_UDP_H_)
#define QUOTE_DATATYPE_ZEUSING_UDP_H_

#include <stdlib.h>
#include <stdint.h>

struct TFTDCHeader
{
    uint8_t Version; /**< 版本号	1	二进制无符号整数。目前版本为1*/
    uint8_t Chain; /**< 报文链	1	ASCII码字符。*/
    uint16_t SequenceSeries; /**< 序列类别号	2	二进制无符号短整数。*/
    uint32_t TransactionId; /**<（TID）	FTD信息正文类型	4	二进制无符号整数。*/
    uint32_t SequenceNumber; /**<（SeqNo）	序列号	4	二进制无符号整数。*/
    uint16_t FieldCount; /**< 数据域数量	2	二进制无符号短整数。*/
    uint16_t FTDCContentLength; /**< FTDC信息正文长度	2	二进制无符号短整数。以字节为单位。*/
    uint32_t RequestId; /**< 请求编号(由发送请求者维护，应答中会带回)  4 二进制无符号整数。*/
};

struct TFieldHeader
{
    uint16_t FieldID;
    uint16_t Size;
};

#pragma pack(push)
#pragma pack(1)
///行情更新时间属性
class CMarketDataUpdateTimeField
{
public:
    ///合约代码
    char InstrumentID[31];
    ///最后修改时间
    char UpdateTime[9];
    ///最后修改毫秒
    uint32_t UpdateMillisec;
    ///业务日期
    char ActionDay[9];
};

///行情最优价属性
class CMarketDataBestPriceField
{
public:
    ///申买价一
    double BidPrice1;
    ///申买量一
    uint32_t BidVolume1;
    ///申卖价一
    double AskPrice1;
    ///申卖量一
    uint32_t AskVolume1;
};

class CMarketDataStaticField
{
public:
    ///今开盘
    double OpenPrice;
    ///最高价
    double HighestPrice;
    ///最低价
    double LowestPrice;
    ///今收盘
    double ClosePrice;
    ///涨停板价
    double UpperLimitPrice;
    ///跌停板价
    double LowerLimitPrice;
    ///今结算
    double SettlementPrice;
    ///今虚实度
    double CurrDelta;
};

class CMarketDataLastMatchField
{
public:
    ///最新价
    double LastPrice;
    ///数量
    uint32_t Volume;
    ///成交金额
    double Turnover;
    ///持仓量
    double OpenInterest;
};

///行情申买二、三属性
class CMarketDataBid23Field
{
public:
    ///申买价二
    double BidPrice2;
    ///申买量二
    uint32_t BidVolume2;
    ///申买价三
    double BidPrice3;
    ///申买量三
    uint32_t BidVolume3;
};

///行情申卖二、三属性
class CMarketDataAsk23Field
{
public:
    ///申卖价二
    double AskPrice2;
    ///申卖量二
    uint32_t AskVolume2;
    ///申卖价三
    double AskPrice3;
    ///申卖量三
    uint32_t AskVolume3;
};

///行情申买四、五属性
class CMarketDataBid45Field
{
public:
    ///申买价四
    double BidPrice4;
    ///申买量四
    uint32_t BidVolume4;
    ///申买价五
    double BidPrice5;
    ///申买量五
    uint32_t BidVolume5;
};

///行情申卖四、五属性
class CMarketDataAsk45Field
{
public:
    ///申卖价四
    double AskPrice4;
    ///申卖量四
    uint32_t AskVolume4;
    ///申卖价五
    double AskPrice5;
    ///申卖量五
    uint32_t AskVolume5;
};

class CFTDMarketDataBaseField
{
public:
    //交易日
    char TradingDay[9];
    //上次结算价
    double PreSettlementPrice;
    //昨收盘
    double PreClosePrice;
    //昨持仓量
    double PreOpenInterest;
    //昨虚实度
    double PreDelta;
};

struct CMBLMarketDataField
{
    ///合约代码
    char InstrumentID[31];
    ///买卖方向
    char Direction;
    ///价格
    double Price;
    ///数量
    int Volume;
};
#pragma pack(pop)

#endif  //QUOTE_DATATYPE_ZEUSING_UDP_H_
