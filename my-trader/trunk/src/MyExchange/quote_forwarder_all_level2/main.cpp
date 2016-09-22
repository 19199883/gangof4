//============================================================================
// Name        : quote_forwarder.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "quote_interface_all_level2.h"

#include <string>
#include "forwarder.h"
#include <signal.h>     /* signal */
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include "quote_source.h"
#include "quotesetting.h"
#include "../catch_sigs.h"
#include "maint.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

typedef quote_source<quote_src_types::ctp, MYShfeMarketData, MDBestAndDeep_MY, CFfexFtdcDepthMarketData, ZCEL2QuotSnapshotField_MY> QuoteSourceT;

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

	 install_sig_handlers();
	maintenance::assemble();

	DOMConfigurator::configure("log4cxx_config.xml");
	quote_setting setting("all_level2_quote_forwarder.xml");

	forwarder<MYShfeMarketData> *forwarder1= new forwarder<MYShfeMarketData>(setting.forwarders["MYShfeMarketData"]);
	forwarder<MDBestAndDeep_MY> *forwarder2= new forwarder<MDBestAndDeep_MY>(setting.forwarders["MDBestAndDeep"]);
	forwarder<CFfexFtdcDepthMarketData> *forwarder3= new forwarder<CFfexFtdcDepthMarketData>(setting.forwarders["CFfexFtdcDepthMarketData"]);
	forwarder<ZCEL2QuotSnapshotField_MY> *forwarder4= new forwarder<ZCEL2QuotSnapshotField_MY>(setting.forwarders["ZCEL2QuotSnapshotField_MY"]);

	quote_sourceInc = new QuoteSourceT(setting.MarketdataConfig,setting.Subscription,forwarder1,forwarder2,forwarder3,forwarder4,NULL,NULL);
	thread t1 = thread(bind(&QuoteSourceT::start,quote_sourceInc));
	t1.join();

	return 0;
}
