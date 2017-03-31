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

CMdclientHandler::CMdclientHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg)
{
    // 初始化
    api_ = NULL;

    const LogonConfig &logon_cfg = cfg_.Logon_config();
    int topic = atoi(logon_cfg.topic.c_str());

	// TODO: wangying, here
//    BOOST_FOREACH(const std::string &v, logon_cfg.quote_provider_addrs) {
//		size_t ipstr_s = v.substr(v.find("//")+2;
//		string ip = 
//	}
    // 初始化
    //int listen_port = UDP_PORT; 
    //api_ = CMdclientApi::Create(pHand,listen_port);
    api_ = CMdclientApi::Create(this);//,10074);
	api_->Subscribe("ag1706");
    int err = api_->Start();
	MY_LOG_INFO("CMdclientApi start: %d",err);
}

CMdclientHandler::~CMdclientHandler(void)
{
    if (api_) {
		int err = api_->Stop();
		MY_LOG_INFO("CMdclientApi stop: %d",err);
        api_ = NULL;
    }
}

void CMdclientHandler::OnRtnDepthMarketData(CDepthMarketDataField *p)
{
    try {
        timeval t;
        gettimeofday(&t, NULL);

        RalaceInvalidValue_Femas(*p);
        CDepthMarketDataField q_level1 = *p;
        if (quote_data_handler_) { quote_data_handler_(&q_level1); }
    }
    catch (...) {
        MY_LOG_FATAL("CMdclientApi- Unknown exception in OnRtnDepthMarketData.");
    }

}

void CMdclientHandler::SetQuoteDataHandler(boost::function<void(const CDepthMarketDataField *)> quote_data_handler)
{
    quote_data_handler_ = quote_data_handler;
}
void CMdclientHandler::RalaceInvalidValue_Femas(CDepthMarketDataField &d)
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

