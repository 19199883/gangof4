#ifndef KSG_FTDCSTRUCT_H
#define KSG_FTDCSTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KSGUserApiDataType.h"

///响应信息
struct CKSG_RspInfoField
{
	///错误代码
	TKSG_ErrorIDType	ErrCode;
	///错误信息
	TKSG_ErrorMsgType	RspMsg;
	//时间戳
	TKSG_TimestampType TimeStamp;
};

///用户登录请求
struct CKSG_ReqUserLoginField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///密码
	TKSG_PasswordType	Pwd;
};

///用户登录应答
struct CKSG_RspUserLoginField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///交易核心日期
	TKSG_TimeType	Time;
	///交易核心时间
	TKSG_DateType	Date;
	///最近成功提交的报单编号
	TKSG_LatestOrderNo	LatestOrderNo;
};

///用户登出请求
struct CKSG_ReqUserLogoutField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
};

///用户登出应答
struct CKSG_RspUserLogoutField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
};

///用户资金查询
struct CKSG_ReqTradingAccountField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
};

///用户资金应答
struct CKSG_RspTradingAccountField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///可用资金
	TKSG_MoneyType	Available;
	///委托冻结资金
	TKSG_MoneyType	FrozenMargin;
	///占用资金
	TKSG_MoneyType	OccupyMargin;
	///平仓盈亏
	TKSG_MoneyType	CloseProfit;
};

///用户资金变化通知
struct CKSG_RtnTradingAccountField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///可用资金
	TKSG_MoneyType	Available;
	///委托冻结资金
	TKSG_MoneyType	FrozenMargin;
	///占用资金
	TKSG_MoneyType	OccupyMargin;
	///平仓盈亏
	TKSG_MoneyType	CloseProfit;
};

//库存查询请求
struct CKSG_ReqStorageField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///交割品种代码
	TKSG_VarietyIDType  VarietyID;
	///合约代码
	TKSG_InstrumentIDType	ContractID;
};

///库存查询应答
struct CKSG_RspStorageField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///交割品种代码
	TKSG_VarietyIDType  VarietyID;            	
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///库存总量
	TKSG_WeightType     TotalStorage;         	
	///可用库存
	TKSG_WeightType	AvailableStorage;
	///现货冻结库存
	TKSG_WeightType	FrozenStorage;
};

///库存变化通知
struct CKSG_RtnStorageField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///交割品种代码
	TKSG_VarietyIDType  VarietyID;            	
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///库存总量
	TKSG_WeightType     TotalStorage;        	
	///可用库存
	TKSG_WeightType	AvailableStorage;
	///现货冻结库存
	TKSG_WeightType	FrozenStorage;
};

///用户持仓查询
struct CKSG_ReqInvestorPositionField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///合约代码
	TKSG_InstrumentIDType	ContractID;
};

///用户持仓应答
struct CKSG_RspInvestorPositionField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///买卖标志
	TKSG_ByteType	BsFlag;
	///上日持仓量
	TKSG_VolumeType	LastQty;
	///今日持仓量
	TKSG_VolumeType	Qty;
	///可平昨
	TKSG_VolumeType	OffsetLastQty;
	///可平今
	TKSG_VolumeType	OffsetTodayQty;
	///持仓额
	TKSG_PriceType		Amt;
	///持仓均价
	TKSG_PriceType		AvePrice;
	///最新价
	TKSG_PriceType		LastPrice;
	///持仓盈亏
	TKSG_PriceType		FloatProfitLoss;

};

///用户持仓变化通知
struct CKSG_RtnInvestorPositionField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///买卖标志
	TKSG_ByteType	BsFlag;
	///上日持仓量
	TKSG_VolumeType	LastQty;
	///今日持仓量
	TKSG_VolumeType	Qty;
	///可平昨
	TKSG_VolumeType	OffsetLastQty;
	///可平今
	TKSG_VolumeType	OffsetTodayQty;
	///持仓额
	TKSG_PriceType		Amt;
	///保证金
	TKSG_PriceType		Margin;
	///持仓均价
	TKSG_PriceType		AvePrice;
	///最新价
	TKSG_PriceType		LastPrice;
	///持仓盈亏
	TKSG_PriceType		FloatProfitLoss;

};

///市场状态应答
struct CKSG_RspMarketStatus
{
	///市场状态
	TKSG_MktStatusType   MktStatus;           
	///状态触发时间
	TKSG_TimeType	MktChgTime;
};

///市场状态变化通知
struct CKSG_RtnMarketStatus
{
	///市场状态
	TKSG_MktStatusType   MktStatus;           
	///状态触发时间
	TKSG_TimeType	MktChgTime;
};

///合约查询应答
struct CKSG_RspInstrumentField
{
	///合约代码
	TKSG_InstrumentIDType	ContractID;        
	///合约名称
	TKSG_InstrumentNameType	ContractName;
	///交割品种代码
	TKSG_VarietyIDType   VarietyID;  
	///合约数量乘数
	TKSG_VolumeMultipleType	Unit;
	//计价单位
	TKSG_MeasureUnit   MeasureUnit;         
	///最小变动价位
	TKSG_PriceType	Tick;
	///市价单最大下单量
	TKSG_VolumeType	MaxHand;
	///市价单最小下单量
	TKSG_VolumeType	MinHand;
	///限价单最大下单量
	TKSG_VolumeType	MaxMarketOrderVolume;
	///限价单最小下单量
	TKSG_VolumeType	MinMarketOrderVolume;
	///涨停板
	TKSG_PriceType		RiseLimit;
	///跌停板
	TKSG_PriceType		FallLimit;
	///上日结算价
	TKSG_PriceType		LastClearPrice;
};

///下单请求
struct CKSG_ReqOrderField
{
	///本地委托号
	TKSG_LocalOrderNo LocalOrderNo;
	///用户代码
	TKSG_UserIDType	TraderNo;
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///买卖标志
	TKSG_ByteType	BsFlag;
	///开平标志
	TKSG_ByteType	EoFlag;
	///价格
	TKSG_PriceType		Price;
	///数量
	TKSG_VolumeType	Qty;
	///定单类型
	TKSG_ByteType	OrderType;
};

///下单应答
struct CKSG_RspOrderField
{
	///本地委托号
	TKSG_LocalOrderNo LocalOrderNo;
	///用户代码
	TKSG_UserIDType	TraderNo;
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///买卖标志
	TKSG_ByteType	BsFlag;
	///开平标志
	TKSG_ByteType	EoFlag;
	///价格
	TKSG_PriceType		Price;
	///数量
	TKSG_VolumeType	Qty;
	///定单类型
	TKSG_ByteType	OrderType;
};

///下单回报
struct CKSG_RtnOrderField
{
	///系统委托号
	TKSG_SysOrderNo SysOrderNo;
	///本地委托号
	TKSG_LocalOrderNo LocalOrderNo;
	///用户代码
	TKSG_UserIDType	TraderNo;
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///买卖标志
	TKSG_ByteType	BsFlag;
	///开平标志
	TKSG_ByteType	EoFlag;
	///价格
	TKSG_PriceType		Price;
	///数量
	TKSG_VolumeType	Qty;
	///定单类型
	TKSG_ByteType	OrderType;
	///委托时间
	TKSG_TimeType	OrderTime;
	///状态
	TKSG_Status	Status;
	///剩余数量
	TKSG_VolumeType	RemainAmt;
};

///撤单请求
struct CKSG_ReqOrderCancelField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///系统委托号
	TKSG_SysOrderNo SysOrderNo;
};

///撤单应答
struct CKSG_RspOrderCancelField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///系统委托号
	TKSG_SysOrderNo SysOrderNo;
};

///撤单响应
struct CKSG_RtnOrderCancelField
{
	///用户代码
	TKSG_UserIDType	TraderNo;
	///系统委托号
	TKSG_SysOrderNo SysOrderNo;
	///本地委托号
	TKSG_LocalOrderNo LocalOrderNo;
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///操作时间
	TKSG_TimeType	ActionTime;
	///撤单数量
	TKSG_VolumeType	OrderCancelQty;
	///定单类型
	TKSG_ByteType	OrderType;
};

///成交回报通知
struct CKSG_RtnTradeField
{
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///成交号
	TKSG_MatchNo	MatchNo;
	///成交时间
	TKSG_TimeType	MatchTime;
	///成交日期
	TKSG_DateType	MatchDate;
	///成交价格
	TKSG_PriceType		Price;
	///成交数量
	TKSG_VolumeType	Qty;
	///用户代码
	TKSG_UserIDType	TraderNo;
	///买卖标志
	TKSG_ByteType	BsFlag;
	///开平标志
	TKSG_ByteType	EoFlag;
	///本地委托号
	TKSG_LocalOrderNo LocalOrderNo;
};

///深度行情
struct CKSG_DepthMarketDataField
{
	///交易日
	TKSG_DateType	TradingDay;
	///合约代码
	TKSG_InstrumentIDType	ContractID;
	///最新价
	TKSG_PriceType	LastPrice;
	///最高价
	TKSG_PriceType	HighPrice;
	///最低价
	TKSG_PriceType	LowPrice;
	///成交量
	TKSG_VolumeType	MatchTotQty;
	///成交重量
	TKSG_WeightType	MatchWeight;
	///成交额
	TKSG_MoneyType	Turnover;
	///持仓量
	TKSG_VolumeType	OpenInterest;
	///开盘价
	TKSG_PriceType	OpenPrice;
	///收盘价
	TKSG_PriceType	ClosePrice;
	///今结算价
	TKSG_PriceType	ClearPrice;
	///上日结算价
	TKSG_PriceType	LastClearPrice;
	///上日收盘价
	TKSG_PriceType	LastClose;
	///涨停板
	TKSG_PriceType	RiseLimit;
	///跌停板
	TKSG_PriceType	FallLimit;
	///涨跌
	TKSG_PriceType	UpDown;
	///涨跌幅度
	TKSG_PriceType	UpDownRate;
	///当日均价
	TKSG_PriceType	AveragePrice;
	///申买价一
	TKSG_PriceType	BidPrice1;
	///申买量一
	TKSG_VolumeType	BidVolume1;
	///申卖价一
	TKSG_PriceType	AskPrice1;
	///申卖量一
	TKSG_VolumeType	AskVolume1;
	///申买价二
	TKSG_PriceType	BidPrice2;
	///申买量二
	TKSG_VolumeType	BidVolume2;
	///申卖价二
	TKSG_PriceType	AskPrice2;
	///申卖量二
	TKSG_VolumeType	AskVolume2;
	///申买价三
	TKSG_PriceType	BidPrice3;
	///申买量三
	TKSG_VolumeType	BidVolume3;
	///申卖价三
	TKSG_PriceType	AskPrice3;
	///申卖量三
	TKSG_VolumeType	AskVolume3;
	///申买价四
	TKSG_PriceType	BidPrice4;
	///申买量四
	TKSG_VolumeType	BidVolume4;
	///申卖价四
	TKSG_PriceType	AskPrice4;
	///申卖量四
	TKSG_VolumeType	AskVolume4;
	///申买价五
	TKSG_PriceType	BidPrice5;
	///申买量五
	TKSG_VolumeType	BidVolume5;
	///申卖价五
	TKSG_PriceType	AskPrice5;
	///申卖量五
	TKSG_VolumeType	AskVolume5;
    
};

#endif
