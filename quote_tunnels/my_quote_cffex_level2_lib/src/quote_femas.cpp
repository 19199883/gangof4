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
    : cfg_(cfg), p_save_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }
    // 初始化
    logoned_ = false;
    api_ = NULL;


    SubsribeDatas code_list = cfg_.Subscribe_datas();
    const LogonConfig &logon_cfg = cfg_.Logon_config();
    int topic = atoi(logon_cfg.topic.c_str());

    sprintf(qtm_name_, "cffex_femas_%s_%u", cfg_.Logon_config().account.c_str(), getpid());

    p_save_ = new QuoteDataSave<CFfexFtdcDepthMarketData>(cfg_, qtm_name_, "cffex_level2", GTA_UDP_CFFEX_QUOTE_TYPE);

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

void MYFEMASDataHandler::ReqLogin()
{
    sleep(6);

    CUstpFtdcReqUserLoginField req_login;
    memset(&req_login, 0, sizeof(CUstpFtdcReqUserLoginField));
    strncpy(req_login.BrokerID, cfg_.Logon_config().broker_id.c_str(), sizeof(TUstpFtdcBrokerIDType));
    strncpy(req_login.UserID, cfg_.Logon_config().account.c_str(), sizeof(TUstpFtdcUserIDType));
    strncpy(req_login.Password, cfg_.Logon_config().password.c_str(), sizeof(TUstpFtdcPasswordType));
    api_->ReqUserLogin(&req_login, 0);

    MY_LOG_INFO("femas - retry login");
}

void MYFEMASDataHandler::OnFrontConnected()
{
    MY_LOG_INFO("femas - OnFrontConnected");
    CUstpFtdcReqUserLoginField req_login;
    memset(&req_login, 0, sizeof(CUstpFtdcReqUserLoginField));
    strncpy(req_login.BrokerID, cfg_.Logon_config().broker_id.c_str(), sizeof(TUstpFtdcBrokerIDType));
    strncpy(req_login.UserID, cfg_.Logon_config().account.c_str(), sizeof(TUstpFtdcUserIDType));
    strncpy(req_login.Password, cfg_.Logon_config().password.c_str(), sizeof(TUstpFtdcPasswordType));
    api_->ReqUserLogin(&req_login, 0);

    MY_LOG_INFO("femas - request logon");
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
        int ret = api_->SubMarketData(pp_instruments_, sub_count_);
        MY_LOG_INFO("femas - SubMarketData, codelist: %s, result: %d", instruments_.c_str(), ret);
    }
    else
    {
        //MonitorClient::instance()->SendMonitorMessage('S', 1, "login failed");
        MY_LOG_WARN("femas - Logon fail, error code: %d; errmsg:%s", error_code, pRspInfo->ErrorMsg);

        // 登录失败，不断重试
        boost::thread(boost::bind(&MYFEMASDataHandler::ReqLogin, this));
    }
}

void MYFEMASDataHandler::OnRspSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument, CUstpFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    int error_code = pRspInfo ? pRspInfo->ErrorID : 0;
    MY_LOG_INFO("femas - OnRspSubMarketData, error code: %d", error_code);

    if (pSpecificInstrument)
    {
        MY_LOG_INFO("femas - OnRspSubMarketData, code: %s", pSpecificInstrument->InstrumentID);
    }
}

void MYFEMASDataHandler::OnRspUnSubMarketData(CUstpFtdcSpecificInstrumentField *pSpecificInstrument,
    CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pSpecificInstrument)
    {
        MY_LOG_INFO("femas - OnRspUnSubMarketData, code: %s", pSpecificInstrument->InstrumentID);
    }
}

void MYFEMASDataHandler::OnRtnDepthMarketData(CUstpFtdcDepthMarketDataField *p)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);

        RalaceInvalidValue_Femas(*p);
        CFfexFtdcDepthMarketData q_cffex_level2 = Convert(*p);

        if (quote_data_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->InstrumentID) != subscribe_contracts_.end()))
        {
            quote_data_handler_(&q_cffex_level2);
        }

        // 存起来
        p_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &q_cffex_level2);
        MY_LOG_DEBUG("femas - OnRtnDepthMarketData, code: %s", p->InstrumentID);
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

void MYFEMASDataHandler::SetQuoteDataHandler(boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler)
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
    d.BidPrice2 = InvalidToZeroD(d.BidPrice2);
    d.BidPrice3 = InvalidToZeroD(d.BidPrice3);
    d.BidPrice4 = InvalidToZeroD(d.BidPrice4);
    d.BidPrice5 = InvalidToZeroD(d.BidPrice5);

    d.AskPrice1 = InvalidToZeroD(d.AskPrice1);
    d.AskPrice2 = InvalidToZeroD(d.AskPrice2);
    d.AskPrice3 = InvalidToZeroD(d.AskPrice3);
    d.AskPrice4 = InvalidToZeroD(d.AskPrice4);
    d.AskPrice5 = InvalidToZeroD(d.AskPrice5);

    d.SettlementPrice = InvalidToZeroD(d.SettlementPrice);
    d.PreSettlementPrice = InvalidToZeroD(d.PreSettlementPrice);

    d.PreDelta = InvalidToZeroD(d.PreDelta);
    d.CurrDelta = InvalidToZeroD(d.CurrDelta);
}

CFfexFtdcDepthMarketData MYFEMASDataHandler::Convert(const CUstpFtdcDepthMarketDataField &femas_data)
{
    CFfexFtdcDepthMarketData q2;
    memset(&q2, 0, sizeof(CFfexFtdcDepthMarketData));

    memcpy(q2.szTradingDay, femas_data.TradingDay, 9); // typedef char TUstpFtdcDateType[9];
    memcpy(q2.szSettlementGroupID, femas_data.SettlementGroupID, 9); // typedef char TUstpFtdcSettlementGroupIDType[9];
    q2.nSettlementID = femas_data.SettlementID;
    q2.dLastPrice = femas_data.LastPrice;
    q2.dPreSettlementPrice = femas_data.PreSettlementPrice;
    q2.dPreClosePrice = femas_data.PreClosePrice;
    q2.dPreOpenInterest = femas_data.PreOpenInterest;
    q2.dOpenPrice = femas_data.OpenPrice;
    q2.dHighestPrice = femas_data.HighestPrice;
    q2.dLowestPrice = femas_data.LowestPrice;
    q2.nVolume = femas_data.Volume;
    q2.dTurnover = femas_data.Turnover;
    q2.dOpenInterest = femas_data.OpenInterest;
    q2.dClosePrice = femas_data.ClosePrice;
    q2.dSettlementPrice = femas_data.SettlementPrice;
    q2.dUpperLimitPrice = femas_data.UpperLimitPrice;
    q2.dLowerLimitPrice = femas_data.LowerLimitPrice;
    q2.dPreDelta = femas_data.PreDelta;
    q2.dCurrDelta = femas_data.CurrDelta;
    memcpy(q2.szUpdateTime, femas_data.UpdateTime, 9); // typedef char TUstpFtdcTimeType[9];
    q2.nUpdateMillisec = femas_data.UpdateMillisec;
    memcpy(q2.szInstrumentID, femas_data.InstrumentID, 31); // typedef char TUstpFtdcInstrumentIDType[31];
    q2.dBidPrice1 = femas_data.BidPrice1;
    q2.nBidVolume1 = femas_data.BidVolume1;
    q2.dAskPrice1 = femas_data.AskPrice1;
    q2.nAskVolume1 = femas_data.AskVolume1;
    q2.dBidPrice2 = femas_data.BidPrice2;
    q2.nBidVolume2 = femas_data.BidVolume2;
    q2.dAskPrice2 = femas_data.AskPrice2;
    q2.nAskVolume2 = femas_data.AskVolume2;
    q2.dBidPrice3 = femas_data.BidPrice3;
    q2.nBidVolume3 = femas_data.BidVolume3;
    q2.dAskPrice3 = femas_data.AskPrice3;
    q2.nAskVolume3 = femas_data.AskVolume3;
    q2.dBidPrice4 = femas_data.BidPrice4;
    q2.nBidVolume4 = femas_data.BidVolume4;
    q2.dAskPrice4 = femas_data.AskPrice4;
    q2.nAskVolume4 = femas_data.AskVolume4;
    q2.dBidPrice5 = femas_data.BidPrice5;
    q2.nBidVolume5 = femas_data.BidVolume5;
    q2.dAskPrice5 = femas_data.AskPrice5;
    q2.nAskVolume5 = femas_data.AskVolume5;

    return q2;
}
