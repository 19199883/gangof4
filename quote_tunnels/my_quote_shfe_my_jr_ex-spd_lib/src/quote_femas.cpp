﻿#include "quote_femas.h"
#include <iomanip>
#include <vector>
#include <iostream>     // std::cin, std::cout

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

	int port = -1;
	string ip = "";
    for(const std::string &v : logon_cfg.quote_provider_addrs) {
		size_t ipstr_start = v.find("//")+2;
		size_t ipstr_end = v.find(":",ipstr_start);
		ip = v.substr (ipstr_start, ipstr_end-ipstr_start);
		port = stoi(v.substr(ipstr_end+1));

	}
	char *ip_c_str = (char*)ip.c_str();
    api_ = CMdclientApi::Create(this,port,ip_c_str);
	MY_LOG_INFO("CMdclientApi ip:%s, port:%d",ip.c_str(),port);

	string contr_file = "";
    SubsribeDatas code_list = cfg_.Subscribe_datas();
    BOOST_FOREACH(const std::string &value, code_list){ contr_file = value; }
	std::ifstream is;
	is.open (contr_file);
	string contrs = "";
	if (is) {
		getline(is, contrs);
		size_t start_pos = 0;
		size_t end_pos = 0;
		string contr = "";
		while ((end_pos=contrs.find(" ",start_pos)) != string::npos){
			contr = contrs.substr (start_pos, end_pos-start_pos);
			api_->Subscribe((char*)contr.c_str());
			MY_LOG_INFO("CMdclientApi subscribe:%s",contr.c_str());
			start_pos = end_pos + 1;
		}
		if(contr.size()>0){
			string contr = contrs.substr (start_pos);
			api_->Subscribe((char*)contr.c_str());
			MY_LOG_INFO("CMdclientApi subscribe:%s",contr.c_str());
		}
	}
	else { MY_LOG_INFO("CMdclientApi can't open: %s",contr_file.c_str()); }

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

void CMdclientHandler::ToString(CDepthMarketDataField &data)
{
	MY_LOG_INFO("CDepthMarketDataField\n"
		"TradingDay:%s\n"
		"SettlementGroupID:%s\n"
		"SettlementID:%d\n"
		"LastPrice:%f \n"
		"PreSettlementPrice:%f \n"
		"PreClosePrice:%f \n"
		"PreOpenInterest:%f \n"
		"OpenPrice:%f \n"
		"HighestPrice:%f \n"
		"LowestPrice:%f \n"
		"Volume:%d \n"
		"Turnover:%f \n"
		"OpenInterest:%f \n"
		"ClosePrice:%f \n"
		"SettlementPrice:%f \n"
		"UpperLimitPrice:%f \n"
		"LowerLimitPrice:%f \n"
		"PreDelta:%f \n"
		"CurrDelta:%f \n"
		"UpdateTime[9]:%s \n"
		"UpdateMillisec:%d \n"
		"InstrumentID:%s \n"
		"BidPrice1:%f \n"
		"BidVolume1:%d \n"
		"AskPrice1:%f \n"
		"AskVolume1:%d \n"
		"BidPrice2:%f \n"
		"BidVolume2:%d \n"
		"AskPrice2:%f \n"
		"AskVolume2:%d \n"
		"BidPrice3:%f \n"
		"BidVolume3:%d \n"
		"AskPrice3:%f \n"
		"AskVolume3:%d \n"
		"BidPrice4:%f \n"
		"BidVolume4:%d \n"
		"AskPrice4:%f \n"
		"AskVolume4:%d \n"
		"BidPrice5:%f \n"
		"BidVolume5:%d \n"
		"AskPrice5:%f \n"
		"AskVolume5:%d \n"
		"ActionDay:%s \n",
		data.TradingDay,
		data.SettlementGroupID,
		data.SettlementID,
		data.LastPrice,
		data.PreSettlementPrice,
		data. PreClosePrice,
		data.PreOpenInterest,
		data.OpenPrice,
		data. HighestPrice,
		data. LowestPrice,
		data.Volume,
		data.Turnover,
		data.OpenInterest,
		data.ClosePrice,
		data.SettlementPrice,
		data.UpperLimitPrice,
		data.LowerLimitPrice,
		data.PreDelta,
		data.CurrDelta,
		data.UpdateTime,
		data.UpdateMillisec,
		data.InstrumentID,
		data.BidPrice1,
		data.BidVolume1,
		data.AskPrice1,
		data.AskVolume1,
		data.BidPrice2,
		data.BidVolume2,
		data.AskPrice2,
		data.AskVolume2,
		data.BidPrice3,
		data.BidVolume3,
		data.AskPrice3,
		data.AskVolume3,
		data.BidPrice4,
		data.BidVolume4,
		data.AskPrice4,
		data.AskVolume4,
		data.BidPrice5,
		data.BidVolume5,
		data.AskPrice5,
		data.AskVolume5,
		data.ActionDay);

}
void CMdclientHandler::SetQuoteDataHandler(std::function<void(const CDepthMarketDataField *)> quote_data_handler)
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

