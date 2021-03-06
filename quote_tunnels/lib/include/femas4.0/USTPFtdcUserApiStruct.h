/////////////////////////////////////////////////////////////////////////
///@system 风控前置系统
///@company CFFEX
///@file USTPFtdcUserApiStruct.h
///@brief 定义了客户端接口使用的业务数据结构
///@history 
/////////////////////////////////////////////////////////////////////////

#if !defined(_USTP_FTDCSTRUCT_H)
#define _USTP_FTDCSTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "USTPFtdcUserApiDataType.h"

///系统用户登录请求
struct CUstpFtdcReqUserLoginField
{
	///交易日
	TUstpFtdcDateType	TradingDay;
	///交易用户代码
	TUstpFtdcUserIDType	UserID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///密码
	TUstpFtdcPasswordType	Password;
	///用户端产品信息
	TUstpFtdcProductInfoType	UserProductInfo;
	///接口端产品信息
	TUstpFtdcProductInfoType	InterfaceProductInfo;
	///协议信息
	TUstpFtdcProtocolInfoType	ProtocolInfo;
	///IP地址
	TUstpFtdcIPAddressType	IPAddress;
	///Mac地址
	TUstpFtdcMacAddressType	MacAddress;
	///数据中心代码
	TUstpFtdcDataCenterIDType	DataCenterID;
	///客户端运行文件大小
	TUstpFtdcFileSizeType	UserProductFileSize;
};
///系统用户登录应答
struct CUstpFtdcRspUserLoginField
{
	///交易日
	TUstpFtdcDateType	TradingDay;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易用户代码
	TUstpFtdcUserIDType	UserID;
	///登录成功时间
	TUstpFtdcTimeType	LoginTime;
	///登录成功时的交易所时间
	TUstpFtdcTimeType	ExchangeTime;
	///用户最大本地报单号
	TUstpFtdcUserOrderLocalIDType	MaxOrderLocalID;
	///交易系统名称
	TUstpFtdcTradingSystemNameType	TradingSystemName;
	///数据中心代码
	TUstpFtdcDataCenterIDType	DataCenterID;
	///会员私有流当前长度
	TUstpFtdcSequenceNoType	PrivateFlowSize;
	///交易员私有流当前长度
	TUstpFtdcSequenceNoType	UserFlowSize;
	///业务发生日期
	TUstpFtdcDateType	ActionDay;
	///飞马版本号
	TUstpFtdcFemasVersionType	FemasVersion;
	///飞马生命周期号
	TUstpFtdcFemasLifeCycleType	FemasLifeCycle;
};
///用户登出请求
struct CUstpFtdcReqUserLogoutField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易用户代码
	TUstpFtdcUserIDType	UserID;
};
///用户登出响应
struct CUstpFtdcRspUserLogoutField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易用户代码
	TUstpFtdcUserIDType	UserID;
};
///强制用户退出
struct CUstpFtdcForceUserExitField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易用户代码
	TUstpFtdcUserIDType	UserID;
};
///用户口令修改
struct CUstpFtdcUserPasswordUpdateField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易用户代码
	TUstpFtdcUserIDType	UserID;
	///旧密码
	TUstpFtdcPasswordType	OldPassword;
	///新密码
	TUstpFtdcPasswordType	NewPassword;
};
///输入报单
struct CUstpFtdcInputOrderField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///系统报单编号
	TUstpFtdcOrderSysIDType	OrderSysID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///指定通过此席位序号下单
	TUstpFtdcSeatNoType	SeatNo;
	///合约代码/套利组合合约号
	TUstpFtdcInstrumentIDType	InstrumentID;
	///用户本地报单号
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
	///报单类型
	TUstpFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
	///开平标志
	TUstpFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///价格
	TUstpFtdcPriceType	LimitPrice;
	///数量
	TUstpFtdcVolumeType	Volume;
	///有效期类型
	TUstpFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TUstpFtdcDateType	GTDDate;
	///成交量类型
	TUstpFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TUstpFtdcVolumeType	MinVolume;
	///止损价止赢价
	TUstpFtdcPriceType	StopPrice;
	///强平原因
	TUstpFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TUstpFtdcBoolType	IsAutoSuspend;
	///业务单元
	TUstpFtdcBusinessUnitType	BusinessUnit;
	///用户自定义域
	TUstpFtdcCustomType	UserCustom;
	///本地业务标识
	TUstpFtdcBusinessLocalIDType	BusinessLocalID;
	///业务发生日期
	TUstpFtdcDateType	ActionDay;
	///策略类别
	TUstpFtdcArbiTypeType	ArbiType;
};
///报单操作
struct CUstpFtdcOrderActionField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///报单编号
	TUstpFtdcOrderSysIDType	OrderSysID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///本次撤单操作的本地编号
	TUstpFtdcUserOrderLocalIDType	UserOrderActionLocalID;
	///被撤订单的本地报单编号
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
	///报单操作标志
	TUstpFtdcActionFlagType	ActionFlag;
	///价格
	TUstpFtdcPriceType	LimitPrice;
	///数量变化
	TUstpFtdcVolumeType	VolumeChange;
	///本地业务标识
	TUstpFtdcBusinessLocalIDType	BusinessLocalID;
};
///内存表导出
struct CUstpFtdcMemDbField
{
	///内存表名
	TUstpFtdcMemTableNameType	MemTableName;
};
///用户请求出入金
struct CUstpFtdcstpUserDepositField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///金额
	TUstpFtdcMoneyType	Amount;
	///出入金方向
	TUstpFtdcAccountDirectionType	AmountDirection;
	///用户本地报单号
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
};
///用户主次席出入金
struct CUstpFtdcstpTransferMoneyField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///金额
	TUstpFtdcMoneyType	Amount;
	///出入金方向
	TUstpFtdcAccountDirectionType	AmountDirection;
	///用户本地报单号
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
	///银行机构代码
	TUstpFtdcBankIDType	BankID;
	///银行转账密码
	TUstpFtdcBankPasswdType	BankPasswd;
	///主席转账密码
	TUstpFtdcAccountPasswdType	AccountPasswd;
};
///响应信息
struct CUstpFtdcRspInfoField
{
	///错误代码
	TUstpFtdcErrorIDType	ErrorID;
	///错误信息
	TUstpFtdcErrorMsgType	ErrorMsg;
};
///报单查询
struct CUstpFtdcQryOrderField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///报单编号
	TUstpFtdcOrderSysIDType	OrderSysID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///报单状态
	TUstpFtdcOrderStatusType	OrderStatus;
	///委托类型
	TUstpFtdcOrderTypeType	OrderType;
};
///成交查询
struct CUstpFtdcQryTradeField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///成交编号
	TUstpFtdcTradeIDType	TradeID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
};
///合约查询
struct CUstpFtdcQryInstrumentField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///产品代码
	TUstpFtdcProductIDType	ProductID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
};
///合约查询应答
struct CUstpFtdcRspInstrumentField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///品种代码
	TUstpFtdcProductIDType	ProductID;
	///品种名称
	TUstpFtdcProductNameType	ProductName;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///合约名称
	TUstpFtdcInstrumentNameType	InstrumentName;
	///交割年份
	TUstpFtdcYearType	DeliveryYear;
	///交割月
	TUstpFtdcMonthType	DeliveryMonth;
	///限价单最大下单量
	TUstpFtdcVolumeType	MaxLimitOrderVolume;
	///限价单最小下单量
	TUstpFtdcVolumeType	MinLimitOrderVolume;
	///市价单最大下单量
	TUstpFtdcVolumeType	MaxMarketOrderVolume;
	///市价单最小下单量
	TUstpFtdcVolumeType	MinMarketOrderVolume;
	///数量乘数
	TUstpFtdcVolumeMultipleType	VolumeMultiple;
	///报价单位
	TUstpFtdcPriceTickType	PriceTick;
	///币种
	TUstpFtdcCurrencyType	Currency;
	///多头限仓
	TUstpFtdcVolumeType	LongPosLimit;
	///空头限仓
	TUstpFtdcVolumeType	ShortPosLimit;
	///跌停板价
	TUstpFtdcPriceType	LowerLimitPrice;
	///涨停板价
	TUstpFtdcPriceType	UpperLimitPrice;
	///昨结算
	TUstpFtdcPriceType	PreSettlementPrice;
	///合约交易状态
	TUstpFtdcInstrumentStatusType	InstrumentStatus;
	///创建日
	TUstpFtdcDateType	CreateDate;
	///上市日
	TUstpFtdcDateType	OpenDate;
	///到期日
	TUstpFtdcDateType	ExpireDate;
	///开始交割日
	TUstpFtdcDateType	StartDelivDate;
	///最后交割日
	TUstpFtdcDateType	EndDelivDate;
	///挂牌基准价
	TUstpFtdcPriceType	BasisPrice;
	///当前是否交易
	TUstpFtdcBoolType	IsTrading;
	///基础商品代码
	TUstpFtdcInstrumentIDType	UnderlyingInstrID;
	///基础商品乘数
	TUstpFtdcUnderlyingMultipleType	UnderlyingMultiple;
	///持仓类型
	TUstpFtdcPositionTypeType	PositionType;
	///执行价
	TUstpFtdcPriceType	StrikePrice;
	///期权类型
	TUstpFtdcOptionsTypeType	OptionsType;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///策略类别
	TUstpFtdcArbiTypeType	ArbiType;
	///第一腿合约代码
	TUstpFtdcInstrumentIDType	InstrumentID_1;
	///第一腿买卖方向
	TUstpFtdcDirectionType	Direction_1;
	///第一腿数量比例
	TUstpFtdcRatioType	Ratio_1;
	///第二腿合约代码
	TUstpFtdcInstrumentIDType	InstrumentID_2;
	///第二腿买卖方向
	TUstpFtdcDirectionType	Direction_2;
	///第二腿数量比例
	TUstpFtdcRatioType	Ratio_2;
};
///合约状态
struct CUstpFtdcInstrumentStatusField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///品种代码
	TUstpFtdcProductIDType	ProductID;
	///品种名称
	TUstpFtdcProductNameType	ProductName;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///合约名称
	TUstpFtdcInstrumentNameType	InstrumentName;
	///交割年份
	TUstpFtdcYearType	DeliveryYear;
	///交割月
	TUstpFtdcMonthType	DeliveryMonth;
	///限价单最大下单量
	TUstpFtdcVolumeType	MaxLimitOrderVolume;
	///限价单最小下单量
	TUstpFtdcVolumeType	MinLimitOrderVolume;
	///市价单最大下单量
	TUstpFtdcVolumeType	MaxMarketOrderVolume;
	///市价单最小下单量
	TUstpFtdcVolumeType	MinMarketOrderVolume;
	///数量乘数
	TUstpFtdcVolumeMultipleType	VolumeMultiple;
	///报价单位
	TUstpFtdcPriceTickType	PriceTick;
	///币种
	TUstpFtdcCurrencyType	Currency;
	///多头限仓
	TUstpFtdcVolumeType	LongPosLimit;
	///空头限仓
	TUstpFtdcVolumeType	ShortPosLimit;
	///跌停板价
	TUstpFtdcPriceType	LowerLimitPrice;
	///涨停板价
	TUstpFtdcPriceType	UpperLimitPrice;
	///昨结算
	TUstpFtdcPriceType	PreSettlementPrice;
	///合约交易状态
	TUstpFtdcInstrumentStatusType	InstrumentStatus;
	///创建日
	TUstpFtdcDateType	CreateDate;
	///上市日
	TUstpFtdcDateType	OpenDate;
	///到期日
	TUstpFtdcDateType	ExpireDate;
	///开始交割日
	TUstpFtdcDateType	StartDelivDate;
	///最后交割日
	TUstpFtdcDateType	EndDelivDate;
	///挂牌基准价
	TUstpFtdcPriceType	BasisPrice;
	///当前是否交易
	TUstpFtdcBoolType	IsTrading;
	///基础商品代码
	TUstpFtdcInstrumentIDType	UnderlyingInstrID;
	///基础商品乘数
	TUstpFtdcUnderlyingMultipleType	UnderlyingMultiple;
	///持仓类型
	TUstpFtdcPositionTypeType	PositionType;
	///执行价
	TUstpFtdcPriceType	StrikePrice;
	///期权类型
	TUstpFtdcOptionsTypeType	OptionsType;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///策略类别
	TUstpFtdcArbiTypeType	ArbiType;
	///第一腿合约代码
	TUstpFtdcInstrumentIDType	InstrumentID_1;
	///第一腿买卖方向
	TUstpFtdcDirectionType	Direction_1;
	///第一腿数量比例
	TUstpFtdcRatioType	Ratio_1;
	///第二腿合约代码
	TUstpFtdcInstrumentIDType	InstrumentID_2;
	///第二腿买卖方向
	TUstpFtdcDirectionType	Direction_2;
	///第二腿数量比例
	TUstpFtdcRatioType	Ratio_2;
	///进入本状态日期
	TUstpFtdcDateType	EnterDate;
};
///投资者资金查询
struct CUstpFtdcQryInvestorAccountField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
};
///投资者资金应答
struct CUstpFtdcRspInvestorAccountField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///上次结算准备金
	TUstpFtdcMoneyType	PreBalance;
	///入金金额
	TUstpFtdcMoneyType	Deposit;
	///出金金额
	TUstpFtdcMoneyType	Withdraw;
	///冻结的保证金
	TUstpFtdcMoneyType	FrozenMargin;
	///冻结手续费
	TUstpFtdcMoneyType	FrozenFee;
	///冻结权利金
	TUstpFtdcMoneyType	FrozenPremium;
	///手续费
	TUstpFtdcMoneyType	Fee;
	///平仓盈亏
	TUstpFtdcMoneyType	CloseProfit;
	///持仓盈亏
	TUstpFtdcMoneyType	PositionProfit;
	///可用资金
	TUstpFtdcMoneyType	Available;
	///多头冻结的保证金
	TUstpFtdcMoneyType	LongFrozenMargin;
	///空头冻结的保证金
	TUstpFtdcMoneyType	ShortFrozenMargin;
	///多头占用保证金
	TUstpFtdcMoneyType	LongMargin;
	///空头占用保证金
	TUstpFtdcMoneyType	ShortMargin;
	///当日释放保证金
	TUstpFtdcMoneyType	ReleaseMargin;
	///动态权益
	TUstpFtdcMoneyType	DynamicRights;
	///今日出入金
	TUstpFtdcMoneyType	TodayInOut;
	///占用保证金
	TUstpFtdcMoneyType	Margin;
	///期权权利金收支
	TUstpFtdcMoneyType	Premium;
	///风险度
	TUstpFtdcMoneyType	Risk;
};
///可用投资者查询
struct CUstpFtdcQryUserInvestorField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
};
///可用投资者
struct CUstpFtdcRspUserInvestorField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
};
///交易编码查询
struct CUstpFtdcQryTradingCodeField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
};
///交易编码查询
struct CUstpFtdcRspTradingCodeField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///客户代码
	TUstpFtdcClientIDType	ClientID;
	///客户代码权限
	TUstpFtdcTradingRightType	ClientRight;
	///客户保值类型
	TUstpFtdcHedgeFlagType	ClientHedgeFlag;
	///是否活跃
	TUstpFtdcIsActiveType	IsActive;
};
///交易所查询请求
struct CUstpFtdcQryExchangeField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
};
///交易所查询应答
struct CUstpFtdcRspExchangeField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///交易所名称
	TUstpFtdcExchangeNameType	ExchangeName;
};
///投资者持仓查询请求
struct CUstpFtdcQryInvestorPositionField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
};
///投资者持仓查询应答
struct CUstpFtdcRspInvestorPositionField
{
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///客户代码
	TUstpFtdcClientIDType	ClientID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///占用保证金
	TUstpFtdcMoneyType	UsedMargin;
	///今总持仓量
	TUstpFtdcVolumeType	Position;
	///今日持仓成本
	TUstpFtdcPriceType	PositionCost;
	///昨持仓量
	TUstpFtdcVolumeType	YdPosition;
	///昨日持仓成本
	TUstpFtdcMoneyType	YdPositionCost;
	///冻结的保证金
	TUstpFtdcMoneyType	FrozenMargin;
	///开仓冻结持仓
	TUstpFtdcVolumeType	FrozenPosition;
	///平仓冻结持仓
	TUstpFtdcVolumeType	FrozenClosing;
	///平昨仓冻结持仓
	TUstpFtdcVolumeType	YdFrozenClosing;
	///冻结的权利金
	TUstpFtdcMoneyType	FrozenPremium;
	///最后一笔成交编号
	TUstpFtdcTradeIDType	LastTradeID;
	///最后一笔本地报单编号
	TUstpFtdcOrderLocalIDType	LastOrderLocalID;
	///投机持仓量
	TUstpFtdcVolumeType	SpeculationPosition;
	///套利持仓量
	TUstpFtdcVolumeType	ArbitragePosition;
	///套保持仓量
	TUstpFtdcVolumeType	HedgePosition;
	///投机平仓冻结量
	TUstpFtdcVolumeType	SpecFrozenClosing;
	///套保平仓冻结量
	TUstpFtdcVolumeType	HedgeFrozenClosing;
	///币种
	TUstpFtdcCurrencyIDType	Currency;
};
///合规参数查询请求
struct CUstpFtdcQryComplianceParamField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
};
///合规参数查询应答
struct CUstpFtdcRspComplianceParamField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///客户号
	TUstpFtdcClientIDType	ClientID;
	///每日最大报单笔
	TUstpFtdcVolumeType	DailyMaxOrder;
	///每日最大撤单笔
	TUstpFtdcVolumeType	DailyMaxOrderAction;
	///每日最大错单笔
	TUstpFtdcVolumeType	DailyMaxErrorOrder;
	///每日最大报单手
	TUstpFtdcVolumeType	DailyMaxOrderVolume;
	///每日最大撤单手
	TUstpFtdcVolumeType	DailyMaxOrderActionVolume;
};
///用户查询
struct CUstpFtdcQryUserField
{
	///交易用户代码
	TUstpFtdcUserIDType	StartUserID;
	///交易用户代码
	TUstpFtdcUserIDType	EndUserID;
};
///用户
struct CUstpFtdcUserField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///用户登录密码
	TUstpFtdcPasswordType	Password;
	///是否活跃
	TUstpFtdcIsActiveType	IsActive;
	///用户名称
	TUstpFtdcUserNameType	UserName;
	///用户类型
	TUstpFtdcUserTypeType	UserType;
	///营业部
	TUstpFtdcDepartmentType	Department;
	///授权功能集
	TUstpFtdcGrantFuncSetType	GrantFuncSet;
	///修改用户编号
	TUstpFtdcUserIDType	SetUserID;
	///操作日期
	TUstpFtdcDateType	CommandDate;
	///操作时间
	TUstpFtdcTimeType	CommandTime;
};
///投资者手续费率查询
struct CUstpFtdcQryInvestorFeeField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
};
///投资者手续费率
struct CUstpFtdcInvestorFeeField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///客户号
	TUstpFtdcClientIDType	ClientID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///品种代码
	TUstpFtdcProductIDType	ProductID;
	///开仓手续费按比例
	TUstpFtdcRatioType	OpenFeeRate;
	///开仓手续费按手数
	TUstpFtdcRatioType	OpenFeeAmt;
	///平仓手续费按比例
	TUstpFtdcRatioType	OffsetFeeRate;
	///平仓手续费按手数
	TUstpFtdcRatioType	OffsetFeeAmt;
	///平今仓手续费按比例
	TUstpFtdcRatioType	OTFeeRate;
	///平今仓手续费按手数
	TUstpFtdcRatioType	OTFeeAmt;
	///行权手续费按比例
	TUstpFtdcRatioType	ExecFeeRate;
	///行权手续费按手数
	TUstpFtdcRatioType	ExecFeeAmt;
	///每笔委托申报费
	TUstpFtdcRatioType	PerOrderAmt;
	///每笔撤单申报费
	TUstpFtdcRatioType	PerCancelAmt;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
};
///投资者保证金率查询
struct CUstpFtdcQryInvestorMarginField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
};
///投资者保证金率
struct CUstpFtdcInvestorMarginField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///客户号
	TUstpFtdcClientIDType	ClientID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///品种代码
	TUstpFtdcProductIDType	ProductID;
	///多头占用保证金按比例
	TUstpFtdcRatioType	LongMarginRate;
	///多头保证金按手数
	TUstpFtdcRatioType	LongMarginAmt;
	///空头占用保证金按比例
	TUstpFtdcRatioType	ShortMarginRate;
	///空头保证金按手数
	TUstpFtdcRatioType	ShortMarginAmt;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
};
///确认对账单
struct CUstpFtdcInputSettlementInfoConfirmField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///确认日期
	TUstpFtdcDateType	ConfirmDate;
	///确认时间
	TUstpFtdcTimeType	ConfirmTime;
};
///确认对账单信息
struct CUstpFtdcSettlementInfoConfirmField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///确认日期
	TUstpFtdcDateType	ConfirmDate;
	///确认时间
	TUstpFtdcTimeType	ConfirmTime;
};
///查询确认对账单信息
struct CUstpFtdcQrySettlementInfoConfirmField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
};
///成交
struct CUstpFtdcTradeField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///交易日
	TUstpFtdcTradingDayType	TradingDay;
	///会员编号
	TUstpFtdcParticipantIDType	ParticipantID;
	///下单席位号
	TUstpFtdcSeatIDType	SeatID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///客户号
	TUstpFtdcClientIDType	ClientID;
	///用户编号
	TUstpFtdcUserIDType	UserID;
	///下单用户编号
	TUstpFtdcUserIDType	OrderUserID;
	///成交编号
	TUstpFtdcTradeIDType	TradeID;
	///报单编号
	TUstpFtdcOrderSysIDType	OrderSysID;
	///本地报单编号
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
	///开平标志
	TUstpFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///成交价格
	TUstpFtdcPriceType	TradePrice;
	///成交数量
	TUstpFtdcVolumeType	TradeVolume;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///清算会员编号
	TUstpFtdcParticipantIDType	ClearingPartID;
	///本次成交手续费
	TUstpFtdcMoneyType	UsedFee;
	///本次成交占用保证金
	TUstpFtdcMoneyType	UsedMargin;
	///本次成交占用权利金
	TUstpFtdcMoneyType	Premium;
	///持仓表今持仓量
	TUstpFtdcVolumeType	Position;
	///持仓表今日持仓成本
	TUstpFtdcPriceType	PositionCost;
	///资金表可用资金
	TUstpFtdcMoneyType	Available;
	///资金表占用保证金
	TUstpFtdcMoneyType	Margin;
	///资金表冻结的保证金
	TUstpFtdcMoneyType	FrozenMargin;
	///本地业务标识
	TUstpFtdcBusinessLocalIDType	BusinessLocalID;
	///业务发生日期
	TUstpFtdcDateType	ActionDay;
	///策略类别
	TUstpFtdcArbiTypeType	ArbiType;
	///合约代码
	TUstpFtdcInstrumentIDType	ArbiInstrumentID;
};
///报单
struct CUstpFtdcOrderField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///系统报单编号
	TUstpFtdcOrderSysIDType	OrderSysID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///指定通过此席位序号下单
	TUstpFtdcSeatNoType	SeatNo;
	///合约代码/套利组合合约号
	TUstpFtdcInstrumentIDType	InstrumentID;
	///用户本地报单号
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
	///报单类型
	TUstpFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
	///开平标志
	TUstpFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///价格
	TUstpFtdcPriceType	LimitPrice;
	///数量
	TUstpFtdcVolumeType	Volume;
	///有效期类型
	TUstpFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TUstpFtdcDateType	GTDDate;
	///成交量类型
	TUstpFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TUstpFtdcVolumeType	MinVolume;
	///止损价止赢价
	TUstpFtdcPriceType	StopPrice;
	///强平原因
	TUstpFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TUstpFtdcBoolType	IsAutoSuspend;
	///业务单元
	TUstpFtdcBusinessUnitType	BusinessUnit;
	///用户自定义域
	TUstpFtdcCustomType	UserCustom;
	///本地业务标识
	TUstpFtdcBusinessLocalIDType	BusinessLocalID;
	///业务发生日期
	TUstpFtdcDateType	ActionDay;
	///策略类别
	TUstpFtdcArbiTypeType	ArbiType;
	///交易日
	TUstpFtdcTradingDayType	TradingDay;
	///会员编号
	TUstpFtdcParticipantIDType	ParticipantID;
	///最初下单用户编号
	TUstpFtdcUserIDType	OrderUserID;
	///客户号
	TUstpFtdcClientIDType	ClientID;
	///下单席位号
	TUstpFtdcSeatIDType	SeatID;
	///插入时间
	TUstpFtdcTimeType	InsertTime;
	///本地报单编号
	TUstpFtdcOrderLocalIDType	OrderLocalID;
	///报单来源
	TUstpFtdcOrderSourceType	OrderSource;
	///报单状态
	TUstpFtdcOrderStatusType	OrderStatus;
	///撤销时间
	TUstpFtdcTimeType	CancelTime;
	///撤单用户编号
	TUstpFtdcUserIDType	CancelUserID;
	///今成交数量
	TUstpFtdcVolumeType	VolumeTraded;
	///剩余数量
	TUstpFtdcVolumeType	VolumeRemain;
	///委托类型
	TUstpFtdcOrderTypeType	OrderType;
	///期权对冲标识
	TUstpFtdcDeliveryFlagType	DeliveryFlag;
};
///数据流回退
struct CUstpFtdcFlowMessageCancelField
{
	///序列系列号
	TUstpFtdcSequenceSeriesType	SequenceSeries;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///数据中心代码
	TUstpFtdcDataCenterIDType	DataCenterID;
	///回退起始序列号
	TUstpFtdcSequenceNoType	StartSequenceNo;
	///回退结束序列号
	TUstpFtdcSequenceNoType	EndSequenceNo;
};
///信息分发
struct CUstpFtdcDisseminationField
{
	///序列系列号
	TUstpFtdcSequenceSeriesType	SequenceSeries;
	///序列号
	TUstpFtdcSequenceNoType	SequenceNo;
};
///出入金结果
struct CUstpFtdcInvestorAccountDepositResField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///资金流水号
	TUstpFtdcAccountSeqNoType	AccountSeqNo;
	///金额
	TUstpFtdcMoneyType	Amount;
	///出入金方向
	TUstpFtdcAccountDirectionType	AmountDirection;
	///可用资金
	TUstpFtdcMoneyType	Available;
	///结算准备金
	TUstpFtdcMoneyType	Balance;
};
///报价录入
struct CUstpFtdcInputQuoteField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
	///交易系统返回的系统报价编号
	TUstpFtdcQuoteSysIDType	QuoteSysID;
	///用户设定的本地报价编号
	TUstpFtdcUserQuoteLocalIDType	UserQuoteLocalID;
	///飞马向交易系统报的本地报价编号
	TUstpFtdcQuoteLocalIDType	QuoteLocalID;
	///买方买入数量
	TUstpFtdcVolumeType	BidVolume;
	///买方开平标志
	TUstpFtdcOffsetFlagType	BidOffsetFlag;
	///买方投机套保标志
	TUstpFtdcHedgeFlagType	BidHedgeFlag;
	///买方买入价格
	TUstpFtdcPriceType	BidPrice;
	///卖方卖出数量
	TUstpFtdcVolumeType	AskVolume;
	///卖方开平标志
	TUstpFtdcOffsetFlagType	AskOffsetFlag;
	///卖方投机套保标志
	TUstpFtdcHedgeFlagType	AskHedgeFlag;
	///卖方卖出价格
	TUstpFtdcPriceType	AskPrice;
	///业务单元
	TUstpFtdcBusinessUnitType	BusinessUnit;
	///用户自定义域
	TUstpFtdcCustomType	UserCustom;
	///拆分出来的买方用户本地报单编号
	TUstpFtdcUserOrderLocalIDType	BidUserOrderLocalID;
	///拆分出来的卖方用户本地报单编号
	TUstpFtdcUserOrderLocalIDType	AskUserOrderLocalID;
	///拆分出来的买方本地报单编号
	TUstpFtdcOrderLocalIDType	BidOrderLocalID;
	///拆分出来的卖方本地报单编号
	TUstpFtdcOrderLocalIDType	AskOrderLocalID;
	///询价编号
	TUstpFtdcQuoteSysIDType	ReqForQuoteID;
	///报价停留时间(秒)
	TUstpFtdcMeasureSecType	StandByTime;
};
///报价通知
struct CUstpFtdcRtnQuoteField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
	///交易系统返回的系统报价编号
	TUstpFtdcQuoteSysIDType	QuoteSysID;
	///用户设定的本地报价编号
	TUstpFtdcUserQuoteLocalIDType	UserQuoteLocalID;
	///飞马向交易系统报的本地报价编号
	TUstpFtdcQuoteLocalIDType	QuoteLocalID;
	///买方买入数量
	TUstpFtdcVolumeType	BidVolume;
	///买方开平标志
	TUstpFtdcOffsetFlagType	BidOffsetFlag;
	///买方投机套保标志
	TUstpFtdcHedgeFlagType	BidHedgeFlag;
	///买方买入价格
	TUstpFtdcPriceType	BidPrice;
	///卖方卖出数量
	TUstpFtdcVolumeType	AskVolume;
	///卖方开平标志
	TUstpFtdcOffsetFlagType	AskOffsetFlag;
	///卖方投机套保标志
	TUstpFtdcHedgeFlagType	AskHedgeFlag;
	///卖方卖出价格
	TUstpFtdcPriceType	AskPrice;
	///业务单元
	TUstpFtdcBusinessUnitType	BusinessUnit;
	///用户自定义域
	TUstpFtdcCustomType	UserCustom;
	///拆分出来的买方用户本地报单编号
	TUstpFtdcUserOrderLocalIDType	BidUserOrderLocalID;
	///拆分出来的卖方用户本地报单编号
	TUstpFtdcUserOrderLocalIDType	AskUserOrderLocalID;
	///拆分出来的买方本地报单编号
	TUstpFtdcOrderLocalIDType	BidOrderLocalID;
	///拆分出来的卖方本地报单编号
	TUstpFtdcOrderLocalIDType	AskOrderLocalID;
	///询价编号
	TUstpFtdcQuoteSysIDType	ReqForQuoteID;
	///报价停留时间(秒)
	TUstpFtdcMeasureSecType	StandByTime;
	///买方系统报单编号
	TUstpFtdcQuoteSysIDType	BidOrderSysID;
	///卖方系统报单编号
	TUstpFtdcQuoteSysIDType	AskOrderSysID;
	///报价单状态
	TUstpFtdcQuoteStatusType	QuoteStatus;
	///插入时间
	TUstpFtdcTimeType	InsertTime;
	///撤销时间
	TUstpFtdcTimeType	CancelTime;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
};
///报价操作
struct CUstpFtdcQuoteActionField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///交易系统返回的系统报价编号
	TUstpFtdcQuoteSysIDType	QuoteSysID;
	///用户设定的被撤的本地报价编号
	TUstpFtdcUserQuoteLocalIDType	UserQuoteLocalID;
	///用户向飞马报的本地撤消报价编号
	TUstpFtdcUserQuoteLocalIDType	UserQuoteActionLocalID;
	///报单操作标志
	TUstpFtdcActionFlagType	ActionFlag;
	///业务单元
	TUstpFtdcBusinessUnitType	BusinessUnit;
	///用户自定义域
	TUstpFtdcCustomType	UserCustom;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
};
///询价请求
struct CUstpFtdcReqForQuoteField
{
	///询价编号
	TUstpFtdcQuoteSysIDType	ReqForQuoteID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///询价时间
	TUstpFtdcTimeType	ReqForQuoteTime;
};
///行情基础属性
struct CUstpFtdcMarketDataBaseField
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
};
///行情静态属性
struct CUstpFtdcMarketDataStaticField
{
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
};
///行情最新成交属性
struct CUstpFtdcMarketDataLastMatchField
{
	///最新价
	TUstpFtdcPriceType	LastPrice;
	///数量
	TUstpFtdcVolumeType	Volume;
	///成交金额
	TUstpFtdcMoneyType	Turnover;
	///持仓量
	TUstpFtdcLargeVolumeType	OpenInterest;
};
///行情最优价属性
struct CUstpFtdcMarketDataBestPriceField
{
	///申买价一
	TUstpFtdcPriceType	BidPrice1;
	///申买量一
	TUstpFtdcVolumeType	BidVolume1;
	///申卖价一
	TUstpFtdcPriceType	AskPrice1;
	///申卖量一
	TUstpFtdcVolumeType	AskVolume1;
};
///行情申买二、三属性
struct CUstpFtdcMarketDataBid23Field
{
	///申买价二
	TUstpFtdcPriceType	BidPrice2;
	///申买量二
	TUstpFtdcVolumeType	BidVolume2;
	///申买价三
	TUstpFtdcPriceType	BidPrice3;
	///申买量三
	TUstpFtdcVolumeType	BidVolume3;
};
///行情申卖二、三属性
struct CUstpFtdcMarketDataAsk23Field
{
	///申卖价二
	TUstpFtdcPriceType	AskPrice2;
	///申卖量二
	TUstpFtdcVolumeType	AskVolume2;
	///申卖价三
	TUstpFtdcPriceType	AskPrice3;
	///申卖量三
	TUstpFtdcVolumeType	AskVolume3;
};
///行情申买四、五属性
struct CUstpFtdcMarketDataBid45Field
{
	///申买价四
	TUstpFtdcPriceType	BidPrice4;
	///申买量四
	TUstpFtdcVolumeType	BidVolume4;
	///申买价五
	TUstpFtdcPriceType	BidPrice5;
	///申买量五
	TUstpFtdcVolumeType	BidVolume5;
};
///行情申卖四、五属性
struct CUstpFtdcMarketDataAsk45Field
{
	///申卖价四
	TUstpFtdcPriceType	AskPrice4;
	///申卖量四
	TUstpFtdcVolumeType	AskVolume4;
	///申卖价五
	TUstpFtdcPriceType	AskPrice5;
	///申卖量五
	TUstpFtdcVolumeType	AskVolume5;
};
///行情更新时间属性
struct CUstpFtdcMarketDataUpdateTimeField
{
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///最后修改时间
	TUstpFtdcTimeType	UpdateTime;
	///最后修改毫秒
	TUstpFtdcMillisecType	UpdateMillisec;
	///业务发生日期
	TUstpFtdcDateType	ActionDay;
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
	///业务发生日期
	TUstpFtdcDateType	ActionDay;
	///历史最高价
	TUstpFtdcPriceType	HisHighestPrice;
	///历史最低价
	TUstpFtdcPriceType	HisLowestPrice;
	///最新成交量
	TUstpFtdcVolumeType	LatestVolume;
	///初始持仓量
	TUstpFtdcVolumeType	InitVolume;
	///持仓量变化
	TUstpFtdcVolumeType	ChangeVolume;
	///申买推导量
	TUstpFtdcVolumeType	BidImplyVolume;
	///申卖推导量
	TUstpFtdcVolumeType	AskImplyVolume;
	///当日均价
	TUstpFtdcPriceType	AvgPrice;
	///策略类别
	TUstpFtdcArbiTypeType	ArbiType;
	///第一腿合约代码
	TUstpFtdcInstrumentIDType	InstrumentID_1;
	///第二腿合约代码
	TUstpFtdcInstrumentIDType	InstrumentID_2;
	///合约名称
	TUstpFtdcInstrumentIDType	InstrumentName;
	///总买入量
	TUstpFtdcVolumeType	TotalBidVolume;
	///总卖出量
	TUstpFtdcVolumeType	TotalAskVolume;
};
///订阅合约的相关信息
struct CUstpFtdcSpecificInstrumentField
{
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
};
///多播通道心跳
struct CUstpFtdcMultiChannelHeartBeatField
{
	///心跳超时时间（秒）
	TUstpFtdcVolumeType	UstpMultiChannelHeartBeatTimeOut;
};
///获取行情订阅号请求
struct CUstpFtdcReqMarketTopicField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
};
///获取行情订阅号应答
struct CUstpFtdcRspMarketTopicField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///订阅号
	TUstpFtdcSequenceSeriesType	TopicID;
};
///申请组合
struct CUstpFtdcInputMarginCombActionField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///交易用户代码
	TUstpFtdcUserIDType	UserID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///用户本地编号
	TUstpFtdcUserOrderLocalIDType	UserActionLocalID;
	///组合合约代码
	TUstpFtdcInstrumentIDType	CombInstrumentID;
	///组合数量
	TUstpFtdcVolumeType	CombVolume;
	///组合动作方向
	TUstpFtdcCombDirectionType	CombDirection;
	///本地编号
	TUstpFtdcOrderLocalIDType	ActionLocalID;
};
///交易编码组合持仓查询
struct CUstpFtdcQryInvestorCombPositionField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///组合合约代码
	TUstpFtdcInstrumentIDType	CombInstrumentID;
};
///交易编码组合持仓查询应答
struct CUstpFtdcRspInvestorCombPositionField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///持仓方向
	TUstpFtdcDirectionType	Direction;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///客户代码
	TUstpFtdcClientIDType	ClientID;
	///组合合约代码
	TUstpFtdcInstrumentIDType	CombInstrumentID;
	///单腿1合约代码
	TUstpFtdcInstrumentIDType	Leg1InstrumentID;
	///单腿2合约代码
	TUstpFtdcInstrumentIDType	Leg2InstrumentID;
	///组合持仓
	TUstpFtdcVolumeType	CombPosition;
	///冻结组合持仓
	TUstpFtdcVolumeType	CombFrozenPosition;
	///组合一手释放的保证金
	TUstpFtdcMoneyType	CombFreeMargin;
};
///组合规则
struct CUstpFtdcMarginCombinationLegField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///组合合约代码
	TUstpFtdcInstrumentIDType	CombInstrumentID;
	///单腿编号
	TUstpFtdcLegIDType	LegID;
	///单腿合约代码
	TUstpFtdcInstrumentIDType	LegInstrumentID;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
	///单腿乘数
	TUstpFtdcLegMultipleType	LegMultiple;
	///优先级
	TUstpFtdcPriorityType	Priority;
};
///交易编码单腿持仓查询
struct CUstpFtdcQryInvestorLegPositionField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///单腿合约代码
	TUstpFtdcInstrumentIDType	LegInstrumentID;
};
///交易编码单腿持仓查询应答
struct CUstpFtdcRspInvestorLegPositionField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///客户代码
	TUstpFtdcClientIDType	ClientID;
	///单腿合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///多头持仓
	TUstpFtdcVolumeType	LongPosition;
	///空头持仓
	TUstpFtdcVolumeType	ShortPosition;
	///多头占用保证金
	TUstpFtdcMoneyType	LongMargin;
	///空头占用保证金
	TUstpFtdcMoneyType	ShortMargin;
	///多头冻结持仓
	TUstpFtdcVolumeType	LongFrozenPosition;
	///空头冻结持仓
	TUstpFtdcVolumeType	ShortFrozenPosition;
	///多头冻结保证金
	TUstpFtdcMoneyType	LongFrozenMargin;
	///空头冻结保证金
	TUstpFtdcMoneyType	ShortFrozenMargin;
};
///查询合约组信息
struct CUstpFtdcQryUstpInstrumentGroupField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
};
///合约组信息查询应答
struct CUstpFtdcRspInstrumentGroupField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///合约组代码
	TUstpFtdcInstrumentGroupIDType	InstrumentGroupID;
};
///查询组合保证金类型
struct CUstpFtdcQryClientMarginCombTypeField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///合约组代码
	TUstpFtdcInstrumentGroupIDType	InstrumentGroupID;
};
///组合保证金类型查询应答
struct CUstpFtdcRspClientMarginCombTypeField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///会员代码
	TUstpFtdcParticipantIDType	ParticipantID;
	///客户代码
	TUstpFtdcClientIDType	ClientID;
	///合约组代码
	TUstpFtdcInstrumentGroupIDType	InstrumentGroupID;
	///保证金组合类型
	TUstpFtdcClientMarginCombTypeType	MarginCombType;
};
///系统时间
struct CUstpFtdcReqQrySystemTimeField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
};
///系统时间
struct CUstpFtdcRspQrySystemTimeField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///系统时间
	TUstpFtdcTimeType	SystemTime;
};
///风控登录请求
struct CUstpFtdcReqRiskLoginField
{
	///风控代码
	TUstpFtdcRiskIDType	RiskID;
	///密码
	TUstpFtdcPasswordType	Password;
};
///风控登录应答
struct CUstpFtdcRspRiskLoginField
{
	///风控代码
	TUstpFtdcRiskIDType	RiskID;
	///密码
	TUstpFtdcPasswordType	Password;
};
///风控登出请求
struct CUstpFtdcReqRiskLogoutField
{
	///风控代码
	TUstpFtdcRiskIDType	RiskID;
};
///风控登出请求
struct CUstpFtdcRspRiskLogoutField
{
	///风控代码
	TUstpFtdcRiskIDType	RiskID;
};
///投资者交易权限查询
struct CUstpFtdcQryInvestorTradingRightField
{
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
};
///投资者交易权限
struct CUstpFtdcInvestorTradingRightField
{
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///交易权限
	TUstpFtdcTradingRightType	TradingRight;
};
///席位可用资金查询
struct CUstpFtdcQrySeatAvailableMoneyField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///席位号
	TUstpFtdcSeatIDType	SeatID;
};
///席位可用资金
struct CUstpFtdcSeatAvailableMoneyField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///席位号
	TUstpFtdcSeatIDType	SeatID;
	///席位可用资金
	TUstpFtdcMoneyType	Available;
};
///风险提示消息
struct CUstpFtdcRiskWarningField
{
	///消息编号
	TUstpFtdcRiskWarningNoType	RiskWarningNo;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///消息类型
	TUstpFtdcRiskWarningTypeType	RiskWarningType;
	///需确认标志
	TUstpFtdcBoolType	NeedConfirmation;
	///消息摘要
	TUstpFtdcRiskWarningAbstractType	Abstract;
	///消息内容
	TUstpFtdcRiskWarningContentType	Content;
};
///风险提示消息应答
struct CUstpFtdcRspRiskWarningField
{
	///消息编号
	TUstpFtdcRiskWarningNoType	RiskWarningNo;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
};
///风险提示消息确认
struct CUstpFtdcRiskWarningConfirmationField
{
	///消息编号
	TUstpFtdcRiskWarningNoType	RiskWarningNo;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///IP地址
	TUstpFtdcIPAddressType	IPAddress;
	///Mac地址
	TUstpFtdcMacAddressType	MacAddress;
};
///保证金监管系统经纪公司资金账户密钥请求
struct CUstpFtdcCFMMCTradingAccountKeyReqField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
};
///保证金监管系统经纪公司资金账户密钥应答
struct CUstpFtdcCFMMCTradingAccountKeyRspField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///会员编号
	TUstpFtdcParticipantIDType	ParticipantID;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///密钥编号
	TUstpFtdcSequenceNoTypeType	KeyID;
	///动态密钥
	TUstpFtdcCFMMCKeyType	CurrentKey;
};
///查询投资者结算结果请求
struct CUstpFtdcSettlementQryReqField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///交易日
	TUstpFtdcDateType	TradingDay;
};
///查询投资者结算结果应答
struct CUstpFtdcSettlementRspField
{
	///交易日
	TUstpFtdcDateType	TradingDay;
	///结算编号
	TUstpFtdcSettlementIDType	SettlementID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///序号
	TUstpFtdcSequenceNoType	SequenceNo;
	///消息内容
	TUstpFtdcLogStrType	Content;
};
///资金账户口令更新请求
struct CUstpFtdcTradingAccountPasswordUpdateReqField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///旧密码
	TUstpFtdcPasswordType	OldPassword;
	///新密码
	TUstpFtdcPasswordType	NewPassword;
};
///资金账户口令更新请求响应
struct CUstpFtdcTradingAccountPasswordUpdateRspField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///旧密码
	TUstpFtdcPasswordType	OldPassword;
	///新密码
	TUstpFtdcPasswordType	NewPassword;
};
///查询转帐银行请求
struct CUstpFtdcTransferBankQryReqField
{
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
};
///查询转帐银行响应
struct CUstpFtdcTransferBankQryRspField
{
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///银行名称
	TUstpFtdcBankNameType	BankName;
	///是否活跃
	TUstpFtdcBoolType	isActive;
};
///查询仓单折抵信息请求
struct CUstpFtdcEWarrantOffsetFieldReqField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
};
///查询仓单折抵信息响应
struct CUstpFtdcEWarrantOffsetFieldRspField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TUstpFtdcDirectionType	Direction;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///数量
	TUstpFtdcVolumeType	Volume;
};
///查询转帐流水请求
struct CUstpFtdcTransferSerialFieldReqField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
};
///查询转帐流水响应响应
struct CUstpFtdcTransferSerialFieldRspField
{
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///交易发起方日期
	TUstpFtdcDateType	TradeDate;
	///交易日期
	TUstpFtdcDateType	TradingDay;
	///交易时间
	TUstpFtdcTimeType	TradeTime;
	///交易代码
	TUstpFtdcTradeCodeType	TradeCode;
	///会话编号
	TUstpFtdcSessionIDType	SessionID;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///期货公司帐号类型
	TUstpFtdcFutureAccType	FutureAccType;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///期货公司流水号
	TUstpFtdcFutureSerialType	FutureSerial;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///金额
	TUstpFtdcMoneyType	TradeAmount;
	///应收客户费用
	TUstpFtdcMoneyType	CustFee;
	///应收期货公司费用
	TUstpFtdcMoneyType	BrokerFee;
	///有效标志
	TUstpFtdcAvailabilityFlagTypeType	AvailabilityFlag;
	///操作员
	TUstpFtdcOperatorCodeType	OperatorCode;
	///新银行帐号
	TUstpFtdcBankAccountType	BankNewAccount;
};
///查询银期签约关系
struct CUstpFtdcAccountregisterReqField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
};
///查询银期签约关系响应
struct CUstpFtdcAccountregisterRspField
{
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///开销户类别
	TUstpFtdcOpenOrDestroyTypeType	OpenOrDestroy;
	///签约日期
	TUstpFtdcDateType	RegDate;
	///解约日期
	TUstpFtdcDateType	OutDate;
	///交易ID
	TUstpFtdcTIDType	TID;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
};
///查询签约银行请求
struct CUstpFtdcContractBankFieldReqField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
};
///查询签约银行响应
struct CUstpFtdcContractBankFieldRspField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///银行名称
	TUstpFtdcBankNameType	BankName;
};
///转账请求
struct CUstpFtdcTransferFieldReqField
{
	///业务功能码
	TUstpFtdcTradeCodeType	TradeCode;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///最后分片标志
	TUstpFtdcBoolType	LastFragment;
	///登录的SessionID
	TUstpFtdcSessionIDType	SessionID;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行密码
	TUstpFtdcPasswordType	BankPassWord;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///期货密码
	TUstpFtdcPasswordType	Password;
	///安装编号
	TUstpFtdcInstallIDType	InstallID;
	///期货公司流水号
	TUstpFtdcFutureSerialType	FutureSerial;
	///用户标识
	TUstpFtdcUserIDType	UserID;
	///客户证件号码标志
	TUstpFtdcBoolType	VerifyCertNoFlag;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TUstpFtdcMoneyType	TradeAmount;
	///期货可取金额
	TUstpFtdcMoneyType	FutureFetchAmount;
	///费用支付标志
	TUstpFtdcFeePayFlagType	FeePayFlag;
	///应收客户费用
	TUstpFtdcMoneyType	CustFee;
	///应收期货公司费用
	TUstpFtdcMoneyType	BrokerFee;
	///发送方给接收方的消息
	TUstpFtdcMessageType	Message;
	///摘要
	TUstpFtdcMessageType	Digest;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///渠道标志
	TUstpFtdcDeviceIdType	DeviceID;
	///期货单位帐号类型
	TUstpFtdcBankAccType	BankSecuAccType;
	///期货公司银行编码
	TUstpFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TUstpFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TUstpFtdcPwdFlagTypeType	BankPwdFlag;
	///期货资金密码核对标志
	TUstpFtdcPwdFlagTypeType	SecuPwdFlag;
	///交易柜员
	TUstpFtdcOperNoType	OperNo;
	///请求编号
	TUstpFtdcRequestIDType	RequestID;
	///交易ID
	TUstpFtdcTIDType	TID;
	///转账交易状态
	TUstpFtdcTransferStatusType	TransferStatus;
};
///查询账户信息请求
struct CUstpFtdcAccountFieldReqField
{
	///业务功能码
	TUstpFtdcTradeCodeType	TradeCode;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///最后分片标志
	TUstpFtdcBoolType	LastFragment;
	///登录的SessionID
	TUstpFtdcSessionIDType	SessionID;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行密码
	TUstpFtdcPasswordType	BankPassWord;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///期货密码
	TUstpFtdcPasswordType	Password;
	///安装编号
	TUstpFtdcInstallIDType	InstallID;
	///期货公司流水号
	TUstpFtdcFutureSerialType	FutureSerial;
	///用户标识
	TUstpFtdcUserIDType	UserID;
	///客户证件号码标志
	TUstpFtdcBoolType	VerifyCertNoFlag;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///摘要
	TUstpFtdcMessageType	Digest;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///渠道标志
	TUstpFtdcDeviceIdType	DeviceID;
	///期货单位帐号类型
	TUstpFtdcBankAccType	BankSecuAccType;
	///期货公司银行编码
	TUstpFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TUstpFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TUstpFtdcPwdFlagTypeType	BankPwdFlag;
	///期货资金密码核对标志
	TUstpFtdcPwdFlagTypeType	SecuPwdFlag;
	///交易柜员
	TUstpFtdcOperNoType	OperNo;
	///请求编号
	TUstpFtdcRequestIDType	RequestID;
	///交易ID
	TUstpFtdcTIDType	TID;
};
///查询账户信息通知
struct CUstpFtdcAccountFieldRtnField
{
	///业务功能码
	TUstpFtdcTradeCodeType	TradeCode;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///最后分片标志
	TUstpFtdcBoolType	LastFragment;
	///登录的SessionID
	TUstpFtdcSessionIDType	SessionID;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行密码
	TUstpFtdcPasswordType	BankPassWord;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///期货密码
	TUstpFtdcPasswordType	Password;
	///安装编号
	TUstpFtdcInstallIDType	InstallID;
	///期货公司流水号
	TUstpFtdcFutureSerialType	FutureSerial;
	///用户标识
	TUstpFtdcUserIDType	UserID;
	///客户证件号码标志
	TUstpFtdcBoolType	VerifyCertNoFlag;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///摘要
	TUstpFtdcMessageType	Digest;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///渠道标志
	TUstpFtdcDeviceIdType	DeviceID;
	///期货单位帐号类型
	TUstpFtdcBankAccType	BankSecuAccType;
	///期货公司银行编码
	TUstpFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TUstpFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TUstpFtdcPwdFlagTypeType	BankPwdFlag;
	///期货资金密码核对标志
	TUstpFtdcPwdFlagTypeType	SecuPwdFlag;
	///交易柜员
	TUstpFtdcOperNoType	OperNo;
	///请求编号
	TUstpFtdcRequestIDType	RequestID;
	///交易ID
	TUstpFtdcTIDType	TID;
	///银行可用金额
	TUstpFtdcMoneyType	BankUseAmount;
	///银行可取金额
	TUstpFtdcMoneyType	BankFetchAmount;
};
///冲正信息
struct CUstpFtdcRepealFieldRspField
{
	///冲正时间间隔
	TUstpFtdcRepealTimeIntervalType	RepealTimeInterval;
	///已经冲正次数
	TUstpFtdcRepealedTimesType	RepealedTimes;
	///银行冲正标志
	TUstpFtdcBankRepealFlagType	BankRepealFlag;
	///已经冲正次数
	TUstpFtdcBrokerRepealFlagType	BrokerRepealFlag;
	///被冲正平台流水号
	TUstpFtdcPlateSerialType	PlateRepealSerial;
	///被冲正银行流水号
	TUstpFtdcBankSerialType	BankRepealSerial;
	///被冲正期货公司流水号
	TUstpFtdcFutureSerialType	FutureRepealSerial;
	///业务功能码
	TUstpFtdcTradeCodeType	TradeCode;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///最后分片标志
	TUstpFtdcBoolType	LastFragment;
	///登录的SessionID
	TUstpFtdcSessionIDType	SessionID;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行密码
	TUstpFtdcPasswordType	BankPassWord;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///期货密码
	TUstpFtdcPasswordType	Password;
	///安装编号
	TUstpFtdcInstallIDType	InstallID;
	///期货公司流水号
	TUstpFtdcFutureSerialType	FutureSerial;
	///用户标识
	TUstpFtdcUserIDType	UserID;
	///客户证件号码标志
	TUstpFtdcBoolType	VerifyCertNoFlag;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TUstpFtdcMoneyType	TradeAmount;
	///期货可取金额
	TUstpFtdcMoneyType	FutureFetchAmount;
	///费用支付标志
	TUstpFtdcFeePayFlagType	FeePayFlag;
	///应收客户费用
	TUstpFtdcMoneyType	CustFee;
	///应收期货公司费用
	TUstpFtdcMoneyType	BrokerFee;
	///发送方给接收方的消息
	TUstpFtdcMessageType	Message;
	///摘要
	TUstpFtdcMessageType	Digest;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///渠道标志
	TUstpFtdcDeviceIdType	DeviceID;
	///期货单位帐号类型
	TUstpFtdcBankAccType	BankSecuAccType;
	///期货公司银行编码
	TUstpFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TUstpFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TUstpFtdcPwdFlagTypeType	BankPwdFlag;
	///期货资金密码核对标志
	TUstpFtdcPwdFlagTypeType	SecuPwdFlag;
	///交易柜员
	TUstpFtdcOperNoType	OperNo;
	///请求编号
	TUstpFtdcRequestIDType	RequestID;
	///交易ID
	TUstpFtdcTIDType	TID;
	///转账交易状态
	TUstpFtdcTransferStatusType	TransferStatus;
};
///银期开销户信息
struct CUstpFtdcAccountFieldField
{
	///业务功能码
	TUstpFtdcTradeCodeType	TradeCode;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///最后分片标志
	TUstpFtdcBoolType	LastFragment;
	///登录的SessionID
	TUstpFtdcSessionIDType	SessionID;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///性别
	TUstpFtdcGenderType	Gender;
	///国家代码
	TUstpFtdcCountryCodeType	CountryCode;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///地址
	TUstpFtdcAddressType	Address;
	///邮政编码类型
	TUstpFtdcZipCodeType	ZipCode;
	///电话号码
	TUstpFtdcTelephoneType	Telephone;
	///手机
	TUstpFtdcMobilePhoneType	MobilePhone;
	///传真
	TUstpFtdcFaxType	Fax;
	///电子邮件
	TUstpFtdcEMailType	EMail;
	///资金账户状态
	TUstpFtdcMoneyAccountStatusType	MoneyAccountStatus;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行密码
	TUstpFtdcPasswordType	BankPassWord;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///期货密码
	TUstpFtdcPasswordType	Password;
	///安装编号
	TUstpFtdcInstallIDType	InstallID;
	///客户证件号码标志
	TUstpFtdcBoolType	VerifyCertNoFlag;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///汇钞标志
	TUstpFtdcCashExchangeCodeType	CashExchangeCode;
	///摘要
	TUstpFtdcMessageType	Digest;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///渠道标志
	TUstpFtdcDeviceIdType	DeviceID;
	///期货单位帐号类型
	TUstpFtdcBankAccType	BankSecuAccType;
	///期货公司银行编码
	TUstpFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TUstpFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TUstpFtdcPwdFlagTypeType	BankPwdFlag;
	///期货资金密码核对标志
	TUstpFtdcPwdFlagTypeType	SecuPwdFlag;
	///交易柜员
	TUstpFtdcOperNoType	OperNo;
	///交易ID
	TUstpFtdcTIDType	TID;
	///错误代码
	TUstpFtdcErrorIDType	ErrorID;
	///错误信息
	TUstpFtdcErrorMsgType	ErrorMsg;
};
///银期变更银行账号信息
struct CUstpFtdcChangeAccountFieldField
{
	///业务功能码
	TUstpFtdcTradeCodeType	TradeCode;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///最后分片标志
	TUstpFtdcBoolType	LastFragment;
	///登录的SessionID
	TUstpFtdcSessionIDType	SessionID;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///性别
	TUstpFtdcGenderType	Gender;
	///国家代码
	TUstpFtdcCountryCodeType	CountryCode;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///地址
	TUstpFtdcAddressType	Address;
	///邮政编码类型
	TUstpFtdcZipCodeType	ZipCode;
	///电话号码
	TUstpFtdcTelephoneType	Telephone;
	///手机
	TUstpFtdcMobilePhoneType	MobilePhone;
	///传真
	TUstpFtdcFaxType	Fax;
	///电子邮件
	TUstpFtdcEMailType	EMail;
	///资金账户状态
	TUstpFtdcMoneyAccountStatusType	MoneyAccountStatus;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行密码
	TUstpFtdcPasswordType	BankPassWord;
	///银行帐号
	TUstpFtdcBankAccountType	NewBankAccount;
	///银行密码
	TUstpFtdcPasswordType	NewBankPassWord;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///期货密码
	TUstpFtdcPasswordType	Password;
	///安装编号
	TUstpFtdcInstallIDType	InstallID;
	///客户证件号码标志
	TUstpFtdcBoolType	VerifyCertNoFlag;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///摘要
	TUstpFtdcMessageType	Digest;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///期货公司银行编码
	TUstpFtdcBankCodingForFutureType	BrokerIDByBank;
	///银行密码标志
	TUstpFtdcPwdFlagTypeType	BankPwdFlag;
	///期货资金密码核对标志
	TUstpFtdcPwdFlagTypeType	SecuPwdFlag;
	///交易ID
	TUstpFtdcTIDType	TID;
	///错误代码
	TUstpFtdcErrorIDType	ErrorID;
	///错误信息
	TUstpFtdcErrorMsgType	ErrorMsg;
};
///银行发起银行资金转期货通知
struct CUstpFtdcTransferFieldRtnField
{
	///业务功能码
	TUstpFtdcTradeCodeType	TradeCode;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///最后分片标志
	TUstpFtdcBoolType	LastFragment;
	///登录的SessionID
	TUstpFtdcSessionIDType	SessionID;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行密码
	TUstpFtdcPasswordType	BankPassWord;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///期货密码
	TUstpFtdcPasswordType	Password;
	///安装编号
	TUstpFtdcInstallIDType	InstallID;
	///期货公司流水号
	TUstpFtdcFutureSerialType	FutureSerial;
	///用户标识
	TUstpFtdcUserIDType	UserID;
	///客户证件号码标志
	TUstpFtdcBoolType	VerifyCertNoFlag;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TUstpFtdcMoneyType	TradeAmount;
	///期货可取金额
	TUstpFtdcMoneyType	FutureFetchAmount;
	///费用支付标志
	TUstpFtdcFeePayFlagType	FeePayFlag;
	///应收客户费用
	TUstpFtdcMoneyType	CustFee;
	///应收期货公司费用
	TUstpFtdcMoneyType	BrokerFee;
	///发送方给接收方的消息
	TUstpFtdcMessageType	Message;
	///摘要
	TUstpFtdcMessageType	Digest;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///渠道标志
	TUstpFtdcDeviceIdType	DeviceID;
	///期货单位帐号类型
	TUstpFtdcBankAccType	BankSecuAccType;
	///期货公司银行编码
	TUstpFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TUstpFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TUstpFtdcPwdFlagTypeType	BankPwdFlag;
	///期货资金密码核对标志
	TUstpFtdcPwdFlagTypeType	SecuPwdFlag;
	///交易柜员
	TUstpFtdcOperNoType	OperNo;
	///请求编号
	TUstpFtdcRequestIDType	RequestID;
	///交易ID
	TUstpFtdcTIDType	TID;
	///转账交易状态
	TUstpFtdcTransferStatusType	TransferStatus;
	///错误代码
	TUstpFtdcErrorIDType	ErrorID;
	///错误信息
	TUstpFtdcErrorMsgType	ErrorMsg;
};
///查询账户信息错误通知
struct CUstpFtdcAccountFieldErrRtnField
{
	///业务功能码
	TUstpFtdcTradeCodeType	TradeCode;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///最后分片标志
	TUstpFtdcBoolType	LastFragment;
	///登录的SessionID
	TUstpFtdcSessionIDType	SessionID;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行密码
	TUstpFtdcPasswordType	BankPassWord;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///期货密码
	TUstpFtdcPasswordType	Password;
	///安装编号
	TUstpFtdcInstallIDType	InstallID;
	///期货公司流水号
	TUstpFtdcFutureSerialType	FutureSerial;
	///用户标识
	TUstpFtdcUserIDType	UserID;
	///客户证件号码标志
	TUstpFtdcBoolType	VerifyCertNoFlag;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///摘要
	TUstpFtdcMessageType	Digest;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///渠道标志
	TUstpFtdcDeviceIdType	DeviceID;
	///期货单位帐号类型
	TUstpFtdcBankAccType	BankSecuAccType;
	///期货公司银行编码
	TUstpFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TUstpFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TUstpFtdcPwdFlagTypeType	BankPwdFlag;
	///期货资金密码核对标志
	TUstpFtdcPwdFlagTypeType	SecuPwdFlag;
	///交易柜员
	TUstpFtdcOperNoType	OperNo;
	///请求编号
	TUstpFtdcRequestIDType	RequestID;
	///交易ID
	TUstpFtdcTIDType	TID;
	///错误代码
	TUstpFtdcErrorIDType	ErrorID;
	///错误信息
	TUstpFtdcErrorMsgType	ErrorMsg;
};
///冲正通知
struct CUstpFtdcRepealFieldRtnField
{
	///冲正时间间隔
	TUstpFtdcRepealTimeIntervalType	RepealTimeInterval;
	///已经冲正次数
	TUstpFtdcRepealedTimesType	RepealedTimes;
	///银行冲正标志
	TUstpFtdcBankRepealFlagType	BankRepealFlag;
	///已经冲正次数
	TUstpFtdcBrokerRepealFlagType	BrokerRepealFlag;
	///被冲正平台流水号
	TUstpFtdcPlateSerialType	PlateRepealSerial;
	///被冲正银行流水号
	TUstpFtdcBankSerialType	BankRepealSerial;
	///被冲正期货公司流水号
	TUstpFtdcFutureSerialType	FutureRepealSerial;
	///业务功能码
	TUstpFtdcTradeCodeType	TradeCode;
	///银行代码
	TUstpFtdcBrokerIDType	BankID;
	///银行分中心代码
	TUstpFtdcInvestorIDType	BankBrchID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TUstpFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TUstpFtdcDateType	TradeDate;
	///成交时间
	TUstpFtdcTimeType	TradeTime;
	///银行流水号
	TUstpFtdcBankSerialType	BankSerial;
	///交易日
	TUstpFtdcDateType	TradingDay;
	///平台流水号
	TUstpFtdcPlateSerialType	PlateSerial;
	///最后分片标志
	TUstpFtdcBoolType	LastFragment;
	///登录的SessionID
	TUstpFtdcSessionIDType	SessionID;
	///客户姓名
	TUstpFtdcIndividualNameType	CustomerName;
	///证件类型
	TUstpFtdcIdCardTypeType	IdCardType;
	///证件号码
	TUstpFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TUstpFtdcCustTypeType	CustType;
	///银行帐号
	TUstpFtdcBankAccountType	BankAccount;
	///银行密码
	TUstpFtdcPasswordType	BankPassWord;
	///资金帐号
	TUstpFtdcAccountIDType	AccountID;
	///期货密码
	TUstpFtdcPasswordType	Password;
	///安装编号
	TUstpFtdcInstallIDType	InstallID;
	///期货公司流水号
	TUstpFtdcFutureSerialType	FutureSerial;
	///用户标识
	TUstpFtdcUserIDType	UserID;
	///客户证件号码标志
	TUstpFtdcBoolType	VerifyCertNoFlag;
	///币种代码
	TUstpFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TUstpFtdcMoneyType	TradeAmount;
	///期货可取金额
	TUstpFtdcMoneyType	FutureFetchAmount;
	///费用支付标志
	TUstpFtdcFeePayFlagType	FeePayFlag;
	///应收客户费用
	TUstpFtdcMoneyType	CustFee;
	///应收期货公司费用
	TUstpFtdcMoneyType	BrokerFee;
	///发送方给接收方的消息
	TUstpFtdcMessageType	Message;
	///摘要
	TUstpFtdcMessageType	Digest;
	///银行帐号类型
	TUstpFtdcBankAccType	BankAccType;
	///渠道标志
	TUstpFtdcDeviceIdType	DeviceID;
	///期货单位帐号类型
	TUstpFtdcBankAccType	BankSecuAccType;
	///期货公司银行编码
	TUstpFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TUstpFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TUstpFtdcPwdFlagTypeType	BankPwdFlag;
	///期货资金密码核对标志
	TUstpFtdcPwdFlagTypeType	SecuPwdFlag;
	///交易柜员
	TUstpFtdcOperNoType	OperNo;
	///请求编号
	TUstpFtdcRequestIDType	RequestID;
	///交易ID
	TUstpFtdcTIDType	TID;
	///转账交易状态
	TUstpFtdcTransferStatusType	TransferStatus;
	///错误代码
	TUstpFtdcErrorIDType	ErrorID;
	///错误信息
	TUstpFtdcErrorMsgType	ErrorMsg;
};
///输入行权
struct CUstpFtdcInputExecOrderField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///系统报单编号
	TUstpFtdcOrderSysIDType	OrderSysID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///用户本地报单号
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
	///委托类型
	TUstpFtdcOrderTypeType	OrderType;
	///期权对冲标识
	TUstpFtdcDeliveryFlagType	DeliveryFlag;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///数量
	TUstpFtdcVolumeType	Volume;
	///用户自定义域
	TUstpFtdcCustomType	UserCustom;
	///业务发生日期
	TUstpFtdcDateType	ActionDay;
	///本地业务标识
	TUstpFtdcBusinessLocalIDType	BusinessLocalID;
	///业务单元
	TUstpFtdcBusinessUnitType	BusinessUnit;
};
///输入行权操作
struct CUstpFtdcInputExecOrderActionField
{
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///报单编号
	TUstpFtdcOrderSysIDType	OrderSysID;
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///本次撤单操作的本地编号
	TUstpFtdcUserOrderLocalIDType	UserOrderActionLocalID;
	///被撤订单的本地报单编号
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
	///报单操作标志
	TUstpFtdcActionFlagType	ActionFlag;
	///数量变化
	TUstpFtdcVolumeType	VolumeChange;
	///本地业务标识
	TUstpFtdcBusinessLocalIDType	BusinessLocalID;
	///委托类型
	TUstpFtdcOrderTypeType	OrderType;
};
///行权通知
struct CUstpFtdcExecOrderField
{
	///经纪公司编号
	TUstpFtdcBrokerIDType	BrokerID;
	///交易所代码
	TUstpFtdcExchangeIDType	ExchangeID;
	///系统报单编号
	TUstpFtdcOrderSysIDType	OrderSysID;
	///投资者编号
	TUstpFtdcInvestorIDType	InvestorID;
	///用户代码
	TUstpFtdcUserIDType	UserID;
	///合约代码
	TUstpFtdcInstrumentIDType	InstrumentID;
	///用户本地报单号
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
	///委托类型
	TUstpFtdcOrderTypeType	OrderType;
	///期权对冲标识
	TUstpFtdcDeliveryFlagType	DeliveryFlag;
	///投机套保标志
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///数量
	TUstpFtdcVolumeType	Volume;
	///用户自定义域
	TUstpFtdcCustomType	UserCustom;
	///业务发生日期
	TUstpFtdcDateType	ActionDay;
	///本地业务标识
	TUstpFtdcBusinessLocalIDType	BusinessLocalID;
	///业务单元
	TUstpFtdcBusinessUnitType	BusinessUnit;
	///交易日
	TUstpFtdcTradingDayType	TradingDay;
	///会员编号
	TUstpFtdcParticipantIDType	ParticipantID;
	///最初下单用户编号
	TUstpFtdcUserIDType	OrderUserID;
	///客户号
	TUstpFtdcClientIDType	ClientID;
	///下单席位号
	TUstpFtdcSeatIDType	SeatID;
	///插入时间
	TUstpFtdcTimeType	InsertTime;
	///本地报单编号
	TUstpFtdcOrderLocalIDType	OrderLocalID;
	///报单来源
	TUstpFtdcOrderSourceType	OrderSource;
	///报单状态
	TUstpFtdcOrderStatusType	OrderStatus;
	///撤销时间
	TUstpFtdcTimeType	CancelTime;
	///撤单用户编号
	TUstpFtdcUserIDType	CancelUserID;
};


#endif
