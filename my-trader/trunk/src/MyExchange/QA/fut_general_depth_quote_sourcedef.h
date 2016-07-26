#include <iostream>
#include <functional>   // std::bind
#include <thread>
#include <stdio.h>      /* printf, tmpnam, L_tmpnam, remove */
#include <stdlib.h>     /* exit */
#include <signal.h>     /* signal */
#include <boost/array.hpp>
#include <sys/types.h>
#include <signal.h>
#include <cstdlib>
#include <exception>
#include <boost/foreach.hpp>
#include <sstream>
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include "quote_source_setting.h"
#include <chrono>
#include <ctime>
#include <ratio>
#include <ctime>
#include "typeutils.h"
#include "pending_quote_dao.h"
#include "../my_exchange.h"
#include <regex>

#ifdef rss
	#include <typeinfo>       // operator typeid
	#include "tcs.h"
	#include "rss_router.h"
	#include "rsssynchro.h"
	using namespace trading_channel_agent;
#endif

using namespace boost::posix_time;
using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace quote_agent;
using namespace std;

#ifdef rss
	#include "rss_quote_playback.h"
#endif

template<typename QuoteT>
quote_source<QuoteT>::quote_source(quote_source_setting setting)
:_subscribed(false),stopped(false),setting_(setting){
#ifdef rss
	myself_type_ = quote_src_type_options::UNASSIGNED;
	if (this->setting_.category==quote_category_options::SPIF){
		myself_type_=quote_src_type_options::IF;
	}
	else if (this->setting_.category==quote_category_options::CF){
		//quote_playback::my_shfe_handler = bind(&quote_source<QuoteT>::OnGTAQuoteData,this,_1);
		//quote_playback::shfe_ex_handler = bind(&quote_source<QuoteT>::OnGTAQuoteData,this,_1);
		myself_type_= quote_src_type_options::CF;
	}
	else if (this->setting_.category==quote_category_options::Stock){
		myself_type_=quote_src_type_options::Stock;
	}
	else if (this->setting_.category==quote_category_options::FullDepth&&
		IsIntegerT<QuoteT>::No){
		throw exception();
		myself_type_=quote_src_type_options::Full;
	}
	else if (this->setting_.category==quote_category_options::MDOrderStatistic){
		myself_type_=quote_src_type_options::Quote5;
	}
//	quote_src_.start();
#endif

	this->setting_ = setting;
	_forwarder = NULL;
}

template<typename QuoteT>
quote_source<QuoteT>::~quote_source(void){
	finalize();
}

template<typename QuoteT>
void quote_source<QuoteT>::intialize(void){
	stopped = false;

	// 执行订阅
	SubscribeContracts contracts;
	map<long, SubscribeContracts>::iterator it = pending_quote_dao<QuoteT>::quote_subscribtion.begin();
	map<long, SubscribeContracts>::iterator end = pending_quote_dao<QuoteT>::quote_subscribtion.end();
	// traversal model id
	for (; it != end; it++){
		SubscribeContracts::iterator it_symbol = it->second.begin();
		SubscribeContracts::iterator end_symbol = it->second.end();
		// traversal contracts subscribed by current model
		for (; it_symbol != end_symbol; it_symbol++){
			SubscribeContracts::iterator found = contracts.find(*it_symbol);
			if ( found == contracts.end()){
				contracts.insert(*it_symbol);
			}
		}
	}
	this->subscribe_to_symbols(contracts);
}

template<typename QuoteT>
void quote_source<QuoteT>::finalize(void){
	stopped = true;

	if (NULL != _forwarder){
		delete _forwarder;
		_forwarder = NULL;
	}
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete quote source successfully.");
}

template<typename QuoteT>
void quote_source<QuoteT>::OnGTAQuoteData(const QuoteT *quote_src){
	// 执行行情通知操作
	if (_subscribed){
		{
			lock_guard<mutex> lock(quote_ready_mtx);

			// duplicated one quote
			QuoteT quote = *quote_src;

			process_one_quote(quote);
			QuoteStateT::iterator it = quote_state.begin();
			QuoteStateT::iterator end = quote_state.end();
			for(;it!=end;++it) it->second = true;
		}
		quote_ready_cv.notify_all();
	}
}

template<typename QuoteT>
void quote_source<QuoteT>::process_one_quote(QuoteT &quote){
	typedef map<long,SubscribeContracts> SubscriptionTableT;
	SubscriptionTableT::iterator it = (pending_quote_dao<QuoteT>::quote_subscribtion).begin();
	SubscriptionTableT::iterator end = (pending_quote_dao<QuoteT>::quote_subscribtion).end();
	// 遍历每个模型的订阅列表
	for (; it!=end; it++){
		long model_id = it->first;
		SubscribeContracts &symbols = it->second;
		// 如果模型订阅该合约，则为该合约插入行情记录
		string symbol = pending_quote_dao<QuoteT>::get_symbol(quote);
		if (true == this->match(symbol,symbols)){
			pending_quote_dao<QuoteT>::insert_quote(model_id, quote);
			pending_quote_dao<QuoteT>::set_local_timestamp(quote);
		}
	}
}

template<typename QuoteT>
bool quote_source<QuoteT>::match(string &lockup_value,SubscribeContracts &lookup_array){
	bool found = false;

	SubscribeContracts::iterator it = lookup_array.begin();
	SubscribeContracts::iterator end = lookup_array.end();
	for( ; it!=end; ++it){
		regex r(*it,regex::grep);
		if (std::regex_match (lockup_value, r)){
			found = true;
			break;
		}
	}

	return found;
}

template<typename QuoteT>
void quote_source<QuoteT>::subscribe_to_symbols(SubscribeContracts subscription){
	if (this->setting_.quote_type == quote_type_options::local){
		if (IsIntegerT<QuoteT>::No){
			LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"mytrader do NOT sopport local type of quote.");
			throw exception();
		}
	}
	else if (this->setting_.quote_type == quote_type_options::forwarder){
		if (IsIntegerT<QuoteT>::No){
#ifndef rss
			_forwarder = new quote_forwarder_agent<QuoteT>(setting_);
			function<void (const QuoteT *)> f = bind(&quote_source<QuoteT>::OnGTAQuoteData, this, _1);
			_forwarder->SetQuoteDataHandler(f);
			_forwarder_thread = thread(bind(&quote_forwarder_agent<QuoteT>::start,_forwarder));
			this_thread::sleep_for(std::chrono::microseconds(10));
#endif
			_subscribed = true;
		}
	}
}

template<typename QuoteT>
void quote_source<QuoteT>::subscribe_to_quote(const long & model_id, SubscribeContracts contracts){
	quote_state[model_id] = false;
	pending_quote_dao<QuoteT>::init(model_id);
	pending_quote_dao<QuoteT>::quote_subscribtion.insert( pair<long, SubscribeContracts>(model_id, contracts));
}
