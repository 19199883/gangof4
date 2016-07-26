//============================================================================
// Name        : quote_forwarder.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "quote_interface_stock_tdf.h"
#include <string>
#include "forwarder.h"
#include <signal.h>     /* signal */
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include "quote_source.h"
#include "quotesetting.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

typedef quote_source<quote_src_types::tdf,TDF_MARKET_DATA_MY,TDF_INDEX_DATA_MY> QuoteSourceT;
typedef forwarder<TDF_MARKET_DATA_MY> forwarderT1;
typedef forwarder<TDF_INDEX_DATA_MY> forwarderT2;

QuoteSourceT *quote_sourceInc;

static void
ctrlc_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"quote forwarder is exiting...");

	if (NULL != quote_sourceInc){
		delete quote_sourceInc;
		quote_sourceInc = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"quote forwarder exited normally.");

	exit(0);		/* call exit for the signal */
}

int main() {
	struct sigaction intrc_handle;
	intrc_handle.sa_handler = ctrlc_handler;
	sigemptyset(&intrc_handle.sa_mask);
	intrc_handle.sa_flags = 0;
	sigaction(SIGINT, &intrc_handle, NULL);

	DOMConfigurator::configure("log4cxx_config.xml");
	quote_setting setting("tdf_quote_forwarder.xml");

	forwarderT1 *forwarder1= new forwarderT1(setting.forwarders["TDF_MARKET_DATA_MY"]);
	forwarderT2 *forwarder2= new forwarderT2(setting.forwarders["TDF_INDEX_DATA_MY"]);

	quote_sourceInc = new QuoteSourceT(setting.MarketdataConfig,setting.Subscription,
			forwarder1,forwarder2,(forwarder<int>*)NULL,(forwarder<int>*)NULL,
			(forwarder<int>*)NULL,(forwarder<int>*)NULL);

	thread t1 = thread(bind(&QuoteSourceT::start,quote_sourceInc));

	t1.join();

	return 0;
}
