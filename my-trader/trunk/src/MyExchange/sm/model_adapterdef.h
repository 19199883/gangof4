#include <sstream>
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include <boost/foreach.hpp>
#include "../my_exchange.h"
#include <boost/date_time.hpp>
#include "pending_quote_dao.h"
#include <ctime>
#include <ratio>
#include <chrono>
#include "my_trade_tunnel_api.h"
#include <regex>
#include "maint.h"

#include <time.h>       /* time_t, struct tm, time, localtime, strftime */


using namespace boost::gregorian;
using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace strategy_manager;

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::model_adapter(model_setting _setting)
{
	InitFAddr = NULL;
	FeedPositionF = NULL;
	FeedInitPositionF = NULL;
	FeedSPIFQuoteF = NULL;
	FeedCFQuoteF = NULL;;
	FeedStockQuoteF = NULL;;
	FeedFullDepthQuoteF = NULL;
	FeedStatisticF = NULL;
	FeedSignalResponseF = NULL;
	FeedTimeEventF = NULL;
	DestroyF=NULL;
	FeedInitPositionF = NULL;
	FeedQuoteNotifyF = NULL;
	InitStrategyF = NULL;
	UpdateStrategyF = NULL;
	GetMonitorInfoF = NULL;

	this->setting = _setting;
	this->_pproxy = CLoadLibraryProxy::CreateLoadLibraryProxy();
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::~model_adapter(void)
{
	finalize();
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
string model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::generate_log_name(char* log_path)
 {
	string log_full_path = "";

	string account = this->setting.account;
	log_full_path = log_path;
	log_full_path += "/";
	log_full_path += account;
	log_full_path += "_";

	// parse model name
	string model_name = "";
	unsigned found = this->setting.file.find_last_of("/");
	if(found==string::npos){
		model_name = this->setting.file;
	}else{
		model_name = this->setting.file.substr(found+1);
	}
	log_full_path += model_name;
	log_full_path += "_";

	time_t rawtime;
	struct tm * timeinfo;
	char buffer [80];
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (buffer,80,"%Y%m%d",timeinfo);
	log_full_path += buffer;
	log_full_path += ".txt";

	return log_full_path;
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
bool
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::match(string &str,map<quote_category_options,set<string> > &rgx_strs)
{
	for (map<quote_category_options,set<string>>::iterator it=rgx_strs.begin(); it!=rgx_strs.end(); ++it){
		set<string> &contracts = it->second;
		for (set<string>::iterator it_of_item=contracts.begin(); it_of_item!=contracts.end(); ++it_of_item){
			regex rex = regex((*it_of_item).c_str());
			if (regex_match(str.c_str(),rex)){
				return true;
			}
		}	// end for (std::set<string>::iterator it_of_item=contracts.begin(); it_of_item!=contracts.end(); ++it_of_item){
	}	// end for (map<quote_category_options,set<string>>::iterator it=rgx_strs.begin(); it!=rgx_strs.end(); ++it){

	return false;
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
void model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::init(T_ContractInfoReturn contractRtn){
	try	{
		string model_log = generate_log_name(setting.config_.log_name);
		strcpy(setting.config_.log_name,model_log.c_str());
		setting.config_.log_id = setting.config_.st_id;

		for(int i=0; i<setting.config_.symbols_cnt; i++){
			string sym_log_name = "";
			sym_log_name = generate_log_name(setting.config_.symbols[i].symbol_log_name);
			strcpy(setting.config_.symbols[i].symbol_log_name,sym_log_name.c_str());
		}

		int &target_idx = this->setting.config_.contract_info_count = 0;
		if(0 == contractRtn.error_no){
			for(size_t i=0; i<contractRtn.datas.size(); i++){
				string contract =contractRtn.datas[i].stock_code;
				if(true==this->setting.isOption){
					if(true==this->match(contract, this->setting.subscription)){
						strcpy(this->setting.config_.contracts[target_idx].contract_code,contractRtn.datas[i].stock_code);
						strcpy(this->setting.config_.contracts[target_idx].TradeDate,contractRtn.datas[i].TradeDate);
						strcpy(this->setting.config_.contracts[target_idx].ExpireDate,contractRtn.datas[i].ExpireDate);
						target_idx++;
					}
				}else{
					strcpy(this->setting.config_.contracts[target_idx].contract_code,contractRtn.datas[i].stock_code);
					strcpy(this->setting.config_.contracts[target_idx].TradeDate,contractRtn.datas[i].TradeDate);
					strcpy(this->setting.config_.contracts[target_idx].ExpireDate,contractRtn.datas[i].ExpireDate);
					target_idx++;
				}
				LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
							"init, item:" << this->setting.config_.contracts[i].contract_code);
			}	// end for(int i=0; i<contractRtn.datas.size(); i++){
		}	// end if(0 == contractRtn.error_no){

		int err = 0;
		this->init_imp(&this->setting.config_, &err);
		if (err < 0){
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
					"failed to feed configuration, err no:" << err);
		}
	}
	catch (exception& ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				"failed to feed init info ,because " << ex.what());
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
void model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::trace(string title,symbol_pos_t *pos)
{
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			title << " long_price:" << pos->long_price
			<< " long_volume:" << pos->long_volume
			<< " short_price:" << pos->short_price
			<< " short_volume:" << pos->short_volume
			<< " symbol:" << pos->symbol
			<< "buy_volume:" <<  pos->today_buy_volume<< ","
			<< "today_aver_price_buy:" <<  pos->today_aver_price_buy<< ","
			<< "today_sell_volume:" <<  pos->today_sell_volume<< ","
			<< "today_aver_price_sell:" <<  pos->today_aver_price_sell<< ","
			<< " changed:" << pos->changed
	);

}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_position(position_t *data, int *sig_cnt, signal_t *sig_out)
{
	try	{
		// reserved to use in the future
		*sig_cnt = 0;
		this->feed_position_imp(data, sig_cnt, sig_out);
		for (int cur_idx=0; cur_idx<*sig_cnt; cur_idx++ ){
			sig_out[cur_idx].st_id = this->setting.id;
		}
	}
	catch (exception& ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"feed_position failed,becasue " << ex.what());
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_init_position(strategy_init_pos_t *data,int *sig_cnt, signal_t *sig_out)
{
	*sig_cnt = 0;
	this->feed_init_position_imp(data,sig_cnt, sig_out);
	for (int cur_idx=0; cur_idx<*sig_cnt; cur_idx++ ){
		sig_out[cur_idx].st_id = this->setting.id;
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"feed_init_position do NOT support parameter 'sig_out'. ");
	}
}
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>::
feed_spif_quote(SPIFQuoteT &quote_ptr,signal_t* signals,int &signals_size)
{
	this->trace(quote_ptr);
	this->feed_spif_quote_imp(quote_ptr,signals,signals_size);
	for (int cur_idx=0; cur_idx<signals_size; cur_idx++ ){
		signals[cur_idx].st_id = this->setting.id;
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>::
feed_cf_quote(CFQuoteT &quote_ptr,signal_t* signals,int &signals_size)
{
	// 锟斤拷锟斤拷锟斤拷锟斤拷锟侥ｏ拷锟?
	signals_size = 0;
	this->feed_cf_quote_imp(quote_ptr,signals,signals_size);
	for (int cur_idx=0; cur_idx<signals_size; cur_idx++ ){
		signals[cur_idx].st_id = this->setting.id;
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>::
feed_stock_quote(StockQuoteT &quote_ptr,signal_t* signals,int &signals_size)
{
	// 锟斤拷锟斤拷锟斤拷锟斤拷锟侥ｏ拷锟?
	this->feed_stock_quote_imp(quote_ptr,signals,signals_size);
	for (int cur_idx=0; cur_idx<signals_size; cur_idx++ ){
		signals[cur_idx].st_id = this->setting.id;
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_full_depth_quote(FullDepthQuoteT &quote_ptr,signal_t* signals,int &signals_size)
{
	// 锟斤拷锟斤拷锟斤拷锟斤拷锟侥ｏ拷锟?
	signals_size = 0;
	this->feed_full_depth_quote_imp(quote_ptr,signals,signals_size);
	for (int cur_idx=0; cur_idx<signals_size; cur_idx++ ){
		signals[cur_idx].st_id = this->setting.id;
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_MDOrderStatistic(QuoteT5 &quote_ptr,signal_t* signals,int &signals_size)
{
	// 锟斤拷锟斤拷锟斤拷锟斤拷锟侥ｏ拷锟?
	this->feed_MDOrderStatistic_imp(quote_ptr,signals,signals_size);
	for (int cur_idx=0; cur_idx<signals_size; cur_idx++ ){
		signals[cur_idx].st_id = this->setting.id;
	}
}
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::initialize(void)
{
	InitFAddr = (init_f)(_pproxy->findObject(this->setting.file,this->setting.init_method));
	if (!InitFAddr){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				"invoking init_f unsuccessfully when loading model," << "error code锟斤拷" << errno);
	}

	FeedSPIFQuoteF = (FeedSPIFQuoteFAddr)(_pproxy->findObject(this->setting.file,this->setting.feed_spif_quote_method));
	if (!FeedSPIFQuoteF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"invoking FeedSPIFQuote unsuccessfully," << "error code:" << errno);
	}

	FeedCFQuoteF = (FeedCFQuoteFAddr)(_pproxy->findObject(this->setting.file,this->setting.feed_cf_quote_method));
	if (!FeedCFQuoteF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"invoking FeedCFQuote unsuccessfully," << "error code:" << errno);
	}

	FeedStockQuoteF = (FeedStockQuoteFAddr)(_pproxy->findObject(this->setting.file,this->setting.feed_stock_quote_method));
	if (!FeedStockQuoteF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"invoking FeedStockQuote unsuccessfully," << "error code:" << errno);
	}

	FeedFullDepthQuoteF = (FeedFullDepthQuoteFAddr)(_pproxy->findObject(this->setting.file,this->setting.feed_full_depth_quote_method));
	if (!FeedFullDepthQuoteF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"invoking feedfullquote unsuccessfully" << ",error code:" << errno);
	}

	FeedStatisticF = (FeedMDOrderStatisticFAddr)(_pproxy->findObject(this->setting.file,this->setting.MDOrderStatistic_method));
	if (!FeedStatisticF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"invoking FeedMDOrderStatistic unsuccessfully" << ",error code:" << errno);
	}

	FeedPositionF = (FeedPositionFAddr)(
			_pproxy->findObject(this->setting.file,this->setting.feed_pos_method));
	if (!FeedPositionF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"invoking FeedPositionF unsuccessfully" << ",error code:" << errno);
	}

	FeedInitPositionF = (FeedInitPositionAddr)(
			_pproxy->findObject(this->setting.file,this->setting.feed_init_pos_method));
	if (!FeedInitPositionF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"creating FeedInitPositionF failed" << ",error code:" << errno);
	}

	FeedSignalResponseF = (FeedSignalResponseFAddr)(
		_pproxy->findObject(this->setting.file,this->setting.feed_sig_response_method));
	if (!FeedSignalResponseF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
				"invoking FeedSignalResponseF unsuccessfully" << ",error code:" << errno);
	}

	FeedTimeEventF = (FeedTimeEventFAddr)(
			_pproxy->findObject(this->setting.file,this->setting.feed_time_event_method));
	if (!FeedTimeEventF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
			"invoking FeedTimeEventF unsuccessfully" << ",error code:" << errno);
	}

	DestroyF = (DestroyFAddr)(
			_pproxy->findObject(this->setting.file,this->setting.destroy_method));
	if (!DestroyF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),
			"invoking FeedTimeEventF unsuccessfully" << ",error code:" << errno);
	}

	// market making
	FeedQuoteNotifyF = (FeedQuoteNotifyFAddr)(_pproxy->findObject(this->setting.file,this->setting.feed_quote_notify_method));
	if (!FeedQuoteNotifyF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"invoking FeedQuoteNotifyF unsuccessfully," << "error code:" << errno);
	}

	InitStrategyF = (InitStrategyFAddr)(_pproxy->findObject(this->setting.file,this->setting.init_option_strategy));
	if (!InitStrategyF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"invoking InitStrategyF failed," << "error code:" << errno);
	}

	UpdateStrategyF = (UpdateStrategyFAddr)(_pproxy->findObject(this->setting.file,this->setting.update_option_strategy));
	if (!UpdateStrategyF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"invoking UpdateStrategyF failed," << "error code:" << errno);
	}

	GetMonitorInfoF = (GetMonitorInfoFAddr)(_pproxy->findObject(this->setting.file,this->setting.get_monitor_info));
	if (!GetMonitorInfoF){
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"invoking GetMonitorInfoF failed," << "error code:" << errno);
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),
			"load model " << this->setting.name << "successfully.");
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::finalize(void)
{
	if (this->DestroyF != NULL){
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"destroy" << this->setting.id << "...");
		DestroyF();
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),this->setting.id << " destroyed");
	}

	if (_pproxy != NULL){
		_pproxy->deleteObject(this->setting.file);
		_pproxy->DeleteLoadLibraryProxy();
		_pproxy = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"unload model so successfully:" << this->setting.file);
}


template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::init_imp(st_config_t *config, int *ret_code)
{
	try{
		this->InitFAddr(config, ret_code);
	}catch(exception& ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"init_imp throw exception:" <<  ex.what());
	} // end try{
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_init_position_imp(strategy_init_pos_t *data, int *sig_cnt, signal_t *sig_out)
{
	this->FeedInitPositionF(data, sig_cnt, sig_out);
	trace(data);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_position_imp(position_t *data, int *sig_cnt, signal_t *sig_out)
{
	*sig_cnt = 0;
	this->FeedPositionF(data, sig_cnt, sig_out);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_spif_quote_imp(SPIFQuoteT &quote_ptr,signal_t* signals,int &signals_size)
{
	try	{
		signals_size = 0;

		// maint.                                                                                       
		if(maintenance::enabled()){                                                                                     
			string contract = pending_quote_dao<SPIFQuoteT>::get_symbol(&quote_ptr);
			contract += "(invoking spif)";                                                                          
			maintenance::log(contract);                                                                                 
		}  

		this->FeedSPIFQuoteF(&quote_ptr,&signals_size,signals);

		// maint.                                                                                       
		if(maintenance::enabled()){                                                                                     
			string contract = pending_quote_dao<SPIFQuoteT>::get_symbol(&quote_ptr);
			contract += "(invoked spif)";                                                                          
			maintenance::log(contract);                                                                                 
		}  

		if (signals_size > 0){
			trace("feed_spif_quote_imp:",signals,signals_size);
		}
	}
	catch(exception& ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"FeedQuoteF throw exception:" <<  ex.what());
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_cf_quote_imp(CFQuoteT &quote_ptr,signal_t* signals,int &signals_size)
{
	try	{
			signals_size = 0;

			// maint.                                                                                       
			if(maintenance::enabled()){
				string contract = pending_quote_dao<CFQuoteT>::get_symbol(&quote_ptr); 
				contract += "(invoking cf)";                                                                          
				maintenance::log(contract);
			}  

			this->FeedCFQuoteF(&quote_ptr,&signals_size,signals);

			// maint.                                                                                       
			if(maintenance::enabled()){ 
				string contract = pending_quote_dao<CFQuoteT>::get_symbol(&quote_ptr);
				contract += "(invoked cf)";                                                                          
				maintenance::log(contract);
			}  

			if (signals_size > 0){
				trace("feed_cf_quote_imp:",signals,signals_size);
			}
		}
		catch(exception& ex){
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"FeedQuoteF throw exception:" <<  ex.what());
		}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_stock_quote_imp(StockQuoteT &quote_ptr,signal_t* signals,int &signals_size)
{
	try	{
		signals_size = 0;

		// maint.                                                                                       
		if(maintenance::enabled()){ 
			string contract = pending_quote_dao<StockQuoteT>::get_symbol(&quote_ptr);
			contract += "(invoking stock)";
			maintenance::log(contract);
		}  

		this->FeedStockQuoteF(&quote_ptr,&signals_size,signals);

		// maint.                                                                                       
		if(maintenance::enabled()){ 
			string contract = pending_quote_dao<StockQuoteT>::get_symbol(&quote_ptr);
			contract += "(invoked stock)";
			maintenance::log(contract);
		}  

	}
	catch(exception& ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"FeedQuoteF throw exception:" <<  ex.what());
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_full_depth_quote_imp(FullDepthQuoteT &quote_ptr,signal_t* signals,int &signals_size)
{
	try{
		signals_size = 0;

		// maint.                                                                                       
		if(maintenance::enabled()){ 
			string contract = pending_quote_dao<FullDepthQuoteT>::get_symbol(&quote_ptr);
			contract += "(invoking full depth)";
			maintenance::log(contract);
		}  	

		this->FeedFullDepthQuoteF(&quote_ptr,&signals_size,signals);
		if (signals_size > 0){
			trace("feed_full_depth_quote_imp:",signals,signals_size);
		}

		// maint.                                                                                       
		if(maintenance::enabled()){ 
			string contract = pending_quote_dao<FullDepthQuoteT>::get_symbol(&quote_ptr);
			contract += "(invoked full depth)";
			maintenance::log(contract);
		}  
	}
	catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"FeedQuoteF error" <<  ex.what());
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_MDOrderStatistic_imp(QuoteT5 &quote_ptr,signal_t* signals,int &signals_size)
{
	try{
		signals_size = 0;

		// maint.                                                                                       
		if(maintenance::enabled()){ 
			string contract = pending_quote_dao<QuoteT5>::get_symbol(&quote_ptr);
			contract += "(invoking full mdorderstatistic)";
			maintenance::log(contract);
		}  

		this->FeedStatisticF(&quote_ptr,&signals_size,signals);
		if (signals_size > 0){
			trace("feed_MDOrderStatistic_imp:",signals,signals_size);
		}

		// maint.                                                                                       
		if(maintenance::enabled()){ 
			string contract = pending_quote_dao<QuoteT5>::get_symbol(&quote_ptr);
			contract += "(invoked mdorderstatistic)";
			maintenance::log(contract);
		}  


	}
	catch(exception &ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"feed_MDOrderStatistic_imp error" <<  ex.what());
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>::
trace(string title,signal_t* signals,const int &signals_size)
{
	int count = 0;
	int len = signals_size;
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"count:" << len);
	for (; count<len; count++ ){
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				title
				<< "exchange:" <<  signals[count].exchange << ","
				<< "orig_sig_id:" <<  signals[count].orig_sig_id << ","
				<<"symbol:"<<  signals[count].symbol << ","
				<< "sig_act:"<<  signals[count].sig_act << ","
				<<"sig_id:" <<  signals[count].sig_id << ","
				<< "st_id:" <<  signals[count].st_id << ","
				<< "buy price:" <<  signals[count].buy_price << ","
				<< "open volume:" <<  signals[count].open_volume << ","
				<< "sell price:" <<  signals[count].sell_price << ","
				<< "close volume:" <<  signals[count].close_volume << ","
				<< "cancel_pattern:" <<  signals[count].cancel_pattern << ","
				<< "sig_openclose:" <<  signals[count].sig_openclose << ","
				<< "instr:" <<  signals[count].instr << ","
				);
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>::
trace(signal_resp_t* rpt)
{
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			"feed rpt:aver_price," <<  rpt->aver_price << ","
			<< "error_no," <<  rpt->error_no << ","
			<<"exec_price,"<<  rpt->exec_price << ","
			<< "exec_volume," <<  rpt->exec_volume << ","
			<< "sig_act,"<<  rpt->sig_act << ","
			<<"sig_id," <<  rpt->sig_id << ","
			<< "status," <<  rpt->status << ","
			<< "killed," <<  rpt->killed << ","
			<< "symbol," <<  rpt->symbol<< ","
			<< "acc_volume," <<  rpt->acc_volume<< ","
			<< "order_volume," <<  rpt->order_volume<< ","
			);

}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::trace(strategy_init_pos_t *data)
{
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
		 "init pos,previous days position, "
		<< "symbol_cnt:" <<  data->_yesterday_pos.symbol_cnt<< ","
		);
	for(int i=0; i<data->_yesterday_pos.symbol_cnt; i++){
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				"long_volume:" <<  data->_yesterday_pos.s_pos[i].long_volume<< ","
				<< "long_price:" <<  data->_yesterday_pos.s_pos[i].long_price<< ","
				<< "short_volume:" <<  data->_yesterday_pos.s_pos[i].short_volume<< ","
				<< "short_price:" <<  data->_yesterday_pos.s_pos[i].short_price<< ","
				<< "symbol:" <<  data->_yesterday_pos.s_pos[i].symbol<< ","
				<< "buy_volume:" <<  data->_yesterday_pos.s_pos[i].today_buy_volume<< ","
				<< "exchg_code:" <<  data->_yesterday_pos.s_pos[i].exchg_code << ","
				<< "today_aver_price_sell:" <<  data->_yesterday_pos.s_pos[i].today_aver_price_sell<< ","
				<< "sell_price:" <<  data->_yesterday_pos.s_pos[i].today_sell_volume<< ","
				<< "sell_volume:" <<  data->_yesterday_pos.s_pos[i].today_aver_price_sell<< ","
				<< "symbol:" <<  data->_yesterday_pos.s_pos[i].symbol<< ","
				);
	}

	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
		 "init pos,today position, "
		<< "symbol_cnt:" <<  data->_today_pos.symbol_cnt<< ","
		);
	for(int i=0; i<data->_today_pos.symbol_cnt; i++){
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				"long_volume:" <<  data->_today_pos.s_pos[i].long_volume<< ","
				<< "long_price:" <<  data->_today_pos.s_pos[i].long_price<< ","
				<< "short_volume:" <<  data->_today_pos.s_pos[i].short_volume<< ","
				<< "short_price:" <<  data->_today_pos.s_pos[i].short_price<< ","
				<< "symbol:" <<  data->_today_pos.s_pos[i].symbol<< ","
				<< "exchg_code:" <<  data->_today_pos.s_pos[i].exchg_code<< ","
				<< "buy_volume:" <<  data->_today_pos.s_pos[i].today_buy_volume<< ","
				<< "today_aver_price_buy:" <<  data->_today_pos.s_pos[i].today_aver_price_buy<< ","
				<< "today_sell_volume:" <<  data->_today_pos.s_pos[i].today_sell_volume<< ","
				<< "today_aver_price_sell:" <<  data->_today_pos.s_pos[i].today_aver_price_sell<< ","
				<< "symbol:" <<  data->_today_pos.s_pos[i].symbol<< ","
				);
	}

	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			 "init pos,"
			<< "name:" <<  data->_name<< ","
			<< "acc_cnt:" <<  data->acc_cnt<< ","
			);

	for(int i=0; i<data->acc_cnt; i++){
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				"buy_price:" <<  data->_today_acc_volume[i].buy_price<< ","
				<< "buy_volume:" <<  data->_today_acc_volume[i].buy_volume<< ","
				<< "sell_price:" <<  data->_today_acc_volume[i].sell_price<< ","
				<< "sell_volume:" <<  data->_today_acc_volume[i].sell_volume<< ","
				<< "symbol:" <<  data->_today_acc_volume[i].symbol<< ","
				<< "exchg_code:" <<  data->_today_acc_volume[i].exchg_code<< ","
				);
	}

	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
		 "today_pos.symbol_cnt," << "name:" <<  data->_today_pos.symbol_cnt<< ","
		);
	for(int i=0; i<data->_today_pos.symbol_cnt; i++){
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				"long_price:" <<  data->_today_pos.s_pos[i].long_price << ","
				"long_volume:" <<  data->_today_pos.s_pos[i].long_volume << ","
				"short_price:" <<  data->_today_pos.s_pos[i].short_price << ","
				"short_volume:" <<  data->_today_pos.s_pos[i].short_volume << ","
				"symbol:" <<  data->_today_pos.s_pos[i].symbol << ","
				"exchg_code:" <<  data->_today_pos.s_pos[i].exchg_code << ","
				);
		}

	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			 "_yesterday_pos.symbol_cnt," << "name:" <<  data->_today_pos.symbol_cnt<< ","
			);
	for(int i=0; i<data->_yesterday_pos.symbol_cnt; i++){
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
				"long_price:" <<  data->_yesterday_pos.s_pos[i].long_price << ","
				"long_volume:" <<  data->_yesterday_pos.s_pos[i].long_volume << ","
				"short_price:" <<  data->_yesterday_pos.s_pos[i].short_price << ","
				"short_volume:" <<  data->_yesterday_pos.s_pos[i].short_volume << ","
				"symbol:" <<  data->_yesterday_pos.s_pos[i].symbol << ","
				"exchg_code:" <<  data->_yesterday_pos.s_pos[i].exchg_code << ","
				);
		}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>::
trace(const SPIFQuoteT &data)
{
//	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
//			 "exchange:" <<  data.InstrumentID << ","
//			<< "BidPrice1:" <<  data.BidPrice1 << ","
//			<< "BidVolume1:" <<  data.BidVolume1 << ","
//			<< "AskPrice1:" <<  data.AskPrice1 << ","
//			<< "AskVolume1:" <<  data.AskVolume1 << ","
//			);

}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>::
trace(const T_RtnForQuote &data)
{
	LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			 "feed_quote_notify,for_quote_id:" <<  data.for_quote_id << ","
			<< "stock_code:" <<  data.stock_code << ","
			);

}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::trace(pending_order_t *pending_ord)
{
	int count = 0;
	int pos_size = pending_ord->req_cnt;
	for (; count<pos_size; count++ ){
		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			"pending orders"<<"," <<  pending_ord->pending_req[count].symbol << ","
			<<  this->setting.id << ","
			<<  pending_ord->pending_req[count].direct << ","
			<<  pending_ord->pending_req[count].price << ","
			<<  pending_ord->pending_req[count].volume << ",");
	}
}
template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_time_event(signal_t* sigs,int &sigs_len)
{
	sigs_len = 0;

	this->feed_time_event_imp(sigs,sigs_len);
	for (int cur_idx=0; cur_idx<sigs_len; cur_idx++ ){
		sigs[cur_idx].st_id = this->setting.id;
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_sig_response(signal_resp_t* rpt,symbol_pos_t *pos,
		pending_order_t *pending_ord,signal_t* sigs,int &sigs_len)
{
	trace(rpt);
	trace(pending_ord);
	trace("position:",pos);

	sigs_len = 0;
	this->feed_sig_response_imp(rpt,pos,pending_ord,sigs,sigs_len);
	for (int cur_idx=0; cur_idx<sigs_len; cur_idx++ ){
		sigs[cur_idx].st_id = this->setting.id;
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_sig_response_imp(signal_resp_t* rpt,symbol_pos_t *pos,
		pending_order_t *pending_ord,signal_t* sigs,int &sigs_len)
{
	try	{
		signal_resp_t tmp = *rpt;
		sigs_len = 0;

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"begin feed_sig_response_imp:" <<  this->setting.id);

		this->FeedSignalResponseF(&tmp,pos,pending_ord,&sigs_len,sigs);

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"end feed_sig_response_imp:" <<  this->setting.id);

		if (sigs_len > 0){
			trace("feed_sig_response_imp:",sigs,sigs_len);
		}
	}
	catch(exception& ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"feed_sig_response_imp throw exception:" <<  ex.what());
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_quote_notify(T_RtnForQuote *quote_notify,signal_t* sigs,int &sigs_len)
{
	trace(*quote_notify);
	sigs_len = 0;
	this->feed_quote_notify_imp(quote_notify,sigs,sigs_len);
	for (int cur_idx=0; cur_idx<sigs_len; cur_idx++ ){
		sigs[cur_idx].st_id = this->setting.id;
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_quote_notify_imp(T_RtnForQuote *quote_notify,signal_t* sigs,int &sigs_len)
{
	try	{
		sigs_len = 0;

		this->FeedQuoteNotifyF(quote_notify,&sigs_len,sigs);
		if (sigs_len > 0){
			trace("feed_quote_notify_imp:",sigs,sigs_len);
		}
	}
	catch(exception& ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"feed_quote_notify_imp throw exception:" <<  ex.what());
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
int
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::InitOptionStrategy(startup_init_t *init_content, int *err)
{
	return this->InitStrategyF(init_content,err);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
int
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::UpdateOptionStrategy(ctrl_t *update_content, int *err)
{
	return this->UpdateStrategyF(update_content,err);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
typename FullDepthQuoteT,typename QuoteT5>
int
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::GetMonitorInfo(monitor_t *monitor_content, int *err)
{
	return this->GetMonitorInfoF(monitor_content,err);
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void
model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>
::feed_time_event_imp(
		signal_t* sigs,int &sigs_len)
{
	try	{
		sigs_len = 0;

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"begin feed_time_event_imp:" <<  this->setting.id)

		this->FeedTimeEventF(&sigs_len,sigs);

		LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),"end feed_time_event_imp:" <<  this->setting.id);

		if (sigs_len > 0){
			trace("feed_time_event_imp:",sigs,sigs_len);
		}
	}
	catch(exception& ex){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"feed_time_event_imp throw exception:" <<  ex.what());
	}
}

