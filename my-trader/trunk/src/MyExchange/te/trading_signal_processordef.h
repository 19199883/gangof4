#include <boost/foreach.hpp>
#include <sstream>
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include "position_manager.h"
#include "signal_entity.h"

#include "my_trade_tunnel_api.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace trading_engine;
using namespace strategy_manager;
using namespace trading_engine;


template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
map<long,map<long,set<long> > >
trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::sig_order_mapping_;

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::trading_signal_processor(
		shared_ptr<tca> _tca_ptr,
		shared_ptr<ModelAdapterT> _model_ptr)
	:rpt_ready(false),stopped(false)
{
	this->rpt_ready = false;
	position_manager* pm_p = new position_manager(_tca_ptr,_model_ptr->setting,_model_ptr->setting.id);
	position_mamager_ptr = shared_ptr<position_manager>(pm_p);
	model_ptr = _model_ptr;
	tca_ptr = _tca_ptr;
}
		
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::~trading_signal_processor(void){}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::run(){}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process(const long &model_id,signal_t *sig)
{
	if (sig->instr == instr_t::REQUEST_OF_QUOTE){
		place_req_quote(sig);
	}else if (sig->instr == instr_t::QUOTE){
		place_quote(sig);
	}else{
		place_order(sig);
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::place_order(signal_t *sig)
{
	my_order ord;
	ord.model_id = this->model_ptr->setting.id;


	if (sig->instr == (unsigned int)instr_t::ROD){
		ord.ord_type = ord_types::general;
	}
	else if (sig->instr == (unsigned int)instr_t::FOK){
		ord.ord_type = ord_types::fok;
	}
	else if (sig->instr == (unsigned int)instr_t::FAK){
		ord.ord_type = ord_types::fak;
	}


	if (sig->instr == instr_t::MARKET){
		ord.price_type = price_options::market;
	}else{
		ord.price_type = price_options::limit;
	}

	ord.request_type = request_types::place_order;

	if(sig->sig_act==signal_act_t::sell){
		ord.side = side_options::sell;
		ord.price = sig->sell_price;
	}
	if(sig->sig_act==signal_act_t::buy){
		ord.side = side_options::buy;
		ord.price = sig->buy_price;
	}
	ord.signal_id = sig->sig_id;
	ord.symbol = sig->symbol;
	ord.exchange = sig->exchange;
	ord.sah_ = model_ptr->setting.sah_;

	if(sig->sig_openclose == alloc_position_effect_t::open_){	// open
		set<long> new_ords;
		ord.position_effect = position_effect_options::open_;
		ord.volume = sig->open_volume;
		this->tca_ptr->place_orders(ord,new_ords);
		sig_order_mapping_[this->model_ptr->setting.id][sig->sig_id].insert(new_ords.begin(),new_ords.end());

	}else if(sig->sig_openclose == alloc_position_effect_t::close_){// close
		set<long> new_ords;
		ord.position_effect = position_effect_options::close_pos;
		ord.volume = sig->close_volume;
		this->tca_ptr->place_orders(ord,new_ords);
		sig_order_mapping_[this->model_ptr->setting.id][sig->sig_id].insert(new_ords.begin(),new_ords.end());
	}else if(sig->sig_openclose == alloc_position_effect_t::close_and_open){// close and then open
		set<long> new_ords;
		my_order close_ord = ord;
		close_ord.position_effect = position_effect_options::close_pos;
		close_ord.volume = sig->close_volume;
		this->tca_ptr->place_orders(close_ord,new_ords);
		sig_order_mapping_[this->model_ptr->setting.id][sig->sig_id].insert(new_ords.begin(),new_ords.end());

		new_ords.clear();
		my_order open_ord = ord;
		open_ord.position_effect = position_effect_options::open_;
		open_ord.volume = sig->open_volume;
		this->tca_ptr->place_orders(open_ord,new_ords);
		sig_order_mapping_[this->model_ptr->setting.id][sig->sig_id].insert(new_ords.begin(),new_ords.end());
	}
}

/*
 * if the field "instr" is equal to  QUOTE, the value of open_volume field is set to
 * the buy_volume field of quote order and the value close_volume field is set to the sell_volume field of quote order
 */
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::place_quote(signal_t *sig)
{
	QuoteOrder ord;
	ord.model_id = this->model_ptr->setting.id;
	ord.signal_id = sig->sig_id;
	ord.request_type = request_types::quote;
	ord.stock_code = sig->symbol;

	ord.buy_limit_price = sig->buy_price;
	ord.buy_speculator = model_ptr->setting.sah_;
	ord.buy_volume = sig->open_volume;
	if(sig->sig_openclose == alloc_position_effect_t::open_){
		ord.buy_open_close = position_effect_options::open_;
	}else if(sig->sig_openclose == alloc_position_effect_t::close_){
		ord.buy_open_close = position_effect_options::close_pos;
	}

	ord.sell_limit_price = sig->sell_price;
	ord.sell_speculator = model_ptr->setting.sah_;
	ord.sell_volume = sig->close_volume;
	if(sig->sig_openclose == alloc_position_effect_t::open_){
		ord.sell_open_close = position_effect_options::open_;
	}else if(sig->sig_openclose == alloc_position_effect_t::close_){
		ord.sell_open_close = position_effect_options::close_pos;
	}

	strcpy(ord.for_quote_id,sig->reply_quote_id);
	ord.exchange = sig->exchange;
	ord.sah_ = model_ptr->setting.sah_;

	this->tca_ptr->quote(ord);

	sig_order_mapping_[this->model_ptr->setting.id][sig->sig_id].insert(ord.cl_ord_id);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::place_req_quote(signal_t *sig)
{
	my_order ord;
	ord.model_id = this->model_ptr->setting.id;
	ord.request_type = request_types::req_quote;
	ord.signal_id = sig->sig_id;
	ord.symbol = sig->symbol;
	ord.exchange = sig->exchange;

	this->tca_ptr->req_quote(ord);
}
