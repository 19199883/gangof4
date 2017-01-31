#include <sstream>
#include <log4cxx/logger.h>
#include <chrono>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include <boost/foreach.hpp>
#include "model_adapter.h"
#include "pending_quote_dao.h"
#include "quote_entity.h"
#include "trading_signal_processor.h"
#include "exchange_names.h"
#include "pending_ord_request_dao.h"
#include <vector>
#include "position_entity.h"
#include "../my_exchange.h"
#include "signal_entity.h"
#include "engine.h"
#include "cancel_signal_processor.h"
#include "unistd.h"

#include "my_trade_tunnel_api.h"
#include <string>

using namespace std::chrono;
using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

using namespace trading_engine;
using namespace trading_channel_agent;

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::strategy_unit(
		shared_ptr<tca> tca_ptr,
		shared_ptr<QaT> qa_ptr,
		shared_ptr<ModelAdapterT> model_ptr,
		int timeEventInterval)
:_tca_ptr(tca_ptr),
 _qa_ptr(qa_ptr),
 _model_ptr(model_ptr),
 _period_counter(0),
 _pos_mgr(tca_ptr, model_ptr->setting,model_ptr->setting.id),
 stopped(false)
{
	// license,prevent any nasty usage,added by wangying on 20160805
	this->check_lic();	
	ready_ = false;
	timeEventInterval_ = timeEventInterval;

	process_sig_response_threads_ = vector<thread*>(100);
	process_quote_notify_threads_ = vector<thread*>(100);

//	duplicated_quote_nitify_records_ = vector<T_RtnForQuote>(100);
//	duplicated_quote_nitify_records_.reserve(100);

	sig_cache = vector<signal_t>(1000);
	sig_cache.reserve(1000);
	sig_cache_size = 0;

	pending_order_cache.req_cnt = 0;

	cache_key_ = ReportNotifyTableKeyT(model_ptr->setting.id,"strategy_unit");
}


template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::check_lic()
{
	char target[1024];
	char buf[1024];                             
	memset(buf,0,sizeof(buf));
	std::ifstream is;

	getcwd(target, sizeof(target));
	is.open ("lic");
	if ( (is.rdstate() & std::ifstream::failbit ) != 0 ){
         throw std::logic_error("illegal user!");
     }else{
        is.getline(buf,1024);
         if ( is.good() ){
            if(strcmp(target,buf)!=0){
                 throw std::logic_error("illegal useri!");            
             }
         }else{
             throw std::logic_error("illegal user!");
         }
 
     }
}


template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::~strategy_unit()
{
	this->finalize();
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
long
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::get_id()
{
	return this->_model_ptr->setting.id;
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::run(void){
	stopped = false;
	stringstream ss_log;
	LoggerPtr logger = log4cxx::Logger::getRootLogger()->getRootLogger();

	// register this strategy unit into tca model
	set<string> symbols;
	int &sym_pos_count = this->_model_ptr->setting.config_.symbols_cnt;
	symbol_t *sym_pos = this->_model_ptr->setting.config_.symbols;

	for (int i=0; i<sym_pos_count; i++){
		if (exchanges_.find(sym_pos[i].exchange) == exchanges_.end()){
			exchanges_.insert(sym_pos[i].exchange);
		}
		symbols.insert(sym_pos[i].name);
	}

	long id = get_id();
	this->_tca_ptr->register_strategy(id,exchanges_);
	pending_ord_request_dao::init(id,symbols);

	CancelSignalProcessorT *csp = new CancelSignalProcessorT(_tca_ptr, _model_ptr,_qa_ptr);
	_csp_ptr = shared_ptr<CancelSignalProcessorT>(csp);

	// 交易信号处理器
	TradeSignalProcessorT *tsp = new TradeSignalProcessorT(_tca_ptr, _model_ptr);
	_tsp_ptr = shared_ptr<TradeSignalProcessorT>(tsp);
	_tsp_ptr->start();

	// multiple channels
	// get corresponding channel for this model
	vector<tcs*> channels = _tca_ptr->get_tcs(this->_model_ptr->setting.id);
	for(int i=0; i<channels.size(); i++){
		thread *tmp = new thread(bind(&strategy_unit::process_sig_response,this,channels[i]));
		process_sig_response_threads_[i] = tmp;
		LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
				"process_sig_response_thread_,model id:" << id
				<< "; thread id:" << tmp->get_id());
		tmp->detach();
        
        if (this->_model_ptr->setting.isOption){            
    		tmp = new thread(bind(&strategy_unit::process_quote_notify,this,channels[i]));
    		process_quote_notify_threads_[i] = tmp;
    		LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
    				"process_quote_notify_thread_,model id:" << id
    				<< "; thread id:" << tmp->get_id());
    		tmp->detach();
        }
	}

	_signal_process_thread = thread(bind(&strategy_unit::trade_main_process,
		this,_model_ptr, _qa_ptr, _tca_ptr, _csp_ptr,_tsp_ptr));
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
			"_signal_process_thread,model id:" << id
		    << "; thread id:" << _signal_process_thread.get_id());
	_signal_process_thread.detach();

	// start quote processing thread
	spif_quote_process_thread = thread(bind(&strategy_unit::process_spif_quote,this));
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
			"spif_quote_process_thread,model id:" << id
			<< "; thread id:" << spif_quote_process_thread.get_id());
	spif_quote_process_thread.detach();

	cf_quote_process_thread = thread(bind(&strategy_unit::process_cf_quote,this));
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
				"cf_quote_process_thread,model id:" << id
				<< "; thread id:" << cf_quote_process_thread.get_id());
	cf_quote_process_thread.detach();

	stock_quote_process_thread = thread(bind(&strategy_unit::process_stock_quote,this));
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
				"stock_quote_process_thread,model id:" << id
				<< "; thread id:" << stock_quote_process_thread.get_id());
	stock_quote_process_thread.detach();

	full_depth_quote_process_thread = thread(bind(&strategy_unit::process_full_depth_quote,this));
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
					"full_depth_quote_process_thread,model id:" << id
					<< "; thread id:" << full_depth_quote_process_thread.get_id());
	full_depth_quote_process_thread.detach();

	quote5_process_thread = thread(bind(&strategy_unit::process_quote5,this));
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
					"quote5_process_thread,model id:" << id
					<< "; thread id:" << quote5_process_thread.get_id());
	quote5_process_thread.detach();

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"model " << _model_ptr->setting.name << "trade main thread:" <<  _signal_process_thread.get_id());
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::finalize(void){
	while (_signal_process_thread.joinable()){
		this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	std::this_thread::yield();
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_init_pos(st_config_t &config){
	strategy_init_pos_t init_pos;
	bool processed_stock = false;
	set<exchange_names>::iterator it = exchanges_.begin();
	set<exchange_names>::iterator end = exchanges_.end();
	for(; it!=end; ++it){
		//DODO hwg ,add  SHANGHAI STOCK EXCHANGE
		if(processed_stock&& (*it==exchange_names::XSHE || *it==exchange_names::XSHG || *it == exchange_names::XHKE)) continue;

		if(!processed_stock && (*it==exchange_names::XSHE || *it==exchange_names::XSHG || *it == exchange_names::XHKE )){
			processed_stock = true;
		}
		strategy_init_pos_t init_pos_tmp;
		_pos_mgr.get_position(config,init_pos_tmp,*it);
		append(init_pos,init_pos_tmp);
	} // end for(; it!=end; ++it){

	int sig_count = 0;
	this->_model_ptr->feed_init_position(&init_pos,&sig_count,sig_cache.data()+sig_cache_size);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::append(strategy_init_pos_t &dest,strategy_init_pos_t &src){
	int acc_count = src.acc_cnt;
	for(int i=0; i<acc_count; i++){
		dest._today_acc_volume[dest.acc_cnt] = src._today_acc_volume[i];
		dest.acc_cnt = dest.acc_cnt+1;
	}

	int today_pos_cnt = src._today_pos.symbol_cnt;
	for(int i=0; i<today_pos_cnt; i++){
		dest._today_pos.s_pos[dest._today_pos.symbol_cnt] = src._today_pos.s_pos[i];
		dest._today_pos.symbol_cnt = dest._today_pos.symbol_cnt + 1;
	}

	int yesterday_pos_cnt = src._yesterday_pos.symbol_cnt;
	for(int i=0; i<yesterday_pos_cnt; i++){
		dest._yesterday_pos.s_pos[dest._yesterday_pos.symbol_cnt] = src._yesterday_pos.s_pos[i];
		dest._yesterday_pos.symbol_cnt = dest._yesterday_pos.symbol_cnt + 1;
	}
}

/***************************************************
 * process timer event
 ****************************************************/
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_time_event()
{
	while(!stopped){
		int sig_count = 0;
		std::chrono::milliseconds span = std::chrono::milliseconds(timeEventInterval_);
		this_thread::sleep_for(span);

		{
			lock_guard<mutex> lock(mut_for_main_proc);

			if(!ready_) continue;

			sig_count = 0;
			this->_model_ptr->feed_time_event(sig_cache.data()+sig_cache_size,sig_count);
			sig_cache_size += sig_count;
			if (sig_count > 0){
				process_sig();
			}
		}
	}
}

/***************************************************
 * process signal responses
 ****************************************************/
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_sig_response(tcs *tcs_ins)
{
	if (tcs_ins == NULL) return;

	ReportNotifyRecordT duplicated_records(5000);
	duplicated_records.reserve(5000);

	int total = 0;
	 while (!stopped){
		 {
			unique_lock<mutex> lock(tcs_ins->rpt_notify_ready_mtx);
			while(!tcs_ins->report_notify_state[cache_key_]){
				//usleep(1);
				//sched_yield();
				tcs_ins->rpt_notify_ready_cv.wait(lock);
			}

			 // enter lock scope
			{
				//unique_lock<mutex> lock(tcs_ins->rpt_notify_ready_mtx);

		        total = 0;
			    pending_ord_request_dao::read_report_notify(cache_key_,total,duplicated_records);
			    tcs_ins->report_notify_state[cache_key_] = false;
			}
				// exit lock scope
		 }
		this->produce_signal_by_sig_response(duplicated_records,total);
	}	// end while (!stopped){
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::produce_signal_by_sig_response(ReportNotifyRecordT duplicated_records_,int total){
	{
		lock_guard<mutex> lock(mut_for_main_proc);

		// feed report
		for(int cur=0; cur<total; cur++){
			int sig_count = 0;
			my_order &ord = duplicated_records_[cur];
			if(ord.model_id != this->get_id() ||
					(ord.state==state_options::new_state && ord.ord_id==0) ||
					ord.state==state_options::pending_new
				){
				continue;
			}
			pos_cache_.s_pos[0].long_price = ord.position_.long_avg_price;
			pos_cache_.s_pos[0].long_volume = ord.position_.long_position;
			pos_cache_.s_pos[0].short_price = ord.position_.short_avg_price;
			pos_cache_.s_pos[0].short_volume = ord.position_.short_position;

			pos_cache_.s_pos[0].today_buy_volume = ord.position_.today_buy_volume;
			pos_cache_.s_pos[0].today_sell_volume = ord.position_.today_sell_volume;

			strcpy(pos_cache_.s_pos[0].symbol,ord.symbol.c_str());
			pos_cache_.s_pos[0].changed = ord.position_.update_flag;
			//this->_model_ptr->feed_position(&pos_cache_,sig_cache.data()+sig_cache_size,sig_count);
			sig_cache_size += sig_count;
			pos_cache_.symbol_cnt = 1;

			// pending volume
			pending_order_cache.req_cnt = 0;
			_pos_mgr.get_pending_vol(ord.symbol, ord.exchange,pending_order_cache);

			// feed reports
			produce_sig_rep(ord,sig_rep_cache_);

			LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"feed_sig_response:" << ord.cl_ord_id);

			this->_model_ptr->feed_sig_response(&sig_rep_cache_,&pos_cache_.s_pos[0],&pending_order_cache,
					sig_cache.data()+sig_cache_size,sig_count);

			sig_cache_size += sig_count;
		} // end BOOST_FOREACH( shared_ptr<SPIFQuoteT> quote_ptr, quotes )

		// 通知trade_main_process开始执行一次主流程
//		cond_for_main_proc.notify_all();
		if (sig_cache_size > 0){
			process_sig();
		}
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::produce_sig_rep(my_order &src,signal_resp_t &dest){
	// dest.exchg_timestamp = ;
	//dest.local_timestamp = ;
	dest.sig_id = src.signal_id;
	strcpy(dest.symbol,src.symbol.c_str());
	if(src.request_type==request_types::place_order){
		if(src.side==side_options::buy){
			dest.sig_act = signal_act_t::buy;
		}
		if(src.side==side_options::sell){
			dest.sig_act = signal_act_t::sell;
		}
	}else if(src.request_type==request_types::cancel_order){
		dest.sig_act = signal_act_t::cancel;
	}
	else if(src.request_type==request_types::quote){
		dest.sig_act = signal_act_t::quote;
	}

	dest.order_price = src.price;
	dest.order_volume = src.volume;
	dest.exec_volume = src.last_qty;
	dest.exec_price = src.last_px;
	dest.aver_price = src.cum_amount /src.cum_qty;
	dest.acc_volume = src.cum_qty;

	if (src.state == state_options::cancelled){
		dest.killed = dest.order_volume - dest.acc_volume;
	}else{
		dest.killed = 0;
	}
	if (src.state == state_options::rejected){
		dest.rejected = dest.order_volume;
	}else{
		dest.rejected = 0;
	}

	dest.error_no = src.error_no;

	if(src.state==state_options::cancelled){
		dest.status = if_sig_state_t::SIG_STATUS_CANCEL;
	}else if(src.state==state_options::filled){
		dest.status = if_sig_state_t::SIG_STATUS_SUCCESS;
	}else if(src.state==state_options::partial_filled){
		dest.status = if_sig_state_t::SIG_STATUS_PARTED;
	}else if(src.state==state_options::rejected){
		dest.status = if_sig_state_t::SIG_STATUS_REJECTED;
	}else if(src.state==state_options::new_state){
		dest.status = if_sig_state_t::SIG_STATUS_ENTRUSTED;
	}else{
		dest.status =src.state;
	}
}


/***************************************************
 * process SPIF category of quote
 ****************************************************/
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_spif_quote()
{
	if (_qa_ptr->spif_quote_source_ptr==0) return;

	typename SPIFPendingQuoteDaoT::QuoteTableRecordT quotes(50000);
	quotes.reserve(50000);
	int total = 0;
	 while (!stopped){
         {
            unique_lock<mutex> lock(_qa_ptr->spif_quote_source_ptr->quote_ready_mtx);

            while(!_qa_ptr->spif_quote_source_ptr->quote_state[this->_model_ptr->setting.id]){

             _qa_ptr->spif_quote_source_ptr->quote_ready_cv.wait(lock);
            }

			total = 0;
			_qa_ptr->spif_quote_source_ptr->quote_state[this->_model_ptr->setting.id] = false;
			SPIFPendingQuoteDaoT::query(this->_model_ptr->setting.id,total,quotes);
		}


		this->produce_signal_by_spif(quotes,total);
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::produce_signal_by_spif(typename SPIFPendingQuoteDaoT::QuoteTableRecordT &quotes,int total){
	// process quote and produce signals
	{
		lock_guard<mutex> lock(mut_for_main_proc);

		if(!ready_) return;


		// 推送行情给模型
		for(int cur=0; cur<total; cur++){
			string symbol = SPIFPendingQuoteDaoT::get_symbol(&quotes[cur]);
			// 推行情
			int sig_count = 0;
			this->_model_ptr->feed_spif_quote(quotes[cur],sig_cache.data()+sig_cache_size,sig_count);
			sig_cache_size += sig_count;
		} // end BOOST_FOREACH( shared_ptr<SPIFQuoteT> quote_ptr, quotes )
		// 通知trade_main_process开始执行一次主流程

		if (sig_cache_size > 0){
			process_sig();
		}
	}
}

/***************************************************
 * process CF category of quote
 ****************************************************/
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_cf_quote()
{
	if (_qa_ptr->cf_quote_source_ptr==0) return;

	typename pending_quote_dao<CFQuoteT>::QuoteTableRecordT quotes(50000);
	quotes.reserve(50000);
	int total = 0;
	 while (!stopped){
		 {
		    unique_lock<mutex> lock(_qa_ptr->cf_quote_source_ptr->quote_ready_mtx);
		    while(!_qa_ptr->cf_quote_source_ptr->quote_state[this->_model_ptr->setting.id]){
			    _qa_ptr->cf_quote_source_ptr->quote_ready_cv.wait(lock);
		    }

			total = 0;
			_qa_ptr->cf_quote_source_ptr->quote_state[this->_model_ptr->setting.id] = false;
			CFPendingQuoteDaoT::query(this->_model_ptr->setting.id,total,quotes);
		 } 

		 this->produce_signal_by_cf(quotes,total);
	} 
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::produce_signal_by_cf(typename pending_quote_dao<CFQuoteT>::QuoteTableRecordT& quotes,int total)
{
	// process quote and produce signals
	{

		lock_guard<mutex> lock(mut_for_main_proc);

		if(!ready_) return;

		// 推送行情给模型
		for(int cur=0; cur<total; cur++){
			// 推行情
			int sig_count = 0;
			this->_model_ptr->feed_cf_quote(quotes[cur],sig_cache.data()+sig_cache_size,sig_count);
			sig_cache_size += sig_count;
		} // end BOOST_FOREACH( shared_ptr<SPIFQuoteT> quote_ptr, quotes )

		if (sig_cache_size > 0){
			process_sig();
		}
	}
}

/***********************************************************
 * process the stock category of quotes
 ************************************************************/
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_stock_quote()
{
	if (_qa_ptr->stock_quote_source_ptr==0) return;

	typename pending_quote_dao<StockQuoteT>::QuoteTableRecordT quotes(50000);
	quotes.reserve(50000);
	int total = 0;
	 while (!stopped){
	   {
	      unique_lock<mutex> lock(_qa_ptr->stock_quote_source_ptr->quote_ready_mtx);
		  while(!_qa_ptr->stock_quote_source_ptr->quote_state[this->_model_ptr->setting.id]){
		    _qa_ptr->stock_quote_source_ptr->quote_ready_cv.wait(lock);
		  }

			total = 0;
			_qa_ptr->stock_quote_source_ptr->quote_state[this->_model_ptr->setting.id] = false;
			StockPendingQuoteDaoT::query(this->_model_ptr->setting.id,total,quotes);
        }
		this->produce_signal_by_stock(quotes,total);
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::produce_signal_by_stock(typename pending_quote_dao<StockQuoteT>::QuoteTableRecordT quotes,int total)
{
	// process quote and produce signals
	{
		lock_guard<mutex> lock(mut_for_main_proc);

		if(!ready_) return;

		// 推送行情给模型
		for(int cur=0; cur<total; cur++){
			string symbol  = pending_quote_dao<StockQuoteT>::get_symbol(&quotes[cur]);
			// 推行情
			int sig_count = 0;
			this->_model_ptr->feed_stock_quote(quotes[cur],sig_cache.data()+sig_cache_size,sig_count);
			sig_cache_size += sig_count;
		} // end BOOST_FOREACH( shared_ptr<SPIFQuoteT> quote_ptr, quotes )

		if (sig_cache_size > 0){
			process_sig();
		}
	}
}


/***********************************************************
 * process full depth category of quotes
 ************************************************************/
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_full_depth_quote()
{
	if (_qa_ptr->full_depth_quote_source_ptr==0) return;

	typename pending_quote_dao<FullDepthQuoteT>::QuoteTableRecordT quotes(50000);
	quotes.reserve(50000);
	int total = 0;
	 while (!stopped){
	   {
	      unique_lock<mutex> lock(_qa_ptr->full_depth_quote_source_ptr->quote_ready_mtx);
		  while(!_qa_ptr->full_depth_quote_source_ptr->quote_state[this->_model_ptr->setting.id]){
		  _qa_ptr->full_depth_quote_source_ptr->quote_ready_cv.wait(lock);
		}

			total = 0;
			_qa_ptr->full_depth_quote_source_ptr->quote_state[this->_model_ptr->setting.id] = false;
			FullDepthPendingQuoteDaoT::query(this->_model_ptr->setting.id,total,quotes);
      }
	  this->produce_signal_by_full_depth(quotes,total);
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::produce_signal_by_full_depth(typename pending_quote_dao<FullDepthQuoteT>::QuoteTableRecordT& quotes,int total)
{
	// process quote and produce signals
	{
		lock_guard<mutex> lock(mut_for_main_proc);

		if(!ready_) return;

		// 推送行情给模型
		for(int cur=0; cur<total; cur++){
			 string symbol = FullDepthPendingQuoteDaoT::get_symbol(&quotes[cur]);
			// 推行情
			int sig_count = 0;
			_model_ptr->feed_full_depth_quote(quotes[cur],sig_cache.data()+sig_cache_size,sig_count);
			sig_cache_size +=sig_count;
		} // end for(int cur=0; cur<count;cur++)

		if (sig_cache_size > 0){
			process_sig();
		}
	}
}


/***********************************************************
 * process quote5 category of quotes
 ************************************************************/
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_quote5()
{
	if (_qa_ptr->quote_source5_ptr==0) return;

	typename PendingQuoteDaoT5::QuoteTableRecordT quotes(50000);
	quotes.reserve(50000);
	int total = 0;
	 while (!stopped){
	   {
	     unique_lock<mutex> lock(_qa_ptr->quote_source5_ptr->quote_ready_mtx);
		 while(!_qa_ptr->quote_source5_ptr->quote_state[this->_model_ptr->setting.id]){
		    _qa_ptr->quote_source5_ptr->quote_ready_cv.wait(lock);
		 }

			total = 0;
			_qa_ptr->quote_source5_ptr->quote_state[this->_model_ptr->setting.id] = false;
			PendingQuoteDaoT5::query(this->_model_ptr->setting.id,total,quotes);
       }
	   this->produce_signal_by_quote5(quotes,total);
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::produce_signal_by_quote5(typename PendingQuoteDaoT5::QuoteTableRecordT& quotes,int total)
{
	// process quote and produce signals
	{
		lock_guard<mutex> lock(mut_for_main_proc);

		if(!ready_) return;

		// 推送行情给模型
		for(int cur=0; cur<total; cur++){
			string symbol = PendingQuoteDaoT5::get_symbol(&quotes[cur]);
			// 推行情
			int sig_count = 0;
			_model_ptr->feed_MDOrderStatistic(quotes[cur],sig_cache.data()+sig_cache_size,sig_count);
			sig_cache_size +=sig_count;
		} // end for(int cur=0; cur<count;cur++)
		if (sig_cache_size > 0){
			process_sig();
		}
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_sig()
 {
	try{
		int start = 0;
		int cancel_sig_cnt = 0;
		int place_sig_cnt = 0;
		while(start < sig_cache_size){
			// cancel orders
			cancel_sig_cnt = get_cancel_sig_count(sig_cache,start,sig_cache_size);
			if(cancel_sig_cnt>0){
				_csp_ptr->process(this->get_id(),sig_cache.data()+start,cancel_sig_cnt);
				start += cancel_sig_cnt;
			}

			place_sig_cnt = get_place_sig_count(sig_cache,start,sig_cache_size);
			// place orders
			for (int cur_idx=0; cur_idx<place_sig_cnt; cur_idx++){
				_tsp_ptr->process(this->get_id(), sig_cache.data()+start+cur_idx);
			}

			start += place_sig_cnt;
		}	// end while(start < sig_cache_size){

		// clear signal cache
		sig_cache_size = 0;
	}
	catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"trade_main_process occured error:" << ex.what());
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::trade_main_process(
		shared_ptr<ModelAdapterT> model_ptr,
		shared_ptr<QaT> qa_ptr,
		shared_ptr<tca> tca_ptr,
		shared_ptr<CancelSignalProcessorT> csp_ptr,
		shared_ptr<TradeSignalProcessorT> tsp_ptr){
	// 初始化模型

	T_ContractInfoReturn contractRtn;

	set<exchange_names>::iterator it = exchanges_.begin();
	set<exchange_names>::iterator end = exchanges_.end();
	for(; it!=end; ++it){
		int rtn = 0;
		T_ContractInfoReturn tmp = _pos_mgr.query_ContractInfo(&model_ptr->setting.config_,*it,&rtn);
		for(int i=0; i<tmp.datas.size();i++){
			contractRtn.datas.push_back(tmp.datas[i]);
			LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
					"query contract,item:" << tmp.datas[i].stock_code);
		}
	}

	model_ptr->init(contractRtn);

	tsp_ptr->run();
	this_thread::sleep_for(std::chrono::microseconds(20));

	csp_ptr->run();
	this_thread::sleep_for(std::chrono::microseconds(20));
	// feed position once when mytrader starts
	this->feed_init_pos(model_ptr->setting.config_);
	ready_ = true;
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"call trade_main_process and set ready_:" << ready_);
	timer_thread_ = thread(bind(&strategy_unit::process_time_event,this));
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger()," thread id:" << timer_thread_.get_id());
	timer_thread_.detach();
    /*
	while (!stopped){	usleep(2000000);	} // end while (!stopped)
	_tsp_ptr->stop();
	*/
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
int
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::get_cancel_sig_count(vector<signal_t> &sigs,int &start,int sig_cnt)
 {
	bool start_with_cancel = false;
	int cancel_sig_cnt = 0;

	if(start<sig_cnt){
		start_with_cancel = (sigs[start].sig_act==signal_act_t::cancel);
	}else{
		start_with_cancel = false;
	}

	if(true==start_with_cancel){
		for(int i=start; i<sig_cnt; i++){
			if(sigs[i].sig_act==signal_act_t::cancel){
				cancel_sig_cnt++;
			}else{
				break;
			}
		}
	}

	return cancel_sig_cnt;
 }

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
int
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::get_place_sig_count(vector<signal_t> &sigs,int &start,int &sig_cnt)
 {
	int place_sig_cnt = 0;
	for(int i=start; i<sig_cnt; i++){
		if(sigs[i].sig_act!=signal_act_t::cancel){
			place_sig_cnt++;
		}else{
			break;
		}
	}
	return place_sig_cnt;
 }


/*
 * market making
 */
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_quote_notify(tcs *channel)
{
	 if (channel == NULL) return;

	 int &model_id = this->_model_ptr->setting.id;
	 int total = 0;
	 vector<T_RtnForQuote> duplicated_quote_nitify_records(5000);
	 duplicated_quote_nitify_records.reserve(5000);
	 while (!stopped){
		while(!channel->quote_notify_state_[model_id]){
			usleep(1);
		}

		{	// enter lock scope
			 speculative_spin_mutex::scoped_lock lock(channel->quote_notify_ready_mtx);
			total = 0;
			pending_ord_request_dao::read_quote_notify(this->_model_ptr->setting.id,total,duplicated_quote_nitify_records);
			channel->quote_notify_state_[model_id] = false;
		}	// exit lock scope
		 this->produce_signal_by_quote_notify(duplicated_quote_nitify_records,total);
	 }  // end while (!stopped)
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::produce_signal_by_quote_notify(vector<T_RtnForQuote> &records,int &count)
{
	// process quote and produce signals
	{
		lock_guard<mutex> lock(mut_for_main_proc);

		for(int cur=0; cur<count; cur++){
			int sig_count = 0;
			this->_model_ptr->feed_quote_notify(&records[cur],sig_cache.data()+sig_cache_size,sig_count);
			sig_cache_size += sig_count;
		}

		if (sig_cache_size > 0){
			this->process_sig();
		}
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
int
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::InitOptionStrategy(startup_init_t *init_content, int *err)
{
	lock_guard<mutex> lock(mut_for_main_proc);
	return this->_model_ptr->InitOptionStrategy(init_content,err);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
int
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::UpdateOptionStrategy(ctrl_t *update_content, int *err)
{
	lock_guard<mutex> lock(mut_for_main_proc);
	return this->_model_ptr->UpdateOptionStrategy(update_content,err);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
int
strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::GetMonitorInfo(monitor_t *monitor_content, int *err)
{
	lock_guard<mutex> lock(mut_for_main_proc);
	return this->_model_ptr->GetMonitorInfo(monitor_content,err);
}
