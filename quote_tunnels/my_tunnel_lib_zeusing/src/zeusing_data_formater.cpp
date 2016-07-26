
#include "zeusing_data_formater.h"
#include <iostream>
#include <sstream>

using namespace std;

static std::string indent_string = "    ";
static std::string newline_string = "\n";

std::string DataStructFormater::ToString(const CZeusingFtdcReqUserLoginField *pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcReqUserLoginField		///Login Request" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "UserID=" << pdata->UserID << endl;
    ss << indent_string << "Password=" << pdata->Password << endl;
    ss << indent_string << "UserProductInfo=" << pdata->UserProductInfo << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcQryTradingAccountField *pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcQryTradingAccountField		///Capital account query" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcQryOrderField *pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcQryOrderField		///Order query" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "InstrumentID=" << pdata->InstrumentID << endl;
    ss << indent_string << "ExchangeID=" << pdata->ExchangeID << endl;
    ss << indent_string << "OrderSysID=" << pdata->OrderSysID << endl;
    ss << indent_string << "InsertTimeStart=" << pdata->InsertTimeStart << endl;
    ss << indent_string << "InsertTimeEnd=" << pdata->InsertTimeEnd << endl;

    return ss.str();

}

std::string DataStructFormater::ToString(const CZeusingFtdcQryInvestorPositionField *pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcQryInvestorPositionField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "InstrumentID=" << pdata->InstrumentID << endl;

    return ss.str();
}

std::string ToString(const CZeusingFtdcQryInvestorPositionDetailField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CZeusingFtdcQryInvestorPositionDetailField\n"
            "    BrokerID=%s\n"
            "    InvestorID=%s\n"
            "    InstrumentID=%s\n",
            p->BrokerID,                   //请求ID
            p->InvestorID,                 //资金账户ID
            p->InstrumentID                //合约代码
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CZeusingFtdcQryInvestorPositionDetailField <null>");
    }

    return buf;
}

std::string DataStructFormater::ToString(const CZeusingFtdcRspUserLoginField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcRspUserLoginField		///Login Respond" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "TradingDay=" << pdata->TradingDay << endl;
    ss << indent_string << "LoginTime=" << pdata->LoginTime << endl;
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "UserID=" << pdata->UserID << endl;
    ss << indent_string << "SystemName=" << pdata->SystemName << endl;
    ss << indent_string << "FrontID=" << pdata->FrontID << endl;
    ss << indent_string << "SessionID=" << pdata->SessionID << endl;
    ss << indent_string << "MaxOrderRef=" << pdata->MaxOrderRef << endl;
    ss << indent_string << "SHFETime=" << pdata->SHFETime << endl;
    ss << indent_string << "DCETime=" << pdata->DCETime << endl;
    ss << indent_string << "CZCETime=" << pdata->CZCETime << endl;
    ss << indent_string << "FFEXTime=" << pdata->FFEXTime << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcUserLogoutField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcUserLogoutField		///Logout Request" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "UserID=" << pdata->UserID << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcUserPasswordUpdateField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcUserPasswordUpdateField		///Password change" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "UserID=" << pdata->UserID << endl;
    ss << indent_string << "OldPassword=" << pdata->OldPassword << endl;
    ss << indent_string << "NewPassword=" << pdata->NewPassword << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcInputOrderField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcInputOrderField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "InstrumentID=" << pdata->InstrumentID << endl;
    ss << indent_string << "OrderRef=" << pdata->OrderRef << endl;
    ss << indent_string << "UserID=" << pdata->UserID << endl;
    ss << indent_string << "OrderPriceType=" << pdata->OrderPriceType << endl;
    ss << indent_string << "Direction=" << pdata->Direction << endl;
    ss << indent_string << "CombOffsetFlag=" << pdata->CombOffsetFlag << endl;
    ss << indent_string << "CombHedgeFlag=" << pdata->CombHedgeFlag << endl;
    ss << indent_string << "LimitPrice=" << pdata->LimitPrice << endl;
    ss << indent_string << "VolumeTotalOriginal=" << pdata->VolumeTotalOriginal << endl;
    ss << indent_string << "TimeCondition=" << pdata->TimeCondition << endl;
    ss << indent_string << "GTDDate=" << pdata->GTDDate << endl;
    ss << indent_string << "VolumeCondition=" << pdata->VolumeCondition << endl;
    ss << indent_string << "MinVolume=" << pdata->MinVolume << endl;
    ss << indent_string << "ContingentCondition=" << pdata->ContingentCondition << endl;
    ss << indent_string << "StopPrice=" << pdata->StopPrice << endl;
    ss << indent_string << "ForceCloseReason=" << pdata->ForceCloseReason << endl;
    ss << indent_string << "IsAutoSuspend=" << pdata->IsAutoSuspend << endl;
    ss << indent_string << "BusinessUnit=" << pdata->BusinessUnit << endl;
    ss << indent_string << "RequestID=" << pdata->RequestID << endl;
    ss << indent_string << "UserForceClose=" << pdata->UserForceClose << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcInputOrderActionField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcInputOrderActionField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "OrderActionRef=" << pdata->OrderActionRef << endl;
    ss << indent_string << "OrderRef=" << pdata->OrderRef << endl;
    ss << indent_string << "RequestID=" << pdata->RequestID << endl;
    ss << indent_string << "FrontID=" << pdata->FrontID << endl;
    ss << indent_string << "SessionID=" << pdata->SessionID << endl;
    ss << indent_string << "ExchangeID=" << pdata->ExchangeID << endl;
    ss << indent_string << "OrderSysID=" << pdata->OrderSysID << endl;
    ss << indent_string << "ActionFlag=" << pdata->ActionFlag << endl;
    ss << indent_string << "LimitPrice=" << pdata->LimitPrice << endl;
    ss << indent_string << "VolumeChange=" << pdata->VolumeChange << endl;
    ss << indent_string << "UserID=" << pdata->UserID << endl;
    ss << indent_string << "InstrumentID=" << pdata->InstrumentID << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcOrderActionField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcOrderActionField	" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "OrderActionRef=" << pdata->OrderActionRef << endl;
    ss << indent_string << "OrderRef=" << pdata->OrderRef << endl;
    ss << indent_string << "RequestID=" << pdata->RequestID << endl;
    ss << indent_string << "FrontID=" << pdata->FrontID << endl;
    ss << indent_string << "SessionID=" << pdata->SessionID << endl;
    ss << indent_string << "ExchangeID=" << pdata->ExchangeID << endl;
    ss << indent_string << "OrderSysID=" << pdata->OrderSysID << endl;
    ss << indent_string << "ActionFlag=" << pdata->ActionFlag << endl;
    ss << indent_string << "LimitPrice=" << pdata->LimitPrice << endl;
    ss << indent_string << "VolumeChange=" << pdata->VolumeChange << endl;
    ss << indent_string << "ActionDate=" << pdata->ActionDate << endl;
    ss << indent_string << "ActionTime=" << pdata->ActionTime << endl;
    ss << indent_string << "TraderID=" << pdata->TraderID << endl;
    ss << indent_string << "InstallID=" << pdata->InstallID << endl;
    ss << indent_string << "OrderLocalID=" << pdata->OrderLocalID << endl;
    ss << indent_string << "ActionLocalID=" << pdata->ActionLocalID << endl;
    ss << indent_string << "ParticipantID=" << pdata->ParticipantID << "    ///会员代码: " << endl;
    ss << indent_string << "ClientID=" << pdata->ClientID << endl;
    ss << indent_string << "BusinessUnit=" << pdata->BusinessUnit << endl;
    ss << indent_string << "OrderActionStatus=" << pdata->OrderActionStatus << endl;
    ss << indent_string << "UserID=" << pdata->UserID << endl;
    ss << indent_string << "StatusMsg=" << pdata->StatusMsg << endl;
    ss << indent_string << "InstrumentID=" << pdata->InstrumentID << endl;
    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcOrderField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcOrderField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "InstrumentID=" << pdata->InstrumentID << endl;
    ss << indent_string << "OrderRef=" << pdata->OrderRef << endl;
    ss << indent_string << "UserID=" << pdata->UserID << endl;
    ss << indent_string << "OrderPriceType=" << pdata->OrderPriceType << endl;
    ss << indent_string << "Direction=" << pdata->Direction << endl;
    ss << indent_string << "CombOffsetFlag=" << pdata->CombOffsetFlag << endl;
    ss << indent_string << "CombHedgeFlag=" << pdata->CombHedgeFlag << endl;
    ss << indent_string << "LimitPrice=" << pdata->LimitPrice << endl;
    ss << indent_string << "VolumeTotalOriginal=" << pdata->VolumeTotalOriginal << endl;
    ss << indent_string << "TimeCondition=" << pdata->TimeCondition << endl;
    ss << indent_string << "GTDDate=" << pdata->GTDDate << endl;
    ss << indent_string << "VolumeCondition=" << pdata->VolumeCondition << endl;
    ss << indent_string << "MinVolume=" << pdata->MinVolume << endl;
    ss << indent_string << "ContingentCondition=" << pdata->ContingentCondition << endl;
    ss << indent_string << "StopPrice=" << pdata->StopPrice << endl;
    ss << indent_string << "ForceCloseReason=" << pdata->ForceCloseReason << endl;
    ss << indent_string << "IsAutoSuspend=" << pdata->IsAutoSuspend << endl;
    ss << indent_string << "BusinessUnit=" << pdata->BusinessUnit << endl;
    ss << indent_string << "RequestID=" << pdata->RequestID << endl;
    ss << indent_string << "OrderLocalID=" << pdata->OrderLocalID << endl;
    ss << indent_string << "ExchangeID=" << pdata->ExchangeID << endl;
    ss << indent_string << "ParticipantID=" << pdata->ParticipantID << endl;
    ss << indent_string << "ClientID=" << pdata->ClientID << endl;
    ss << indent_string << "ExchangeInstID=" << pdata->ExchangeInstID << endl;
    ss << indent_string << "TraderID=" << pdata->TraderID << endl;
    ss << indent_string << "InstallID=" << pdata->InstallID << endl;
    ss << indent_string << "OrderSubmitStatus=" << pdata->OrderSubmitStatus << endl;
    ss << indent_string << "NotifySequence=" << pdata->NotifySequence << endl;
    ss << indent_string << "TradingDay=" << pdata->TradingDay << endl;
    ss << indent_string << "SettlementID=" << pdata->SettlementID << endl;
    ss << indent_string << "OrderSysID=" << pdata->OrderSysID << endl;

    ss << indent_string << "OrderSource=";
    pdata->OrderSource == '\0' ? ss << "<null>" : ss << pdata->OrderSource << endl;

    ss << indent_string << "OrderStatus=";
    pdata->OrderStatus == '\0' ? ss << "<null>" : ss << pdata->OrderStatus << endl;

    ss << indent_string << "OrderType=";
    pdata->OrderType == '\0' ? ss << "<null>" : ss << pdata->OrderType << endl;

    ss << indent_string << "VolumeTraded=" << pdata->VolumeTraded << endl;
    ss << indent_string << "VolumeTotal=" << pdata->VolumeTotal << endl;
    ss << indent_string << "InsertDate=" << pdata->InsertDate << endl;
    ss << indent_string << "InsertTime=" << pdata->InsertTime << endl;
    ss << indent_string << "ActiveTime=" << pdata->ActiveTime << endl;
    ss << indent_string << "SuspendTime=" << pdata->SuspendTime << endl;
    ss << indent_string << "UpdateTime=" << pdata->UpdateTime << endl;
    ss << indent_string << "CancelTime=" << pdata->CancelTime << endl;
    ss << indent_string << "ActiveTraderID=" << pdata->ActiveTraderID << endl;
    ss << indent_string << "ClearingPartID=" << pdata->ClearingPartID << endl;
    ss << indent_string << "SequenceNo=" << pdata->SequenceNo << endl;
    ss << indent_string << "FrontID=" << pdata->FrontID << endl;
    ss << indent_string << "SessionID=" << pdata->SessionID << endl;
    ss << indent_string << "UserProductInfo=" << pdata->UserProductInfo << endl;
    ss << indent_string << "StatusMsg=" << pdata->StatusMsg << endl;
    ss << indent_string << "UserForceClose=" << pdata->UserForceClose << endl;
    ss << indent_string << "ActiveUserID=" << pdata->ActiveUserID << endl;
    ss << indent_string << "BrokerOrderSeq=" << pdata->BrokerOrderSeq << endl;
    ss << indent_string << "RelativeOrderSysID=" << pdata->RelativeOrderSysID << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcTradeField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcTradeField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }

    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "InstrumentID=" << pdata->InstrumentID << endl;
    ss << indent_string << "OrderRef=" << pdata->OrderRef << endl;
    ss << indent_string << "UserID=" << pdata->UserID << endl;
    ss << indent_string << "ExchangeID=" << pdata->ExchangeID << endl;
    ss << indent_string << "TradeID=" << pdata->TradeID << endl;
    ss << indent_string << "Direction=" << pdata->Direction << endl;
    ss << indent_string << "OrderSysID=" << pdata->OrderSysID << endl;
    ss << indent_string << "ParticipantID=" << pdata->ParticipantID << endl;
    ss << indent_string << "ClientID=" << pdata->ClientID << endl;

    //ss << indent_string << "TradingRole=" << pdata->TradingRole << "    ///交易角色: " << endl;
    ss << indent_string << "TradingRole=";
    pdata->TradingRole == '\0' ? ss << "<null>" : ss << pdata->TradingRole << endl;

    ss << indent_string << "ExchangeInstID=" << pdata->ExchangeInstID << endl;
    ss << indent_string << "OffsetFlag=" << pdata->OffsetFlag << endl;
    ss << indent_string << "HedgeFlag=" << pdata->HedgeFlag << endl;
    ss << indent_string << "Price=" << pdata->Price << endl;
    ss << indent_string << "Volume=" << pdata->Volume << endl;
    ss << indent_string << "TradeDate=" << pdata->TradeDate << endl;
    ss << indent_string << "TradeTime=" << pdata->TradeTime << endl;

    //ss << indent_string << "TradeType=" << pdata->TradeType << "    ///成交类型: " << endl;
    ss << indent_string << "TradeType=";
    pdata->TradeType == '\0' ? ss << "<null>" : ss << pdata->TradeType << endl;

    //ss << indent_string << "PriceSource=" << pdata->PriceSource << "    ///成交价来源: " << endl;
    ss << indent_string << "PriceSource=";
    pdata->PriceSource == '\0' ? ss << "<null>" : ss << pdata->PriceSource << endl;

    ss << indent_string << "TraderID=" << pdata->TraderID << endl;
    ss << indent_string << "OrderLocalID=" << pdata->OrderLocalID << endl;
    ss << indent_string << "ClearingPartID=" << pdata->ClearingPartID << endl;
    ss << indent_string << "BusinessUnit=" << pdata->BusinessUnit << endl;
    ss << indent_string << "SequenceNo=" << pdata->SequenceNo << endl;
    ss << indent_string << "TradingDay=" << pdata->TradingDay << endl;
    ss << indent_string << "SettlementID=" << pdata->SettlementID << endl;
    ss << indent_string << "BrokerOrderSeq=" << pdata->BrokerOrderSeq << endl;
    ss << indent_string << "TradeSource=" << pdata->TradeSource << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcSettlementInfoConfirmField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcSettlementInfoConfirmField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "ConfirmDate=" << pdata->ConfirmDate << endl;
    ss << indent_string << "ConfirmTime=" << pdata->ConfirmTime << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcInvestorPositionField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcInvestorPositionField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "InstrumentID=" << pdata->InstrumentID << endl;
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "PosiDirection=" << pdata->PosiDirection << endl;
    ss << indent_string << "HedgeFlag=" << pdata->HedgeFlag << endl;
    ss << indent_string << "PositionDate=" << pdata->PositionDate << endl;
    ss << indent_string << "YdPosition=" << pdata->YdPosition << endl;
    ss << indent_string << "Position=" << pdata->Position << endl;
    ss << indent_string << "LongFrozen=" << pdata->LongFrozen << endl;
    ss << indent_string << "ShortFrozen=" << pdata->ShortFrozen << endl;
    ss << indent_string << "LongFrozenAmount=" << pdata->LongFrozenAmount << endl;
    ss << indent_string << "ShortFrozenAmount=" << pdata->ShortFrozenAmount << endl;
    ss << indent_string << "OpenVolume=" << pdata->OpenVolume << endl;
    ss << indent_string << "CloseVolume=" << pdata->CloseVolume << endl;
    ss << indent_string << "OpenAmount=" << pdata->OpenAmount << endl;
    ss << indent_string << "CloseAmount=" << pdata->CloseAmount << endl;
    ss << indent_string << "PositionCost=" << pdata->PositionCost << endl;
    ss << indent_string << "PreMargin=" << pdata->PreMargin << endl;
    ss << indent_string << "UseMargin=" << pdata->UseMargin << endl;
    ss << indent_string << "FrozenMargin=" << pdata->FrozenMargin << endl;
    ss << indent_string << "FrozenCash=" << pdata->FrozenCash << endl;
    ss << indent_string << "FrozenCommission=" << pdata->FrozenCommission << endl;
    ss << indent_string << "CashIn=" << pdata->CashIn << endl;
    ss << indent_string << "Commission=" << pdata->Commission << endl;
    ss << indent_string << "CloseProfit=" << pdata->CloseProfit << endl;
    ss << indent_string << "PositionProfit=" << pdata->PositionProfit << endl;
    ss << indent_string << "PreSettlementPrice=" << pdata->PreSettlementPrice << endl;
    ss << indent_string << "SettlementPrice=" << pdata->SettlementPrice << endl;
    ss << indent_string << "TradingDay=" << pdata->TradingDay << endl;
    ss << indent_string << "SettlementID=" << pdata->SettlementID << endl;
    ss << indent_string << "OpenCost=" << pdata->OpenCost << endl;
    ss << indent_string << "ExchangeMargin=" << pdata->ExchangeMargin << endl;
    ss << indent_string << "CombPosition=" << pdata->CombPosition << endl;
    ss << indent_string << "CombLongFrozen=" << pdata->CombLongFrozen << endl;
    ss << indent_string << "CombShortFrozen=" << pdata->CombShortFrozen << endl;
    ss << indent_string << "CloseProfitByDate=" << pdata->CloseProfitByDate << endl;
    ss << indent_string << "CloseProfitByTrade=" << pdata->CloseProfitByTrade << endl;
    ss << indent_string << "TodayPosition=" << pdata->TodayPosition << endl;
    ss << indent_string << "MarginRateByMoney=" << pdata->MarginRateByMoney << endl;
    ss << indent_string << "MarginRateByVolume=" << pdata->MarginRateByVolume << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcRspInfoField* pdata)
{
    stringstream ss;
    ss << newline_string << indent_string << "structName=CZeusingFtdcRspInfoField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }

    ss << indent_string << "ErrorID=" << pdata->ErrorID << endl;
    ss << indent_string << "ErrorMsg=" << pdata->ErrorMsg << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcTradingAccountField *pdata)
{
    stringstream ss;
    ss << std::fixed << newline_string << "structName=CZeusingFtdcTradingAccountField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "AccountID=" << pdata->AccountID << endl;
    ss << indent_string << "PreMortgage=" << pdata->PreMortgage << endl;
    ss << indent_string << "PreCredit=" << pdata->PreCredit << endl;
    ss << indent_string << "PreDeposit=" << pdata->PreDeposit << endl;
    ss << indent_string << "PreBalance=" << pdata->PreBalance << endl;
    ss << indent_string << "PreMargin=" << pdata->PreMargin << endl;
    ss << indent_string << "InterestBase=" << pdata->InterestBase << endl;
    ss << indent_string << "Interest=" << pdata->Interest << endl;
    ss << indent_string << "Deposit=" << pdata->Deposit << endl;
    ss << indent_string << "Withdraw=" << pdata->Withdraw << endl;
    ss << indent_string << "FrozenMargin=" << pdata->FrozenMargin << endl;
    ss << indent_string << "FrozenCash=" << pdata->FrozenCash << endl;
    ss << indent_string << "FrozenCommission=" << pdata->FrozenCommission << endl;
    ss << indent_string << "CurrMargin=" << pdata->CurrMargin << endl;
    ss << indent_string << "CashIn=" << pdata->CashIn << endl;
    ss << indent_string << "Commission=" << pdata->Commission << endl;
    ss << indent_string << "CloseProfit=" << pdata->CloseProfit << endl;
    ss << indent_string << "PositionProfit=" << pdata->PositionProfit << endl;
    ss << indent_string << "Balance=" << pdata->Balance << endl;
    ss << indent_string << "Available=" << pdata->Available << endl;
    ss << indent_string << "WithdrawQuota=" << pdata->WithdrawQuota << endl;
    ss << indent_string << "Reserve=" << pdata->Reserve << endl;
    ss << indent_string << "TradingDay=" << pdata->TradingDay << endl;
    ss << indent_string << "SettlementID=" << pdata->SettlementID << endl;
    ss << indent_string << "Credit=" << pdata->Credit << endl;
    ss << indent_string << "Mortgage=" << pdata->Mortgage << endl;
    ss << indent_string << "ExchangeMargin=" << pdata->ExchangeMargin << endl;
    ss << indent_string << "DeliveryMargin=" << pdata->DeliveryMargin << endl;
    ss << indent_string << "ExchangeDeliveryMargin=" << pdata->ExchangeDeliveryMargin << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcSettlementInfoField *pdata)
{
    stringstream ss;
    ss << std::fixed << newline_string << "structName=CZeusingFtdcSettlementInfoField" << endl;
    if (!pdata)
    {
        ss << "<null>" << endl;
        return ss.str();
    }
    ss << indent_string << "TradingDay=" << pdata->TradingDay << endl;
    ss << indent_string << "SettlementID=" << pdata->SettlementID << endl;
    ss << indent_string << "BrokerID=" << pdata->BrokerID << endl;
    ss << indent_string << "InvestorID=" << pdata->InvestorID << endl;
    ss << indent_string << "SequenceNo=" << pdata->SequenceNo << endl;
    ss << indent_string << "Content=" << pdata->Content << endl;

    return ss.str();
}

std::string DataStructFormater::ToString(const CZeusingFtdcInvestorPositionDetailField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CZeusingFtdcInvestorPositionDetailField\n"
            "    InstrumentID=%s\n"
            "    BrokerID=%s\n"
            "    InvestorID=%s\n"
            "    HedgeFlag=%c\n"
            "    Direction=%c\n"
            "    OpenDate=%s\n"
            "    TradeID=%s\n"
            "    Volume=%d\n"
            "    OpenPrice=%.4f\n"
            "    TradingDay=%s\n"
            "    SettlementID=%d\n"
            "    TradeType=%c\n"
            "    CombInstrumentID=%s\n"
            "    ExchangeID=%s\n"
            "    CloseProfitByDate=%.4f\n"
            "    CloseProfitByTrade=%.4f\n"
            "    PositionProfitByDate=%.4f\n"
            "    PositionProfitByTrade=%.4f\n"
            "    Margin=%.4f\n"
            "    ExchMargin=%.4f\n"
            "    MarginRateByMoney=%.4f\n"
            "    MarginRateByVolume=%.4f\n"
            "    LastSettlementPrice=%.4f\n"
            "    SettlementPrice=%.4f\n"
            "    CloseVolume=%d\n"
            "    CloseAmount=%.4f\n",
            p->InstrumentID,                //合约代码
            p->BrokerID,                    //经纪公司代码
            p->InvestorID,                  //投资者代码
            p->HedgeFlag,                   //投机套保标志
            p->Direction,                   //买卖
            p->OpenDate,                    //开仓日期
            p->TradeID,                     //成交编号
            p->Volume,                      //数量
            p->OpenPrice,                   //开仓价
            p->TradingDay,                  //交易日
            p->SettlementID,                //结算编号
            p->TradeType,                   //成交类型
            p->CombInstrumentID,            //组合合约代码
            p->ExchangeID,                  //交易所代码
            p->CloseProfitByDate,           //逐日盯市平仓盈亏
            p->CloseProfitByTrade,          //逐笔对冲平仓盈亏
            p->PositionProfitByDate,        //逐日盯市持仓盈亏
            p->PositionProfitByTrade,       //逐笔对冲持仓盈亏
            p->Margin,                      //投资者保证金
            p->ExchMargin,                  //交易所保证金
            p->MarginRateByMoney,           //保证金率
            p->MarginRateByVolume,          //保证金率(按手数)
            p->LastSettlementPrice,         //昨结算价
            p->SettlementPrice,             //结算价
            p->CloseVolume,                 //平仓量
            p->CloseAmount                  //平仓金额
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CZeusingFtdcInvestorPositionDetailField <null>");
    }

    return buf;
}

std::string DataStructFormater::ToString(const CZeusingFtdcInstrumentField *p)
{
    char buf[2048];

    if (p)
    {
        snprintf(buf, sizeof(buf), "structName=CZeusingFtdcInstrumentField\n"
            "   InstrumentID=%s\n"
            "   ExchangeID=%s\n"
            "   InstrumentName=%s\n"
            "   ExchangeInstID=%s\n"
            "   ProductID=%s\n"
            "   ProductClass=0X%02X\n"
            "   DeliveryYear=%04d\n"
            "   DeliveryMonth=%02d\n"
            "   MaxMarketOrderVolume=%d\n"
            "   MinMarketOrderVolume=%d\n"
            "   MaxLimitOrderVolume=%d\n"
            "   MinLimitOrderVolume=%d\n"
            "   VolumeMultiple=%d\n"
            "   PriceTick=%.4f\n"
            "   CreateDate=%s\n"
            "   OpenDate=%s\n"
            "   ExpireDate=%s\n"
            "   StartDelivDate=%s\n"
            "   EndDelivDate=%s\n"
            "   InstLifePhase=0X%02X\n"
            "   IsTrading=%d\n"
            "   PositionType=0X%02X\n"
            "   PositionDateType=0X%02X\n",
            p->InstrumentID,                   //合约代码
            p->ExchangeID,                     //交易所代码
            p->InstrumentName,                 //合约名称
            p->ExchangeInstID,                 //合约在交易所的代码
            p->ProductID,                      //产品代码
            p->ProductClass,                   //产品类型
            p->DeliveryYear,                   //交割年份
            p->DeliveryMonth,                  //交割月
            p->MaxMarketOrderVolume,           //市价单最大下单量
            p->MinMarketOrderVolume,           //市价单最小下单量
            p->MaxLimitOrderVolume,            //限价单最大下单量
            p->MinLimitOrderVolume,            //限价单最小下单量
            p->VolumeMultiple,                 //合约数量乘数
            p->PriceTick,                      //最小变动价位
            p->CreateDate,                     //创建日
            p->OpenDate,                       //上市日
            p->ExpireDate,                     //到期日
            p->StartDelivDate,                 //开始交割日
            p->EndDelivDate,                   //结束交割日
            p->InstLifePhase,                  //合约生命周期状态
            p->IsTrading,                      //当前是否交易
            p->PositionType,                   //持仓类型
            p->PositionDateType               //持仓日期类型
            );
    }
    else
    {
        snprintf(buf, sizeof(buf), "structName=CZeusingFtdcInstrumentField <null>");
    }

    return buf;
}
