/*
 * quote_setting.h
 *
 *  Created on: Apr 24, 2014
 *      Author: root
 */

#pragma once

#include <vector>
#include <string>
#include <map>
#include "forwardersetting.h"
#include <set>
#include "../my_exchange.h"

#ifdef rss
#endif

using namespace std;


class quote_setting{
private:
	/*
	 * store quote forwarder configuration file path.
	 */
	string _config;

	void read_config();

public:
	quote_setting(string config);
	~quote_setting();

	map<string,forwarder_setting> forwarders;

	/*
	 * symbol list to subscribe to.
	 * if asterisk '*' is given, it represents all contracts
	 * will be subscribed.
	 */
	set<string> Subscription;

	string MarketdataConfig;

};

