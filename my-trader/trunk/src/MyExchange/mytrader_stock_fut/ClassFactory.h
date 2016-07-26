#pragma once

#include <string>
#include "engine.h"
#include "strategy_unit.h"
#include "quote_entity.h"

typedef strategy_unit<T_OptionQuote,CDepthMarketDataField,TDF_MARKET_DATA_MY,IBDepth,MDOrderStatistic_MY> StrategyT;
