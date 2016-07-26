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
	// 加载quote_source
	BOOST_FOREACH( quote_source_setting source_setting, this->setting.quote_sources ){
		if (source_setting.category==quote_category_options::SPIF &&
			IsIntegerT<SPIFQuoteT>::No){
			quote_source<SPIFQuoteT> *source = new quote_source<SPIFQuoteT>(source_setting);
			spif_quote_source_ptr = shared_ptr<quote_source<SPIFQuoteT>>(source);
#ifdef rss
	#ifdef STOCK_INDEX_FUTURE_YES
			quote_playback::gta_udp_cffex_handler = bind(&quote_source<SPIFQuoteT>::OnGTAQuoteData,spif_quote_source_ptr,_1);
	#endif
#endif
		}
		else if (source_setting.category==quote_category_options::CF&&
				IsIntegerT<CFQuoteT>::No){
			quote_source<CFQuoteT> *source = new quote_source<CFQuoteT>(source_setting);
			cf_quote_source_ptr = shared_ptr<quote_source<CFQuoteT>>(source);
#ifdef rss
	#ifdef COMMODITY_DBestAndDeep_MY_YES
			quote_playback::best_and_deep_handler = bind(&quote_source<CFQuoteT>::OnGTAQuoteData,cf_quote_source_ptr,_1);
	#endif
	#ifdef COMMODITY_CDepthMarketDataField_YES
			quote_playback::shfe_ex_handler = bind(&quote_source<CFQuoteT>::OnGTAQuoteData,cf_quote_source_ptr,_1);
	#endif
	#ifdef COMMODITY_MYShfeMarketData_YES
			quote_playback::my_shfe_handler = bind(&quote_source<CFQuoteT>::OnGTAQuoteData,cf_quote_source_ptr,_1);
	#endif

#endif
		}
		else if (source_setting.category==quote_category_options::Stock&&
				IsIntegerT<StockQuoteT>::No){
			quote_source<StockQuoteT> *source = new quote_source<StockQuoteT>(source_setting);
			stock_quote_source_ptr = shared_ptr<quote_source<StockQuoteT>>(source);
		}
		else if (source_setting.category==quote_category_options::FullDepth&&
				IsIntegerT<FullDepthQuoteT>::No){
			quote_source<FullDepthQuoteT> *source = new quote_source<FullDepthQuoteT>(source_setting);
			full_depth_quote_source_ptr = shared_ptr<quote_source<FullDepthQuoteT>>(source);
		}
		else if (source_setting.category==quote_category_options::MDOrderStatistic&&
				IsIntegerT<QuoteT5>::No){
			quote_source<QuoteT5> *source = new quote_source<QuoteT5>(source_setting);
			quote_source5_ptr = shared_ptr<quote_source<QuoteT5>>(source);
#ifdef rss
	#ifdef QUOTE5_MDOrderStatistic_YES
			quote_playback::order_statistic_handler = bind(&quote_source<QuoteT5>::OnGTAQuoteData,quote_source5_ptr,_1);
	#endif
#endif
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

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete qa successfully.");
}


