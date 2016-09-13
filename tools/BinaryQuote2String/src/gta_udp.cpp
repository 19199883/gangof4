#include "gta_udp.h"
#include "util.h"
#include <string>

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

std::string GTA_UDP_QuoteToString( const SaveData_GTA_UDP * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const CFfexFtdcDepthMarketData * const p = &(p_data->data_);	
	char buffer[64];
	char total_quote[2048];

	// 当前的行情序列化
	std::string csTemp = p->szUpdateTime;
	boost::erase_all(csTemp, ":");
	sprintf(buffer, "%03d", p->nUpdateMillisec);
	csTemp.append(buffer);


	double total_amount = InvalidToZeroD(p->dTurnover);
	double avg_price = 0;
	if (p->nVolume > 0)
	{
		avg_price = total_amount / p->nVolume;
	}

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

	std::string trade_day(p->szTradingDay, 8);

	//R|预留|预留|时间|交易所代码|合约代码|成交价|累计成交量|买一价|买一量|卖一价|卖一量|持仓量|昨结算价|当前量
	//|买二价|买三价|买四价|买五价|买二量|买三量|买四量|买五量
	//|卖二价|卖三价|卖四价|卖五价|卖二量|卖三量|卖四量|卖五量
	//|涨停价|跌停价|当日开盘价|当日最高|当日最低|昨收盘|昨持仓|当日成交均价
	//|累计成交金额|当前成交金额\n

	sprintf(total_quote,
		"R|%s||%s|G|%s|%.4f|%d|%.4f|%d|%.4f|%d|%.0f|%.4f|%d|"
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
		p->szInstrumentID,                                  // 合约代码
		InvalidToZeroD(p->dLastPrice),            // 成交价
		p->nVolume,                            // 成交量（总量）
		InvalidToZeroD(p->dBidPrice1),            // 买一价
		p->nBidVolume1,                        // 买一量
		InvalidToZeroD(p->dAskPrice1),            // 卖一价
		p->nAskVolume1,                        // 卖一量
		InvalidToZeroD(p->dOpenInterest),         // 持仓量
		InvalidToZeroD(p->dPreSettlementPrice),   // 昨结算价
		cur_volumn,                           // 当前量

		InvalidToZeroD(p->dBidPrice2),            // 买二价
		InvalidToZeroD(p->dBidPrice3),            // 买三价
		InvalidToZeroD(p->dBidPrice4),            // 买四价
		InvalidToZeroD(p->dBidPrice5),            // 买五价
		p->nBidVolume2,                        // 买二量
		p->nBidVolume3,                        // 买三量
		p->nBidVolume4,                        // 买四量
		p->nBidVolume5,                        // 买五量

		InvalidToZeroD(p->dAskPrice2),            // 卖二价
		InvalidToZeroD(p->dAskPrice3),            // 卖三价
		InvalidToZeroD(p->dAskPrice4),            // 卖四价
		InvalidToZeroD(p->dAskPrice5),            // 卖五价
		p->nAskVolume2,                        // 卖二量
		p->nAskVolume3,                        // 卖三量
		p->nAskVolume4,                        // 卖四量
		p->nAskVolume5,                        // 卖五量

		InvalidToZeroD(p->dUpperLimitPrice),      // 涨停价
		InvalidToZeroD(p->dLowerLimitPrice),      // 跌停价
		InvalidToZeroD(p->dOpenPrice),            // 开盘
		InvalidToZeroD(p->dHighestPrice),         // 当日最高
		InvalidToZeroD(p->dLowestPrice),          // 当日最低
		InvalidToZeroD(p->dPreClosePrice),        // 昨收
		InvalidToZeroD(p->dPreOpenInterest),      // 昨持仓
		avg_price,                            // 均价

		// 20131015，新增两个字段
		total_amount,                         // 累计成交金额
		cur_amount,                           // 当前成交金额

		0.0, //涨跌
		0.0, //涨跌幅
		0.0, //档差
		InvalidToZeroD(p->dSettlementPrice), //本次结算价
		trade_day.c_str(), //交易日期

		//结算组代码
		//结算编号
		InvalidToZeroD(p->dClosePrice), //今收盘
		0, //成交笔数
		InvalidToZeroD(p->dPreDelta), //昨虚实度
		InvalidToZeroD(p->dCurrDelta), //今虚实度

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
