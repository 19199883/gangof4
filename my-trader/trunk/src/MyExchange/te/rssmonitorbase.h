/*
 * rssmonitorbase.h
 *
 *  Created on: Aug 28, 2015
 *      Author: root
 */

#ifndef RSSMONITORBASE_H_
#define RSSMONITORBASE_H_

#include "strategy_unit.h"
#include <memory>
#include "quote_entity.h"
#include "../my_exchange.h"
#include "ClassFactory.h"

class rss_monitor_base {
public:

	  virtual void start(std::shared_ptr<StrategyT> strategy) = 0;

	  virtual void stop() = 0;

	  virtual ~rss_monitor_base()
	  {

	  }
};

#endif /* RSSMONITORBASE_H_ */
