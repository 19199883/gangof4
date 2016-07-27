
#ifndef __DATA_STRUCT_H__
#define __DATA_STRUCT_H__

/////////////////////////////////////////字段类型///////////////////////////////////
//用户代码
typedef char TUstpFtdcUserIDType[16];
//密码
typedef char TUstpFtdcPasswordType[41];
///日期类型
typedef char TUstpFtdcDateType[9];
///结算组代码类型
typedef char TUstpFtdcSettlementGroupIDType[9];
///结算编号类型
typedef int TUstpFtdcSettlementIDType;
///价格类型
typedef double TUstpFtdcPriceType;
///大额数量类型
typedef double TUstpFtdcLargeVolumeType;
///比率类型
typedef double TUstpFtdcRatioType;
///数量类型
typedef int TUstpFtdcVolumeType;
///资金类型
typedef double TUstpFtdcMoneyType;
///合约编号类型
typedef char TUstpFtdcInstrumentIDType[31];
///时间类型
typedef char TUstpFtdcTimeType[9];
///最后修改毫秒类型
typedef int TUstpFtdcMillisecType;
///错误代码
typedef int TUstpErrorID;
///错误信息
typedef char TUstpErrorMsg[80];
///主题号
typedef int  TUstpTopicID;
///用户接收多播网卡  
typedef char TUstpIP[40];
///用户接收多播mac  
typedef char TUstpMAC[40];
/////////////////////////////////////////结构体类型///////////////////////////////////

// 响应信息表
struct CUstpRspInfo
{
	TUstpErrorID    ErrorID;                       //错误代码  
	TUstpErrorMsg   ErrorMsg;                      //错误信息
};

///用户登录请求
struct CUstpFtdcReqUserLoginField
{
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///密码
	TUstpFtdcPasswordType	Password;
	///主题号
	TUstpTopicID TopicID;
	///用户接收多播IP  chenhui
	TUstpIP IP;
	///用户接收多播MAC  chenhui
	TUstpMAC Mac;
};


///深度行情
struct CUstpFtdcDepthMarketDataField
{
	///交易日
	TUstpFtdcDateType	TradingDay;
	///结算组代码
	TUstpFtdcSettlementGroupIDType	SettlementGroupID;
	///结算编号
	TUstpFtdcSettlementIDType	SettlementID;
	///昨结算
	TUstpFtdcPriceType	PreSettlementPrice;
	///昨收盘
	TUstpFtdcPriceType	PreClosePrice;
	///昨持仓量
	TUstpFtdcLargeVolumeType	PreOpenInterest;
	///昨虚实度
	TUstpFtdcRatioType	PreDelta;
	///今开盘
	TUstpFtdcPriceType	OpenPrice;
	///最高价
	TUstpFtdcPriceType	HighestPrice;
	///最低价
	TUstpFtdcPriceType	LowestPrice;
	///今收盘
	TUstpFtdcPriceType	ClosePrice;
	///涨停板价
	TUstpFtdcPriceType	UpperLimitPrice;
	///跌停板价
	TUstpFtdcPriceType	LowerLimitPrice;
	///今结算
	TUstpFtdcPriceType	SettlementPrice;
	///今虚实度
	TUstpFtdcRatioType	CurrDelta;
	///最新价
	TUstpFtdcPriceType	LastPrice;
	///数量
	TUstpFtdcVolumeType	Volume;
	///成交金额
	TUstpFtdcMoneyType	Turnover;
	///持仓量
	TUstpFtdcLargeVolumeType	OpenInterest;
	///申买价一
	TUstpFtdcPriceType	BidPrice1;
	///申买量一
	TUstpFtdcVolumeType	BidVolume1;
	///申卖价一
	TUstpFtdcPriceType	AskPrice1;
	///申卖量一
	TUstpFtdcVolumeType	AskVolume1;
	///申买价二
	TUstpFtdcPriceType	BidPrice2;
	///申买量二
	TUstpFtdcVolumeType	BidVolume2;
	///申买价三
	TUstpFtdcPriceType	BidPrice3;
	///申买量三
	TUstpFtdcVolumeType	BidVolume3;
	///申卖价二
	TUstpFtdcPriceType	AskPrice2;
	///申卖量二
	TUstpFtdcVolumeType	AskVolume2;
	///申卖价三
	TUstpFtdcPriceType	AskPrice3;
	///申卖量三
	TUstpFtdcVolumeType	AskVolume3;
	///申买价四
	TUstpFtdcPriceType	BidPrice4;
	///申买量四
	TUstpFtdcVolumeType	BidVolume4;
	///申买价五
	TUstpFtdcPriceType	BidPrice5;
	///申买量五
	TUstpFtdcVolumeType	BidVolume5;
	///申卖价四
	TUstpFtdcPriceType	AskPrice4;
	///申卖量四
	TUstpFtdcVolumeType	AskVolume4;
	///申卖价五
	TUstpFtdcPriceType	AskPrice5;
	///申卖量五
	TUstpFtdcVolumeType	AskVolume5;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///最后修改时间
	TUstpFtdcTimeType	UpdateTime;
	///最后修改毫秒
	TUstpFtdcMillisecType	UpdateMillisec;
};


#endif
