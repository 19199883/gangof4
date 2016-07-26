#pragma once



#define DEBUG
// #define GTA_QUOTE
#define LINUX
// #define rss
#define SHFE

//#define FUTURE

#ifdef DCE
	#define COMMODITY_DBestAndDeep_MY_YES
	#define COMMODITY_CDepthMarketDataField_NO
	#define COMMODITY_MYShfeMarketData_NO
	#define STOCK_INDEX_FUTURE_NO
	#define STOCK_NO
	#define FULL_DEPTH_NO
	#define QUOTE5_MDOrderStatistic_YES
#endif

#ifdef CZCE
	#define COMMODITY_DBestAndDeep_MY_NO
	#define COMMODITY_CDepthMarketDataField_YES
	#define COMMODITY_MYShfeMarketData_NO
	#define STOCK_INDEX_FUTURE_NO
	#define STOCK_NO
	#define FULL_DEPTH_NO
	#define QUOTE5_MDOrderStatistic_NO
#endif

#ifdef SHFE
	#define COMMODITY_DBestAndDeep_MY_NO
	#define COMMODITY_CDepthMarketDataField_NO
	#define COMMODITY_MYShfeMarketData_YES
	#define STOCK_INDEX_FUTURE_NO
	#define STOCK_NO
	#define FULL_DEPTH_NO
	#define QUOTE5_MDOrderStatistic_NO



#endif

#ifdef CFFEX
	#define COMMODITY_DBestAndDeep_MY_NO
	#define COMMODITY_CDepthMarketDataField_NO
	#define COMMODITY_MYShfeMarketData_NO
	#define STOCK_INDEX_FUTURE_YES
	#define STOCK_NO
	#define FULL_DEPTH_NO
	#define QUOTE5_MDOrderStatistic_NO
#endif
