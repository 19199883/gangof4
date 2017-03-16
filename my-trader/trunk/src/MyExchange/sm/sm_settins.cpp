#include <boost/filesystem/path.hpp>
#include "boost/algorithm/string.hpp"  
#include "boost/filesystem/path.hpp"  
#include "boost/filesystem/operations.hpp"  
#include "boost/format.hpp" 
#include "sm_settings.h"
#include <string>     // std::string, std::stol
#include "model_config.h"
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>

using namespace strategy_manager;
using namespace std;
using namespace quote_agent;
using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

string sm_settings::config_path = "trasev.config";

sm_settings::sm_settings(void){

}

sm_settings::~sm_settings(void){}

void sm_settings::initialize(void){
	//����һ��XML���ĵ�����
	TiXmlDocument config = TiXmlDocument(sm_settings::config_path.c_str());
    config.LoadFile();
    //��ø�Ԫ�أ���MyExchange
    TiXmlElement *RootElement = config.RootElement();    
    //��õ�һ��strategies�ڵ�
    TiXmlElement *strategies = RootElement->FirstChildElement("strategies");
	if (strategies != 0){
		TiXmlElement *strategy = strategies->FirstChildElement();
		while (strategy != 0){
			model_setting source = this->create_model_setting(strategy);
			source.account = this->get_account(source.id);
			this->models.push_back(source);
			strategy = strategy->NextSiblingElement();			
		}
	}  
}

void sm_settings::init_model_if(model_setting &strategy)
{
	// model interfaces
	TiXmlDocument model_if_config = TiXmlDocument("straif.config");
	model_if_config.LoadFile();
	TiXmlElement *model_if_root_ele = model_if_config.RootElement();
	TiXmlElement *strategy_ele = model_if_root_ele->FirstChildElement("strategy");
	// init������
	strategy.init_method = strategy_ele->Attribute("init_method");
	// feed_pos_method������
	strategy.feed_pos_method = strategy_ele->Attribute("feed_pos_method");
	// feed_init_pos_method������
	strategy.feed_init_pos_method = strategy_ele->Attribute("feed_init_pos_method");
	// feed_quote_method������
	strategy.feed_spif_quote_method = strategy_ele->Attribute("feed_spif_quote_method");
	strategy.feed_cf_quote_method = strategy_ele->Attribute("feed_cf_quote_method");
	strategy.feed_stock_quote_method = strategy_ele->Attribute("feed_stock_quote_method");
	// feed_quote_method������
	strategy.feed_full_depth_quote_method = strategy_ele->Attribute("feed_full_depth_quote_method");
	// MDOrderStatistic_method
	strategy.MDOrderStatistic_method = strategy_ele->Attribute("MDOrderStatistic_method");
	// feed_sig_response_method
	strategy.feed_sig_response_method = strategy_ele->Attribute("feed_sig_response_method");
	// feed_time_event_method
	strategy.feed_time_event_method = strategy_ele->Attribute("feed_time_event_method");
	// feed_time_event_method
	strategy.destroy_method = strategy_ele->Attribute("destroy_method");
	// feed_quote_notify_method
	strategy.feed_quote_notify_method = strategy_ele->Attribute("feed_quote_notify_method");

	strategy.init_option_strategy = strategy_ele->Attribute("init_option_strategy");
	strategy.update_option_strategy = strategy_ele->Attribute("update_option_strategy");
	strategy.get_monitor_info = strategy_ele->Attribute("get_monitor_info");
}

model_setting sm_settings::create_model_setting(TiXmlElement* xml_ele)
{
	model_setting strategy;

	strcpy(strategy.config_.st_name, xml_ele->Attribute("model_file"));

	strategy.config_.st_id = stoi(xml_ele->Attribute("id"));
	// T29
	strategy.id = stol(xml_ele->Attribute("id"));
	if(strategy.id >= 100000000){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
			"strategy config error: the specified value of id is beyond the allowed maximum(99999999).");
	}

	xml_ele->QueryIntAttribute("cancel_timeout",&strategy.cancel_timeout);

	// ģ��dll�ļ�
	strategy.file = xml_ele->Attribute("model_file");

	char sah = xml_ele->Attribute("sah_flag")[0];
	strategy.sah_ = static_cast<sah_options>(sah);

	init_model_if(strategy);

	// log_id
	strategy.config_.log_id = strategy.id *10 + 0;
	// log_name
	strcpy(strategy.config_.log_name, xml_ele->Attribute("log_name"));
	// iv_id
	strategy.config_.iv_id = strategy.id *10 + 1;
	// iv_name
	strcpy(strategy.config_.iv_name ,xml_ele->Attribute("iv_name"));
	// ev_id
	strategy.config_.ev_id = strategy.id*10 + 2;
	// ev_name
	const char * name = xml_ele->Attribute("ev_name");
	strcpy(strategy.config_.ev_name ,xml_ele->Attribute("ev_name"));
	// orders_limit_per_sec
	xml_ele->QueryIntAttribute("orders_limit_per_sec",&strategy.config_.orders_limit_per_sec);
	//	cancel_limit_per_day
	xml_ele->QueryIntAttribute("cancel_limit_per_day",&strategy.config_.cancel_limit_per_day);
	// option
	xml_ele->QueryBoolAttribute("isOption",&strategy.isOption);

	// Ϊģ��������Ӻ�Լ
	int counter = 0;
	TiXmlElement* symbol_ele = xml_ele->FirstChildElement();		
	while (symbol_ele != 0)	{		
		symbol_t &tmp = strategy.config_.symbols[counter];

		symbol_ele->QueryIntAttribute("max_pos", &tmp.max_pos);
		strcpy(tmp.name,symbol_ele->Attribute("name"));
		// orders_limit_per_sec
		symbol_ele->QueryIntAttribute("orders_limit_per_sec",&tmp.orders_limit_per_sec);
		//	cancel_limit_per_day
		symbol_ele->QueryIntAttribute("cancel_limit_per_day",&tmp.cancel_limit_per_day);
		// symbol_log_id
		symbol_ele->QueryIntAttribute("symbol_log_id",&tmp.symbol_log_id);
		// symbol_log_name
		strcpy(tmp.symbol_log_name, symbol_ele->Attribute("symbol_log_name"));

		string exc_str = symbol_ele->Attribute("exchange");
		tmp.exchange = static_cast<exchange_names>(exc_str[0]);
		
		string categories_str = symbol_ele->Attribute("category");
		categories_str = categories_str+",";
		size_t found = 0;
		size_t start = 0;
		found = categories_str.find(",",start);
		while (found != std::string::npos){
			string category_str = categories_str.substr(start,found-start);
			quote_category_options category;
			if (category_str == "fulldepth")  category = quote_category_options::FullDepth;
			else if (category_str == "spif")  category = quote_category_options::SPIF;
			else if (category_str == "cf")    category = quote_category_options::CF;
			else if (category_str == "stock") category = quote_category_options::Stock;
			else if (category_str == "MDOrderStatistic") category = quote_category_options::MDOrderStatistic;
			strategy.subscription[category].insert(tmp.name);
			start = found+1;
			found = categories_str.find(",",start);
		 } // end while (found != std::string::npos){

		// ��һ��strategyԪ��
		symbol_ele = symbol_ele->NextSiblingElement();
		counter++;
	}	//end while (symbol_ele != 0)
	strategy.config_.symbols_cnt = counter;
	   
	return strategy;
}

void sm_settings::finalize(void)
{
}	

string sm_settings::get_account(long model_id)
{
	string fund_acc = "";

	//����һ��XML���ĵ�����
	TiXmlDocument config = TiXmlDocument(sm_settings::config_path.c_str());
	config.LoadFile();
	//��ø�Ԫ�أ���MyExchange
	TiXmlElement *RootElement = config.RootElement();
	//��õ�һ��tca�ڵ�
	TiXmlElement *tca = RootElement->FirstChildElement("tca");
	string ex_file = "";
	if (tca != 0){
		TiXmlElement *src = tca->FirstChildElement();
		while (src != 0){
			string model_str = src->Attribute("models");
			model_str = model_str + ",";
			int pos = 0;
			int cur_pos = 0;
			while((pos=model_str.find(",",cur_pos)) != string::npos)
			{
				string model_id_str = model_str.substr(cur_pos,pos-cur_pos);
				if(model_id == stol(model_id_str)){
					ex_file = src->Attribute("config");
					break;
				}
				cur_pos = pos+1;
			}

			src = src->NextSiblingElement();
		} // end while (src != 0)
	} // end if (tca != 0){

	if(ex_file != ""){
		// read my_exchange configuration to get tunnel configuration file
		TiXmlDocument ex_file_doc = TiXmlDocument(ex_file.c_str());
		ex_file_doc.LoadFile();
		TiXmlElement *ex_root = ex_file_doc.RootElement();
		string tunnel_file = ex_file;

		// read my_tunnel configuration to get fund account
		TiXmlDocument tunnel_file_doc = TiXmlDocument(tunnel_file.c_str());
		tunnel_file_doc.LoadFile();
		TiXmlElement *tunnel_root = tunnel_file_doc.RootElement();
		TiXmlElement *login_ele = tunnel_root->FirstChildElement("login");
		TiXmlElement *investorid_ele = login_ele->FirstChildElement("investorid");

		fund_acc = investorid_ele->GetText();
	} // end if(ex_file!=""){

	return fund_acc;
}


