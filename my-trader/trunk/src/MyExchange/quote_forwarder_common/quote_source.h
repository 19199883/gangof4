#ifndef general_depth_quote_source_H_
#define general_depth_quote_source_H_

#include <list>
#include <cstdlib>
#include <list>
#include <boost/shared_ptr.hpp>
#include <mutex>          // std::mutex
#include <condition_variable> // std::condition_variable
#include <memory>
#include <thread>
#include <log4cxx/logger.h>
#include "forwarder.h"
#include "typeutils.h"
#include "../my_exchange.h"
#include <log4cxx/logger.h>
#include <set>
//#include "quote_interface_datasource.h"

using namespace std::chrono;
using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

enum quote_src_types{
	dce 		= 1,
	ctp			= 2,
	gta_tcp 	= 3,
	gta_udp 	= 4,
	shfe_deep 	= 5,
	shfe_ex 	= 6,
	tdf 		= 7,
	mydata 		= 8,
	// provides all types of market data for shanghai future exchange
	shfe		= 9,
	czce		= 10,
	mymarketdata		= 11,
	tap		= 12,
	ib		= 13,
	sgit	= 30,
};

template<quote_src_types quote_src,
		 typename QuoteT1 = int,
		 typename QuoteT2 = int,
		 typename QuoteT3 = int,
		 typename QuoteT4 = int,
		 typename QuoteT5 = int,
		 typename QuoteT6 = int>
class quote_source {
public:

public:
	quote_source(
			string mktdata_config,
			set<string> subscription,
			forwarder<QuoteT1> *forwarderInc1,
			forwarder<QuoteT2> *forwarderInc2,
			forwarder<QuoteT3> *forwarderInc3,
			forwarder<QuoteT4> *forwarderInc4,
			forwarder<QuoteT5> *forwarderInc5,
			forwarder<QuoteT6> *forwarderInc6);
	~quote_source(void);
	void start(void);
private:
	forwarder<QuoteT1> *_forwarderInc1;
	forwarder<QuoteT2> *_forwarderInc2;
	forwarder<QuoteT3> *_forwarderInc3;
	forwarder<QuoteT4> *_forwarderInc4;
	forwarder<QuoteT5> *_forwarderInc5;
	forwarder<QuoteT6> *_forwarderInc6;

	bool _stopped;
	MYQuoteData *_quote_src;
	SubscribeContracts _subscription;
	string mktdata_config_;

public:
	void subscribe_to_symbols();

	void OnGTAQuoteData1(const QuoteT1 *quote);
	void OnGTAQuoteData2(const QuoteT2 *quote);
	void OnGTAQuoteData3(const QuoteT3 *quote);
	void OnGTAQuoteData4(const QuoteT4 *quote);
	void OnGTAQuoteData5(const QuoteT5 *quote);
	void OnGTAQuoteData6(const QuoteT6 *quote);

	void subscribe_to_quote(const long &model_id, SubscribeContracts contracts);

	MYQuoteData* build_quote_provider(SubscribeContracts &subscription) {
		return new MYQuoteData(&subscription, mktdata_config_);
	}
};

#include "quote_sourcedef.h"
#endif
