
#if !defined(ZEUSING_FTDCTRADERAPI_H)
#define ZEUSING_FTDCTRADERAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZeusingFtdcUserApiStruct.h"

#if defined(ISLIB) && defined(WIN32)
#ifdef LIB_TRADER_API_EXPORT
#define TRADER_API_EXPORT __declspec(dllexport)
#else
#define TRADER_API_EXPORT __declspec(dllimport)
#endif
#else
#define TRADER_API_EXPORT 
#endif

class CZeusingFtdcTraderSpi
{
public:
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected(){};
	
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason){};
		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	virtual void OnHeartBeatWarning(int nTimeLapse){};
	
	///客户端认证响应
	virtual void OnRspAuthenticate(CZeusingFtdcRspAuthenticateField *pRspAuthenticateField, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};


	///登录请求响应
	virtual void OnRspUserLogin(CZeusingFtdcRspUserLoginField *pRspUserLogin, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///登出请求响应
	virtual void OnRspUserLogout(CZeusingFtdcUserLogoutField *pUserLogout, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///用户口令更新请求响应
	virtual void OnRspUserPasswordUpdate(CZeusingFtdcUserPasswordUpdateField *pUserPasswordUpdate, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///资金账户口令更新请求响应
	virtual void OnRspTradingAccountPasswordUpdate(CZeusingFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///报单录入请求响应
	virtual void OnRspOrderInsert(CZeusingFtdcInputOrderField *pInputOrder, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///预埋单录入请求响应
	virtual void OnRspParkedOrderInsert(CZeusingFtdcParkedOrderField *pParkedOrder, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///预埋撤单录入请求响应
	virtual void OnRspParkedOrderAction(CZeusingFtdcParkedOrderActionField *pParkedOrderAction, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///报单操作请求响应
	virtual void OnRspOrderAction(CZeusingFtdcInputOrderActionField *pInputOrderAction, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///查询最大报单数量响应
	virtual void OnRspQueryMaxOrderVolume(CZeusingFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CZeusingFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///删除预埋单响应
	virtual void OnRspRemoveParkedOrder(CZeusingFtdcRemoveParkedOrderField *pRemoveParkedOrder, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///删除预埋撤单响应
	virtual void OnRspRemoveParkedOrderAction(CZeusingFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///执行宣告录入请求响应
	virtual void OnRspExecOrderInsert(CZeusingFtdcInputExecOrderField *pInputExecOrder, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///执行宣告操作请求响应
	virtual void OnRspExecOrderAction(CZeusingFtdcInputExecOrderActionField *pInputExecOrderAction, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///报价录入请求响应
	virtual void OnRspQuoteInsert(CZeusingFtdcInputQuoteField *pInputQuote, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///报价操作请求响应
	virtual void OnRspQuoteAction(CZeusingFtdcInputQuoteActionField *pInputQuoteAction, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///询价操作应答
	virtual void OnRspForQuote(CZeusingFtdcInputForQuoteField *pInputForQuote, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求资金内转响应
	virtual void OnRspInternalTransfer(CZeusingFtdcInternalTransferField *pInternalTransfer, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询报单响应
	virtual void OnRspQryOrder(CZeusingFtdcOrderField *pOrder, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询成交响应
	virtual void OnRspQryTrade(CZeusingFtdcTradeField *pTrade, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者持仓响应
	virtual void OnRspQryInvestorPosition(CZeusingFtdcInvestorPositionField *pInvestorPosition, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CZeusingFtdcTradingAccountField *pTradingAccount, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者响应
	virtual void OnRspQryInvestor(CZeusingFtdcInvestorField *pInvestor, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询交易编码响应
	virtual void OnRspQryTradingCode(CZeusingFtdcTradingCodeField *pTradingCode, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询合约保证金率响应
	virtual void OnRspQryInstrumentMarginRate(CZeusingFtdcInstrumentMarginRateField *pInstrumentMarginRate, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询合约手续费率响应
	virtual void OnRspQryInstrumentCommissionRate(CZeusingFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询交易所响应
	virtual void OnRspQryExchange(CZeusingFtdcExchangeField *pExchange, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询合约响应
	virtual void OnRspQryInstrument(CZeusingFtdcInstrumentField *pInstrument, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询行情响应
	virtual void OnRspQryDepthMarketData(CZeusingFtdcDepthMarketDataField *pDepthMarketData, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者结算结果响应
	virtual void OnRspQrySettlementInfo(CZeusingFtdcSettlementInfoField *pSettlementInfo, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询转帐银行响应
	virtual void OnRspQryTransferBank(CZeusingFtdcTransferBankField *pTransferBank, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者持仓明细响应
	virtual void OnRspQryInvestorPositionDetail(CZeusingFtdcInvestorPositionDetailField *pInvestorPositionDetail, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询投资者持仓明细响应
	virtual void OnRspQryInvestorPositionCombineDetail(CZeusingFtdcInvestorPositionCombineDetailField *pInvestorPositionCombineDetail, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///查询保证金监管系统经纪公司资金账户密钥响应
	virtual void OnRspQryCFMMCTradingAccountKey(CZeusingFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询仓单折抵信息响应
	virtual void OnRspQryEWarrantOffset(CZeusingFtdcEWarrantOffsetField *pEWarrantOffset, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询执行宣告响应
	virtual void OnRspQryExecOrder(CZeusingFtdcExecOrderField *pExecOrder, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询执行宣告操作响应
	virtual void OnRspQryExecOrderAction(CZeusingFtdcExecOrderActionField *pExecOrderAction, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询错误执行宣告响应
	virtual void OnRspQryErrExecOrder(CZeusingFtdcErrExecOrderField *pErrExecOrder, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询错误执行宣告操作响应
	virtual void OnRspQryErrExecOrderAction(CZeusingFtdcErrExecOrderActionField *pErrExecOrderAction, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询转帐流水响应
	virtual void OnRspQryTransferSerial(CZeusingFtdcTransferSerialField *pTransferSerial, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询银期签约关系响应
	virtual void OnRspQryAccountregister(CZeusingFtdcAccountregisterField *pAccountregister, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///错误应答
	virtual void OnRspError(CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///报单通知
	virtual void OnRtnOrder(CZeusingFtdcOrderField *pOrder) {};

	///成交通知
	virtual void OnRtnTrade(CZeusingFtdcTradeField *pTrade) {};

	///报单录入错误回报
	virtual void OnErrRtnOrderInsert(CZeusingFtdcInputOrderField *pInputOrder, CZeusingFtdcRspInfoField *pRspInfo) {};

	///报单操作错误回报
	virtual void OnErrRtnOrderAction(CZeusingFtdcOrderActionField *pOrderAction, CZeusingFtdcRspInfoField *pRspInfo) {};

	///合约交易状态通知
	virtual void OnRtnInstrumentStatus(CZeusingFtdcInstrumentStatusField *pInstrumentStatus) {};

	///交易通知
	virtual void OnRtnTradingNotice(CZeusingFtdcTradingNoticeInfoField *pTradingNoticeInfo) {};

	///提示条件单校验错误
	virtual void OnRtnErrorConditionalOrder(CZeusingFtdcErrorConditionalOrderField *pErrorConditionalOrder) {};

	///报价录入错误回报
	virtual void OnErrRtnQuoteInsert(CZeusingFtdcInputQuoteField *pInputQuote, CZeusingFtdcRspInfoField *pRspInfo) {};

	///报价操作错误回报
	virtual void OnErrRtnQuoteAction(CZeusingFtdcQuoteActionField *pQuoteAction, CZeusingFtdcRspInfoField *pRspInfo) {};

	///询价通知
	virtual void OnRtnForQuote(CZeusingFtdcForQuoteField *pForQuote) {};

	///询价回报
	virtual void OnRtnExchRspForQuote(CZeusingFtdcExchRspForQuoteField *pExchRspForQuote) {};

	///提示交易所询价失败
	virtual void OnRtnErrExchRtnForQuote(CZeusingFtdcErrRtnExchRtnForQuoteField *pErrRtnExchRtnForQuote) {};

	///请求查询签约银行响应
	virtual void OnRspQryContractBank(CZeusingFtdcContractBankField *pContractBank, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询交易通知响应
	virtual void OnRspQryTradingNotice(CZeusingFtdcTradingNoticeField *pTradingNotice, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询经纪公司交易参数响应
	virtual void OnRspQryBrokerTradingParams(CZeusingFtdcBrokerTradingParamsField *pBrokerTradingParams, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///请求查询经纪公司交易算法响应
	virtual void OnRspQryBrokerTradingAlgos(CZeusingFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///银行发起银行资金转期货通知
	virtual void OnRtnFromBankToFutureByBank(CZeusingFtdcRspTransferField *pRspTransfer) {};

	///银行发起期货资金转银行通知
	virtual void OnRtnFromFutureToBankByBank(CZeusingFtdcRspTransferField *pRspTransfer) {};

	///银行发起冲正银行转期货通知
	virtual void OnRtnRepealFromBankToFutureByBank(CZeusingFtdcRspRepealField *pRspRepeal) {};

	///银行发起冲正期货转银行通知
	virtual void OnRtnRepealFromFutureToBankByBank(CZeusingFtdcRspRepealField *pRspRepeal) {};

	///期货发起银行资金转期货通知
	virtual void OnRtnFromBankToFutureByFuture(CZeusingFtdcRspTransferField *pRspTransfer) {};

	///期货发起期货资金转银行通知
	virtual void OnRtnFromFutureToBankByFuture(CZeusingFtdcRspTransferField *pRspTransfer) {};

	///系统运行时期货端手工发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
	virtual void OnRtnRepealFromBankToFutureByFutureManual(CZeusingFtdcRspRepealField *pRspRepeal) {};

	///系统运行时期货端手工发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
	virtual void OnRtnRepealFromFutureToBankByFutureManual(CZeusingFtdcRspRepealField *pRspRepeal) {};

	///期货发起查询银行余额通知
	virtual void OnRtnQueryBankBalanceByFuture(CZeusingFtdcNotifyQueryAccountField *pNotifyQueryAccount) {};

	///期货发起银行资金转期货错误回报
	virtual void OnErrRtnBankToFutureByFuture(CZeusingFtdcReqTransferField *pReqTransfer, CZeusingFtdcRspInfoField *pRspInfo) {};

	///期货发起期货资金转银行错误回报
	virtual void OnErrRtnFutureToBankByFuture(CZeusingFtdcReqTransferField *pReqTransfer, CZeusingFtdcRspInfoField *pRspInfo) {};

	///系统运行时期货端手工发起冲正银行转期货错误回报
	virtual void OnErrRtnRepealBankToFutureByFutureManual(CZeusingFtdcReqRepealField *pReqRepeal, CZeusingFtdcRspInfoField *pRspInfo) {};

	///系统运行时期货端手工发起冲正期货转银行错误回报
	virtual void OnErrRtnRepealFutureToBankByFutureManual(CZeusingFtdcReqRepealField *pReqRepeal, CZeusingFtdcRspInfoField *pRspInfo) {};

	///期货发起查询银行余额错误回报
	virtual void OnErrRtnQueryBankBalanceByFuture(CZeusingFtdcReqQueryAccountField *pReqQueryAccount, CZeusingFtdcRspInfoField *pRspInfo) {};

	///期货发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
	virtual void OnRtnRepealFromBankToFutureByFuture(CZeusingFtdcRspRepealField *pRspRepeal) {};

	///期货发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
	virtual void OnRtnRepealFromFutureToBankByFuture(CZeusingFtdcRspRepealField *pRspRepeal) {};

	///期货发起银行资金转期货应答
	virtual void OnRspFromBankToFutureByFuture(CZeusingFtdcReqTransferField *pReqTransfer, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///期货发起期货资金转银行应答
	virtual void OnRspFromFutureToBankByFuture(CZeusingFtdcReqTransferField *pReqTransfer, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///期货发起查询银行余额应答
	virtual void OnRspQueryBankAccountMoneyByFuture(CZeusingFtdcReqQueryAccountField *pReqQueryAccount, CZeusingFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///银行发起银期开户通知
	virtual void OnRtnOpenAccountByBank(CZeusingFtdcOpenAccountField *pOpenAccount) {};

	///银行发起银期销户通知
	virtual void OnRtnCancelAccountByBank(CZeusingFtdcCancelAccountField *pCancelAccount) {};

	///银行发起变更银行账号通知
	virtual void OnRtnChangeAccountByBank(CZeusingFtdcChangeAccountField *pChangeAccount) {};
};

#ifndef WINDOWS
	#if __GNUC__ >= 4
		#pragma GCC visibility push(default)
	#endif
#endif
class TRADER_API_EXPORT CZeusingFtdcTraderApi
{
public:
	///创建TraderApi
	///@param pszFlowPath 存贮订阅信息文件的目录，默认为当前目录
	///@return 创建出的UserApi
	//modify for udp marketdata
	static CZeusingFtdcTraderApi *CreateFtdcTraderApi(const char *pszFlowPath = "", const bool bIsUsingUdp=false);
	
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
	virtual void RegisterFront(char *pszFrontAddress, unsigned int priority = 0) = 0;
	
	///注册回调接口
	///@param pSpi 派生自回调接口类的实例
	virtual void RegisterSpi(CZeusingFtdcTraderSpi *pSpi) = 0;
	
	///订阅私有流。
	///@param nResumeType 私有流重传方式  
	///        ZEUSING_TERT_RESTART:从本交易日开始重传
	///        ZEUSING_TERT_RESUME:从上次收到的续传
	///        ZEUSING_TERT_QUICK:只传送登录后私有流的内容
	///@remark 该方法要在Init方法前调用。若不调用则不会收到私有流的数据。
	virtual void SubscribePrivateTopic(ZEUSING_TE_RESUME_TYPE nResumeType) = 0;
	
	///订阅公共流。
	///@param nResumeType 公共流重传方式  
	///        ZEUSING_TERT_RESTART:从本交易日开始重传
	///        ZEUSING_TERT_RESUME:从上次收到的续传
	///        ZEUSING_TERT_QUICK:只传送登录后公共流的内容
	///@remark 该方法要在Init方法前调用。若不调用则不会收到公共流的数据。
	virtual void SubscribePublicTopic(ZEUSING_TE_RESUME_TYPE nResumeType) = 0;

  ///客户端认证请求
	virtual int ReqAuthenticate(CZeusingFtdcReqAuthenticateField *pReqAuthenticateField, int nRequestID) = 0;
	
	///用户登录请求
	virtual int ReqUserLogin(CZeusingFtdcReqUserLoginField *pReqUserLoginField, int nRequestID) = 0;
	

	///登出请求
	virtual int ReqUserLogout(CZeusingFtdcUserLogoutField *pUserLogout, int nRequestID) = 0;

	///用户口令更新请求
	virtual int ReqUserPasswordUpdate(CZeusingFtdcUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID) = 0;

	///资金账户口令更新请求
	virtual int ReqTradingAccountPasswordUpdate(CZeusingFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, int nRequestID) = 0;

	///报单录入请求
	virtual int ReqOrderInsert(CZeusingFtdcInputOrderField *pInputOrder, int nRequestID) = 0;

	///预埋单录入请求
	virtual int ReqParkedOrderInsert(CZeusingFtdcParkedOrderField *pParkedOrder, int nRequestID) = 0;

	///预埋撤单录入请求
	virtual int ReqParkedOrderAction(CZeusingFtdcParkedOrderActionField *pParkedOrderAction, int nRequestID) = 0;

	///报单操作请求
	virtual int ReqOrderAction(CZeusingFtdcInputOrderActionField *pInputOrderAction, int nRequestID) = 0;

	///查询最大报单数量请求
	virtual int ReqQueryMaxOrderVolume(CZeusingFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, int nRequestID) = 0;

	///投资者结算结果确认
	virtual int ReqSettlementInfoConfirm(CZeusingFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int nRequestID) = 0;

	///请求删除预埋单
	virtual int ReqRemoveParkedOrder(CZeusingFtdcRemoveParkedOrderField *pRemoveParkedOrder, int nRequestID) = 0;

	///请求删除预埋撤单
	virtual int ReqRemoveParkedOrderAction(CZeusingFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, int nRequestID) = 0;

	///执行宣告录入请求
	virtual int ReqExecOrderInsert(CZeusingFtdcInputExecOrderField *pInputExecOrder, int nRequestID) = 0;

	///执行宣告操作请求
	virtual int ReqExecOrderAction(CZeusingFtdcInputExecOrderActionField *pInputExecOrderAction, int nRequestID) = 0;

	///报价录入请求
	virtual int ReqQuoteInsert(CZeusingFtdcInputQuoteField *pInputQuote, int nRequestID) = 0;

	///报价操作请求
	virtual int ReqQuoteAction(CZeusingFtdcInputQuoteActionField *pInputQuoteAction, int nRequestID) = 0;

	///询价操作请求
	virtual int ReqForQuote(CZeusingFtdcInputForQuoteField *pInputForQuote, int nRequestID) = 0;

	///请求资金内转
	virtual int ReqInternalTransfer(CZeusingFtdcInternalTransferField *pInternalTransfer, int nRequestID) = 0;

	///请求查询报单
	virtual int ReqQryOrder(CZeusingFtdcQryOrderField *pQryOrder, int nRequestID) = 0;

	///请求查询成交
	virtual int ReqQryTrade(CZeusingFtdcQryTradeField *pQryTrade, int nRequestID) = 0;

	///请求查询投资者持仓
	virtual int ReqQryInvestorPosition(CZeusingFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID) = 0;

	///请求查询资金账户
	virtual int ReqQryTradingAccount(CZeusingFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID) = 0;

	///请求查询投资者
	virtual int ReqQryInvestor(CZeusingFtdcQryInvestorField *pQryInvestor, int nRequestID) = 0;

	///请求查询交易编码
	virtual int ReqQryTradingCode(CZeusingFtdcQryTradingCodeField *pQryTradingCode, int nRequestID) = 0;

	///请求查询合约保证金率
	virtual int ReqQryInstrumentMarginRate(CZeusingFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate, int nRequestID) = 0;

	///请求查询合约手续费率
	virtual int ReqQryInstrumentCommissionRate(CZeusingFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate, int nRequestID) = 0;

	///请求查询交易所
	virtual int ReqQryExchange(CZeusingFtdcQryExchangeField *pQryExchange, int nRequestID) = 0;

	///请求查询合约
	virtual int ReqQryInstrument(CZeusingFtdcQryInstrumentField *pQryInstrument, int nRequestID) = 0;

	///请求查询行情
	virtual int ReqQryDepthMarketData(CZeusingFtdcQryDepthMarketDataField *pQryDepthMarketData, int nRequestID) = 0;

	///请求查询投资者结算结果
	virtual int ReqQrySettlementInfo(CZeusingFtdcQrySettlementInfoField *pQrySettlementInfo, int nRequestID) = 0;

	///请求查询转帐银行
	virtual int ReqQryTransferBank(CZeusingFtdcQryTransferBankField *pQryTransferBank, int nRequestID) = 0;

	///请求查询投资者持仓明细
	virtual int ReqQryInvestorPositionDetail(CZeusingFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail, int nRequestID) = 0;

	///请求查询投资者持仓明细
	virtual int ReqQryInvestorPositionCombineDetail(CZeusingFtdcQryInvestorPositionCombineDetailField *pQryInvestorPositionCombineDetail, int nRequestID) = 0;

	///请求查询保证金监管系统经纪公司资金账户密钥
	virtual int ReqQryCFMMCTradingAccountKey(CZeusingFtdcQryCFMMCTradingAccountKeyField *pQryCFMMCTradingAccountKey, int nRequestID) = 0;

	///请求查询仓单折抵信息
	virtual int ReqQryEWarrantOffset(CZeusingFtdcQryEWarrantOffsetField *pQryEWarrantOffset, int nRequestID) = 0;

	///请求查询执行宣告
	virtual int ReqQryExecOrder(CZeusingFtdcQryExecOrderField *pQryExecOrder, int nRequestID) = 0;

	///请求查询执行宣告操作
	virtual int ReqQryExecOrderAction(CZeusingFtdcQryExecOrderActionField *pQryExecOrderAction, int nRequestID) = 0;

	///请求查询错误执行宣告
	virtual int ReqQryErrExecOrder(CZeusingFtdcQryErrExecOrderField *pQryErrExecOrder, int nRequestID) = 0;

	///请求查询错误执行宣告操作
	virtual int ReqQryErrExecOrderAction(CZeusingFtdcQryErrExecOrderActionField *pQryErrExecOrderAction, int nRequestID) = 0;

	///请求查询转帐流水
	virtual int ReqQryTransferSerial(CZeusingFtdcQryTransferSerialField *pQryTransferSerial, int nRequestID) = 0;

	///请求查询银期签约关系
	virtual int ReqQryAccountregister(CZeusingFtdcQryAccountregisterField *pQryAccountregister, int nRequestID) = 0;

	///请求查询签约银行
	virtual int ReqQryContractBank(CZeusingFtdcQryContractBankField *pQryContractBank, int nRequestID) = 0;

	///请求查询交易通知
	virtual int ReqQryTradingNotice(CZeusingFtdcQryTradingNoticeField *pQryTradingNotice, int nRequestID) = 0;

	///请求查询经纪公司交易参数
	virtual int ReqQryBrokerTradingParams(CZeusingFtdcQryBrokerTradingParamsField *pQryBrokerTradingParams, int nRequestID) = 0;

	///请求查询经纪公司交易算法
	virtual int ReqQryBrokerTradingAlgos(CZeusingFtdcQryBrokerTradingAlgosField *pQryBrokerTradingAlgos, int nRequestID) = 0;

	///期货发起银行资金转期货请求
	virtual int ReqFromBankToFutureByFuture(CZeusingFtdcReqTransferField *pReqTransfer, int nRequestID) = 0;

	///期货发起期货资金转银行请求
	virtual int ReqFromFutureToBankByFuture(CZeusingFtdcReqTransferField *pReqTransfer, int nRequestID) = 0;

	///期货发起查询银行余额请求
	virtual int ReqQueryBankAccountMoneyByFuture(CZeusingFtdcReqQueryAccountField *pReqQueryAccount, int nRequestID) = 0;
protected:
	~CZeusingFtdcTraderApi(){};
};

#ifndef WINDOWS
	#if __GNUC__ >= 4
		#pragma GCC visibility pop
	#endif
#endif
#endif
