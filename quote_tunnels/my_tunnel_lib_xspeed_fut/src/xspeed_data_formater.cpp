/*
 * xspeed_data_formater.cpp
 *
 *  Created on: 2013-8-13
 *      Author: oliver
 */

#include "xspeed_data_formater.h"
#include <iostream>

using namespace std;

std::string XSpeedDatatypeFormater::ToString(const DFITCUserLoginField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCUserLoginField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    passwd=%s\n"
            "    companyID=%d\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账户ID
            p->passwd,                       //密码
            p->companyID                     //厂商ID
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCUserLoginField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCInsertOrderField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCInsertOrderField\n"
            "    accountID=%s\n"
            "    localOrderID=%ld\n"
            "    instrumentID=%s\n"
            "    insertPrice=%.4f\n"
            "    orderAmount=%ld\n"
            "    buySellType=%d\n"
            "    openCloseType=%d\n"
            "    speculator=%d\n"
            "    insertType=%d\n"
            "    orderType=%d\n"
            "    orderProperty=%d\n"
            "    instrumentType=%d\n"
            "    lRequestID=%ld\n"
            "    customCategory=%s\n"
            "    reservedType2=%d\n"
            "    minMatchAmount=%ld\n"
            "    profitLossPrice=%.4f\n",
            p->accountID,                    //资金账户
            p->localOrderID,                 //本地委托号, 由API使用者维护，在同一个会话中不能重复
            p->instrumentID,                 //合约代码, 支持目前国内4个期货交易所的所有合约，包括大商所/郑商所的套利合约
            p->insertPrice,                  //报单价格, 当报单类型为市价时，该字段不起作用
            p->orderAmount,                  //报单数量
            p->buySellType,                  //买卖标志
            p->openCloseType,                //开平标志
            p->speculator,                   //投保类型, 支持投机、套利、套保
            p->insertType,                   //委托类别(默认为普通订单)
            p->orderType,                    //报单类型, 支持限价 、市价；上期所合约不支持市价，均按限价进行处理
            p->orderProperty,                //报单附加属性, 支持None、FAK、FOK，目前只有大商所合约支持该报单附加属性 FAK/FOK
            p->instrumentType,               //合约类型, 可选值：期货、期权
            p->lRequestID,                   //请求ID
            p->customCategory,               //自定义类别
            p->reservedType2,                //预留字段2
			p->minMatchAmount,               //上期FAK,FOK单的最小成交量
			p->profitLossPrice               //止盈止损价格
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCInsertOrderField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCCancelOrderField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCCancelOrderField\n"
            "    accountID=%s\n"
            "    spdOrderID=%ld\n"
            "    localOrderID=%ld\n"
            "    instrumentID=%s\n"
            "    lRequestID=%ld\n",
            p->accountID,                    //资金账户ID
            p->spdOrderID,                   //柜台委托号
            p->localOrderID,                 //本地委托号
            p->instrumentID,                 //合约代码
            p->lRequestID                    //请求ID
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCCancelOrderField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCPositionField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCPositionField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    instrumentID=%s\n"
            "    instrumentType=%d\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账户ID
            p->instrumentID,                 //合约代码
            p->instrumentType                //合约类型
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCPositionField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCPositionDetailField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCPositionDetailField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    instrumentID=%s\n"
            "    instrumentType=%d\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账户ID
            p->instrumentID,                 //合约代码
            p->instrumentType                //合约类型
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCPositionDetailField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCOrderField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    instrumentType=%d\n"
            "    customCategory=%s\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账户ID
            p->instrumentType,               //合约类型
            p->customCategory                //自定义类别
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCMatchField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCMatchField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    instrumentType=%d\n"
            "    customCategory=%s\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账户ID
            p->instrumentType,               //合约类型
            p->customCategory                //自定义类别
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCMatchField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCQuoteSubscribeField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCQuoteSubscribeField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    exchangeID=%s\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账号
            p->exchangeID);                   //交易所              

    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCQuoteSubscribeField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCQuoteUnSubscribeField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCQuoteUnSubscribeField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    exchangeID=%s\n"
        	"    instrumentType=%d\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账号
            p->exchangeID                   //交易所
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCQuoteUnSubscribeField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCQuoteInsertField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCQuoteInsertField\n"
            "    accountID=%s\n"
            "    lRequestID=%ld\n"
            "    localOrderID=%ld\n"
            "    insertType=%d\n"
            "    instrumentID=%s\n"
            "    quoteID=%s\n"
            "    instrumentType=%d\n"
            "    bOrderAmount=%ld\n"
            "    sOrderAmount=%ld\n"
            "    bInsertPrice=%.4f\n"
            "    sInsertPrice=%.4f\n"
            "    bOpenCloseType=%d\n"
            "    sOpenCloseType=%d\n"
            "    bSpeculator=%d\n"
            "    sSpeculator=%d\n"
            "    stayTime=%d\n",
            p->accountID,                    //资金账号
            p->lRequestID,                   //请求ID
            p->localOrderID,                 //本地委托号
            p->insertType,                   //委托类别
            p->instrumentID,                 //合约代码
            p->quoteID,                      //询价编号
            p->instrumentType,               //合约类型
            p->bOrderAmount,                 //报单数量（买）
            p->sOrderAmount,                 //报单数量（卖）
            p->bInsertPrice,                 //委托价格（买）
            p->sInsertPrice,                 //委托价格（卖）
            p->bOpenCloseType,               //开平标志（买）
            p->sOpenCloseType,               //开平标志（卖）
            p->bSpeculator,                  //投资类别（买）
            p->sSpeculator,                  //投资类别（卖）
            p->stayTime                     //停留时间，仅支持郑州。其它情况可设置为0
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCQuoteInsertField <null>");
    }

    return buf;
}


std::string XSpeedDatatypeFormater::ToString(const DFITCExchangeInstrumentField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCExchangeInstrumentField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    exchangeID=%s\n"
            "    instrumentType=%d\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账户ID
            p->exchangeID,                   //交易所编码
            p->instrumentType                //合约类型
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCExchangeInstrumentField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCErrorRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCErrorRtnField\n"
            "    requestID=%ld\n"
            "    sessionID=%ld\n"
            "    accountID=%s\n"
            "    nErrorID=%d\n"
            "    spdOrderID=%ld\n"
            "    localOrderID=%ld\n"
            "    errorMsg=%s\n",
            p->requestID,                    //请求ID
            p->sessionID,                    //会话标识
            p->accountID,                    //资金账号
            p->nErrorID,                     //错误ID
            p->spdOrderID,                   //柜台委托号
            p->localOrderID,                 //本地委托号
            p->errorMsg                      //错误信息
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCErrorRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCUserLoginInfoRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCUserLoginInfoRtnField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    loginResult=%d\n"
            "    initLocalOrderID=%ld\n"
            "    sessionID=%ld\n"
            "    nErrorID=%d\n"
            "    errorMsg=%s\n"
            "    DCEtime=%s\n"
            "    SHFETime=%s\n"
            "    CFFEXTime=%s\n"
            "    CZCETime=%s\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金帐号ID
            p->loginResult,                  //登录结果
            p->initLocalOrderID,             //初始本地委托号
            p->sessionID,                    //sessionID
            p->nErrorID,                     //错误ID
            p->errorMsg,                     //错误信息
            p->DCEtime,                      //大商所时间
            p->SHFETime,                     //上期所时间
            p->CFFEXTime,                    //中金所时间
            p->CZCETime                      //郑商所时间
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCUserLoginInfoRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCUserLogoutInfoRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCUserLogoutInfoRtnField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    logoutResult=%d\n"
            "    nErrorID=%d\n"
            "    errorMsg=%s\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账户ID
            p->logoutResult,                 //退出结果
            p->nErrorID,                     //错误ID
            p->errorMsg                      //错误信息
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCUserLogoutInfoRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCOrderRspDataRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderRspDataRtnField\n"
            "    localOrderID=%ld\n"
            "    spdOrderID=%ld\n"
            "    orderStatus=%d\n"
            "    lRequestID=%ld\n"
            "    fee=%.4f\n"
            "    margin=%.4f\n"
            "    customCategory=%s\n",
            p->localOrderID,                 //本地委托号
            p->spdOrderID,                   //柜台委托号
            p->orderStatus,                  //委托状态
            p->lRequestID,                   //请求ID
            p->fee,                          //手续费,该字段仅供下单时使用
            p->margin,                       //冻结保证金,该字段仅供下单时使用
            p->customCategory                //自定义类别
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderRspDataRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCOrderRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderRtnField\n"
            "    localOrderID=%ld\n"
            "    spdOrderID=%ld\n"
            "    OrderSysID=%s\n"
            "    orderStatus=%d\n"
            "    sessionID=%ld\n"
            "    SuspendTime=%s\n"
            "    instrumentID=%s\n"
            "    exchangeID=%s\n"
            "    buySellType=%d\n"
            "    OpenCloseType=%d\n"
            "    instrumentType=%d\n"
            "    speculator=%d\n"
            "    insertPrice=%.4f\n"
            "    accountID=%s\n"
            "    cancelAmount=%ld\n"
            "    orderAmount=%ld\n"
            "    insertType=%d\n"
            "    extspdOrderID=%ld\n"
            "    customCategory=%s\n"
            "    reservedType2=%d\n"
            "    profitLossPrice=%.4f\n",
            p->localOrderID,                 //本地委托号
            p->spdOrderID,                   //柜台委托号
            p->OrderSysID,                   //报单编号
            p->orderStatus,                  //委托状态
            p->sessionID,                    //会话ID
            p->SuspendTime,                  //挂起时间
            p->instrumentID,                 //合约代码
            p->exchangeID,                   //交易所
            p->buySellType,                  //买卖
            p->openCloseType,                //开平
            p->instrumentType,               //合约类型
            p->speculator,                   //投资类别
            p->insertPrice,                  //委托价
            p->accountID,                    //资金账号
            p->cancelAmount,                 //撤单数量
            p->orderAmount,                  //委托数量
            p->insertType,                   //委托类别
            p->extSpdOrderID,                //算法单编号
            p->customCategory,               //自定义类别
            p->reservedType2,                //预留字段2
			p->profitLossPrice               //止盈止损价格
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCMatchRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCMatchRtnField\n"
            "    localOrderID=%ld\n"
            "    OrderSysID=%s\n"
            "    matchID=%s\n"
            "    instrumentID=%s\n"
            "    buySellType=%d\n"
            "    OpenCloseType=%d\n"
            "    matchedPrice=%.4f\n"
            "    orderAmount=%ld\n"
            "    matchedAmount=%ld\n"
            "    matchedTime=%s\n"
            "    insertPrice=%.4f\n"
            "    spdOrderID=%ld\n"
            "    matchType=%ld\n"
            "    speculator=%d\n"
            "    exchangeID=%s\n"
            "    fee=%.4f\n"
            "    sessionID=%ld\n"
            "    instrumentType=%d\n"
            "    accountID=%s\n"
            "    orderStatus=%d\n"
            "    margin=%.4f\n"
            "    frozenCapita=%.4f\n"
            "    adjustmentInfo=%s\n"
            "    customCategory=%s\n"
            "    turnover=%.4f\n",
            p->localOrderID,                 //本地委托号
            p->OrderSysID,                   //报单编号(交易所报单编号)
            p->matchID,                      //成交编号
            p->instrumentID,                 //合约代码
            p->buySellType,                  //买卖
            p->openCloseType,                //开平标志
            p->matchedPrice,                 //成交价格
            p->orderAmount,                  //委托数量
            p->matchedAmount,                //成交数量
            p->matchedTime,                  //成交时间
            p->insertPrice,                  //报价
            p->spdOrderID,                   //柜台委托号
            p->matchType,                    //成交类型
            p->speculator,                   //投保
            p->exchangeID,                   //交易所ID
            p->fee,                          //手续费
            p->sessionID,                    //会话标识
            p->instrumentType,               //合约类型
            p->accountID,                    //资金账号
            p->orderStatus,                  //申报结果
            p->margin,                       //开仓为保证金,平仓为解冻保证金
            p->frozenCapita,                 //成交解冻委托冻结的资金
            p->adjustmentInfo,               //组合或对锁的保证金调整信息,格式:[合约代码,买卖标志,投资类别,调整金额,]
            p->customCategory,               //自定义类别
            p->turnover                      //成交金额
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCMatchRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCOrderCanceledRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderCanceledRtnField\n"
            "    localOrderID=%ld\n"
            "    OrderSysID=%s\n"
            "    instrumentID=%s\n"
            "    insertPrice=%.4f\n"
            "    buySellType=%d\n"
            "    OpenCloseType=%d\n"
            "    cancelAmount=%ld\n"
            "    spdOrderID=%ld\n"
            "    speculator=%d\n"
            "    exchangeID=%s\n"
            "    canceledTime=%s\n"
            "    sessionID=%ld\n"
            "    orderStatus=%d\n"
            "    instrumentType=%d\n"
            "    accountID=%s\n"
            "    orderAmount=%ld\n"
            "    margin=%.4f\n"
            "    fee=%.4f\n",
            p->localOrderID,                 //本地委托号
            p->OrderSysID,                   //报单编号
            p->instrumentID,                 //合约代码
            p->insertPrice,                  //报单价格
            p->buySellType,                  //买卖类型
            p->openCloseType,                //开平标志
            p->cancelAmount,                 //撤单数量
            p->spdOrderID,                   //柜台委托号
            p->speculator,                   //投保
            p->exchangeID,                   //交易所ID
            p->canceledTime,                 //撤单时间
            p->sessionID,                    //会话标识
            p->orderStatus,                  //申报结果
            p->instrumentType,               //合约类型
            p->accountID,                    //资金账号
            p->orderAmount,                  //委托数量
            p->margin,                       //保证金
            p->fee
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderCanceledRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCOrderCommRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderCommRtnField\n"
            "    lRequestID=%ld\n"
            "    spdOrderID=%ld\n"
            "    orderStatus=%d\n"
            "    instrumentID=%s\n"
            "    buySellType=%d\n"
            "    openClose=%d\n"
            "    insertPrice=%.4f\n"
            "    orderAmount=%ld\n"
            "    matchedPrice=%.4f\n"
            "    matchedAmount=%ld\n"
            "    cancelAmount=%ld\n"
            "    insertType=%d\n"
            "    speculator=%d\n"
            "    commTime=%s\n"
            "    submitTime=%s\n"
            "    clientID=%s\n"
            "    exchangeID=%s\n"
            "    operStation=%s\n"
            "    accountID=%s\n"
            "    instrumentType=%d\n"
            "    OrderSysID=%s\n"
            "    customCategory=%s\n"
            "    margin=%.4f\n"
            "    fee=%.4f\n"
            "    localOrderID=%ld\n"
            "    sessionId=%ld\n"
            "    reservedType2=%d\n"
            "    profitLossPrice=%.4f\n",
            p->lRequestID,                   //请求ID
            p->spdOrderID,                   //柜台委托号
            p->orderStatus,                  //委托状态
            p->instrumentID,                 //合约代码
            p->buySellType,                  //买卖
            p->openClose,                    //开平标志
            p->insertPrice,                  //委托价
            p->orderAmount,                  //委托数量
            p->matchedPrice,                 //成交价格
            p->matchedAmount,                //成交数量
            p->cancelAmount,                 //撤单数量
            p->insertType,                   //委托类别
            p->speculator,                   //投保
            p->commTime,                     //委托时间
            p->submitTime,                   //申报时间
            p->clientID,                     //交易编码
            p->exchangeID,                   //交易所ID
            p->operStation,                  //委托地址
            p->accountID,                    //客户号
            p->instrumentType,               //合约类型
            p->OrderSysID,                   //报单编号
            p->customCategory,               //自定义类别
            p->margin,                       //保证金
            p->fee,                          //手续费
            p->localOrderID,                 //本地委托号
            p->sessionId,                    //会话ID
            p->reservedType2,                //预留字段2
			p->profitLossPrice               //止盈止损价
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCOrderCommRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCMatchedRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCMatchedRtnField\n"
            "    lRequestID=%ld\n"
            "    spdOrderID=%ld\n"
            "    exchangeID=%s\n"
            "    instrumentID=%s\n"
            "    buySellType=%d\n"
            "    openClose=%d\n"
            "    matchedPrice=%.4f\n"
            "    matchedAmount=%ld\n"
            "    matchedMort=%.4f\n"
            "    speculator=%d\n"
            "    matchedTime=%s\n"
            "    matchedID=%s\n"
            "    localOrderID=%ld\n"
            "    clientID=%s\n"
            "    matchType=%ld\n"
            "    instrumentType=%d\n"
            "    customCategory=%s\n"
            "    fee=%.4f\n"
            "    reservedType1=%d\n"
            "    reservedType2=%d\n",
            p->lRequestID,                   //请求ID
            p->spdOrderID,                   //柜台委托号
            p->exchangeID,                   //交易所ID
            p->instrumentID,                 //合约代码
            p->buySellType,                  //买卖
            p->openClose,                    //开平
            p->matchedPrice,                 //成交价格
            p->matchedAmount,                //成交数量
            p->matchedMort,                  //成交金额
            p->speculator,                   //投保类别
            p->matchedTime,                  //成交时间
            p->matchedID,                    //成交编号
            p->localOrderID,                 //本地委托号
            p->clientID,                     //交易编码
            p->matchType,                    //成交类型
            p->instrumentType,               //合约类型
            p->customCategory,               //自定义类别
            p->fee                          //手续费
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCMatchedRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCPositionInfoRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCPositionInfoRtnField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    exchangeID=%s\n"
            "    instrumentID=%s\n"
            "    buySellType=%d\n"
            "    openAvgPrice=%.4f\n"
            "    positionAvgPrice=%.4f\n"
            "    positionAmount=%ld\n"
            "    totalAvaiAmount=%ld\n"
            "    todayAvaiAmount=%ld\n"
            "    lastAvaiAmount=%ld\n"
            "    todayAmount=%ld\n"
            "    lastAmount=%ld\n"
            "    tradingAmount=%ld\n"
            "    datePositionProfitLoss=%.4f\n"
            "    dateCloseProfitLoss=%.4f\n"
            "    dPremium=%.4f\n"
            "    floatProfitLoss=%.4f\n"
            "    dmargin=%.4f\n"
            "    speculator=%d\n"
            "    clientID=%s\n"
            "    preSettlementPrice=%.4f\n"
            "    instrumentType=%d\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金帐号ID
            p->exchangeID,                   //交易所代码
            p->instrumentID,                 //合约号
            p->buySellType,                  //买卖
            p->openAvgPrice,                 //开仓均价
            p->positionAvgPrice,             //持仓均价
            p->positionAmount,               //持仓量
            p->totalAvaiAmount,              //总可用
            p->todayAvaiAmount,              //今可用
            p->lastAvaiAmount,               //昨可用
            p->todayAmount,                  //今仓
            p->lastAmount,                   //昨仓
            p->tradingAmount,                //挂单量
            p->datePositionProfitLoss,       //盯市持仓盈亏
            p->dateCloseProfitLoss,          //盯市平仓盈亏
            p->dPremium,                     //权利金
            p->floatProfitLoss,              //浮动盈亏
            p->dMargin,                      //占用保证金
            p->speculator,                   //投保类别
            p->clientID,                     //交易编码
            p->preSettlementPrice,           //昨结算价
            p->instrumentType                //合约类型
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCPositionInfoRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCPositionDetailRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCPositionDetailRtnField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    exchangeID=%s\n"
            "    instrumentID=%s\n"
            "    buySellType=%d\n"
            "    openPrice=%.4f\n"
            "    volume=%ld\n"
            "    matchID=%s\n"
            "    matchedDate=%s\n"
            "    datePositionProfitLoss=%.4f\n"
            "    dateCloseProfitLoss=%.4f\n"
            "    floatProfitLoss=%.4f\n"
            "    dMargin=%.4f\n"
            "    speculator=%d\n"
            "    clientID=%s\n"
            "    preSettlementPrice=%.4f\n"
            "    instrumentType=%d\n"
            "    spdOrderID=%ld\n"
            "    customCategory=%s\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金帐号ID
            p->exchangeID,                   //交易所代码
            p->instrumentID,                 //合约号
            p->buySellType,                  //买卖
            p->openPrice,                    //开仓价
            p->volume,                       //手数
            p->matchID,                      //成交编号
            p->matchedDate,                  //成交日期
            p->datePositionProfitLoss,       //盯市持仓盈亏
            p->dateCloseProfitLoss,          //盯市平仓盈亏
            p->floatProfitLoss,              //浮动盈亏
            p->dMargin,                      //占用保证金
            p->speculator,                   //投保类别
            p->clientID,                     //交易编码
            p->preSettlementPrice,           //昨结算价
            p->instrumentType,               //合约类型
            p->spdOrderID,                   //柜台委托号
            p->customCategory                //自定义类别
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCPositionDetailRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCExchangeInstrumentRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCExchangeInstrumentRtnField\n"
            "    lRequestID=%ld\n"
            "    exchangeID=%s\n"
            "    instrumentID=%s\n"
            "    VarietyName=%s\n"
            "    instrumentType=%d\n"
            "    orderTopLimit=%ld\n"
            "    contractMultiplier=%.4f\n"
            "    minPriceFluctuation=%.4f\n"
            "    instrumentMaturity=%s\n"
            "    upperLimitPrice=%.4f\n"
            "    lowerLimitPrice=%.4f\n"
            "    preClosePrice=%.4f\n"
            "    preSettlementPrice=%.4f\n"
            "    settlementPrice=%.4f\n"
            "    preOpenInterest=%ld\n",
            p->lRequestID,                   //请求ID
            p->exchangeID,                   //交易所编码
            p->instrumentID,                 //合约代码
            p->VarietyName,                  //品种名称
            p->instrumentType,               //合约类型
            p->orderTopLimit,                //委托上限
            p->contractMultiplier,           //合约乘数
            p->minPriceFluctuation,          //最小变动价位
            p->instrumentMaturity,           //合约最后交易日
            p->upperLimitPrice,              //涨停板价
            p->lowerLimitPrice,              //跌停板价
            p->preClosePrice,                //昨收盘
            p->preSettlementPrice,           //昨结算价
            p->settlementPrice,              //结算价
            p->preOpenInterest               //昨持仓量
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCExchangeInstrumentRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCTradingDayRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCTradingDayRtnField\n"
            "    lRequestID=%ld\n"
            "    date=%s\n",
            p->lRequestID,                   //请求ID
            p->date                          //交易日
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCTradingDayRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCExchangeStatusRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCExchangeStatusRtnField\n"
            "    exchangeID=%s\n"
            "    exchangeStatus=%d\n",
            p->exchangeID,                   //交易所
            p->exchangeStatus                //交易所状态
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCExchangeStatusRtnField <null>");
    }

    return buf;
}

std::string XSpeedDatatypeFormater::ToString(const DFITCCapitalInfoRtnField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCCapitalInfoRtnField\n"
            "    requestID=%ld\n"
            "    accountID=%s\n"
            "    preEquity=%.4f\n"
            "    todayEquity=%.4f\n"
            "    closeProfitLoss=%.4f\n"
            "    positionProfitLoss=%.4f\n"
            "    frozenMargin=%.4f\n"
            "    margin=%.4f\n"
            "    fee=%.4f\n"
            "    available=%.4f\n"
            "    withdraw=%.4f\n"
            "    riskDegree=%.4f\n"
            "    todayPremiumIncome=%.4f\n"
            "    todayPremiumPay=%.4f\n"
            "    yesterdayPremium=%.4f\n"
            "    optMarketValue=%.4f\n"
            "    floatProfitLoss=%.4f\n"
            "    totFundOut=%.4f\n"
            "    totFundIn=%.4f\n",
            p->requestID,                    //请求ID
            p->accountID,                    //资金帐号
            p->preEquity,                    //上日权益
            p->todayEquity,                  //当日客户权益
            p->closeProfitLoss,              //平仓盈亏
            p->positionProfitLoss,           //持仓盈亏
            p->frozenMargin,                 //冻结资金
            p->margin,                       //持仓保证金
            p->fee,                          //当日手续费
            p->available,                    //可用资金
            p->withdraw,                     //可取资金
            p->riskDegree,                   //风险度
            p->todayPremiumIncome,           //本日权利金收入
            p->todayPremiumPay,              //本日权利金付出
            p->yesterdayPremium,             //昨权利金收付
            p->optMarketValue,               //期权市值
            p->floatProfitLoss,              //浮动盈亏
            p->totFundOut,                   //总出金
            p->totFundIn                    //总入金
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCCapitalInfoRtnField <null>");
    }

    return buf;
}
std::string XSpeedDatatypeFormater::ToString(const DFITCBillConfirmRspField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=DFITCBillConfirmRspField\n"
            "    lRequestID=%ld\n"
            "    accountID=%s\n"
            "    execState=%d\n",
            p->lRequestID,                   //请求ID
            p->accountID,                    //资金账号
            p->execState                     //状态标志
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=DFITCBillConfirmRspField <null>");
    }

    return buf;
}
