#pragma once

#ifndef DLL_PUBLIC
#define DLL_PUBLIC  __attribute__ ((visibility("default")))
#endif

#include <string>

#pragma pack(push)
#pragma pack(8)

///深度行情
struct DLL_PUBLIC ksg_depth_market_data
{
	///交易日
	char	TradingDay[8];
	///合约代码
	char	ContractID[8];
	///最新价
	double	LastPrice;
	///最高价
	double	HighPrice;
	///最低价
	double	LowPrice;
	///成交量
	int	MatchTotQty;
	///成交重量
	double	MatchWeight;
	///成交额
	double	Turnover;
	///持仓量
	int	OpenInterest;
	///开盘价
	double	OpenPrice;
	///收盘价
	double	ClosePrice;
	///今结算价
	double	ClearPrice;
	///上日结算价
	double	LastClearPrice;
	///上日收盘价
	double	LastClose;
	///涨停板
	double	RiseLimit;
	///跌停板
	double	FallLimit;
	///涨跌
	double	UpDown;
	///涨跌幅度
	double	UpDownRate;
	///当日均价
	double	AveragePrice;
	///申买价一
	double	BidPrice1;
	///申买量一
	int	BidVolume1;
	///申卖价一
	double	AskPrice1;
	///申卖量一
	int	AskVolume1;
	///申买价二
	double	BidPrice2;
	///申买量二
	int	BidVolume2;
	///申卖价二
	double	AskPrice2;
	///申卖量二
	int	AskVolume2;
	///申买价三
	double	BidPrice3;
	///申买量三
	int	BidVolume3;
	///申卖价三
	double	AskPrice3;
	///申卖量三
	int	AskVolume3;
	///申买价四
	double	BidPrice4;
	///申买量四
	int	BidVolume4;
	///申卖价四
	double	AskPrice4;
	///申卖量四
	int	AskVolume4;
	///申买价五
	double	BidPrice5;
	///申买量五
	int	BidVolume5;
	///申卖价五
	double	AskPrice5;
	///申卖量五
	int	AskVolume5;   
};
#pragma pack(pop)

struct DLL_PUBLIC CKSG_DepthMarketDataField
{
	ksg_depth_market_data data;

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        return std::string();
    }
};
