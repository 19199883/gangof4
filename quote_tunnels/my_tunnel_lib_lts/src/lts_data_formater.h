#pragma once

#include <string>

#include "SecurityFtdcUserApiStruct.h"

// LTS结构的格式化信息接口，仅有静态函数，无对象实例
class LTSDatatypeFormater
{
public:
	// 所有请求消息的格式化
	static std::string ToString(const CSecurityFtdcReqUserLoginField *pdata);
	static std::string ToString(const CSecurityFtdcQryOrderField *pdata);
	static std::string ToString(const CSecurityFtdcQryInvestorPositionField *pdata);
	static std::string ToString(const CSecurityFtdcQryTradingAccountField *pdata);
	static std::string ToString(const CSecurityFtdcQryInvestorPositionDetailField *pdata);

	// 所有响应消息的格式化
	static std::string ToString(const CSecurityFtdcRspUserLoginField *pdata);
	static std::string ToString(const CSecurityFtdcUserLogoutField *pdata);
	static std::string ToString(const CSecurityFtdcUserPasswordUpdateField *pdata);
	static std::string ToString(const CSecurityFtdcInputOrderField *pdata);
	static std::string ToString(const CSecurityFtdcInputOrderActionField *pdata);
	static std::string ToString(const CSecurityFtdcOrderActionField *pdata);
	static std::string ToString(const CSecurityFtdcOrderField *pdata);
	static std::string ToString(const CSecurityFtdcTradeField *pdata);
	static std::string ToString(const CSecurityFtdcInvestorPositionField *pdata);
	static std::string ToString(const CSecurityFtdcRspInfoField *pdata);
	static std::string ToString(const CSecurityFtdcTradingAccountField *pdata);
	static std::string ToString(const CSecurityFtdcInvestorPositionDetailField *pdata);

	static std::string ToString(const CSecurityFtdcInstrumentField *pdata);

	static std::string ToString(const CSecurityFtdcAuthRandCodeField *pdata);

private:
	LTSDatatypeFormater()
	{
	}
	~LTSDatatypeFormater()
	{
	}
};
