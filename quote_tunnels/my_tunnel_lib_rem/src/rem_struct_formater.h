/*
 * rem_struct_formater.h
 *
 *  Created on: 2015-7-15
 *      Author: chenyongshun
 */

#ifndef REM_STRUCT_FORMATER_H_
#define REM_STRUCT_FORMATER_H_

#include <string>

#include "EesTraderDefine.h"

// api结构的格式化信息接口
namespace RemStructFormater
{
    // request struct
    std::string ToString(const EES_EnterOrderField *p);
    std::string ToString(const EES_CancelOrder *p);

    // respond struct
    std::string ToString(const EES_LogonResponse *p);
    std::string ToString(const EES_AccountInfo *p);
    std::string ToString(const EES_AccountPosition *p);
    std::string ToString(const EES_AccountBP *p);
    std::string ToString(const EES_SymbolField *p);
    std::string ToString(const EES_AccountMargin *p);
    std::string ToString(const EES_AccountFee *p);

    std::string ToString(const EES_OrderAcceptField *p);
    std::string ToString(const EES_OrderMarketAcceptField *p);
    std::string ToString(const EES_OrderRejectField *p);
    std::string ToString(const EES_OrderMarketRejectField *p);
    std::string ToString(const EES_OrderExecutionField *p);
    std::string ToString(const EES_OrderCxled *p);
    std::string ToString(const EES_CxlOrderRej *p);

    std::string ToString(const EES_QueryAccountOrder *p);
    std::string ToString(const EES_QueryOrderExecution *p);

    std::string ToString(const EES_PostOrder *p);
    std::string ToString(const EES_PostOrderExecution *p);
    std::string ToString(const EES_SymbolStatus *p);

	std::string ToString(const EES_ExchangeMarketSession *p);
}

#endif
