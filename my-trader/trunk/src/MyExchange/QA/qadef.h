#include <boost/foreach.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include "pending_quote_dao.h"
#include <functional>
#include "quote_entity.h"
#include "quotecategoryoptions.h"
#include "typeutils.h"

using namespace quote_agent;

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
qa<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>::qa(void)
: setting()
{
	// 初始化qa的配置信息
	this->setting.Initialize();
	
	// TODO:improve 2
	md_provider_ = build_quote_provider(subs_);

	// 加载quote_source
	BOOST_FOREACH( quote_source_setting source_setting, this->setting.quote_sources ){
		if (source_setting.category==quote_category_options::SPIF &&
			IsIntegerT<SPIFQuoteT>::No){
			quote_source<SPIFQuoteT> *source = new quote_source<SPIFQuoteT>(source_setting,md_provider_ );
			spif_quote_source_ptr = shared_ptr<quote_source<SPIFQuoteT>>(source);
		}
		else if (source_setting.category==quote_category_options::CF&&
				IsIntegerT<CFQuoteT>::No){
			quote_source<CFQuoteT> *source = new quote_source<CFQuoteT>(source_setting,md_provider_ );
			cf_quote_source_ptr = shared_ptr<quote_source<CFQuoteT>>(source);
		}
		else if (source_setting.category==quote_category_options::Stock&&
				IsIntegerT<StockQuoteT>::No){
			quote_source<StockQuoteT> *source = new quote_source<StockQuoteT>(source_setting,md_provider_ );
			stock_quote_source_ptr = shared_ptr<quote_source<StockQuoteT>>(source);
		}
		else if (source_setting.category==quote_category_options::FullDepth&&
				IsIntegerT<FullDepthQuoteT>::No){
			quote_source<FullDepthQuoteT> *source = new quote_source<FullDepthQuoteT>(source_setting,md_provider_ );
			full_depth_quote_source_ptr = shared_ptr<quote_source<FullDepthQuoteT>>(source);
		}
		else if (source_setting.category==quote_category_options::MDOrderStatistic&&
				IsIntegerT<QuoteT5>::No){
			quote_source<QuoteT5> *source = new quote_source<QuoteT5>(source_setting,md_provider_ );
			quote_source5_ptr = shared_ptr<quote_source<QuoteT5>>(source);
		}
	}
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
qa<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>::~qa(void)
{
	finalize();
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void qa<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>:: Initialize(void)
{
	stopped = false;
	// 加载quote_source
	if (spif_quote_source_ptr!=0) 		spif_quote_source_ptr->intialize();
	if (cf_quote_source_ptr) 			cf_quote_source_ptr->intialize();
	if (stock_quote_source_ptr!= 0) 	stock_quote_source_ptr->intialize();
	if (full_depth_quote_source_ptr!=0) full_depth_quote_source_ptr->intialize();
	if (quote_source5_ptr!=0) 			quote_source5_ptr->intialize();
}

template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
void qa<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5>:: finalize(void)
{	
	stopped = true;	
	std::this_thread::yield();
	// 终止配置对象
	this->setting.finalize();

	// 终止quote_source
	if (spif_quote_source_ptr != 0) 		spif_quote_source_ptr->finalize();
	if (cf_quote_source_ptr != 0) 			cf_quote_source_ptr->finalize();
	if (stock_quote_source_ptr != 0) 		stock_quote_source_ptr->finalize();
	if (full_depth_quote_source_ptr != 0) 	full_depth_quote_source_ptr->finalize();
	if (quote_source5_ptr != 0) 			quote_source5_ptr->finalize();

	if (NULL!=md_provider_){
		delete md_provider_;
		md_provider_ = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete qa successfully.");
}


