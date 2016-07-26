#ifndef ENGINE_H_
#define ENGINE_H_

#include <list>
#include <boost/function.hpp>
#include <boost/function_equal.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include "model_manager.h"
#include "model_adapter.h"
#include "qa.h"
#include "tca.h"
#include "strategy_unit.h"
#include <dlfcn.h>
#include "moduleloadlibrarylinux.h"
#include "loadlibraryproxy.h"
#include <memory>
#include "proxy.h"

using namespace trading_channel_agent;
using namespace quote_agent;
using namespace strategy_manager;
using namespace std;

namespace trading_engine
{
	template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
	class engine
	{
		typedef model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> ModelAdapterT;
		typedef model_manager<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> ModelManagerT;
		typedef strategy_unit<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> StrategyUnitT;
		typedef qa<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> QaT;
		typedef list<shared_ptr<ModelAdapterT> > ModelAdapterListT;
	public:		
		/**
		该字段记录te模块当前状态，如果为true，则表示该模块处于非工作状态
		*/
		static bool stopped;
		shared_ptr<ModelManagerT> model_manager_ptr;
		shared_ptr<QaT> qa_ptr;
		shared_ptr<tca> tca_ptr;

		static int timeEventInterval_;

	private:		
		/*
		key:策略id
		value:策略实例
		*/
		map <long,shared_ptr<StrategyUnitT> > _unit_ptrs;
		CLoadLibraryProxy *_pproxy;

		bool disposed;
		monitor_proxy *monitor_proxy_;

		/* a tcp port used to accept connection from tcp client*/
		int listen_;
		boost::asio::io_service io_service_;

	public:
		void finalize(void);
		engine(void);
		~engine(void);
		void initialize(void);
		void run(void);
		void subscribe(shared_ptr<ModelAdapterT> model_ptr);
					
	};
}

#include "enginedef.h"
#endif
