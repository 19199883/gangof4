/*
 * forwarder.h
 *
 *  Created on: Apr 24, 2014
 *      Author: root
 */

#ifndef FORWARDER_H_
#define FORWARDER_H_

#include "forwardersetting.h"
#include <vector>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <log4cxx/logger.h>
#include <thread>
#include <atomic>         // std::atomic_flag
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <ctime>
#include <ratio>
#include <chrono>
#include <unistd.h>
#include <mutex>
#include "quote_entity.h"

using namespace std;
using namespace std::chrono;


//union semun{
//	 int val;
//	 struct semid_ds *buf;
//	 ushort *array;
// }sem_u;

template<typename QuoteT>
class forwarder {
public:
	forwarder(forwarder_setting setting);
	virtual ~forwarder();

	void start();
	void forward(const QuoteT &quote);

	forwarder_setting _setting;

	bool _stopped;

private:

	void generate_shm_for_data_flag();
	void generate_shm_for_quote();
	void generate_semids();
	void v(int semid);

	/*
	 * forwarder do NOT exit until mytrader connected to it finish processing all quotes.
	 */
	void sync_exit();

	// suppose that the fastest frequency is one quote per millisecond
	const size_t MAX_QUOTE_COUNT;
	size_t data_shared_mmemory_max_size;
	size_t data_flag_shared_mmemory_max_size;

	/*
	 * the field stores s whole day of quote data.
	 *
	 */
	QuoteT *QuoteTalbe;

	atomic<bool> *quote_available_table;

	/*
	 * record the latest quote of position
	 */
	int _cursor;


	/*
	 * store semaphore id for P/V
	 */
	vector<int> semids;

	/*
	 * store shared memory id.
	 */
	int data_shmid;
	int data_flag_shmid;

	int quote_count;
};

#include "forwarderdef.h"

#endif /* FORWARDER_H_ */
