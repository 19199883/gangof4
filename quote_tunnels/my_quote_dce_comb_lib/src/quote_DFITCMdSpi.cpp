#include <iomanip>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "quote_DFITCMdSpi.h"

using namespace my_cmn;
using namespace std;

MyMdSpi::MyMdSpi(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_save_(NULL)
{
    if (subscribe_contracts) { subscribe_contracts_ = *subscribe_contracts; }

    // 初始化
    logoned_ = false;
    api_ = NULL;

    p_save_ = new QuoteDataSave<CDepthMarketDataField>(cfg_, qtm_name, "quote_level1", SHFE_EX_QUOTE_TYPE);
    const SubsribeDatas &code_list = cfg_.Subscribe_datas();
    const LogonConfig &logon_cfg = cfg_.Logon_config();

    pp_instruments_ = NULL;
    sub_count_ = 0;
	// TODO: wangying, coding, using shell script to update subscription content of config
    // 解析订阅列表
	sub_count_ = code_list.size();
	pp_instruments_ = new char *[code_list.size()];
	int i = 0;
	for (const std::string &value : code_list) {
		instruments_.append(value + "|");
		pp_instruments_[i] = new char[value.length() + 1];
		memcpy(pp_instruments_[i], value.c_str(), value.length() + 1);
		++i;
	}

    if (!instruments_.empty()) { instruments_.pop_back(); }

    // 初始化
    api_ = DFITCMdApi::CreateDFITCMdApi();

    // set front address
    for (const std::string &v : logon_cfg.quote_provider_addrs){
        char *addr_tmp = new char[v.size() + 1];
        memcpy(addr_tmp, v.c_str(), v.size() + 1);
        int err = api_->Init(addr_tmp, this);
        MY_LOG_INFO("xSpeed - Init addr: %s, err:%d", addr_tmp,err);
        delete[] addr_tmp;
    }
}

MyMdSpi::~MyMdSpi(void)
{
    if (api_) {
        api_->Release();
        api_ = NULL;
    }

    if (p_save_) delete p_save_;
}

void MyMdSpi::OnFrontConnected()
{
    MY_LOG_INFO("(xSpeed): OnFrontConnected");

    struct DFITCUserLoginField req_login;
    memset(&req_login, 0, sizeof(req_login));
    req_login.lRequestID = 1;
    strncpy(req_login.accountID, cfg_.Logon_config().account.c_str(), sizeof(DFITCAccountIDType));
    strncpy(req_login.passwd, cfg_.Logon_config().password.c_str(), sizeof(DFITCPasswdType));
    api_->ReqUserLogin(&req_login);

    MY_LOG_INFO("xSpeed - request logon");
}

void MyMdSpi::OnFrontDisconnected(int nReason)
{
    logoned_ = false;
    MY_LOG_ERROR("xSpeed - OnFrontDisconnected, nReason: %d", nReason);
}

void MyMdSpi::OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pRspUserLogin, struct DFITCErrorRtnField * pRspInfo)
{
    int error_code = pRspInfo ? pRspInfo->nErrorID : 0;
    MY_LOG_INFO("xSpeed - OnRspUserLogin, error code: %d", error_code);

    if (error_code == 0) {
        logoned_ = true;
        int err = api_->SubscribeMarketData(pp_instruments_, sub_count_,1);
        MY_LOG_INFO("xSpeed - SubMarketData, codelist: %s, err:%d", instruments_.c_str(),err);
    } else {
        std::string err_str("null");
        if (pRspInfo && pRspInfo->errorMsg[0] != '\0') { err_str = pRspInfo->errorMsg; }
        MY_LOG_WARN("xSpeed - Logon fail, error code: %d; error info: %s", error_code, err_str.c_str());
    }
}


void MyMdSpi::OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pRspUsrLogout, struct DFITCErrorRtnField * pRspInfo)
{
    int error_code = pRspInfo ? pRspInfo->nErrorID : 0;
    MY_LOG_INFO("xSpeed - OnRspUserLogout, error code: %d", error_code);

    if (error_code == 0) {
        logoned_ = false;
    } else {
        std::string err_str("null");
        if (pRspInfo && pRspInfo->errorMsg[0] != '\0') {
            err_str = pRspInfo->errorMsg;
        }
        MY_LOG_WARN("xSpeed - Logout fail, error code: %d; error info: %s", error_code, err_str.c_str());
    }
}
void MyMdSpi::OnRspSubMarketData(struct DFITCSpecificInstrumentField *pSpecificInstrument, struct DFITCErrorRtnField *pErrorInfo)
{
    MY_LOG_DEBUG("xSpeed - OnRspSubMarketData");
    //MY_LOG_DEBUG("xSpeed - OnRspSubMarketData, code: %s, errorID: %d,errMsg:%s", pSpecificInstrument->InstrumentID, pErrorInfo->nErrorID, pErrorInfo->errorMsg);
}

void MyMdSpi::OnRspUnSubMarketData(struct DFITCSpecificInstrumentField * pSpecificInstrument, struct DFITCErrorRtnField * pRspInfo)
{
    MY_LOG_DEBUG("xSpeed - OnRspUnSubMarketData, code: %s,errorID:%d,errorMsg:%s", pSpecificInstrument->InstrumentID, pRspInfo->nErrorID,pRspInfo->errorMsg);
}

void MyMdSpi::OnMarketData(struct DFITCDepthMarketDataField * pMarketDataField)
{
	// TODO: test, commented by wangying on 2017-4-12
    MY_LOG_DEBUG("xSpeed - OnMarketData:contract: %s,time:%s %d", pMarketDataField->instrumentID,pMarketDataField->UpdateTime,pMarketDataField->UpdateMillisec);
	/*
    try
    {
        timeval t;
        gettimeofday(&t, NULL);

        RalaceInvalidValue_CTP(*p);
        CDepthMarketDataField q_level1 = Convert(*p);

        if (quote_data_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->InstrumentID) != subscribe_contracts_.end()))
        {
            quote_data_handler_(&q_level1);
        }

        // 存起来
        p_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &q_level1);
    }
    catch (...)
    {
        MY_LOG_FATAL("CTP - Unknown exception in OnRtnDepthMarketData.");
    }
	*/
}

void MyMdSpi::OnRspError(struct DFITCErrorRtnField *pRspInfo)
{
    int error_code = pRspInfo ? 0 : pRspInfo->nErrorID;
    if (error_code != 0){
        MY_LOG_INFO("xSpeed - OnRspError, code: %d; info: %s", error_code, pRspInfo->errorMsg);
    }
}

void MyMdSpi::SetQuoteDataHandler(boost::function<void(const CDepthMarketDataField *)> quote_data_handler)
{
    quote_data_handler_ = quote_data_handler;
}

//void MyMdSpi::RalaceInvalidValue_CTP(CThostFtdcDepthMarketDataField &d)
//{
//    d.Turnover = InvalidToZeroD(d.Turnover);
//    d.LastPrice = InvalidToZeroD(d.LastPrice);
//    d.UpperLimitPrice = InvalidToZeroD(d.UpperLimitPrice);
//    d.LowerLimitPrice = InvalidToZeroD(d.LowerLimitPrice);
//    d.HighestPrice = InvalidToZeroD(d.HighestPrice);
//    d.LowestPrice = InvalidToZeroD(d.LowestPrice);
//    d.OpenPrice = InvalidToZeroD(d.OpenPrice);
//    d.ClosePrice = InvalidToZeroD(d.ClosePrice);
//    d.PreClosePrice = InvalidToZeroD(d.PreClosePrice);
//    d.OpenInterest = InvalidToZeroD(d.OpenInterest);
//    d.PreOpenInterest = InvalidToZeroD(d.PreOpenInterest);
//    d.AveragePrice = InvalidToZeroD(d.AveragePrice);
//
//    d.BidPrice1 = InvalidToZeroD(d.BidPrice1);
////    d.BidPrice2 = InvalidToZeroD(d.BidPrice2);
////    d.BidPrice3 = InvalidToZeroD(d.BidPrice3);
////    d.BidPrice4 = InvalidToZeroD(d.BidPrice4);
////    d.BidPrice5 = InvalidToZeroD(d.BidPrice5);
//
//    d.AskPrice1 = InvalidToZeroD(d.AskPrice1);
////    d.AskPrice2 = InvalidToZeroD(d.AskPrice2);
////    d.AskPrice3 = InvalidToZeroD(d.AskPrice3);
////    d.AskPrice4 = InvalidToZeroD(d.AskPrice4);
////    d.AskPrice5 = InvalidToZeroD(d.AskPrice5);
//
//    d.SettlementPrice = InvalidToZeroD(d.SettlementPrice);
//    d.PreSettlementPrice = InvalidToZeroD(d.PreSettlementPrice);
//
////    d.PreDelta = InvalidToZeroD(d.PreDelta);
////    d.CurrDelta = InvalidToZeroD(d.CurrDelta);
//}
//
//CDepthMarketDataField MyMdSpi::Convert(const CThostFtdcDepthMarketDataField &ctp_data)
//{
//    CDepthMarketDataField quote_level1;
//    memset(&quote_level1, 0, sizeof(CDepthMarketDataField));
//
//    memcpy(quote_level1.TradingDay, ctp_data.TradingDay, 9); /// char       TradingDay[9];
//    //SettlementGroupID[9];       /// char       SettlementGroupID[9];
//    //SettlementID ;        /// int            SettlementID;
//    quote_level1.LastPrice = ctp_data.LastPrice;           /// double LastPrice;
//    quote_level1.PreSettlementPrice = ctp_data.PreSettlementPrice;  /// double PreSettlementPrice;
//    quote_level1.PreClosePrice = ctp_data.PreClosePrice;       /// double PreClosePrice;
//    quote_level1.PreOpenInterest = ctp_data.PreOpenInterest;     /// double PreOpenInterest;
//    quote_level1.OpenPrice = ctp_data.OpenPrice;           /// double OpenPrice;
//    quote_level1.HighestPrice = ctp_data.HighestPrice;        /// double HighestPrice;
//    quote_level1.LowestPrice = ctp_data.LowestPrice;         /// double LowestPrice;
//    quote_level1.Volume = ctp_data.Volume;              /// int            Volume;
//    quote_level1.Turnover = ctp_data.Turnover;            /// double Turnover;
//    quote_level1.OpenInterest = ctp_data.OpenInterest;        /// double OpenInterest;
//    quote_level1.ClosePrice = ctp_data.ClosePrice;          /// double ClosePrice;
//    quote_level1.SettlementPrice = ctp_data.SettlementPrice;     /// double SettlementPrice;
//    quote_level1.UpperLimitPrice = ctp_data.UpperLimitPrice;     /// double UpperLimitPrice;
//    quote_level1.LowerLimitPrice = ctp_data.LowerLimitPrice;     /// double LowerLimitPrice;
////    quote_level1.PreDelta = ctp_data.PreDelta;            /// double PreDelta;
////    quote_level1.CurrDelta = ctp_data.CurrDelta;           /// double CurrDelta;
//    memcpy(quote_level1.UpdateTime, ctp_data.UpdateTime, 9);       /// char       UpdateTime[9]; typedef char TThostFtdcTimeType[9];
//    quote_level1.UpdateMillisec = ctp_data.UpdateMillisec;      /// int            UpdateMillisec;
//    memcpy(quote_level1.InstrumentID, ctp_data.InstrumentID, 31); /// char       InstrumentID[31]; typedef char TThostFtdcInstrumentIDType[31];
//    quote_level1.BidPrice1 = ctp_data.BidPrice1;           /// double BidPrice1;
//    quote_level1.BidVolume1 = ctp_data.BidVolume1;          /// int            BidVolume1;
//    quote_level1.AskPrice1 = ctp_data.AskPrice1;           /// double AskPrice1;
//    quote_level1.AskVolume1 = ctp_data.AskVolume1;          /// int            AskVolume1;
////    quote_level1.BidPrice2 = ctp_data.BidPrice2;           /// double BidPrice2;
////    quote_level1.BidVolume2 = ctp_data.BidVolume2;          /// int            BidVolume2;
////    quote_level1.AskPrice2 = ctp_data.AskPrice2;           /// double AskPrice2;
////    quote_level1.AskVolume2 = ctp_data.AskVolume2;          /// int            AskVolume2;
////    quote_level1.BidPrice3 = ctp_data.BidPrice3;           /// double BidPrice3;
////    quote_level1.BidVolume3 = ctp_data.BidVolume3;          /// int            BidVolume3;
////    quote_level1.AskPrice3 = ctp_data.AskPrice3;           /// double AskPrice3;
////    quote_level1.AskVolume3 = ctp_data.AskVolume3;          /// int            AskVolume3;
////    quote_level1.BidPrice4 = ctp_data.BidPrice4;           /// double BidPrice4;
////    quote_level1.BidVolume4 = ctp_data.BidVolume4;          /// int            BidVolume4;
////    quote_level1.AskPrice4 = ctp_data.AskPrice4;           /// double AskPrice4;
////    quote_level1.AskVolume4 = ctp_data.AskVolume4;          /// int            AskVolume4;
////    quote_level1.BidPrice5 = ctp_data.BidPrice5;           /// double BidPrice5;
////    quote_level1.BidVolume5 = ctp_data.BidVolume5;          /// int            BidVolume5;
////    quote_level1.AskPrice5 = ctp_data.AskPrice5;           /// double AskPrice5;
////    quote_level1.AskVolume5 = ctp_data.AskVolume5;          /// int            AskVolume5;
//        //ActionDay[9];        /// char       ActionDay[9];
//
//    return quote_level1;
//}
