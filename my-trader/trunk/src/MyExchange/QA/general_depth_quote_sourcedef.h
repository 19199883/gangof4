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
#include <string>
#include "maint.h"

using namespace boost::posix_time;
using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace quote_agent;
using namespace std;

template<typename QuoteT>
quote_source<QuoteT>::quote_source(quote_source_setting setting,MYQuoteData *md_provider)
:_subscribed(false),stopped(false),setting_(setting)
{
	md_provider_ = md_provider;
	this->setting_ = setting;
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
}

template<typename QuoteT>
void quote_source<QuoteT>::finalize(void){
	stopped = true;

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete quote source successfully.");
}

template<typename QuoteT>
void quote_source<QuoteT>::OnGTAQuoteData(const QuoteT *quote_src){
	// TODO: improve: combine forwarder and trader into the same process
	// maint.                                                    
	if(maintenance::enabled()){                                                                                         
		string contract = pending_quote_dao<QuoteT>::get_symbol(quote_src);
		contract += "(qa rev)";
		maintenance::log(contract);	
	}   


	// 执行行情通知操作
	if (_subscribed){
	    {
			lock_guard<mutex> lock(quote_ready_mtx);
			process_one_quote(quote_src);
       }
       quote_ready_cv.notify_all();
	}
}

template<typename QuoteT>
void quote_source<QuoteT>::process_one_quote(const QuoteT *quote){
	string symbol = pending_quote_dao<QuoteT>::get_symbol(quote);

    //推送非期权策略
    int index = get_hash_index(symbol.c_str(), symbol.length());
	typename pending_quote_dao<QuoteT>::ContractNode* p_node = NULL;
	p_node = pending_quote_dao<QuoteT>::contract_model_hash_map[index];
	typename pending_quote_dao<QuoteT>::ModuleNode * p_module = NULL;
	while (p_node){
		if (0 == strcmp(p_node->contract, symbol.c_str())){
			p_module = p_node->module;
			while (p_module){
				pending_quote_dao<QuoteT>::set_local_timestamp(quote);
				pending_quote_dao<QuoteT>::insert_quote(p_module->module_id, quote);
		        quote_state[p_module->module_id] = true;
		        p_module = p_module->next;
			}
			break;
		}
		p_node = p_node->next;
	}

	/*
	auto model_it = pending_quote_dao<QuoteT>::contract_model_map.find(symbol);
	if (model_it != pending_quote_dao<QuoteT>::contract_model_map.end()){
		for (auto it = model_it->second.begin(); it != model_it->second.end(); ++it){
			//LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"insert quote. models:"	<< *it);
			pending_quote_dao<QuoteT>::set_local_timestamp(quote);
			pending_quote_dao<QuoteT>::insert_quote(*it, quote);
            quote_state[*it] = true;
            __sync_add_and_fetch(&push_module_count,1);
		}
	}
    */

	// TODO: improve
    //推送期权策略
//	typedef map<long,SubscribeContracts> SubscriptionTableT;
//	SubscriptionTableT::iterator it = (pending_quote_dao<QuoteT>::quote_subscribtion).begin();
//	SubscriptionTableT::iterator end = (pending_quote_dao<QuoteT>::quote_subscribtion).end();	
//	for (; it!=end; it++){
//		long model_id = it->first;
//		SubscribeContracts &symbols = it->second;
//		if (true == this->match(symbol,symbols)){
//			pending_quote_dao<QuoteT>::set_local_timestamp(quote);
//			pending_quote_dao<QuoteT>::insert_quote(model_id, quote);
//            quote_state[model_id] = true;
//		}
//	}
}

template<typename QuoteT>
bool quote_source<QuoteT>::match(string &lockup_value,SubscribeContracts &lookup_array){

	bool found = false;

	SubscribeContracts::iterator it = lookup_array.begin();
	SubscribeContracts::iterator end = lookup_array.end();
	for( ; it!=end; ++it){
		std::regex rex = std::regex((*it).c_str());
		if (std::regex_match(lockup_value.c_str(),rex)){
		//if (0 == strcmp(lockup_value.c_str(),(*it).c_str())){
			found = true;
			break;
		}
	}

	return found;
}

template<typename QuoteT>
void quote_source<QuoteT>::subscribe_to_symbols(SubscribeContracts subscription){
		// TODO:improve 1
		if (IsIntegerT<QuoteT>::No){
			boost::function<void (const QuoteT *)> f = boost::bind(&quote_source<QuoteT>::OnGTAQuoteData, this, _1);
			md_provider_->SetQuoteDataHandler(f);
			_subscribed = true;
		}
}

template<typename QuoteT>
void quote_source<QuoteT>::subscribe_to_quote(const long & model_id,bool isOption, SubscribeContracts contracts)
{
    quote_state[model_id] = false;    
    pending_quote_dao<QuoteT>::all_model.insert(model_id);
    pending_quote_dao<QuoteT>::init(model_id);

	//如果非期权(股票、期货)，使用合约名称做map的key
	int index = 0;
	typename pending_quote_dao<QuoteT>::ModuleNode * p_module = NULL;
	for (auto it : contracts){
		index = get_hash_index(it.c_str(), it.length());
		typename pending_quote_dao<QuoteT>::ContractNode* p_node = NULL;
		p_node = pending_quote_dao<QuoteT>::contract_model_hash_map[index];
		while (p_node){
			//存在合约节点
			if (0 == strcmp(it.c_str(), p_node->contract)){
				//合约一样
				p_module = (typename pending_quote_dao<QuoteT>::ModuleNode*)malloc(sizeof(typename pending_quote_dao<QuoteT>::ModuleNode));
				p_module->module_id = model_id;
				p_module->next = p_node->module;
				p_node->module = p_module;
				LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "add module. index:" << index <<" insert models:"<< model_id << " contract:"<< it.c_str());
				break;
			}
			p_node = p_node->next;
		}
		if (!p_node){

			p_module = (typename pending_quote_dao<QuoteT>::ModuleNode*)malloc(sizeof(typename pending_quote_dao<QuoteT>::ModuleNode));
			p_module->module_id = model_id;
			p_module->next = NULL;

			p_node = (typename pending_quote_dao<QuoteT>::ContractNode*)malloc(sizeof(typename pending_quote_dao<QuoteT>::ContractNode));
			strncpy(p_node->contract, it.c_str(),sizeof(p_node->contract));
			p_node->module = p_module;

			typename pending_quote_dao<QuoteT>::ContractNode* block = pending_quote_dao<QuoteT>::contract_model_hash_map[index];
			if (block){
				//p_node->next = block->next;
				//block->next = p_node;
				p_node->next = block;
				pending_quote_dao<QuoteT>::contract_model_hash_map[index] = p_node;
				LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "add node. index:" << index <<" insert models:"<< model_id << " contract:"<< it.c_str());
			}else{
				pending_quote_dao<QuoteT>::contract_model_hash_map[index] = p_node;
				p_node->next = NULL;
				LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "add block. index:" << index <<" insert models:"<< model_id << " contract:"<< it.c_str());
			}
			//不存在合约节点
		}
		/*
		auto model = pending_quote_dao<QuoteT>::contract_model_map.find(it);
		if (model == pending_quote_dao<QuoteT>::contract_model_map.end()){
			set<long> models;
			models.insert(model_id);
			pending_quote_dao<QuoteT>::contract_model_map[it] = models;
		}else{
			model->second.insert(model_id);
		}
		*/
		//LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"insert models:"	<< it.c_str() << " model id:" << model_id);
	}    
    
	this->subscribe_to_symbols(contracts);
}
