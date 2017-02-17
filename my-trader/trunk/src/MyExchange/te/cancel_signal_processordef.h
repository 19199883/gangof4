#include <boost/foreach.hpp>
#include "tca.h"
#include "pending_ord_request_dao.h"
#include <log4cxx/logger.h>
#include <functional>
#include <mutex>          // std::mutex
#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>

using namespace std::chrono;
using namespace std;
using namespace log4cxx;

using namespace log4cxx::helpers;
using namespace trading_channel_agent;
using namespace trading_engine;
using namespace strategy_manager;

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::cancel_signal_processor(
		shared_ptr<tca> _tca_ptr,
		shared_ptr<model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> > _model_ptr,
		shared_ptr<qa<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> > qa_ptr)
	:rpt_ready(true)
{
	_qa_ptr = qa_ptr;
	model_ptr = _model_ptr;
	tca_ptr = _tca_ptr;
	this->_report_record_cache = ReportNotifyRecordT(10000);
	this->_report_record_cache.reserve(10000);

	this->_quote_record_cache = QuoteOrderRecordT(10000);
	this->_quote_record_cache.reserve(10000);

	this->stopped = true;
	this->rpt_notify_key = ReportNotifyTableKeyT(this->model_ptr->setting.id,"cancel_signal_processor");
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::~cancel_signal_processor(void)
{
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::run(void)
{
	this->stopped = false;
	vector<tcs*> chanells = this->tca_ptr->get_tcs(model_ptr->setting.id);
	vector<tcs*>::iterator it = chanells.begin();
	vector<tcs*>::iterator end = chanells.end();
	for(; it != end; ++it){
		process_ord_report_thread = thread(
				bind(&cancel_signal_processor::cancel_result_handler,this,*it));
		LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
				"cancel_signal_processor,model id:" << this->model_ptr->setting.id
				<< "; thread id:" << process_ord_report_thread.get_id());

		process_ord_report_thread.detach();
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process(const long &model_id,signal_t *sigs, int &sig_cnt)
{
	if(0==sig_cnt) return;

	async_cancel_ords(model_id,sigs, sig_cnt);
	sync_cancel_ords(model_id,sigs, sig_cnt);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::async_cancel_ords(const long &model_id,signal_t *sigs, int &sig_cnt)
 {
	if(0==sig_cnt) return;

	for(int i=0; i<sig_cnt; i++){
		if(sigs[i].cancel_pattern==cancel_pattern_t::SIG_CANCEL_MODE_ASYNC){
			set<long> cl_ord_ids = TradeSigProcessorT::sig_order_mapping_[model_id][(long)sigs[i].orig_sig_id];
			for (set<long>::iterator it=cl_ord_ids.begin(); it!=cl_ord_ids.end(); ++it){
				long cl_ord_id = *it;
				if(sigs[i].instr==instr_t::QUOTE){
					QuoteOrder ord;
					pending_ord_request_dao::query_quote_order(cl_ord_id,ord);
					QuoteOrder cancel_ord = ord;
					cancel_ord.signal_id = sigs[i].sig_id;
					cancel_quote(cancel_ord);
				}else{
					my_order ord;
					pending_ord_request_dao::query_request(cl_ord_id,ord);
					my_order cancel_ord = ord;
					cancel_ord.signal_id = sigs[i].sig_id;
					cancel(cancel_ord);
				}
			}
		}
	}
 }

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::sync_cancel_ords(const long &model_id,signal_t *sigs, int &sig_cnt)
 {
	int sync_sig_cnt = 0;
	for(int i=0; i<sig_cnt; i++){
		if(sigs[i].cancel_pattern==cancel_pattern_t::SIG_CANCEL_MODE_SYNC){
			sync_sig_id_cache_[(sigs+i)->orig_sig_id] = (sigs+i)->sig_id;
			sync_sig_cnt++;
		}
	}
	sync_cancel_order_core(model_id,sync_sig_id_cache_,sync_sig_cnt);
 }

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::cancel_quote(QuoteOrder &ord)
{
	tca_ptr->cancel_quote(ord);
	TradeSigProcessorT::sig_order_mapping_[model_ptr->setting.id][ord.signal_id].insert(ord.cl_ord_id);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::cancel(my_order &ord)
{
	tca_ptr->cancel_ord(ord);
	TradeSigProcessorT::sig_order_mapping_[model_ptr->setting.id][ord.signal_id].insert(ord.cl_ord_id);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::process_one_report(const my_order &ord)
{
	bool exist_ord = pending_ord_request_dao::exist(ord.orig_cl_ord_id);
	bool exist_quote = pending_ord_request_dao::exist_quote_order(ord.orig_cl_ord_id);
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			"sync_reqs.erase:" << ord.orig_cl_ord_id
			<< "; state:" << ord.state
			<< "; request_type:" << ord.request_type
			<< "; exist_ord:" << exist_ord);
	if (ord.state==state_options::rejected &&
			((ord.request_type==request_types::cancel_order && (!exist_ord)) ||(ord.request_type==request_types::quote && (!exist_quote)))
		){

		sync_reqs.erase(ord.orig_cl_ord_id);
		if (sync_reqs.empty()) rpt_ready = true;
	}else{
		map<long, long>::iterator cur = sync_reqs.find(ord.cl_ord_id);
		if (cur != sync_reqs.end()){
			LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
					"sync cancel" <<cur->first << "," << cur->second);
		}
		if (cur == sync_reqs.end()) return;

		if(ord.request_type==request_types::place_order){
			if((ord.state == state_options::cancelled ||
					(ord.state == state_options::filled && ord.last_qty > 0) ||
					ord.state == state_options::rejected)
			) sync_reqs.erase(cur->first);
		}else if(ord.request_type==request_types::quote){
			bool existed = pending_ord_request_dao::exist_quote_order(ord.cl_ord_id);
			if(!existed) sync_reqs.erase(cur->first);
		}

		if (sync_reqs.empty()) {
			rpt_ready = true;
		}
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::cancel_result_handler(tcs*tcs)
{
	ReportNotifyRecordT duplicated_records(500);
	duplicated_records.reserve(500);

	while (!stopped){
		int total = 0;
		unique_lock<mutex> lock(tcs->rpt_notify_ready_mtx);
		while(!tcs->report_notify_state[rpt_notify_key]){
			//usleep(1);
			tcs->rpt_notify_ready_cv.wait(lock);
		}

		 // enter lock scope

			if (rpt_ready){ // currently there is not cancel request
				pending_ord_request_dao::clear_cache(rpt_notify_key);
				tcs->report_notify_state[rpt_notify_key] = false;
				continue;
			}
			else{
				// read report notify
				pending_ord_request_dao::read_report_notify(rpt_notify_key,total,duplicated_records);
				tcs->report_notify_state[rpt_notify_key] = false;
			}
		 // exit lock scope

		{
			lock_guard<mutex> lock(mut_for_sync_cancel);

			sync_req_rep_lock.lock();
			for (int counter=0 ; counter < total; counter++){
				process_one_report(duplicated_records[counter]);
				tcs->trace("process_rpt_notify,process_one_report",duplicated_records[counter]);
				if (rpt_ready){
					break;
				}
			}
			sync_req_rep_lock.unlock();
		}
		if (rpt_ready) {
			cond_for_sync_cancel.notify_all();
		}
	}	// end while (!stopped)
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::wait_for_ord_id(const long &model_id,map<unsigned long,unsigned long> sig_ids)
{
	bool has_null_ord_id = true;
	while(has_null_ord_id){
		has_null_ord_id = false;
		int total = 0;
		tca_ptr->get_not_final_requests(model_id,this->_report_record_cache,total);
		for(int counter=0; counter<total; counter++){
			my_order &tmpOrd = this->_report_record_cache[counter];
			LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
					"wait_for_ord_id,order to be checked whose cl_ord_id is:"
					<< tmpOrd.cl_ord_id << "; and sig is:" << tmpOrd.signal_id);
			if (sig_ids.find(tmpOrd.signal_id)!=sig_ids.end() && tmpOrd.ord_id==0){
				has_null_ord_id = true;
				LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"ord_id is empty,cl_ord_id:"
						<< this->_report_record_cache[counter].cl_ord_id);
				break;
			}
		}
		if (has_null_ord_id){
			this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else{
			break;
		}
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::wait_for_quote_id(const long &model_id,map<unsigned long,unsigned long> sig_ids)
{
	// market making, quote
	bool has_null_ord_id = true;
	while(has_null_ord_id){
		has_null_ord_id = false;
		int total = 0;
		pending_ord_request_dao::query_not_final_quote_order(model_id,this->_quote_record_cache,total);
		for(int counter=0; counter<total; counter++){
			QuoteOrder &tmpOrd = this->_quote_record_cache[counter];
			LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				"wait_for_ord_id,order to be checked whose cl_ord_id is:"
				<< tmpOrd.cl_ord_id << "; and sig is:" << tmpOrd.signal_id);
			if (sig_ids.find(tmpOrd.signal_id)!=sig_ids.end() && tmpOrd.ord_id==0){
				has_null_ord_id = true;
				LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"ord_id is empty,cl_ord_id:"
						<< tmpOrd.cl_ord_id);
				break;
			}
		}
		if (has_null_ord_id){
			this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else{
			break;
		}
	}
}

template<typename SPIFQuoteT, typename CFQuoteT, typename StockQuoteT,
		typename FullDepthQuoteT, typename QuoteT5>
int
cancel_signal_processor<SPIFQuoteT, CFQuoteT, StockQuoteT, FullDepthQuoteT,QuoteT5>
::cancel_orders_imp(const long & model_id,map<unsigned long, unsigned long> sig_ids)
 {
	int cancel_counter = 0;
	int total = 0;
	pending_ord_request_dao::query_not_final_ord_requests(model_id, this->_report_record_cache,total);
	for (int counter = 0; counter < total; counter++) {
		my_order &tmpOrd = this->_report_record_cache[counter];
		map<long, long>::iterator it = sync_reqs.find(tmpOrd.cl_ord_id);
		map<long, long>::iterator end = sync_reqs.end();
		if (it == end) {
			map<unsigned long, unsigned long>::iterator it_find = sig_ids.find(tmpOrd.signal_id);
			if (it_find != sig_ids .end() && tmpOrd.ord_id > 0) {
				// duplicate a new order from exist order
				my_order ord_to_cancel = this->_report_record_cache[counter];
				ord_to_cancel.signal_id = it_find->second;
				cancel(ord_to_cancel);

				LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
						"cancel_orders_imp:orig_cl_ord_id=" << ord_to_cancel.cl_ord_id
						<< ";cancel req id=" << ord_to_cancel.cl_ord_id);

				long &cancel_cl_ord_id = ord_to_cancel.cl_ord_id;
				sync_reqs[tmpOrd.cl_ord_id] = cancel_cl_ord_id;
				cancel_counter++;
			} else if (0 == tmpOrd.ord_id) {
				LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
						"ord_id is empty,cl_ord_id:" << this->_report_record_cache[counter].cl_ord_id);
			}
		}
	}
	return cancel_counter;
}

template<typename SPIFQuoteT, typename CFQuoteT, typename StockQuoteT,
		typename FullDepthQuoteT, typename QuoteT5>
int
cancel_signal_processor<SPIFQuoteT, CFQuoteT, StockQuoteT, FullDepthQuoteT,QuoteT5>
::cancel_quotes_imp(const long & model_id,map<unsigned long, unsigned long> sig_ids)
 {
	int cancel_counter = 0;
	int total = 0;
	pending_ord_request_dao::query_not_final_quote_order(model_id, this->_quote_record_cache,total);
	for (int counter = 0; counter < total; counter++) {
		QuoteOrder &tmpOrd = this->_quote_record_cache[counter];
		map<long, long>::iterator it = sync_reqs.find(tmpOrd.cl_ord_id);
		map<long, long>::iterator end = sync_reqs.end();
		if (it == end) {
			map<unsigned long, unsigned long>::iterator it_find = sig_ids.find(tmpOrd.signal_id);
			if (it_find != sig_ids.end() && tmpOrd.ord_id > 0) {
				// duplicate a new order from exist order
				QuoteOrder ord_to_cancel = this->_quote_record_cache[counter];
				ord_to_cancel.signal_id = it_find->second;
				cancel_quote(ord_to_cancel);

				long &cancel_cl_ord_id = ord_to_cancel.cl_ord_id;
				sync_reqs[tmpOrd.cl_ord_id] = cancel_cl_ord_id;
				cancel_counter++;
			} else if (0 == tmpOrd.ord_id) {
				LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
						"ord_id is empty,cl_ord_id:" << this->_quote_record_cache[counter].cl_ord_id);
			}
		}
	}
	return cancel_counter;
}
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::sync_cancel_order_core(const long &model_id,map<unsigned long,unsigned long> sig_ids,int &sig_cnt)
{
	rpt_ready = true;

	if(0 != sig_cnt){
		wait_for_ord_id(model_id,sig_ids);
		wait_for_quote_id(model_id,sig_ids);

		bool timeout = true;
		while(timeout){
			int cancel_total = 0;
			rpt_ready = true;
			timeout = false;
			sync_reqs.clear();

			sync_req_rep_lock.lock();
			rpt_ready = false;
			cancel_total = cancel_total + cancel_orders_imp(model_id, sig_ids);
			cancel_total = cancel_total + cancel_quotes_imp(model_id, sig_ids);
			sync_req_rep_lock.unlock();

			if(cancel_total > 0){
				// 同步等待撤单结果
				unique_lock<mutex> lock(mut_for_sync_cancel);
				while(!rpt_ready){
					// 将来要改成支持超时
					if (cond_for_sync_cancel.wait_for(lock, std::chrono::milliseconds(model_ptr->setting.cancel_timeout)) == cv_status::timeout){
						timeout = true;
						LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"Cancellation is time out,model_id:" << model_id);
						break;
					}
				}
			}	// end if(cancel_total > 0)
		}	// end while(timeout)

		sync_reqs.clear();
		rpt_ready = true;

//		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"sync_cancel_order_core end:"
//				<< rpt_ready);
	} // end if(0!==sig_cnt){
}





