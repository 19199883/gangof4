//============================================================================
// Name        : quote_forwarder.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "quote_interface_option_lts.h"
#include <string>
#include "forwarder.h"
#include <signal.h>     /* signal */
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include "quote_source.h"
#include "quotesetting.h"
#include "stock_config_robot.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

typedef quote_source<quote_src_types::tdf,T_OptionQuote> QuoteSourceT;
typedef forwarder<T_OptionQuote> forwarderT1;

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

	forwarderT1 *forwarder1= new forwarderT1(setting.forwarders["OptionQuote"]);

	quote_sourceInc = new QuoteSourceT(setting.MarketdataConfig,setting.Subscription,forwarder1,(forwarder<int>*)NULL,
			(forwarder<int>*)NULL,(forwarder<int>*)NULL,(forwarder<int>*)NULL,(forwarder<int>*)NULL);

	thread t1 = thread(bind(&QuoteSourceT::start,quote_sourceInc));

	t1.join();

	return 0;
}
