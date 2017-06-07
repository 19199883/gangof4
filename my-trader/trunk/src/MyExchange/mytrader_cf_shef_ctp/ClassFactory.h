#pragma once

#include <string>
#include "engine.h"
#include "strategy_unit.h"
#include "quote_entity.h"

typedef strategy_unit<CDepthMarketDataField,MYShfeMarketData,int,int,int> StrategyT;
