/*
 * ctp_data_formater.cpp
 *
 *  Created on: 2013-8-13
 *      Author: oliver
 */

#include "rem_struct_formater.h"
#include <iostream>
#include <sstream>

using namespace std;

static std::string HexFormatter(const char *c, size_t len)
{
    if (len * 3 > 512) return "too long";
    char t_g[512];
    char *ps = t_g;
    for (size_t i = 0; i < len; ++i, ps += 3)
    {
        sprintf(ps, "%02x ", c[i]);
    }
    return t_g;
}

std::string RemStructFormater::ToString(const EES_EnterOrderField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_EnterOrderField\n"
            "  Account=%s\n"
            "  Side=%d\n"
            "  Exchange=%d\n"
            "  Symbol=%s\n"
            "  SecType=%d\n"
            "  Price=%.4f\n"
            "  Qty=%d\n"
            "  ForceCloseReason=%d\n"
            "  ClientOrderToken=%d\n"
            "  Tif=%d\n"
            "  MinQty=%d\n"
            "  CustomField=%lld\n"
            "  MarketSessionId=%d\n"
            "  HedgeFlag=%d\n",
            p->m_Account,
            (int) p->m_Side,
            (int) p->m_Exchange,
            p->m_Symbol,
            (int) p->m_SecType,
            p->m_Price,
            p->m_Qty,
            (int) p->m_ForceCloseReason,
            p->m_ClientOrderToken,
            p->m_Tif,
            p->m_MinQty,
            p->m_CustomField,
            (int) p->m_MarketSessionId,
            (int) p->m_HedgeFlag
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_EnterOrderField <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_CancelOrder* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_CancelOrder\n"
            "  MarketOrderToken=%lld\n"
            "  Quantity=%d\n"
            "  Account=%s\n",
            p->m_MarketOrderToken,
            p->m_Quantity,
            p->m_Account
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_CancelOrder <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_LogonResponse* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_LogonResponse\n"
            "  Result=%d\n"
            "  UserId=%d\n"
            "  TradingDate=%d\n"
            "  MaxToken=%d\n",
            p->m_Result,
            p->m_UserId,
            p->m_TradingDate,
            p->m_MaxToken
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_LogonResponse <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_AccountInfo* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountInfo\n"
            "  Account=%s\n"
            "  Previlege=%d\n"
            "  InitialBp=%.4f\n"
            "  AvailableBp=%.4f\n"
            "  Margin=%.6f\n"
            "  FrozenMargin=%.4f\n"
            "  CommissionFee=%.4f\n"
            "  FrozenCommission=%.4f\n",
            p->m_Account,
            p->m_Previlege,
            p->m_InitialBp,
            p->m_AvailableBp,
            p->m_Margin,
            p->m_FrozenMargin,
            p->m_CommissionFee,
            p->m_FrozenCommission
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountInfo <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_AccountPosition* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountPosition\n"
            "  actId=%s\n"
            "  Symbol=%s\n"
            "  PosiDirection=%d\n"
            "  InitOvnQty=%d\n"
            "  OvnQty=%d\n"
            "  FrozenOvnQty=%d\n"
            "  TodayQty=%d\n"
            "  FrozenTodayQty=%d\n"
            "  OvnMargin=%.4f\n"
            "  TodayMargin=%.4f\n"
            "  HedgeFlag=%d\n",
            p->m_actId,
            p->m_Symbol,
            p->m_PosiDirection,
            p->m_InitOvnQty,
            p->m_OvnQty,
            p->m_FrozenOvnQty,
            p->m_TodayQty,
            p->m_FrozenTodayQty,
            p->m_OvnMargin,
            p->m_TodayMargin,
            (int) p->m_HedgeFlag
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountPosition <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_AccountBP* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountBP\n"
            "  account=%s\n"
            "  InitialBp=%.4f\n"
            "  AvailableBp=%.4f\n"
            "  Margin=%.4f\n"
            "  FrozenMargin=%.4f\n"
            "  CommissionFee=%.4f\n"
            "  FrozenCommission=%.4f\n"
            "  OvnInitMargin=%.4f\n"
            "  TotalLiquidPL=%.4f\n"
            "  TotalMarketPL=%.4f\n",
            p->m_account,
            p->m_InitialBp,
            p->m_AvailableBp,
            p->m_Margin,
            p->m_FrozenMargin,
            p->m_CommissionFee,
            p->m_FrozenCommission,
            p->m_OvnInitMargin,
            p->m_TotalLiquidPL,
            p->m_TotalMarketPL
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountBP <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_SymbolField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_SymbolField\n"
            "  SecType=%d\n"
            "  symbol=%s\n"
            "  symbolName=%s\n"
            "  ExchangeID=%d\n"
            "  ProdID=%s\n"
            "  DeliveryYear=%d\n"
            "  DeliveryMonth=%02d\n"
            "  MaxMarketOrderVolume=%d\n"
            "  MinMarketOrderVolume=%d\n"
            "  MaxLimitOrderVolume=%d\n"
            "  MinLimitOrderVolume=%d\n"
            "  VolumeMultiple=%d\n"
            "  PriceTick=%.4f\n"
            "  CreateDate=%d\n"
            "  OpenDate=%d\n"
            "  ExpireDate=%d\n"
            "  StartDelivDate=%d\n"
            "  EndDelivDate=%d\n"
            "  InstLifePhase=%d\n"
            "  IsTrading=%d\n",
            (int) p->m_SecType,
            p->m_symbol,
            p->m_symbolName,
            (int) p->m_ExchangeID,
            p->m_ProdID,
            p->m_DeliveryYear,
            p->m_DeliveryMonth,
            p->m_MaxMarketOrderVolume,
            p->m_MinMarketOrderVolume,
            p->m_MaxLimitOrderVolume,
            p->m_MinLimitOrderVolume,
            p->m_VolumeMultiple,
            p->m_PriceTick,
            p->m_CreateDate,
            p->m_OpenDate,
            p->m_ExpireDate,
            p->m_StartDelivDate,
            p->m_EndDelivDate,
            p->m_InstLifePhase,
            p->m_IsTrading
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_SymbolField <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_AccountMargin* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountMargin\n"
            "  SecType=%d\n"
            "  symbol=%s\n"
            "  ExchangeID=%d\n"
            "  ProdID=%s\n"
            "  LongMarginRatio=%.4f\n"
            "  ShortMarginRatio=%.4f\n",
            (int) p->m_SecType,
            p->m_symbol,
            (int) p->m_ExchangeID,
            p->m_ProdID,
            p->m_LongMarginRatio,
            p->m_ShortMarginRatio
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountMargin <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_AccountFee* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountFee\n"
            "  SecType=%d\n"
            "  symbol=%s\n"
            "  ExchangeID=%d\n"
            "  ProdID=%s\n"
            "  OpenRatioByMoney=%.6f\n"
            "  OpenRatioByVolume=%.6f\n"
            "  CloseYesterdayRatioByMoney=%.6f\n"
            "  CloseYesterdayRatioByVolume=%.6f\n"
            "  CloseTodayRatioByMoney=%.6f\n"
            "  CloseTodayRatioByVolume=%.6f\n",
            (int) p->m_SecType,
            p->m_symbol,
            (int) p->m_ExchangeID,
            p->m_ProdID,
            p->m_OpenRatioByMoney,
            p->m_OpenRatioByVolume,
            p->m_CloseYesterdayRatioByMoney,
            p->m_CloseYesterdayRatioByVolume,
            p->m_CloseTodayRatioByMoney,
            p->m_CloseTodayRatioByVolume
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_AccountFee <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_OrderAcceptField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderMarketAcceptField\n"
            "  ClientOrderToken=%d\n"
            "  MarketOrderToken=%lld\n"
            "  OrderState=%d\n"
            "  UserID=%d\n"
            "  AcceptTime=%lld\n"
            "  Account=%s\n"
            "  Side=%d\n"
            "  Exchange=%d\n"
            "  Symbol=%s\n"
            "  SecType=%d\n"
            "  Price=%.4f\n"
            "  Qty=%d\n"
            "  ForceCloseReason=%d\n"
            "  Tif=%d\n"
            "  MinQty=%d\n"
            "  CustomField=%lld\n"
            "  MarketSessionId=%d\n"
            "  HedgeFlag=%d\n",
            p->m_ClientOrderToken,
            p->m_MarketOrderToken,
            (int) p->m_OrderState,
            p->m_UserID,
            p->m_AcceptTime,
            p->m_Account,
            (int) p->m_Side,
            (int) p->m_Exchange,
            p->m_Symbol,
            (int) p->m_SecType,
            p->m_Price,
            p->m_Qty,
            (int) p->m_ForceCloseReason,
            p->m_Tif,
            p->m_MinQty,
            p->m_CustomField,
            (int) p->m_MarketSessionId,
            (int) p->m_HedgeFlag
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderMarketAcceptField <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_OrderMarketAcceptField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderMarketAcceptField\n"
            "  Account=%s\n"
            "  MarketOrderToken=%lld\n"
            "  MarketOrderId=%s\n"
            "  MarketTime=%lld\n",
            p->m_Account,
            p->m_MarketOrderToken,
            p->m_MarketOrderId,
            p->m_MarketTime
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderMarketAcceptField <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_OrderRejectField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderRejectField\n"
            "  Userid=%d\n"
            "  Timestamp=%lld\n"
            "  ClientOrderToken=%d\n"
            "  RejectedMan=%d\n"
            "  ReasonCode=%d\n"
            "  GrammerResult=%s\n"
            "  RiskResult=%s\n"
            "  GrammerText=%s\n"
            "  RiskText=%s\n",
            p->m_Userid,
            p->m_Timestamp,
            p->m_ClientOrderToken,
            (int) p->m_RejectedMan,
            (int) p->m_ReasonCode,
            HexFormatter(p->m_GrammerResult, sizeof(p->m_GrammerResult)).c_str(),
            HexFormatter(p->m_RiskResult, sizeof(p->m_RiskResult)).c_str(),
            p->m_GrammerText,
            p->m_RiskText
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderRejectField <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_OrderMarketRejectField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderMarketRejectField\n"
            "  Account=%s\n"
            "  MarketOrderToken=%lld\n"
            "  MarketTimestamp=%lld\n"
            "  ReasonText=%s\n",
            p->m_Account,
            p->m_MarketOrderToken,
            p->m_MarketTimestamp,
            p->m_ReasonText
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderMarketRejectField <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_OrderExecutionField* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderExecutionField\n"
            "  Userid=%d\n"
            "  Timestamp=%lld\n"
            "  ClientOrderToken=%d\n"
            "  MarketOrderToken=%lld\n"
            "  Quantity=%d\n"
            "  Price=%.4f\n"
            "  ExecutionID=%lld\n",
            p->m_Userid,
            p->m_Timestamp,
            p->m_ClientOrderToken,
            p->m_MarketOrderToken,
            p->m_Quantity,
            p->m_Price,
            p->m_ExecutionID
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderExecutionField <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_OrderCxled* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderCxled\n"
            "  Userid=%d\n"
            "  Timestamp=%lld\n"
            "  ClientOrderToken=%d\n"
            "  MarketOrderToken=%lld\n"
            "  Decrement=%d\n"
            "  Reason=%d\n",
            p->m_Userid,
            p->m_Timestamp,
            p->m_ClientOrderToken,
            p->m_MarketOrderToken,
            p->m_Decrement,
            (int) p->m_Reason
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_OrderCxled <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_CxlOrderRej* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_CxlOrderRej\n"
            "  account=%s\n"
            "  MarketOrderToken=%lld\n"
            "  ReasonCode=%d\n"
            "  ReasonText=%s\n",
            p->m_account,
            p->m_MarketOrderToken,
            p->m_ReasonCode,
            p->m_ReasonText
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_CxlOrderRej <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_QueryAccountOrder* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_QueryAccountOrder\n"
            "  Userid=%d\n"
            "  Timestamp=%lld\n"
            "  ClientOrderToken=%d\n"
            "  SideType=%d\n"
            "  Quantity=%d\n"
            "  InstrumentType=%d\n"
            "  symbol=%s\n"
            "  Price=%.4f\n"
            "  account=%s\n"
            "  ExchengeID=%d\n"
            "  ForceCloseReason=%d\n"
            "  MarketOrderToken=%lld\n"
            "  OrderStatus=%d\n"
            "  CloseTime=%lld\n"
            "  FilledQty=%d\n"
            "  Tif=%d\n"
            "  MinQty=%d\n"
            "  CustomField=%lld\n"
            "  MarketOrderId=%s\n"
            "  HedgeFlag=%d\n",
            p->m_Userid,
            p->m_Timestamp,
            p->m_ClientOrderToken,
            (int) p->m_SideType,
            p->m_Quantity,
            (int) p->m_InstrumentType,
            p->m_symbol,
            p->m_Price,
            p->m_account,
            (int) p->m_ExchengeID,
            (int) p->m_ForceCloseReason,
            p->m_MarketOrderToken,
            (int) p->m_OrderStatus,
            p->m_CloseTime,
            p->m_FilledQty,
            p->m_Tif,
            p->m_MinQty,
            p->m_CustomField,
            p->m_MarketOrderId,
            (int) p->m_HedgeFlag
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_QueryAccountOrder <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_QueryOrderExecution* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_QueryOrderExecution\n"
            "  Userid=%d\n"
            "  Timestamp=%lld\n"
            "  ClientOrderToken=%d\n"
            "  MarketOrderToken=%lld\n"
            "  ExecutedQuantity=%d\n"
            "  ExecutionPrice=%.4f\n"
            "  ExecutionID=%lld\n",
            p->m_Userid,
            p->m_Timestamp,
            p->m_ClientOrderToken,
            p->m_MarketOrderToken,
            p->m_ExecutedQuantity,
            p->m_ExecutionPrice,
            p->m_ExecutionID
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_QueryOrderExecution <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_PostOrder* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_PostOrder\n"
            "  Userid=%d\n"
            "  Timestamp=%lld\n"
            "  MarketOrderToken=%lld\n"
            "  ClientOrderToken=%d\n"
            "  SideType=%d\n"
            "  Quantity=%d\n"
            "  SecType=%d\n"
            "  Symbol=%s\n"
            "  price=%.4f\n"
            "  account=%s\n"
            "  ExchangeID=%d\n"
            "  ForceCloseReason=%d\n"
            "  OrderState=%d\n"
            "  ExchangeOrderID=%s\n"
            "  HedgeFlag=%d\n",
            p->m_Userid,
            p->m_Timestamp,
            p->m_MarketOrderToken,
            p->m_ClientOrderToken,
            (int) p->m_SideType,
            p->m_Quantity,
            (int) p->m_SecType,
            p->m_Symbol,
            p->m_price,
            p->m_account,
            (int) p->m_ExchangeID,
            (int) p->m_ForceCloseReason,
            (int) p->m_OrderState,
            p->m_ExchangeOrderID,
            (int) p->m_HedgeFlag
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_PostOrder <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_PostOrderExecution* p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_PostOrderExecution\n"
            "  Userid=%d\n"
            "  Timestamp=%lld\n"
            "  MarketOrderToken=%lld\n"
            "  ExecutedQuantity=%d\n"
            "  ExecutionPrice=%.4f\n"
            "  ExecutionNumber=%lld\n",
            p->m_Userid,
            p->m_Timestamp,
            p->m_MarketOrderToken,
            p->m_ExecutedQuantity,
            p->m_ExecutionPrice,
            p->m_ExecutionNumber
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_PostOrderExecution <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_SymbolStatus *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_SymbolStatus\n"
            "  ExchangeID=%d\n"
            "  Symbol=%s\n"
            "  InstrumentStatus=%d\n"
            "  TradingSegmentSN=%d\n"
            "  EnterTime=%s\n"
            "  EnterReason=%d\n",
            (int) p->m_ExchangeID,
            p->m_Symbol,
            (int) p->m_InstrumentStatus,
            p->m_TradingSegmentSN,
            p->m_EnterTime,
            (int) p->m_EnterReason
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_SymbolStatus <null>");
    }

    return buf;
}

std::string RemStructFormater::ToString(const EES_ExchangeMarketSession *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "struct=EES_ExchangeMarketSession\n"
            "  ExchangeID=%d\n"
            "  SessionCount=%d\n"
            "  SessionId=%s\n",
            (int) p->m_ExchangeID,
            (int) p->m_SessionCount,
            p->m_SessionId
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "struct=EES_ExchangeMarketSession <null>");
    }

    return buf;
}
