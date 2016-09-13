#if !defined(QUOTE_DATATYPE_TDF_H_)
#define QUOTE_DATATYPE_TDF_H_

#include "TDFAPIStruct.h"

// 按8字节对齐的转换后数据结构
#include <string.h>
#pragma pack(push)
#pragma pack(8)

//股票level2数据
struct TDF_MARKET_DATA_MY
{
    char szWindCode[32];         //600001.SH
    char szCode[32];             //原始Code
    int nActionDay;             //业务发生日(自然日)
    int nTradingDay;            //交易日
    int nTime;					//时间(HHMMSSmmm)
    int nStatus;				//状态
    unsigned int nPreClose;				//前收盘价
    unsigned int nOpen;					//开盘价
    unsigned int nHigh;					//最高价
    unsigned int nLow;					//最低价
    unsigned int nMatch;				//最新价
    unsigned int nAskPrice[10];			//申卖价
    unsigned int nAskVol[10];			//申卖量
    unsigned int nBidPrice[10];			//申买价
    unsigned int nBidVol[10];			//申买量
    unsigned int nNumTrades;			//成交笔数
    long long iVolume;				//成交总量
    long long iTurnover;				//成交总金额
    long long nTotalBidVol;			//委托买入总量
    long long nTotalAskVol;			//委托卖出总量
    unsigned int nWeightedAvgBidPrice;	//加权平均委买价格
    unsigned int nWeightedAvgAskPrice;  //加权平均委卖价格
    int nIOPV;					//IOPV净值估值
    int nYieldToMaturity;		//到期收益率
    unsigned int nHighLimited;			//涨停价
    unsigned int nLowLimited;			//跌停价
    char chPrefix[4];			//证券信息前缀
    int nSyl1;					//市盈率1
    int nSyl2;					//市盈率2
    int nSD2;					//升跌2（对比上一笔）

    TDF_MARKET_DATA_MY()
    {
    }

    TDF_MARKET_DATA_MY(const TDF_MARKET_DATA &data)
    {
        memcpy(szWindCode, data.szWindCode, sizeof(szWindCode)); // 32 32
        memcpy(szCode, data.szCode, sizeof(szCode)); // 32 32
        nActionDay = data.nActionDay;
        nTradingDay = data.nTradingDay;
        nTime = data.nTime;
        nStatus = data.nStatus;
        nPreClose = data.nPreClose;
        nOpen = data.nOpen;
        nHigh = data.nHigh;
        nLow = data.nLow;
        nMatch = data.nMatch;
        memcpy(nAskPrice, data.nAskPrice, sizeof(nAskPrice)); // 10 10
        memcpy(nAskVol, data.nAskVol, sizeof(nAskVol)); // 10 10
        memcpy(nBidPrice, data.nBidPrice, sizeof(nBidPrice)); // 10 10
        memcpy(nBidVol, data.nBidVol, sizeof(nBidVol)); // 10 10
        nNumTrades = data.nNumTrades;
        iVolume = data.iVolume;
        iTurnover = data.iTurnover;
        nTotalBidVol = data.nTotalBidVol;
        nTotalAskVol = data.nTotalAskVol;
        nWeightedAvgBidPrice = data.nWeightedAvgBidPrice;
        nWeightedAvgAskPrice = data.nWeightedAvgAskPrice;
        nIOPV = data.nIOPV;
        nYieldToMaturity = data.nYieldToMaturity;
        nHighLimited = data.nHighLimited;
        nLowLimited = data.nLowLimited;
        memcpy(chPrefix, data.chPrefix, sizeof(chPrefix)); // 4 4
        nSyl1 = data.nSyl1;
        nSyl2 = data.nSyl2;
        nSD2 = data.nSD2;

    }
};

//指数level2数据
struct TDF_INDEX_DATA_MY
{
    char szWindCode[32];         //600001.SH
    char szCode[32];             //原始Code
    int nActionDay;             //业务发生日(自然日)
    int nTradingDay;            //交易日
    int nTime;			//时间(HHMMSSmmm)
    int nOpenIndex;		//今开盘指数
    int nHighIndex;		//最高指数
    int nLowIndex;		//最低指数
    int nLastIndex;		//最新指数
    long long iTotalVolume;	//参与计算相应指数的交易数量
    long long iTurnover;		//参与计算相应指数的成交金额
    int nPreCloseIndex;	//前盘指数

    TDF_INDEX_DATA_MY()
    {
    }

    TDF_INDEX_DATA_MY(const TDF_INDEX_DATA &data)
    {
        memcpy(szWindCode, data.szWindCode, sizeof(szWindCode)); // 32 32
        memcpy(szCode, data.szCode, sizeof(szCode)); // 32 32
        nActionDay = data.nActionDay;
        nTradingDay = data.nTradingDay;
        nTime = data.nTime;
        nOpenIndex = data.nOpenIndex;
        nHighIndex = data.nHighIndex;
        nLowIndex = data.nLowIndex;
        nLastIndex = data.nLastIndex;
        iTotalVolume = data.iTotalVolume;
        iTurnover = data.iTurnover;
        nPreCloseIndex = data.nPreCloseIndex;
    }
};
#pragma pack(pop)

#endif  //QUOTE_DATATYPE_TDF_H_
