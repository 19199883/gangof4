#include <boost/shared_ptr.hpp>
#include <sstream>
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include "tca.h"
#include <boost/foreach.hpp>
#include "pending_ord_request_dao.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace trading_channel_agent;

long tca::request_counter = 1000000;
speculative_spin_mutex  tca::mut_for_request;
speculative_spin_mutex  tca::mu_for_request_counter;

tca::tca(void){
	tcs_map = map<pair<long,exchange_names>,tcs*>();
	_split_cache = vector<int>(100);
	_split_cache.reserve(100);
}

tca::~tca(void)
{	
	finalize();
}

void tca::initialize(void)
{
	int count = 1;

	// 初始化tca的配置信息
	this->setting.Initialize();
	// 加载quote_source
	BOOST_FOREACH( tcs_setting source_setting, this->setting.tcs_sources ){
		// TO IMPROVE: 统一改成智能指针
        tcs *source = new tcs(source_setting);
		this->sources.push_back(source);
		source->initialize();
		source->setting.id = count++;
    }
}

vector<tcs*> tca::get_tcs(const long &model_id)
{
	vector<tcs*> tcses;
	BOOST_FOREACH( tcs* tcs, this->sources )
	{
		if (tcs->setting.models.find(model_id) != tcs->setting.models.end()){
			tcses.push_back(tcs);
		}
	}
	return tcses;
}

void tca:: finalize(void)
{	
	// 终止配置对象
	this->setting.finalize();

	// 终止tcs
	BOOST_FOREACH( tcs *source, this->sources )
    {        	
		//source->finalize();
		delete source;
		source = 0;
    }

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"deleted tca succesfully.");

	sources.clear();
}

long tca::generate_cl_ord_id(const long &model_id , const long &signal_id)
{
	long cl_ord_id;
	speculative_spin_mutex::scoped_lock lock(mu_for_request_counter);
	// T29
    cl_ord_id = model_id + tca::request_counter*100000000;		
	request_counter++;

	return cl_ord_id;
}

void tca::get_not_final_requests(const long& model_id,vector<my_order> &ords, int &count)
{
	pending_ord_request_dao::query_not_final_ord_requests(model_id,ords,count);
}

int tca::split(string symbol,int ori_vol,vector<int> &split_vols){
	int count = 0;
	int max_vol = this->setting.ord_vol_limits[symbol];
	int left_vol = ori_vol;
	while(max_vol > 0 && left_vol > max_vol){
		split_vols[count] =max_vol;
		left_vol = left_vol - max_vol;
		count++;
	}
	split_vols[count] = left_vol;
	count++;

	return count;
}

void tca::place_orders(my_order &ord_ori,set<long> &new_ords){
	my_order ord  = ord_ori;
	// 分配ClOrdID
	ord.cl_ord_id = tca::generate_cl_ord_id(ord.model_id, ord.signal_id);
	new_ords.insert(ord.cl_ord_id);
	ord.state = state_options::pending_new;
	ord.is_final = false;
	pending_ord_request_dao::insert_request(ord);
	tcs * target = get_tcs(pair<long,exchange_names>(ord.model_id,ord.exchange));
	if (target != NULL){
		target->place_request(ord);
	}
	else{
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"configuration error, one model is not mapped to certain channel. model id:" << ord.model_id);
	}
}

void tca::req_quote(my_order &ord_ori){
	my_order &ord  = ord_ori;
	ord.cl_ord_id = tca::generate_cl_ord_id(ord.model_id, ord.signal_id);
	ord.is_final = false;
	pending_ord_request_dao::insert_request(ord);
	tcs * target = get_tcs(pair<long,exchange_names>(ord.model_id,ord.exchange));
	if (target != NULL){
		target->req_quote(ord);
	}
	else{
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"configuration error, one model is not mapped to certain channel. model id:" << ord.model_id);
	}

}

void tca::quote(QuoteOrder &ord)
{
	// 分配ClOrdID
	ord.cl_ord_id = tca::generate_cl_ord_id(ord.model_id, ord.signal_id);
	ord.buy_state = state_options::pending_new;
	ord.buy_is_final = false;
	ord.sell_state = state_options::pending_new;
	ord.sell_is_final = false;
	pending_ord_request_dao::insert_quote_order(ord);

	tcs * target = get_tcs(pair<long,exchange_names>(ord.model_id,ord.exchange));
	if (target != NULL){
		target->quote(ord);
	}
	else{
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
			"configuration error, one model is not mapped to certain channel. model id:" << ord.model_id);
	}
}

void tca::cancel_ord(my_order &ord)
{
	// 分配ClOrdID
	long ori_cl_ord_id = ord.cl_ord_id;
	long ori_ord_id = ord.ord_id;
	ord.cl_ord_id = tca::generate_cl_ord_id(ord.model_id, ord.signal_id);
	ord.is_final = false;
	ord.request_type = request_types::cancel_order;
	ord.state = state_options::pending_cancel;
	ord.ord_id = 0;
	ord.orig_cl_ord_id = ori_cl_ord_id;
	ord.orig_ord_id = ori_ord_id;
	if (ori_ord_id == 0){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"error occurs when cancel order，order id:" << ori_ord_id
				<< ",orig_cl_ord_id:" << ord.orig_cl_ord_id
				<< ",cl_ord_id:" << ord.cl_ord_id
				);
	}
	pending_ord_request_dao::insert_request(ord);
	tcs * target = get_tcs(pair<long,exchange_names>(ord.model_id,ord.exchange));
	if (target != NULL){
		target->cancel_request(ord);
	}
	else{
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"configuration error, one model is not mapped to certain channel. model id:" << ord.model_id);
	}
}

void tca::cancel_quote(QuoteOrder &ord)
{
	long ori_cl_ord_id = ord.cl_ord_id;
	long ori_ord_id = ord.ord_id;
	ord.cl_ord_id = tca::generate_cl_ord_id(ord.model_id, ord.signal_id);
	ord.request_type = request_types::cancel_order;
	ord.buy_state = state_options::pending_cancel;
	ord.sell_state = state_options::pending_cancel;
	ord.ord_id = 0;
	ord.orig_cl_ord_id = ori_cl_ord_id;
	ord.orig_ord_id = ori_ord_id;
	if (ori_ord_id == 0){
//		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
//			"error occurs when cancel order，order id:" << ori_ord_id
//			<< ",orig_cl_ord_id:" << ord.orig_cl_ord_id
//			<< ",cl_ord_id:" << ord.cl_ord_id
//			);
	}
	pending_ord_request_dao::insert_quote_order(ord);
	tcs * target = get_tcs(pair<long,exchange_names>(ord.model_id,ord.exchange));
	if (target != NULL){
		target->cancel_quote(ord);
	}
	else{
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"configuration error, one model is not mapped to certain channel. model id:" << ord.model_id);
	}
}

void tca::register_strategy(const long &model_id, const set<exchange_names> &exchanges)
{
	ReportNotifyTableKeyT csp_key = ReportNotifyTableKeyT(model_id,"cancel_signal_processor");
	pending_ord_request_dao::init(csp_key);


	ReportNotifyTableKeyT st_unit_key = ReportNotifyTableKeyT(model_id,"strategy_unit");
	pending_ord_request_dao::init(st_unit_key);

	set<exchange_names>::iterator it = exchanges.begin();
	set<exchange_names>::iterator end = exchanges.end();
	for (; it != end; ++it)
	{
		TcsTableKeyT key = TcsTableKeyT(model_id,*it);
		if (tcs_map.find(key) != tcs_map.end()){
			LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
					"configuration error, one model is mapped to multiple channels." << model_id);
		}
		else{
			tcs * target = get_tcs(model_id,*it);
			if (target == NULL){
				LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
						"configuration error, one model is not mapped to certain channel,model id" << model_id);
			}
			else{
				target->init(model_id);

				tcs_map[key] = target;
			}
		}
	}
}

tcs* tca::get_tcs(const long &model_id,const exchange_names &exchange)
{
	tcs* target = NULL;

	BOOST_FOREACH( tcs* tcs, this->sources )
	{
		if (tcs->setting.models.find(model_id) != tcs->setting.models.end() &&
				tcs->setting.exchanges.find(exchange) != tcs->setting.exchanges.end())
		{
			target = tcs;
			break;
		}
	}
	return target;
}

tcs* tca::get_tcs(const pair<long,exchange_names> &key)
{
	map<pair<long,exchange_names>,tcs*>::iterator it = tcs_map.find(key);
	if (it == tcs_map.end())
	{
		return NULL;
	}
	else
	{
		return it->second;
	}
}
