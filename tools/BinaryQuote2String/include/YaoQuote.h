
#ifndef   YAOQUOTE_H_
#define   YAOQUOTE_H_

enum YaoExchanges
{
	YSHFE	= 'A',
	YCFFEX	= 'G',
	YDCE		= 'B',
	YCZCE	= 'C'
};

enum FeedTypes
{
	// 中金所深度行情
	CffexLevel2 = 0,
	//	大商所深度
	DceLevel2 = 1,
	// 大商所OrderStats
	DceOrderStats = 3,
	// 大商所深度与大商所OrderStats拼接
	DceCombine = 27,
	// 上期深度
	ShfeLevel2 = 6,
	// 大商所深度行情
	CzceLevel2 = 7,
	// CTP一档行情
	CTPLevel1 = 12
};

struct YaoQuote
{
public:
	static std::string ToString(const YaoQuote * p)
	{
		char buf[10240];
		if (p) {
			snprintf(buf, sizeof(buf), "structName=YaoQuote\n"
				"\tfeed_type=%d\n"
				"\tsymbol=%s\n"
				"\texchange=%c\n"
				"\tint_time=%d\n"
				"\tpre_close_px=%f\n"
				"\tpre_settle_px=%f\n"
				"\tpre_open_interest=%f\n"
				"\topen_interest=%f\n"
				"\topen_px=%f\n"
				"\thigh_px=%f\n"
				"\tlow_px=%f\n"
				"\tavg_px=%f\n"
				"\tlast_px=%f\n"
				"\ttotal_vol=%d\n"
				"\ttotal_notional=%f\n"
				"\tupper_limit_px=%f\n"
				"\tlower_limit_px=%f\n"
				"\tclose_px=%f\n"
				"\tsettle_px=%f\n"
				"\ttotal_buy_ordsize=%d\n"
				"\ttotal_sell_ordsize=%d\n"
				
				"\tBidPrice1=%f\n"
				"\tBidPrice2=%f\n"
				"\tBidPrice3=%f\n"
				"\tBidPrice4=%f\n"
				"\tBidPrice5=%f\n"

				"\tAskPrice1=%f\n"
				"\tAskPrice2=%f\n"
				"\tAskPrice3=%f\n"
				"\tAskPrice4=%f\n"
				"\tAskPrice5=%f\n"

				"\tBidLot1=%i\n"
				"\tBidLot2=%i\n"
				"\tBidLot3=%i\n"
				"\tBidLot4=%i\n"
				"\tBidLot5=%i\n"

				"\tAskLot1=%i\n"
				"\tAskLot2=%i\n"
				"\tAskLot3=%i\n"
				"\tAskLot4=%i\n"
				"\tAskLot5=%i\n",
				(int)p->feed_type,             
				p->symbol,
				p->exchange,        
				p->int_time,         
				p->pre_close_px,  
				p->pre_settle_px,        
				p->pre_open_interest,        
				p->open_interest,         
				p->open_px,        
				p->high_px,      
				p->low_px,     
				p->avg_px,       
				p->last_px,  
				p->total_vol,	
				p->total_notional,     
				p->upper_limit_px,         
				p->lower_limit_px,          
				p->close_px,        
				p->settle_px,         
				p->total_buy_ordsize,      
				p->total_sell_ordsize,      

				p->bp_array[0],                
				p->bp_array[1],                
				p->bp_array[2],                
				p->bp_array[3],                
				p->bp_array[4],                

				p->ap_array[0],                
				p->ap_array[1],                
				p->ap_array[2],                
				p->ap_array[3],                
				p->ap_array[4],                

				p->bv_array[0],              
				p->bv_array[1],              
				p->bv_array[2],              
				p->bv_array[3],              
				p->bv_array[4],              

				p->av_array[0],              
				p->av_array[1],              
				p->av_array[2],              
				p->av_array[3],              
				p->av_array[4]              

			);
		}
		return buf;
	}


public:
	FeedTypes feed_type;				
	char symbol[48];			//	合约代码: 如'm1801','SR801','Au(T+D)'
	char exchange;			//	交易所代码
	//	交易所行情时间(HHMMssmmm), 如：90000306表示09:00:00 306. 0点-3点的数据 +24hrs
	int int_time;				
	float pre_close_px;			//	昨收盘价
	float pre_settle_px;		//	昨结算价
	double pre_open_interest;	//	昨市场总持仓量
	double open_interest;		//	市场总持仓量
	float open_px;				//	开盘价
	float high_px;				//	最高价
	float low_px;				//	最低价
	float avg_px;				//	平均价
	float last_px;				//	最新价
	float bp_array[5];			//	多档买价
	float ap_array[5];			//	多档卖价
	int bv_array[5];			//	多档买量
	int av_array[5];			//	多档卖量
	int total_vol;				//	总成交量
	float total_notional;		//	总成交额
	float upper_limit_px;		//	涨停价
	float lower_limit_px;		//	跌停价
	float close_px;				//	收盘价 只在日盘收盘后的几笔行情有效，其余时间为0
	float settle_px;			//	结算价 只在日盘收盘后的几笔行情有效，其余时间为0
	int implied_bid_size[5];	//	推导买量
	int implied_ask_size[5];	//	推导卖量
	int total_buy_ordsize;		//	总买量 DCE OrderStat quote
	int total_sell_ordsize;		//	总卖量 DCE OrderStat quote
	float weighted_buy_px;		//	平均买价 DCE OrderStat quote
	float weighted_sell_px;		//	平均卖价 DCE OrderStat quote	
};

#endif // YAOQUOTE_H_
