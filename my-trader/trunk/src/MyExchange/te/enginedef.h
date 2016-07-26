#include <sstream>
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/logmanager.h>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include "model_adapter.h"
#include "pending_quote_dao.h"
#include "quote_entity.h"
#include "exchange_names.h"
#include <map>
#include <tinyxml.h>
#include <tinystr.h>

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

using namespace trading_engine;

template<typename SPIFQuoteT,typename CFQuotet,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
bool engine<SPIFQuoteT,CFQuotet,StockQuoteT,FullDepthQuoteT,QuoteT5>::stopped = false;

template<typename SPIFQuoteT,typename CFQuotet,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
int engine<SPIFQuoteT,CFQuotet,StockQuoteT,FullDepthQuoteT,QuoteT5>::timeEventInterval_ = 0;

template<typename SPIFQuoteT,typename CFQuotet,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
engine<SPIFQuoteT,CFQuotet,StockQuoteT,FullDepthQuoteT,QuoteT5>::engine(void)
	:model_manager_ptr(new ModelManagerT()),
	 qa_ptr(new QaT()),
	 tca_ptr(new tca()),
	 listen_(0)
{
	disposed = false;
	this->_pproxy = NULL;

	//创建一个XML的文档对象。
	TiXmlDocument config = TiXmlDocument(sm_settings::config_path.c_str());
    config.LoadFile();
    //获得根元素，即MyExchange
    TiXmlElement *RootElement = config.RootElement();
    //获得第一个strategies节点
//    TiXmlElement *strategies = RootElement->FirstChildElement("strategies");
    RootElement->QueryIntAttribute("timeEventInterval",&engine::timeEventInterval_);
}

template<typename SPIFQuoteT,typename CFQuotet,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
engine<SPIFQuoteT,CFQuotet,StockQuoteT,FullDepthQuoteT,QuoteT5>::~engine(void)
{
	this->finalize();
}

template<typename SPIFQuoteT,typename CFQuotet,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void engine<SPIFQuoteT,CFQuotet,StockQuoteT,FullDepthQuoteT,QuoteT5>::finalize(void)
{
	if (false == disposed){

		io_service_.stop();
		monitor_proxy_->stop();
		if(NULL != monitor_proxy_){
			delete monitor_proxy_;
			monitor_proxy_ = NULL;
		}

		this->qa_ptr->finalize();
		this->tca_ptr->finalize();

		LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete model_manager_ptr...");

		this->model_manager_ptr->finalize();
		_pproxy->DeleteLoadLibraryProxy();
		engine::stopped = true;

		this_thread::sleep_for(std::chrono::milliseconds(500));

		disposed = true;
	}
}

template<typename SPIFQuoteT,typename CFQuotet,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void engine<SPIFQuoteT,CFQuotet,StockQuoteT,FullDepthQuoteT,QuoteT5>::initialize(void)
{
	engine::stopped = false;
	TiXmlDocument config = TiXmlDocument(sm_settings::config_path.c_str());
	config.LoadFile();
	TiXmlElement *RootElement = config.RootElement();
	RootElement->QueryIntAttribute("listen",&this->listen_);

	// 初始化te依赖的各个模块
	_pproxy = CLoadLibraryProxy::CreateLoadLibraryProxy();
	_pproxy->setModuleLoadLibrary(new CModuleLoadLibraryLinux());
	_pproxy->setBasePathLibrary(_pproxy->getexedir());
	this->model_manager_ptr->initialize();			

	// 到qa模块订阅行情
	for (typename ModelAdapterListT::iterator it=model_manager_ptr->models.begin();it!=model_manager_ptr->models.end();++it)
	{  
		this->subscribe(*it);
	}
	this->tca_ptr->initialize();			
}

template<typename SPIFQuoteT,typename CFQuotet,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void engine<SPIFQuoteT,CFQuotet,StockQuoteT,FullDepthQuoteT,QuoteT5>::
subscribe(shared_ptr<ModelAdapterT> model_ptr)
{
	map<quote_category_options,SubscribeContracts> subscription = model_ptr->setting.subscription;
	if (qa_ptr->spif_quote_source_ptr != NULL){
		qa_ptr->spif_quote_source_ptr->
				subscribe_to_quote(model_ptr->setting.id, 
				model_ptr->setting.isOption,
				subscription[quote_category_options::SPIF]);
	}
	if (qa_ptr->cf_quote_source_ptr != NULL){
		qa_ptr->cf_quote_source_ptr->
				subscribe_to_quote(model_ptr->setting.id, 
				model_ptr->setting.isOption, 
				subscription[quote_category_options::CF]);
	}
	if (qa_ptr->stock_quote_source_ptr != NULL){
		qa_ptr->stock_quote_source_ptr->
				subscribe_to_quote(model_ptr->setting.id, 
				model_ptr->setting.isOption, 
				subscription[quote_category_options::Stock]);
	}
	if (qa_ptr->full_depth_quote_source_ptr != NULL){
		qa_ptr->full_depth_quote_source_ptr->
			subscribe_to_quote(model_ptr->setting.id, 
			model_ptr->setting.isOption, 
			subscription[quote_category_options::FullDepth]);
	}

	if (qa_ptr->quote_source5_ptr != NULL){
		qa_ptr->quote_source5_ptr->
			subscribe_to_quote(model_ptr->setting.id, 
			model_ptr->setting.isOption,
			subscription[quote_category_options::MDOrderStatistic]);
	}
}

template<typename SPIFQuoteT,typename CFQuotet,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void engine<SPIFQuoteT,CFQuotet,StockQuoteT,FullDepthQuoteT,QuoteT5>::run(void)
{
	stringstream ss_log;
	
	list<shared_ptr<StrategyUnitT>> unit_tmp;

	// 到qa模块订阅行情
	BOOST_FOREACH(shared_ptr<ModelAdapterT> model_ptr, model_manager_ptr->models){
		// T29. skip if invalid strategy
		if(!model_ptr->setting.is_valid()){
			continue;
		}

		StrategyUnitT  * unit = new StrategyUnitT(tca_ptr, qa_ptr, model_ptr,engine::timeEventInterval_);
		shared_ptr<StrategyUnitT> unit_ptr(unit);
		_unit_ptrs.insert(pair<long,shared_ptr<StrategyUnitT> >(model_ptr->setting.id, unit_ptr));
		unit_tmp.push_back(unit_ptr);
		unit_ptr->run();
		this_thread::sleep_for(std::chrono::milliseconds(10));
        
        if ((model_ptr->setting.isOption) && (this->listen_ > 0)){
    		tcp::endpoint endpoint(tcp::v4(), this->listen_);
    		monitor_proxy_ = new monitor_proxy(io_service_,endpoint);
    		monitor_proxy_->start(unit_ptr);
    		thread t = thread(boost::bind(&boost::asio::io_service::run,boost::ref(io_service_)));
    		t.detach();
        }
	}	
	this->qa_ptr->Initialize();

	while(!stopped)
	{
		this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"engine exited.");

	BOOST_FOREACH(shared_ptr<StrategyUnitT> unit_ptr, unit_tmp)
	{
		unit_ptr->stop();
	}
}



