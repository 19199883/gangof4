/*
 * quoteforwarderagent.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: root
 */

#include "quoteforwarderagent.h"
#include <string>
#include <stdio.h>
#include <string.h>
#include <perfctx.h>

using namespace std;

template<typename QuoteT>
quote_forwarder_agent<QuoteT>::
quote_forwarder_agent(quote_source_setting setting):
#ifdef rss
	MAX_QUOTE_COUNT(40000)
#else
	MAX_QUOTE_COUNT(6000)
#endif
{
	semid = -1;
	semkey = -1;
	shm_data_key = -1;
	shm_data_flag_key=  -1;

	_setting = setting;
	QuoteTalbe = NULL;
	quote_available_table = NULL;
	_cursor = 0;
	_stopped = true;
	_forward_handler = NULL;
	quote_count = 0;
}

template<typename QuoteT>
quote_forwarder_agent<QuoteT>::
~quote_forwarder_agent() {
	_stopped = true;
	_forward_handler = NULL;
}

template<typename QuoteT>
inline
void
quote_forwarder_agent<QuoteT>::
SetQuoteDataHandler(function<void(const QuoteT*)> handler) {
	_forward_handler = handler;
}

template<typename QuoteT>
void quote_forwarder_agent<QuoteT>
::p()
{
	struct sembuf sem_p;
	sem_p.sem_num=0;
	sem_p.sem_op=-1;
	// TODO: consider whether there is a bug.
	// commented by wangying on 20160727 for CPU overly occupied
//	sem_p.sem_flg = IPC_NOWAIT;
	while(semop(semid,&sem_p,1)==-1){
		usleep(5);
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"p operation failed,cause:"<<strerror(errno));
	}
	
	// TODO: debug
	union semun semopts;
	int val = semctl(semid,0,GETVAL,semopts);
	LOG4CXX_DEBUG(log4cxx::Logger::getRootLogger(),"sem value:" << val);
	LOG4CXX_DEBUG(log4cxx::Logger::getRootLogger(),"sem id:" << semid);
}

template<typename QuoteT>
void
quote_forwarder_agent<QuoteT>::
generate_shm_data_id() {
	key_t shm_data_key;
	shm_data_key = ftok(this->_setting.shmdatakeyfile.c_str(), 0);
	if (shm_data_key == -1) {
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate shm_data_key,cause:" << strerror(errno));
	}

	this->shm_data_id = shmget(shm_data_key,0,0666);
	if(shm_data_id==-1){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate shm_data_id,cause:" << strerror(errno));
	}
	else{
		QuoteTalbe = (QuoteT*)shmat(shm_data_id,0,0);
		if(QuoteTalbe == (QuoteT*)-1)
		{
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to shmat,cause:" << strerror(errno));
		}
	}
}

template<typename QuoteT>
void
quote_forwarder_agent<QuoteT>::
generate_shm_data_flag_id() {
	key_t shm_data_flag_key;
	shm_data_flag_key = ftok(this->_setting.shmdataflagkeyfile.c_str(), 0);
	if (shm_data_flag_key == -1) {
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate shm_data_flag_key,cause:" << strerror(errno));
	}

	this->shm_data_flag_id = shmget(shm_data_flag_key,0,0666);
	if(shm_data_flag_id==-1){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate shm_data_flag_id,cause:" << strerror(errno));
	}
	else{
		quote_available_table = (std::atomic<bool>*)shmat(shm_data_flag_id,0,0);
		if(quote_available_table == (std::atomic<bool>*)-1)
		{
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to shmat,cause:" << strerror(errno));
		}
	}
}

template<typename QuoteT>
void
quote_forwarder_agent<QuoteT>::
generate_semids() {
	key_t semkey = ftok(this->_setting.semkeyfile.c_str(), 0);
	if (semkey == -1) {
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate semkey,cause:" << strerror(errno));
	}

	semid = semget(semkey,2,0666);
	if (semid == -1) {
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate semid,cause:"<<strerror(errno));
	}
	LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"semid:"<< semid);
}

template<typename QuoteT>
void quote_forwarder_agent<QuoteT>::discard_obsolete_quotes() {
	// discard obsolete quote
	while (true == quote_available_table[_cursor]) {
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"discard quote");
		quote_count++;
		_cursor++;
		if(_cursor>=MAX_QUOTE_COUNT){
			_cursor = 0;
		}
	}
}

template<typename QuoteT>
void
quote_forwarder_agent<QuoteT>::
start()
{
	generate_semids();
	generate_shm_data_id();
	generate_shm_data_flag_id();

#ifndef rss
	discard_obsolete_quotes();
#endif

	_stopped = false;
    while (!_stopped){
		p();

		while (true == quote_available_table[_cursor]) {
			if (NULL != _forward_handler){

#ifdef LATENCY_MEASURE
                // latency measure
                perf_ctx::insert_t0(_cursor);
#endif
				_forward_handler(this->QuoteTalbe+_cursor);
			}
			else{
				LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"_forward_handler is null.");
			}

			_cursor++;
			if(_cursor>=MAX_QUOTE_COUNT){
				_cursor = 0;
			}

			quote_count++;
		}	// end while (true == quote_available_table[_cursor])
    }
}

