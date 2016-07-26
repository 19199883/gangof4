#include "../my_exchange.h"
#include <iostream>
#include <boost/array.hpp>
#include <cstdlib>
#include <exception>
#include <boost/foreach.hpp>
#include <sstream>
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include <chrono>
#include <ctime>
#include <ratio>
#include <ctime>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <stdio.h>
#include <string.h>

using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
quote_source(
		string mktdata_config,
		set<string> subscription,
		forwarder<QuoteT1> *forwarderInc1,
		forwarder<QuoteT2> *forwarderInc2,
		forwarder<QuoteT3> *forwarderInc3,
		forwarder<QuoteT4> *forwarderInc4,
		forwarder<QuoteT5> *forwarderInc5,
		forwarder<QuoteT6> *forwarderInc6){
	_stopped = true;
	_quote_src = NULL;
	_subscription = subscription;
	_forwarderInc1 = forwarderInc1;
	_forwarderInc2 = forwarderInc2;
	_forwarderInc3 = forwarderInc3;
	_forwarderInc4 = forwarderInc4;
	_forwarderInc5 = forwarderInc5;
	_forwarderInc6 = forwarderInc6;

	mktdata_config_ = mktdata_config;
	_subscription = subscription;
}

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
~quote_source(void){
	_stopped = true;

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete real quote source...");
	if (_quote_src != NULL)	{
		delete _quote_src;
		_quote_src = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete _forwarderInc1...");
	// delete all forwarders
	if (NULL !=_forwarderInc1){
		delete _forwarderInc1;
		_forwarderInc1 = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete _forwarderInc2...");
	if(NULL != _forwarderInc2){
		delete _forwarderInc2;
		_forwarderInc2 = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete _forwarderInc3...");
	if(NULL != _forwarderInc3){
		delete _forwarderInc3;
		_forwarderInc3 = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete _forwarderInc4...");
	if(NULL != _forwarderInc4){
		delete _forwarderInc4;
		_forwarderInc4 = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete _forwarderInc5...");
	if(NULL != _forwarderInc5){
		delete _forwarderInc5;
		_forwarderInc5 = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete _forwarderInc6...");
	if(NULL != _forwarderInc6){
		delete _forwarderInc6;
		_forwarderInc6 = NULL;
	}

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"delete quote source  successfully.");
}

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
void quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
start(void){
	_stopped = false;

	if (NULL!=_forwarderInc1){
		_forwarderInc1->start();
	}
	if (NULL!=_forwarderInc2){
		_forwarderInc2->start();
	}
	if (NULL!=_forwarderInc3){
		_forwarderInc3->start();
	}
	if (NULL!=_forwarderInc4){
		_forwarderInc4->start();
	}
	if (NULL!=_forwarderInc5){
		_forwarderInc5->start();
	}
	if (NULL!=_forwarderInc6){
		_forwarderInc6->start();
	}

	// use template specialization technology.
	_quote_src = build_quote_provider(_subscription);
	this->subscribe_to_symbols();

	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"quote forwarder started.");

	while(!_stopped){
		this_thread::sleep_for(std::chrono::milliseconds(2));
	}
}

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
void
quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
OnGTAQuoteData1(const QuoteT1 *quote)
{
	if (!_stopped){
		QuoteT1 duplicated_quote = *quote;
		this->_forwarderInc1->forward(duplicated_quote);
	}
}

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
void
quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
OnGTAQuoteData2(const QuoteT2 *quote)
{
	if (!_stopped){
		QuoteT2 duplicated_quote = *quote;
		this->_forwarderInc2->forward(duplicated_quote);
	}
}

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
void
quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
OnGTAQuoteData3(const QuoteT3 *quote)
{
	if (!_stopped){
		QuoteT3 duplicated_quote = *quote;
		this->_forwarderInc3->forward(duplicated_quote);
	}
}

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
void
quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
OnGTAQuoteData4(const QuoteT4 *quote)
{
	if (!_stopped){
		QuoteT4 duplicated_quote = *quote;
		this->_forwarderInc4->forward(duplicated_quote);
	}
}

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
void
quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
OnGTAQuoteData5(const QuoteT5 *quote)
{
	if (!_stopped){
		QuoteT5 duplicated_quote = *quote;
		this->_forwarderInc5->forward(duplicated_quote);
	}
}

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
void
quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
OnGTAQuoteData6(const QuoteT6 *quote)
{
	if (!_stopped){
		QuoteT6 duplicated_quote = *quote;
		this->_forwarderInc6->forward(duplicated_quote);
	}
}

template<quote_src_types quote_src,typename QuoteT1,typename QuoteT2,typename QuoteT3,typename QuoteT4,typename QuoteT5,typename QuoteT6>
void
quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6>::
subscribe_to_symbols()
{
	typedef quote_source<quote_src,QuoteT1,QuoteT2,QuoteT3,QuoteT4,QuoteT5,QuoteT6> QuoteSrcT;


	if (IsIntegerT<QuoteT1>::No){
		boost::function<void (const QuoteT1 *)> f = boost::bind(&QuoteSrcT::OnGTAQuoteData1, this, _1);
		_quote_src->SetQuoteDataHandler(f);
	}

	if (IsIntegerT<QuoteT2>::No){
		boost::function<void (const QuoteT2 *)> f1 = boost::bind(&QuoteSrcT::OnGTAQuoteData2, this, _1);
		_quote_src->SetQuoteDataHandler(f1);
	}

	if (IsIntegerT<QuoteT3>::No){
		boost::function<void (const QuoteT3 *)> f2 = boost::bind(&QuoteSrcT::OnGTAQuoteData3, this, _1);
		_quote_src->SetQuoteDataHandler(f2);
	}

	if (IsIntegerT<QuoteT4>::No){
		boost::function<void (const QuoteT4 *)> f3 = boost::bind(&QuoteSrcT::OnGTAQuoteData4, this, _1);
		_quote_src->SetQuoteDataHandler(f3);
	}

	if (IsIntegerT<QuoteT5>::No){
		boost::function<void (const QuoteT5 *)> f4 = boost::bind(&QuoteSrcT::OnGTAQuoteData5, this, _1);
		_quote_src->SetQuoteDataHandler(f4);
	}

	if (IsIntegerT<QuoteT6>::No){
		boost::function<void (const QuoteT6 *)> f5 = boost::bind(&QuoteSrcT::OnGTAQuoteData6, this, _1);
		_quote_src->SetQuoteDataHandler(f5);
	}

	this_thread::sleep_for(std::chrono::microseconds(10));
}
