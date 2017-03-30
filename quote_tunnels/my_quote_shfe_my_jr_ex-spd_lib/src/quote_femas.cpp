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

	// TODO: wangying, here
    // 初始化
    //int listen_port = UDP_PORT; 
    //api_ = CMdclientApi::Create(pHand,listen_port);
    api_ = CMdclientApi::Create(this);//,10074);
	api_->Subscribe("ag1706");
    api_->Start();
}

CMdclientHandler::~CMdclientHandler(void)
{
    if (api_) {
		api_->Stop();
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
        MY_LOG_FATAL("femas - Unknown exception in OnRtnDepthMarketData.");
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

