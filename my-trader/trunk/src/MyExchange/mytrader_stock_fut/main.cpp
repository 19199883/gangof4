// main.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <functional>   // std::bind
#include <thread>
#include <stdio.h>      /* printf, tmpnam, L_tmpnam, remove */
#include <stdlib.h>     /* exit */
#include <signal.h>     /* signal */
#include <string>
#include "engine.h"

#include "quote_datatype_cffex_level2.h"
#include "quote_datatype_level1.h"
#include "quote_datatype_shfe_my.h"
#include "quote_datatype_shfe_deep.h"
#include "quote_datatype_level1.h"
#include "quote_datatype_dce_level2.h"
#include <log4cxx/logmanager.h>
#include "quote_datatype_stock_tdf.h"
#include "quote_datatype_sec_kmds.h"
#include "ClassFactory.h"
#include "my_cmn_log.h"
#include "../catch_sigs.h"
#include "stock_config_robot.h"

using namespace trading_engine;
using namespace std;

typedef engine<T_OptionQuote,CDepthMarketDataField,TDF_MARKET_DATA_MY,IBDepth,MDOrderStatistic_MY> EngineT;

EngineT *engine_ins;

static void
SIGINT_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete engine...");

	engine_ins->finalize();

	delete engine_ins;

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete engine.");

	log4cxx::LogManager::shutdown();
	exit(0);		/* call exit for the signal */
}


int main(int argc, char  *argv[])
{
	(void) my_cmn::my_log::instance(NULL);

	// Load configuration file
	DOMConfigurator::configure("log4cxx_config.xml");

	if(3 == argc){
		string hs300_file = argv[1];
		string pos_file = argv[2];
		map<string,string> pos;

		output_positions(pos_file);
		read_pos(pos_file, pos);
		return update_symbols_in_config(hs300_file,pos);
	}

	/*
	struct sched_param param;
	param.sched_priority = 99;
	pid_t pid = getpid();
	sched_setscheduler(pid,SCHED_RR,&param);
    */

	install_sig_handlers();

	struct sigaction SIGINT_act;
	SIGINT_act.sa_handler = SIGINT_handler;
	sigemptyset(&SIGINT_act.sa_mask);
	SIGINT_act.sa_flags = 0;
	sigaction(SIGINT, &SIGINT_act, NULL);

	engine_ins = new EngineT();
	engine_ins->initialize();
	thread t = thread(bind(&EngineT::run,engine_ins));

	t.join();
	
	return 0;
}

