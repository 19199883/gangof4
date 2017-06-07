/*
 * forwardersetting.cpp
 *
 *  Created on: Apr 24, 2014
 *      Author: root
 */

#include "quotesetting.h"
#include <tinyxml.h>
#include <tinystr.h>


quote_setting::quote_setting(string config){
	_config = config;
	read_config();
}

quote_setting::~quote_setting() {
}


void quote_setting::read_config() {
	TiXmlDocument config = TiXmlDocument(this->_config.c_str());
	config.LoadFile();
	// root element，QuoteSrc
	TiXmlElement* RootElement = config.RootElement();

	// subscription element
	TiXmlElement* subscriptionE = RootElement->FirstChildElement("subscription");
	if (NULL != subscriptionE) {
		// item element
		TiXmlElement *itemE = subscriptionE->FirstChildElement();
		while (itemE != 0) {
			this->Subscription.insert(itemE->Attribute("value"));
			itemE = itemE->NextSiblingElement();
		}
	}

	TiXmlElement* MarketData = RootElement->FirstChildElement("MarketData");
	if (NULL != MarketData) {
		MarketdataConfig = MarketData->Attribute("config");
	}

	TiXmlElement* forwardersE = RootElement->FirstChildElement("forwarders");
	if (NULL != forwardersE) {
		// forwarder element
		TiXmlElement *forwarderE = forwardersE->FirstChildElement();
		while (forwarderE != 0) {
			string quoteType = forwarderE->Attribute("quoteType");
			this->forwarders[quoteType] = forwarder_setting();
			// shared memory element
			TiXmlElement* sharedMemoryE = forwarderE->FirstChildElement("sharedmemory");
			if (NULL != sharedMemoryE) {
				this->forwarders[quoteType].shm_data_key_file = sharedMemoryE->Attribute("datakey");
				this->forwarders[quoteType].shm_data_flag_key_file = sharedMemoryE->Attribute("dataflagkey");
			}

			// semaphores element
			TiXmlElement* semaphoresE = forwarderE->FirstChildElement("semaphores");
			if (NULL != semaphoresE) {
				// semaphore element
				TiXmlElement *semaphoreE = semaphoresE->FirstChildElement();
				while (NULL != semaphoreE) {
					this->forwarders[quoteType].semkey_files.push_back(semaphoreE->Attribute("key"));
					semaphoreE = semaphoreE->NextSiblingElement();
				}
			}

			forwarderE = forwarderE->NextSiblingElement();
		} // end while (forwarderE != 0) {
	} // end if (NULL != forwardersE) {
}



