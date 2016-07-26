//============================================================================
// Name        : quote_forwarder.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include <iostream>
#include "quote_interface_lts_level2.h"
#include <string>
#include "forwarder.h"
#include <signal.h>     /* signal */
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include "quote_source.h"
#include "quotesetting.h"
#include "stock_config_robot.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#include "kdb_quote_loader.h"
//#include "quote_loader.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

typedef quote_source<quote_src_types::tdf,TDF_MARKET_DATA_MY,TDF_INDEX_DATA_MY,T_PerEntrust,T_PerBargain> QuoteSourceT;
typedef forwarder<TDF_MARKET_DATA_MY> forwarderT1;
typedef forwarder<TDF_INDEX_DATA_MY> forwarderT2;
typedef forwarder<T_PerEntrust> forwarderT3;
typedef forwarder<T_PerBargain> forwarderT4;

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



/* the argument list in argv:
 * first argument: this executable file
 * second argument: path of hs300 file
 * third argument: file path which is used to store position data
 */
int main(int argc, char  *argv[]) {
	if(argc == 3){
		string hs300_file = argv[1];
		string pos_file = argv[2];
		map<string,string> pos;
		read_pos(pos_file, pos);
		return update_symbols_in_config(hs300_file,pos);
	}

	struct sigaction intrc_handle;
	intrc_handle.sa_handler = ctrlc_handler;
	sigemptyset(&intrc_handle.sa_mask);
	intrc_handle.sa_flags = 0;
	sigaction(SIGINT, &intrc_handle, NULL);

	DOMConfigurator::configure("log4cxx_config.xml");
	quote_setting setting("tdf_quote_forwarder.xml");

	forwarderT1 *forwarder1= new forwarderT1(setting.forwarders["TDF_MARKET_DATA_MY"]);
	forwarderT2 *forwarder2= new forwarderT2(setting.forwarders["TDF_INDEX_DATA_MY"]);
	forwarderT3 *forwarder3= new forwarderT3(setting.forwarders["PerEntrust"]);
	forwarderT4 *forwarder4= new forwarderT4(setting.forwarders["PerBargain"]);


	quote_sourceInc = new QuoteSourceT(setting.MarketdataConfig,setting.Subscription,
			forwarder1,forwarder2,forwarder3,forwarder4,(forwarder<int>*)NULL,(forwarder<int>*)NULL);

	thread t1 = thread(bind(&QuoteSourceT::start,quote_sourceInc));


	t1.join();

	return 0;
}
