#pragma once

#include <list>
#include <functional>   // std::bind
#include <thread>
#include "../my_exchange.h"
#include "model_manager.h"
#include "model_adapter.h"
#include "qa.h"
#include "tca.h"
#include "trading_signal_processor.h"
#include "pending_quote_dao.h"
#include "pending_ord_request_dao.h"
#include "general_depth_quote_source.h"
#include <condition_variable> // std::condition_variable
#include "tbb/include/tbb/spin_mutex.h"
#include <memory>
#include "signal_entity.h"
#include "position_entity.h"
#include "cancel_signal_processor.h"
#include "option_interface.h"
#include <mutex>          // std::mutex, std::lock_guard
#include "my_trade_tunnel_api.h"

using namespace trading_channel_agent;
using namespace quote_agent;
using namespace strategy_manager;
using namespace std;
using namespace trading_channel_agent;
using namespace trading_engine;
using namespace tbb;

namespace trading_engine
{
	template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,typename FullDepthQuoteT,typename QuoteT5>
	class strategy_unit
	{
	private:
		typedef qa<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> QaT;
		typedef model_adapter<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> ModelAdapterT;
		typedef cancel_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> CancelSignalProcessorT;
		typedef trading_signal_processor<SPIFQuoteT,CFQuoteT,StockQuoteT,FullDepthQuoteT,QuoteT5> TradeSignalProcessorT;
		typedef pending_quote_dao<SPIFQuoteT> SPIFPendingQuoteDaoT;
		typedef pending_quote_dao<CFQuoteT> CFPendingQuoteDaoT;
		typedef pending_quote_dao<StockQuoteT> StockPendingQuoteDaoT;
		typedef pending_quote_dao<FullDepthQuoteT> FullDepthPendingQuoteDaoT;
		typedef pending_quote_dao<QuoteT5> PendingQuoteDaoT5;

		vector<signal_t> sig_cache;
		int sig_cache_size;

		/*
		 * the model is using trading channel.
		 * currently,only support one channel for one model
		 */
		//tcs *channel_;

		int timeEventInterval_;

		position_t pos_cache_;
		pending_order_t pending_order_cache;

		ReportNotifyTableKeyT cache_key_;

		bool ready_;
	public:

		bool stopped;

		shared_ptr<tca> _tca_ptr;
		shared_ptr<QaT> _qa_ptr;
		shared_ptr<ModelAdapterT> _model_ptr;
		int _period_counter;

		/**
		* 推持仓线程
		*/
		thread feed_pos_thread;
		thread  spif_quote_process_thread;
		thread  cf_quote_process_thread;
		thread  stock_quote_process_thread;
		thread  full_depth_quote_process_thread;
		thread  quote5_process_thread;
		thread  _signal_process_thread;
		vector<thread*>  process_sig_response_threads_;
		thread timer_thread_;
		vector<thread*> process_quote_notify_threads_;
	public:
		mutex mut_for_main_proc;

		position_manager _pos_mgr;

		shared_ptr<CancelSignalProcessorT> _csp_ptr;

		shared_ptr<TradeSignalProcessorT> _tsp_ptr;

		//ReportNotifyRecordT duplicated_records_;

		//vector<T_RtnForQuote> duplicated_quote_nitify_records_;

		signal_resp_t sig_rep_cache_;
	public:
		strategy_unit(
				shared_ptr<tca> tca_ptr,
				shared_ptr<QaT> qa_ptr,
				shared_ptr<ModelAdapterT> model_ptr,
				int timeEventInterval);

		~strategy_unit(void);
		void initialize(void);
		void finalize(void);
		void run(void);
		long get_id();

		void start()
		{
			stopped = false;
		}

		void stop()
		{
			stopped = true;
		}

		int InitOptionStrategy(startup_init_t *init_content, int *err);
		int UpdateOptionStrategy(ctrl_t *update_content, int *err);
		int GetMonitorInfo(monitor_t *monitor_content, int *err);
	public:
		/**
		processing_signal_thread线程的线程函数，该函数实现了针对于每个模型而言，在交易阶段
		的交易主流程，包括：
		*/
		void trade_main_process(
			shared_ptr<ModelAdapterT> model_ptr,
			shared_ptr<QaT> qa_ptr,
			shared_ptr<tca> tca_ptr,
			shared_ptr<CancelSignalProcessorT> csp_ptr,
			shared_ptr<TradeSignalProcessorT> tsp_ptr);

	private:

		void feed_init_pos(st_config_t &config);

		void process_sig_response(tcs *tcs_ins);
		void produce_signal_by_sig_response(ReportNotifyRecordT duplicated_records_,int total);
		void produce_sig_rep(my_order &src,signal_resp_t &dest);
		/**
		当qa模块有新的行情接收时，则会调用该方法。
		该方法通过线程同步技术，促使开始执行一次主流程，见:trade_main_process
		*/
		void process_spif_quote();
		void process_cf_quote();
		void process_stock_quote();
		void process_full_depth_quote();
		void process_quote5();
		void process_time_event();

		void produce_signal_by_spif(typename SPIFPendingQuoteDaoT::QuoteTableRecordT &quotes,int total);
		void produce_signal_by_cf(typename pending_quote_dao<CFQuoteT>::QuoteTableRecordT& quotes,int total);
		void produce_signal_by_stock(typename pending_quote_dao<StockQuoteT>::QuoteTableRecordT quotes,int total);
		void produce_signal_by_full_depth(typename pending_quote_dao<FullDepthQuoteT>::QuoteTableRecordT& quotes,int total);
		void produce_signal_by_quote5(typename PendingQuoteDaoT5::QuoteTableRecordT& quotes,int total);

		void process_sig();

		void produce_signal_by_time_event();

		int get_cancel_sig_count(vector<signal_t> &sigs,int &start, int sig_cnt);
		int get_place_sig_count(vector<signal_t> &sigs,int &start,int &sig_cnt);

		/*
		 * market making
		 */
		void process_quote_notify(tcs *channel);
		void produce_signal_by_quote_notify(vector<T_RtnForQuote> &records, int &count);

		void append(strategy_init_pos_t &dest,strategy_init_pos_t &src);
		
		// license,prevent any nasty usage,added by wangying on 20160805
		void check_lic();

		set<exchange_names> exchanges_;
	};
}

#include "strategy_unitdef.h"
