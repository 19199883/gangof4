#include <sstream>
#include <chrono>
#include <log4cxx/logger.h>
#include "pending_ord_request_dao.h"
#include <boost/foreach.hpp>
#include <string>
#include "exchange_names.h"

using namespace trading_channel_agent;

set<long> pending_ord_request_dao::cl_ord_id_cache = set<long>();
speculative_spin_mutex pending_ord_request_dao::lock_rpt_notify_cache;
ReportNotifyTableT pending_ord_request_dao::rpt_notify_cache;
speculative_spin_mutex pending_ord_request_dao::lock_local_request_cache;
map<ReportNotifyTableKeyT,int> pending_ord_request_dao::cache_record_count;

OrderDatabaseT pending_ord_request_dao::OrderDatabase(ORDER_DATABASE_MAX_SIZE);
OrderDatabaseClOrdIdIndexTableT pending_ord_request_dao::OrderDatabaseClOrdIdIndexTable;
OrderDatabaseModelIndexTableT pending_ord_request_dao::OrderDatabaseModelIndexTable;
OrderDatabaseSymbolTableT pending_ord_request_dao::OrderDatabaseSymbolTable;
speculative_spin_mutex pending_ord_request_dao::mtx_for_OrderDatabase;
size_t pending_ord_request_dao::order_database_index = 0;

/*
 * market making
 */
speculative_spin_mutex pending_ord_request_dao::lock_quote_notify_cache;
map<long,vector<T_RtnForQuote> > pending_ord_request_dao::quote_notify_cache;
map<long,int> pending_ord_request_dao::cache_quote_notify_count;
// TODO: wangying 2017-2-17
QuoteOrderDatabaseT pending_ord_request_dao::QuoteOrderDatabase(20);
QuoteOrderDatabaseClOrdIdIndexTableT pending_ord_request_dao::QuoteOrderDatabaseClOrdIdIndexTable;
QuoteOrderDatabaseModelIndexTableT pending_ord_request_dao::QuoteOrderDatabaseModelIndexTable;
QuoteOrderDatabaseSymbolTableT pending_ord_request_dao::QuoteOrderDatabaseSymbolTable;
speculative_spin_mutex pending_ord_request_dao::mtx_for_QuoteOrderDatabase;
size_t pending_ord_request_dao::quote_order_database_index = 0;
set<long> pending_ord_request_dao::cl_quote_ord_id_cache = set<long>();


#define PRINT_OUT_OF_RANGE_WARN(index, size) do{             \
        if (index >= size){                                  \
            LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),  \
			   #index " out of range, exit. size:"           \
			   << size << " index:" << index);               \
            sleep(1);                                        \
            exit(0);                                         \
        }                                                    \
    }while(0)	

void pending_ord_request_dao::init(const ReportNotifyTableKeyT &key){
	rpt_notify_cache[key] = vector<my_order>(5000);
	rpt_notify_cache.find(key)->second.reserve(5000);
	cache_record_count[key] = 0;

	quote_notify_cache[key.first] = vector<T_RtnForQuote>(5000);
	quote_notify_cache.find(key.first)->second.reserve(5000);
	cache_quote_notify_count[key.first] = 0;
}

void pending_ord_request_dao::init(const long& model_id,set<string> &symbols){
	OrderDatabase.reserve(ORDER_DATABASE_MAX_SIZE);
	OrderDatabaseModelIndexTable[model_id] = unordered_set<size_t>();

	// market making. quote
	QuoteOrderDatabase.reserve(ORDER_DATABASE_MAX_SIZE);
	QuoteOrderDatabaseModelIndexTable[model_id] = unordered_set<size_t>();

	set<string>::iterator it = symbols.begin();
	set<string>::iterator end = symbols.end();
	for(; it!=end; ++it){
		OrderDatabaseSymbolTable[*it] = unordered_set<size_t>();
		// market making, quote
		QuoteOrderDatabaseSymbolTable[*it] = unordered_set<size_t>();
	}
}

void pending_ord_request_dao::clear_cache(const ReportNotifyTableKeyT &key){
	speculative_spin_mutex::scoped_lock lock(lock_rpt_notify_cache);

	map<ReportNotifyTableKeyT,int>::iterator it = cache_record_count.find(key);
	if (it != cache_record_count.end()){
		it->second = 0;
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
					"clear_cache:" << key.second);
	}
	else{
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
			"clear_cache, error occurred :can not find " << key.first);
	}
}

void pending_ord_request_dao::cache(const my_order &ord){
	 speculative_spin_mutex::scoped_lock lock(lock_rpt_notify_cache);

	if(0==ord.model_id){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				"tcs,model id:" << ord.model_id);
		return;
	}

	ReportNotifyTableKeyT csp_cache_key = ReportNotifyTableKeyT(ord.model_id,"cancel_signal_processor");
	int cur_count = cache_record_count[csp_cache_key];
	rpt_notify_cache.find(csp_cache_key)->second[cur_count] = ord;
	cache_record_count[csp_cache_key] = cur_count + 1;

	ReportNotifyTableKeyT st_unit_cache_key = ReportNotifyTableKeyT(ord.model_id,"strategy_unit");
	int st_unit_count = cache_record_count[st_unit_cache_key];
	rpt_notify_cache.find(st_unit_cache_key)->second[st_unit_count] = ord;
	cache_record_count[st_unit_cache_key] = st_unit_count + 1;

}

ReportNotifyRecordT&
pending_ord_request_dao::read_report_notify(const ReportNotifyTableKeyT &key,int &count,ReportNotifyRecordT& result)
{
	 speculative_spin_mutex::scoped_lock lock(lock_rpt_notify_cache);

	ReportNotifyRecordT &records = rpt_notify_cache.find(key)->second;
	records.swap(result);
	count = cache_record_count[key];

	cache_record_count[key] = 0;

	return records;
}

void
pending_ord_request_dao::
update_ord(const long &cl_ord_id,
		const long &ord_id,const state_options &status,const bool &is_final,
		const channel_errors &err){
	 speculative_spin_mutex::scoped_lock lock(mtx_for_OrderDatabase);

	try{
		size_t &pos = OrderDatabaseClOrdIdIndexTable.at(cl_ord_id);
		my_order &ord = OrderDatabase[pos];
		ord.ord_id = ord_id;
		ord.state = status;
		ord.is_final = is_final;
		ord.error_no = err;

		// remove final orders from index table
		if (true == ord.is_final){
			{
				 speculative_spin_mutex::scoped_lock lock(lock_local_request_cache);
				cl_ord_id_cache.erase(cl_ord_id);
			}

			unordered_set<size_t> & OrderDatabaseModelIndex = OrderDatabaseModelIndexTable[ord.model_id];
			OrderDatabaseModelIndex.erase(pos);
			OrderDatabaseSymbolTable[ord.symbol].erase(pos);
			OrderDatabaseClOrdIdIndexTable.erase(ord.cl_ord_id);
		}
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				"update_ord, error occurred :" << cl_ord_id);
	}
}

void pending_ord_request_dao::insert_request(const my_order &ord){
	// represent this order was placed by this my_exchange
	if (ord.model_id > 0){
		{
			 speculative_spin_mutex::scoped_lock lock(lock_local_request_cache);
			cl_ord_id_cache.insert(ord.cl_ord_id);
		}
	}

	 speculative_spin_mutex::scoped_lock lock(mtx_for_OrderDatabase);
    PRINT_OUT_OF_RANGE_WARN(order_database_index,ORDER_DATABASE_MAX_SIZE);
	OrderDatabase[order_database_index] = ord;
	OrderDatabaseClOrdIdIndexTable[ord.cl_ord_id] = order_database_index;
	OrderDatabaseModelIndexTable[ord.model_id].insert(order_database_index);
	OrderDatabaseSymbolTable[ord.symbol].insert(order_database_index);
	OrderDatabase[order_database_index].is_final = false;
	OrderDatabase[order_database_index].cum_qty = 0;
	if (ord.request_type == request_types::cancel_order){
		OrderDatabase[order_database_index].state = state_options::pending_cancel;
	}
	else if (ord.request_type == request_types::place_order){
		OrderDatabase[order_database_index].state = state_options::pending_new;
		OrderDatabase[order_database_index].orig_cl_ord_id = 0;
		OrderDatabase[order_database_index].orig_ord_id = 0;
	}

	order_database_index++;
}

bool pending_ord_request_dao::exist(const long &cl_ord_id){
	bool exist = false;

	 speculative_spin_mutex::scoped_lock lock(lock_local_request_cache);
	if (cl_ord_id_cache.find(cl_ord_id) != cl_ord_id_cache.end()) exist = true;
	else exist = false;

	return exist;
}

void
pending_ord_request_dao::
query_not_final_ord_requests(const long &model_id,ReportNotifyRecordT &ords, int &total){
	 speculative_spin_mutex::scoped_lock lock(mtx_for_OrderDatabase);

	total = 0;
	try{
		unordered_set<size_t> &indexes = OrderDatabaseModelIndexTable.at(model_id);
		unordered_set<size_t>::iterator it = indexes.begin();
		unordered_set<size_t>::iterator end = indexes.end();
		for(; it!=end; ++it){
			reset(ords[total]);
			my_order &ord = OrderDatabase[*it];
			if(request_types::place_order == ord.request_type){
				ords[total] = ord;
				total++;
			}
		}
	}
	catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
							"query_not_final_ord_requests, error occurred :" << model_id);
	}
}

void
pending_ord_request_dao::
query_not_final_ord_requests_by_symbol(const string &symbol,ReportNotifyRecordT &ords, int &count){
	 speculative_spin_mutex::scoped_lock lock(mtx_for_OrderDatabase);

	count = 0;
	try{
		OrderDatabaseSymbolTableT::iterator it = OrderDatabaseSymbolTable.find(symbol);
		if(it!=OrderDatabaseSymbolTable.end()){
			unordered_set<size_t> &indexes = OrderDatabaseSymbolTable.at(symbol);
			unordered_set<size_t>::iterator it = indexes.begin();
			unordered_set<size_t>::iterator end = indexes.end();
			for(; it!=end; ++it){
				reset(ords[count]);
				my_order &ord = OrderDatabase[*it];
				if(request_types::place_order == ord.request_type){
					ords[count] = ord;
					count++;
				}
			}
		}else{
			count = 0;
		}

	}
	catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
						"query_not_final_ord_requests_by_symbol, error occurred :" << symbol);
	}
}

void pending_ord_request_dao::reset(my_order &ord){
	ord.cl_ord_id = 0;
	ord.ord_id = 0;
	ord.model_id = 0;
	ord.signal_id = 0;
	ord.is_final = false;
	ord.symbol = "";
	ord.price = 0.0;
	ord.volume = 0;
	ord.side = side_options::undefined1;
	ord.position_effect = position_effect_options::undefined;
	ord.ord_type = ord_types::general;
	ord.last_px = 0.0;
	ord.last_qty = 0;
	ord.error_no = channel_errors::RESULT_SUCCESS;
	ord.price_type = price_options::limit;
	ord.last_px = 0;
	ord.last_qty = 0;
	ord.state = state_options::undefined2;
	ord.orig_cl_ord_id = 0;
	ord.orig_ord_id = 0;

	ord.cum_qty = 0;
}

void
pending_ord_request_dao::
query_request(const long& cl_ord_id, my_order &ord){
	 speculative_spin_mutex::scoped_lock lock(mtx_for_OrderDatabase);

	reset(ord);

	OrderDatabaseClOrdIdIndexTableT::iterator it = OrderDatabaseClOrdIdIndexTable.find(cl_ord_id);
	if(it != OrderDatabaseClOrdIdIndexTable.end()){
		size_t &index = OrderDatabaseClOrdIdIndexTable.at(cl_ord_id);
		ord = OrderDatabase[index];
	}
	else{
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
			"query_request, error occurred :" << cl_ord_id);
	}
}

void
pending_ord_request_dao::
update_filled_info(const long& cl_ord_id, const double &px, const int &qty,const bool &is_final){
	 speculative_spin_mutex::scoped_lock lock(mtx_for_OrderDatabase);

	try{
		size_t &index = OrderDatabaseClOrdIdIndexTable.at(cl_ord_id);
		my_order &ord = OrderDatabase.at(index);
		ord.cum_qty += qty;
		ord.is_final = is_final;
		// remove final orders from index table
		if (true == ord.is_final){
			{
				 speculative_spin_mutex::scoped_lock lock(lock_local_request_cache);
				cl_ord_id_cache.erase(cl_ord_id);
			}

			OrderDatabaseModelIndexTable[ord.model_id].erase(index);
			OrderDatabaseSymbolTable[ord.symbol].erase(index);
			OrderDatabaseClOrdIdIndexTable.erase(ord.cl_ord_id);
		}
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				"update_filled_info, error occurred :" << cl_ord_id);
	}
}


/*
 * market making, quote notification
 */
void pending_ord_request_dao::cache(const T_RtnForQuote &ord,map<long,bool> &state){
	 speculative_spin_mutex::scoped_lock lock(lock_quote_notify_cache);

	map<long,vector<T_RtnForQuote> >::iterator it = quote_notify_cache.begin();
	map<long,vector<T_RtnForQuote> >::iterator end = quote_notify_cache.end();
	for(; it!=end; ++it){
		long modle_id = it->first;
		state[modle_id] = true;
		int cur_count = cache_quote_notify_count[modle_id];
		quote_notify_cache.find(modle_id)->second[cur_count] = ord;
		cache_quote_notify_count[modle_id] = cur_count + 1;
	}
}

void pending_ord_request_dao::clear_quote_notify_cache(const long &modle_id){
	 speculative_spin_mutex::scoped_lock lock(lock_quote_notify_cache);

	map<long,int>::iterator it = cache_quote_notify_count.find(modle_id);
	if (it != cache_quote_notify_count.end()){
		it->second = 0;
	}
	else{
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				"clear_quote_notify_cache, error occurred :can not find " << modle_id);
	}
}

vector<T_RtnForQuote>&
pending_ord_request_dao::read_quote_notify(const long &key,int &count,vector<T_RtnForQuote>& result)
{
	 speculative_spin_mutex::scoped_lock lock(lock_quote_notify_cache);

	vector<T_RtnForQuote> &records = quote_notify_cache.find(key)->second;
	records.swap(result);
	count = cache_quote_notify_count[key];
	cache_quote_notify_count[key] = 0;

	return records;
}

/*
 * market making, quote order
 */
void
pending_ord_request_dao::
query_quote_order(const long& cl_ord_id, QuoteOrder &ord)
{
	 speculative_spin_mutex::scoped_lock lock(mtx_for_QuoteOrderDatabase);

	try{
		size_t &index = QuoteOrderDatabaseClOrdIdIndexTable.at(cl_ord_id);
		ord = QuoteOrderDatabase[index];
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
			"query_quote_order, error occurred :" << cl_ord_id);
	}
}

bool pending_ord_request_dao::exist_quote_order(const long &cl_ord_id)
{
	bool exist = false;

	 speculative_spin_mutex::scoped_lock lock(lock_local_request_cache);

	if (cl_quote_ord_id_cache.find(cl_ord_id) != cl_quote_ord_id_cache.end()) exist = true;
	else exist = false;

	return exist;
}

void
pending_ord_request_dao::
update_quote_order(const side_options &side,const long &cl_ord_id,const long &ord_id,const state_options &status,
		const bool &is_final, const channel_errors &err)
{
	// market making.
	 speculative_spin_mutex::scoped_lock lock(mtx_for_QuoteOrderDatabase);

	try{
		size_t &pos = QuoteOrderDatabaseClOrdIdIndexTable.at(cl_ord_id);
		QuoteOrder &ord = QuoteOrderDatabase[pos];
		ord.ord_id = ord_id;
		if(side==side_options::buy){
			ord.buy_state = status;
			ord.buy_is_final = is_final;
		}else if(side==side_options::sell){
			ord.sell_state = status;
			ord.sell_is_final = is_final;
		}
		if(status==state_options::rejected){
			ord.buy_state = status;
			ord.buy_is_final = is_final;
			ord.sell_state = status;
			ord.sell_is_final = is_final;
		}
		ord.error_no = err;

		// remove final orders from index table
		if (true == ord.buy_is_final && true == ord.sell_is_final){
			{
				 speculative_spin_mutex::scoped_lock lock(lock_local_request_cache);
				cl_quote_ord_id_cache.erase(cl_ord_id);
			}

			unordered_set<size_t> & QuoteOrderDatabaseModelIndex = QuoteOrderDatabaseModelIndexTable[ord.model_id];
			QuoteOrderDatabaseModelIndex.erase(pos);
			QuoteOrderDatabaseSymbolTable[ord.stock_code].erase(pos);
			QuoteOrderDatabaseClOrdIdIndexTable.erase(ord.cl_ord_id);
		}
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				"update_quote_order, error occurred :" << cl_ord_id);
	}
}

void pending_ord_request_dao::insert_quote_order(const QuoteOrder &ord){
	// represent this order was placed by this my_exchange
	if (ord.model_id > 0){
		{
			 speculative_spin_mutex::scoped_lock lock(lock_local_request_cache);
			cl_quote_ord_id_cache.insert(ord.cl_ord_id);
		}
	}

	speculative_spin_mutex::scoped_lock lock(mtx_for_QuoteOrderDatabase);
    PRINT_OUT_OF_RANGE_WARN(quote_order_database_index,ORDER_DATABASE_MAX_SIZE);
	QuoteOrderDatabase[quote_order_database_index] = ord;
	QuoteOrderDatabaseClOrdIdIndexTable[ord.cl_ord_id] = quote_order_database_index;
	QuoteOrderDatabaseModelIndexTable[ord.model_id].insert(quote_order_database_index);
	QuoteOrderDatabaseSymbolTable[ord.stock_code].insert(quote_order_database_index);
	QuoteOrderDatabase[quote_order_database_index].buy_is_final = false;
	QuoteOrderDatabase[quote_order_database_index].buy_cum_qty = 0;
	QuoteOrderDatabase[quote_order_database_index].sell_is_final = false;
	QuoteOrderDatabase[quote_order_database_index].sell_cum_qty = 0;
	if (ord.request_type == request_types::cancel_order){
		QuoteOrderDatabase[quote_order_database_index].buy_state = state_options::pending_cancel;
		QuoteOrderDatabase[quote_order_database_index].sell_state = state_options::pending_cancel;
	}
	else if (ord.request_type == request_types::place_order){
		QuoteOrderDatabase[quote_order_database_index].buy_state = state_options::pending_new;
		QuoteOrderDatabase[quote_order_database_index].sell_state = state_options::pending_new;
		QuoteOrderDatabase[quote_order_database_index].orig_cl_ord_id = 0;
		QuoteOrderDatabase[quote_order_database_index].orig_ord_id = 0;
	}

	quote_order_database_index++;
}

void
pending_ord_request_dao::
query_not_final_quote_order(const long &model_id,QuoteOrderRecordT &ords, int &total){
	 speculative_spin_mutex::scoped_lock lock(mtx_for_QuoteOrderDatabase);

	total = 0;
	try{
		unordered_set<size_t> &indexes = QuoteOrderDatabaseModelIndexTable.at(model_id);
		unordered_set<size_t>::iterator it = indexes.begin();
		unordered_set<size_t>::iterator end = indexes.end();
		for(; it!=end; ++it){
			reset_quote_order(ords[total]);
			QuoteOrder &ord = QuoteOrderDatabase[*it];
			if(request_types::quote == ord.request_type){
				ords[total] = ord;
				total++;
			}
		}
	}
	catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
			"query_not_final_quote_order, error occurred :" << model_id);
	}

}

void
pending_ord_request_dao::
query_not_final_quote_order_by_symbol(const string &symbol,ReportNotifyRecordT &ords, int &count)
{
	 speculative_spin_mutex::scoped_lock lock(mtx_for_QuoteOrderDatabase);

	try{
		QuoteOrderDatabaseSymbolTableT::iterator it = QuoteOrderDatabaseSymbolTable.find(symbol);
		if(it != QuoteOrderDatabaseSymbolTable.end()){
			unordered_set<size_t> &indexes = QuoteOrderDatabaseSymbolTable.at(symbol);
			unordered_set<size_t>::iterator it = indexes.begin();
			unordered_set<size_t>::iterator end = indexes.end();
			for(; it!=end; ++it){
				reset(ords[count]);
				QuoteOrder &ord = QuoteOrderDatabase[*it];
				if(request_types::quote == ord.request_type){
					ords[count].cl_ord_id = ord.cl_ord_id;
					ords[count].ord_id = ord.ord_id;
					ords[count].last_px = ord.buy_last_px;
					ords[count].last_qty = ord.buy_last_qty;
					ords[count].cum_qty = ord.buy_cum_qty;
					ords[count].cum_amount = ord.buy_cum_amount;
					ords[count].price = ord.buy_limit_price;
					ords[count].volume = ord.buy_volume;
					ords[count].side = side_options::buy;
					count++;

					reset(ords[count]);
					ords[count].cl_ord_id = ord.cl_ord_id;
					ords[count].ord_id = ord.ord_id;
					ords[count].last_px = ord.sell_last_px;
					ords[count].last_qty = ord.sell_last_qty;
					ords[count].cum_qty = ord.sell_cum_qty;
					ords[count].cum_amount = ord.sell_cum_amount;
					ords[count].price = ord.sell_limit_price;
					ords[count].volume = ord.sell_volume;
					ords[count].side = side_options::sell;
					count++;
				}
			}
		}else{
			count = 0;
		}

	}
	catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				"query_not_final_quote_order_by_symbol, error occurred :" << symbol);
	}
}

void pending_ord_request_dao::reset_quote_order(QuoteOrder &ord){
	ord = QuoteOrder();
}

void
pending_ord_request_dao::
update_quote_fill(side_options &side,const long& cl_ord_id, const double &px,
		const int &qty,const bool &is_final)
{
	 speculative_spin_mutex::scoped_lock lock(mtx_for_QuoteOrderDatabase);

	try{
		size_t &index = QuoteOrderDatabaseClOrdIdIndexTable.at(cl_ord_id);
		QuoteOrder &quote = QuoteOrderDatabase.at(index);
		if(side==side_options::buy){
			quote.buy_cum_qty += qty;
			quote.buy_cum_amount += px*qty;
			quote.buy_is_final = is_final;
		}else if(side==side_options::sell){
			quote.sell_cum_qty += qty;
			quote.sell_cum_amount += px*qty;
			quote.sell_is_final = is_final;
		}

		// remove final orders from index table
		if (true == quote.buy_is_final && true == quote.sell_is_final){
			{
				 speculative_spin_mutex::scoped_lock lock(lock_local_request_cache);
				cl_quote_ord_id_cache.erase(cl_ord_id);
			}

			QuoteOrderDatabaseModelIndexTable[quote.model_id].erase(index);
			QuoteOrderDatabaseSymbolTable[quote.stock_code].erase(index);
			QuoteOrderDatabaseClOrdIdIndexTable.erase(quote.cl_ord_id);
		}
	}
	catch (exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
			"update_quote_fill, error occurred :" << cl_ord_id);
	}
}
