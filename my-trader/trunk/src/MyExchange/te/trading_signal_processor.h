#pragma once

#include <list>
#include <map>
#include <boost/shared_ptr.hpp>
#include "signal_entity.h"
#include "tca.h"
#include "position_entity.h"
#include "model_adapter.h"
#include "position_manager.h"
//#include "engine.h"
#include "my_order.h"
#include <vector>
#include <memory>

using namespace trading_channel_agent;
using namespace quote_agent;
using namespace strategy_manager;
using namespace std;

namespace trading_engine
{
	/**
	假设：
	（1）每一个信号周期内，对于一个合约，则只有一个交易信号
	TO IMPROVE:
	（1）撤单时，未完成的顺序模式的信号暂时不需要处理，将来要详细考虑
	*/
	template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
	class trading_signal_processor
	{
		typedef model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> ModelAdapterT;
	private:		
		bool stopped;
		bool rpt_ready;

		 shared_ptr<position_manager> position_mamager_ptr;

	private:
		shared_ptr<ModelAdapterT> model_ptr;
		shared_ptr<tca> tca_ptr;

	public:
		/*
		 * key: model id
		 * value: map:key:signal id,value: an object of set<long> whose elements are cl_ord_ids of orders
		 */
		static map<long, map<long,set<long> > > sig_order_mapping_;

	private:
		void place_order(signal_t *sig);
		void place_quote(signal_t *sig);
		void place_req_quote(signal_t *sig);

	public:
		void start()
		{
			stopped = false;
		}

		void stop()
		{
			stopped = true;
		}

		void run();
		trading_signal_processor(shared_ptr<tca> tca_ptr,shared_ptr<ModelAdapterT> _model_ptr);

		~trading_signal_processor(void);

		void process(const long &model_id,signal_t *sig);
		void cancel(signal_t *sig);
	};
}

#include "trading_signal_processordef.h"
