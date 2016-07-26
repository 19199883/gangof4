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

// ģ��dll��init_f�ӿڵĺ���ָ�루fortran���ԣ�
typedef void (* init_f)(st_config_t *config, int *ret_code);

typedef void (*FeedPositionFAddr)(position_t *PDPosition,position_t *TDPosition);

namespace strategy_manager{
	/*
	* model_adapter��ģ��������������װ����ģ��dll�Ľ�����
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

		// ģ��dll��FeedQuote�ӿڵĺ���ָ�루fortran���ԣ�
		typedef void ( *FeedSPIFQuoteFAddr)(
			SPIFQuoteT* quote,		/*���͸�ģ��dll����������*/
			int *signals_length,
			signal_t* signals		/*ģ��dll���ص��ź�*/
			);		/*specify the size of signals array*/

		// ģ��dll��FeedQuote�ӿڵĺ���ָ�루fortran���ԣ�
		typedef void ( *FeedCFQuoteFAddr)(
			CFQuoteT* quote,		/*���͸�ģ��dll����������*/
			int *signals_length,
			signal_t* signals		/*ģ��dll���ص��ź�*/
			);		/*specify the size of signals array*/

		// ģ��dll��FeedQuote�ӿڵĺ���ָ�루fortran���ԣ�
		typedef void ( *FeedStockQuoteFAddr)(
			StockQuoteT* quote,		/*���͸�ģ��dll����������*/
			int *signals_length,
			signal_t* signals		/*ģ��dll���ص��ź�*/
			);

		// ģ��dll��FeedQuote�ӿڵĺ���ָ�루fortran���ԣ�
		typedef void (*FeedFullDepthQuoteFAddr)(
			FullDepthQuoteT* quote,			/*���͸�ģ��dll����������*/
			int *signals_length,
			signal_t* signals		/*ģ��dll���ص��ź�*/
			);		/*specify the size of signals array*/

		// ģ��dll��FeedQuote�ӿڵĺ���ָ�루fortran���ԣ�
		typedef void (*FeedMDOrderStatisticFAddr)(
			QuoteT5* quote,			/*���͸�ģ��dll����������*/
			int *signals_length,
			signal_t* signals		/*ģ��dll���ص��ź�*/
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
		��ģ��Ҫ���׵ĺ�Լ�б�
		*/
		list<string> targets;

		/**
		���ֶδ洢ģ�͵�������Ϣ
		*/
		model_setting setting;

	public:
		/**
		���캯��
		*/
		model_adapter(model_setting _setting);

		/**
		��������
		*/
		virtual ~model_adapter(void);

		/*
		��ʼ��ģ��
		*/
		 void init(T_ContractInfoReturn contractRtn);

		/**
		���ͳֲ���Ϣ��ģ�ͣ�������
		��1��MyExchange����ʱ���Ƴ�ʼ�ֲ�.
		��2���ڽ��׽׶Σ�����ʵʱ�ֲ֡�
		*/
		 void feed_position(position_t *data, int *sig_cnt, signal_t *sig_out);

		 void feed_init_position(strategy_init_pos_t *data,int *sig_cnt, signal_t *sig_out);

		/**
		*	����ʵʱ�����ģ��,ģ�ͷ��ؽ����źš�
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
		���ͳֲ���Ϣ��ģ�͵ľ�����ģ��ֱ�ӽ����ķ���
		*/
		 void feed_position_imp(position_t *data, int *sig_cnt, signal_t *sig_out);
		 void feed_init_position_imp(strategy_init_pos_t *data,
				 int *sig_cnt, signal_t *sig_out);

		/**
		����ʵʱ�����ģ�͵ľ�����ģ��ֱ�ӽ����ķ�����
		*/
		void feed_spif_quote_imp(SPIFQuoteT &quote_ptr,
				signal_t* signals,int &signals_size);

		void feed_cf_quote_imp(CFQuoteT &quote_ptr,signal_t* signals,int &signals_size);


		void feed_stock_quote_imp(StockQuoteT &quote_ptr,
				signal_t* signals,int &signals_size);

		/**
		����ʵʱ�����ģ�͵ľ�����ģ��ֱ�ӽ����ķ�����
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
		��Ҫִ�����³�ʼ��������
		��1������dll
		*/
		void initialize(void);

		/**
		��Ҫִ��������ֹ��������
		��1��ж��dll
		*/
		void finalize(void);

	private:

		/**
		���ֶδ洢ģ��dll init_f�ĺ���ָ��
		*/
		init_f InitFAddr;

		/**
		���ֶδ洢ģ��dll FeedQuote�ĺ���ָ��
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
		���ֶδ洢dll FeedPosition�ĺ���ָ��
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
