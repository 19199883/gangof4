//============================================================================
// Name        : quote_forwarder.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>

#include "quote_interface_level1.h"
#include "quote_entity.h"
#include "forwarder.h"
#include <signal.h>     /* signal */
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include "quote_source.h"
#include "quotesetting.h"
#include "../catch_sigs.h"
#include "../catch_sigs.h"
#include "maint.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

typedef quote_source<quote_src_types::ctp,CDepthMarketDataField> QuoteSourceT;

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
	//this_thread::sleep_for(std::chrono::seconds(20));

	struct sigaction intrc_handle;
	intrc_handle.sa_handler = ctrlc_handler;
	sigemptyset(&intrc_handle.sa_mask);
	intrc_handle.sa_flags = 0;
	sigaction(SIGINT, &intrc_handle, NULL);

	install_sig_handlers();
	maintenance::assemble();

	DOMConfigurator::configure("log4cxx_config.xml");
	quote_setting setting("ctp_quote_forwarder.xml");

	forwarder<CDepthMarketDataField> *forwarder1= new forwarder<CDepthMarketDataField>(setting.forwarders["CDepthMarketDataField"]);
	quote_sourceInc = new QuoteSourceT(setting.MarketdataConfig,setting.Subscription,forwarder1,
			NULL,NULL,NULL,NULL,NULL);
	thread t1 = thread(bind(&QuoteSourceT::start,quote_sourceInc));

	t1.join();

	return 0;
}
