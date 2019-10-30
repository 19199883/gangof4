#include "yao_quote.h"
#include <string>
#include "util.h"

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

std::string YaoQuoteToString(const SaveData_YaoQuote* const p)
{
	if (!p_data)
	{
		return "";
	}

	const YaoQuote * const p = &(p_data->data_);	
	char buf[5120];
	if (p) 
	{
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
	
	
	return buf
}

