
#if !defined(ZEUSING_FTDCSTRUCT_H)
#define ZEUSING_FTDCSTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZeusingFtdcUserApiDataType.h"

#pragma pack(push)
#pragma pack(1)

///信息分发
struct CZeusingFtdcDisseminationField
{
	///序列系列号
	TZeusingFtdcSequenceSeriesType	SequenceSeries;
	///序列号
	TZeusingFtdcSequenceNoType	SequenceNo;
};

///用户登录请求
struct CZeusingFtdcReqUserLoginField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///密码
	TZeusingFtdcPasswordType	Password;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
	///接口端产品信息
	TZeusingFtdcProductInfoType	InterfaceProductInfo;
	///协议信息
	TZeusingFtdcProtocolInfoType	ProtocolInfo;
	///Mac地址
	TZeusingFtdcMacAddressType	MacAddress;
	///动态密码
	TZeusingFtdcPasswordType	OneTimePassword;
	///终端IP地址
	TZeusingFtdcIPAddressType	ClientIPAddress;
};

///用户登录应答
struct CZeusingFtdcRspUserLoginField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///登录成功时间
	TZeusingFtdcTimeType	LoginTime;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///交易系统名称
	TZeusingFtdcSystemNameType	SystemName;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///最大报单引用
	TZeusingFtdcOrderRefType	MaxOrderRef;
	///上期所时间
	TZeusingFtdcTimeType	SHFETime;
	///大商所时间
	TZeusingFtdcTimeType	DCETime;
	///郑商所时间
	TZeusingFtdcTimeType	CZCETime;
	///中金所时间
	TZeusingFtdcTimeType	FFEXTime;
};

///用户登出请求
struct CZeusingFtdcUserLogoutField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
};

///强制交易员退出
struct CZeusingFtdcForceUserLogoutField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
};

///客户端认证请求
struct CZeusingFtdcReqAuthenticateField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
	///认证码
	TZeusingFtdcAuthCodeType	AuthCode;
};

///客户端认证响应
struct CZeusingFtdcRspAuthenticateField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
};

///客户端认证信息
struct CZeusingFtdcAuthenticationInfoField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
	///认证信息
	TZeusingFtdcAuthInfoType	AuthInfo;
	///是否为认证结果
	TZeusingFtdcBoolType	IsResult;
};

///银期转帐报文头
struct CZeusingFtdcTransferHeaderField
{
	///版本号，常量，1.0
	TZeusingFtdcVersionType	Version;
	///交易代码，必填
	TZeusingFtdcTradeCodeType	TradeCode;
	///交易日期，必填，格式：yyyymmdd
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间，必填，格式：hhmmss
	TZeusingFtdcTradeTimeType	TradeTime;
	///发起方流水号，N/A
	TZeusingFtdcTradeSerialType	TradeSerial;
	///期货公司代码，必填
	TZeusingFtdcFutureIDType	FutureID;
	///银行代码，根据查询银行得到，必填
	TZeusingFtdcBankIDType	BankID;
	///银行分中心代码，根据查询银行得到，必填
	TZeusingFtdcBankBrchIDType	BankBrchID;
	///操作员，N/A
	TZeusingFtdcOperNoType	OperNo;
	///交易设备类型，N/A
	TZeusingFtdcDeviceIDType	DeviceID;
	///记录数，N/A
	TZeusingFtdcRecordNumType	RecordNum;
	///会话编号，N/A
	TZeusingFtdcSessionIDType	SessionID;
	///请求编号，N/A
	TZeusingFtdcRequestIDType	RequestID;
};

///银行资金转期货请求，TradeCode=202001
struct CZeusingFtdcTransferBankToFutureReqField
{
	///期货资金账户
	TZeusingFtdcAccountIDType	FutureAccount;
	///密码标志
	TZeusingFtdcFuturePwdFlagType	FuturePwdFlag;
	///密码
	TZeusingFtdcFutureAccPwdType	FutureAccPwd;
	///转账金额
	TZeusingFtdcMoneyType	TradeAmt;
	///客户手续费
	TZeusingFtdcMoneyType	CustFee;
	///币种：RMB-人民币 USD-美圆 HKD-港元
	TZeusingFtdcCurrencyCodeType	CurrencyCode;
};

///银行资金转期货请求响应
struct CZeusingFtdcTransferBankToFutureRspField
{
	///响应代码
	TZeusingFtdcRetCodeType	RetCode;
	///响应信息
	TZeusingFtdcRetInfoType	RetInfo;
	///资金账户
	TZeusingFtdcAccountIDType	FutureAccount;
	///转帐金额
	TZeusingFtdcMoneyType	TradeAmt;
	///应收客户手续费
	TZeusingFtdcMoneyType	CustFee;
	///币种
	TZeusingFtdcCurrencyCodeType	CurrencyCode;
};

///期货资金转银行请求，TradeCode=202002
struct CZeusingFtdcTransferFutureToBankReqField
{
	///期货资金账户
	TZeusingFtdcAccountIDType	FutureAccount;
	///密码标志
	TZeusingFtdcFuturePwdFlagType	FuturePwdFlag;
	///密码
	TZeusingFtdcFutureAccPwdType	FutureAccPwd;
	///转账金额
	TZeusingFtdcMoneyType	TradeAmt;
	///客户手续费
	TZeusingFtdcMoneyType	CustFee;
	///币种：RMB-人民币 USD-美圆 HKD-港元
	TZeusingFtdcCurrencyCodeType	CurrencyCode;
};

///期货资金转银行请求响应
struct CZeusingFtdcTransferFutureToBankRspField
{
	///响应代码
	TZeusingFtdcRetCodeType	RetCode;
	///响应信息
	TZeusingFtdcRetInfoType	RetInfo;
	///资金账户
	TZeusingFtdcAccountIDType	FutureAccount;
	///转帐金额
	TZeusingFtdcMoneyType	TradeAmt;
	///应收客户手续费
	TZeusingFtdcMoneyType	CustFee;
	///币种
	TZeusingFtdcCurrencyCodeType	CurrencyCode;
};

///查询银行资金请求，TradeCode=204002
struct CZeusingFtdcTransferQryBankReqField
{
	///期货资金账户
	TZeusingFtdcAccountIDType	FutureAccount;
	///密码标志
	TZeusingFtdcFuturePwdFlagType	FuturePwdFlag;
	///密码
	TZeusingFtdcFutureAccPwdType	FutureAccPwd;
	///币种：RMB-人民币 USD-美圆 HKD-港元
	TZeusingFtdcCurrencyCodeType	CurrencyCode;
};

///查询银行资金请求响应
struct CZeusingFtdcTransferQryBankRspField
{
	///响应代码
	TZeusingFtdcRetCodeType	RetCode;
	///响应信息
	TZeusingFtdcRetInfoType	RetInfo;
	///资金账户
	TZeusingFtdcAccountIDType	FutureAccount;
	///银行余额
	TZeusingFtdcMoneyType	TradeAmt;
	///银行可用余额
	TZeusingFtdcMoneyType	UseAmt;
	///银行可取余额
	TZeusingFtdcMoneyType	FetchAmt;
	///币种
	TZeusingFtdcCurrencyCodeType	CurrencyCode;
};

///查询银行交易明细请求，TradeCode=204999
struct CZeusingFtdcTransferQryDetailReqField
{
	///期货资金账户
	TZeusingFtdcAccountIDType	FutureAccount;
};

///查询银行交易明细请求响应
struct CZeusingFtdcTransferQryDetailRspField
{
	///交易日期
	TZeusingFtdcDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///交易代码
	TZeusingFtdcTradeCodeType	TradeCode;
	///期货流水号
	TZeusingFtdcTradeSerialNoType	FutureSerial;
	///期货公司代码
	TZeusingFtdcFutureIDType	FutureID;
	///资金帐号
	TZeusingFtdcFutureAccountType	FutureAccount;
	///银行流水号
	TZeusingFtdcTradeSerialNoType	BankSerial;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分中心代码
	TZeusingFtdcBankBrchIDType	BankBrchID;
	///银行账号
	TZeusingFtdcBankAccountType	BankAccount;
	///证件号码
	TZeusingFtdcCertCodeType	CertCode;
	///货币代码
	TZeusingFtdcCurrencyCodeType	CurrencyCode;
	///发生金额
	TZeusingFtdcMoneyType	TxAmount;
	///有效标志
	TZeusingFtdcTransferValidFlagType	Flag;
};

///响应信息
struct CZeusingFtdcRspInfoField
{
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///交易所
struct CZeusingFtdcExchangeField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///交易所名称
	TZeusingFtdcExchangeNameType	ExchangeName;
	///交易所属性
	TZeusingFtdcExchangePropertyType	ExchangeProperty;
};

///产品
struct CZeusingFtdcProductField
{
	///产品代码
	TZeusingFtdcInstrumentIDType	ProductID;
	///产品名称
	TZeusingFtdcProductNameType	ProductName;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///产品类型
	TZeusingFtdcProductClassType	ProductClass;
	///合约数量乘数
	TZeusingFtdcVolumeMultipleType	VolumeMultiple;
	///最小变动价位
	TZeusingFtdcPriceType	PriceTick;
	///市价单最大下单量
	TZeusingFtdcVolumeType	MaxMarketOrderVolume;
	///市价单最小下单量
	TZeusingFtdcVolumeType	MinMarketOrderVolume;
	///限价单最大下单量
	TZeusingFtdcVolumeType	MaxLimitOrderVolume;
	///限价单最小下单量
	TZeusingFtdcVolumeType	MinLimitOrderVolume;
	///持仓类型
	TZeusingFtdcPositionTypeType	PositionType;
	///持仓日期类型
	TZeusingFtdcPositionDateTypeType	PositionDateType;
	///平仓处理类型
	TZeusingFtdcCloseDealTypeType	CloseDealType;
};

///合约
struct CZeusingFtdcInstrumentField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///合约名称
	TZeusingFtdcInstrumentNameType	InstrumentName;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///产品代码
	TZeusingFtdcInstrumentIDType	ProductID;
	///产品类型
	TZeusingFtdcProductClassType	ProductClass;
	///交割年份
	TZeusingFtdcYearType	DeliveryYear;
	///交割月
	TZeusingFtdcMonthType	DeliveryMonth;
	///市价单最大下单量
	TZeusingFtdcVolumeType	MaxMarketOrderVolume;
	///市价单最小下单量
	TZeusingFtdcVolumeType	MinMarketOrderVolume;
	///限价单最大下单量
	TZeusingFtdcVolumeType	MaxLimitOrderVolume;
	///限价单最小下单量
	TZeusingFtdcVolumeType	MinLimitOrderVolume;
	///合约数量乘数
	TZeusingFtdcVolumeMultipleType	VolumeMultiple;
	///最小变动价位
	TZeusingFtdcPriceType	PriceTick;
	///创建日
	TZeusingFtdcDateType	CreateDate;
	///上市日
	TZeusingFtdcDateType	OpenDate;
	///到期日
	TZeusingFtdcDateType	ExpireDate;
	///开始交割日
	TZeusingFtdcDateType	StartDelivDate;
	///结束交割日
	TZeusingFtdcDateType	EndDelivDate;
	///合约生命周期状态
	TZeusingFtdcInstLifePhaseType	InstLifePhase;
	///当前是否交易
	TZeusingFtdcBoolType	IsTrading;
	///持仓类型
	TZeusingFtdcPositionTypeType	PositionType;
	///持仓日期类型
	TZeusingFtdcPositionDateTypeType	PositionDateType;
	///是否使用大额单边保证金算法
	TZeusingFtdcMaxMarginSideAlgorithmType	MaxMarginSideAlgorithm;
	///基础商品代码
	TZeusingFtdcInstrumentIDType	UnderlyingInstrID;
	///执行价
	TZeusingFtdcPriceType	StrikePrice;
	///期权类型
	TZeusingFtdcOptionsTypeType	OptionsType;
	///合约基础商品乘数
	TZeusingFtdcVolumeMultipleType	UnderlyingMultiple;
};

///经纪公司
struct CZeusingFtdcBrokerField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///经纪公司简称
	TZeusingFtdcBrokerAbbrType	BrokerAbbr;
	///经纪公司名称
	TZeusingFtdcBrokerNameType	BrokerName;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
};

///交易所交易员
struct CZeusingFtdcTraderField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///密码
	TZeusingFtdcPasswordType	Password;
	///安装数量
	TZeusingFtdcInstallCountType	InstallCount;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
};

///投资者
struct CZeusingFtdcInvestorField
{
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者分组代码
	TZeusingFtdcInvestorIDType	InvestorGroupID;
	///投资者名称
	TZeusingFtdcPartyNameType	InvestorName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdentifiedCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
	///联系电话
	TZeusingFtdcTelephoneType	Telephone;
	///通讯地址
	TZeusingFtdcAddressType	Address;
	///开户日期
	TZeusingFtdcDateType	OpenDate;
	///手机
	TZeusingFtdcMobileType	Mobile;
	///手续费率模板代码
	TZeusingFtdcInvestorIDType	CommModelID;
	///填充字段
	TZeusingFtdcPadFieldType	PadField;
	///保证金率模板代码
	TZeusingFtdcInvestorIDType	MarginModelID;
};

///交易编码
struct CZeusingFtdcTradingCodeField
{
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
	///交易编码类型
	TZeusingFtdcClientIDTypeType	ClientIDType;
};

///会员编码和经纪公司编码对照表
struct CZeusingFtdcPartBrokerField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
};

///管理用户
struct CZeusingFtdcSuperUserField
{
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///用户名称
	TZeusingFtdcUserNameType	UserName;
	///密码
	TZeusingFtdcPasswordType	Password;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
};

///管理用户功能权限
struct CZeusingFtdcSuperUserFunctionField
{
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///功能代码
	TZeusingFtdcFunctionCodeType	FunctionCode;
};

///投资者组
struct CZeusingFtdcInvestorGroupField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者分组代码
	TZeusingFtdcInvestorIDType	InvestorGroupID;
	///投资者分组名称
	TZeusingFtdcInvestorGroupNameType	InvestorGroupName;
};

///资金账户
struct CZeusingFtdcTradingAccountField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///上次质押金额
	TZeusingFtdcMoneyType	PreMortgage;
	///上次信用额度
	TZeusingFtdcMoneyType	PreCredit;
	///上次存款额
	TZeusingFtdcMoneyType	PreDeposit;
	///上次结算准备金
	TZeusingFtdcMoneyType	PreBalance;
	///上次占用的保证金
	TZeusingFtdcMoneyType	PreMargin;
	///利息基数
	TZeusingFtdcMoneyType	InterestBase;
	///利息收入
	TZeusingFtdcMoneyType	Interest;
	///入金金额
	TZeusingFtdcMoneyType	Deposit;
	///出金金额
	TZeusingFtdcMoneyType	Withdraw;
	///冻结的保证金
	TZeusingFtdcMoneyType	FrozenMargin;
	///冻结的资金
	TZeusingFtdcMoneyType	FrozenCash;
	///冻结的手续费
	TZeusingFtdcMoneyType	FrozenCommission;
	///当前保证金总额
	TZeusingFtdcMoneyType	CurrMargin;
	///资金差额
	TZeusingFtdcMoneyType	CashIn;
	///手续费
	TZeusingFtdcMoneyType	Commission;
	///平仓盈亏
	TZeusingFtdcMoneyType	CloseProfit;
	///持仓盈亏
	TZeusingFtdcMoneyType	PositionProfit;
	///期货结算准备金
	TZeusingFtdcMoneyType	Balance;
	///可用资金
	TZeusingFtdcMoneyType	Available;
	///可取资金
	TZeusingFtdcMoneyType	WithdrawQuota;
	///基本准备金
	TZeusingFtdcMoneyType	Reserve;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///信用额度
	TZeusingFtdcMoneyType	Credit;
	///质押金额
	TZeusingFtdcMoneyType	Mortgage;
	///交易所保证金
	TZeusingFtdcMoneyType	ExchangeMargin;
	///投资者交割保证金
	TZeusingFtdcMoneyType	DeliveryMargin;
	///交易所交割保证金
	TZeusingFtdcMoneyType	ExchangeDeliveryMargin;
	///保底期货结算准备金
	TZeusingFtdcMoneyType	ReserveBalance;
	///期权平仓盈亏
	TZeusingFtdcMoneyType	OptionCloseProfit;
	///帐户级别
	TZeusingFtdcAccountLevelType	AccountLevel;
};

///投资者持仓
struct CZeusingFtdcInvestorPositionField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///持仓多空方向
	TZeusingFtdcPosiDirectionType	PosiDirection;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///持仓日期
	TZeusingFtdcPositionDateType	PositionDate;
	///上日持仓
	TZeusingFtdcVolumeType	YdPosition;
	///今日持仓
	TZeusingFtdcVolumeType	Position;
	///多头冻结
	TZeusingFtdcVolumeType	LongFrozen;
	///空头冻结
	TZeusingFtdcVolumeType	ShortFrozen;
	///开仓冻结金额
	TZeusingFtdcMoneyType	LongFrozenAmount;
	///开仓冻结金额
	TZeusingFtdcMoneyType	ShortFrozenAmount;
	///开仓量
	TZeusingFtdcVolumeType	OpenVolume;
	///平仓量
	TZeusingFtdcVolumeType	CloseVolume;
	///开仓金额
	TZeusingFtdcMoneyType	OpenAmount;
	///平仓金额
	TZeusingFtdcMoneyType	CloseAmount;
	///平昨仓金额
	TZeusingFtdcMoneyType	YdCloseAmount;
	///平昨仓数量
	TZeusingFtdcVolumeType	YdCloseVolume;
	///持仓成本
	TZeusingFtdcMoneyType	PositionCost;
	///上次占用的保证金
	TZeusingFtdcMoneyType	PreMargin;
	///占用的保证金
	TZeusingFtdcMoneyType	UseMargin;
	///冻结的保证金
	TZeusingFtdcMoneyType	FrozenMargin;
	///冻结的资金
	TZeusingFtdcMoneyType	FrozenCash;
	///冻结的手续费
	TZeusingFtdcMoneyType	FrozenCommission;
	///资金差额
	TZeusingFtdcMoneyType	CashIn;
	///手续费
	TZeusingFtdcMoneyType	Commission;
	///平仓盈亏
	TZeusingFtdcMoneyType	CloseProfit;
	///持仓盈亏
	TZeusingFtdcMoneyType	PositionProfit;
	///上次结算价
	TZeusingFtdcPriceType	PreSettlementPrice;
	///本次结算价
	TZeusingFtdcPriceType	SettlementPrice;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///开仓成本
	TZeusingFtdcMoneyType	OpenCost;
	///交易所保证金
	TZeusingFtdcMoneyType	ExchangeMargin;
	///组合成交形成的持仓
	TZeusingFtdcVolumeType	CombPosition;
	///组合多头冻结
	TZeusingFtdcVolumeType	CombLongFrozen;
	///组合空头冻结
	TZeusingFtdcVolumeType	CombShortFrozen;
	///逐日盯市平仓盈亏
	TZeusingFtdcMoneyType	CloseProfitByDate;
	///逐笔对冲平仓盈亏
	TZeusingFtdcMoneyType	CloseProfitByTrade;
	///今日持仓
	TZeusingFtdcVolumeType	TodayPosition;
	///保证金率
	TZeusingFtdcRatioType	MarginRateByMoney;
	///保证金率(按手数)
	TZeusingFtdcRatioType	MarginRateByVolume;
	///执行冻结
	TZeusingFtdcVolumeType	StrikeFrozen;
	///执行冻结金额
	TZeusingFtdcMoneyType	StrikeFrozenAmount;
	///放弃执行冻结
	TZeusingFtdcVolumeType	AbandonFrozen;
};

///合约保证金率
struct CZeusingFtdcInstrumentMarginRateField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///多头保证金率
	TZeusingFtdcRatioType	LongMarginRatioByMoney;
	///多头保证金费
	TZeusingFtdcMoneyType	LongMarginRatioByVolume;
	///空头保证金率
	TZeusingFtdcRatioType	ShortMarginRatioByMoney;
	///空头保证金费
	TZeusingFtdcMoneyType	ShortMarginRatioByVolume;
	///是否相对交易所收取
	TZeusingFtdcBoolType	IsRelative;
};

///合约手续费率
struct CZeusingFtdcInstrumentCommissionRateField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///开仓手续费率
	TZeusingFtdcRatioType	OpenRatioByMoney;
	///开仓手续费
	TZeusingFtdcRatioType	OpenRatioByVolume;
	///平仓手续费率
	TZeusingFtdcRatioType	CloseRatioByMoney;
	///平仓手续费
	TZeusingFtdcRatioType	CloseRatioByVolume;
	///平今手续费率
	TZeusingFtdcRatioType	CloseTodayRatioByMoney;
	///平今手续费
	TZeusingFtdcRatioType	CloseTodayRatioByVolume;
};

///深度行情
struct CZeusingFtdcDepthMarketDataField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///最新价
	TZeusingFtdcPriceType	LastPrice;
	///上次结算价
	TZeusingFtdcPriceType	PreSettlementPrice;
	///昨收盘
	TZeusingFtdcPriceType	PreClosePrice;
	///昨持仓量
	TZeusingFtdcLargeVolumeType	PreOpenInterest;
	///今开盘
	TZeusingFtdcPriceType	OpenPrice;
	///最高价
	TZeusingFtdcPriceType	HighestPrice;
	///最低价
	TZeusingFtdcPriceType	LowestPrice;
	///数量
	TZeusingFtdcVolumeType	Volume;
	///成交金额
	TZeusingFtdcMoneyType	Turnover;
	///持仓量
	TZeusingFtdcLargeVolumeType	OpenInterest;
	///今收盘
	TZeusingFtdcPriceType	ClosePrice;
	///本次结算价
	TZeusingFtdcPriceType	SettlementPrice;
	///涨停板价
	TZeusingFtdcPriceType	UpperLimitPrice;
	///跌停板价
	TZeusingFtdcPriceType	LowerLimitPrice;
	///昨虚实度
	TZeusingFtdcRatioType	PreDelta;
	///今虚实度
	TZeusingFtdcRatioType	CurrDelta;
	///最后修改时间
	TZeusingFtdcTimeType	UpdateTime;
	///最后修改毫秒
	TZeusingFtdcMillisecType	UpdateMillisec;
	///申买价一
	TZeusingFtdcPriceType	BidPrice1;
	///申买量一
	TZeusingFtdcVolumeType	BidVolume1;
	///申卖价一
	TZeusingFtdcPriceType	AskPrice1;
	///申卖量一
	TZeusingFtdcVolumeType	AskVolume1;
	///申买价二
	TZeusingFtdcPriceType	BidPrice2;
	///申买量二
	TZeusingFtdcVolumeType	BidVolume2;
	///申卖价二
	TZeusingFtdcPriceType	AskPrice2;
	///申卖量二
	TZeusingFtdcVolumeType	AskVolume2;
	///申买价三
	TZeusingFtdcPriceType	BidPrice3;
	///申买量三
	TZeusingFtdcVolumeType	BidVolume3;
	///申卖价三
	TZeusingFtdcPriceType	AskPrice3;
	///申卖量三
	TZeusingFtdcVolumeType	AskVolume3;
	///申买价四
	TZeusingFtdcPriceType	BidPrice4;
	///申买量四
	TZeusingFtdcVolumeType	BidVolume4;
	///申卖价四
	TZeusingFtdcPriceType	AskPrice4;
	///申卖量四
	TZeusingFtdcVolumeType	AskVolume4;
	///申买价五
	TZeusingFtdcPriceType	BidPrice5;
	///申买量五
	TZeusingFtdcVolumeType	BidVolume5;
	///申卖价五
	TZeusingFtdcPriceType	AskPrice5;
	///申卖量五
	TZeusingFtdcVolumeType	AskVolume5;
	///当日均价
	TZeusingFtdcPriceType	AveragePrice;
	///业务日期
	TZeusingFtdcDateType	ActionDay;
};

///投资者合约交易权限
struct CZeusingFtdcInstrumentTradingRightField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易权限
	TZeusingFtdcTradingRightType	TradingRight;
	///报价交易权限
	TZeusingFtdcTradingRightType	QuoteTradingRight;
};

///经纪公司用户
struct CZeusingFtdcBrokerUserField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///用户名称
	TZeusingFtdcUserNameType	UserName;
	///用户类型
	TZeusingFtdcUserTypeType	UserType;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
	///是否使用令牌
	TZeusingFtdcBoolType	IsUsingOTP;
};

///经纪公司用户口令
struct CZeusingFtdcBrokerUserPasswordField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///密码
	TZeusingFtdcPasswordType	Password;
};

///经纪公司用户功能权限
struct CZeusingFtdcBrokerUserFunctionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///经纪公司功能代码
	TZeusingFtdcBrokerFunctionCodeType	BrokerFunctionCode;
};

///交易所交易员报盘机
struct CZeusingFtdcTraderOfferField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///交易所交易员连接状态
	TZeusingFtdcTraderConnectStatusType	TraderConnectStatus;
	///发出连接请求的日期
	TZeusingFtdcDateType	ConnectRequestDate;
	///发出连接请求的时间
	TZeusingFtdcTimeType	ConnectRequestTime;
	///上次报告日期
	TZeusingFtdcDateType	LastReportDate;
	///上次报告时间
	TZeusingFtdcTimeType	LastReportTime;
	///完成连接日期
	TZeusingFtdcDateType	ConnectDate;
	///完成连接时间
	TZeusingFtdcTimeType	ConnectTime;
	///启动日期
	TZeusingFtdcDateType	StartDate;
	///启动时间
	TZeusingFtdcTimeType	StartTime;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///本席位最大成交编号
	TZeusingFtdcTradeIDType	MaxTradeID;
	///本席位最大报单备拷
	TZeusingFtdcReturnCodeType	MaxOrderMessageReference;
};

///投资者结算结果
struct CZeusingFtdcSettlementInfoField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///消息正文
	TZeusingFtdcContentType	Content;
};

///合约保证金率调整
struct CZeusingFtdcInstrumentMarginRateAdjustField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///多头保证金率
	TZeusingFtdcRatioType	LongMarginRatioByMoney;
	///多头保证金费
	TZeusingFtdcMoneyType	LongMarginRatioByVolume;
	///空头保证金率
	TZeusingFtdcRatioType	ShortMarginRatioByMoney;
	///空头保证金费
	TZeusingFtdcMoneyType	ShortMarginRatioByVolume;
	///是否相对交易所收取
	TZeusingFtdcBoolType	IsRelative;
};

///交易所保证金率
struct CZeusingFtdcExchangeMarginRateField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///多头保证金率
	TZeusingFtdcRatioType	LongMarginRatioByMoney;
	///多头保证金费
	TZeusingFtdcMoneyType	LongMarginRatioByVolume;
	///空头保证金率
	TZeusingFtdcRatioType	ShortMarginRatioByMoney;
	///空头保证金费
	TZeusingFtdcMoneyType	ShortMarginRatioByVolume;
};

///交易所保证金率调整
struct CZeusingFtdcExchangeMarginRateAdjustField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///跟随交易所投资者多头保证金率
	TZeusingFtdcRatioType	LongMarginRatioByMoney;
	///跟随交易所投资者多头保证金费
	TZeusingFtdcMoneyType	LongMarginRatioByVolume;
	///跟随交易所投资者空头保证金率
	TZeusingFtdcRatioType	ShortMarginRatioByMoney;
	///跟随交易所投资者空头保证金费
	TZeusingFtdcMoneyType	ShortMarginRatioByVolume;
	///交易所多头保证金率
	TZeusingFtdcRatioType	ExchLongMarginRatioByMoney;
	///交易所多头保证金费
	TZeusingFtdcMoneyType	ExchLongMarginRatioByVolume;
	///交易所空头保证金率
	TZeusingFtdcRatioType	ExchShortMarginRatioByMoney;
	///交易所空头保证金费
	TZeusingFtdcMoneyType	ExchShortMarginRatioByVolume;
	///不跟随交易所投资者多头保证金率
	TZeusingFtdcRatioType	NoLongMarginRatioByMoney;
	///不跟随交易所投资者多头保证金费
	TZeusingFtdcMoneyType	NoLongMarginRatioByVolume;
	///不跟随交易所投资者空头保证金率
	TZeusingFtdcRatioType	NoShortMarginRatioByMoney;
	///不跟随交易所投资者空头保证金费
	TZeusingFtdcMoneyType	NoShortMarginRatioByVolume;
};

///结算引用
struct CZeusingFtdcSettlementRefField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
};

///当前时间
struct CZeusingFtdcCurrentTimeField
{
	///当前日期
	TZeusingFtdcDateType	CurrDate;
	///当前时间
	TZeusingFtdcTimeType	CurrTime;
	///当前时间（毫秒）
	TZeusingFtdcMillisecType	CurrMillisec;
	///业务日期
	TZeusingFtdcDateType	ActionDay;
};

///通讯阶段
struct CZeusingFtdcCommPhaseField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///通讯时段编号
	TZeusingFtdcCommPhaseNoType	CommPhaseNo;
	///系统编号
	TZeusingFtdcSystemIDType	SystemID;
};

///登录信息
struct CZeusingFtdcLoginInfoField
{
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///登录日期
	TZeusingFtdcDateType	LoginDate;
	///登录时间
	TZeusingFtdcTimeType	LoginTime;
	///IP地址
	TZeusingFtdcIPAddressType	IPAddress;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
	///接口端产品信息
	TZeusingFtdcProductInfoType	InterfaceProductInfo;
	///协议信息
	TZeusingFtdcProtocolInfoType	ProtocolInfo;
	///系统名称
	TZeusingFtdcSystemNameType	SystemName;
	///密码
	TZeusingFtdcPasswordType	Password;
	///最大报单引用
	TZeusingFtdcOrderRefType	MaxOrderRef;
	///上期所时间
	TZeusingFtdcTimeType	SHFETime;
	///大商所时间
	TZeusingFtdcTimeType	DCETime;
	///郑商所时间
	TZeusingFtdcTimeType	CZCETime;
	///中金所时间
	TZeusingFtdcTimeType	FFEXTime;
	///Mac地址
	TZeusingFtdcMacAddressType	MacAddress;
	///动态密码
	TZeusingFtdcPasswordType	OneTimePassword;
};

///登录信息
struct CZeusingFtdcLogoutAllField
{
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///系统名称
	TZeusingFtdcSystemNameType	SystemName;
};

///前置状态
struct CZeusingFtdcFrontStatusField
{
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///上次报告日期
	TZeusingFtdcDateType	LastReportDate;
	///上次报告时间
	TZeusingFtdcTimeType	LastReportTime;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
};

///用户口令变更
struct CZeusingFtdcUserPasswordUpdateField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///原来的口令
	TZeusingFtdcPasswordType	OldPassword;
	///新的口令
	TZeusingFtdcPasswordType	NewPassword;
};

///输入报单
struct CZeusingFtdcInputOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///报单价格条件
	TZeusingFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///组合开平标志
	TZeusingFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量
	TZeusingFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
	TZeusingFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TZeusingFtdcDateType	GTDDate;
	///成交量类型
	TZeusingFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TZeusingFtdcVolumeType	MinVolume;
	///触发条件
	TZeusingFtdcContingentConditionType	ContingentCondition;
	///止损价
	TZeusingFtdcPriceType	StopPrice;
	///强平原因
	TZeusingFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TZeusingFtdcBoolType	IsAutoSuspend;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///用户强评标志
	TZeusingFtdcBoolType	UserForceClose;
	///互换单标志
	TZeusingFtdcBoolType	IsSwapOrder;
};

///报单
struct CZeusingFtdcOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///报单价格条件
	TZeusingFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///组合开平标志
	TZeusingFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量
	TZeusingFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
	TZeusingFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TZeusingFtdcDateType	GTDDate;
	///成交量类型
	TZeusingFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TZeusingFtdcVolumeType	MinVolume;
	///触发条件
	TZeusingFtdcContingentConditionType	ContingentCondition;
	///止损价
	TZeusingFtdcPriceType	StopPrice;
	///强平原因
	TZeusingFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TZeusingFtdcBoolType	IsAutoSuspend;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///报单提交状态
	TZeusingFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///报单提示序号
	TZeusingFtdcSequenceNoType	NotifySequence;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///报单来源
	TZeusingFtdcOrderSourceType	OrderSource;
	///报单状态
	TZeusingFtdcOrderStatusType	OrderStatus;
	///报单类型
	TZeusingFtdcOrderTypeType	OrderType;
	///今成交数量
	TZeusingFtdcVolumeType	VolumeTraded;
	///剩余数量
	TZeusingFtdcVolumeType	VolumeTotal;
	///报单日期
	TZeusingFtdcDateType	InsertDate;
	///委托时间
	TZeusingFtdcTimeType	InsertTime;
	///激活时间
	TZeusingFtdcTimeType	ActiveTime;
	///挂起时间
	TZeusingFtdcTimeType	SuspendTime;
	///最后修改时间
	TZeusingFtdcTimeType	UpdateTime;
	///撤销时间
	TZeusingFtdcTimeType	CancelTime;
	///最后修改交易所交易员代码
	TZeusingFtdcTraderIDType	ActiveTraderID;
	///结算会员编号
	TZeusingFtdcParticipantIDType	ClearingPartID;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
	///状态信息
	TZeusingFtdcErrorMsgType	StatusMsg;
	///用户强评标志
	TZeusingFtdcBoolType	UserForceClose;
	///操作用户代码
	TZeusingFtdcUserIDType	ActiveUserID;
	///经纪公司报单编号
	TZeusingFtdcSequenceNoType	BrokerOrderSeq;
	///相关报单
	TZeusingFtdcOrderSysIDType	RelativeOrderSysID;
	///郑商所成交数量
	TZeusingFtdcVolumeType	ZCETotalTradedVolume;
	///互换单标志
	TZeusingFtdcBoolType	IsSwapOrder;
	///交易编码类型
	TZeusingFtdcClientIDTypeType	ClientIDType;
};

///交易所报单
struct CZeusingFtdcExchangeOrderField
{
	///报单价格条件
	TZeusingFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///组合开平标志
	TZeusingFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量
	TZeusingFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
	TZeusingFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TZeusingFtdcDateType	GTDDate;
	///成交量类型
	TZeusingFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TZeusingFtdcVolumeType	MinVolume;
	///触发条件
	TZeusingFtdcContingentConditionType	ContingentCondition;
	///止损价
	TZeusingFtdcPriceType	StopPrice;
	///强平原因
	TZeusingFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TZeusingFtdcBoolType	IsAutoSuspend;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///报单提交状态
	TZeusingFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///报单提示序号
	TZeusingFtdcSequenceNoType	NotifySequence;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///报单来源
	TZeusingFtdcOrderSourceType	OrderSource;
	///报单状态
	TZeusingFtdcOrderStatusType	OrderStatus;
	///报单类型
	TZeusingFtdcOrderTypeType	OrderType;
	///今成交数量
	TZeusingFtdcVolumeType	VolumeTraded;
	///剩余数量
	TZeusingFtdcVolumeType	VolumeTotal;
	///报单日期
	TZeusingFtdcDateType	InsertDate;
	///委托时间
	TZeusingFtdcTimeType	InsertTime;
	///激活时间
	TZeusingFtdcTimeType	ActiveTime;
	///挂起时间
	TZeusingFtdcTimeType	SuspendTime;
	///最后修改时间
	TZeusingFtdcTimeType	UpdateTime;
	///撤销时间
	TZeusingFtdcTimeType	CancelTime;
	///最后修改交易所交易员代码
	TZeusingFtdcTraderIDType	ActiveTraderID;
	///结算会员编号
	TZeusingFtdcParticipantIDType	ClearingPartID;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
};

///交易所报单插入失败
struct CZeusingFtdcExchangeOrderInsertErrorField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///输入报单操作
struct CZeusingFtdcInputOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///报单操作引用
	TZeusingFtdcOrderActionRefType	OrderActionRef;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量变化
	TZeusingFtdcVolumeType	VolumeChange;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///报单操作
struct CZeusingFtdcOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///报单操作引用
	TZeusingFtdcOrderActionRefType	OrderActionRef;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量变化
	TZeusingFtdcVolumeType	VolumeChange;
	///操作日期
	TZeusingFtdcDateType	ActionDate;
	///操作时间
	TZeusingFtdcTimeType	ActionTime;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///操作本地编号
	TZeusingFtdcOrderLocalIDType	ActionLocalID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///报单操作状态
	TZeusingFtdcOrderActionStatusType	OrderActionStatus;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///状态信息
	TZeusingFtdcErrorMsgType	StatusMsg;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///交易所报单操作
struct CZeusingFtdcExchangeOrderActionField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量变化
	TZeusingFtdcVolumeType	VolumeChange;
	///操作日期
	TZeusingFtdcDateType	ActionDate;
	///操作时间
	TZeusingFtdcTimeType	ActionTime;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///操作本地编号
	TZeusingFtdcOrderLocalIDType	ActionLocalID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///报单操作状态
	TZeusingFtdcOrderActionStatusType	OrderActionStatus;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
};

///交易所报单操作失败
struct CZeusingFtdcExchangeOrderActionErrorField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///操作本地编号
	TZeusingFtdcOrderLocalIDType	ActionLocalID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///交易所成交
struct CZeusingFtdcExchangeTradeField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///成交编号
	TZeusingFtdcTradeIDType	TradeID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///交易角色
	TZeusingFtdcTradingRoleType	TradingRole;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///开平标志
	TZeusingFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///价格
	TZeusingFtdcPriceType	Price;
	///数量
	TZeusingFtdcVolumeType	Volume;
	///成交时期
	TZeusingFtdcDateType	TradeDate;
	///成交时间
	TZeusingFtdcTimeType	TradeTime;
	///成交类型
	TZeusingFtdcTradeTypeType	TradeType;
	///成交价来源
	TZeusingFtdcPriceSourceType	PriceSource;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///结算会员编号
	TZeusingFtdcParticipantIDType	ClearingPartID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///成交来源
	TZeusingFtdcTradeSourceType	TradeSource;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///成交
struct CZeusingFtdcTradeField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///成交编号
	TZeusingFtdcTradeIDType	TradeID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///交易角色
	TZeusingFtdcTradingRoleType	TradingRole;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///开平标志
	TZeusingFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///价格
	TZeusingFtdcPriceType	Price;
	///数量
	TZeusingFtdcVolumeType	Volume;
	///成交时期
	TZeusingFtdcDateType	TradeDate;
	///成交时间
	TZeusingFtdcTimeType	TradeTime;
	///成交类型
	TZeusingFtdcTradeTypeType	TradeType;
	///成交价来源
	TZeusingFtdcPriceSourceType	PriceSource;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///结算会员编号
	TZeusingFtdcParticipantIDType	ClearingPartID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///经纪公司报单编号
	TZeusingFtdcSequenceNoType	BrokerOrderSeq;
	///成交来源
	TZeusingFtdcTradeSourceType	TradeSource;
};

///用户会话
struct CZeusingFtdcUserSessionField
{
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///登录日期
	TZeusingFtdcDateType	LoginDate;
	///登录时间
	TZeusingFtdcTimeType	LoginTime;
	///IP地址
	TZeusingFtdcIPAddressType	IPAddress;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
	///接口端产品信息
	TZeusingFtdcProductInfoType	InterfaceProductInfo;
	///协议信息
	TZeusingFtdcProtocolInfoType	ProtocolInfo;
	///Mac地址
	TZeusingFtdcMacAddressType	MacAddress;
};

///查询最大报单数量
struct CZeusingFtdcQueryMaxOrderVolumeField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///开平标志
	TZeusingFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///最大允许报单数量
	TZeusingFtdcVolumeType	MaxVolume;
};

///投资者结算结果确认信息
struct CZeusingFtdcSettlementInfoConfirmField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///确认日期
	TZeusingFtdcDateType	ConfirmDate;
	///确认时间
	TZeusingFtdcTimeType	ConfirmTime;
};

///出入金同步
struct CZeusingFtdcSyncDepositField
{
	///出入金流水号
	TZeusingFtdcDepositSeqNoType	DepositSeqNo;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///入金金额
	TZeusingFtdcMoneyType	Deposit;
	///是否强制进行
	TZeusingFtdcBoolType	IsForce;
};

///经纪公司同步
struct CZeusingFtdcBrokerSyncField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
};

///正在同步中的投资者
struct CZeusingFtdcSyncingInvestorField
{
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者分组代码
	TZeusingFtdcInvestorIDType	InvestorGroupID;
	///投资者名称
	TZeusingFtdcPartyNameType	InvestorName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdentifiedCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
	///联系电话
	TZeusingFtdcTelephoneType	Telephone;
	///通讯地址
	TZeusingFtdcAddressType	Address;
	///开户日期
	TZeusingFtdcDateType	OpenDate;
	///手机
	TZeusingFtdcMobileType	Mobile;
	///手续费率模板代码
	TZeusingFtdcInvestorIDType	CommModelID;
	///填充字段
	TZeusingFtdcPadFieldType	PadField;
	///保证金率模板代码
	TZeusingFtdcInvestorIDType	MarginModelID;
};

///正在同步中的交易代码
struct CZeusingFtdcSyncingTradingCodeField
{
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
	///交易编码类型
	TZeusingFtdcClientIDTypeType	ClientIDType;
};

///正在同步中的投资者分组
struct CZeusingFtdcSyncingInvestorGroupField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者分组代码
	TZeusingFtdcInvestorIDType	InvestorGroupID;
	///投资者分组名称
	TZeusingFtdcInvestorGroupNameType	InvestorGroupName;
};

///正在同步中的交易账号
struct CZeusingFtdcSyncingTradingAccountField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///上次质押金额
	TZeusingFtdcMoneyType	PreMortgage;
	///上次信用额度
	TZeusingFtdcMoneyType	PreCredit;
	///上次存款额
	TZeusingFtdcMoneyType	PreDeposit;
	///上次结算准备金
	TZeusingFtdcMoneyType	PreBalance;
	///上次占用的保证金
	TZeusingFtdcMoneyType	PreMargin;
	///利息基数
	TZeusingFtdcMoneyType	InterestBase;
	///利息收入
	TZeusingFtdcMoneyType	Interest;
	///入金金额
	TZeusingFtdcMoneyType	Deposit;
	///出金金额
	TZeusingFtdcMoneyType	Withdraw;
	///冻结的保证金
	TZeusingFtdcMoneyType	FrozenMargin;
	///冻结的资金
	TZeusingFtdcMoneyType	FrozenCash;
	///冻结的手续费
	TZeusingFtdcMoneyType	FrozenCommission;
	///当前保证金总额
	TZeusingFtdcMoneyType	CurrMargin;
	///资金差额
	TZeusingFtdcMoneyType	CashIn;
	///手续费
	TZeusingFtdcMoneyType	Commission;
	///平仓盈亏
	TZeusingFtdcMoneyType	CloseProfit;
	///持仓盈亏
	TZeusingFtdcMoneyType	PositionProfit;
	///期货结算准备金
	TZeusingFtdcMoneyType	Balance;
	///可用资金
	TZeusingFtdcMoneyType	Available;
	///可取资金
	TZeusingFtdcMoneyType	WithdrawQuota;
	///基本准备金
	TZeusingFtdcMoneyType	Reserve;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///信用额度
	TZeusingFtdcMoneyType	Credit;
	///质押金额
	TZeusingFtdcMoneyType	Mortgage;
	///交易所保证金
	TZeusingFtdcMoneyType	ExchangeMargin;
	///投资者交割保证金
	TZeusingFtdcMoneyType	DeliveryMargin;
	///交易所交割保证金
	TZeusingFtdcMoneyType	ExchangeDeliveryMargin;
	///保底期货结算准备金
	TZeusingFtdcMoneyType	ReserveBalance;
	///期权平仓盈亏
	TZeusingFtdcMoneyType	OptionCloseProfit;
	///帐户级别
	TZeusingFtdcAccountLevelType	AccountLevel;
};

///正在同步中的投资者持仓
struct CZeusingFtdcSyncingInvestorPositionField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///持仓多空方向
	TZeusingFtdcPosiDirectionType	PosiDirection;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///持仓日期
	TZeusingFtdcPositionDateType	PositionDate;
	///上日持仓
	TZeusingFtdcVolumeType	YdPosition;
	///今日持仓
	TZeusingFtdcVolumeType	Position;
	///多头冻结
	TZeusingFtdcVolumeType	LongFrozen;
	///空头冻结
	TZeusingFtdcVolumeType	ShortFrozen;
	///开仓冻结金额
	TZeusingFtdcMoneyType	LongFrozenAmount;
	///开仓冻结金额
	TZeusingFtdcMoneyType	ShortFrozenAmount;
	///开仓量
	TZeusingFtdcVolumeType	OpenVolume;
	///平仓量
	TZeusingFtdcVolumeType	CloseVolume;
	///开仓金额
	TZeusingFtdcMoneyType	OpenAmount;
	///平仓金额
	TZeusingFtdcMoneyType	CloseAmount;
	///平昨仓金额
	TZeusingFtdcMoneyType	YdCloseAmount;
	///平昨仓数量
	TZeusingFtdcVolumeType	YdCloseVolume;
	///持仓成本
	TZeusingFtdcMoneyType	PositionCost;
	///上次占用的保证金
	TZeusingFtdcMoneyType	PreMargin;
	///占用的保证金
	TZeusingFtdcMoneyType	UseMargin;
	///冻结的保证金
	TZeusingFtdcMoneyType	FrozenMargin;
	///冻结的资金
	TZeusingFtdcMoneyType	FrozenCash;
	///冻结的手续费
	TZeusingFtdcMoneyType	FrozenCommission;
	///资金差额
	TZeusingFtdcMoneyType	CashIn;
	///手续费
	TZeusingFtdcMoneyType	Commission;
	///平仓盈亏
	TZeusingFtdcMoneyType	CloseProfit;
	///持仓盈亏
	TZeusingFtdcMoneyType	PositionProfit;
	///上次结算价
	TZeusingFtdcPriceType	PreSettlementPrice;
	///本次结算价
	TZeusingFtdcPriceType	SettlementPrice;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///开仓成本
	TZeusingFtdcMoneyType	OpenCost;
	///交易所保证金
	TZeusingFtdcMoneyType	ExchangeMargin;
	///组合成交形成的持仓
	TZeusingFtdcVolumeType	CombPosition;
	///组合多头冻结
	TZeusingFtdcVolumeType	CombLongFrozen;
	///组合空头冻结
	TZeusingFtdcVolumeType	CombShortFrozen;
	///逐日盯市平仓盈亏
	TZeusingFtdcMoneyType	CloseProfitByDate;
	///逐笔对冲平仓盈亏
	TZeusingFtdcMoneyType	CloseProfitByTrade;
	///今日持仓
	TZeusingFtdcVolumeType	TodayPosition;
	///保证金率
	TZeusingFtdcRatioType	MarginRateByMoney;
	///保证金率(按手数)
	TZeusingFtdcRatioType	MarginRateByVolume;
	///执行冻结
	TZeusingFtdcVolumeType	StrikeFrozen;
	///执行冻结金额
	TZeusingFtdcMoneyType	StrikeFrozenAmount;
	///放弃执行冻结
	TZeusingFtdcVolumeType	AbandonFrozen;
};

///正在同步中的合约保证金率
struct CZeusingFtdcSyncingInstrumentMarginRateField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///多头保证金率
	TZeusingFtdcRatioType	LongMarginRatioByMoney;
	///多头保证金费
	TZeusingFtdcMoneyType	LongMarginRatioByVolume;
	///空头保证金率
	TZeusingFtdcRatioType	ShortMarginRatioByMoney;
	///空头保证金费
	TZeusingFtdcMoneyType	ShortMarginRatioByVolume;
	///是否相对交易所收取
	TZeusingFtdcBoolType	IsRelative;
};

///正在同步中的合约手续费率
struct CZeusingFtdcSyncingInstrumentCommissionRateField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///开仓手续费率
	TZeusingFtdcRatioType	OpenRatioByMoney;
	///开仓手续费
	TZeusingFtdcRatioType	OpenRatioByVolume;
	///平仓手续费率
	TZeusingFtdcRatioType	CloseRatioByMoney;
	///平仓手续费
	TZeusingFtdcRatioType	CloseRatioByVolume;
	///平今手续费率
	TZeusingFtdcRatioType	CloseTodayRatioByMoney;
	///平今手续费
	TZeusingFtdcRatioType	CloseTodayRatioByVolume;
};

///正在同步中的合约交易权限
struct CZeusingFtdcSyncingInstrumentTradingRightField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易权限
	TZeusingFtdcTradingRightType	TradingRight;
	///报价交易权限
	TZeusingFtdcTradingRightType	QuoteTradingRight;
};

///查询报单
struct CZeusingFtdcQryOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///开始时间
	TZeusingFtdcTimeType	InsertTimeStart;
	///结束时间
	TZeusingFtdcTimeType	InsertTimeEnd;
};

///查询成交
struct CZeusingFtdcQryTradeField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///成交编号
	TZeusingFtdcTradeIDType	TradeID;
	///开始时间
	TZeusingFtdcTimeType	TradeTimeStart;
	///结束时间
	TZeusingFtdcTimeType	TradeTimeEnd;
};

///查询投资者持仓
struct CZeusingFtdcQryInvestorPositionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///查询资金账户
struct CZeusingFtdcQryTradingAccountField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///查询投资者
struct CZeusingFtdcQryInvestorField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///查询交易编码
struct CZeusingFtdcQryTradingCodeField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///交易编码类型
	TZeusingFtdcClientIDTypeType	ClientIDType;
};

///查询交易编码
struct CZeusingFtdcQryInvestorGroupField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
};

///查询交易编码
struct CZeusingFtdcQryInstrumentMarginRateField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
};

///查询交易编码
struct CZeusingFtdcQryInstrumentCommissionRateField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
};

///查询交易编码
struct CZeusingFtdcQryInstrumentTradingRightField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///查询经纪公司
struct CZeusingFtdcQryBrokerField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
};

///查询交易员
struct CZeusingFtdcQryTraderField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
};

///查询经纪公司会员代码
struct CZeusingFtdcQryPartBrokerField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
};

///查询管理用户功能权限
struct CZeusingFtdcQrySuperUserFunctionField
{
	///用户代码
	TZeusingFtdcUserIDType	UserID;
};

///查询用户会话
struct CZeusingFtdcQryUserSessionField
{
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
};

///查询前置状态
struct CZeusingFtdcQryFrontStatusField
{
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
};

///查询交易所报单
struct CZeusingFtdcQryExchangeOrderField
{
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
};

///查询报单操作
struct CZeusingFtdcQryOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///查询交易所报单操作
struct CZeusingFtdcQryExchangeOrderActionField
{
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
};

///查询管理用户
struct CZeusingFtdcQrySuperUserField
{
	///用户代码
	TZeusingFtdcUserIDType	UserID;
};

///查询交易所
struct CZeusingFtdcQryExchangeField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///查询产品
struct CZeusingFtdcQryProductField
{
	///产品代码
	TZeusingFtdcInstrumentIDType	ProductID;
	///产品类型
	TZeusingFtdcProductClassType	ProductClass;
};

///查询合约
struct CZeusingFtdcQryInstrumentField
{
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///产品代码
	TZeusingFtdcInstrumentIDType	ProductID;
};

///查询行情
struct CZeusingFtdcQryDepthMarketDataField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///查询经纪公司用户
struct CZeusingFtdcQryBrokerUserField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
};

///查询经纪公司用户权限
struct CZeusingFtdcQryBrokerUserFunctionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
};

///查询交易员报盘机
struct CZeusingFtdcQryTraderOfferField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
};

///查询出入金流水
struct CZeusingFtdcQrySyncDepositField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///出入金流水号
	TZeusingFtdcDepositSeqNoType	DepositSeqNo;
};

///查询投资者结算结果
struct CZeusingFtdcQrySettlementInfoField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易日
	TZeusingFtdcDateType	TradingDay;
};

///查询报单
struct CZeusingFtdcQryHisOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///开始时间
	TZeusingFtdcTimeType	InsertTimeStart;
	///结束时间
	TZeusingFtdcTimeType	InsertTimeEnd;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
};

///当前期权合约最小保证金
struct CZeusingFtdcOptionInstrMiniMarginField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///单位（手）期权合约最小保证金
	TZeusingFtdcMoneyType	MinMargin;
	///取值方式
	TZeusingFtdcValueMethodType	ValueMethod;
	///是否跟随交易所收取
	TZeusingFtdcBoolType	IsRelative;
};

///经纪公司交易所期权保证金算法
struct CZeusingFtdcBrokerExchOptMarginAlgoField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///最低保障系数
	TZeusingFtdcRatioType	GuaranteeRatio;
	///虚值额优惠比率
	TZeusingFtdcRatioType	OutRatio;
};

///当前期权合约保证金调整系数
struct CZeusingFtdcOptionInstrMarginAdjustField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///空头保证金调整系数
	TZeusingFtdcRatioType	ShortMarginRatioByMoney;
	///空头保证金调整系数
	TZeusingFtdcMoneyType	ShortMarginRatioByVolume;
	///是否跟随交易所收取
	TZeusingFtdcBoolType	IsRelative;
};

///当前期权合约手续费的详细内容
struct CZeusingFtdcOptionInstrCommRateField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///开仓手续费率
	TZeusingFtdcRatioType	OpenRatioByMoney;
	///开仓手续费
	TZeusingFtdcRatioType	OpenRatioByVolume;
	///平仓手续费率
	TZeusingFtdcRatioType	CloseRatioByMoney;
	///平仓手续费
	TZeusingFtdcRatioType	CloseRatioByVolume;
	///平今手续费率
	TZeusingFtdcRatioType	CloseTodayRatioByMoney;
	///平今手续费
	TZeusingFtdcRatioType	CloseTodayRatioByVolume;
	///执行手续费率
	TZeusingFtdcRatioType	StrikeRatioByMoney;
	///执行手续费
	TZeusingFtdcMoneyType	StrikeRatioByVolume;
};

///期权交易成本
struct CZeusingFtdcOptionInstrTradeCostField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///期权合约保证金
	TZeusingFtdcMoneyType	Margin;
	///期权合约权利金
	TZeusingFtdcMoneyType	Royalty;
};

///期权交易成本查询
struct CZeusingFtdcQryOptionInstrTradeCostField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///期权合约报价
	TZeusingFtdcPriceType	InputPrice;
};

///期权手续费率查询
struct CZeusingFtdcQryOptionInstrCommRateField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
};

///股指现货指数
struct CZeusingFtdcIndexPriceField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///现货收盘价
	TZeusingFtdcPriceType	ClosePrice;
};

///输入的执行宣告
struct CZeusingFtdcInputExecOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///执行宣告引用
	TZeusingFtdcOrderRefType	ExecOrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///数量
	TZeusingFtdcVolumeType	Volume;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///开平标志
	TZeusingFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///执行类型
	TZeusingFtdcActionTypeType	ActionType;
	///保留头寸申请的持仓方向
	TZeusingFtdcPosiDirectionType	PosiDirection;
	///期权行权后是否保留期货头寸的标记
	TZeusingFtdcExecOrderPositionFlagType	ReservePositionFlag;
	///期权行权后生成的头寸是否自动平仓
	TZeusingFtdcExecOrderCloseFlagType	CloseFlag;
};

///输入执行宣告操作
struct CZeusingFtdcInputExecOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///执行宣告操作引用
	TZeusingFtdcOrderActionRefType	ExecOrderActionRef;
	///执行宣告引用
	TZeusingFtdcOrderRefType	ExecOrderRef;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///执行宣告操作编号
	TZeusingFtdcExecOrderSysIDType	ExecOrderSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///执行宣告
struct CZeusingFtdcExecOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///执行宣告引用
	TZeusingFtdcOrderRefType	ExecOrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///数量
	TZeusingFtdcVolumeType	Volume;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///开平标志
	TZeusingFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///执行类型
	TZeusingFtdcActionTypeType	ActionType;
	///保留头寸申请的持仓方向
	TZeusingFtdcPosiDirectionType	PosiDirection;
	///期权行权后是否保留期货头寸的标记
	TZeusingFtdcExecOrderPositionFlagType	ReservePositionFlag;
	///期权行权后生成的头寸是否自动平仓
	TZeusingFtdcExecOrderCloseFlagType	CloseFlag;
	///本地执行宣告编号
	TZeusingFtdcOrderLocalIDType	ExecOrderLocalID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///执行宣告提交状态
	TZeusingFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///报单提示序号
	TZeusingFtdcSequenceNoType	NotifySequence;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///执行宣告编号
	TZeusingFtdcExecOrderSysIDType	ExecOrderSysID;
	///报单日期
	TZeusingFtdcDateType	InsertDate;
	///插入时间
	TZeusingFtdcTimeType	InsertTime;
	///撤销时间
	TZeusingFtdcTimeType	CancelTime;
	///执行结果
	TZeusingFtdcExecResultType	ExecResult;
	///结算会员编号
	TZeusingFtdcParticipantIDType	ClearingPartID;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
	///状态信息
	TZeusingFtdcErrorMsgType	StatusMsg;
	///操作用户代码
	TZeusingFtdcUserIDType	ActiveUserID;
	///经纪公司报单编号
	TZeusingFtdcSequenceNoType	BrokerExecOrderSeq;
	///交易编码类型
	TZeusingFtdcClientIDTypeType	ClientIDType;
};

///执行宣告操作
struct CZeusingFtdcExecOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///执行宣告操作引用
	TZeusingFtdcOrderActionRefType	ExecOrderActionRef;
	///执行宣告引用
	TZeusingFtdcOrderRefType	ExecOrderRef;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///执行宣告操作编号
	TZeusingFtdcExecOrderSysIDType	ExecOrderSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///操作日期
	TZeusingFtdcDateType	ActionDate;
	///操作时间
	TZeusingFtdcTimeType	ActionTime;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地执行宣告编号
	TZeusingFtdcOrderLocalIDType	ExecOrderLocalID;
	///操作本地编号
	TZeusingFtdcOrderLocalIDType	ActionLocalID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///报单操作状态
	TZeusingFtdcOrderActionStatusType	OrderActionStatus;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///状态信息
	TZeusingFtdcErrorMsgType	StatusMsg;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///执行宣告查询
struct CZeusingFtdcQryExecOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///执行宣告编号
	TZeusingFtdcExecOrderSysIDType	ExecOrderSysID;
	///开始时间
	TZeusingFtdcTimeType	InsertTimeStart;
	///结束时间
	TZeusingFtdcTimeType	InsertTimeEnd;
};

///交易所执行宣告信息
struct CZeusingFtdcExchangeExecOrderField
{
	///数量
	TZeusingFtdcVolumeType	Volume;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///开平标志
	TZeusingFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///执行类型
	TZeusingFtdcActionTypeType	ActionType;
	///保留头寸申请的持仓方向
	TZeusingFtdcPosiDirectionType	PosiDirection;
	///期权行权后是否保留期货头寸的标记
	TZeusingFtdcExecOrderPositionFlagType	ReservePositionFlag;
	///期权行权后生成的头寸是否自动平仓
	TZeusingFtdcExecOrderCloseFlagType	CloseFlag;
	///本地执行宣告编号
	TZeusingFtdcOrderLocalIDType	ExecOrderLocalID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///执行宣告提交状态
	TZeusingFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///报单提示序号
	TZeusingFtdcSequenceNoType	NotifySequence;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///执行宣告编号
	TZeusingFtdcExecOrderSysIDType	ExecOrderSysID;
	///报单日期
	TZeusingFtdcDateType	InsertDate;
	///插入时间
	TZeusingFtdcTimeType	InsertTime;
	///撤销时间
	TZeusingFtdcTimeType	CancelTime;
	///执行结果
	TZeusingFtdcExecResultType	ExecResult;
	///结算会员编号
	TZeusingFtdcParticipantIDType	ClearingPartID;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
};

///交易所执行宣告查询
struct CZeusingFtdcQryExchangeExecOrderField
{
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
};

///执行宣告操作查询
struct CZeusingFtdcQryExecOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///交易所执行宣告操作
struct CZeusingFtdcExchangeExecOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///执行宣告操作编号
	TZeusingFtdcExecOrderSysIDType	ExecOrderSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///操作日期
	TZeusingFtdcDateType	ActionDate;
	///操作时间
	TZeusingFtdcTimeType	ActionTime;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地执行宣告编号
	TZeusingFtdcOrderLocalIDType	ExecOrderLocalID;
	///操作本地编号
	TZeusingFtdcOrderLocalIDType	ActionLocalID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///报单操作状态
	TZeusingFtdcOrderActionStatusType	OrderActionStatus;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///交易所执行宣告操作查询
struct CZeusingFtdcQryExchangeExecOrderActionField
{
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
};

///错误执行宣告
struct CZeusingFtdcErrExecOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///执行宣告引用
	TZeusingFtdcOrderRefType	ExecOrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///数量
	TZeusingFtdcVolumeType	Volume;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///开平标志
	TZeusingFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///执行类型
	TZeusingFtdcActionTypeType	ActionType;
	///保留头寸申请的持仓方向
	TZeusingFtdcPosiDirectionType	PosiDirection;
	///期权行权后是否保留期货头寸的标记
	TZeusingFtdcExecOrderPositionFlagType	ReservePositionFlag;
	///期权行权后生成的头寸是否自动平仓
	TZeusingFtdcExecOrderCloseFlagType	CloseFlag;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///查询错误执行宣告
struct CZeusingFtdcQryErrExecOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///错误执行宣告操作
struct CZeusingFtdcErrExecOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///执行宣告操作引用
	TZeusingFtdcOrderActionRefType	ExecOrderActionRef;
	///执行宣告引用
	TZeusingFtdcOrderRefType	ExecOrderRef;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///执行宣告操作编号
	TZeusingFtdcExecOrderSysIDType	ExecOrderSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///查询错误执行宣告操作
struct CZeusingFtdcQryErrExecOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///报价输入
struct CZeusingFtdcInputQuoteField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///买方数量
	TZeusingFtdcVolumeType	BidVolumeTotalOriginal;
	///价格
	TZeusingFtdcPriceType	BidPrice;
	///买方组合开平标志
	TZeusingFtdcCombOffsetFlagType	BidCombOffsetFlag;
	///买方组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	BidCombHedgeFlag;
	///卖方数量
	TZeusingFtdcVolumeType	AskVolumeTotalOriginal;
	///价格
	TZeusingFtdcPriceType	AskPrice;
	///卖方组合开平标志
	TZeusingFtdcCombOffsetFlagType	AskCombOffsetFlag;
	///卖方组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	AskCombHedgeFlag;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///报价操作输入
struct CZeusingFtdcInputQuoteActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///报单操作引用
	TZeusingFtdcOrderActionRefType	QuoteActionRef;
	///报单引用
	TZeusingFtdcOrderRefType	QuoteRef;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	QuoteSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///查询报价
struct CZeusingFtdcQryQuoteField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报价编号
	TZeusingFtdcOrderSysIDType	QuoteSysID;
	///开始时间
	TZeusingFtdcTimeType	InsertTimeStart;
	///结束时间
	TZeusingFtdcTimeType	InsertTimeEnd;
};

///报价
struct CZeusingFtdcQuoteField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///买方数量
	TZeusingFtdcVolumeType	BidVolumeTotalOriginal;
	///价格
	TZeusingFtdcPriceType	BidPrice;
	///买方组合开平标志
	TZeusingFtdcCombOffsetFlagType	BidCombOffsetFlag;
	///买方组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	BidCombHedgeFlag;
	///卖方数量
	TZeusingFtdcVolumeType	AskVolumeTotalOriginal;
	///价格
	TZeusingFtdcPriceType	AskPrice;
	///卖方组合开平标志
	TZeusingFtdcCombOffsetFlagType	AskCombOffsetFlag;
	///卖方组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	AskCombHedgeFlag;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///报单提交状态
	TZeusingFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///报单提示序号
	TZeusingFtdcSequenceNoType	NotifySequence;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///报单编号
	TZeusingFtdcOrderSysIDType	QuoteSysID;
	///买方报单编号
	TZeusingFtdcOrderSysIDType	BidOrderSysID;
	///卖方报单编号
	TZeusingFtdcOrderSysIDType	AskOrderSysID;
	///报单状态
	TZeusingFtdcOrderStatusType	QuoteStatus;
	///买方剩余数量
	TZeusingFtdcVolumeType	BidVolumeTotal;
	///卖方剩余数量
	TZeusingFtdcVolumeType	AskVolumeTotal;
	///买方成交数量
	TZeusingFtdcVolumeType	BidVolumeTraded;
	///卖方成交数量
	TZeusingFtdcVolumeType	AskVolumeTraded;
	///报单日期
	TZeusingFtdcDateType	InsertDate;
	///委托时间
	TZeusingFtdcTimeType	InsertTime;
	///激活时间
	TZeusingFtdcTimeType	ActiveTime;
	///挂起时间
	TZeusingFtdcTimeType	SuspendTime;
	///最后修改时间
	TZeusingFtdcTimeType	UpdateTime;
	///撤销时间
	TZeusingFtdcTimeType	CancelTime;
	///最后修改交易所交易员代码
	TZeusingFtdcTraderIDType	ActiveTraderID;
	///结算会员编号
	TZeusingFtdcParticipantIDType	ClearingPartID;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
	///状态信息
	TZeusingFtdcErrorMsgType	StatusMsg;
	///操作用户代码
	TZeusingFtdcUserIDType	ActiveUserID;
	///经纪公司报单编号
	TZeusingFtdcSequenceNoType	BrokerOrderSeq;
	///交易编码类型
	TZeusingFtdcClientIDTypeType	ClientIDType;
};

///资金内转
struct CZeusingFtdcInternalTransferField
{
	///出入金流水号
	TZeusingFtdcDepositSeqNoType	DepositSeqNo;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///转出投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///转入投资者代码
	TZeusingFtdcInvestorIDType	InInvestorID;
	///内转金额
	TZeusingFtdcMoneyType	Deposit;
	///是否强制进行
	TZeusingFtdcBoolType	IsForce;
};

///报价
struct CZeusingFtdcQuoteActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///报单操作引用
	TZeusingFtdcOrderActionRefType	QuoteActionRef;
	///报单引用
	TZeusingFtdcOrderRefType	QuoteRef;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	QuoteSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///操作日期
	TZeusingFtdcDateType	ActionDate;
	///操作时间
	TZeusingFtdcTimeType	ActionTime;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地报价编号
	TZeusingFtdcOrderLocalIDType	QuoteLocalID;
	///操作本地编号
	TZeusingFtdcOrderLocalIDType	ActionLocalID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///报单操作状态
	TZeusingFtdcOrderActionStatusType	OrderActionStatus;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///状态信息
	TZeusingFtdcErrorMsgType	StatusMsg;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///报价
struct CZeusingFtdcQryQuoteActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///投资者产品组保证金
struct CZeusingFtdcInvestorProductGroupMarginField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///品种/跨品种标示
	TZeusingFtdcInstrumentIDType	ProductGroupID;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///冻结的保证金
	TZeusingFtdcMoneyType	FrozenMargin;
	///多头冻结的保证金
	TZeusingFtdcMoneyType	LongFrozenMargin;
	///空头冻结的保证金
	TZeusingFtdcMoneyType	ShortFrozenMargin;
	///占用的保证金
	TZeusingFtdcMoneyType	UseMargin;
	///多头保证金
	TZeusingFtdcMoneyType	LongUseMargin;
	///空头保证金
	TZeusingFtdcMoneyType	ShortUseMargin;
	///交易所保证金
	TZeusingFtdcMoneyType	ExchMargin;
	///交易所多头保证金
	TZeusingFtdcMoneyType	LongExchMargin;
	///交易所空头保证金
	TZeusingFtdcMoneyType	ShortExchMargin;
	///平仓盈亏
	TZeusingFtdcMoneyType	CloseProfit;
	///冻结的手续费
	TZeusingFtdcMoneyType	FrozenCommission;
	///手续费
	TZeusingFtdcMoneyType	Commission;
	///冻结的资金
	TZeusingFtdcMoneyType	FrozenCash;
	///资金差额
	TZeusingFtdcMoneyType	CashIn;
	///持仓盈亏
	TZeusingFtdcMoneyType	PositionProfit;
	///折抵总金额
	TZeusingFtdcMoneyType	OffsetAmount;
	///多头折抵总金额
	TZeusingFtdcMoneyType	LongOffsetAmount;
	///空头折抵总金额
	TZeusingFtdcMoneyType	ShortOffsetAmount;
	///交易所折抵总金额
	TZeusingFtdcMoneyType	ExchOffsetAmount;
	///交易所多头折抵总金额
	TZeusingFtdcMoneyType	LongExchOffsetAmount;
	///交易所空头折抵总金额
	TZeusingFtdcMoneyType	ShortExchOffsetAmount;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
};

///投资者产品组保证金
struct CZeusingFtdcQryInvestorProductGroupMarginField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///品种/跨品种标示
	TZeusingFtdcInstrumentIDType	ProductGroupID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
};

///市场行情
struct CZeusingFtdcMarketDataField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///最新价
	TZeusingFtdcPriceType	LastPrice;
	///上次结算价
	TZeusingFtdcPriceType	PreSettlementPrice;
	///昨收盘
	TZeusingFtdcPriceType	PreClosePrice;
	///昨持仓量
	TZeusingFtdcLargeVolumeType	PreOpenInterest;
	///今开盘
	TZeusingFtdcPriceType	OpenPrice;
	///最高价
	TZeusingFtdcPriceType	HighestPrice;
	///最低价
	TZeusingFtdcPriceType	LowestPrice;
	///数量
	TZeusingFtdcVolumeType	Volume;
	///成交金额
	TZeusingFtdcMoneyType	Turnover;
	///持仓量
	TZeusingFtdcLargeVolumeType	OpenInterest;
	///今收盘
	TZeusingFtdcPriceType	ClosePrice;
	///本次结算价
	TZeusingFtdcPriceType	SettlementPrice;
	///涨停板价
	TZeusingFtdcPriceType	UpperLimitPrice;
	///跌停板价
	TZeusingFtdcPriceType	LowerLimitPrice;
	///昨虚实度
	TZeusingFtdcRatioType	PreDelta;
	///今虚实度
	TZeusingFtdcRatioType	CurrDelta;
	///最后修改时间
	TZeusingFtdcTimeType	UpdateTime;
	///最后修改毫秒
	TZeusingFtdcMillisecType	UpdateMillisec;
	///业务日期
	TZeusingFtdcDateType	ActionDay;
};

///行情基础属性
struct CZeusingFtdcMarketDataBaseField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///上次结算价
	TZeusingFtdcPriceType	PreSettlementPrice;
	///昨收盘
	TZeusingFtdcPriceType	PreClosePrice;
	///昨持仓量
	TZeusingFtdcLargeVolumeType	PreOpenInterest;
	///昨虚实度
	TZeusingFtdcRatioType	PreDelta;
};

///行情静态属性
struct CZeusingFtdcMarketDataStaticField
{
	///今开盘
	TZeusingFtdcPriceType	OpenPrice;
	///最高价
	TZeusingFtdcPriceType	HighestPrice;
	///最低价
	TZeusingFtdcPriceType	LowestPrice;
	///今收盘
	TZeusingFtdcPriceType	ClosePrice;
	///涨停板价
	TZeusingFtdcPriceType	UpperLimitPrice;
	///跌停板价
	TZeusingFtdcPriceType	LowerLimitPrice;
	///本次结算价
	TZeusingFtdcPriceType	SettlementPrice;
	///今虚实度
	TZeusingFtdcRatioType	CurrDelta;
};

///行情最新成交属性
struct CZeusingFtdcMarketDataLastMatchField
{
	///最新价
	TZeusingFtdcPriceType	LastPrice;
	///数量
	TZeusingFtdcVolumeType	Volume;
	///成交金额
	TZeusingFtdcMoneyType	Turnover;
	///持仓量
	TZeusingFtdcLargeVolumeType	OpenInterest;
};

///行情最优价属性
struct CZeusingFtdcMarketDataBestPriceField
{
	///申买价一
	TZeusingFtdcPriceType	BidPrice1;
	///申买量一
	TZeusingFtdcVolumeType	BidVolume1;
	///申卖价一
	TZeusingFtdcPriceType	AskPrice1;
	///申卖量一
	TZeusingFtdcVolumeType	AskVolume1;
};

///行情申买二、三属性
struct CZeusingFtdcMarketDataBid23Field
{
	///申买价二
	TZeusingFtdcPriceType	BidPrice2;
	///申买量二
	TZeusingFtdcVolumeType	BidVolume2;
	///申买价三
	TZeusingFtdcPriceType	BidPrice3;
	///申买量三
	TZeusingFtdcVolumeType	BidVolume3;
};

///行情申卖二、三属性
struct CZeusingFtdcMarketDataAsk23Field
{
	///申卖价二
	TZeusingFtdcPriceType	AskPrice2;
	///申卖量二
	TZeusingFtdcVolumeType	AskVolume2;
	///申卖价三
	TZeusingFtdcPriceType	AskPrice3;
	///申卖量三
	TZeusingFtdcVolumeType	AskVolume3;
};

///行情申买四、五属性
struct CZeusingFtdcMarketDataBid45Field
{
	///申买价四
	TZeusingFtdcPriceType	BidPrice4;
	///申买量四
	TZeusingFtdcVolumeType	BidVolume4;
	///申买价五
	TZeusingFtdcPriceType	BidPrice5;
	///申买量五
	TZeusingFtdcVolumeType	BidVolume5;
};

///行情申卖四、五属性
struct CZeusingFtdcMarketDataAsk45Field
{
	///申卖价四
	TZeusingFtdcPriceType	AskPrice4;
	///申卖量四
	TZeusingFtdcVolumeType	AskVolume4;
	///申卖价五
	TZeusingFtdcPriceType	AskPrice5;
	///申卖量五
	TZeusingFtdcVolumeType	AskVolume5;
};

///行情更新时间属性
struct CZeusingFtdcMarketDataUpdateTimeField
{
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///最后修改时间
	TZeusingFtdcTimeType	UpdateTime;
	///最后修改毫秒
	TZeusingFtdcMillisecType	UpdateMillisec;
	///业务日期
	TZeusingFtdcDateType	ActionDay;
};

///行情交易所代码属性
struct CZeusingFtdcMarketDataExchangeField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///指定的合约
struct CZeusingFtdcSpecificInstrumentField
{
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///合约状态
struct CZeusingFtdcInstrumentStatusField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///结算组代码
	TZeusingFtdcSettlementGroupIDType	SettlementGroupID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///合约交易状态
	TZeusingFtdcInstrumentStatusType	InstrumentStatus;
	///交易阶段编号
	TZeusingFtdcTradingSegmentSNType	TradingSegmentSN;
	///进入本状态时间
	TZeusingFtdcTimeType	EnterTime;
	///进入本状态原因
	TZeusingFtdcInstStatusEnterReasonType	EnterReason;
};

///查询合约状态
struct CZeusingFtdcQryInstrumentStatusField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
};

///投资者账户
struct CZeusingFtdcInvestorAccountField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
};

///浮动盈亏算法
struct CZeusingFtdcPositionProfitAlgorithmField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///盈亏算法
	TZeusingFtdcAlgorithmType	Algorithm;
	///备注
	TZeusingFtdcMemoType	Memo;
};

///会员资金折扣
struct CZeusingFtdcDiscountField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///资金折扣比例
	TZeusingFtdcRatioType	Discount;
};

///查询转帐银行
struct CZeusingFtdcQryTransferBankField
{
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分中心代码
	TZeusingFtdcBankBrchIDType	BankBrchID;
};

///转帐银行
struct CZeusingFtdcTransferBankField
{
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分中心代码
	TZeusingFtdcBankBrchIDType	BankBrchID;
	///银行名称
	TZeusingFtdcBankNameType	BankName;
	///是否活跃
	TZeusingFtdcBoolType	IsActive;
};

///查询投资者持仓明细
struct CZeusingFtdcQryInvestorPositionDetailField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///投资者持仓明细
struct CZeusingFtdcInvestorPositionDetailField
{
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///买卖
	TZeusingFtdcDirectionType	Direction;
	///开仓日期
	TZeusingFtdcDateType	OpenDate;
	///成交编号
	TZeusingFtdcTradeIDType	TradeID;
	///数量
	TZeusingFtdcVolumeType	Volume;
	///开仓价
	TZeusingFtdcPriceType	OpenPrice;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///成交类型
	TZeusingFtdcTradeTypeType	TradeType;
	///组合合约代码
	TZeusingFtdcInstrumentIDType	CombInstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///逐日盯市平仓盈亏
	TZeusingFtdcMoneyType	CloseProfitByDate;
	///逐笔对冲平仓盈亏
	TZeusingFtdcMoneyType	CloseProfitByTrade;
	///逐日盯市持仓盈亏
	TZeusingFtdcMoneyType	PositionProfitByDate;
	///逐笔对冲持仓盈亏
	TZeusingFtdcMoneyType	PositionProfitByTrade;
	///投资者保证金
	TZeusingFtdcMoneyType	Margin;
	///交易所保证金
	TZeusingFtdcMoneyType	ExchMargin;
	///保证金率
	TZeusingFtdcRatioType	MarginRateByMoney;
	///保证金率(按手数)
	TZeusingFtdcRatioType	MarginRateByVolume;
	///昨结算价
	TZeusingFtdcPriceType	LastSettlementPrice;
	///结算价
	TZeusingFtdcPriceType	SettlementPrice;
	///平仓量
	TZeusingFtdcVolumeType	CloseVolume;
	///平仓金额
	TZeusingFtdcMoneyType	CloseAmount;
};

///资金账户口令域
struct CZeusingFtdcTradingAccountPasswordField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///密码
	TZeusingFtdcPasswordType	Password;
};

///交易所行情报盘机
struct CZeusingFtdcMDTraderOfferField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///交易所交易员连接状态
	TZeusingFtdcTraderConnectStatusType	TraderConnectStatus;
	///发出连接请求的日期
	TZeusingFtdcDateType	ConnectRequestDate;
	///发出连接请求的时间
	TZeusingFtdcTimeType	ConnectRequestTime;
	///上次报告日期
	TZeusingFtdcDateType	LastReportDate;
	///上次报告时间
	TZeusingFtdcTimeType	LastReportTime;
	///完成连接日期
	TZeusingFtdcDateType	ConnectDate;
	///完成连接时间
	TZeusingFtdcTimeType	ConnectTime;
	///启动日期
	TZeusingFtdcDateType	StartDate;
	///启动时间
	TZeusingFtdcTimeType	StartTime;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///本席位最大成交编号
	TZeusingFtdcTradeIDType	MaxTradeID;
	///本席位最大报单备拷
	TZeusingFtdcReturnCodeType	MaxOrderMessageReference;
};

///查询行情报盘机
struct CZeusingFtdcQryMDTraderOfferField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
};

///查询客户通知
struct CZeusingFtdcQryNoticeField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
};

///客户通知
struct CZeusingFtdcNoticeField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///消息正文
	TZeusingFtdcContentType	Content;
	///经纪公司通知内容序列号
	TZeusingFtdcSequenceLabelType	SequenceLabel;
};

///用户权限
struct CZeusingFtdcUserRightField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///客户权限类型
	TZeusingFtdcUserRightTypeType	UserRightType;
	///是否禁止
	TZeusingFtdcBoolType	IsForbidden;
};

///查询结算信息确认域
struct CZeusingFtdcQrySettlementInfoConfirmField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///装载结算信息
struct CZeusingFtdcLoadSettlementInfoField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
};

///经纪公司可提资金算法表
struct CZeusingFtdcBrokerWithdrawAlgorithmField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///可提资金算法
	TZeusingFtdcAlgorithmType	WithdrawAlgorithm;
	///资金使用率
	TZeusingFtdcRatioType	UsingRatio;
	///可提是否包含平仓盈利
	TZeusingFtdcIncludeCloseProfitType	IncludeCloseProfit;
	///本日无仓且无成交客户是否受可提比例限制
	TZeusingFtdcAllWithoutTradeType	AllWithoutTrade;
	///可用是否包含平仓盈利
	TZeusingFtdcIncludeCloseProfitType	AvailIncludeCloseProfit;
	///是否启用用户事件
	TZeusingFtdcBoolType	IsBrokerUserEvent;
};

///资金账户口令变更域
struct CZeusingFtdcTradingAccountPasswordUpdateV1Field
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///原来的口令
	TZeusingFtdcPasswordType	OldPassword;
	///新的口令
	TZeusingFtdcPasswordType	NewPassword;
};

///资金账户口令变更域
struct CZeusingFtdcTradingAccountPasswordUpdateField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///原来的口令
	TZeusingFtdcPasswordType	OldPassword;
	///新的口令
	TZeusingFtdcPasswordType	NewPassword;
};

///查询组合合约分腿
struct CZeusingFtdcQryCombinationLegField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///组合合约代码
	TZeusingFtdcInstrumentIDType	CombInstrumentID;
	///单腿编号
	TZeusingFtdcLegIDType	LegID;
	///单腿合约代码
	TZeusingFtdcInstrumentIDType	LegInstrumentID;
};

///查询组合合约分腿
struct CZeusingFtdcQrySyncStatusField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
};

///组合交易合约的单腿
struct CZeusingFtdcCombinationLegField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///组合合约代码
	TZeusingFtdcInstrumentIDType	CombInstrumentID;
	///单腿编号
	TZeusingFtdcLegIDType	LegID;
	///单腿合约代码
	TZeusingFtdcInstrumentIDType	LegInstrumentID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///单腿乘数
	TZeusingFtdcLegMultipleType	LegMultiple;
	///派生层数
	TZeusingFtdcImplyLevelType	ImplyLevel;
};

///数据同步状态
struct CZeusingFtdcSyncStatusField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///数据同步状态
	TZeusingFtdcDataSyncStatusType	DataSyncStatus;
};

///查询联系人
struct CZeusingFtdcQryLinkManField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///联系人
struct CZeusingFtdcLinkManField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///联系人类型
	TZeusingFtdcPersonTypeType	PersonType;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdentifiedCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///名称
	TZeusingFtdcPartyNameType	PersonName;
	///联系电话
	TZeusingFtdcTelephoneType	Telephone;
	///通讯地址
	TZeusingFtdcAddressType	Address;
	///邮政编码
	TZeusingFtdcZipCodeType	ZipCode;
	///优先级
	TZeusingFtdcPriorityType	Priority;
};

///查询经纪公司用户事件
struct CZeusingFtdcQryBrokerUserEventField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///用户事件类型
	TZeusingFtdcUserEventTypeType	UserEventType;
};

///查询经纪公司用户事件
struct CZeusingFtdcBrokerUserEventField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///用户事件类型
	TZeusingFtdcUserEventTypeType	UserEventType;
	///用户事件序号
	TZeusingFtdcSequenceNoType	EventSequenceNo;
	///事件发生日期
	TZeusingFtdcDateType	EventDate;
	///事件发生时间
	TZeusingFtdcTimeType	EventTime;
	///用户事件信息
	TZeusingFtdcUserEventInfoType	UserEventInfo;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///查询签约银行请求
struct CZeusingFtdcQryContractBankField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分中心代码
	TZeusingFtdcBankBrchIDType	BankBrchID;
};

///查询签约银行响应
struct CZeusingFtdcContractBankField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分中心代码
	TZeusingFtdcBankBrchIDType	BankBrchID;
	///银行名称
	TZeusingFtdcBankNameType	BankName;
};

///投资者组合持仓明细
struct CZeusingFtdcInvestorPositionCombineDetailField
{
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///开仓日期
	TZeusingFtdcDateType	OpenDate;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///组合编号
	TZeusingFtdcTradeIDType	ComTradeID;
	///撮合编号
	TZeusingFtdcTradeIDType	TradeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///买卖
	TZeusingFtdcDirectionType	Direction;
	///持仓量
	TZeusingFtdcVolumeType	TotalAmt;
	///投资者保证金
	TZeusingFtdcMoneyType	Margin;
	///交易所保证金
	TZeusingFtdcMoneyType	ExchMargin;
	///保证金率
	TZeusingFtdcRatioType	MarginRateByMoney;
	///保证金率(按手数)
	TZeusingFtdcRatioType	MarginRateByVolume;
	///单腿编号
	TZeusingFtdcLegIDType	LegID;
	///单腿乘数
	TZeusingFtdcLegMultipleType	LegMultiple;
	///组合持仓合约编码
	TZeusingFtdcInstrumentIDType	CombInstrumentID;
	///成交组号
	TZeusingFtdcTradeGroupIDType	TradeGroupID;
};

///预埋单
struct CZeusingFtdcParkedOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///报单价格条件
	TZeusingFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///组合开平标志
	TZeusingFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量
	TZeusingFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
	TZeusingFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TZeusingFtdcDateType	GTDDate;
	///成交量类型
	TZeusingFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TZeusingFtdcVolumeType	MinVolume;
	///触发条件
	TZeusingFtdcContingentConditionType	ContingentCondition;
	///止损价
	TZeusingFtdcPriceType	StopPrice;
	///强平原因
	TZeusingFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TZeusingFtdcBoolType	IsAutoSuspend;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///用户强评标志
	TZeusingFtdcBoolType	UserForceClose;
	///互换单标志
	TZeusingFtdcBoolType	IsSwapOrder;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///预埋报单编号
	TZeusingFtdcParkedOrderIDType	ParkedOrderID;
	///用户类型
	TZeusingFtdcUserTypeType	UserType;
	///预埋单状态
	TZeusingFtdcParkedOrderStatusType	Status;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///输入预埋单操作
struct CZeusingFtdcParkedOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///报单操作引用
	TZeusingFtdcOrderActionRefType	OrderActionRef;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量变化
	TZeusingFtdcVolumeType	VolumeChange;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///预埋撤单单编号
	TZeusingFtdcParkedOrderActionIDType	ParkedOrderActionID;
	///用户类型
	TZeusingFtdcUserTypeType	UserType;
	///预埋撤单状态
	TZeusingFtdcParkedOrderStatusType	Status;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///查询预埋单
struct CZeusingFtdcQryParkedOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///查询预埋撤单
struct CZeusingFtdcQryParkedOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///删除预埋单
struct CZeusingFtdcRemoveParkedOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///预埋报单编号
	TZeusingFtdcParkedOrderIDType	ParkedOrderID;
};

///删除预埋撤单
struct CZeusingFtdcRemoveParkedOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///预埋撤单编号
	TZeusingFtdcParkedOrderActionIDType	ParkedOrderActionID;
};

///经纪公司可提资金算法表
struct CZeusingFtdcInvestorWithdrawAlgorithmField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///可提资金比例
	TZeusingFtdcRatioType	UsingRatio;
};

///查询组合持仓明细
struct CZeusingFtdcQryInvestorPositionCombineDetailField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///组合持仓合约编码
	TZeusingFtdcInstrumentIDType	CombInstrumentID;
};

///成交均价
struct CZeusingFtdcMarketDataAveragePriceField
{
	///当日均价
	TZeusingFtdcPriceType	AveragePrice;
};

///校验投资者密码
struct CZeusingFtdcVerifyInvestorPasswordField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///密码
	TZeusingFtdcPasswordType	Password;
};

///用户IP
struct CZeusingFtdcUserIPField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///IP地址
	TZeusingFtdcIPAddressType	IPAddress;
	///IP地址掩码
	TZeusingFtdcIPAddressType	IPMask;
	///Mac地址
	TZeusingFtdcMacAddressType	MacAddress;
};

///用户事件通知信息
struct CZeusingFtdcTradingNoticeInfoField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///发送时间
	TZeusingFtdcTimeType	SendTime;
	///消息正文
	TZeusingFtdcContentType	FieldContent;
	///序列系列号
	TZeusingFtdcSequenceSeriesType	SequenceSeries;
	///序列号
	TZeusingFtdcSequenceNoType	SequenceNo;
};

///用户事件通知
struct CZeusingFtdcTradingNoticeField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者范围
	TZeusingFtdcInvestorRangeType	InvestorRange;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///序列系列号
	TZeusingFtdcSequenceSeriesType	SequenceSeries;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///发送时间
	TZeusingFtdcTimeType	SendTime;
	///序列号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///消息正文
	TZeusingFtdcContentType	FieldContent;
};

///查询交易事件通知
struct CZeusingFtdcQryTradingNoticeField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///查询错误报单
struct CZeusingFtdcQryErrOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///错误报单
struct CZeusingFtdcErrOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///报单价格条件
	TZeusingFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///组合开平标志
	TZeusingFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量
	TZeusingFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
	TZeusingFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TZeusingFtdcDateType	GTDDate;
	///成交量类型
	TZeusingFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TZeusingFtdcVolumeType	MinVolume;
	///触发条件
	TZeusingFtdcContingentConditionType	ContingentCondition;
	///止损价
	TZeusingFtdcPriceType	StopPrice;
	///强平原因
	TZeusingFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TZeusingFtdcBoolType	IsAutoSuspend;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///用户强评标志
	TZeusingFtdcBoolType	UserForceClose;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
	///互换单标志
	TZeusingFtdcBoolType	IsSwapOrder;
};

///查询错误报单操作
struct CZeusingFtdcErrorConditionalOrderField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///报单价格条件
	TZeusingFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///组合开平标志
	TZeusingFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TZeusingFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量
	TZeusingFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
	TZeusingFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TZeusingFtdcDateType	GTDDate;
	///成交量类型
	TZeusingFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TZeusingFtdcVolumeType	MinVolume;
	///触发条件
	TZeusingFtdcContingentConditionType	ContingentCondition;
	///止损价
	TZeusingFtdcPriceType	StopPrice;
	///强平原因
	TZeusingFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TZeusingFtdcBoolType	IsAutoSuspend;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///合约在交易所的代码
	TZeusingFtdcExchangeInstIDType	ExchangeInstID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///报单提交状态
	TZeusingFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///报单提示序号
	TZeusingFtdcSequenceNoType	NotifySequence;
	///交易日
	TZeusingFtdcDateType	TradingDay;
	///结算编号
	TZeusingFtdcSettlementIDType	SettlementID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///报单来源
	TZeusingFtdcOrderSourceType	OrderSource;
	///报单状态
	TZeusingFtdcOrderStatusType	OrderStatus;
	///报单类型
	TZeusingFtdcOrderTypeType	OrderType;
	///今成交数量
	TZeusingFtdcVolumeType	VolumeTraded;
	///剩余数量
	TZeusingFtdcVolumeType	VolumeTotal;
	///报单日期
	TZeusingFtdcDateType	InsertDate;
	///委托时间
	TZeusingFtdcTimeType	InsertTime;
	///激活时间
	TZeusingFtdcTimeType	ActiveTime;
	///挂起时间
	TZeusingFtdcTimeType	SuspendTime;
	///最后修改时间
	TZeusingFtdcTimeType	UpdateTime;
	///撤销时间
	TZeusingFtdcTimeType	CancelTime;
	///最后修改交易所交易员代码
	TZeusingFtdcTraderIDType	ActiveTraderID;
	///结算会员编号
	TZeusingFtdcParticipantIDType	ClearingPartID;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///用户端产品信息
	TZeusingFtdcProductInfoType	UserProductInfo;
	///状态信息
	TZeusingFtdcErrorMsgType	StatusMsg;
	///用户强评标志
	TZeusingFtdcBoolType	UserForceClose;
	///操作用户代码
	TZeusingFtdcUserIDType	ActiveUserID;
	///经纪公司报单编号
	TZeusingFtdcSequenceNoType	BrokerOrderSeq;
	///相关报单
	TZeusingFtdcOrderSysIDType	RelativeOrderSysID;
	///郑商所成交数量
	TZeusingFtdcVolumeType	ZCETotalTradedVolume;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
	///互换单标志
	TZeusingFtdcBoolType	IsSwapOrder;
};

///查询错误报单操作
struct CZeusingFtdcQryErrOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///错误报单操作
struct CZeusingFtdcErrOrderActionField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///报单操作引用
	TZeusingFtdcOrderActionRefType	OrderActionRef;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///前置编号
	TZeusingFtdcFrontIDType	FrontID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///报单编号
	TZeusingFtdcOrderSysIDType	OrderSysID;
	///操作标志
	TZeusingFtdcActionFlagType	ActionFlag;
	///价格
	TZeusingFtdcPriceType	LimitPrice;
	///数量变化
	TZeusingFtdcVolumeType	VolumeChange;
	///操作日期
	TZeusingFtdcDateType	ActionDate;
	///操作时间
	TZeusingFtdcTimeType	ActionTime;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///操作本地编号
	TZeusingFtdcOrderLocalIDType	ActionLocalID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
	///报单操作状态
	TZeusingFtdcOrderActionStatusType	OrderActionStatus;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///状态信息
	TZeusingFtdcErrorMsgType	StatusMsg;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///查询交易所状态
struct CZeusingFtdcQryExchangeSequenceField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///交易所状态
struct CZeusingFtdcExchangeSequenceField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///序号
	TZeusingFtdcSequenceNoType	SequenceNo;
	///合约交易状态
	TZeusingFtdcInstrumentStatusType	MarketStatus;
};

///根据价格查询最大报单数量
struct CZeusingFtdcQueryMaxOrderVolumeWithPriceField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///开平标志
	TZeusingFtdcOffsetFlagType	OffsetFlag;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///最大允许报单数量
	TZeusingFtdcVolumeType	MaxVolume;
	///报单价格
	TZeusingFtdcPriceType	Price;
};

///查询经纪公司交易参数
struct CZeusingFtdcQryBrokerTradingParamsField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///经纪公司交易参数
struct CZeusingFtdcBrokerTradingParamsField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///保证金价格类型
	TZeusingFtdcMarginPriceTypeType	MarginPriceType;
	///盈亏算法
	TZeusingFtdcAlgorithmType	Algorithm;
	///可用是否包含平仓盈利
	TZeusingFtdcIncludeCloseProfitType	AvailIncludeCloseProfit;
};

///查询经纪公司交易算法
struct CZeusingFtdcQryBrokerTradingAlgosField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///经纪公司交易算法
struct CZeusingFtdcBrokerTradingAlgosField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///持仓处理算法编号
	TZeusingFtdcHandlePositionAlgoIDType	HandlePositionAlgoID;
	///寻找保证金率算法编号
	TZeusingFtdcFindMarginRateAlgoIDType	FindMarginRateAlgoID;
	///资金处理算法编号
	TZeusingFtdcHandleTradingAccountAlgoIDType	HandleTradingAccountAlgoID;
};

///查询经纪公司资金
struct CZeusingFtdcQueryBrokerDepositField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
};

///经纪公司资金
struct CZeusingFtdcBrokerDepositField
{
	///交易日期
	TZeusingFtdcTradeDateType	TradingDay;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///上次结算准备金
	TZeusingFtdcMoneyType	PreBalance;
	///当前保证金总额
	TZeusingFtdcMoneyType	CurrMargin;
	///平仓盈亏
	TZeusingFtdcMoneyType	CloseProfit;
	///期货结算准备金
	TZeusingFtdcMoneyType	Balance;
	///入金金额
	TZeusingFtdcMoneyType	Deposit;
	///出金金额
	TZeusingFtdcMoneyType	Withdraw;
	///可提资金
	TZeusingFtdcMoneyType	Available;
	///基本准备金
	TZeusingFtdcMoneyType	Reserve;
	///冻结的保证金
	TZeusingFtdcMoneyType	FrozenMargin;
};

///查询保证金监管系统经纪公司密钥
struct CZeusingFtdcQryCFMMCBrokerKeyField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
};

///保证金监管系统经纪公司密钥
struct CZeusingFtdcCFMMCBrokerKeyField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///经纪公司统一编码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///密钥生成日期
	TZeusingFtdcDateType	CreateDate;
	///密钥生成时间
	TZeusingFtdcTimeType	CreateTime;
	///密钥编号
	TZeusingFtdcSequenceNoType	KeyID;
	///动态密钥
	TZeusingFtdcCFMMCKeyType	CurrentKey;
	///动态密钥类型
	TZeusingFtdcCFMMCKeyKindType	KeyKind;
};

///保证金监管系统经纪公司资金账户密钥
struct CZeusingFtdcCFMMCTradingAccountKeyField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///经纪公司统一编码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///密钥编号
	TZeusingFtdcSequenceNoType	KeyID;
	///动态密钥
	TZeusingFtdcCFMMCKeyType	CurrentKey;
};

///请求查询保证金监管系统经纪公司资金账户密钥
struct CZeusingFtdcQryCFMMCTradingAccountKeyField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
};

///用户动态令牌参数
struct CZeusingFtdcBrokerUserOTPParamField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///动态令牌提供商
	TZeusingFtdcOTPVendorsIDType	OTPVendorsID;
	///动态令牌序列号
	TZeusingFtdcSerialNumberType	SerialNumber;
	///令牌密钥
	TZeusingFtdcAuthKeyType	AuthKey;
	///漂移值
	TZeusingFtdcLastDriftType	LastDrift;
	///成功值
	TZeusingFtdcLastSuccessType	LastSuccess;
	///动态令牌类型
	TZeusingFtdcOTPTypeType	OTPType;
};

///手工同步用户动态令牌
struct CZeusingFtdcManualSyncBrokerUserOTPField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///动态令牌类型
	TZeusingFtdcOTPTypeType	OTPType;
	///第一个动态密码
	TZeusingFtdcPasswordType	FirstOTP;
	///第二个动态密码
	TZeusingFtdcPasswordType	SecondOTP;
};

///投资者手续费率模板
struct CZeusingFtdcCommRateModelField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///手续费率模板代码
	TZeusingFtdcInvestorIDType	CommModelID;
	///模板名称
	TZeusingFtdcCommModelNameType	CommModelName;
};

///请求查询投资者手续费率模板
struct CZeusingFtdcQryCommRateModelField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///手续费率模板代码
	TZeusingFtdcInvestorIDType	CommModelID;
};

///投资者保证金率模板
struct CZeusingFtdcMarginModelField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///保证金率模板代码
	TZeusingFtdcInvestorIDType	MarginModelID;
	///模板名称
	TZeusingFtdcCommModelNameType	MarginModelName;
};

///仓单折抵信息
struct CZeusingFtdcEWarrantOffsetField
{
	///交易日期
	TZeusingFtdcTradeDateType	TradingDay;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TZeusingFtdcDirectionType	Direction;
	///投机套保标志
	TZeusingFtdcHedgeFlagType	HedgeFlag;
	///数量
	TZeusingFtdcVolumeType	Volume;
};

///查询仓单折抵信息
struct CZeusingFtdcQryEWarrantOffsetField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
};

///转帐开户请求
struct CZeusingFtdcReqOpenAccountField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///性别
	TZeusingFtdcGenderType	Gender;
	///国家代码
	TZeusingFtdcCountryCodeType	CountryCode;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///地址
	TZeusingFtdcAddressType	Address;
	///邮编
	TZeusingFtdcZipCodeType	ZipCode;
	///电话号码
	TZeusingFtdcTelephoneType	Telephone;
	///手机
	TZeusingFtdcMobilePhoneType	MobilePhone;
	///传真
	TZeusingFtdcFaxType	Fax;
	///电子邮件
	TZeusingFtdcEMailType	EMail;
	///资金账户状态
	TZeusingFtdcMoneyAccountStatusType	MoneyAccountStatus;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///汇钞标志
	TZeusingFtdcCashExchangeCodeType	CashExchangeCode;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
};

///转帐销户请求
struct CZeusingFtdcReqCancelAccountField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///性别
	TZeusingFtdcGenderType	Gender;
	///国家代码
	TZeusingFtdcCountryCodeType	CountryCode;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///地址
	TZeusingFtdcAddressType	Address;
	///邮编
	TZeusingFtdcZipCodeType	ZipCode;
	///电话号码
	TZeusingFtdcTelephoneType	Telephone;
	///手机
	TZeusingFtdcMobilePhoneType	MobilePhone;
	///传真
	TZeusingFtdcFaxType	Fax;
	///电子邮件
	TZeusingFtdcEMailType	EMail;
	///资金账户状态
	TZeusingFtdcMoneyAccountStatusType	MoneyAccountStatus;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///汇钞标志
	TZeusingFtdcCashExchangeCodeType	CashExchangeCode;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
};

///变更银行账户请求
struct CZeusingFtdcReqChangeAccountField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///性别
	TZeusingFtdcGenderType	Gender;
	///国家代码
	TZeusingFtdcCountryCodeType	CountryCode;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///地址
	TZeusingFtdcAddressType	Address;
	///邮编
	TZeusingFtdcZipCodeType	ZipCode;
	///电话号码
	TZeusingFtdcTelephoneType	Telephone;
	///手机
	TZeusingFtdcMobilePhoneType	MobilePhone;
	///传真
	TZeusingFtdcFaxType	Fax;
	///电子邮件
	TZeusingFtdcEMailType	EMail;
	///资金账户状态
	TZeusingFtdcMoneyAccountStatusType	MoneyAccountStatus;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///新银行帐号
	TZeusingFtdcBankAccountType	NewBankAccount;
	///新银行密码
	TZeusingFtdcPasswordType	NewBankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///摘要
	TZeusingFtdcDigestType	Digest;
};

///转账请求
struct CZeusingFtdcReqTransferField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///期货公司流水号
	TZeusingFtdcFutureSerialType	FutureSerial;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TZeusingFtdcTradeAmountType	TradeAmount;
	///期货可取金额
	TZeusingFtdcTradeAmountType	FutureFetchAmount;
	///费用支付标志
	TZeusingFtdcFeePayFlagType	FeePayFlag;
	///应收客户费用
	TZeusingFtdcCustFeeType	CustFee;
	///应收期货公司费用
	TZeusingFtdcFutureFeeType	BrokerFee;
	///发送方给接收方的消息
	TZeusingFtdcAddInfoType	Message;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///转账交易状态
	TZeusingFtdcTransferStatusType	TransferStatus;
};

///银行发起银行资金转期货响应
struct CZeusingFtdcRspTransferField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///期货公司流水号
	TZeusingFtdcFutureSerialType	FutureSerial;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TZeusingFtdcTradeAmountType	TradeAmount;
	///期货可取金额
	TZeusingFtdcTradeAmountType	FutureFetchAmount;
	///费用支付标志
	TZeusingFtdcFeePayFlagType	FeePayFlag;
	///应收客户费用
	TZeusingFtdcCustFeeType	CustFee;
	///应收期货公司费用
	TZeusingFtdcFutureFeeType	BrokerFee;
	///发送方给接收方的消息
	TZeusingFtdcAddInfoType	Message;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///转账交易状态
	TZeusingFtdcTransferStatusType	TransferStatus;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///冲正请求
struct CZeusingFtdcReqRepealField
{
	///冲正时间间隔
	TZeusingFtdcRepealTimeIntervalType	RepealTimeInterval;
	///已经冲正次数
	TZeusingFtdcRepealedTimesType	RepealedTimes;
	///银行冲正标志
	TZeusingFtdcBankRepealFlagType	BankRepealFlag;
	///期商冲正标志
	TZeusingFtdcBrokerRepealFlagType	BrokerRepealFlag;
	///被冲正平台流水号
	TZeusingFtdcPlateSerialType	PlateRepealSerial;
	///被冲正银行流水号
	TZeusingFtdcBankSerialType	BankRepealSerial;
	///被冲正期货流水号
	TZeusingFtdcFutureSerialType	FutureRepealSerial;
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///期货公司流水号
	TZeusingFtdcFutureSerialType	FutureSerial;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TZeusingFtdcTradeAmountType	TradeAmount;
	///期货可取金额
	TZeusingFtdcTradeAmountType	FutureFetchAmount;
	///费用支付标志
	TZeusingFtdcFeePayFlagType	FeePayFlag;
	///应收客户费用
	TZeusingFtdcCustFeeType	CustFee;
	///应收期货公司费用
	TZeusingFtdcFutureFeeType	BrokerFee;
	///发送方给接收方的消息
	TZeusingFtdcAddInfoType	Message;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///转账交易状态
	TZeusingFtdcTransferStatusType	TransferStatus;
};

///冲正响应
struct CZeusingFtdcRspRepealField
{
	///冲正时间间隔
	TZeusingFtdcRepealTimeIntervalType	RepealTimeInterval;
	///已经冲正次数
	TZeusingFtdcRepealedTimesType	RepealedTimes;
	///银行冲正标志
	TZeusingFtdcBankRepealFlagType	BankRepealFlag;
	///期商冲正标志
	TZeusingFtdcBrokerRepealFlagType	BrokerRepealFlag;
	///被冲正平台流水号
	TZeusingFtdcPlateSerialType	PlateRepealSerial;
	///被冲正银行流水号
	TZeusingFtdcBankSerialType	BankRepealSerial;
	///被冲正期货流水号
	TZeusingFtdcFutureSerialType	FutureRepealSerial;
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///期货公司流水号
	TZeusingFtdcFutureSerialType	FutureSerial;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TZeusingFtdcTradeAmountType	TradeAmount;
	///期货可取金额
	TZeusingFtdcTradeAmountType	FutureFetchAmount;
	///费用支付标志
	TZeusingFtdcFeePayFlagType	FeePayFlag;
	///应收客户费用
	TZeusingFtdcCustFeeType	CustFee;
	///应收期货公司费用
	TZeusingFtdcFutureFeeType	BrokerFee;
	///发送方给接收方的消息
	TZeusingFtdcAddInfoType	Message;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///转账交易状态
	TZeusingFtdcTransferStatusType	TransferStatus;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///查询账户信息请求
struct CZeusingFtdcReqQueryAccountField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///期货公司流水号
	TZeusingFtdcFutureSerialType	FutureSerial;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
};

///查询账户信息响应
struct CZeusingFtdcRspQueryAccountField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///期货公司流水号
	TZeusingFtdcFutureSerialType	FutureSerial;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///银行可用金额
	TZeusingFtdcTradeAmountType	BankUseAmount;
	///银行可取金额
	TZeusingFtdcTradeAmountType	BankFetchAmount;
};

///期商签到签退
struct CZeusingFtdcFutureSignIOField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
};

///期商签到响应
struct CZeusingFtdcRspFutureSignInField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
	///PIN密钥
	TZeusingFtdcPasswordKeyType	PinKey;
	///MAC密钥
	TZeusingFtdcPasswordKeyType	MacKey;
};

///期商签退请求
struct CZeusingFtdcReqFutureSignOutField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
};

///期商签退响应
struct CZeusingFtdcRspFutureSignOutField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///查询指定流水号的交易结果请求
struct CZeusingFtdcReqQueryTradeResultBySerialField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///流水号
	TZeusingFtdcSerialType	Reference;
	///本流水号发布者的机构类型
	TZeusingFtdcInstitutionTypeType	RefrenceIssureType;
	///本流水号发布者机构编码
	TZeusingFtdcOrganCodeType	RefrenceIssure;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TZeusingFtdcTradeAmountType	TradeAmount;
	///摘要
	TZeusingFtdcDigestType	Digest;
};

///查询指定流水号的交易结果响应
struct CZeusingFtdcRspQueryTradeResultBySerialField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
	///流水号
	TZeusingFtdcSerialType	Reference;
	///本流水号发布者的机构类型
	TZeusingFtdcInstitutionTypeType	RefrenceIssureType;
	///本流水号发布者机构编码
	TZeusingFtdcOrganCodeType	RefrenceIssure;
	///原始返回代码
	TZeusingFtdcReturnCodeType	OriginReturnCode;
	///原始返回码描述
	TZeusingFtdcDescrInfoForReturnCodeType	OriginDescrInfoForReturnCode;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///转帐金额
	TZeusingFtdcTradeAmountType	TradeAmount;
	///摘要
	TZeusingFtdcDigestType	Digest;
};

///日终文件就绪请求
struct CZeusingFtdcReqDayEndFileReadyField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///文件业务功能
	TZeusingFtdcFileBusinessCodeType	FileBusinessCode;
	///摘要
	TZeusingFtdcDigestType	Digest;
};

///返回结果
struct CZeusingFtdcReturnResultField
{
	///返回代码
	TZeusingFtdcReturnCodeType	ReturnCode;
	///返回码描述
	TZeusingFtdcDescrInfoForReturnCodeType	DescrInfoForReturnCode;
};

///验证期货资金密码
struct CZeusingFtdcVerifyFuturePasswordField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///交易ID
	TZeusingFtdcTIDType	TID;
};

///验证客户信息
struct CZeusingFtdcVerifyCustInfoField
{
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
};

///验证期货资金密码和客户信息
struct CZeusingFtdcVerifyFuturePasswordAndCustInfoField
{
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
};

///验证期货资金密码和客户信息
struct CZeusingFtdcDepositResultInformField
{
	///出入金流水号，该流水号为银期报盘返回的流水号
	TZeusingFtdcDepositSeqNoType	DepositSeqNo;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///入金金额
	TZeusingFtdcMoneyType	Deposit;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///返回代码
	TZeusingFtdcReturnCodeType	ReturnCode;
	///返回码描述
	TZeusingFtdcDescrInfoForReturnCodeType	DescrInfoForReturnCode;
};

///交易核心向银期报盘发出密钥同步请求
struct CZeusingFtdcReqSyncKeyField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///交易核心给银期报盘的消息
	TZeusingFtdcAddInfoType	Message;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
};

///交易核心向银期报盘发出密钥同步响应
struct CZeusingFtdcRspSyncKeyField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///交易核心给银期报盘的消息
	TZeusingFtdcAddInfoType	Message;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///查询账户信息通知
struct CZeusingFtdcNotifyQueryAccountField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///期货公司流水号
	TZeusingFtdcFutureSerialType	FutureSerial;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///银行可用金额
	TZeusingFtdcTradeAmountType	BankUseAmount;
	///银行可取金额
	TZeusingFtdcTradeAmountType	BankFetchAmount;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///银期转账交易流水表
struct CZeusingFtdcTransferSerialField
{
	///平台流水号
	TZeusingFtdcPlateSerialType	PlateSerial;
	///交易发起方日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易日期
	TZeusingFtdcDateType	TradingDay;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///交易代码
	TZeusingFtdcTradeCodeType	TradeCode;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///银行编码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构编码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///期货公司编码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///期货公司帐号类型
	TZeusingFtdcFutureAccTypeType	FutureAccType;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///期货公司流水号
	TZeusingFtdcFutureSerialType	FutureSerial;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///交易金额
	TZeusingFtdcTradeAmountType	TradeAmount;
	///应收客户费用
	TZeusingFtdcCustFeeType	CustFee;
	///应收期货公司费用
	TZeusingFtdcFutureFeeType	BrokerFee;
	///有效标志
	TZeusingFtdcAvailabilityFlagType	AvailabilityFlag;
	///操作员
	TZeusingFtdcOperatorCodeType	OperatorCode;
	///新银行帐号
	TZeusingFtdcBankAccountType	BankNewAccount;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///请求查询转帐流水
struct CZeusingFtdcQryTransferSerialField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///银行编码
	TZeusingFtdcBankIDType	BankID;
};

///期商签到通知
struct CZeusingFtdcNotifyFutureSignInField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
	///PIN密钥
	TZeusingFtdcPasswordKeyType	PinKey;
	///MAC密钥
	TZeusingFtdcPasswordKeyType	MacKey;
};

///期商签退通知
struct CZeusingFtdcNotifyFutureSignOutField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///交易核心向银期报盘发出密钥同步处理结果的通知
struct CZeusingFtdcNotifySyncKeyField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///交易核心给银期报盘的消息
	TZeusingFtdcAddInfoType	Message;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///请求编号
	TZeusingFtdcRequestIDType	RequestID;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///请求查询银期签约关系
struct CZeusingFtdcQryAccountregisterField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///银行编码
	TZeusingFtdcBankIDType	BankID;
};

///客户开销户信息表
struct CZeusingFtdcAccountregisterField
{
	///交易日期
	TZeusingFtdcTradeDateType	TradeDay;
	///银行编码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构编码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///期货公司编码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期货公司分支机构编码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///开销户类别
	TZeusingFtdcOpenOrDestroyType	OpenOrDestroy;
	///签约日期
	TZeusingFtdcTradeDateType	RegDate;
	///解约日期
	TZeusingFtdcTradeDateType	OutDate;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
};

///银期开户信息
struct CZeusingFtdcOpenAccountField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///性别
	TZeusingFtdcGenderType	Gender;
	///国家代码
	TZeusingFtdcCountryCodeType	CountryCode;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///地址
	TZeusingFtdcAddressType	Address;
	///邮编
	TZeusingFtdcZipCodeType	ZipCode;
	///电话号码
	TZeusingFtdcTelephoneType	Telephone;
	///手机
	TZeusingFtdcMobilePhoneType	MobilePhone;
	///传真
	TZeusingFtdcFaxType	Fax;
	///电子邮件
	TZeusingFtdcEMailType	EMail;
	///资金账户状态
	TZeusingFtdcMoneyAccountStatusType	MoneyAccountStatus;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///汇钞标志
	TZeusingFtdcCashExchangeCodeType	CashExchangeCode;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///银期销户信息
struct CZeusingFtdcCancelAccountField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///性别
	TZeusingFtdcGenderType	Gender;
	///国家代码
	TZeusingFtdcCountryCodeType	CountryCode;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///地址
	TZeusingFtdcAddressType	Address;
	///邮编
	TZeusingFtdcZipCodeType	ZipCode;
	///电话号码
	TZeusingFtdcTelephoneType	Telephone;
	///手机
	TZeusingFtdcMobilePhoneType	MobilePhone;
	///传真
	TZeusingFtdcFaxType	Fax;
	///电子邮件
	TZeusingFtdcEMailType	EMail;
	///资金账户状态
	TZeusingFtdcMoneyAccountStatusType	MoneyAccountStatus;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///汇钞标志
	TZeusingFtdcCashExchangeCodeType	CashExchangeCode;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///渠道标志
	TZeusingFtdcDeviceIDType	DeviceID;
	///期货单位帐号类型
	TZeusingFtdcBankAccTypeType	BankSecuAccType;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///期货单位帐号
	TZeusingFtdcBankAccountType	BankSecuAcc;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易柜员
	TZeusingFtdcOperNoType	OperNo;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///用户标识
	TZeusingFtdcUserIDType	UserID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///银期变更银行账号信息
struct CZeusingFtdcChangeAccountField
{
	///业务功能码
	TZeusingFtdcTradeCodeType	TradeCode;
	///银行代码
	TZeusingFtdcBankIDType	BankID;
	///银行分支机构代码
	TZeusingFtdcBankBrchIDType	BankBranchID;
	///期商代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///期商分支机构代码
	TZeusingFtdcFutureBranchIDType	BrokerBranchID;
	///交易日期
	TZeusingFtdcTradeDateType	TradeDate;
	///交易时间
	TZeusingFtdcTradeTimeType	TradeTime;
	///银行流水号
	TZeusingFtdcBankSerialType	BankSerial;
	///交易系统日期 
	TZeusingFtdcTradeDateType	TradingDay;
	///银期平台消息流水号
	TZeusingFtdcSerialType	PlateSerial;
	///最后分片标志
	TZeusingFtdcLastFragmentType	LastFragment;
	///会话号
	TZeusingFtdcSessionIDType	SessionID;
	///客户姓名
	TZeusingFtdcIndividualNameType	CustomerName;
	///证件类型
	TZeusingFtdcIdCardTypeType	IdCardType;
	///证件号码
	TZeusingFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///性别
	TZeusingFtdcGenderType	Gender;
	///国家代码
	TZeusingFtdcCountryCodeType	CountryCode;
	///客户类型
	TZeusingFtdcCustTypeType	CustType;
	///地址
	TZeusingFtdcAddressType	Address;
	///邮编
	TZeusingFtdcZipCodeType	ZipCode;
	///电话号码
	TZeusingFtdcTelephoneType	Telephone;
	///手机
	TZeusingFtdcMobilePhoneType	MobilePhone;
	///传真
	TZeusingFtdcFaxType	Fax;
	///电子邮件
	TZeusingFtdcEMailType	EMail;
	///资金账户状态
	TZeusingFtdcMoneyAccountStatusType	MoneyAccountStatus;
	///银行帐号
	TZeusingFtdcBankAccountType	BankAccount;
	///银行密码
	TZeusingFtdcPasswordType	BankPassWord;
	///新银行帐号
	TZeusingFtdcBankAccountType	NewBankAccount;
	///新银行密码
	TZeusingFtdcPasswordType	NewBankPassWord;
	///投资者帐号
	TZeusingFtdcAccountIDType	AccountID;
	///期货密码
	TZeusingFtdcPasswordType	Password;
	///银行帐号类型
	TZeusingFtdcBankAccTypeType	BankAccType;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///验证客户证件号码标志
	TZeusingFtdcYesNoIndicatorType	VerifyCertNoFlag;
	///币种代码
	TZeusingFtdcCurrencyIDType	CurrencyID;
	///期货公司银行编码
	TZeusingFtdcBankCodingForFutureType	BrokerIDByBank;
	///银行密码标志
	TZeusingFtdcPwdFlagType	BankPwdFlag;
	///期货资金密码核对标志
	TZeusingFtdcPwdFlagType	SecuPwdFlag;
	///交易ID
	TZeusingFtdcTIDType	TID;
	///摘要
	TZeusingFtdcDigestType	Digest;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///灾备中心交易权限
struct CZeusingFtdcUserRightsAssignField
{
	///应用单元代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///交易中心代码
	TZeusingFtdcDRIdentityIDType	DRIdentityID;
};

///经济公司是否有在本标示的交易权限
struct CZeusingFtdcBrokerUserRightAssignField
{
	///应用单元代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易中心代码
	TZeusingFtdcDRIdentityIDType	DRIdentityID;
	///能否交易
	TZeusingFtdcBoolType	Tradeable;
};

///灾备交易转换报文
struct CZeusingFtdcDRTransferField
{
	///原交易中心代码
	TZeusingFtdcDRIdentityIDType	OrigDRIdentityID;
	///目标交易中心代码
	TZeusingFtdcDRIdentityIDType	DestDRIdentityID;
	///原应用单元代码
	TZeusingFtdcBrokerIDType	OrigBrokerID;
	///目标易用单元代码
	TZeusingFtdcBrokerIDType	DestBrokerID;
};

///询价
struct CZeusingFtdcForQuoteField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///用户代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///询价时间
	TZeusingFtdcTimeType	ReqForQuoteTime;
	///询价编号
	TZeusingFtdcOrderSysIDType	ReqForQuoteID;
	///交易日
	TZeusingFtdcDateType	TradingDay;
};

///询价输入
struct CZeusingFtdcInputForQuoteField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///用户代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///询价时间
	TZeusingFtdcTimeType	ReqForQuoteTime;
};

///交易所询价输入
struct CZeusingFtdcExchRspForQuoteField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///用户代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///询价时间
	TZeusingFtdcTimeType	ReqForQuoteTime;
};

///交易所询价错误
struct CZeusingFtdcErrRtnExchRtnForQuoteField
{
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///合约代码
	TZeusingFtdcInstrumentIDType	InstrumentID;
	///会话编号
	TZeusingFtdcSessionIDType	SessionID;
	///报单引用
	TZeusingFtdcOrderRefType	OrderRef;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	OrderLocalID;
	///用户代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///用户代码
	TZeusingFtdcUserIDType	UserID;
	///询价时间
	TZeusingFtdcTimeType	ReqForQuoteTime;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
};

///交易所报价错误
struct CZeusingFtdcExchangeQuoteInsertErrorField
{
	///经纪公司代码
	TZeusingFtdcBrokerIDType	BrokerID;
	///交易所代码
	TZeusingFtdcExchangeIDType	ExchangeID;
	///会员代码
	TZeusingFtdcParticipantIDType	ParticipantID;
	///交易所交易员代码
	TZeusingFtdcTraderIDType	TraderID;
	///安装编号
	TZeusingFtdcInstallIDType	InstallID;
	///本地报单编号
	TZeusingFtdcOrderLocalIDType	QuoteLocalID;
	///错误代码
	TZeusingFtdcErrorIDType	ErrorID;
	///错误信息
	TZeusingFtdcErrorMsgType	ErrorMsg;
	///客户代码
	TZeusingFtdcClientIDType	ClientID;
	///投资者代码
	TZeusingFtdcInvestorIDType	InvestorID;
	///业务单元
	TZeusingFtdcBusinessUnitType	BusinessUnit;
};

///UDP组播组信息
struct CZeusingFtdcMulticastGroupInfoField
{
	///组播组IP地址
	TZeusingFtdcIPAddressType	GroupIP;
	///组播组IP端口
	TZeusingFtdcIPPortType	GroupPort;
	///源地址
	TZeusingFtdcIPAddressType	SourceIP;
};


#pragma pack(pop)

#endif
