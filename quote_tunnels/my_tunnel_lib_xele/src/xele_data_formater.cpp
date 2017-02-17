/*
 * xele_data_formater.cpp
 *
 *  Created on: 2015-6-10
 *      Author: oliver
 */

#include "xele_data_formater.h"
#include <iostream>

using namespace std;

static double CheckAndInvalidToZero(double v)
{
    if (v > -100000000 && v < 100000000) return v;

    return 0;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcReqUserLoginField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcReqUserLoginField\n"
            "\tTradingDay=%s\n"
            "\tUserID=%s\n"
            "\tParticipantID=%s\n"
            "\tPassword=%s\n"
            "\tUserProductInfo=%s\n"
            "\tInterfaceProductInfo=%s\n"
            "\tProtocolInfo=%s\n"
            "\tDataCenterID=%d\n",
            p->TradingDay,                          ///交易日
            p->UserID,                              ///交易用户代码
            p->ParticipantID,                       ///会员代码
            p->Password,                            ///密码
            p->UserProductInfo,                     ///用户端产品信息
            p->InterfaceProductInfo,                ///接口端产品信息
            p->ProtocolInfo,                        ///协议信息
            p->DataCenterID                         ///数据中心代码
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcReqUserLoginField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcReqUserLogoutField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcReqUserLogoutField\n"
            "\tUserID=%s\n"
            "\tParticipantID=%s\n",
            p->UserID,                              ///交易用户代码
            p->ParticipantID                        ///会员代码
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcReqUserLogoutField <null>");
    }

    return buf;
}

//std::string XeleDatatypeFormater::ToString(const CXeleFtdcQryPartAccountField *p)
//{
//    char buf[2048];
//
//    if (p)
//    {
//        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryPartAccountField\n"
//            "\tPartIDStart=%s\n"
//            "\tPartIDEnd=%s\n"
//            "\tAccountID=%s\n",
//            p->PartIDStart,                         ///起始会员代码
//            p->PartIDEnd,                           ///结束会员代码
//            p->AccountID                            ///资金帐号
//            );
//    }
//    else
//    {
//        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryPartAccountField <null>");
//    }
//
//    return buf;
//}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcQryOrderField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryOrderField\n"
            "\tClientID=%s\n"
            "\tOrderSysID=%s\n"
            "\tInstrumentID=%s\n",
            p->ClientID,                              ///交易用户代码
            p->OrderSysID,                          ///报单编号
            p->InstrumentID                         ///合约代码
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryOrderField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcQryTradeField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryTradeField\n"
            "\tClientID=%s\n"
            "\tInstrumentID=%s\n"
            "\tTradeID=%s\n"
            "\tTimeStart=%s\n"
            "\tTimeEnd=%s\n",
            p->ClientID,                              ///交易用户代码
            p->InstrumentID,                        ///合约代码
            p->TradeID,                             ///成交编号
            p->TimeStart,                           ///开始时间
            p->TimeEnd                              ///结束时间
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryTradeField <null>");
    }

    return buf;
}

//std::string XeleDatatypeFormater::ToString(const CXeleFtdcQryPartPositionField *p)
//{
//    char buf[2048];
//
//    if (p)
//    {
//        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryPartPositionField\n"
//            "\tPartIDStart=%s\n"
//            "\tPartIDEnd=%s\n"
//            "\tInstIDStart=%s\n"
//            "\tInstIDEnd=%s\n",
//            p->PartIDStart,                         ///起始会员代码
//            p->PartIDEnd,                           ///结束会员代码
//            p->InstIDStart,                         ///起始合约代码
//            p->InstIDEnd                            ///结束合约代码
//            );
//    }
//    else
//    {
//        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryPartPositionField <null>");
//    }
//
//    return buf;
//}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcQryClientPositionField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryClientPositionField\n"
            "\tClientID=%s\n"
            "\tInstrumentID=%s\n"
            "\tClientType=0X%02X\n",
            p->ClientID,                            ///客户代码
            p->InstrumentID,                        ///合约代码
            p->ClientType                           ///客户类型
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryClientPositionField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcQryInstrumentField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryInstrumentField\n"
            "\tProductID=%s\n"
            "\tInstrumentID=%s\n",
            p->ProductID,                           ///产品代码
            p->InstrumentID                         ///合约代码
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcQryInstrumentField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcInputOrderField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcInputOrderField\n"
            "\tOrderSysID=%s\n"
            "\tParticipantID=%s\n"
            "\tClientID=%s\n"
            "\tUserID=%s\n"
            "\tInstrumentID=%s\n"
            "\tOrderPriceType=0X%02X\n"
            "\tDirection=%c\n"
            "\tCombOffsetFlag=%s\n"
            "\tCombHedgeFlag=%s\n"
            "\tLimitPrice=%.6f\n"
            "\tVolumeTotalOriginal=%d\n"
            "\tTimeCondition=0X%02X\n"
            "\tGTDDate=%s\n"
            "\tVolumeCondition=0X%02X\n"
            "\tMinVolume=%d\n"
            "\tContingentCondition=0X%02X\n"
            "\tStopPrice=%.6f\n"
            "\tForceCloseReason=0X%02X\n"
            "\tOrderLocalID=%s\n"
            "\tIsAutoSuspend=%d\n",
            p->OrderSysID,                          ///报单编号
            p->ParticipantID,                       ///会员代码
            p->ClientID,                            ///客户代码
            p->UserID,                              ///交易用户代码
            p->InstrumentID,                        ///合约代码
            p->OrderPriceType,                      ///报单价格条件
            p->Direction,                           ///买卖方向
            p->CombOffsetFlag,                      ///组合开平标志
            p->CombHedgeFlag,                       ///组合投机套保标志
            p->LimitPrice,                          ///价格
            p->VolumeTotalOriginal,                 ///数量
            p->TimeCondition,                       ///有效期类型
            p->GTDDate,                             ///GTD日期
            p->VolumeCondition,                     ///成交量类型
            p->MinVolume,                           ///最小成交量
            p->ContingentCondition,                 ///触发条件
            p->StopPrice,                           ///止损价
            p->ForceCloseReason,                    ///强平原因
            p->OrderLocalID,                        ///本地报单编号
            p->IsAutoSuspend                        ///自动挂起标志
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcInputOrderField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcOrderActionField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcOrderActionField\n"
            "\tOrderSysID=%s\n"
            "\tOrderLocalID=%s\n"
            "\tActionFlag=0X%02X\n"
            "\tParticipantID=%s\n"
            "\tClientID=%s\n"
            "\tUserID=%s\n"
            "\tLimitPrice=%.6f\n"
            "\tVolumeChange=%d\n"
            "\tActionLocalID=%s\n"
            "\tBusinessUnit=%s\n",
            p->OrderSysID,                          ///报单编号
            p->OrderLocalID,                        ///本地报单编号
            p->ActionFlag,                          ///报单操作标志
            p->ParticipantID,                       ///会员代码
            p->ClientID,                            ///客户代码
            p->UserID,                              ///交易用户代码
            p->LimitPrice,                          ///价格
            p->VolumeChange,                        ///数量变化
            p->ActionLocalID,                       ///操作本地编号
            p->BusinessUnit                         ///业务单元
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcOrderActionField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcRspInfoField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspInfoField\n"
            "\tErrorID=%d\n"
            "\tErrorMsg=%s\n",
            p->ErrorID,                             ///错误代码
            p->ErrorMsg                             ///错误信息
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspInfoField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcRspUserLoginField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspUserLoginField\n"
            "\tTradingDay=%s\n"
            "\tLoginTime=%s\n"
            "\tMaxOrderLocalID=%s\n"
            "\tUserID=%s\n"
            "\tParticipantID=%s\n"
            "\tTradingSystemName=%s\n"
            "\tDataCenterID=%d\n"
            "\tPrivateFlowSize=%d\n"
            "\tUserFlowSize=%d\n",
            p->TradingDay,                          ///交易日
            p->LoginTime,                           ///登录成功时间
            p->MaxOrderLocalID,                     ///最大本地报单号
            p->UserID,                              ///交易用户代码
            p->ParticipantID,                       ///会员代码
            p->TradingSystemName,                   ///交易系统名称
            p->DataCenterID,                        ///数据中心代码
            p->PrivateFlowSize,                     ///会员私有流当前长度
            p->UserFlowSize                         ///交易员私有流当前长度
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspUserLoginField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcRspUserLogoutField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspUserLogoutField\n"
            "\tUserID=%s\n"
            "\tParticipantID=%s\n",
            p->UserID,                              ///交易用户代码
            p->ParticipantID                        ///会员代码
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspUserLogoutField <null>");
    }

    return buf;
}

//std::string XeleDatatypeFormater::ToString(const CXeleFtdcRspPartAccountField *p)
//{
//    char buf[2048];
//
//    if (p)
//    {
//        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspPartAccountField\n"
//            "\tTradingDay=%s\n"
//            "\tSettlementGroupID=%s\n"
//            "\tSettlementID=%d\n"
//            "\tPreBalance=%.6f\n"
//            "\tCurrMargin=%.6f\n"
//            "\tCloseProfit=%.6f\n"
//            "\tPremium=%.6f\n"
//            "\tDeposit=%.6f\n"
//            "\tWithdraw=%.6f\n"
//            "\tBalance=%.6f\n"
//            "\tAvailable=%.6f\n"
//            "\tAccountID=%s\n"
//            "\tFrozenMargin=%.6f\n"
//            "\tFrozenPremium=%.6f\n"
//            "\tBaseReserve=%.6f\n",
//            p->TradingDay,                          ///交易日
//            p->SettlementGroupID,                   ///结算组代码
//            p->SettlementID,                        ///结算编号
//            p->PreBalance,                          ///上次结算准备金
//            p->CurrMargin,                          ///当前保证金总额
//            p->CloseProfit,                         ///平仓盈亏
//            p->Premium,                             ///期权权利金收支
//            p->Deposit,                             ///入金金额
//            p->Withdraw,                            ///出金金额
//            p->Balance,                             ///期货结算准备金
//            p->Available,                           ///可提资金
//            p->AccountID,                           ///资金帐号
//            p->FrozenMargin,                        ///冻结的保证金
//            p->FrozenPremium,                       ///冻结的权利金
//            p->BaseReserve                          ///基本准备金
//            );
//    }
//    else
//    {
//        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspPartAccountField <null>");
//    }
//
//    return buf;
//}

//std::string XeleDatatypeFormater::ToString(const CXeleFtdcRspPartPositionField *p)
//{
//    char buf[2048];
//
//    if (p)
//    {
//        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspPartPositionField\n"
//            "\tTradingDay:%s\n"
//            "\tSettlementGroupID:%s\n"
//            "\tSettlementID:%d\n"
//            "\tHedgeFlag:%c\n"
//            "\tPosiDirection:%c\n"
//            "\tYdPosition:%d\n"
//            "\tPosition:%d\n"
//            "\tLongFrozen:%d\n"
//            "\tShortFrozen:%d\n"
//            "\tYdLongFrozen:%d\n"
//            "\tYdShortFrozen:%d\n"
//            "\tInstrumentID:%s\n"
//            "\tParticipantID:%s\n"
//            "\tTradingRole:0X%02X\n",
//            p->TradingDay,
//            p->SettlementGroupID,
//            p->SettlementID,
//            p->HedgeFlag,
//            p->PosiDirection,
//            p->YdPosition,
//            p->Position,
//            p->LongFrozen,
//            p->ShortFrozen,
//            p->YdLongFrozen,
//            p->YdShortFrozen,
//            p->InstrumentID,
//            p->ParticipantID,
//            p->TradingRole
//            );
//    }
//    else
//    {
//        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspPartPositionField <null>");
//    }
//
//    return buf;
//}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcRspClientPositionField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspClientPositionField\n"
            "\tTradingDay=%s\n"
            "\tSettlementGroupID=%s\n"
            "\tSettlementID=%d\n"
            "\tHedgeFlag=%c\n"
            "\tPosiDirection=%c\n"
            "\tYdPosition=%d\n"
            "\tPosition=%d\n"
            "\tLongFrozen=%d\n"
            "\tShortFrozen=%d\n"
            "\tYdLongFrozen=%d\n"
            "\tYdShortFrozen=%d\n"
            "\tBuyTradeVolume=%d\n"
            "\tSellTradeVolume=%d\n"
            "\tPositionCost=%.6f\n"
            "\tYdPositionCost=%.6f\n"
            "\tUseMargin=%.6f\n"
            "\tFrozenMargin=%.6f\n"
            "\tLongFrozenMargin=%.6f\n"
            "\tShortFrozenMargin=%.6f\n"
            "\tFrozenPremium=%.6f\n"
            "\tInstrumentID=%s\n"
            "\tClientID=%s\n",
            p->TradingDay,                          ///交易日
            p->SettlementGroupID,                   ///结算组代码
            p->SettlementID,                        ///结算编号
            p->HedgeFlag,                           ///投机套保标志
            p->PosiDirection,                       ///持仓多空方向
            p->YdPosition,                          ///上日持仓
            p->Position,                            ///今日持仓
            p->LongFrozen,                          ///多头冻结
            p->ShortFrozen,                         ///空头冻结
            p->YdLongFrozen,                        ///昨日多头冻结
            p->YdShortFrozen,                       ///昨日空头冻结
            p->BuyTradeVolume,                      ///当日买成交量
            p->SellTradeVolume,                     ///当日卖成交量
            p->PositionCost,                        ///持仓成本
            p->YdPositionCost,                      ///昨日持仓成本
            CheckAndInvalidToZero(p->UseMargin),    ///占用的保证金
            p->FrozenMargin,                        ///冻结的保证金
            p->LongFrozenMargin,                    ///多头冻结的保证金
            p->ShortFrozenMargin,                   ///空头冻结的保证金
            p->FrozenPremium,                       ///冻结的权利金
            p->InstrumentID,                        ///合约代码
            p->ClientID                             ///客户代码
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspClientPositionField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcRspInstrumentField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspInstrumentField\n"
            "\tSettlementGroupID=%s\n"
            "\tProductID=%s\n"
            "\tProductGroupID=%s\n"
            "\tUnderlyingInstrID=%s\n"
            "\tProductClass=0X%02X\n"
            "\tPositionType=0X%02X\n"
            "\tStrikePrice=%.6f\n"
            "\tOptionsType=0X%02X\n"
            "\tVolumeMultiple=%d\n"
            "\tUnderlyingMultiple=%.6f\n"
            "\tInstrumentID=%s\n"
            "\tInstrumentName=%s\n"
            "\tDeliveryYear=%d\n"
            "\tDeliveryMonth=%02d\n"
            "\tAdvanceMonth=%s\n"
            "\tIsTrading=%d\n"
            "\tCreateDate=%s\n"
            "\tOpenDate=%s\n"
            "\tExpireDate=%s\n"
            "\tStartDelivDate=%s\n"
            "\tEndDelivDate=%s\n"
            "\tBasisPrice=%.6f\n"
            "\tMaxMarketOrderVolume=%d\n"
            "\tMinMarketOrderVolume=%d\n"
            "\tMaxLimitOrderVolume=%d\n"
            "\tMinLimitOrderVolume=%d\n"
            "\tPriceTick=%.6f\n"
            "\tAllowDelivPersonOpen=%d\n",
            p->SettlementGroupID,                   ///结算组代码
            p->ProductID,                           ///产品代码
            p->ProductGroupID,                      ///产品组代码
            p->UnderlyingInstrID,                   ///基础商品代码
            p->ProductClass,                        ///产品类型
            p->PositionType,                        ///持仓类型
            CheckAndInvalidToZero(p->StrikePrice),  ///执行价
            p->OptionsType,                         ///期权类型
            p->VolumeMultiple,                      ///合约数量乘数
            p->UnderlyingMultiple,                  ///合约基础商品乘数
            p->InstrumentID,                        ///合约代码
            p->InstrumentName,                      ///合约名称
            p->DeliveryYear,                        ///交割年份
            p->DeliveryMonth,                       ///交割月
            p->AdvanceMonth,                        ///提前月份
            p->IsTrading,                           ///当前是否交易
            p->CreateDate,                          ///创建日
            p->OpenDate,                            ///上市日
            p->ExpireDate,                          ///到期日
            p->StartDelivDate,                      ///开始交割日
            p->EndDelivDate,                        ///最后交割日
            p->BasisPrice,                          ///挂牌基准价
            p->MaxMarketOrderVolume,                ///市价单最大下单量
            p->MinMarketOrderVolume,                ///市价单最小下单量
            p->MaxLimitOrderVolume,                 ///限价单最大下单量
            p->MinLimitOrderVolume,                 ///限价单最小下单量
            p->PriceTick,                           ///最小变动价位
            p->AllowDelivPersonOpen                 ///交割月自然人开仓
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcRspInstrumentField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcOrderField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcOrderField\n"
            "\tTradingDay=%s\n"
            "\tSettlementGroupID=%s\n"
            "\tSettlementID=%d\n"
            "\tOrderSysID=%s\n"
            "\tParticipantID=%s\n"
            "\tClientID=%s\n"
            "\tUserID=%s\n"
            "\tInstrumentID=%s\n"
            "\tOrderPriceType=%c\n"
            "\tDirection=%c\n"
            "\tCombOffsetFlag=%s\n"
            "\tCombHedgeFlag=%s\n"
            "\tLimitPrice=%.6f\n"
            "\tVolumeTotalOriginal=%d\n"
            "\tTimeCondition=%c\n"
            "\tGTDDate=%s\n"
            "\tVolumeCondition=%c\n"
            "\tMinVolume=%d\n"
            "\tContingentCondition=%c\n"
            "\tStopPrice=%.6f\n"
            "\tForceCloseReason=%c\n"
            "\tOrderLocalID=%s\n"
            "\tIsAutoSuspend=%d\n"
            "\tOrderSource=%c\n"
            "\tOrderStatus=%c\n"
            "\tOrderType=%c\n"
            "\tVolumeTraded=%d\n"
            "\tVolumeTotal=%d\n"
            "\tInsertDate=%s\n"
            "\tInsertTime=%s\n"
            "\tActiveTime=%s\n"
            "\tSuspendTime=%s\n"
            "\tUpdateTime=%s\n"
            "\tCancelTime=%s\n"
            "\tActiveUserID=%s\n"
            "\tPriority=%d\n"
            "\tTimeSortID=%d\n"
            "\tClearingPartID=%s\n"
            "\tBusinessUnit=%s\n",
            p->TradingDay,                          ///交易日
            p->SettlementGroupID,                   ///结算组代码
            p->SettlementID,                        ///结算编号
            p->OrderSysID,                          ///报单编号
            p->ParticipantID,                       ///会员代码
            p->ClientID,                            ///客户代码
            p->UserID,                              ///交易用户代码
            p->InstrumentID,                        ///合约代码
            p->OrderPriceType,                      ///报单价格条件
            p->Direction,                           ///买卖方向
            p->CombOffsetFlag,                      ///组合开平标志
            p->CombHedgeFlag,                       ///组合投机套保标志
            p->LimitPrice,                          ///价格
            p->VolumeTotalOriginal,                 ///数量
            p->TimeCondition,                       ///有效期类型
            p->GTDDate,                             ///GTD日期
            p->VolumeCondition,                     ///成交量类型
            p->MinVolume,                           ///最小成交量
            p->ContingentCondition,                 ///触发条件
            p->StopPrice,                           ///止损价
            p->ForceCloseReason,                    ///强平原因
            p->OrderLocalID,                        ///本地报单编号
            p->IsAutoSuspend,                       ///自动挂起标志
            p->OrderSource,                         ///报单来源
            p->OrderStatus,                         ///报单状态
            p->OrderType,                           ///报单类型
            p->VolumeTraded,                        ///今成交数量
            p->VolumeTotal,                         ///剩余数量
            p->InsertDate,                          ///报单日期
            p->InsertTime,                          ///插入时间
            p->ActiveTime,                          ///激活时间
            p->SuspendTime,                         ///挂起时间
            p->UpdateTime,                          ///最后修改时间
            p->CancelTime,                          ///撤销时间
            p->ActiveUserID,                        ///最后修改交易用户代码
            p->Priority,                            ///优先权
            p->TimeSortID,                          ///按时间排队的序号
            p->ClearingPartID,                      ///结算会员编号
            p->BusinessUnit                         ///业务单元
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcOrderField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcTradeField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcTradeField\n"
            "\tTradingDay=%s\n"
            "\tSettlementGroupID=%s\n"
            "\tSettlementID=%d\n"
            "\tTradeID=%s\n"
            "\tDirection=%c\n"
            "\tOrderSysID=%s\n"
            "\tParticipantID=%s\n"
            "\tClientID=%s\n"
            "\tTradingRole=0X%02X\n"
            "\tAccountID=%s\n"
            "\tInstrumentID=%s\n"
            "\tOffsetFlag=%c\n"
            "\tHedgeFlag=%c\n"
            "\tPrice=%.6f\n"
            "\tVolume=%d\n"
            "\tTradeTime=%s\n"
            "\tTradeType=%c\n"
            "\tPriceSource=0X%02X\n"
            "\tUserID=%s\n"
            "\tOrderLocalID=%s\n"
            "\tClearingPartID=%s\n"
            "\tBusinessUnit=%s\n",
            p->TradingDay,                          ///交易日
            p->SettlementGroupID,                   ///结算组代码
            p->SettlementID,                        ///结算编号
            p->TradeID,                             ///成交编号
            p->Direction,                           ///买卖方向
            p->OrderSysID,                          ///报单编号
            p->ParticipantID,                       ///会员代码
            p->ClientID,                            ///客户代码
            p->TradingRole,                         ///交易角色
            p->AccountID,                           ///资金帐号
            p->InstrumentID,                        ///合约代码
            p->OffsetFlag,                          ///开平标志
            p->HedgeFlag,                           ///投机套保标志
            p->Price,                               ///价格
            p->Volume,                              ///数量
            p->TradeTime,                           ///成交时间
            p->TradeType,                           ///成交类型
            p->PriceSource,                         ///成交价来源
            p->UserID,                              ///交易用户代码
            p->OrderLocalID,                        ///本地报单编号
            p->ClearingPartID,                      ///结算会员编号
            p->BusinessUnit                         ///业务单元
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcTradeField <null>");
    }

    return buf;
}

std::string XeleDatatypeFormater::ToString(const CXeleFtdcInstrumentStatusField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcInstrumentStatusField\n"
            "\tSettlementGroupID=%s\n"
            "\tInstrumentID=%s\n"
            "\tInstrumentStatus=%c\n"
            "\tTradingSegmentSN=%d\n"
            "\tEnterTime=%s\n"
            "\tEnterReason=0X%02X\n",
            p->SettlementGroupID,                   ///结算组代码
            p->InstrumentID,                        ///合约代码
            p->InstrumentStatus,                    ///合约交易状态
            p->TradingSegmentSN,                    ///交易阶段编号
            p->EnterTime,                           ///进入本状态时间
            p->EnterReason                          ///进入本状态原因
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CXeleFtdcInstrumentStatusField <null>");
    }

    return buf;
}
