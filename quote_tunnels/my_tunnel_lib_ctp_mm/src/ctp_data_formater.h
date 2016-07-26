﻿/*
 * ctp_data_formater.h
 *
 *  Created on: 2013-8-13
 *      Author: oliver
 */

#ifndef CTP_DATATYPEFORMATER_H_
#define CTP_DATATYPEFORMATER_H_

#include <string>

#include "ThostFtdcUserApiDataType.h"
#include "ThostFtdcUserApiStruct.h"

// CTP结构的格式化信息接口，仅有静态函数，无对象实例
class CTPDatatypeFormater
{
public:
	// 所有请求消息的格式化
	static std::string ToString(const CThostFtdcReqUserLoginField *pdata);
	static std::string ToString(const CThostFtdcQryOrderField *pdata);
	static std::string ToString(const CThostFtdcQryInvestorPositionField *pdata);
	static std::string ToString(const CThostFtdcQryTradingAccountField *pdata);

	static std::string ToString(const CThostFtdcQryInvestorPositionDetailField *pdata);

	// 所有响应消息的格式化
	static std::string ToString(const CThostFtdcRspUserLoginField *pdata);
	static std::string ToString(const CThostFtdcUserLogoutField *pdata);
	static std::string ToString(const CThostFtdcUserPasswordUpdateField *pdata);
	static std::string ToString(const CThostFtdcInputOrderField *pdata);
	static std::string ToString(const CThostFtdcInputOrderActionField *pdata);
	static std::string ToString(const CThostFtdcOrderActionField *pdata);
	static std::string ToString(const CThostFtdcOrderField *pdata);
	static std::string ToString(const CThostFtdcTradeField *pdata);
	static std::string ToString(const CThostFtdcSettlementInfoConfirmField *pdata);
	static std::string ToString(const CThostFtdcInvestorPositionField *pdata);
	static std::string ToString(const CThostFtdcRspInfoField *pdata);
	static std::string ToString(const CThostFtdcTradingAccountField *pdata);

	static std::string ToString(const CThostFtdcSettlementInfoField *pdata);

	// added on 20141217, for mm interface
	static std::string ToString(const CThostFtdcInputForQuoteField *pdata);
	static std::string ToString(const CThostFtdcInputQuoteField *pdata);
	static std::string ToString(const CThostFtdcInputQuoteActionField *pdata);
	static std::string ToString(const CThostFtdcQryQuoteField *pdata);
	static std::string ToString(const CThostFtdcQuoteField *pdata);
	static std::string ToString(const CThostFtdcQuoteActionField *pdata);
	static std::string ToString(const CThostFtdcForQuoteRspField *pdata);

	static std::string ToString(const CThostFtdcInvestorPositionDetailField *pdata);

	static std::string ToString(const CThostFtdcInstrumentField *pdata);

private:
	CTPDatatypeFormater()
	{
	}
	~CTPDatatypeFormater()
	{
	}
};

#endif /* CTP_DATATYPEFORMATER_H_ */
