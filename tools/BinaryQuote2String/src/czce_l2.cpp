#include "czce_l2.h"
#include <string>
#include "util.h"

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

std::string CZCE_L2_QuoteToString(const SaveData_CZCE_L2_DATA * const p_data)
{
	if (!p_data)
	{
		return "";
	}

	const ZCEL2QuotSnapshotField_MY * const p = &(p_data->data_);	
	char total_quote[2048];

	// 当前的行情序列化
	std::string csTemp = p->TimeStamp; // 2014-02-03 13:23:45.180
	boost::erase_all(csTemp, ":");
	boost::erase_all(csTemp, ".");
	csTemp = csTemp.substr(11);


	double total_amount = 0;
	double avg_price = p->AveragePrice;

	// 当前量, 当前成交金额
	int cur_volumn = 0;
	double cur_amount = 0;
	//InstrumentToVolumnAndAmountMap::iterator it = instrument_to_quoteinfo_.find(p->InstrumentID);
	//if (it != instrument_to_quoteinfo_.end())
	//{
	//	cur_volumn = p->Volume - it->second.first;
	//	cur_amount = total_amount - it->second.second;
	//	it->second = std::make_pair(p->Volume, total_amount);
	//}
	//else
	//{
	//	cur_volumn = p->Volume;
	//	cur_amount = total_amount;
	//	instrument_to_quoteinfo_.insert(std::make_pair(p->InstrumentID,
	//		std::make_pair(p->Volume, total_amount)));
	//}

	std::string trade_day = p->TimeStamp;
	boost::erase_all(trade_day, "-");
	trade_day = trade_day.substr(0, 8);


	//R|预留|预留|时间|交易所代码|合约代码|成交价|累计成交量|买一价|买一量|卖一价|卖一量|持仓量|昨结算价|当前量
	//|买二价|买三价|买四价|买五价|买二量|买三量|买四量|买五量
	//|卖二价|卖三价|卖四价|卖五价|卖二量|卖三量|卖四量|卖五量
	//|涨停价|跌停价|当日开盘价|当日最高|当日最低|昨收盘|昨持仓|当日成交均价
	//|累计成交金额|当前成交金额\n

	sprintf(total_quote,
		"R|%s||%s|C|%s|%.4f|%d|%.4f|%d|%.4f|%d|%.0f|%.4f|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.0f|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%s"
		"|||%.4f|%d|%.6f|%.6f"
		"|%s||||||"
		"||||||"
		"%lld|",
		ConvertToShortTimval(p_data->t_).c_str(),
		csTemp.c_str(),                       // 时间
		// 交易所编码
		p->ContractID,                                  // 合约代码
		InvalidToZeroD(p->LastPrice),            // 成交价
		p->TotalVolume,                            // 成交量（总量）
		InvalidToZeroD(p->BidPrice[0]),            // 买一价
		p->BidLot[0],                        // 买一量
		InvalidToZeroD(p->AskPrice[0]),            // 卖一价
		p->AskLot[0],                        // 卖一量
		InvalidToZeroD(p->OpenInterest),         // 持仓量
		InvalidToZeroD(p->PreSettle),   // 昨结算价
		cur_volumn,                           // 当前量

		InvalidToZeroD(p->BidPrice[1]),            // 买二价
		InvalidToZeroD(p->BidPrice[2]),            // 买三价
		InvalidToZeroD(p->BidPrice[3]),            // 买四价
		InvalidToZeroD(p->BidPrice[4]),            // 买五价
		p->BidLot[1],                        // 买二量
		p->BidLot[2],                        // 买三量
		p->BidLot[3],                        // 买四量
		p->BidLot[4],                        // 买五量

		InvalidToZeroD(p->AskPrice[1]),            // 卖二价
		InvalidToZeroD(p->AskPrice[2]),            // 卖三价
		InvalidToZeroD(p->AskPrice[3]),            // 卖四价
		InvalidToZeroD(p->AskPrice[4]),            // 卖五价
		p->AskLot[1],                        // 卖二量
		p->AskLot[2],                        // 卖三量
		p->AskLot[3],                        // 卖四量
		p->AskLot[4],                        // 卖五量

		InvalidToZeroD(p->HighLimit),      // 涨停价
		InvalidToZeroD(p->LowLimit),      // 跌停价
		InvalidToZeroD(p->OpenPrice),            // 开盘
		InvalidToZeroD(p->HighPrice),         // 当日最高
		InvalidToZeroD(p->LowPrice),          // 当日最低
		InvalidToZeroD(p->PreClose),        // 昨收
		InvalidToZeroD(p->PreOpenInterest),      // 昨持仓
		avg_price,                            // 均价

		// 20131015，新增两个字段
		total_amount,                         // 累计成交金额
		cur_amount,                           // 当前成交金额

		0.0, //涨跌
		0.0, //涨跌幅
		0.0, //档差
		InvalidToZeroD(p->SettlePrice), //本次结算价
		trade_day.c_str(), //交易日期

		//结算组代码
		//结算编号
		InvalidToZeroD(p->ClosePrice), //今收盘
		0, //成交笔数
		0.0, //昨虚实度
		0.0, //今虚实度

		"", //合约在交易所的代码
		//停牌标志
		//持仓量变化
		//历史最低价
		//历史最高价
		//申买推导量
		//申卖推导量

		//delta（期权用）
		//gama（期权用）
		//rho（期权用）
		//theta（期权用）
		//vega（期权用）
		p_data->t_
		);

	return total_quote;
}

std::string CZCE_CMB_QuoteToString( const SaveData_CZCE_CMB_DATA * const p_data )
{
	char total_quote[2048];

	const ZCEQuotCMBQuotField_MY * const p = &(p_data->data_);	

	sprintf(total_quote, "%s|%s|%c|"
		"%.4f|%d|%d|%.4f|%d|%d|%lld|",
		p->TimeStamp, p->ContractID, p->CmbType,
		p->BidPrice,p->BidLot, p->VolBidLot, p->AskPrice, p->AskLot, p->VolAskLot,p_data->t_);

	return total_quote;
}
