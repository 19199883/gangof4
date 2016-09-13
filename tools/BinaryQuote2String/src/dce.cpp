#include "dce.h"
#include <string>

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "util.h"

using namespace std;

#pragma pack(push)
#pragma pack(4)
	////////////////////////////////////////////////
	///MDBestAndDeep：最优与五档深度行情
	////////////////////////////////////////////////
	struct bd_tt{
		INT1		Type;
		UINT4		Length;							//报文长度
		UINT4		Version;						//版本从1开始
		UINT4		Time;							//预留字段
		INT1		Exchange[3];					//交易所
		INT1		Contract[80];					//合约代码
		BOOL		SuspensionSign;					//停牌标志
		REAL4		LastClearPrice;					//昨结算价
		REAL4		ClearPrice;						//今结算价
		REAL4		AvgPrice;						//成交均价
		REAL4		LastClose;						//昨收盘
		REAL4		Close;							//今收盘
		REAL4		OpenPrice;						//今开盘
		UINT4		LastOpenInterest;				//昨持仓量
		UINT4		OpenInterest;					//持仓量
		REAL4		LastPrice;						//最新价
		UINT4		MatchTotQty;					//成交数量
		REAL8		Turnover;						//成交金额
		REAL4		RiseLimit;						//最高报价
		REAL4		FallLimit;						//最低报价
		REAL4		HighPrice;						//最高价
		REAL4		LowPrice;						//最低价
		REAL4		PreDelta;						//昨虚实度
		REAL4		CurrDelta;						//今虚实度
		REAL4		BuyPriceOne;					//买入价格1
		UINT4		BuyQtyOne;						//买入数量1
		UINT4		BuyImplyQtyOne;					//买1推导量
		REAL4		BuyPriceTwo;
		UINT4		BuyQtyTwo;
		UINT4		BuyImplyQtyTwo;
		REAL4		BuyPriceThree;
		UINT4		BuyQtyThree;
		UINT4		BuyImplyQtyThree;
		REAL4		BuyPriceFour;
		UINT4		BuyQtyFour;
		UINT4		BuyImplyQtyFour;
		REAL4		BuyPriceFive;
		UINT4		BuyQtyFive;
		UINT4		BuyImplyQtyFive;
		REAL4		SellPriceOne;					//卖出价格1
		UINT4		SellQtyOne;						//买出数量1
		UINT4		SellImplyQtyOne;				//卖1推导量
		REAL4		SellPriceTwo;
		UINT4		SellQtyTwo;
		UINT4		SellImplyQtyTwo;
		REAL4		SellPriceThree;
		UINT4		SellQtyThree;
		UINT4		SellImplyQtyThree;
		REAL4		SellPriceFour;
		UINT4		SellQtyFour;
		UINT4		SellImplyQtyFour;
		REAL4		SellPriceFive;
		UINT4		SellQtyFive;
		UINT4		SellImplyQtyFive;
		INT1		GenTime[13];					//行情产生时间
		UINT4		LastMatchQty;					//最新成交量
		INT4		InterestChg;					//持仓量变化
		REAL4		LifeLow;						//历史最低价
		REAL4		LifeHigh;						//历史最高价
		REAL8		Delta;							//delta
		REAL8		Gamma;							//gama
		REAL8		Rho;							//rho
		REAL8		Theta;							//theta
		REAL8		Vega;							//vega
		INT1		TradeDate[9];					//行情日期
		INT1		LocalDate[9];					//本地日期
	};
#pragma pack(pop)



std::string BestAndDeepToString( const SaveData_MDBestAndDeep * const p_data )
{
	if (!p_data)
	{
		return "";
	}
	double d1 = DBL_MAX;
	double d2 = DBL_MIN;
	float f1 = FLT_MAX;
	float f2 = FLT_MIN;
	const MDBestAndDeep_MY &quote = p_data->data_;

	const bd_tt * ttp = (const bd_tt *)&p_data->data_;
	char total_quote[2048];

	bool b1 = quote.ClearPrice > f1;
	bool b2 = quote.ClearPrice < f2;
	bool b3 = quote.Delta > d1;
	bool b4 = quote.Delta < d2;

	double d3 = InvalidToZeroD(quote.Delta);
	float f3 = InvalidToZeroF(quote.ClearPrice);

	// 当前的行情序列化
	string quote_time = quote.GenTime;
	boost::erase_all(quote_time, ":");
	boost::erase_all(quote_time, ".");
	if (quote_time.empty())
	{
		quote_time = "0";
	}

	// 当前量, 当前成交金额
	double total_amount = InvalidToZeroD(quote.Turnover);
	unsigned int cur_volumn = quote.LastMatchQty;
	double cur_amount = 0;

	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);

	//InstrumentToVolumnAndAmountMap::iterator it = best_quoteinfo_.find(quote.Contract);
	//if (it != best_quoteinfo_.end())
	//{
	//	cur_volumn = quote.MatchTotQty - it->second.first;
	//	cur_amount = total_amount - it->second.second;
	//	it->second = std::make_pair(quote.MatchTotQty, total_amount);
	//}
	//else
	//{
	//	cur_volumn = quote.MatchTotQty;
	//	cur_amount = total_amount;
	//	best_quoteinfo_.insert(std::make_pair(quote.Contract, 
	//		std::make_pair(quote.MatchTotQty, total_amount)));
	//}

	//R|预留|预留|时间|交易所代码|合约代码|成交价|累计成交量|买一价|买一量|卖一价|卖一量|持仓量|昨结算价|当前量|
	//买二价|买三价|买四价|买五价|买二量|买三量|买四量|买五量|
	//卖二价|卖三价|卖四价|卖五价|卖二量|卖三量|卖四量|卖五量|
	//涨停价|跌停价|当日开盘价|当日最高|当日最低|昨收盘|昨持仓|当日成交均价|
	//累计成交金额|当前成交金额\n

	sprintf(total_quote,
		"R|||%s|B|%s|%.4f|%u|%.4f|%u|%.4f|%u|%u|%.4f|%u|"
		"%.4f|%.4f|%.4f|%.4f|%u|%u|%u|%u|"
		"%.4f|%.4f|%.4f|%.4f|%u|%u|%u|%u|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%u|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%08d|"
		"||%.4f|%d|%.6f|%.6f|"
		"%s|%d|%d|%.6f|%.6f|%d|%d|"
		"%.6f|%.6f|%.6f|%.6f|%.6f|"
		"%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|"
		"%lld|",
		quote_time.c_str(),               // 时间
		// 交易所编码
		quote.Contract,                      // 合约代码
		InvalidToZeroF(quote.LastPrice),        // 成交价
		quote.MatchTotQty,                   // 成交量（总量）
		InvalidToZeroF(quote.BuyPriceOne),      // 买一价
		quote.BuyQtyOne,                     // 买一量
		InvalidToZeroF(quote.SellPriceOne),     // 卖一价
		quote.SellQtyOne,                    // 卖一量
		quote.OpenInterest,                  // 持仓量
		InvalidToZeroF(quote.LastClearPrice),   // 昨结算价
		cur_volumn,                       // 当前量

		InvalidToZeroF(quote.BuyPriceTwo),      // 买二价
		InvalidToZeroF(quote.BuyPriceThree),    // 买三价
		InvalidToZeroF(quote.BuyPriceFour),     // 买四价
		InvalidToZeroF(quote.BuyPriceFive),     // 买五价
		quote.BuyQtyTwo,                     // 买二量
		quote.BuyQtyThree,                   // 买三量
		quote.BuyQtyFour,                    // 买四量
		quote.BuyQtyFive,                    // 买五量

		InvalidToZeroF(quote.SellPriceTwo),     // 卖二价
		InvalidToZeroF(quote.SellPriceThree),   // 卖三价
		InvalidToZeroF(quote.SellPriceFour),    // 卖四价
		InvalidToZeroF(quote.SellPriceFive),    // 卖五价
		quote.SellQtyTwo,                    // 卖二量
		quote.SellQtyThree,                  // 卖三量
		quote.SellQtyFour,                   // 卖四量
		quote.SellQtyFive,                   // 卖五量

		quote.RiseLimit,                     // 涨停价
		quote.FallLimit,                     // 跌停价
		InvalidToZeroF(quote.OpenPrice),        // 开盘
		InvalidToZeroF(quote.HighPrice),        // 当日最高
		InvalidToZeroF(quote.LowPrice),         // 当日最低
		InvalidToZeroF(quote.LastClose),        // 昨收
		quote.LastOpenInterest,              // 昨持仓
		InvalidToZeroF(quote.AvgPrice),         // 均价

		// 20131015，新增两个字段
		total_amount,                     // 累计成交金额
		cur_amount,                       // 当前成交金额

		0.0, //涨跌
		0.0, //涨跌幅
		0.0, //档差
		InvalidToZeroD(quote.ClearPrice),		  //本次结算价
		d_, //交易日期

		//结算组代码
		//结算编号
		InvalidToZeroD(quote.Close),					//今收盘
		0,										//成交笔数
		InvalidToZeroD(quote.PreDelta),				//昨虚实度
		InvalidToZeroD(quote.CurrDelta),				//今虚实度

		"",										//合约在交易所的代码
		(int)quote.SuspensionSign,					//停牌标志
		quote.InterestChg,							//持仓量变化
		InvalidToZeroD(quote.LifeLow),				//历史最低价
		InvalidToZeroD(quote.LifeHigh),				//历史最高价
		quote.BuyImplyQtyOne,			//申买推导量
		quote.SellImplyQtyOne,			//申卖推导量

		InvalidToZeroD(quote.Delta),					//delta（期权用）
		InvalidToZeroD(quote.Gamma),					//gama（期权用）
		InvalidToZeroD(quote.Rho),					//rho（期权用）
		InvalidToZeroD(quote.Theta),					//theta（期权用）
		InvalidToZeroD(quote.Vega),					//vega（期权用）	

		quote.BuyImplyQtyOne,
		quote.BuyImplyQtyTwo,
		quote.BuyImplyQtyThree,
		quote.BuyImplyQtyFour,
		quote.BuyImplyQtyFive,
		quote.SellImplyQtyOne,
		quote.SellImplyQtyTwo,
		quote.SellImplyQtyThree,
		quote.SellImplyQtyFour,
		quote.SellImplyQtyFive,

		p_data->t_
		);

	return total_quote;
}

std::string ArbiToString( const SaveData_Arbi * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDBestAndDeep_MY &quote = p_data->data_;
	char total_quote[2048];

	// 当前的行情序列化
	string quote_time = quote.GenTime;
	boost::erase_all(quote_time, ":");
	boost::erase_all(quote_time, ".");
	if (quote_time.empty())
	{
		quote_time = "0";
	}

	// 当前量, 当前成交金额
	double total_amount = InvalidToZeroD(quote.Turnover);
	unsigned int cur_volumn = 0;
	double cur_amount = 0;
	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);
	//InstrumentToVolumnAndAmountMap::iterator it = best_quoteinfo_.find(quote.Contract);
	//if (it != best_quoteinfo_.end())
	//{
	//	cur_volumn = quote.MatchTotQty - it->second.first;
	//	cur_amount = total_amount - it->second.second;
	//	it->second = std::make_pair(quote.MatchTotQty, total_amount);
	//}
	//else
	//{
	//	cur_volumn = quote.MatchTotQty;
	//	cur_amount = total_amount;
	//	best_quoteinfo_.insert(std::make_pair(quote.Contract, 
	//		std::make_pair(quote.MatchTotQty, total_amount)));
	//}

	//R|预留|预留|时间|交易所代码|合约代码|成交价|累计成交量|买一价|买一量|卖一价|卖一量|持仓量|昨结算价|当前量|
	//买二价|买三价|买四价|买五价|买二量|买三量|买四量|买五量|
	//卖二价|卖三价|卖四价|卖五价|卖二量|卖三量|卖四量|卖五量|
	//涨停价|跌停价|当日开盘价|当日最高|当日最低|昨收盘|昨持仓|当日成交均价|
	//累计成交金额|当前成交金额\n

	sprintf(total_quote,
		"R|||%s|B|%s|%.4f|%u|%.4f|%u|%.4f|%u|%u|%.4f|%u|"
		"%.4f|%.4f|%.4f|%.4f|%u|%u|%u|%u|"
		"%.4f|%.4f|%.4f|%.4f|%u|%u|%u|%u|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%u|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%08d|"
		"||%.4f|%d|%.6f|%.6f|"
		"%s|%d|%d|%.6f|%.6f|%.6f|%.6f|"
		"%.6f|%.6f|%.6f|%.6f|%.6f|"
		"%lld|",
		quote_time.c_str(),               // 时间
		// 交易所编码
		quote.Contract,                      // 合约代码
		InvalidToZeroF(quote.LastPrice),        // 成交价
		quote.MatchTotQty,                   // 成交量（总量）
		InvalidToZeroF(quote.BuyPriceOne),      // 买一价
		quote.BuyQtyOne,                     // 买一量
		InvalidToZeroF(quote.SellPriceOne),     // 卖一价
		quote.SellQtyOne,                    // 卖一量
		quote.OpenInterest,                  // 持仓量
		InvalidToZeroF(quote.LastClearPrice),   // 昨结算价
		cur_volumn,                       // 当前量

		InvalidToZeroF(quote.BuyPriceTwo),      // 买二价
		InvalidToZeroF(quote.BuyPriceThree),    // 买三价
		InvalidToZeroF(quote.BuyPriceFour),     // 买四价
		InvalidToZeroF(quote.BuyPriceFive),     // 买五价
		quote.BuyQtyTwo,                     // 买二量
		quote.BuyQtyThree,                   // 买三量
		quote.BuyQtyFour,                    // 买四量
		quote.BuyQtyFive,                    // 买五量

		InvalidToZeroF(quote.SellPriceTwo),     // 卖二价
		InvalidToZeroF(quote.SellPriceThree),   // 卖三价
		InvalidToZeroF(quote.SellPriceFour),    // 卖四价
		InvalidToZeroF(quote.SellPriceFive),    // 卖五价
		quote.SellQtyTwo,                    // 卖二量
		quote.SellQtyThree,                  // 卖三量
		quote.SellQtyFour,                   // 卖四量
		quote.SellQtyFive,                   // 卖五量

		quote.RiseLimit,                     // 涨停价
		quote.FallLimit,                     // 跌停价
		InvalidToZeroF(quote.OpenPrice),        // 开盘
		InvalidToZeroF(quote.HighPrice),        // 当日最高
		InvalidToZeroF(quote.LowPrice),         // 当日最低
		InvalidToZeroF(quote.LastClose),        // 昨收
		quote.LastOpenInterest,              // 昨持仓
		InvalidToZeroF(quote.AvgPrice),         // 均价

		// 20131015，新增两个字段
		total_amount,                     // 累计成交金额
		cur_amount,                       // 当前成交金额

		0.0, //涨跌
		0.0, //涨跌幅
		0.0, //档差
		InvalidToZeroD(quote.ClearPrice),		  //本次结算价
		d_, //交易日期

		//结算组代码
		//结算编号
		InvalidToZeroD(quote.Close),					//今收盘
		0,										//成交笔数
		InvalidToZeroD(quote.PreDelta),				//昨虚实度
		InvalidToZeroD(quote.CurrDelta),				//今虚实度

		"",										//合约在交易所的代码
		(int)quote.SuspensionSign,					//停牌标志
		quote.InterestChg,							//持仓量变化
		InvalidToZeroD(quote.LifeHigh),				//历史最低价
		InvalidToZeroD(quote.LifeLow),				//历史最高价
		InvalidToZeroD(quote.BuyImplyQtyOne),			//申买推导量
		InvalidToZeroD(quote.SellImplyQtyOne),			//申卖推导量

		InvalidToZeroD(quote.Delta),					//delta（期权用）
		InvalidToZeroD(quote.Gamma),					//gama（期权用）
		InvalidToZeroD(quote.Rho),					//rho（期权用）
		InvalidToZeroD(quote.Theta),					//theta（期权用）
		InvalidToZeroD(quote.Vega),					//vega（期权用）				

		p_data->t_
		);

	return total_quote;
}

std::string TenEntrustToString( const SaveData_MDTenEntrust * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDTenEntrust_MY &quote = p_data->data_;
	char total_quote[2048];

	// 当前的行情序列化
	string quote_time = quote.GenTime;
	boost::erase_all(quote_time, ":");
	boost::erase_all(quote_time, ".");
	if (quote_time.empty())
	{
		quote_time = "0";
	}
	//"R,预留,预留,时间,交易所代码,合约代码,最优买价,最优卖价,"
	//"委买量1,委买量2,委买量3,委买量4,委买量5,委买量6,委买量7,委买量8,委买量9,委买量10,"
	//"委卖量1,委卖量2,委卖量3,委卖量4,委卖量5,委卖量6,委卖量7,委卖量8,委卖量9,委卖量10"
	sprintf(total_quote,
		"R|||%s|B|%s|"
		"%.4f|%.4f|"
		"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
		"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
		"%lld|",
		quote_time.c_str(),							// 时间
		// 交易所编码
		quote.Contract,								// 合约代码

		InvalidToZeroD(quote.BestBuyOrderPrice),			// 最优买价
		InvalidToZeroD(quote.BestSellOrderPrice),			// 最优卖价

		quote.BestBuyOrderQtyOne,			// 委买量1
		quote.BestBuyOrderQtyTwo,			// 委买量2
		quote.BestBuyOrderQtyThree,		// 委买量3
		quote.BestBuyOrderQtyFour,			// 委买量4
		quote.BestBuyOrderQtyFive,			// 委买量5
		quote.BestBuyOrderQtySix,			// 委买量6
		quote.BestBuyOrderQtySeven,		// 委买量7
		quote.BestBuyOrderQtyEight,		// 委买量8
		quote.BestBuyOrderQtyNine,			// 委买量9
		quote.BestBuyOrderQtyTen,			// 委买量10

		quote.BestSellOrderQtyOne,			// 委卖量1
		quote.BestSellOrderQtyTwo,			// 委卖量2
		quote.BestSellOrderQtyThree,		// 委卖量3
		quote.BestSellOrderQtyFour,		// 委卖量4
		quote.BestSellOrderQtyFive,		// 委卖量5
		quote.BestSellOrderQtySix,			// 委卖量6
		quote.BestSellOrderQtySeven,		// 委卖量7
		quote.BestSellOrderQtyEight,		// 委卖量8
		quote.BestSellOrderQtyNine,		// 委卖量9
		quote.BestSellOrderQtyTen,			// 委卖量10

		p_data->t_
		);

	return total_quote;
}

std::string OrderStatisticToString( const SaveData_MDOrderStatistic * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDOrderStatistic_MY &quote = p_data->data_;
	char total_quote[2048];
	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);

	//"R,预留,预留,时间,交易所代码,合约代码,"
	//"买委托总量,卖委托总量,加权平均委买价格,加权平均委卖价格"
	sprintf(total_quote,
		"R|||%09d|B|%s|"
		"%u|%u|%.6f|%.6f|"
		"%lld|",
		t_,							// 时间
		// 交易所编码
		quote.ContractID,					// 合约代码

		quote.TotalBuyOrderNum,			// 买委托总量
		quote.TotalSellOrderNum,			// 卖委托总量
		InvalidToZeroD(quote.WeightedAverageBuyOrderPrice),	// 加权平均委买价格
		InvalidToZeroD(quote.WeightedAverageSellOrderPrice),	// 加权平均委卖价格

		p_data->t_
		);

	return total_quote;
}

std::string RealTimePriceToString( const SaveData_MDRealTimePrice * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDRealTimePrice_MY &quote = p_data->data_;
	char total_quote[2048];
	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);

	//"R,预留,预留,时间,交易所代码,合约代码,实时结算价"
	sprintf(total_quote,
		"R|||%09d|B|%s|%.4f|"
		"%lld|",
		t_,							// 时间
		// 交易所编码
		quote.ContractID,					// 合约代码
		InvalidToZeroD(quote.RealTimePrice),	// 实时结算价

		p_data->t_
		);

	return total_quote;
}

std::string MarchPriceQtyToString( const SaveData_MDMarchPriceQty * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDMarchPriceQty_MY &quote = p_data->data_;
	char total_quote[2048];
	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);

	//"R,预留,预留,时间,交易所代码,合约代码,"
	//"价格1,买开数量1,买平数量1,卖开数量1,卖平数量1,"
	//"价格2,买开数量2,买平数量2,卖开数量2,卖平数量2,"
	//"价格3,买开数量3,买平数量3,卖开数量3,卖平数量3,"
	//"价格4,买开数量4,买平数量4,卖开数量4,卖平数量4,"
	//"价格5,买开数量5,买平数量5,卖开数量5,卖平数量5,"
	sprintf(total_quote,
		"R|||%09d|B|%s|"
		"%.4f|%u|%u|%u|%u|"
		"%.4f|%u|%u|%u|%u|"
		"%.4f|%u|%u|%u|%u|"
		"%.4f|%u|%u|%u|%u|"
		"%.4f|%u|%u|%u|%u|"
		"%lld|",
		t_,							// 时间
		// 交易所编码
		quote.ContractID,					// 合约代码

		InvalidToZeroD(quote.PriceOne),	// 价格1
		quote.PriceOneBOQty,			// 买开数量1
		quote.PriceOneBEQty,			// 买平数量1
		quote.PriceOneSOQty,			// 卖开数量1
		quote.PriceOneSEQty,			// 卖平数量1

		InvalidToZeroD(quote.PriceTwo),	// 价格2
		quote.PriceTwoBOQty,			// 买开数量2
		quote.PriceTwoBEQty,			// 买平数量2
		quote.PriceTwoSOQty,			// 卖开数量2
		quote.PriceTwoSEQty,			// 卖平数量2

		InvalidToZeroD(quote.PriceThree),	// 价格3
		quote.PriceThreeBOQty,			// 买开数量3
		quote.PriceThreeBEQty,			// 买平数量3
		quote.PriceThreeSOQty,			// 卖开数量3
		quote.PriceThreeSEQty,			// 卖平数量3

		InvalidToZeroD(quote.PriceFour),	// 价格4
		quote.PriceFourBOQty,			// 买开数量4
		quote.PriceFourBEQty,			// 买平数量4
		quote.PriceFourSOQty,			// 卖开数量4
		quote.PriceFourSEQty,			// 卖平数量4

		InvalidToZeroD(quote.PriceFive),	// 价格5
		quote.PriceFiveBOQty,			// 买开数量5
		quote.PriceFiveBEQty,			// 买平数量5
		quote.PriceFiveSOQty,			// 卖开数量5
		quote.PriceFiveSEQty,			// 卖平数量5

		p_data->t_
		);

	return total_quote;
}
