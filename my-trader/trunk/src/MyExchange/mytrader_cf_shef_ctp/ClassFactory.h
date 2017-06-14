#pragma once

#include <string>
#include "quote_interface_shfe_my.h"
#include "engine.h"
#include "strategy_unit.h"
#include "quote_entity.h"

typedef strategy_unit<CDepthMarketDataField,MYShfeMarketData,int,int,int> StrategyT;
