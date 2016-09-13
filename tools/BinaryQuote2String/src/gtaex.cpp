
#include "gtaex.h"

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <Windows.h>

InstrumentToAmountMap instrument_to_cffex_quoteinfo_;

// 行情信息，GTA的行情时间没有毫秒数，股指当前是1s两笔行情，
// 根据当前秒是否首次收到，附加毫秒信息
struct QuoteInfo
{
	std::string security_code;
	std::string update_time;

	QuoteInfo(const std::string &s, const std::string &t)
		:security_code(s), update_time(t)
	{

	}

	bool operator==(const QuoteInfo &rhs)
	{
		return this->security_code == rhs.security_code;
	}
};
typedef std::vector<QuoteInfo> QuoteDatas;
static QuoteDatas s_quotes_;
static boost::mutex s_quote_mutex_;

using namespace std;
#include "gtaex.h"

std::string GTAQuoteToString( const SaveData_GTAEX * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const ZJS_Future_Input_MY * const p = &(p_data->data_);
	char whole_quote_str[2048];
	bool first_of_this_second = false;
	std::string code(p->szStockNo, 6);
	std::string time_str;
	{
		boost::mutex::scoped_lock lock(s_quote_mutex_);
		QuoteDatas::iterator it;
		for (it = s_quotes_.begin(); it != s_quotes_.end(); ++it)
		{
			if (it->security_code == code)
			{
				break;
			}
		}

		if (it == s_quotes_.end())
		{
			s_quotes_.push_back(QuoteInfo(code, p->szUpdateTime));
			it = s_quotes_.end() - 1;
			first_of_this_second = true;
		}

		time_str.append(p->szUpdateTime, 2);
		time_str.append(p->szUpdateTime + 3, 2);
		time_str.append(p->szUpdateTime + 6, 2);
		if (first_of_this_second || it->update_time != p->szUpdateTime)
		{
			time_str.append("000");
			it->update_time = p->szUpdateTime;
		}
		else
		{
			time_str.append("500");
		}
	}

	// 当前成交金额
	double cur_amount = 0;
	InstrumentToAmountMap::iterator it = instrument_to_cffex_quoteinfo_.find(code);
	if (it != instrument_to_cffex_quoteinfo_.end())
	{
		cur_amount = p->dAmount - it->second;
		it->second = p->dAmount;
	}
	else
	{
		cur_amount = p->dAmount;
		instrument_to_cffex_quoteinfo_.insert(std::make_pair(code, p->dAmount));
	}
	std::string trade_day(p->szTradingDay, 8); 

	//R|||134500500|G|IF1309|2182.8|41533|2182.8|4|2183.0|1|22549|2231|7|2182.2|2182.0|2181.8|2181.6|13|9|2|5|2184.0|2184.6|2184.8|2185.0|6|5|18|14|2454.0|2008.0|2230.8|2235.2|2164.0|2236.6|21357|2199.05|
	// 国债期货，价格要保留小数点后4位
	sprintf_s(whole_quote_str, "R|||%s|G|%s|%.4f|%.0f|%.4f|%.0f|%.4f|%.0f|%.0f|%.4f|%.0f|"
		"%.4f|%.4f|%.4f|%.4f|%.0f|%.0f|%.0f|%.0f|%.4f|%.4f|%.4f|%.4f|%.0f|%.0f|%.0f|%.0f|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.0f|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%s"
		"|||%.4f|%d|%.6f|%.6f"
		"|||||||"
		"||||||%llu|",
		time_str.c_str(),	// 时间               
		code.c_str(),		// stock       
		p->dLastTrade,		// 成交价               
		p->dVolume,			// 成交量（总量）       
		p->dBuyprice1,		// 买一价               
		p->dBuyvol1,		// 买一量               
		p->dSellprice1,		// 卖一价           
		p->dSellvol1,		// 卖一量               
		p->dOpenInterest,	// 持仓量           
		p->dPreSettlementPrice,	// 昨结算价 
		p->dLastVolume,// 当前量               

		p->dBuyprice2,	// 买二价               
		p->dBuyprice3,	// 买三价               
		p->dBuyprice4,	// 买四价               
		p->dBuyprice5,	// 买五价               
		p->dBuyvol2,	// 买二量               
		p->dBuyvol3,	// 买三量               
		p->dBuyvol4,	// 买四量               
		p->dBuyvol5,	// 买五量               
		p->dSellprice2,	// 卖二价           
		p->dSellprice3,	// 卖三价           
		p->dSellprice4,	// 卖四价           
		p->dSellprice5,	// 卖五价           
		p->dSellvol2,	// 卖二量               
		p->dSellvol3,	// 卖三量               
		p->dSellvol4,	// 卖四量               
		p->dSellvol5,	// 卖五量     

		p->dUpB,		// 涨停价               
		p->dLowB,		// 跌停价               
		p->dOpen,		// 开盘                 
		p->dHigh,		// 当日最高             
		p->dLow,		// 当日最低             
		p->dYclose,		// 昨收                 
		p->dPreOpenInterest,	// 昨持仓       
		p->dAvgPrice,	// 均价

		// 20131015，新增两个字段
		p->dAmount,		// 累计成交金额
		cur_amount,		// 当前成交金额

		p->dChg, //涨跌
		p->dChgPct, //涨跌幅
		p->dPriceGap, //档差
		InvalidToZeroD(p->dSettlementPrice), //本次结算价
		trade_day.c_str(), //交易日期

		//结算组代码
		//结算编号
		0.0, //今收盘
		0, //成交笔数
		0.0, //昨虚实度
		0.0, //今虚实度

		//合约在交易所的代码
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

	return whole_quote_str;
}
