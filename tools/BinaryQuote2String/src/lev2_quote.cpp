#include "lev2_quote.h"
#include <string>
#include "util.h"

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

std::string Lev2QuoteToString(const SaveData_Lev2Data* const p)
{
	if (!p_data)
	{
		return "";
	}

	const CThostFtdcDepthMarketDataField * const p = &(p_data->data_);	
	
	char buf[5120];
	sprintf (buf,
		"CThostFtdcDepthMarketDataField  "
		"InstrumentID:%s; "
		"UpdateTime[9]:%s; "
		"UpdateMillisec:%d; "
		"TradingDay:%s; "
		"LastPrice:%.4f; "
		"PreSettlementPrice:%.4f; "
		"PreClosePrice:%.4f; "
		"PreOpenInterest:%.4f; "
		"OpenPrice:%.4f; "
		"HighestPrice:%.4f; "
		"LowestPrice:%.4f; "
		"Volume:%d; "
		"Turnover:%.4f; "
		"OpenInterest:%.4f; "
		"ClosePrice:%.4f; "
		"SettlementPrice:%.4f; "
		"UpperLimitPrice:%.4f; "
		"LowerLimitPrice:%.4f; "
		"PreDelta:%.4f; "
		"CurrDelta:%.4f; "
		"BidPrice1:%.4f; "
		"BidVolume1:%d; "
		"AskPrice1:%.4f; "
		"AskVolume1:%d; "
		"BidPrice2:%.4f; "
		"BidVolume2:%d; "
		"AskPrice2:%.4f; "
		"AskVolume2:%d; "
		"BidPrice3:%.4f; "
		"BidVolume3:%d; "
		"AskPrice3:%.4f; "
		"AskVolume3:%d; "
		"BidPrice4:%.4f; "
		"BidVolume4:%d; "
		"AskPrice4:%.4f; "
		"AskVolume4:%d; "
		"BidPrice5:%.4f; "
		"BidVolume5:%d; "
		"AskPrice5:%.4f; "
		"AskVolume5:%d; "
		"ActionDay:%s;",
		source.InstrumentID,
		source.UpdateTime,
		source.UpdateMillisec,
		source.TradingDay,
		source.LastPrice,
		source.PreSettlementPrice,
		source. PreClosePrice,
		source.PreOpenInterest,
		source.OpenPrice,
		source. HighestPrice,
		source. LowestPrice,
		source.Volume,
		source.Turnover,
		source.OpenInterest,
		source.ClosePrice,
		source.SettlementPrice,
		source.UpperLimitPrice,
		source.LowerLimitPrice,
		source.PreDelta,
		source.CurrDelta,
		source.BidPrice1,
		source.BidVolume1,
		source.AskPrice1,
		source.AskVolume1,
		source.BidPrice2,
		source.BidVolume2,
		source.AskPrice2,
		source.AskVolume2,
		source.BidPrice3,
		source.BidVolume3,
		source.AskPrice3,
		source.AskVolume3,
		source.BidPrice4,
		source.BidVolume4,
		source.AskPrice4,
		source.AskVolume4,
		source.BidPrice5,
		source.BidVolume5,
		source.AskPrice5,
		source.AskVolume5,
		source.ActionDay);
	return buf
}

