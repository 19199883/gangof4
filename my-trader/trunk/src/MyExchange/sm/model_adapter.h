#pragma once

#include "../my_exchange.h"
#include <list>
#include <string>
#include "position_entity.h"
#include "quote_entity.h"
#include "signal_entity.h"
#include <boost/shared_ptr.hpp>
#include "model_setting.h"
#include "model_config.h"

#include "model_adapter.h"
#include <dlfcn.h>
#include "moduleloadlibrarylinux.h"
#include "loadlibraryproxy.h"
#include <memory>
#include "position_entity.h"
#include "option_interface.h"

using namespace quote_agent;
using namespace std;
using namespace strategy_manager;

// 模型dll的init_f接口的函数指针（fortran语言）
typedef void (* init_f)(st_config_t *config, int *ret_code);

typedef void (*FeedPositionFAddr)(position_t *PDPosition,position_t *TDPosition);

namespace strategy_manager{
	/*
	* model_adapter，模型适配器。它封装了与模型dll的交互。
	*/
	template<typename SPIFQuoteT,typename CFQuoteT,typename StockQuoteT,
	typename FullDepthQuoteT,typename QuoteT5>
	class model_adapter	{
		typedef void ( *FeedSignalResponseFAddr)(
			signal_resp_t* rpt,
			symbol_pos_t *pos,
			pending_order_t *pending_ord,
			int *sigs_len,
			signal_t* sigs
			);

		// 模型dll的FeedQuote接口的函数指针（fortran语言）
		typedef void ( *FeedSPIFQuoteFAddr)(
			SPIFQuoteT* quote,		/*推送给模型dll的行情数据*/
			int *signals_length,
			signal_t* signals		/*模型dll返回的信号*/
			);		/*specify the size of signals array*/

		// 模型dll的FeedQuote接口的函数指针（fortran语言）
		typedef void ( *FeedCFQuoteFAddr)(
			CFQuoteT* quote,		/*推送给模型dll的行情数据*/
			int *signals_length,
			signal_t* signals		/*模型dll返回的信号*/
			);		/*specify the size of signals array*/

		// 模型dll的FeedQuote接口的函数指针（fortran语言）
		typedef void ( *FeedStockQuoteFAddr)(
			StockQuoteT* quote,		/*推送给模型dll的行情数据*/
			int *signals_length,
			signal_t* signals		/*模型dll返回的信号*/
			);

		// 模型dll的FeedQuote接口的函数指针（fortran语言）
		typedef void (*FeedFullDepthQuoteFAddr)(
			FullDepthQuoteT* quote,			/*推送给模型dll的行情数据*/
			int *signals_length,
			signal_t* signals		/*模型dll返回的信号*/
			);		/*specify the size of signals array*/

		// 模型dll的FeedQuote接口的函数指针（fortran语言）
		typedef void (*FeedMDOrderStatisticFAddr)(
			QuoteT5* quote,			/*推送给模型dll的行情数据*/
			int *signals_length,
			signal_t* signals		/*模型dll返回的信号*/
			);

		typedef void (*FeedTimeEventFAddr)(int *signals_length,signal_t* signals);

		typedef void (*DestroyFAddr)();

		// reserved to use in future
		typedef void (*FeedPositionAddr)(position_t *data, int *sig_cnt, signal_t *sig_out);

		typedef void (*FeedInitPositionAddr)(strategy_init_pos_t *data, int *sig_cnt,signal_t *sig_out);

		typedef void ( *FeedQuoteNotifyFAddr)(T_RtnForQuote* quote_notify,int *sigs_len,signal_t* sigs);

		typedef int ( *InitStrategyFAddr)(startup_init_t *init_content, int *err);

		typedef int (*UpdateStrategyFAddr)(ctrl_t *update_content, int *err);

		typedef int (*GetMonitorInfoFAddr)(monitor_t *monitor_content, int *err);

	public:

		/**
		该模型要交易的合约列表
		*/
		list<string> targets;

		/**
		该字段存储模型的配置信息
		*/
		model_setting setting;

	public:
		/**
		构造函数
		*/
		model_adapter(model_setting _setting);

		/**
		析构函数
		*/
		virtual ~model_adapter(void);

		/*
		初始化模型
		*/
		 void init(T_ContractInfoReturn contractRtn);

		/**
		推送持仓信息给模型，包括：
		（1）MyExchange启动时，推初始持仓.
		（2）在交易阶段，推送实时持仓。
		*/
		 void feed_position(position_t *data, int *sig_cnt, signal_t *sig_out);

		 void feed_init_position(strategy_init_pos_t *data,int *sig_cnt, signal_t *sig_out);

		/**
		*	推送实时行情给模型,模型返回交易信号。
		*/
		void feed_spif_quote(SPIFQuoteT &quote,signal_t* signals,int &signals_size);
		void feed_cf_quote(CFQuoteT &quote,signal_t* signals,int &signals_size);
		void feed_stock_quote(StockQuoteT &quote,signal_t* signals,int &signals_size);
		void feed_full_depth_quote(FullDepthQuoteT &quote,signal_t* signals,int &signals_size);
		void feed_MDOrderStatistic(QuoteT5 &quote,signal_t* signals,int &signals_size);

		void feed_sig_response(signal_resp_t* rpt,symbol_pos_t *pos,
				pending_order_t *pending_ord,signal_t* sigs,int &sigs_len);
		void feed_time_event(signal_t* sigs,int &sigs_len);
		void feed_quote_notify(T_RtnForQuote *quote_notify,signal_t* sigs,int &sigs_len);

		int InitOptionStrategy(startup_init_t *init_content, int *err);
		int UpdateOptionStrategy(ctrl_t *update_content, int *err);
		int GetMonitorInfo(monitor_t *monitor_content, int *err);
	private:

		/*
		* feed configuration to model.
		*/
		 void init_imp(st_config_t *config, int *ret_code);

		/**
		推送持仓信息给模型的具体与模型直接交互的方法
		*/
		 void feed_position_imp(position_t *data, int *sig_cnt, signal_t *sig_out);
		 void feed_init_position_imp(strategy_init_pos_t *data,
				 int *sig_cnt, signal_t *sig_out);

		/**
		推送实时行情给模型的具体与模型直接交互的方法。
		*/
		void feed_spif_quote_imp(SPIFQuoteT &quote_ptr,
				signal_t* signals,int &signals_size);

		void feed_cf_quote_imp(CFQuoteT &quote_ptr,signal_t* signals,int &signals_size);


		void feed_stock_quote_imp(StockQuoteT &quote_ptr,
				signal_t* signals,int &signals_size);

		/**
		推送实时行情给模型的具体与模型直接交互的方法。
		*/
		void feed_full_depth_quote_imp(FullDepthQuoteT &quote_ptr,
				signal_t* signals,int &signals_size);

		void feed_MDOrderStatistic_imp(QuoteT5 &quote_ptr,
				signal_t* signals,int &signals_size);

		void feed_sig_response_imp(signal_resp_t* rpt,symbol_pos_t *pos,
				pending_order_t *pending_ord,signal_t* sigs,int &sigs_len);

		void feed_time_event_imp(signal_t* sigs, int &sigs_len);

		void feed_quote_notify_imp(T_RtnForQuote *quote_notify,signal_t* sigs,int &sigs_len);

		void trace(string title,signal_t* signals,const int &signals_size);
		void trace(pending_order_t *ords);
		void trace(string title,symbol_pos_t *pos);
		void trace(const SPIFQuoteT &data);
		void trace(signal_resp_t* rpt);
		void trace(strategy_init_pos_t *data);
		void trace(const T_RtnForQuote &data);

		string generate_log_name(char * log_path);
		bool match(string &str,map<quote_category_options,set<string>> &rgx_strs);
	public:
		/**
		需要执行如下初始化工作：
		（1）加载dll
		*/
		void initialize(void);

		/**
		需要执行如下终止化工作：
		（1）卸载dll
		*/
		void finalize(void);

	private:

		/**
		该字段存储模型dll init_f的函数指针
		*/
		init_f InitFAddr;

		/**
		该字段存储模型dll FeedQuote的函数指针
		*/
		FeedSPIFQuoteFAddr FeedSPIFQuoteF;
		FeedCFQuoteFAddr FeedCFQuoteF;
		FeedStockQuoteFAddr FeedStockQuoteF;
		FeedFullDepthQuoteFAddr FeedFullDepthQuoteF;
		FeedMDOrderStatisticFAddr FeedStatisticF;

		FeedSignalResponseFAddr FeedSignalResponseF;
		FeedTimeEventFAddr FeedTimeEventF;
		DestroyFAddr DestroyF;
		/**
		该字段存储dll FeedPosition的函数指针
		*/
		FeedPositionFAddr FeedPositionF;
		FeedInitPositionAddr FeedInitPositionF;

		FeedQuoteNotifyFAddr FeedQuoteNotifyF;

		InitStrategyFAddr InitStrategyF;
		UpdateStrategyFAddr UpdateStrategyF;
		GetMonitorInfoFAddr GetMonitorInfoF;

		CLoadLibraryProxy *_pproxy;
	};
}

#include "model_adapterdef.h"
