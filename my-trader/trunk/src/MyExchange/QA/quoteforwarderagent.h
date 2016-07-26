/*
 * quoteforwarderagent.h
 *
 *  Created on: Apr 25, 2014
 *      Author: root
 */

#ifndef QUOTEFORWARDERAGENT_H_
#define QUOTEFORWARDERAGENT_H_

#include <ctime>
#include <ratio>
#include <chrono>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <ostream>
#include <thread>         // std::this_thread::sleep_for
#include <errno.h>
#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <thread>
#include <atomic>         // std::atomic_flag
#include "../my_exchange.h"

using namespace std;

template<typename QuoteT>
class quote_forwarder_agent {
public:
	quote_forwarder_agent(quote_source_setting setting);
	virtual ~quote_forwarder_agent();
	void SetQuoteDataHandler(function<void (const QuoteT *)> quote_handler);
	void start();

private:
	void generate_shm_data_id();
	void generate_shm_data_flag_id();
	void generate_semids();
	void p();

	void discard_obsolete_quotes();

	const size_t MAX_QUOTE_COUNT;
	int quote_count;

	function<void(const QuoteT *)> _forward_handler;

	quote_source_setting _setting;

	key_t semkey;
	key_t shm_data_key;
	key_t shm_data_flag_key;

	int semid;
	int shm_data_id;
	int shm_data_flag_id;

	/*
	 * the field stores s whole day of quote data.
	 *
	 */
	QuoteT *QuoteTalbe;
	std::atomic<bool> *quote_available_table;

	/*
	 * record the latest quote of position
	 */
	int _cursor;

	bool _stopped;
};

#include "quoteforwarderagentdef.h"

#endif /* QUOTEFORWARDERAGENT_H_ */
