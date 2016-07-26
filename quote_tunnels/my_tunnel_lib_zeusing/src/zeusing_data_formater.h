#pragma once

#include <string>

#include "ZeusingFtdcUserApiDataType.h"
#include "ZeusingFtdcUserApiStruct.h"

// api结构的格式化信息接口，仅有静态函数，无对象实例
class DataStructFormater
{
public:
	// 所有请求消息的格式化
	static std::string ToString(const CZeusingFtdcReqUserLoginField *pdata);
	static std::string ToString(const CZeusingFtdcQryOrderField *pdata);
	static std::string ToString(const CZeusingFtdcQryInvestorPositionField *pdata);
	static std::string ToString(const CZeusingFtdcQryTradingAccountField *pdata);
	static std::string ToString(const CZeusingFtdcQryInvestorPositionDetailField *pdata);

	// 所有响应消息的格式化
	static std::string ToString(const CZeusingFtdcRspUserLoginField *pdata);
	static std::string ToString(const CZeusingFtdcUserLogoutField *pdata);
	static std::string ToString(const CZeusingFtdcUserPasswordUpdateField *pdata);
	static std::string ToString(const CZeusingFtdcInputOrderField *pdata);
	static std::string ToString(const CZeusingFtdcInputOrderActionField *pdata);
	static std::string ToString(const CZeusingFtdcOrderActionField *pdata);
	static std::string ToString(const CZeusingFtdcOrderField *pdata);
	static std::string ToString(const CZeusingFtdcTradeField *pdata);
	static std::string ToString(const CZeusingFtdcSettlementInfoConfirmField *pdata);
	static std::string ToString(const CZeusingFtdcInvestorPositionField *pdata);
	static std::string ToString(const CZeusingFtdcRspInfoField *pdata);
	static std::string ToString(const CZeusingFtdcTradingAccountField *pdata);
	static std::string ToString(const CZeusingFtdcSettlementInfoField *pdata);
	static std::string ToString(const CZeusingFtdcInvestorPositionDetailField *pdata);

	static std::string ToString(const CZeusingFtdcInstrumentField *pdata);

private:
	DataStructFormater()
	{
	}
	~DataStructFormater()
	{
	}
};
