#include "quote_femas.h"
#include <iomanip>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"


using namespace my_cmn;
using namespace std;

MYFEMASDataHandler::MYFEMASDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg)
{
    // 初始化
    logoned_ = false;
    api_ = NULL;

    SubsribeDatas code_list = cfg_.Subscribe_datas();
    const LogonConfig &logon_cfg = cfg_.Logon_config();
    int topic = atoi(logon_cfg.topic.c_str());

    pp_instruments_ = NULL;
    if (code_list.empty())
    {
        // 订阅全部
        code_list.insert("*");
    }
    if (code_list.size() == 1 && (boost::to_upper_copy(*(code_list.begin())) == "ALL" || code_list.begin()->empty()))
    {
        MY_LOG_INFO("subscribe all, transfer to *: %s", code_list.begin()->c_str());
        // 订阅全部
        code_list.clear();
        code_list.insert("*");
    }

    // 解析订阅列表
    sub_count_ = code_list.size();
    pp_instruments_ = new char *[sub_count_];
    int i = 0;
    BOOST_FOREACH(const std::string &value, code_list)
    {
        instruments_.append(value + "|");
        pp_instruments_[i] = new char[value.length() + 1];
        memcpy(pp_instruments_[i], value.c_str(), value.length() + 1);
        ++i;
    }

    // 初始化
    api_ = CUstpFtdcMduserApi::CreateFtdcMduserApi();
    api_->RegisterSpi(this);

    api_->SubscribeMarketDataTopic(topic, USTP_TERT_QUICK);
    // set front address
    BOOST_FOREACH(const std::string &v, logon_cfg.quote_provider_addrs)
    {
        char *addr_tmp = new char[v.size() + 1];
        memcpy(addr_tmp, v.c_str(), v.size() + 1);
        api_->RegisterFront(addr_tmp);
        MY_LOG_INFO("femas - RegisterFront, addr: %s", addr_tmp);
        delete[] addr_tmp;
    }

    api_->Init();
}

MYFEMASDataHandler::~MYFEMASDataHandler(void)
{
    if (api_)
    {
        api_->RegisterSpi(NULL);
        api_->Release();
        api_ = NULL;
    }
}

void MYFEMASDataHandler::RequestLogin()
{
    CUstpFtdcReqUserLoginField req_login;
    memset(&req_login, 0, sizeof(CUstpFtdcReqUserLoginField));
    strncpy(req_login.BrokerID, cfg_.Logon_config().broker_id.c_str(), sizeof(TUstpFtdcBrokerIDType));
    strncpy(req_login.UserID, cfg_.Logon_config().account.c_str(), sizeof(TUstpFtdcUserIDType));
    strncpy(req_login.Password, cfg_.Logon_config().password.c_str(), sizeof(TUstpFtdcPasswordType));
    api_->ReqUserLogin(&req_login, 0);

    MY_LOG_INFO("femas - request logon");
}

void MYFEMASDataHandler::OnFrontConnected()
{
    MY_LOG_INFO("femas - OnFrontConnected");
    RequestLogin();
}

void MYFEMASDataHandler::OnFrontDisconnected(int nReason)
{
    logoned_ = false;
    MY_LOG_ERROR("femas - OnFrontDisconnected, nReason: %d", nReason);
}

void MYFEMASDataHandler::OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
{
    int error_code = pRspInfo ? pRspInfo->ErrorID : 0;
    MY_LOG_INFO("femas - OnRspUserLogin, error code: %d", error_code);

    if (error_code == 0)
    {
        logoned_ = true;
        api_->SubMarketData(pp_instruments_, sub_count_);
        MY_LOG_INFO("femas - SubMarketData, codelist: %s", instruments_.c_str());
    }
    else
    {
        //MonitorClient::instance()->SendMonitorMessage('S', 1, "login failed");
        MY_LOG_WARN("femas - Logon fail, error code: %d", error_code);
    }
}

void MYFEMASDataHandler::OnRspSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument, CUstpFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    MY_LOG_DEBUG("femas - OnRspSubMarketData, code: %s", pSpecificInstrument->InstrumentID);
}

void MYFEMASDataHandler::OnRspUnSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument,
    CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    MY_LOG_DEBUG("femas - OnRspUnSubMarketData, code: %s", pSpecificInstrument->InstrumentID);
}

void MYFEMASDataHandler::OnRtnDepthMarketData(CUstpFtdcDepthMarketDataField *p)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);

        RalaceInvalidValue_Femas(*p);
        CDepthMarketDataField q_level1 = Convert(*p);

        if (quote_data_handler_)
        {
            quote_data_handler_(&q_level1);
        }
    }
    catch (...)
    {
        MY_LOG_FATAL("femas - Unknown exception in OnRtnDepthMarketData.");
    }

}

void MYFEMASDataHandler::OnRspError(CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int error_code = pRspInfo ? 0 : pRspInfo->ErrorID;
    if (error_code != 0)
    {
        MY_LOG_INFO("femas - OnRspError, code: %d; info: %s", error_code, pRspInfo->ErrorMsg);
    }
}

void MYFEMASDataHandler::SetQuoteDataHandler(boost::function<void(const CDepthMarketDataField *)> quote_data_handler)
{
    quote_data_handler_ = quote_data_handler;
}
void MYFEMASDataHandler::RalaceInvalidValue_Femas(CUstpFtdcDepthMarketDataField &d)
{
    d.Turnover = InvalidToZeroD(d.Turnover);
    d.LastPrice = InvalidToZeroD(d.LastPrice);
    d.UpperLimitPrice = InvalidToZeroD(d.UpperLimitPrice);
    d.LowerLimitPrice = InvalidToZeroD(d.LowerLimitPrice);
    d.HighestPrice = InvalidToZeroD(d.HighestPrice);
    d.LowestPrice = InvalidToZeroD(d.LowestPrice);
    d.OpenPrice = InvalidToZeroD(d.OpenPrice);
    d.ClosePrice = InvalidToZeroD(d.ClosePrice);
    d.PreClosePrice = InvalidToZeroD(d.PreClosePrice);
    d.OpenInterest = InvalidToZeroD(d.OpenInterest);
    d.PreOpenInterest = InvalidToZeroD(d.PreOpenInterest);

    d.BidPrice1 = InvalidToZeroD(d.BidPrice1);
//    d.BidPrice2 = InvalidToZeroD(d.BidPrice2);
//    d.BidPrice3 = InvalidToZeroD(d.BidPrice3);
//    d.BidPrice4 = InvalidToZeroD(d.BidPrice4);
//    d.BidPrice5 = InvalidToZeroD(d.BidPrice5);

    d.AskPrice1 = InvalidToZeroD(d.AskPrice1);
//    d.AskPrice2 = InvalidToZeroD(d.AskPrice2);
//    d.AskPrice3 = InvalidToZeroD(d.AskPrice3);
//    d.AskPrice4 = InvalidToZeroD(d.AskPrice4);
//    d.AskPrice5 = InvalidToZeroD(d.AskPrice5);

    d.SettlementPrice = InvalidToZeroD(d.SettlementPrice);
    d.PreSettlementPrice = InvalidToZeroD(d.PreSettlementPrice);

//    d.PreDelta = InvalidToZeroD(d.PreDelta);
//    d.CurrDelta = InvalidToZeroD(d.CurrDelta);
}

CDepthMarketDataField MYFEMASDataHandler::Convert(const CUstpFtdcDepthMarketDataField &femas_data)
{
    CDepthMarketDataField quote_level1;
    memset(&quote_level1, 0, sizeof(CDepthMarketDataField));

    memcpy(quote_level1.TradingDay, femas_data.TradingDay, 9); /// char       TradingDay[9];
    //SettlementGroupID[9];       /// char       SettlementGroupID[9];
    //SettlementID ;        /// int            SettlementID;
    quote_level1.LastPrice = femas_data.LastPrice;           /// double LastPrice;
    quote_level1.PreSettlementPrice = femas_data.PreSettlementPrice;  /// double PreSettlementPrice;
    quote_level1.PreClosePrice = femas_data.PreClosePrice;       /// double PreClosePrice;
    quote_level1.PreOpenInterest = femas_data.PreOpenInterest;     /// double PreOpenInterest;
    quote_level1.OpenPrice = femas_data.OpenPrice;           /// double OpenPrice;
    quote_level1.HighestPrice = femas_data.HighestPrice;        /// double HighestPrice;
    quote_level1.LowestPrice = femas_data.LowestPrice;         /// double LowestPrice;
    quote_level1.Volume = femas_data.Volume;              /// int            Volume;
    quote_level1.Turnover = femas_data.Turnover;            /// double Turnover;
    quote_level1.OpenInterest = femas_data.OpenInterest;        /// double OpenInterest;
    quote_level1.ClosePrice = femas_data.ClosePrice;          /// double ClosePrice;
    quote_level1.SettlementPrice = femas_data.SettlementPrice;     /// double SettlementPrice;
    quote_level1.UpperLimitPrice = femas_data.UpperLimitPrice;     /// double UpperLimitPrice;
    quote_level1.LowerLimitPrice = femas_data.LowerLimitPrice;     /// double LowerLimitPrice;
//    quote_level1.PreDelta = femas_data.PreDelta;            /// double PreDelta;
//    quote_level1.CurrDelta = femas_data.CurrDelta;           /// double CurrDelta;
    memcpy(quote_level1.UpdateTime, femas_data.UpdateTime, 9);       /// char       UpdateTime[9]; typedef char TThostFtdcTimeType[9];
    quote_level1.UpdateMillisec = femas_data.UpdateMillisec;      /// int            UpdateMillisec;
    memcpy(quote_level1.InstrumentID, femas_data.InstrumentID, 31); /// char       InstrumentID[31]; typedef char TThostFtdcInstrumentIDType[31];
    quote_level1.BidPrice1 = femas_data.BidPrice1;           /// double BidPrice1;
    quote_level1.BidVolume1 = femas_data.BidVolume1;          /// int            BidVolume1;
    quote_level1.AskPrice1 = femas_data.AskPrice1;           /// double AskPrice1;
    quote_level1.AskVolume1 = femas_data.AskVolume1;          /// int            AskVolume1;
//    quote_level1.BidPrice2 = femas_data.BidPrice2;           /// double BidPrice2;
//    quote_level1.BidVolume2 = femas_data.BidVolume2;          /// int            BidVolume2;
//    quote_level1.AskPrice2 = femas_data.AskPrice2;           /// double AskPrice2;
//    quote_level1.AskVolume2 = femas_data.AskVolume2;          /// int            AskVolume2;
//    quote_level1.BidPrice3 = femas_data.BidPrice3;           /// double BidPrice3;
//    quote_level1.BidVolume3 = femas_data.BidVolume3;          /// int            BidVolume3;
//    quote_level1.AskPrice3 = femas_data.AskPrice3;           /// double AskPrice3;
//    quote_level1.AskVolume3 = femas_data.AskVolume3;          /// int            AskVolume3;
//    quote_level1.BidPrice4 = femas_data.BidPrice4;           /// double BidPrice4;
//    quote_level1.BidVolume4 = femas_data.BidVolume4;          /// int            BidVolume4;
//    quote_level1.AskPrice4 = femas_data.AskPrice4;           /// double AskPrice4;
//    quote_level1.AskVolume4 = femas_data.AskVolume4;          /// int            AskVolume4;
//    quote_level1.BidPrice5 = femas_data.BidPrice5;           /// double BidPrice5;
//    quote_level1.BidVolume5 = femas_data.BidVolume5;          /// int            BidVolume5;
//    quote_level1.AskPrice5 = femas_data.AskPrice5;           /// double AskPrice5;
//    quote_level1.AskVolume5 = femas_data.AskVolume5;          /// int            AskVolume5;
    //ActionDay[9];        /// char       ActionDay[9];

    return quote_level1;
}
