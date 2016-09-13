#include "kmds.h"
#include "util.h"
#include <string>

#include <sstream>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

static char GetExchCodeByMarket(int market)
{
	//1 上交所
	//2 深交所
	//4 中金所
	//5 上期所
	//6 大商所
	//7 郑商所
	//const char CONST_EXCHCODE_SHFE = 'A';  //上海期货
	//const char CONST_EXCHCODE_DCE = 'B';  //大连期货
	//const char CONST_EXCHCODE_CZCE = 'C';  //郑州期货
	//const char CONST_EXCHCODE_CFFEX = 'G';  //中金所
	//const char CONST_EXCHCODE_SZEX = '0';  //深交所
	//const char CONST_EXCHCODE_SHEX = '1';  //上交所
	char ex = ' ';
	switch(market)
	{
	case 1:ex ='1';break;
	case 2:ex ='0';break;
	case 4:ex ='G';break;
	case 5:ex ='A';break;
	case 6:ex ='B';break;
	case 7:ex ='C';break;
	case 15:ex ='O';break;
	case 19:ex ='O';break;
	default:break;
	}

	return ex;
}

std::string KMDS_QuoteToString( const SaveData_StockQuote_KMDS * const p )
{
	if (!p)
	{
		return "";
	}
	char quote_string[1024];
	const T_StockQuote * const pd = &(p->data_);

	try
	{
		char exch_code = GetExchCodeByMarket(pd->market);
		//时间,交易所,股票代码,状态,最新价,总成交量,成交总金额,
		//买一价,买二价,买三价,买四价,买五价,买六价,买七价,买八价,买九价,买十价,
		//买一量,买二量,买三量,买四量,买五量,买六量,买七量,买八量,买九量,买十量,
		//卖一价,卖二价,卖三价,卖四价,卖五价,卖六价,卖七价,卖八价,卖九价,卖十价,
		//卖一量,卖二量,卖三量,卖四量,卖五量,卖六量,卖七量,卖八量,卖九量,卖十量,
		//前收盘价,开盘价,最高价,最低价,涨停价,跌停价,成交笔数,委托买入总量,委托卖出总量,
		//加权平均委买价格,加权平均委卖价格,IOPV净值估值,到期收益率,市盈率1,市盈率2,
		sprintf_s(quote_string, sizeof(quote_string),
			"R||1|%09d|%c|%s|%c|%.4f|%lld|%.4f|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|"
			"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|"
			"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%u|%lld|%lld|"
			"%.4f|%.4f|%d|%d|%d|%d|%llu|",
			pd->time,
			exch_code,
			pd->scr_code,
			pd->status == 0 ? '-' : pd->status,
			pd->last_price / 10000.0,
			pd->bgn_total_vol,
			pd->bgn_total_amt,
			pd->bid_price[0] / 10000.0,
			pd->bid_price[1] / 10000.0,
			pd->bid_price[2] / 10000.0,
			pd->bid_price[3] / 10000.0,
			pd->bid_price[4] / 10000.0,
			pd->bid_price[5] / 10000.0,
			pd->bid_price[6] / 10000.0,
			pd->bid_price[7] / 10000.0,
			pd->bid_price[8] / 10000.0,
			pd->bid_price[9] / 10000.0,
			pd->bid_volume[0],
			pd->bid_volume[1],
			pd->bid_volume[2],
			pd->bid_volume[3],
			pd->bid_volume[4],
			pd->bid_volume[5],
			pd->bid_volume[6],
			pd->bid_volume[7],
			pd->bid_volume[8],
			pd->bid_volume[9],
			pd->sell_price[0] / 10000.0,
			pd->sell_price[1] / 10000.0,
			pd->sell_price[2] / 10000.0,
			pd->sell_price[3] / 10000.0,
			pd->sell_price[4] / 10000.0,
			pd->sell_price[5] / 10000.0,
			pd->sell_price[6] / 10000.0,
			pd->sell_price[7] / 10000.0,
			pd->sell_price[8] / 10000.0,
			pd->sell_price[9] / 10000.0,
			pd->sell_volume[0],
			pd->sell_volume[1],
			pd->sell_volume[2],
			pd->sell_volume[3],
			pd->sell_volume[4],
			pd->sell_volume[5],
			pd->sell_volume[6],
			pd->sell_volume[7],
			pd->sell_volume[8],
			pd->sell_volume[9],
			0.0,
			0.0,
			pd->high_price / 10000.0,
			pd->low_price / 10000.0,
			0.0,
			0.0,
			pd->bgn_count,
			pd->total_bid_lot,
			pd->total_sell_lot,
			pd->wght_avl_bid_price / 10000.0,
			pd->wght_avl_sell_price / 10000.0,
			pd->iopv_net_val_valt,
			pd->mtur_yld,
			pd->pe_ratio_1,
			pd->pe_ratio_2,
			p->t_
			);	}
	catch (...)
	{
		cout << "error occurs when format SaveData_StockQuote_KMDS." << std::endl;
	}

	static bool kmds_stock_flag = true;
	if(kmds_stock_flag)
	{
		kmds_stock_flag = false;
		return std::string("R||1|时间|交易所|股票代码|状态|最新价|总成交量|成交总金额|买一价|买二价|买三价|买四价|买五价|买六价|买七价|买八价|买九价|买十价|买一量|买二量|买三量|买四量|买五量|买六量|买七量|买八量|买九量|买十量|卖一价|卖二价|卖三价|卖四价|卖五价|卖六价|卖七价|卖八价|卖九价|卖十价|卖一量|卖二量|卖三量|卖四量|卖五量|卖六量|卖七量|卖八量|卖九量|卖十量|前收盘价|开盘价|最高价|最低价|涨停价|跌停价|成交笔数|委托买入总量|委托卖出总量|加权平均委买价格|加权平均委卖价格|IOPV净值估值|到期收益率|市盈率1|市盈率2|时间戳|\n") 
			+ std::string(quote_string);
	}

	return quote_string;
}

std::string KMDS_QuoteToString( const SaveData_IndexQuote_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	char quote_string[1024];
	const T_IndexQuote * const p = &(pd->data_);

	try
	{
		char exch_code = GetExchCodeByMarket(p->market);

		sprintf_s(quote_string, sizeof(quote_string),
			"R||2|%09d|%c|%s|%0.4f|%0.4f|%0.4f|%0.4f|%0.4f|%lld|%lld|%llu|",
			p->time,
			exch_code,
			p->scr_code,
			p->last_price / 10000.0,
			0.0,
			p->high_price / 10000.0,
			p->low_price / 10000.0,
			0.0,
			p->bgn_total_vol,
			p->bgn_total_amt,
			pd->t_
			);
	}
	catch (...)
	{
		cout << "error occurs when format SaveData_IndexQuote_KMDS." << std::endl;
	}

	static bool kmds_idx_flag = true;
	if(kmds_idx_flag)
	{
		kmds_idx_flag = false;
		return std::string("R||2|时间|交易所|股票代码|最新价|0.0|最高价|最低价|0.0|成交总量|成交总金额|时间戳|\n") 
			+ std::string(quote_string);
	}

	return quote_string;
}

std::string KMDS_QuoteToString( const SaveData_Option_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	char quote_string[10240];
	const T_OptionQuote * const p = &(pd->data_);

	try
	{
		char exch_code = GetExchCodeByMarket(p->market);
		//R|预留|预留|时间|交易所代码|合约代码|成交价|累计成交量|买一价|买一量|卖一价|卖一量|持仓量|昨结算价|当前量
		//|买二价|买三价|买四价|买五价|买二量|买三量|买四量|买五量
		//|卖二价|卖三价|卖四价|卖五价|卖二量|卖三量|卖四量|卖五量
		//|涨停价|跌停价|当日开盘价|当日最高|当日最低|昨收盘|昨持仓|当日成交均价
		//|累计成交金额|当前成交金额\n

		sprintf(quote_string,
			"R|||%09d|%c|%s|%.4f|%lld|%.4f|%lld|%.4f|%lld|%lld|%.4f|%d|"
			"%.4f|%.4f|%.4f|%.4f|%lld|%lld|%lld|%lld|"
			"%.4f|%.4f|%.4f|%.4f|%lld|%lld|%lld|%lld|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.0f|%.4f|"
			"%.4f|%.4f|"
			"%.4f|%.4f|%.4f|%.4f|"
			"|||%.4f|%d|%.6f|%.6f"
			"|%s||||||"
			"||||||"
			"%lld|",
			(int)p->time,                       // 时间
			exch_code,// 交易所编码
			std::string(p->scr_code,8).c_str(),                                  // 合约代码
			p->last_price / 10000.0,            // 成交价
			p->bgn_total_vol,                            // 成交量（总量）
			p->bid_price[0] / 10000.0,            // 买一价
			p->bid_volume[0],                        // 买一量
			p->sell_price[0] / 10000.0,            // 卖一价
			p->sell_volume[0],                        // 卖一量
			p->long_position,         // 持仓量
			0.0,   // 昨结算价
			0,                           // 当前量

			p->bid_price[1] / 10000.0,            // 买二价
			p->bid_price[2] / 10000.0,            // 买三价
			p->bid_price[3] / 10000.0,            // 买四价
			p->bid_price[4] / 10000.0,            // 买五价
			p->bid_volume[1],                        // 买二量
			p->bid_volume[2],                        // 买三量
			p->bid_volume[3],                        // 买四量
			p->bid_volume[4],                        // 买五量

			p->sell_price[1] / 10000.0,            // 卖二价
			p->sell_price[2] / 10000.0,            // 卖三价
			p->sell_price[3] / 10000.0,            // 卖四价
			p->sell_price[4] / 10000.0,            // 卖五价
			p->sell_volume[1],                        // 卖二量
			p->sell_volume[2],                        // 卖三量
			p->sell_volume[3],                        // 卖四量
			p->sell_volume[4],                        // 卖五量

			p->high_limit_price / 10000.0,      // 涨停价
			p->low_limit_price / 10000.0,      // 跌停价
			p->open_price / 10000.0,            // 开盘
			p->high_price / 10000.0,         // 当日最高
			p->low_price / 10000.0,          // 当日最低
			p->pre_close_price / 10000.0,        // 昨收
			0.0,      // 昨持仓
			0.0,                            // 均价

			// 20131015，新增两个字段
			p->bgn_total_amt / 10000.0,                         // 累计成交金额
			0.0,                           // 当前成交金额

			0.0, //涨跌
			0.0, //涨跌幅
			0.0, //档差
			0.0, //本次结算价
			//交易日期

			//结算组代码
			//结算编号
			0.0, //今收盘
			0, //成交笔数
			0.0, //昨虚实度
			0.0, //今虚实度

			p->contract_code, //合约在交易所的代码
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
			pd->t_
			);
	}
	catch (...)
	{
		cout << "error occurs when format SaveData_Option_KMDS." << std::endl;
	}

	return quote_string;
}

std::string KMDS_QuoteToString( const SaveData_FutureQuote_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	char quote_string[1024];
	const T_FutureQuote * const p = &(pd->data_);

	try
	{
	}
	catch (...)
	{
		cout << "error occurs when format SaveData_FutureQuote_KMDS." << std::endl;
	}

	return quote_string;
}

std::string KMDS_QuoteToString( const SaveData_OrderQueue_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	const T_OrderQueue * const p = &(pd->data_);

	char time_str[64];
	sprintf(time_str, "%09d", p->time);

	std::stringstream ss;
	// 时间，市场，代码，订单价格，订单数量，明细个数，订单明细数量
	ss << std::fixed << std::setprecision(2)
		<< ConvertToShortTimval(pd->t_) << ","
		<< time_str << ","
		<< p->market << ","
		<< p->scr_code << ","
		<< p->insr_txn_tp_code << ","
		<< p->ord_price << ","
		<< p->ord_qty << ","
		<< p->ord_nbr << ",";
	for (int i = 0; i < p->ord_nbr; ++i)
	{
		ss << p->ord_detail_vol[i] << "|";
	}
	ss << "," << pd->t_;

	static bool kmds_queue_flag = true;
	if(kmds_queue_flag)
	{
		kmds_queue_flag = false;
		return std::string("local_time,时间,交易所,股票代码,交易类型,价格,订单数量,明细个数,(明细数据),时间戳,\n") 
			+ ss.str();
	}

	return ss.str();
}

std::string KMDS_QuoteToString( const SaveData_PerEntrust_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	const T_PerEntrust * const p = &(pd->data_);

	char time_str[64];
	sprintf(time_str, "%09d", p->entrt_time);

	std::stringstream ss;
	// 委托时间，市场，代码，委托价，委托编号，委托量，指令交易类型（B，S），委托类别
	ss << std::fixed << std::setprecision(2)
		<< ConvertToShortTimval(pd->t_) << ","
		<< time_str << ","
		<< p->market << ","
		<< p->scr_code << ","
		<< p->entrt_id << ","
		<< p->entrt_price << ","
		<< p->entrt_vol << ","
		<< std::string(p->insr_txn_tp_code) << ","
		<< std::string(p->entrt_tp) << ","
		<< pd->t_ << ","
		;

	static bool kmds_perentrust_flag = true;
	if(kmds_perentrust_flag)
	{
		kmds_perentrust_flag = false;
		return std::string("local_time,时间,交易所,股票代码,委托编号,委托价,委托量,交易类型,委托类别,时间戳,\n") 
			+ ss.str();
	}

	return ss.str();
}

std::string KMDS_QuoteToString( const SaveData_PerBargain_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}

	const T_PerBargain * const p = &(pd->data_);

	char time_str[64];
	sprintf(time_str, "%09d", p->time);

	std::stringstream ss;
	// 成交时间，市场，代码，成交价，成交编号，成交量，成交金额，成交类别，指令交易类型
	ss << std::fixed << std::setprecision(2)
		<< ConvertToShortTimval(pd->t_) << ","
		<< time_str << ","
		<< p->market << ","
		<< p->scr_code << ","
		<< p->bgn_price << ","
		<< p->bgn_id << ","
		<< p->bgn_qty << ","
		<< p->bgn_amt << ","
		<< std::string(p->nsr_txn_tp_code) << ","
		<< std::string(p->bgn_flg) << ","
		<< pd->t_ << ","
		;

	static bool kmds_perbargain_flag = true;
	if(kmds_perbargain_flag)
	{
		kmds_perbargain_flag = false;
		return std::string("local_time,时间,交易所,股票代码,成交价,成交编号,成交量,成交金额,交易类型,成交类别,时间戳,\n") 
			+ ss.str();
	}

	return ss.str();
}
