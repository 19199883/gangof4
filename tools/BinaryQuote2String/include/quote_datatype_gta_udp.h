/////////////////////////////////////////////////////////////////////////////////////////
// @file：CfxUdpStruct.h
// 描    述：中金所UDP发送数据结构定义
// 作    者：mars.zippo
// 创建日期：2014-03-20
// 版    本：v1.0.0
// 修改  者：
// 修改日期：
// 修改内容：
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef GTA_CFX_UDP_STRUCT_H
#define GTA_CFX_UDP_STRUCT_H

#include "quote_datatype_common.h"
#include "quote_datatype_gtaex.h"

/// 中金所深度市场行情
struct CFfexFtdcDepthMarketData
{
    char szTradingDay[9];        ///< 交易日
    char szSettlementGroupID[9]; ///< 结算组代码
    int nSettlementID;          ///< 结算编号
    double dLastPrice;             ///< 最新价
    double dPreSettlementPrice;    ///< 昨结算
    double dPreClosePrice;         ///< 昨收盘
    double dPreOpenInterest;       ///< 昨持仓量
    double dOpenPrice;             ///< 今开盘
    double dHighestPrice;          ///< 最高价
    double dLowestPrice;           ///< 最低价
    int nVolume;                ///< 数量
    double dTurnover;              ///< 成交金额
    double dOpenInterest;          ///< 持仓量
    double dClosePrice;            ///< 今收盘
    double dSettlementPrice;       ///< 今结算
    double dUpperLimitPrice;       ///< 涨停板价
    double dLowerLimitPrice;       ///< 跌停板价
    double dPreDelta;              ///< 昨虚实度
    double dCurrDelta;             ///< 今虚实度
    char szUpdateTime[9];        ///< 最后修改时间
    int nUpdateMillisec;        ///< 最后修改毫秒
    char szInstrumentID[31];     ///< 合约代码
    double dBidPrice1;             ///< 申买价一
    int nBidVolume1;            ///< 申买量一
    double dAskPrice1;             ///< 申卖价一
    int nAskVolume1;            ///< 申卖量一
    double dBidPrice2;             ///< 申买价二
    int nBidVolume2;            ///< 申买量二
    double dAskPrice2;             ///< 申卖价二
    int nAskVolume2;            ///< 申卖量二
    double dBidPrice3;             ///< 申买价三
    int nBidVolume3;            ///< 申买量三
    double dAskPrice3;             ///< 申卖价三
    int nAskVolume3;            ///< 申卖量三
    double dBidPrice4;             ///< 申买价四
    int nBidVolume4;            ///< 申买量四
    double dAskPrice4;             ///< 申卖价四
    int nAskVolume4;            ///< 申卖量四
    double dBidPrice5;             ///< 申买价五
    int nBidVolume5;            ///< 申买量五
    double dAskPrice5;             ///< 申卖价五
    int nAskVolume5;            ///< 申卖量五

    CFfexFtdcDepthMarketData()
    {
        memset(this, 0, sizeof(CFfexFtdcDepthMarketData));
    }
    CFfexFtdcDepthMarketData(const ZJS_Future_Input &ex_data)
    {
        memset(this, 0, sizeof(CFfexFtdcDepthMarketData));

        strcpy(szSettlementGroupID, "SG01");  // 9
        nSettlementID = 1;
        dLastPrice = ex_data.dLastTrade;
        dPreSettlementPrice = ex_data.dPreSettlementPrice;
        dPreClosePrice = ex_data.dYclose;
        dPreOpenInterest = ex_data.dPreOpenInterest;
        dOpenPrice = ex_data.dOpen;
        dHighestPrice = ex_data.dHigh;
        dLowestPrice = ex_data.dLow;
        nVolume = (int)ex_data.dVolume;
        dTurnover = ex_data.dAmount;
        dOpenInterest = ex_data.dOpenInterest;
        dClosePrice = 0;
        dSettlementPrice = ex_data.dSettlementPrice;
        dUpperLimitPrice = ex_data.dUpB;
        dLowerLimitPrice = ex_data.dLowB;
        dPreDelta = 0;
        dCurrDelta = 0;
        memcpy(szUpdateTime, ex_data.szUpdateTime, 9); // 9, 9
        nUpdateMillisec = 0;
        memcpy(szInstrumentID, ex_data.szStockNo, 6); // 31, 6
        dBidPrice1 = ex_data.dBuyprice1;
        nBidVolume1 = (int)ex_data.dBuyvol1;
        dAskPrice1 = ex_data.dSellprice1;
        nAskVolume1 = (int)ex_data.dSellvol1;
        dBidPrice2 = ex_data.dBuyprice2;
        nBidVolume2 = (int)ex_data.dBuyvol2;
        dAskPrice2 = ex_data.dSellprice2;
        nAskVolume2 = (int)ex_data.dSellvol2;
        dBidPrice3 = ex_data.dBuyprice3;
        nBidVolume3 = (int)ex_data.dBuyvol3;
        dAskPrice3 = ex_data.dSellprice3;
        nAskVolume3 = (int)ex_data.dSellvol3;
        dBidPrice4 = ex_data.dBuyprice4;
        nBidVolume4 = (int)ex_data.dBuyvol4;
        dAskPrice4 = ex_data.dSellprice4;
        nAskVolume4 = (int)ex_data.dSellvol4;
        dBidPrice5 = ex_data.dBuyprice5;
        nBidVolume5 = (int)ex_data.dBuyvol5;
        dAskPrice5 = ex_data.dSellprice5;
        nAskVolume5 = (int)ex_data.dSellvol5;
    }
};

#endif
