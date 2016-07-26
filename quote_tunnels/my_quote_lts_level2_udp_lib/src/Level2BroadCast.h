#pragma once

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#pragma pack(1)
	struct CL2FAST_MD
	{
		double		LastPrice;					// 最新价
		double		OpenPrice;					// 开盘价
		double		ClosePrice;					// 收盘价
		double		HighPrice;					// 最高价
		double		LowPrice;					// 最低价
		double		TotalTradeVolume;			// 成交数量
		double		TotalTradeValue;			// 成交金额
		double		TradeCount;					// 成交笔数
		double		OpenInterest;				// 持仓量
		double		IOPV;						// 基金净值
		double		YieldToMaturity;			// 到期收益率
		double		AuctionPrice;				// 动态参考价格
		double		TotalBidVolume;				// 申买总量
		double		WeightedAvgBidPrice;		// 申买加权均价
		double		AltWeightedAvgBidPrice;		// 债券申买加权均价
		double		TotalOfferVolume;			// 申卖总量
		double		WeightedAvgOfferPrice;		// 申卖加权均价
		double		AltWeightedAvgOfferPrice;	// 债券申卖加权均价
		int			BidPriceLevel;				// 买价深度
		int			OfferPriceLevel;			// 卖价深度
		double		BidPrice1;					// 申买价一
		double		BidVolume1;					// 申买量一
		double		BidCount1;					// 申买笔数一
		double		OfferPrice1;				// 申卖价一
		double		OfferVolume1;				// 申卖量一
		double		OfferCount1;				// 申卖笔数一
		double		BidPrice2;					// 申买价二
		double		BidVolume2;					// 申买量二
		double		BidCount2;					// 申买笔数二
		double		OfferPrice2;				// 申卖价二
		double		OfferVolume2;				// 申卖量二
		double		OfferCount2;				// 申卖笔数二
		double		BidPrice3;					// 申买价三
		double		BidVolume3;					// 申买量三
		double		BidCount3;					// 申买笔数三
		double		OfferPrice3;				// 申卖价三
		double		OfferVolume3;				// 申卖量三
		double		OfferCount3;				// 申卖笔数三
		double		BidPrice4;					// 申买价四
		double		BidVolume4;					// 申买量四
		double		BidCount4;					// 申买笔数四
		double		OfferPrice4;				// 申卖价四
		double		OfferVolume4;				// 申卖量四
		double		OfferCount4;				// 申卖笔数四
		double		BidPrice5;					// 申买价五
		double		BidVolume5;					// 申买量五
		double		BidCount5;					// 申买笔数五
		double		OfferPrice5;				// 申卖价五
		double		OfferVolume5;				// 申卖量五
		double		OfferCount5;				// 申卖笔数五
		double		BidPrice6;					// 申买价六
		double		BidVolume6;					// 申买量六
		double		BidCount6;					// 申买笔数六
		double		OfferPrice6;				// 申卖价六
		double		OfferVolume6;				// 申卖量六
		double		OfferCount6;				// 申卖笔数六
		double		BidPrice7;					// 申买价七
		double		BidVolume7;					// 申买量七
		double		BidCount7;					// 申买笔数七
		double		OfferPrice7;				// 申卖价七
		double		OfferVolume7;				// 申卖量七
		double		OfferCount7;				// 申卖笔数七
		double		BidPrice8;					// 申买价八
		double		BidVolume8;					// 申买量八
		double		BidCount8;					// 申买笔数八
		double		OfferPrice8;				// 申卖价八
		double		OfferVolume8;				// 申卖量八
		double		OfferCount8;				// 申卖笔数八
		double		BidPrice9;					// 申买价九
		double		BidVolume9;					// 申买量九
		double		BidCount9;					// 申买笔数九
		double		OfferPrice9;				// 申卖价九
		double		OfferVolume9;				// 申卖量九
		double		OfferCount9;				// 申卖笔数九
		double		BidPriceA;					// 申买价十
		double		BidVolumeA;					// 申买量十
		double		BidCountA;					// 申买笔数十
		double		OfferPriceA;				// 申卖价十
		double		OfferVolumeA;				// 申卖量十
		double		OfferCountA;				// 申卖笔数十
		char		ExchangeID[4];				// 交易所代码
		char		InstrumentID[9];			// 证券代码
		char		TimeStamp[13];				// 时间戳
		char		TradingPhase;				// 交易阶段
		char		OpenRestriction;			// 开仓限制
	};


	struct CL2FAST_INDEX
	{
		double		LastIndex;					// 最新指数
		double		OpenIndex;					// 开盘指数
		double		CloseIndex;					// 收盘指数
		double		HighIndex;					// 最高指数
		double		LowIndex;					// 最低指数
		double		TurnOver;					// 成交金额
		double		TotalVolume;				// 成交数量
		char		ExchangeID[4];				// 交易所代码
		char		InstrumentID[9];			// 指数代码
		char		TimeStamp[13];				// 时间戳
	};

	struct CL2FAST_ORDER
	{
		double		Price;						// 委托价格
		double		Volume;						// 委托数量
		int			OrderGroupID;				// 委托组
		int			OrderIndex;					// 委托序号
		char		OrderKind;					// 报单类型
		char		FunctionCode;				// 功能码
		char		ExchangeID[4];				// 交易所代码
		char		InstrumentID[9];			// 合约代码
		char		OrderTime[13];				// 时间戳
	};

	struct CL2FAST_TRADE
	{
		double		Price;						// 成交价格
		double		Volume;						// 成交数量
		int			TradeGroupID;				// 成交组
		int			TradeIndex;					// 成交序号
		int			BuyIndex;					// 买方委托序号
		int			SellIndex;					// 卖方委托序号
		char		OrderKind;					// 报单类型
		char		FunctionCode;				// 功能码
		char		ExchangeID[4];				// 交易所代码
		char		InstrumentID[9];			// 合约代码
		char		TradeTime[13];				// 时间戳
	};
#pragma pack()

