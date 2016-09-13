#include "ctp.h"
#include <string>

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
std::string CTPQuoteToString( const SaveData_CTP * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const CThostFtdcDepthMarketDataField * const p = &(p_data->data_);	
	
	char buffer[64];
	char total_quote[2048];

	std::string quote_time = p->UpdateTime;
	boost::erase_all(quote_time, ":");
	sprintf(buffer, "%03d", p->UpdateMillisec);
	quote_time.append(buffer);

	double total_amount = InvalidToZeroD(p->Turnover);
	double avg_price = InvalidToZeroD(p->AveragePrice);
	int cur_volumn = 0;
	double cur_amount = 0;
	std::string exchange_code("U");
	std::string trade_day(p->TradingDay, 8);

	//R|预留|预留|时间|交易所代码|合约代码|成交价|累计成交量|买一价|买一量|卖一价|卖一量|持仓量|昨结算价|当前量
	//|买二价|买三价|买四价|买五价|买二量|买三量|买四量|买五量
	//|卖二价|卖三价|卖四价|卖五价|卖二量|卖三量|卖四量|卖五量
	//|涨停价|跌停价|当日开盘价|当日最高|当日最低|昨收盘|昨持仓|当日成交均价
	//|累计成交金额|当前成交金额\n

	sprintf(total_quote,
		"R|||%s|%s|%s|%.4f|%d|%.4f|%d|%.4f|%d|%.0f|%.4f|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.0f|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%s"
		"|||%.4f|%d|%.6f|%.6f"
		"|%s||||||"
		"||||||"
		"%lld|",
		quote_time.c_str(),                   // 时间
		exchange_code.c_str(),                // 交易所编码
		p->InstrumentID,			          // 合约代码
		InvalidToZeroD(p->LastPrice),            // 成交价
		p->Volume,                            // 成交量（总量）
		InvalidToZeroD(p->BidPrice1),            // 买一价
		p->BidVolume1,                        // 买一量
		InvalidToZeroD(p->AskPrice1),            // 卖一价
		p->AskVolume1,                        // 卖一量
		InvalidToZeroD(p->OpenInterest),         // 持仓量
		InvalidToZeroD(p->PreSettlementPrice),   // 昨结算价
		cur_volumn,                           // 当前量

		InvalidToZeroD(p->BidPrice2),            // 买二价
		InvalidToZeroD(p->BidPrice3),            // 买三价
		InvalidToZeroD(p->BidPrice4),            // 买四价
		InvalidToZeroD(p->BidPrice5),            // 买五价
		p->BidVolume2,                        // 买二量
		p->BidVolume3,                        // 买三量
		p->BidVolume4,                        // 买四量
		p->BidVolume5,                        // 买五量

		InvalidToZeroD(p->AskPrice2),            // 卖二价
		InvalidToZeroD(p->AskPrice3),            // 卖三价
		InvalidToZeroD(p->AskPrice4),            // 卖四价
		InvalidToZeroD(p->AskPrice5),            // 卖五价
		p->AskVolume2,                        // 卖二量
		p->AskVolume3,                        // 卖三量
		p->AskVolume4,                        // 卖四量
		p->AskVolume5,                        // 卖五量

		InvalidToZeroD(p->UpperLimitPrice),      // 涨停价
		InvalidToZeroD(p->LowerLimitPrice),      // 跌停价
		InvalidToZeroD(p->OpenPrice),            // 开盘
		InvalidToZeroD(p->HighestPrice),         // 当日最高
		InvalidToZeroD(p->LowestPrice),          // 当日最低
		InvalidToZeroD(p->PreClosePrice),        // 昨收
		InvalidToZeroD(p->PreOpenInterest),      // 昨持仓
		avg_price,                            // 均价

		// 20131015，新增两个字段
		total_amount,                         // 累计成交金额
		cur_amount,                           // 当前成交金额

		0.0, //涨跌
		0.0, //涨跌幅
		0.0, //档差
		InvalidToZeroD(p->SettlementPrice), //本次结算价
		trade_day.c_str(), //交易日期

		//结算组代码
		//结算编号
		InvalidToZeroD(p->ClosePrice), //今收盘
		0, //成交笔数
		InvalidToZeroD(p->PreDelta), //昨虚实度
		InvalidToZeroD(p->CurrDelta), //今虚实度

		p->ExchangeInstID, //合约在交易所的代码
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
