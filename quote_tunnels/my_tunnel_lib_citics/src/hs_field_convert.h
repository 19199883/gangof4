#ifndef FIELD_CONVERT_H_
#define FIELD_CONVERT_H_

#include "trade_data_type.h"
#include <string>
#include <iomanip>
#include <string.h>
using namespace std;

namespace HSFieldConvert
{
	//entrust_status  委托状态

	//'1'-买入 '2'-卖出
	const string  kHSBS_Buy = "1";
	const string  kHSBS_Sell = "2";
	inline const string ConvertUftBS(char dir)
	{
		if (dir == FUNC_CONST_DEF::CONST_ENTRUST_BUY){
			return kHSBS_Buy;
		}else{
			return kHSBS_Sell;
		}
	}

	inline const char ConvertDirFromHS(char dir)
	{
		if (dir == kHSBS_Buy[0])
		{
			return FUNC_CONST_DEF::CONST_ENTRUST_BUY;
		}
		else
		{
			return FUNC_CONST_DEF::CONST_ENTRUST_SELL;
		}
	}

	inline char ConvertFromUtfExchange(string myExchange)
	{
		char hsexchange;

		if("F3"==myExchange){// SHANGHAI FUTURES EXCHANGE，上海期货交易所
			hsexchange = 'A';
		}else if("F4"==myExchange){// CHINA FINANCIAL FUTURES EXCHANGE,中国金融期货交易所
			hsexchange = 'G';
		}else if("F2"==myExchange){// DALIAN COMMODITY EXCHANGE，大连商品交易所
			hsexchange = 'B';
		}else if("F1"==myExchange){// ZHENGZHOU COMMODITY EXCHANGE， 郑州商品交易所
			hsexchange = 'C';
		}else if("2"==myExchange){	// SHENZHEN STOCK EXCHANGE，深圳股票交易所
			hsexchange = '0';
		}else if("1"==myExchange){	// SHANGHAI STOCK EXCHANGE，上海股票交易所
			hsexchange = '1';
		}else{
			hsexchange = '!';
		}

		return hsexchange;
	}

	inline string Convert2UtfExchange(char myExchange)
	{
		string hsexchange;

		switch(myExchange){
		case 'A':	// SHANGHAI FUTURES EXCHANGE，上海期货交易所
			hsexchange = "F3";
			break;
		case 'G':	// CHINA FINANCIAL FUTURES EXCHANGE,中国金融期货交易所
			hsexchange = "F4";
			break;
		case 'B':	// DALIAN COMMODITY EXCHANGE，大连商品交易所
			hsexchange = "F2";
			break;
		case 'C':	// ZHENGZHOU COMMODITY EXCHANGE， 郑州商品交易所
			hsexchange = "F1";
			break;
		case '0':	// SHENZHEN STOCK EXCHANGE，深圳股票交易所
			hsexchange = "2";
			break;
		case '1':	// SHANGHAI STOCK EXCHANGE，上海股票交易所
			hsexchange = "1";
			break;
		default:
			hsexchange = "0";
			break;
		}

		return hsexchange;
	}

	// 'O'-开仓 'C'-平仓 'X'-行权
	const char kHSOC_Open[] = "O";
	const char kHSOC_Close[] = "C";
	const char kHSOC_Execute[] = "X";
	inline const char *ConvertOC(char oc)
	{
		if (oc == FUNC_CONST_DEF::CONST_ENTRUST_OPEN)
		{
			return kHSOC_Open;
		}
		else
		{
			return kHSOC_Close;
		}
	}

	const char kHsEntrustStatus_Filled = '0';		//	成交
	const char kHsEntrustStatus_Discarded = '2';	//	废单
	const char kHsEntrustStatus_Reported = '4';		//	确认
	inline char RealtStatusTransForPlace(char utfStatus,long acc_amount,long entrust_amount)
	{
		switch (utfStatus){
			case kHsEntrustStatus_Reported:
				return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_REPORDED;
			case kHsEntrustStatus_Filled:
				if(acc_amount==entrust_amount){
					return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_COMPLETED;
				}else{
					return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_PARTIALCOM;
				}
			case kHsEntrustStatus_Discarded:
				return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_DISABLE;

			default:
				return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR;
		}

		//错误委托
		return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR;
	}

	inline char RealtStatusTransForCancel(char utfStatus,long acc_amount,long entrust_amount)
	{
		switch (utfStatus){
			case kHsEntrustStatus_Reported:
				return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_WITHDRAWING;
			case kHsEntrustStatus_Filled:
				return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_WITHDRAWED;
			case kHsEntrustStatus_Discarded:
				return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_DISABLE;

			default:
				return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR;
		}

		//错误委托
		return FUNC_CONST_DEF::CONST_ENTRUST_STATUS_ERROR;
	}

	//	委托
	const char REAL_TYPE_PLACE = '0';
	//
	const char REAL_TYPE_QUERY = '1';
	//	撤单
	const char REAL_TYPE_CANCEL = '2';
}
#endif
