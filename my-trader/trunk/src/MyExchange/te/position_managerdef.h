#include <boost/foreach.hpp>
#include "../my_exchange.h"
#include "pending_ord_request_dao.cpp"
#include "position_entity.h"
#include <log4cxx/logger.h>
#include <map>
#include <chrono>
#include <thread>
#include <list>

#include "my_trade_tunnel_api.h"

using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace trading_channel_agent;
using namespace trading_engine;

position_manager::position_manager(shared_ptr<tca> _tca_ptr,model_setting model_setting_ins,const long &_model_id)
{
	this->model_setting_ins_ = model_setting_ins;
	this->model_id = _model_id;
	this->tca_ptr = _tca_ptr;

	_report_record_cache = ReportNotifyRecordT(10000);
	_report_record_cache.reserve(10000);

	_quote_report_record_cache = ReportNotifyRecordT(10000);
	_quote_report_record_cache.reserve(10000);

	pending_vol_cache = vector<order_request_t>(1000);
	pending_vol_cache.reserve(1000);
	pending_vol_cache_size = 0;
}

position_manager::~position_manager(void)
{
}

void position_manager::fill_today_traded_info(
		position_t today_pos,acc_volume_t *today_acc_volume,int count)
{
	for(int i=0; i<today_pos.symbol_cnt; i++){
		for(int j=0; j<count; j++){
			if(0 == strcmp(today_acc_volume[j].symbol,today_pos.s_pos[i].symbol)){
				today_pos.s_pos[i].today_buy_volume = today_acc_volume[j].buy_volume;
				today_pos.s_pos[i].today_aver_price_buy = today_acc_volume[j].buy_price;
				today_pos.s_pos[i].today_sell_volume = today_acc_volume[j].sell_volume;
				today_pos.s_pos[i].today_aver_price_sell = today_acc_volume[j].sell_price;
				today_pos.s_pos[i].exchg_code = today_acc_volume[j].exchg_code;
			}
		} // end for(int j=0; j<count; j++){
	} // end for(int i=0; i<today_pos.symbol_cnt; i++){

}


T_ContractInfoReturn position_manager::query_ContractInfo(st_config_t *config, exchange_names ex,int *ret_code)
{

	T_ContractInfoReturn contracts;
	
	tcs *channel_ = this->tca_ptr->get_tcs(model_id,ex);
	if(NULL==channel_){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
			"can' find a tunnel when exectting query_ContractInfo method, criteria: model id=" << model_id
			<< "; exchange=" << ex);
		return contracts;
	}

	contracts = channel_->query_ContractInfo();
	while(0 != contracts.error_no){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
						"query_position failed,cause:" << contracts.error_no);
		this_thread::sleep_for(std::chrono::milliseconds(500));
		contracts = channel_->query_ContractInfo();
	} // end while(0 != contracts.error_no){


	return contracts;
}

void position_manager::get_position(st_config_t &config,strategy_init_pos_t &ini_pos,const exchange_names &ex){


	tcs *channel_ = this->tca_ptr->get_tcs(this->model_id,ex);
	T_PositionReturn pos_rtn = channel_->query_position();
	while(0 != pos_rtn.error_no){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
						"query_position failed,cause:" << pos_rtn.error_no);
		this_thread::sleep_for(std::chrono::milliseconds(500));
		pos_rtn = channel_->query_position();
	}
	assign(ini_pos,pos_rtn,ex);

	// accumulate filled quantity separately on direction
	T_TradeDetailReturn trans_rtn = channel_->query_transactions();

	while(0 != trans_rtn.error_no){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
						"query_transactions failed,cause:" << trans_rtn.error_no);
		this_thread::sleep_for(std::chrono::milliseconds(500));
		trans_rtn = channel_->query_transactions();
	}
	assign(ini_pos,trans_rtn,ex);

}

bool position_manager::is_subscribed_symbol(string symbol)
{
	for(int i=0; i<model_setting_ins_.config_.symbols_cnt; i++){
		if(0==strcmp(symbol.c_str(),model_setting_ins_.config_.symbols[i].name) ||
		   0 == strcmp(symbol.c_str(),"#CASH")){
			return true;
		}
	} // end for(int i=0; i<model_setting_ins_.config_.symbols_cnt; i++){

	return false;
}

void position_manager::assign(strategy_init_pos_t &dest,T_PositionReturn &src,const exchange_names &ex)
{
	if(0 == src.error_no){
		//	group  T_PositionReturn by symbol
		std::vector<PositionDetail>::iterator it = src.datas.begin();
		std::vector<PositionDetail>::iterator end = src.datas.end();
		map<string,list<PositionDetail> > group;
		for( ; it!=end; ++it){
			string symbol = it->stock_code;
//			if(true == is_subscribed_symbol(symbol)) {
				PositionDetail &detail = *it;
				group[symbol].push_back(detail);
//			}
		}

		int &yesterday_pos_symbol_cnt = dest._yesterday_pos.symbol_cnt = 0;
		int &today_pos_symbol_cnt = dest._today_pos.symbol_cnt = 0;
		map<string,list<PositionDetail> >::iterator it_group = group.begin();
		map<string,list<PositionDetail> >::iterator end_group = group.end();
		for( ; it_group!=end_group; ++it_group){
			symbol_pos_t &yesterday_pos = dest._yesterday_pos.s_pos[yesterday_pos_symbol_cnt];
			symbol_pos_t &today_pos = dest._today_pos.s_pos[today_pos_symbol_cnt];
			list<PositionDetail>::iterator it_pos = it_group->second.begin();
			list<PositionDetail>::iterator end_pos = it_group->second.end();
			for( ;it_pos!=end_pos;++it_pos){
				if(side_options::buy==it_pos->direction){
					// previous day position
					yesterday_pos.long_volume = it_pos->yestoday_position;
					yesterday_pos.long_price = it_pos->yd_position_avg_price;
					yesterday_pos.exchg_code = ex;
					// today position
					today_pos.long_volume = it_pos->position;
					today_pos.long_price = it_pos->position_avg_price;
					today_pos.exchg_code = ex;

					today_pos.today_buy_volume = it_pos->today_buy_volume;
					today_pos.today_aver_price_buy = it_pos->today_aver_price_buy;
					today_pos.today_sell_volume = it_pos->today_sell_volume;
					today_pos.today_aver_price_sell = it_pos->today_aver_price_sell;

				}else if(side_options::sell==it_pos->direction){
					// previous day position
					yesterday_pos.short_volume = it_pos->yestoday_position;
					yesterday_pos.short_price = it_pos->yd_position_avg_price;
					yesterday_pos.exchg_code = ex;
					// today position
					today_pos.short_volume = it_pos->position;
					today_pos.short_price = it_pos->position_avg_price;
					today_pos.exchg_code = ex;
				} // end if(side_options::buy==it_pos->direction){

			} // end for( ;it_pos!=end_pos;++it_pos){
			strcpy(yesterday_pos.symbol,it_group->first.c_str());
			strcpy(today_pos.symbol,it_group->first.c_str());

			yesterday_pos_symbol_cnt++;
			today_pos_symbol_cnt++;
		} // end for( ; it_group!=end_group; ++it_group){

	}else{
		dest._yesterday_pos.symbol_cnt = 0;
		dest._today_pos.symbol_cnt = 0;
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				"query_position error,cause:" << src.error_no);
	} // end if(0 == src.error_no){
}

void position_manager::assign(strategy_init_pos_t &dest,T_TradeDetailReturn &src,const exchange_names &ex)
{
	if(0 == src.error_no){
		map<string,map<side_options,list<TradeDetail>> > group;
		vector<TradeDetail>::iterator it = src.datas.begin();
		vector<TradeDetail>::iterator end = src.datas.end();
		// first,push records into a corresponding container separately on direction
		for( ; it!=end; ++it){
			string symbol = it->stock_code;
			if(true == is_subscribed_symbol(symbol)) {
				group[it->stock_code][static_cast<side_options>(it->direction)].push_back(*it);
			}
		}

		// accumulate filled quantity of buy direction
		dest.acc_cnt = 0;
		map<string,map<side_options,list<TradeDetail>> >::iterator it_group = group.begin();
		map<string,map<side_options,list<TradeDetail>> >::iterator end_group = group.end();
		for( ; it_group!=end_group; ++it_group){
			acc_volume_t &sum_item = dest._today_acc_volume[dest.acc_cnt];
			sum_item.exchg_code = ex;
			strcpy(sum_item.symbol,it_group->first.c_str());
			this->sum(sum_item, it_group->second);

			dest.acc_cnt++;
		}

	}else{
		dest.acc_cnt = 0;
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
			"query_position error,cause:" << src.error_no);
	}
}

void position_manager::sum (acc_volume_t &sum, map<side_options,list<TradeDetail>> &addend)
{
	map<side_options,list<TradeDetail>>::iterator it_side_group = addend.begin();
	map<side_options,list<TradeDetail>>::iterator end_side_group = addend.end();
	for( ; it_side_group!=end_side_group; ++it_side_group){
		if(it_side_group->first==side_options::buy){
			list<TradeDetail>::iterator it_item_group = it_side_group->second.begin();
			list<TradeDetail>::iterator end_item_group = it_side_group->second.end();
			for( ;it_item_group!=end_item_group;++it_item_group){
				long &last_qty = it_item_group->trade_volume;
				double &last_px = it_item_group->trade_price;
				sum.buy_price = (sum.buy_volume*sum.buy_price + last_qty * last_px)
						/ (sum.buy_volume + last_qty);
				sum.buy_volume += last_qty;

			}
		}else if(it_side_group->first==side_options::sell){
			list<TradeDetail>::iterator it_item_group = it_side_group->second.begin();
			list<TradeDetail>::iterator end_item_group = it_side_group->second.end();
			for( ;it_item_group!=end_item_group;++it_item_group){
				long &last_qty = it_item_group->trade_volume;
				double &last_px = it_item_group->trade_price;
				sum.sell_price = (sum.sell_volume*sum.sell_price + last_qty * last_px)
						/ (sum.sell_volume + last_qty);
				sum.sell_volume += last_qty;
			}
		}

	}
}

void position_manager::get_pending_vol(const string &symbol,const exchange_names &exchange,
		pending_order_t &table)
{
	typedef pair<double,char> KeyT;
	typedef map<KeyT,int> TalbeT;

	TalbeT cache;
	int total = 0;
	int quote_total = 0;
	int counter = 0;
	int quote_counter = 0;

	pending_ord_request_dao::query_not_final_ord_requests_by_symbol(symbol,_report_record_cache,total);
	for (; counter < total; counter++){
		KeyT key(_report_record_cache[counter].price,((char)_report_record_cache[counter].side));
		int unfilled_qty = _report_record_cache[counter].volume-_report_record_cache[counter].cum_qty;
		if (cache.find(key)==cache.end()) cache[key] = unfilled_qty; 	// not finded
		else cache.find(key)->second += unfilled_qty; 					// finded
	}

	pending_ord_request_dao::query_not_final_quote_order_by_symbol(symbol,_quote_report_record_cache,quote_total);
	for (; quote_counter < quote_total; quote_counter++){
		KeyT key(_quote_report_record_cache[quote_counter].price,((char)_quote_report_record_cache[quote_counter].side));
		int unfilled_qty = _quote_report_record_cache[quote_counter].volume-_quote_report_record_cache[quote_counter].cum_qty;
		if (cache.find(key)==cache.end()) cache[key] = unfilled_qty; 	// not finded
		else cache.find(key)->second += unfilled_qty; 					// finded
	}

	int &cursor = table.req_cnt;
	TalbeT::iterator it = cache.begin();
	// travel the different price record
	for (; it!=cache.end(); ++it){
		memcpy(table.pending_req[cursor].symbol,symbol.c_str(),symbol.size()+1);
		 // field price
		table.pending_req[cursor].price = it->first.first;
		 // side field
		table.pending_req[cursor].direct = static_cast<side_options>((char)it->first.second);
		 // volume field
		table.pending_req[cursor].volume = it->second;

		cursor++;
	 }
}
