/*
 * forwardersetting.h
 *
 *  Created on: Apr 24, 2014
 *      Author: root
 */

#ifndef FORWARDERSETTING_H_
#define FORWARDERSETTING_H_

#include <vector>
#include <string>
#include <map>
#include "../my_exchange.h"

#ifdef rss
#endif

using namespace std;

class forwarder_setting {
public:
	/*
	 * file paths used to generate key for semaphore of P/V.
	 */
	vector<string> semkey_files;

	/*
	 * file paths used to generate key for shared memory.
	 */
	string shm_data_key_file;
	string shm_data_flag_key_file;

#ifdef rss
	string shm_local_timestamp_key_file;
#endif

public:
	forwarder_setting();
	virtual ~forwarder_setting();
};

#endif /* FORWARDERSETTING_H_ */
