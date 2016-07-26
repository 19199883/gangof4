#pragma once

#include <string>

#include "SgitFtdcUserApiDataType.h"
#include "SgitFtdcUserApiStruct.h"

// api结构的格式化信息接口，仅有静态函数，无对象实例
class DatatypeFormater
{
public:
	// 所有请求消息的格式化
	static std::string ToString(const CSgitFtdcReqUserLoginField *pdata);
	static std::string ToString(const CSgitFtdcQryOrderField *pdata);
	static std::string ToString(const CSgitFtdcQryInvestorPositionField *pdata);
	static std::string ToString(const CSgitFtdcQryTradingAccountField *pdata);
	static std::string ToString(const CSgitFtdcQryInvestorPositionDetailField *pdata);

	// 所有响应消息的格式化
	static std::string ToString(const CSgitFtdcRspUserLoginField *pdata);
	static std::string ToString(const CSgitFtdcUserLogoutField *pdata);
	static std::string ToString(const CSgitFtdcUserPasswordUpdateField *pdata);
	static std::string ToString(const CSgitFtdcInputOrderField *pdata);
	static std::string ToString(const CSgitFtdcInputOrderActionField *pdata);
	static std::string ToString(const CSgitFtdcOrderActionField *pdata);
	static std::string ToString(const CSgitFtdcOrderField *pdata);
	static std::string ToString(const CSgitFtdcTradeField *pdata);
	static std::string ToString(const CSgitFtdcSettlementInfoConfirmField *pdata);
	static std::string ToString(const CSgitFtdcInvestorPositionField *pdata);
	static std::string ToString(const CSgitFtdcRspInfoField *pdata);
	static std::string ToString(const CSgitFtdcTradingAccountField *pdata);
	static std::string ToString(const CSgitFtdcSettlementInfoField *pdata);
	static std::string ToString(const CSgitFtdcInvestorPositionDetailField *pdata);

	static std::string ToString(const CSgitFtdcInstrumentField *pdata);

private:
	DatatypeFormater()
	{
	}
	~DatatypeFormater()
	{
	}
};
