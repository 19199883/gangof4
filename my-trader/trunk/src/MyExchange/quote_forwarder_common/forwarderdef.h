/*
 * forwarder.cpp
 *
 *  Created on: Apr 24, 2014
 *      Author: root
 */

#include "forwarder.h"
#include <functional>   // std::bind
#include <thread>
#include <stdio.h>      /* printf, tmpnam, L_tmpnam, remove */
#include <stdlib.h>     /* exit */
#include <signal.h>     /* signal */
#include <log4cxx/logger.h>

template<typename QuoteT>
forwarder<QuoteT>::forwarder(forwarder_setting setting)
:MAX_QUOTE_COUNT(6000)
{
	_setting = setting;
	data_shared_mmemory_max_size = MAX_QUOTE_COUNT*sizeof(QuoteT);
	data_flag_shared_mmemory_max_size = MAX_QUOTE_COUNT*sizeof(atomic<bool>);
	_cursor = 0;
	QuoteTalbe = NULL;
	quote_available_table = NULL;
	data_flag_shmid = -1;
	data_shmid = -1;
	_stopped = true;
	quote_count = 0;
}

template<typename QuoteT>
forwarder<QuoteT>::~forwarder() {
	this->_stopped = true;

	vector<int>::iterator it = this->semids.begin();
	vector<int>::iterator end = this->semids.end();
	for(; it!=end; it++){
		if(semctl(*it,0,IPC_RMID,0)==-1){
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"failed to delete sem,cause:"<<strerror(errno));
		}
	}
	semids.clear();
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"released semaphore resources.");

	if(shmctl(data_shmid,IPC_RMID,NULL)==-1){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"failed to delete data_shmid,cause:"<<strerror(errno));
	}
	data_shmid = -1;
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"released data_shmid shared memory resources.");

	if(shmctl(data_flag_shmid,IPC_RMID,NULL)==-1){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"failed to delete data_flag_shmid,cause:"<<strerror(errno));
	}
	data_flag_shmid = -1;
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"released data_flag_shmid shared memory resources.");
}

template<typename QuoteT>
void forwarder<QuoteT>::generate_shm_for_data_flag() {
	key_t data_flag_key;
	data_flag_key = ftok(this->_setting.shm_data_flag_key_file.c_str(), 0);
	if (data_flag_key == -1) {
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate data_flag_key,cause:"
			<<strerror(errno) << "file:"<<this->_setting.shm_data_flag_key_file);
		return;
	}

	this->data_flag_shmid = shmget(data_flag_key,data_flag_shared_mmemory_max_size,0666|IPC_CREAT);
	if(data_flag_shmid==-1){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate data_flag_shmid,cause:"<<strerror(errno));
	}
	else{
		quote_available_table = (atomic<bool>*)shmat(data_flag_shmid,0,0);
		if(quote_available_table == (atomic<bool>*)-1){
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to shmat,cause:"<<strerror(errno));
		}
		else{
			memset(quote_available_table,0,data_flag_shared_mmemory_max_size);
			for(int i=0; i<MAX_QUOTE_COUNT; i++){
				quote_available_table[i] = false;
			}
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				this->_setting.shm_data_flag_key_file << "data_flag_shmid:" << this->_setting.shm_data_key_file);
		}
	}
}

template<typename QuoteT>
void forwarder<QuoteT>::generate_shm_for_quote() {
	key_t data_key;
	data_key = ftok(this->_setting.shm_data_key_file.c_str(), 0);
	if (data_key == -1) {
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
				this->_setting.shm_data_key_file << ",data_key,cause:"<<strerror(errno)
				<< "file:"<<this->_setting.shm_data_key_file);
		return;
	}

	this->data_shmid = shmget(data_key,data_shared_mmemory_max_size,0666|IPC_CREAT);
	if(data_shmid==-1){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"this->_setting.shm_data_key_file,data_shmid,cause:"<<strerror(errno));
	}
	else{
		QuoteTalbe = (QuoteT*)shmat(data_shmid,0,0);
		if(QuoteTalbe == (QuoteT*)-1){
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to shmat,cause:"<<strerror(errno));
		}
		else{
			memset(QuoteTalbe,0,data_shared_mmemory_max_size);
		}
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),
			this->_setting.shm_data_flag_key_file << "shm_data_key_file:" << this->_setting.shm_data_key_file);
	}
}

template<typename QuoteT>
inline void forwarder<QuoteT>::forward(const QuoteT& quote) {
	if (quote_available_table==NULL || QuoteTalbe==NULL){
		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"quote_available_table or QuoteTalbe is NULL");
		return;
	}

	quote_count++;

	int cur_pos = _cursor;
	int next_pos = cur_pos + 1;
	if (next_pos >= MAX_QUOTE_COUNT){
		next_pos = 0;
	}
	quote_available_table[cur_pos] = false;
	quote_available_table[next_pos] = false;

	QuoteTalbe[cur_pos] = quote;
	quote_available_table[cur_pos] = true;

	_cursor = next_pos;

	vector<int>::iterator it = this->semids.begin();
	vector<int>::iterator end = this->semids.end();

	for(; it!=end; it++){
		v(*it);
	}
}

template<typename QuoteT>
void forwarder<QuoteT>::v(int semid)
{
	struct sembuf sem_v;
	sem_v.sem_num=0;
	sem_v.sem_op=1;
	sem_v.sem_flg = IPC_NOWAIT;
	if(semop(semid,&sem_v,1)==-1){
//		LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"v operation is failed,cause:"<<strerror(errno));
	}
}

template<typename QuoteT>
void forwarder<QuoteT>::generate_semids() {
	vector<string>::iterator it = this->_setting.semkey_files.begin();
	vector<string>::iterator end = this->_setting.semkey_files.end();
	for (; it != end; it++) {
		key_t semkey = ftok((*it).c_str(), 0);
		if (semkey == -1) {
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate semkey,cause:"<<strerror(errno)
					<<";file" << (*it));
		}

		int semid = semget(semkey,2,0666|IPC_CREAT);
		if (semid == -1) {
			LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to generate semid,cause:"<<strerror(errno));
		}
		else{
			union semun semopts;
			semopts.val = 0;
			if (semctl(semid,0,SETVAL,semopts)==-1){
				LOG4CXX_ERROR(log4cxx::Logger::getRootLogger(),"it's failed to semctl,cause:"<<strerror(errno));
			}
			else{
				this->semids.push_back(semid);
			}
			LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"semid:"<< semid);
		}
	}
}

template<typename QuoteT>
void forwarder<QuoteT>::start() {
	generate_shm_for_quote();
	generate_shm_for_data_flag();
	generate_semids();

	_stopped = false;
}
