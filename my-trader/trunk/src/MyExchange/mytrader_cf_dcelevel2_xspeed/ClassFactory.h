#pragma once

#include <string>
#include "engine.h"
#include "strategy_unit.h"
#include "quote_entity.h"

typedef strategy_unit<CFfexFtdcDepthMarketData,MDBestAndDeep_MY,MDTenEntrust_MY,CDepthMarketDataField,MDOrderStatistic_MY> StrategyT;
