#if !defined(SECURITY_FTDCQUERYAPI2_H)
#define SECURITY_FTDCQUERYAPI2_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SecurityFtdcQueryApi.h"

#if defined(ISLIB) && defined(WIN32)
#ifdef LIB_QUERY_API_EXPORT
#define QUERY_API_EXPORT __declspec(dllexport)
#else
#define QUERY_API_EXPORT __declspec(dllimport)
#endif
#else
#define QUERY_API_EXPORT 
#endif

#ifndef WINDOWS
	#if __GNUC__ >= 4
		#pragma GCC visibility push(default)
	#endif
#endif

class QUERY_API_EXPORT CSecurityFtdcQueryApi2
{
public:
	///创建QueryApi
	///@param pszFlowPath 存贮订阅信息文件的目录，默认为当前目录
	///@return 创建出的UserApi
	static CSecurityFtdcQueryApi2 *CreateFtdcQueryApi(const char *pszFlowPath = "");
	
	///删除接口对象本身
	///@remark 不再使用本接口对象时,调用该函数删除接口对象
	virtual void Release() = 0;
	
	///初始化
	///@remark 初始化运行环境,只有调用后,接口才开始工作
	virtual void Init() = 0;
	
	///等待接口线程结束运行
	///@return 线程退出代码
	virtual int Join() = 0;
	
	///获取当前交易日
	///@retrun 获取到的交易日
	///@remark 只有登录成功后,才能得到正确的交易日
	virtual const char *GetTradingDay() = 0;
	
	///注册前置机网络地址
	///@param pszFrontAddress：前置机网络地址。
	///@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:17001”。 
	///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”17001”代表服务器端口号。
	virtual void RegisterFront(char *pszFrontAddress) = 0;
	
	///注册回调接口
	///@param pSpi 派生自回调接口类的实例
	virtual void RegisterSpi(CSecurityFtdcQuerySpi *pSpi) = 0;
	
	///用户登录请求
	virtual int ReqUserLogin(CSecurityFtdcReqUserLoginField *pReqUserLoginField, int nRequestID) = 0;
	

	///登出请求
	virtual int ReqUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, int nRequestID) = 0;

	///获取认证随机码请求
	virtual int ReqFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pAuthRandCode, int nRequestID) = 0;

	///请求查询交易所
	virtual int ReqQryExchange(CSecurityFtdcQryExchangeField *pQryExchange, int nRequestID) = 0;

	///请求查询合约
	virtual int ReqQryInstrument(CSecurityFtdcQryInstrumentField *pQryInstrument, int nRequestID) = 0;

	///请求查询投资者
	virtual int ReqQryInvestor(CSecurityFtdcQryInvestorField *pQryInvestor, int nRequestID) = 0;

	///请求查询交易编码
	virtual int ReqQryTradingCode(CSecurityFtdcQryTradingCodeField *pQryTradingCode, int nRequestID) = 0;

	///请求查询资金账户
	virtual int ReqQryTradingAccount(CSecurityFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID) = 0;

	///请求查询债券利息
	virtual int ReqQryBondInterest(CSecurityFtdcQryBondInterestField *pQryBondInterest, int nRequestID) = 0;

	///请求查询市值配售信息
	virtual int ReqQryMarketRationInfo(CSecurityFtdcQryMarketRationInfoField *pQryMarketRationInfo, int nRequestID) = 0;

	///请求查询合约手续费率
	virtual int ReqQryInstrumentCommissionRate(CSecurityFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate, int nRequestID) = 0;

	///请求查询ETF合约
	virtual int ReqQryETFInstrument(CSecurityFtdcQryETFInstrumentField *pQryETFInstrument, int nRequestID) = 0;

	///请求查询ETF股票篮
	virtual int ReqQryETFBasket(CSecurityFtdcQryETFBasketField *pQryETFBasket, int nRequestID) = 0;

	///请求查询OF合约
	virtual int ReqQryOFInstrument(CSecurityFtdcQryOFInstrumentField *pQryOFInstrument, int nRequestID) = 0;

	///请求查询SF合约
	virtual int ReqQrySFInstrument(CSecurityFtdcQrySFInstrumentField *pQrySFInstrument, int nRequestID) = 0;

	///请求查询合约单手保证金
	virtual int ReqQryInstrumentUnitMargin(CSecurityFtdcQryInstrumentUnitMarginField *pQryInstrumentUnitMargin, int nRequestID) = 0;

	///请求查询预交割信息
	virtual int ReqQryPreDelivInfo(CSecurityFtdcQryPreDelivInfoField *pQryPreDelivInfo, int nRequestID) = 0;

	///请求查询可融券分配信息
	virtual int ReqQryCreditStockAssignInfo(CSecurityFtdcQryCreditStockAssignInfoField *pQryCreditStockAssignInfo, int nRequestID) = 0;

	///请求查询可融资分配信息
	virtual int ReqQryCreditCashAssignInfo(CSecurityFtdcQryCreditCashAssignInfoField *pQryCreditCashAssignInfo, int nRequestID) = 0;

	///请求查询证券折算率
	virtual int ReqQryConversionRate(CSecurityFtdcQryConversionRateField *pQryConversionRate, int nRequestID) = 0;

	///请求查询历史信用负债信息
	virtual int ReqQryHisCreditDebtInfo(CSecurityFtdcQryHisCreditDebtInfoField *pQryHisCreditDebtInfo, int nRequestID) = 0;

	///请求查询行情静态信息
	virtual int ReqQryMarketDataStaticInfo(CSecurityFtdcQryMarketDataStaticInfoField *pQryMarketDataStaticInfo, int nRequestID) = 0;

	///请求查询到期回购信息
	virtual int ReqQryExpireRepurchInfo(CSecurityFtdcQryExpireRepurchInfoField *pQryExpireRepurchInfo, int nRequestID) = 0;

	///请求查询债券质押为标准券比例
	virtual int ReqQryBondPledgeRate(CSecurityFtdcQryBondPledgeRateField *pQryBondPledgeRate, int nRequestID) = 0;

	///请求查询债券质押代码对照关系
	virtual int ReqQryPledgeBond(CSecurityFtdcQryPledgeBondField *pQryPledgeBond, int nRequestID) = 0;

	///请求查询报单
	virtual int ReqQryOrder(CSecurityFtdcQryOrderField *pQryOrder, int nRequestID) = 0;

	///请求查询成交
	virtual int ReqQryTrade(CSecurityFtdcQryTradeField *pQryTrade, int nRequestID) = 0;

	///请求查询投资者持仓
	virtual int ReqQryInvestorPosition(CSecurityFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID) = 0;

	///资金转账查询请求
	virtual int ReqQryFundTransferSerial(CSecurityFtdcQryFundTransferSerialField *pQryFundTransferSerial, int nRequestID) = 0;

	///资金内转流水查询请求
	virtual int ReqQryFundInterTransferSerial(CSecurityFtdcQryFundInterTransferSerialField *pQryFundInterTransferSerial, int nRequestID) = 0;
protected:
	~CSecurityFtdcQueryApi2(){};
};
  
#ifndef WINDOWS
	#if __GNUC__ >= 4
		#pragma GCC visibility pop
	#endif
#endif

#endif
