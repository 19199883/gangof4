#pragma once

#include <list>
#include <map>
#include <functional>   // std::bind
#include <thread>
#include "signal_entity.h"
#include "tca.h"
#include "model_adapter.h"
#include "qa.h"
#include "tcs.h"
#include "my_order.h"
#include <condition_variable> // std::condition_variable
#include <mutex>
#include <ctime>
#include <ratio>
#include <chrono>
#include <memory>
#include "signal_entity.h"
#include "../my_exchange.h"
#include "trading_signal_processor.h"

using namespace std;
using namespace trading_channel_agent;
using namespace strategy_manager;
using namespace quote_agent;

namespace trading_engine
{
	template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
	class cancel_signal_processor
	{
	private:

		typedef qa<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> QaT;
		typedef model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> ModelAdapterT;
		typedef cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> CancelSignalProcessorT;
		typedef trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> TradeSigProcessorT;

		ReportNotifyRecordT _report_record_cache;

		QuoteOrderRecordT _quote_record_cache;

		bool stopped;
		ReportNotifyTableKeyT rpt_notify_key;
	public:
		shared_ptr<tca> tca_ptr;

		shared_ptr<model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5 > > model_ptr;

	public:
		cancel_signal_processor(
				shared_ptr<tca> tca_ptr,
				shared_ptr<model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> > model_ptr,
				shared_ptr<QaT> qa_ptr);

		virtual ~cancel_signal_processor(void);

		/*
		 * 处理撤单信号。
		 * 根据撤单执行模式，执行相应的撤单操作
		 * 返回值：异步撤单信息：key：进行撤单的原委托请求的cl_ord_id；value：撤单的cl_ord_id
		 */
		void process(const long &model_id,signal_t *sigs, int &sig_cnt);
		void async_cancel_ords(const long &model_id,signal_t *sigs, int &sig_cnt);

		void cancel(my_order &ord);
		void cancel_quote(QuoteOrder &ord);

		/**
		执行同步撤单
		*/
		void sync_cancel_order(const long &model_id);
		void wait_for_ord_id(const long &model_id,map<unsigned long,unsigned long> sig_ids);
		void sync_cancel_order_core(const long &model_id,map<unsigned long,unsigned long> sig_ids,int &sig_cnt);
		void sync_cancel_ords(const long &model_id,signal_t *sigs, int &sig_cnt);

//		mutex sync_reqs_lock_;
//		std::promise<bool> sync_cancel_ords_prom_;
		map<unsigned long,unsigned long> sync_sig_id_cache_;

		/*
		 * market making
		 */
		void wait_for_quote_id(const long &model_id,map<unsigned long,unsigned long> sig_ids);

	public:
		void cancel_result_handler(tcs*tcs);

		// 记录最后完成撤单时间（初始值为0）
		high_resolution_clock::time_point _last_cancel_time;

	private:

		/**
		该条件变量负责同步请求接收操作与请求的处理操作.
		*/
		condition_variable cond_for_sync_cancel;
		mutex mut_for_sync_cancel;
		bool rpt_ready;

		mutex sync_req_rep_lock;

		/**
		key：进行撤单的原委托请求的cl_ord_id
		value：撤单的cl_ord_id
		*/
		map<long, long> sync_reqs;

		shared_ptr<QaT> _qa_ptr;

		thread process_ord_report_thread;
	public:

		void run(void);

	private:
		void process_one_report(const my_order &ord);
	int cancel_orders_imp(const long & model_id, map<unsigned long, unsigned long> sig_ids);

	int cancel_quotes_imp(const long & model_id, map<unsigned long, unsigned long> sig_ids);
};
}

#include "cancel_signal_processordef.h"
