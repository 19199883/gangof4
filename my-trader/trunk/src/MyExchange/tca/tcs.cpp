#include <boost/foreach.hpp>
#include <boost/array.hpp>
#include <functional>
#include <sstream>
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include "tcs.h"
#include "pending_ord_request_dao.h"
#include "tbb/include/tbb/spin_mutex.h"
#include <condition_variable> // std::condition_variable
#include <chrono>
#include <ctime>
#include <ratio>
#include <ctime>
#include <functional>   // std::bind
#include <thread>
#include "../my_exchange.h"
#include <vector>
#include "signal_entity.h"
#include "unistd.h"
#include "my_trade_tunnel_api.h"
#include "perfctx.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace trading_channel_agent;
using namespace std::chrono;

tcs::tcs(tcs_setting setting)
:channel(NULL),status_(0)
{
	stopped = true;

	ord_response_cache = vector<T_OrderRespond>(400);
	ord_response_cache.reserve(400);
	ord_response_pos_cache = vector<T_PositionData>(400);
	ord_response_pos_cache.reserve(400);
	ord_response_counter = 0;
	ord_res_ready = false;

	cancel_response_cache = vector<T_CancelRespond>(400);
	cancel_response_cache.reserve(400);
	cancel_response_counter = 0;
	cancel_response_ready = false;

	ord_rtn_cache = vector<T_OrderReturn>(400);
	ord_rtn_cache.reserve(400);
	ord_rtn_pos_cache = vector<T_PositionData>(400);
	ord_rtn_pos_cache.reserve(400);
	ord_rtn_counter = 0;
	ord_rtn_ready = false;

	trde_rtn_cache = vector<T_TradeReturn>(400);
	trde_rtn_cache.reserve(400);
	trade_rtn_pos_cache = vector<T_PositionData>(400);
	trade_rtn_pos_cache.reserve(400);
	trde_rtn_counter = 0;
	trde_rtn_ready = false;

	req_quote_response_cache = vector<T_RspOfReqForQuote>(400);
	req_quote_response_cache.reserve(400);
	req_quote_counter = 0;
	req_quote_ready = false;

	quote_response_cache = vector<T_InsertQuoteRespond>(400);
	quote_response_cache.reserve(400);
	quote_response_pos_cache = vector<T_PositionData>(400);
	quote_response_pos_cache.reserve(400);
	quote_response_counter = 0;
	quote_response_ready = false;

	quote_rtn_cache = vector<T_QuoteReturn>(400);
	quote_rtn_cache.reserve(400);
	quote_rtn_pos_cache = vector<T_PositionData>(400);
	quote_rtn_pos_cache.reserve(400);
	quote_rtn_counter = 0;
	quote_rtn_ready = false;

	quote_trade_rtn_cache = vector<T_QuoteTrade>(400);
	quote_trade_rtn_cache.reserve(400);
	quote_trade_rtn_pos_cache = vector<T_PositionData>(400);
	quote_trade_rtn_pos_cache.reserve(400);
	quote_trade_rtn_counter = 0;
	quote_trade_rtn_ready = false;

	cancel_quote_response_cache = vector<T_CancelQuoteRespond>(400);
	cancel_quote_response_cache.reserve(400);
	cancel_quote_response_counter = 0;
	cancel_quote_response_ready = false;

	to_dispatch_ord_cache = vector<my_order>(1000);
	to_dispatch_ord_cache.reserve(1000);
	to_dispatch_ord_counter = 0;

	ori_rpt_sort_table = vector<Original_rpt_type_options>(3000);
	ori_rpt_sort_table.reserve(3000);
	ori_rpt_sort_table_counter = 0;

	this->setting = setting;
}

tcs::~tcs(void)
{
	this->finalize();
}

void tcs::initialize(void)
{
	stopped = false;
	// 启动回报接收线程
	process_report_thread = thread(bind(&tcs::process_reports, this));
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"tcs process_report_thread,model id:" << this->setting.name<< "; thread id:" << process_report_thread.get_id());

	process_report_thread.detach();

	try {
		channel = MyExchangeFactory::create(this->setting.so_file,this->setting.my_xchg_cfg_);

	} catch (exception &ex) {
		cout<<"some exception happened:" << ex.what()<<endl;
	}

	std::function<void (const T_OrderRespond *, const T_PositionData *)> ord_resf =
			std::bind(&tcs::rev_ord_response, this, std::placeholders::_1,std::placeholders::_2);
	channel->SetCallbackHandler(ord_resf);
	std::function<void (const T_CancelRespond *)> cancel_resf =
			std::bind(&tcs::rev_cancel_ord_response, this, std::placeholders::_1);
	channel->SetCallbackHandler(cancel_resf);
	std::function<void (const T_OrderReturn *, const T_PositionData *)> ord_rtnf =
			std::bind(&tcs::rev_ord_return, this, std::placeholders::_1,std::placeholders::_2);
	channel->SetCallbackHandler(ord_rtnf);
	std::function<void (const T_TradeReturn *, const T_PositionData *)> trad_rtnf =
			std::bind(&tcs::rev_trade_return, this, std::placeholders::_1,std::placeholders::_2);
	channel->SetCallbackHandler(trad_rtnf);

	std::function<void (const T_OrderDetailReturn *)> queyr_ords_f =
			std::bind(&tcs::query_orders_result_handler, this, std::placeholders::_1);
	channel->SetCallbackHandler(queyr_ords_f);
	std::function<void (const T_TradeDetailReturn *)> query_trans_f =
			std::bind(&tcs::query_transactions_result_handler, this, std::placeholders::_1);
	channel->SetCallbackHandler(query_trans_f);
	std::function<void (const T_PositionReturn *)> query_pos_f =
			std::bind(&tcs::query_position_result_handler, this, std::placeholders::_1);
	channel->SetCallbackHandler(query_pos_f);

	std::function<void (const T_ContractInfoReturn *)> query_contract_f =
			std::bind(&tcs::query_ContractInfo_result_handler, this, std::placeholders::_1);
	channel->SetCallbackHandler(query_contract_f);

	// market making, request quote
	std::function<void (const T_RspOfReqForQuote *)> req_quote_f =
			std::bind(&tcs::rev_rep_of_req_quote, this, std::placeholders::_1);
	channel->SetCallbackHandler(req_quote_f);

	// market making, quote notification
	std::function<void (const T_RtnForQuote *)> quote_notify_f =
			std::bind(&tcs::rev_rtn_for_quote, this, std::placeholders::_1);
	channel->SetCallbackHandler(quote_notify_f);

	// market making quote,
	std::function<void (const T_InsertQuoteRespond *, const T_PositionData *)> rep_quote_f =
			std::bind(&tcs::rev_rep_of_quote, this, std::placeholders::_1,std::placeholders::_2);
	channel->SetCallbackHandler(rep_quote_f);
	std::function<void (const T_QuoteReturn *, const T_PositionData *)> rtn_of_quote_f =
			std::bind(&tcs::rev_rtn_of_quote, this, std::placeholders::_1,std::placeholders::_2);
	channel->SetCallbackHandler(rtn_of_quote_f);
	std::function<void (const T_QuoteTrade *, const T_PositionData *)> trade_rtn_of_quote_f =
			std::bind(&tcs::rev_trade_rtn_of_quote, this, std::placeholders::_1,std::placeholders::_2);
	channel->SetCallbackHandler(trade_rtn_of_quote_f);
	std::function<void (const T_CancelQuoteRespond *)> rep_of_cancel_quote_f =
			std::bind(&tcs::rev_rep_of_cancel_quote, this, std::placeholders::_1);
	channel->SetCallbackHandler(rep_of_cancel_quote_f);

	// cancel all uncompleted orders
	this->cancel_orders();
}

void tcs::cancel_orders()
{
	// currently, this feature was moved into channel module.
}

void tcs::finalize(void)
{	
	stopped = true;
	if (this->channel !=0)
	{
		MyExchangeFactory::destroy(this->setting.so_file,this->channel);
		this->channel = 0;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"deleted channel successfully.");
}

void tcs::rev_ord_response(const T_OrderRespond *ord_res, const T_PositionData *pos)
{
	{	// start lock scope
		//speculative_spin_mutex::scoped_lock lock(ori_rpt_ready_mtx);
		 lock_guard<mutex> lock(ori_rpt_ready_mtx);

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			"rev_ord_response,"
			<<  ord_res->entrust_no << ","
			<<  ord_res->entrust_status << ","
			<<  ord_res->serial_no << ",");

		this->ord_response_cache[ord_response_counter] = *ord_res;
		this->ord_response_pos_cache[ord_response_counter] = *pos;
		ord_response_counter++;
		ord_res_ready = true;

		ori_rpt_sort_table[ori_rpt_sort_table_counter] = Original_rpt_type_options::ord_response;
		ori_rpt_sort_table_counter++;
	}	// end lock scope
	ori_rpt_ready_cv.notify_all();
}

void tcs::rev_cancel_ord_response(const T_CancelRespond *canel_res)
{
	{	// start lock scope
		//speculative_spin_mutex::scoped_lock lock(ori_rpt_ready_mtx);
		lock_guard<mutex> lock(ori_rpt_ready_mtx);
		
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				"rev_cancel_ord_response,"
				<<  canel_res->entrust_no << ","
				<<  canel_res->entrust_status << ","
				<<  canel_res->serial_no << ",");

		this->cancel_response_cache[cancel_response_counter] = *canel_res;
		cancel_response_counter++;
		cancel_response_ready = true;

		ori_rpt_sort_table[ori_rpt_sort_table_counter] = Original_rpt_type_options::cancel_response;
		ori_rpt_sort_table_counter++;
	}	// end lock scope
	ori_rpt_ready_cv.notify_all();
}

void tcs::rev_ord_return(const T_OrderReturn *ord_rtn, const T_PositionData * pos)
{
	{	// start lock scope
		//speculative_spin_mutex::scoped_lock lock(ori_rpt_ready_mtx);
        lock_guard<mutex> lock(ori_rpt_ready_mtx);
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			"rev_ord_return,"
			<<  ord_rtn->entrust_no << ","
			<<  ord_rtn->entrust_status << ","
			<<  ord_rtn->serial_no << ",");

		this->ord_rtn_cache[ord_rtn_counter] = *ord_rtn;
		this->ord_rtn_pos_cache[ord_rtn_counter] = *pos;
		ord_rtn_counter++;
		ord_rtn_ready = true;

		ori_rpt_sort_table[ori_rpt_sort_table_counter] = Original_rpt_type_options::ord_return;
		ori_rpt_sort_table_counter++;
	}	// end lock scope
	ori_rpt_ready_cv.notify_all();
}

void tcs::rev_trade_return(const T_TradeReturn *trade_rtn, const T_PositionData *pos)
{
	{	// start lock scope
		//speculative_spin_mutex::scoped_lock  lock(ori_rpt_ready_mtx);
        lock_guard<mutex>  lock(ori_rpt_ready_mtx);
		
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
					"trade_rtn,"
					<<  trade_rtn->entrust_no << ","
					<<  trade_rtn->serial_no << ",");

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
					"long_avg_price:" << pos->long_avg_price
					<< " long_position:" << pos->long_position
					<< " short_avg_price:" << pos->short_avg_price
					<< " short_position:" << pos->short_position
			);

		this->trde_rtn_cache[trde_rtn_counter] = *trade_rtn;
		this->trade_rtn_pos_cache[trde_rtn_counter] = *pos;
		trde_rtn_counter++;
		trde_rtn_ready = true;

		ori_rpt_sort_table[ori_rpt_sort_table_counter] = Original_rpt_type_options::trade_return;
		ori_rpt_sort_table_counter++;
	}	// end lock scope
    ori_rpt_ready_cv.notify_all();
}

void tcs::rev_rep_of_quote(const T_InsertQuoteRespond *quote_rep, const T_PositionData *pos)
{
	// market making, quote
	{	// start lock scope
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				"rev_rep_of_quote,"
				<<  "entrust_no:" << quote_rep->entrust_no << ","
				<<  "entrust_status:" << quote_rep->entrust_status << ","
				<<  "serial_no:" << quote_rep->serial_no << ",");

		//speculative_spin_mutex::scoped_lock lock(ori_rpt_ready_mtx);
        lock_guard<mutex> lock(ori_rpt_ready_mtx);
		this->quote_response_cache[quote_response_counter] = *quote_rep;
		this->quote_response_pos_cache[quote_response_counter] = *pos;
		quote_response_counter++;
		quote_response_ready = true;

		ori_rpt_sort_table[ori_rpt_sort_table_counter] = Original_rpt_type_options::quote_reponse;
		ori_rpt_sort_table_counter++;
	}	// end lock scope
    ori_rpt_ready_cv.notify_all();
}

void tcs::rev_rtn_of_quote(const T_QuoteReturn *rtn, const T_PositionData *pos)
{
	// market making, quote
	{	// enter lock scope
	    //speculative_spin_mutex::scoped_lock lock(ori_rpt_ready_mtx);
        lock_guard<mutex> lock(ori_rpt_ready_mtx);
		
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			"rev_rtn_of_quote,"
			<<  "direction:" << rtn->direction << ","
			<<  "entrust_no:" << rtn->entrust_no << ","
			<<  "entrust_status:" << rtn->entrust_status << ","
			<<  "limit_price:" << rtn->limit_price << ","
			<<  "serial_no:" << rtn->serial_no << ","
			<<  "stock_code:" << rtn->stock_code << ","
			<<  "volume:" << rtn->volume << ","
			<<  "volume_remain:" << rtn->volume_remain << ",");

		this->quote_rtn_cache[quote_rtn_counter] = *rtn;
		this->quote_rtn_pos_cache[quote_rtn_counter] = *pos;
		quote_rtn_counter++;
		quote_rtn_ready = true;

		ori_rpt_sort_table[ori_rpt_sort_table_counter] = Original_rpt_type_options::quote_return;
		ori_rpt_sort_table_counter++;
	}	// exit lock scope
	ori_rpt_ready_cv.notify_all();
}

void tcs::rev_trade_rtn_of_quote(const T_QuoteTrade *rtn, const T_PositionData *pos)
{
	{	// enter lock scope
		//speculative_spin_mutex::scoped_lock lock(ori_rpt_ready_mtx);
        lock_guard<mutex> lock(ori_rpt_ready_mtx);
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
					"rev_trade_rtn_of_quote,"
					<<  "business_no:" << rtn->business_no << ","
					<<  "business_price:" << rtn->business_price << ","
					<<  "business_volume:" << rtn->business_volume << ","
					<<  "direction:" << rtn->direction << ","
					<<  "entrust_no:" << rtn->entrust_no << ","
					<<  "serial_no:" << rtn->serial_no << ","
					<<  "stock_code:" << rtn->stock_code << ","
					);

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
					"long_avg_price:" << pos->long_avg_price
					<< " long_position:" << pos->long_position
					<< " short_avg_price:" << pos->short_avg_price
					<< " short_position:" << pos->short_position
			);

		this->quote_trade_rtn_cache[quote_trade_rtn_counter] = *rtn;
		this->quote_trade_rtn_pos_cache[quote_trade_rtn_counter] = *pos;
		quote_trade_rtn_counter++;
		quote_trade_rtn_ready = true;

		ori_rpt_sort_table[ori_rpt_sort_table_counter] = Original_rpt_type_options::quote_trade_return;
		ori_rpt_sort_table_counter++;
	}	// exit lock scope
    ori_rpt_ready_cv.notify_all();
}

void tcs::rev_rep_of_cancel_quote(const T_CancelQuoteRespond *res)
{
	{	// enter lock scope
	    //speculative_spin_mutex::scoped_lock  lock(ori_rpt_ready_mtx);
        lock_guard<mutex>  lock(ori_rpt_ready_mtx);
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
					"rev_rep_of_cancel_quote,"
					<<  "entrust_no:" << res->entrust_no << ","
					<<  "entrust_status:" << res->entrust_status << ","
					<<  "serial_no:" << res->serial_no << ",");

		this->cancel_quote_response_cache[cancel_quote_response_counter] = *res;
		cancel_quote_response_counter++;
		cancel_quote_response_ready = true;

		ori_rpt_sort_table[ori_rpt_sort_table_counter] = Original_rpt_type_options::cancel_quote_response;
		ori_rpt_sort_table_counter++;
	}	// exit lock scope
	ori_rpt_ready_cv.notify_all();
}

void tcs::process_reports(void)
{
	while (!stopped){
        
		{ 
            unique_lock<mutex> lock(ori_rpt_ready_mtx);
    		while(!ord_res_ready &&
    				!ord_rtn_ready &&
    				!trde_rtn_ready &&
    				!cancel_response_ready &&
    				!req_quote_ready &&
    				!quote_response_ready &&
    				!quote_rtn_ready &&
    				!cancel_quote_response_ready &&
    				!quote_trade_rtn_ready ) {
                ori_rpt_ready_cv.wait(lock);
    		}            
            
			int cur_ord_res = 0;
			int cur_cancel_res = 0;
			int cur_ord_rtn = 0;
			int cur_trade_rtn = 0;
			int cur_rep_of_req_quote = 0;
			int cur_rtn_of_quote = 0;
			int cur_quote_response = 0;
			int cur_trade_rtn_of_quote = 0;
			int cur_cancel_quote_res = 0;
			for (int cur=0; cur<ori_rpt_sort_table_counter; cur++)
			{
				Original_rpt_type_options &ori_rpt_type = ori_rpt_sort_table[cur];
				if (ori_rpt_type == Original_rpt_type_options::ord_response){
					process_ord_response(cur_ord_res);
					cur_ord_res++;
				}
				if (ori_rpt_type == Original_rpt_type_options::cancel_response){
					process_cancel_response(cur_cancel_res);
					cur_cancel_res++;
				}
				if (ori_rpt_type == Original_rpt_type_options::ord_return){
					process_ord_rtn(cur_ord_rtn);
					cur_ord_rtn++;
				}
				if (ori_rpt_type == Original_rpt_type_options::trade_return){
					process_trade_rtn(cur_trade_rtn);
					cur_trade_rtn++;
				}
				if (ori_rpt_type == Original_rpt_type_options::rep_of_req_quote){
					this->process_rep_of_req_quote(cur_rep_of_req_quote);
					cur_rep_of_req_quote++;
				}
				if (ori_rpt_type == Original_rpt_type_options::quote_reponse){
					this->process_quote_response(cur_quote_response);
					cur_quote_response++;
				}
				if (ori_rpt_type == Original_rpt_type_options::quote_return){
					this->process_quote_return(cur_rtn_of_quote);
					cur_rtn_of_quote++;
				}
				if (ori_rpt_type == Original_rpt_type_options::quote_trade_return){
					this->process_quote_trade_return(cur_trade_rtn_of_quote);
					cur_trade_rtn_of_quote++;
				}
				if (ori_rpt_type == Original_rpt_type_options::cancel_quote_response){
					this->process_cancel_quote_response(cur_cancel_quote_res);
					cur_cancel_quote_res++;
				}
			}
			ord_res_ready = false;
			ord_response_counter = 0;
			cur_ord_res = 0;

			cur_cancel_res = 0;
			cancel_response_ready = false;
			cancel_response_counter = 0;

			ord_rtn_ready = false;
			ord_rtn_counter = 0;
			cur_ord_rtn = 0;

			trde_rtn_ready = false;
			trde_rtn_counter = 0;
			cur_trade_rtn = 0;

			req_quote_ready = false;
			req_quote_counter = 0;
			cur_rep_of_req_quote = 0;

			quote_response_counter = 0;
			quote_response_ready = false;
			cur_quote_response = 0;

			quote_rtn_counter = 0;
			quote_rtn_ready = false;
			cur_rtn_of_quote = 0;

			quote_trade_rtn_counter = 0;
			quote_trade_rtn_ready = false;
			cur_trade_rtn_of_quote = 0;

			cancel_quote_response_ready = false;
			cancel_quote_response_counter = 0;
			cur_cancel_quote_res = 0;

			ori_rpt_sort_table_counter = 0;
		} // end lock scope

		{	// enter lock scope
			unique_lock<mutex> lock(rpt_notify_ready_mtx);
			for (int cur=0; cur<to_dispatch_ord_counter; cur++)
			{
				my_order &ord = to_dispatch_ord_cache[cur];
				this->trace("process_reports",ord);

				ReportNotifyTableKeyT key = ReportNotifyTableKeyT(ord.model_id,"cancel_signal_processor");
				report_notify_state[key] = true;

				ReportNotifyTableKeyT st_unit_key = ReportNotifyTableKeyT(ord.model_id,"strategy_unit");
				report_notify_state[st_unit_key] = true;

				pending_ord_request_dao::cache(ord);
			}
			to_dispatch_ord_counter = 0;
		}	// exit lock scope
		rpt_notify_ready_cv.notify_all();
	}	// while (!stopped)
}

void tcs::process_ord_response(const int &cur)
{
	bool is_final = false;
	my_order &ord = to_dispatch_ord_cache[to_dispatch_ord_counter];
	T_OrderRespond &ord_response = ord_response_cache[cur];

	pending_ord_request_dao::query_request(ord_response.serial_no,ord);
	ord.ord_id = ord_response.entrust_no;
	ord.state = convert_from(ord_response.entrust_status);
	ord.error_no = static_cast<channel_errors>(ord_response.error_no);
	// 如果委托到了最终状态，则在pending表将其标记为完成
	if (ord.state == state_options::cancelled || ord.state == state_options::rejected)
	{
		is_final = true;
	}
	else
	{
		is_final = false;
	}
	ord.is_final = is_final;
	ord.position_ = ord_response_pos_cache[cur];
	pending_ord_request_dao::update_ord(ord.cl_ord_id, ord.ord_id,ord.state,is_final,ord.error_no);

	trace("process_ord_response",ord);
	to_dispatch_ord_counter++;
}

void tcs::process_cancel_response(const int &cur)
{
	bool is_final = false;
	my_order &ord = to_dispatch_ord_cache[to_dispatch_ord_counter];
	T_CancelRespond &cancel_response = cancel_response_cache[cur];

	pending_ord_request_dao::query_request(cancel_response.serial_no,ord);
	ord.ord_id = cancel_response.entrust_no;
	ord.state = convert_from(cancel_response.entrust_status);
	ord.error_no = static_cast<channel_errors>(cancel_response.error_no);
	// 如果委托到了最终状态，则在pending表将其标记为完成
	if (ord.state == state_options::cancelled || ord.state == state_options::rejected)
	{
		is_final = true;
	}
	else
	{
		is_final = false;
	}
	ord.is_final = is_final;
	pending_ord_request_dao::update_ord(ord.cl_ord_id, ord.ord_id,ord.state,is_final,ord.error_no);

	trace("process_cancel_response",ord);
	to_dispatch_ord_counter++;
}

void tcs::process_ord_rtn(const int &cur)
{
	my_order &ord = to_dispatch_ord_cache[to_dispatch_ord_counter];
	T_OrderReturn &ord_rtn = ord_rtn_cache[cur];

	if (pending_ord_request_dao::exist(ord_rtn.serial_no) == false)
	{
		ord.cl_ord_id = ord_rtn.serial_no;
		state_options state = tcs::convert_from(ord_rtn.entrust_status);
		if (state_options::rejected == state)
		{
			ord.is_final = true;
		}
		else
		{
			ord.is_final = false;
		}

		ord.model_id = 0;
		ord.ord_id = ord_rtn.entrust_no;
		ord.ord_type = ord_types::general;
		ord.position_effect = static_cast<position_effect_options>(ord_rtn.open_close);
		ord.price = ord_rtn.limit_price;
		ord.price_type = price_options::limit;
		ord.request_type = request_types::place_order;
		ord.side = static_cast<side_options>(ord_rtn.direction);
		ord.signal_id = 0;
		ord.state = state;
		ord.symbol = ord_rtn.stock_code;
		ord.volume = ord_rtn.volume;

		pending_ord_request_dao::insert_request(ord);
	}
	else
	{
		bool is_final = false;
		pending_ord_request_dao::query_request(ord_rtn.serial_no,ord);
		ord.ord_id = ord_rtn.entrust_no;
		ord.state = convert_from(ord_rtn.entrust_status);
		// 如果委托到了最终状态，则在pending表将其标记为完成
		if (ord.state==state_options::cancelled || ord.state==state_options::rejected)
		{
			is_final = true;
		}
		else
		{
			is_final = false;
		}
		ord.position_ = ord_rtn_pos_cache[cur];
		ord.is_final = is_final;
		pending_ord_request_dao::update_ord(ord.cl_ord_id, ord.ord_id,ord.state,is_final,channel_errors::RESULT_SUCCESS);
	}

	trace("process_ord_rtn",ord);
	to_dispatch_ord_counter++;
}

void tcs::process_trade_rtn(const int &cur)
{
	bool is_final = false;
	T_TradeReturn &trade_rtn = trde_rtn_cache[cur];
	my_order &ord = to_dispatch_ord_cache[to_dispatch_ord_counter];

	pending_ord_request_dao::query_request(trade_rtn.serial_no,ord);
	ord.last_px = trade_rtn.business_price;
	ord.last_qty = trade_rtn.business_volume;
	ord.cum_qty += trade_rtn.business_volume;
	ord.cum_amount += ord.last_px * ord.last_qty;
	if (ord.state==state_options::filled && ord.cum_qty==ord.volume) is_final = true;
	else is_final = false;

	ord.is_final = is_final;
	ord.position_ = trade_rtn_pos_cache[cur];
	pending_ord_request_dao::update_filled_info(ord.cl_ord_id, trade_rtn.business_price,
			trade_rtn.business_volume,is_final);

	trace("process_trade_rtn",ord);
	to_dispatch_ord_counter++;
}

void tcs::trace(const string &invoker,const my_order &ord)
{
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			invoker<<","
			<< "model_id:" <<  ord.model_id << ","
			<< "signal_id:" <<  ord.signal_id << ","
			<< "cl_ord_id:"<<  ord.cl_ord_id << ","
			<< "ord_id:" <<  ord.ord_id << ","
			<< "is_final:" <<  ord.is_final << ","
			<< "state:"<<  ord.state << ","
			<< "volume:"<<  ord.volume << ","
			<< "side:"<<  ord.side << ","
			<< "last_qty:"<<  ord.last_qty << ","
			<< "last_px:"<<  ord.last_px << ","
			<< "cum_qty:"<<  ord.cum_qty << ","
			<<  ord.orig_cl_ord_id << ","
			<< "request_type:"<<  ord.request_type << ",");
}

void tcs::trace(const string &invoker,const QuoteOrder &ord)
{
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			invoker<<","
			<< "model_id:" <<  ord.model_id << ","
			<< "signal_id:" <<  ord.signal_id << ","
			<< "cl_ord_id:"<<  ord.cl_ord_id << ","
			<< "ord_id:" <<  ord.ord_id << ","
			<< "buy_is_final:" <<  ord.buy_is_final << ","
			<< "buy_state:"<<  ord.buy_state << ","
			<< "buy_volume:"<<  ord.buy_volume << ","
			<< "buy_cum_qty:"<<  ord.buy_cum_qty << ","
			<< "buy_cum_amount:"<<  ord.buy_cum_amount << ","
			<< "buy_last_px:"<<  ord.buy_last_px << ","
			<< "buy_last_qty:"<<  ord.buy_last_qty << ","

			<< "sell_is_final:" <<  ord.sell_is_final << ","
			<< "sell_state:"<<  ord.sell_state << ","
			<< "sell_volume:"<<  ord.sell_volume << ","
			<< "sell_cum_qty:"<<  ord.sell_cum_qty << ","
			<< "sell_last_px:"<<  ord.sell_last_px << ","
			<< "sell_last_qty:"<<  ord.sell_last_qty << ","

			<< "request_type:"<<  ord.request_type << ","
			<< "stock_code:"<<  ord.stock_code << ","
			<< "orig_cl_ord_id:" <<  ord.orig_cl_ord_id << ","
			<< "request_type:"<<  ord.request_type << ",");
}

	
void tcs::place_request(const my_order &ord)
{
	trace("place_request",ord);

	if (channel == NULL) {
		return;
	}

	try
	{
		T_PlaceOrder chn_ord;
		chn_ord.direction = ord.side;
		chn_ord.limit_price = ord.price;
		chn_ord.open_close = ord.position_effect;
		chn_ord.order_kind = ord.price_type;
		chn_ord.order_type = ord.ord_type;
		chn_ord.serial_no = ord.cl_ord_id;
		chn_ord.speculator = ord.sah_;
		chn_ord.exchange_type = ord.exchange;


		strcpy(chn_ord.stock_code,ord.symbol.c_str());
		chn_ord.volume = ord.volume;

#ifdef LATENCY_MEASURE
        // latency measure
		int latency = perf_ctx::calcu_latency(ord.model_id,ord.signal_id);
        if(latency > 0){
                LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"place latency:" << latency << " us");
        }
#endif
		this->channel->PlaceOrder(&chn_ord);
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"place_request throw exception," << ex.what());
	}
}

void tcs::req_quote(const my_order &ord)
{
	trace("req_quote",ord);

	if (channel == NULL) {
		return;
	}

	try
	{
		T_ReqForQuote chn_ord;
		chn_ord.serial_no = ord.cl_ord_id;
		strcpy(chn_ord.stock_code,ord.symbol.c_str());

		this->channel->ReqForQuoteInsert(&chn_ord);
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"place_request throw exception," << ex.what());
	}
}

void tcs::quote(const QuoteOrder &ord)
{
	trace("quote",ord);
	if (channel == NULL) return;

	try
	{
		T_InsertQuote chn_ord;
		chn_ord.serial_no = ord.cl_ord_id;
		strcpy(chn_ord.stock_code,ord.stock_code.c_str());
		chn_ord.buy_limit_price = ord.buy_limit_price;
		chn_ord.buy_volume = ord.buy_volume;
		chn_ord.buy_open_close = position_effect_options::open_;
		chn_ord.buy_speculator = ord.buy_speculator;

		chn_ord.sell_limit_price = ord.sell_limit_price;
		chn_ord.sell_volume = ord.sell_volume;
		chn_ord.sell_open_close = position_effect_options::open_;
		chn_ord.sell_speculator = ord.sell_speculator;
		strcpy(chn_ord.for_quote_id,ord.for_quote_id);

		this->channel->ReqQuoteInsert(&chn_ord);
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"place_request throw exception," << ex.what());
	}
}

void tcs::cancel_quote(const QuoteOrder &ord)
{
	trace("cancel_quote",ord);

	if (channel == NULL) {
		return;
	}

	try
	{
		T_CancelQuote chn_ord;
		chn_ord.serial_no = ord.cl_ord_id;
		strcpy(chn_ord.stock_code,ord.stock_code.c_str());
		chn_ord.entrust_no = ord.orig_ord_id;
		chn_ord.org_serial_no = ord.orig_cl_ord_id;

		this->channel->ReqQuoteAction(&chn_ord);
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"place_request throw exception," << ex.what());
	}
}

void tcs::cancel_request(const my_order &ord)
{
	try
	{
		trace("cancel_request",ord);

		T_CancelOrder chn_ord;
 		chn_ord.direction = ord.side;
		chn_ord.limit_price = ord.price;
		chn_ord.open_close = ord.position_effect;
		chn_ord.serial_no = ord.cl_ord_id;
		chn_ord.speculator = '0';
		strcpy(chn_ord.stock_code,ord.symbol.c_str());
		chn_ord.volume = ord.volume;
		chn_ord.org_serial_no = ord.orig_cl_ord_id;
		chn_ord.entrust_no = ord.orig_ord_id;
		chn_ord.exchange_type = ord.exchange;

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				"cancel_request,org_serial_no:" << chn_ord.org_serial_no
				<< "entrust_no:" << chn_ord.entrust_no);
#ifdef LATENCY_MEASURE
        // latency measure
		int latency = perf_ctx::calcu_latency(ord.model_id,ord.signal_id);
        if(latency > 0){
                LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"cancel latency:" << latency << " us");
        }
#endif
		this->channel->CancelOrder(&chn_ord);
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"cancel_request throw exception," << ex.what());
	}
}

state_options tcs::convert_from(char state)
{

		if ((char)state == '0' ||
			(char)state == 'n' ||
			(char)state == 's')
		{
			return state_options::pending_new;
		}
		else if ((char)state == 'a')
		{
			return state_options::new_state;
		}
		else if ((char)state == 'p')
		{
			return state_options::partial_filled;
		}
		else if ((char)state == 'c')
		{
			return state_options::filled;
		}
		else if ((char)state == 'e' || (char)state == 'q')
		{
			return state_options::rejected;
		}
		// 部成部撤状态对应cancelled，但是成交量> 0
		else if ( state == 'b')
		{
			return state_options::cancelled;
		}
		else if ((char)state == 'd' )
		{
			return state_options::cancelled;
		}
		else if ((char)state == 'f')
		{
			return state_options::pending_cancel;
		}
		else
		{
			return state_options::undefined2;
		}
}

bool tcs::is_final(OrderDetail &ord)
{
	if(ord.entrust_status == 'c' 	//全成
		||ord.entrust_status == 'd'	//已撤
		||ord.entrust_status =='e' 	//错误报单
		){
		return false;
	}else{
		return true;
	}
}

void tcs::query_orders_result_handler(const T_OrderDetailReturn *result)
{
	try{
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"query_orders_result_handler");
		query_ords_prom_.set_value(*result);
	}catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),ex.what());
	}
}

T_TradeDetailReturn tcs::query_transactions()
{
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"query_transactions");

	 speculative_spin_mutex::scoped_lock lock(sync_query_lock_);

	query_trans_prom_ = std::promise<T_TradeDetailReturn>();

	std::future<T_TradeDetailReturn> future= query_trans_prom_.get_future();
	T_QryTradeDetail criteria;
	this->channel->QueryTradeDetail(&criteria);
	T_TradeDetailReturn trans = future.get();

	return trans;
}

void tcs::query_transactions_result_handler(const T_TradeDetailReturn *trans)
{	try{
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"query_transactions_result_handler");
		query_trans_prom_.set_value(*trans);
	}catch(exception &ex){
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),ex.what());
		}
}

T_PositionReturn tcs::query_position()
{

	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"query_position");

	 speculative_spin_mutex::scoped_lock lock(sync_query_lock_);

	query_pos_prom_ = std::promise<T_PositionReturn>();

	std::future<T_PositionReturn> future= query_pos_prom_.get_future();
	T_QryPosition criteria;

	this->channel->QueryPosition(&criteria);
	T_PositionReturn pos = future.get();

	return pos;
}

void tcs::query_position_result_handler(const T_PositionReturn *pos)
{
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"query_position_result_handler");
	try{
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"query_position_result_handler");
		query_pos_prom_.set_value(*pos);
	}catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),ex.what());
	}
}

T_ContractInfoReturn tcs::query_ContractInfo()
{
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"query_ContractInfo");

	 speculative_spin_mutex::scoped_lock lock(sync_query_lock_);

	query_contracts_prom_ = std::promise<T_ContractInfoReturn>();

	std::future<T_ContractInfoReturn> future= query_contracts_prom_.get_future();
	T_QryContractInfo criteria;

	this->channel->QueryContractInfo(&criteria);
	T_ContractInfoReturn contracts = future.get();

	return contracts;
}

void tcs::query_ContractInfo_result_handler(const T_ContractInfoReturn *contracts)
{
	try{
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"query_ContractInfo_result_handler");
		query_contracts_prom_.set_value(*contracts);
	}catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),ex.what());
	}
}

bool tcs::is_normal()
{
	if(0==this->status_){
		return true;
	}else{
		return false;
	}
}

/*
 * the following methods are for market making
 */
void tcs::rev_rep_of_req_quote(const T_RspOfReqForQuote *rpt)
{
	{
		lock_guard<mutex> lock(ori_rpt_ready_mtx);

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			"rev_rep_of_req_quote,"
			<<  rpt->serial_no << ","
			<<  rpt->error_no);

		this->req_quote_response_cache[req_quote_counter] = *rpt;
		req_quote_counter++;
		req_quote_ready = true;

		ori_rpt_sort_table[ori_rpt_sort_table_counter] = Original_rpt_type_options::rep_of_req_quote;
		ori_rpt_sort_table_counter++;
	}
}

void tcs::process_rep_of_req_quote(const int &cur)
{
	bool is_final = false;
	my_order &ord = to_dispatch_ord_cache[to_dispatch_ord_counter];
	T_RspOfReqForQuote &rep = req_quote_response_cache[cur];

	pending_ord_request_dao::query_request(rep.serial_no,ord);
	//ord.error_no = rep.error_no;
	if(0==ord.error_no){
		ord.state = state_options::filled;
	}else{
		ord.state = state_options::rejected;
	}
	is_final = true;
	ord.is_final = is_final;
	pending_ord_request_dao::update_ord(ord.cl_ord_id, ord.ord_id,ord.state,is_final,ord.error_no);

	trace("process_rep_of_req_quote",ord);
	to_dispatch_ord_counter++;
}

void tcs::rev_rtn_for_quote(const T_RtnForQuote *rpt)
{
	// market making, quote, to here
	{
		 speculative_spin_mutex::scoped_lock  lock(quote_notify_ready_mtx);

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			"rev_rtn_for_quote,"
			<<  "for_quote_id:" << rpt->for_quote_id << ","
			<<  "stock_code:" << rpt->stock_code);

		pending_ord_request_dao::cache(*rpt,quote_notify_state_);
	}
}

void tcs::init(const long &model)
{

//	string straegykey = get_semkey_for_strategyunit(model);
//	v_op_.sem_keys_.push_back(straegykey);
//
//	string cancelprocessorkey = get_semkey_for_cancel_processor(model);
//	v_op_.sem_keys_.push_back(cancelprocessorkey);

}

string get_semkey_for_strategyunit(long model)
{
	string sem_key = to_string(model);
//	sem_key += "strategy_unit_";
//	sem_key += to_string(this->setting.id);
	return sem_key;
}

string get_semkey_for_cancel_processor(long model)
{
	string sem_key = to_string(model);
//	sem_key += "cancel_signal_processor_";
//	sem_key += to_string(this->setting.id);
	return sem_key;
}

void tcs::process_quote_response(const int &cur)
{
	bool is_final = false;
	my_order &ord = to_dispatch_ord_cache[to_dispatch_ord_counter];
	pending_ord_request_dao::reset(ord);
	T_InsertQuoteRespond &rep = quote_response_cache[cur];

	QuoteOrder quote_ord;
	pending_ord_request_dao::query_quote_order(rep.serial_no,quote_ord);
	//ord.side = rep.
	ord.model_id = quote_ord.model_id;
	ord.signal_id = quote_ord.signal_id;
	ord.cl_ord_id = quote_ord.cl_ord_id;
	ord.exchange = quote_ord.exchange;
	ord.symbol = quote_ord.stock_code;
	ord.request_type = request_types::quote;
	ord.ord_id = rep.entrust_no;
	ord.state = convert_from(rep.entrust_status);
	ord.error_no = static_cast<channel_errors>(rep.error_no);
	// 如果委托到了最终状态，则在pending表将其标记为完成
	if (ord.state == state_options::cancelled || ord.state == state_options::rejected)
	{
		is_final = true;
	}
	else
	{
		is_final = false;
	}
	ord.is_final = is_final;
	ord.position_ = quote_response_pos_cache[cur];
	pending_ord_request_dao::update_quote_order(side_options::undefined1,ord.cl_ord_id,
			ord.ord_id,ord.state,is_final,ord.error_no);

	trace("process_quote_response",ord);
	to_dispatch_ord_counter++;
}

void tcs::process_quote_return(const int &cur)
{
	my_order &ord = to_dispatch_ord_cache[to_dispatch_ord_counter];
	T_QuoteReturn &rtn = quote_rtn_cache[cur];
	bool is_final = false;
	QuoteOrder quote_order;
	pending_ord_request_dao::query_quote_order(rtn.serial_no,quote_order);

	pending_ord_request_dao::reset(ord);
	ord.request_type = request_types::place_order;
	ord.model_id = quote_order.model_id;
	ord.signal_id = quote_order.signal_id;
	ord.cl_ord_id = quote_order.cl_ord_id;
	ord.ord_id = rtn.entrust_no;
	ord.symbol = quote_order.stock_code;
	ord.exchange = quote_order.exchange;
	ord.state = convert_from(rtn.entrust_status);
	ord.side = static_cast<side_options>(rtn.direction);
	ord.position_effect = static_cast<position_effect_options>(rtn.open_close);
	ord.sah_ = static_cast<sah_options>(rtn.speculator);
	ord.volume = rtn.volume;
	ord.price = rtn.limit_price;

	// 如果委托到了最终状态，则在pending表将其标记为完成
	if (ord.state==state_options::cancelled || ord.state==state_options::rejected){
		is_final = true;
	}
	else{ is_final = false; }
	ord.position_ = quote_rtn_pos_cache[cur];
	ord.is_final = is_final;
	pending_ord_request_dao::update_quote_order(ord.side,ord.cl_ord_id, ord.ord_id,ord.state,is_final,channel_errors::RESULT_SUCCESS);

	trace("process_quote_rtn",ord);
	to_dispatch_ord_counter++;
}

void tcs::process_quote_trade_return(const int &cur)
{
	bool is_final = false;
	T_QuoteTrade &trade = quote_trade_rtn_cache[cur];
	my_order &ord = to_dispatch_ord_cache[to_dispatch_ord_counter];
	pending_ord_request_dao::reset(ord);

	QuoteOrder quote_order;
	pending_ord_request_dao::query_quote_order(trade.serial_no,quote_order);
	ord.request_type = request_types::place_order;
	ord.cl_ord_id = quote_order.cl_ord_id;
	ord.model_id = quote_order.model_id;
	ord.signal_id = quote_order.signal_id;
	ord.ord_id = trade.entrust_no;
	ord.symbol = quote_order.stock_code;
	ord.side = static_cast<side_options>(trade.direction);
	ord.last_px = trade.business_price;
	ord.last_qty = trade.business_volume;

	if(ord.side==side_options::buy){
		ord.volume = quote_order.buy_volume;
		ord.state = quote_order.buy_state;
		ord.cum_qty = quote_order.buy_cum_qty;
		ord.cum_amount = quote_order.buy_cum_amount;
	}else if(ord.side==side_options::sell){
		ord.volume = quote_order.sell_volume;
		ord.state = quote_order.sell_state;
		ord.cum_qty = quote_order.sell_cum_qty;
		ord.cum_amount = quote_order.sell_cum_amount;
	}
	ord.cum_qty += trade.business_volume;
	ord.cum_amount += ord.last_px * ord.last_qty;

	if (ord.state==state_options::filled && ord.cum_qty==ord.volume){
		is_final = true;
	}
	else{
		is_final = false;
	}

	ord.is_final = is_final;
	ord.position_ = quote_trade_rtn_pos_cache[cur];
	pending_ord_request_dao::update_quote_fill(ord.side,ord.cl_ord_id, trade.business_price, trade.business_volume,is_final);

	trace("process_quote_trade_return",ord);
	to_dispatch_ord_counter++;
}

void tcs::process_cancel_quote_response(const int &cur)
{
	bool is_final = false;
	my_order &ord = to_dispatch_ord_cache[to_dispatch_ord_counter];
	pending_ord_request_dao::reset(ord);
	T_CancelQuoteRespond &cancel_response = cancel_quote_response_cache[cur];

	QuoteOrder quote_ord;
	pending_ord_request_dao::query_quote_order(cancel_response.serial_no,quote_ord);
	ord.request_type = quote_ord.request_type;
	ord.cl_ord_id = quote_ord.cl_ord_id;
	ord.model_id = quote_ord.model_id;
	ord.signal_id = quote_ord.signal_id;
	ord.ord_id = cancel_response.entrust_no;
	ord.state = convert_from(cancel_response.entrust_status);
	ord.error_no = static_cast<channel_errors>(cancel_response.error_no);
	// 如果委托到了最终状态，则在pending表将其标记为完成
	if (ord.state == state_options::cancelled || ord.state == state_options::rejected)
	{
		is_final = true;
	}
	else
	{
		is_final = false;
	}
	ord.is_final = is_final;
	pending_ord_request_dao::update_quote_order(side_options::undefined1,ord.cl_ord_id, ord.ord_id,ord.state,is_final,ord.error_no);

	trace("process_cancel_quote_response",ord);
	to_dispatch_ord_counter++;
}
