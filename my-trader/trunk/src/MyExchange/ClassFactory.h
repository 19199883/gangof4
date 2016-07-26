#pragma once

#include "my_quote_lib.h"
#include <string>
#include "engine.h"
#include "strategy_unit.h"
#include "quote_entity.h"
#include "my_quote_lib.h"

// A50
//typedef strategy_unit<TapAPIQuoteWhole_MY,my_datasource_data,TDF_MARKET_DATA_MY,T_PerBargain,T_OrderQueue> StrategyT;

// SHFE MYShfeMarketData
//typedef strategy_unit<CFfexFtdcDepthMarketData,MYShfeMarketData,MDTenEntrust_MY,SHFEQuote,MDOrderStatistic_MY> StrategyT;

// CFFE CFfexFtdcDepthMarketData
// typedef strategy_unit<CFfexFtdcDepthMarketData,MDBestAndDeep_MY,MDTenEntrust_MY,SHFEQuote,MDOrderStatistic_MY> StrategyT;

// stock option
//typedef strategy_unit<T_OptionQuote,CDepthMarketDataField,TDF_MARKET_DATA_MY,IBDepth,MDOrderStatistic_MY> StrategyT;

// typedef strategy_unit<T_OptionQuote,CDepthMarketDataField,TDF_MARKET_DATA_MY,IBDepth,MDOrderStatistic_MY> StrategyT;

// STOCK
//typedef strategy_unit<CDepthMarketDataField,T_PerEntrust,TDF_MARKET_DATA_MY,T_PerBargain,TDF_INDEX_DATA_MY> StrategyT;

// a50 IB
//typedef strategy_unit<IBDepth,inner_quote_fmt,IBTick,T_PerBargain,T_OrderQueue> StrategyT;

// MDBestAndDeep_MY DCE
//typedef strategy_unit<CFfexFtdcDepthMarketData,MDBestAndDeep_MY,MDTenEntrust_MY,SHFEQuote,MDOrderStatistic_MY> StrategyT;

// DCE market making
//typedef strategy_unit<CDepthMarketDataField,CDepthMarketDataField,MDTenEntrust_MY,SHFEQuote,CDepthMarketDataField> StrategyT;

// CZCE
//typedef strategy_unit<my_marketdata,ZCEL2QuotSnapshotField_MY,MDTenEntrust_MY,SHFEQuote,ZCEQuotCMBQuotField_MY> StrategyT;


// CZCE level1
//typedef strategy_unit<CFfexFtdcDepthMarketData,CDepthMarketDataField,MDTenEntrust_MY,SHFEQuote,MDOrderStatistic_MY> StrategyT;

// stock KGI FIX
//typedef strategy_unit<CDepthMarketDataField,T_PerEntrust,TDF_MARKET_DATA_MY,T_PerBargain,my_datasource_data> StrategyT;

// SHEX level1
//typedef strategy_unit<CDepthMarketDataField,CFfexFtdcDepthMarketData,MDTenEntrust_MY,SHFEQuote,MDOrderStatistic_MY> StrategyT;

// level1, deep of SHFE
// typedef strategy_unit<CDepthMarketDataField,MYShfeMarketData,MDTenEntrust_MY,SHFEQuote,MDOrderStatistic_MY> StrategyT;
//
// data: cffex level2;
//typedef strategy_unit<CFfexFtdcDepthMarketData,MDBestAndDeep_MY,MDTenEntrust_MY,SHFEQuote,MDOrderStatistic_MY> StrategyT;

// Shanghai Gold Exchange
typedef strategy_unit<CDepthMarketDataField,SGIT_Depth_Market_Data_Field,TDF_MARKET_DATA_MY,IBDepth,MDOrderStatistic_MY> StrategyT;






