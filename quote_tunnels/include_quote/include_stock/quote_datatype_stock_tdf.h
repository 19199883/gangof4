#pragma once

#ifndef DLL_PUBLIC
#define DLL_PUBLIC  __attribute__ ((visibility("default")))
#endif

#include <string>

#pragma pack(push)
#pragma pack(8)

//股票level2数据
struct DLL_PUBLIC TDF_MARKET_DATA_MY
{
    char szWindCode[32];         //600001.SH
    char szCode[32];             //原始Code
    int nActionDay;             //业务发生日(自然日)
    int nTradingDay;            //交易日
    int nTime;					//时间(HHMMSSmmm)
    int nStatus;				//状态
    unsigned int nPreClose;				//前收盘价 * 10000
    unsigned int nOpen;					//开盘价 * 10000
    unsigned int nHigh;					//最高价 * 10000
    unsigned int nLow;					//最低价 * 10000
    unsigned int nMatch;				//最新价 * 10000
    unsigned int nAskPrice[10];			//申卖价 * 10000
    unsigned int nAskVol[10];			//申卖量
    unsigned int nBidPrice[10];			//申买价 * 10000
    unsigned int nBidVol[10];			//申买量
    unsigned int nNumTrades;			//成交笔数
    long long iVolume;				//成交总量
    long long iTurnover;				//成交总金额
    long long nTotalBidVol;			//委托买入总量
    long long nTotalAskVol;			//委托卖出总量
    unsigned int nWeightedAvgBidPrice;	//加权平均委买价格 * 10000
    unsigned int nWeightedAvgAskPrice;  //加权平均委卖价格 * 10000
    int nIOPV;					//IOPV净值估值  * 10000 （基金）
    int nYieldToMaturity;		//到期收益率	* 10000	（债券）
    unsigned int nHighLimited;			//涨停价 * 10000
    unsigned int nLowLimited;			//跌停价 * 10000
    char chPrefix[4];			//证券信息前缀
    int nSyl1;					//市盈率1	未使用（当前值为0）
    int nSyl2;					//市盈率2	未使用（当前值为0）
    int nSD2;					//升跌2（对比上一笔）	未使用（当前值为0）

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        int t = nTime;                  //时间(HHMMSSmmm)
        sprintf(buf, "%02d:%02d:%02d.%03d", t / 10000000,
            (t / 100000) % 100,
            (t / 1000) % 100,
            t % 1000);
        return std::string(buf);
    }
};

//指数level2数据
struct DLL_PUBLIC TDF_INDEX_DATA_MY
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

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        int t = nTime;                  //时间(HHMMSSmmm)
        sprintf(buf, "%02d:%02d:%02d.%03d", t / 10000000,
            (t / 100000) % 100,
            (t / 1000) % 100,
            t % 1000);
        return std::string(buf);
    }
};
#pragma pack(pop)
