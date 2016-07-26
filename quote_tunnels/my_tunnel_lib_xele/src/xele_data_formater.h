/*
 * xele_data_formater.h
 *
 *  Created on: 2015-6-10
 *      Author: oliver
 */

#ifndef XELE_DATATYPEFORMATER_H_
#define XELE_DATATYPEFORMATER_H_

#include <string>

#include "CXeleFtdcUserApiDataType.h"
#include "CXeleFtdcUserApiStruct.h"

// Xele结构的格式化信息接口，仅有静态函数，无对象实例
class XeleDatatypeFormater
{
public:
	// 所有请求消息的格式化
    static std::string ToString(const CXeleFtdcReqUserLoginField *p);
	static std::string ToString(const CXeleFtdcReqUserLogoutField *p);
	//static std::string ToString(const CXeleFtdcQryPartAccountField *p);
	static std::string ToString(const CXeleFtdcQryOrderField *p);
	static std::string ToString(const CXeleFtdcQryTradeField *p);
	//static std::string ToString(const CXeleFtdcQryPartPositionField *p);
	static std::string ToString(const CXeleFtdcQryClientPositionField *p);
	static std::string ToString(const CXeleFtdcQryInstrumentField *p);

	static std::string ToString(const CXeleFtdcInputOrderField *p);
	static std::string ToString(const CXeleFtdcOrderActionField *p);

	// 所有响应消息的格式化
	static std::string ToString(const CXeleFtdcRspInfoField *p);
	static std::string ToString(const CXeleFtdcRspUserLoginField *p);
	static std::string ToString(const CXeleFtdcRspUserLogoutField *p);
	//static std::string ToString(const CXeleFtdcRspPartAccountField *p);
	//static std::string ToString(const CXeleFtdcRspPartPositionField *p);
	static std::string ToString(const CXeleFtdcRspClientPositionField *p);
	static std::string ToString(const CXeleFtdcRspInstrumentField *p);
	//static std::string ToString(const CXeleFtdcInputOrderField *p);  // same as request
	//static std::string ToString(const CXeleFtdcOrderActionField *p); // same as request
	static std::string ToString(const CXeleFtdcOrderField *p);
	static std::string ToString(const CXeleFtdcTradeField *p);
	//static std::string ToString(const CXeleFtdcInputOrderField *p);  // same as request
	//static std::string ToString(const CXeleFtdcOrderActionField *p); // same as request
	static std::string ToString(const CXeleFtdcInstrumentStatusField *p);

private:
	XeleDatatypeFormater()
	{
	}
	~XeleDatatypeFormater()
	{
	}
};

#endif /* Xele_DATATYPEFORMATER_H_ */
