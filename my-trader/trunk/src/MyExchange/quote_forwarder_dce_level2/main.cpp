//============================================================================
// Name        : quote_forwarder.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "quote_interface_dce_level2.h"

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

typedef quote_source<quote_src_types::dce,MDBestAndDeep_MY,MDArbi_MY,MDTenEntrust_MY,MDOrderStatistic_MY,MDRealTimePrice_MY,MDMarchPriceQty_MY> QuoteSourceT;

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
	quote_setting setting("dce_quote_forwarder.xml");

	forwarder<MDBestAndDeep_MY> *forwarder1= new forwarder<MDBestAndDeep_MY>(setting.forwarders["MDBestAndDeep"]);
	forwarder<MDArbi_MY> *forwarder2= new forwarder<MDArbi_MY>(setting.forwarders["MDArbi"]);
	forwarder<MDTenEntrust_MY> *forwarder3= new forwarder<MDTenEntrust_MY>(setting.forwarders["MDTenEntrust"]);
	forwarder<MDOrderStatistic_MY> *forwarder4= new forwarder<MDOrderStatistic_MY>(setting.forwarders["MDOrderStatistic"]);
	forwarder<MDRealTimePrice_MY> *forwarder5= new forwarder<MDRealTimePrice_MY>(setting.forwarders["MDRealTimePrice"]);
	forwarder<MDMarchPriceQty_MY> *forwarder6= new forwarder<MDMarchPriceQty_MY>(setting.forwarders["MDMarchPriceQty"]);


	quote_sourceInc = new QuoteSourceT(setting.MarketdataConfig,setting.Subscription,forwarder1,forwarder2,forwarder3,forwarder4,forwarder5,forwarder6);
	thread t1 = thread(bind(&QuoteSourceT::start,quote_sourceInc));
	t1.join();

	return 0;
}
