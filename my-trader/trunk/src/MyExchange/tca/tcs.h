#pragma once

#include <functional>     // std::ref
#include <thread>         // std::thread
#include <future>         // std::promise, std::future
#include <cstdlib>
#include <list>
#include <map>
#include <functional>   // std::bind
#include <thread>
#include "tcs_setting.h"
#include "my_order.h"
#include "pending_ord_request_dao.h"
#include <condition_variable> // std::condition_variable
#include <mutex>
#include <vector>
#include "../my_exchange.h"
#include "MyExchangeAgent.h"
#include "tbb/include/tbb/spin_mutex.h"
#include <mutex>          // std::mutex, std::lock_guard

using namespace std;
//using namespace tbb;

namespace trading_channel_agent
{
	/**
	tcs使trading channel source的简称。该类封装了与指定通道的访问。主要功能包括：
	(1) 接收到回报后通知给te
	(2) 保存回报数据
	*/
	class tcs
	{
	private:
		enum Original_rpt_type_options
		{
			ord_response = 1,
			ord_return = 2,
			trade_return = 3,
			cancel_response = 4,
			rep_of_req_quote = 5,
			quote_reponse = 6,
			quote_return = 7,
			quote_trade_return = 8,
			cancel_quote_response = 9,

		};
	private:
		/*
		 * the following fields track report form counter system
		 */
		vector<T_OrderRespond> ord_response_cache;
		vector<T_PositionData> ord_response_pos_cache;
		int ord_response_counter;
		bool ord_res_ready;

		vector<T_CancelRespond> cancel_response_cache;
		int cancel_response_counter;
		bool cancel_response_ready;

		vector<T_OrderReturn> ord_rtn_cache;
		vector<T_PositionData> ord_rtn_pos_cache;
		int ord_rtn_counter;
		bool ord_rtn_ready;

		vector<T_TradeReturn> trde_rtn_cache;
		vector<T_PositionData> trade_rtn_pos_cache;
		int trde_rtn_counter;
		bool trde_rtn_ready;

		/*
		 * the following fields caches reports from counter system
		 */
		vector<T_RspOfReqForQuote> req_quote_response_cache;
		int req_quote_counter;
		bool req_quote_ready;

		vector<T_InsertQuoteRespond> quote_response_cache;
		vector<T_PositionData> quote_response_pos_cache;
		int quote_response_counter;
		bool quote_response_ready;

		vector<T_QuoteReturn> quote_rtn_cache;
		vector<T_PositionData> quote_rtn_pos_cache;
		int quote_rtn_counter;
		bool quote_rtn_ready;

		vector<T_QuoteTrade> quote_trade_rtn_cache;
		vector<T_PositionData> quote_trade_rtn_pos_cache;
		int quote_trade_rtn_counter;
		bool quote_trade_rtn_ready;

		vector<T_CancelQuoteRespond> cancel_quote_response_cache;
		int cancel_quote_response_counter;
		bool cancel_quote_response_ready;

		vector<my_order> to_dispatch_ord_cache;
		int to_dispatch_ord_counter;

		vector<Original_rpt_type_options> ori_rpt_sort_table;
		int ori_rpt_sort_table_counter;

		/**
		该字段记录tcs模块当前状态，如果为true，则表示该模块处于非工作状态
		*/
		bool stopped;

		std::promise<T_OrderDetailReturn> query_ords_prom_;
		std::promise<T_TradeDetailReturn> query_trans_prom_;
		std::promise<T_PositionReturn> query_pos_prom_;
		std::promise<T_ContractInfoReturn> query_contracts_prom_;
		speculative_spin_mutex sync_query_lock_;

	public:
		MYExchangeInterface *channel;

		/**
		该线程负责从交易通道接收行情数据
		*/
		thread process_report_thread;

		// used to process report in tca module
		condition_variable ori_rpt_ready_cv;
		mutex ori_rpt_ready_mtx;
		condition_variable rpt_notify_ready_cv;
		mutex rpt_notify_ready_mtx;
		speculative_spin_mutex quote_notify_ready_mtx;
		map<long,bool> quote_notify_state_;

		ReportNotifyStateT report_notify_state;

		/**
		该字段存储tcs的配置信息
		*/
		tcs_setting setting;

		/*
		 * 0,normal;
		 * others,error
		 */
		int status_;

//		tcs_v_operator tcs_v_op_;

	public:
		tcs(tcs_setting setting);
		~tcs(void);
		void initialize(void);
		void finalize(void);

		/**
		发送委托\撤单请求到交易通道系统。如果：
		（1）成功，则标记其已执行
		（2）不成功，保持未执行状态，但是记录出错原因(to improve:以后可能会增加对未发送成功的请求的处理模式)
		     出错原因采用追加方式，且记录每次的时间戳
		*/
		void place_request(const my_order &ord);

		void cancel_request(const my_order &ord);

		void req_quote(const my_order &ord);
		void quote(const QuoteOrder &ord);
		void cancel_quote(const QuoteOrder &ord);

		void query_orders_result_handler(const T_OrderDetailReturn *result);
		bool is_final(OrderDetail &ord);
		/*
		 * query all transactions of this day.
		 */
		T_TradeDetailReturn query_transactions();
		void query_transactions_result_handler(const T_TradeDetailReturn *transaction);

		/*
		 * query the current position.
		 */
		T_PositionReturn query_position(void);
		void query_position_result_handler(const T_PositionReturn *pos);

		/*
		 * market making.
		 * receives response of request quote
		 */
		void rev_rep_of_req_quote(const T_RspOfReqForQuote *pos);

		/*
		 * market making.
		 * receives response of request quote
		 */
		void rev_rtn_for_quote(const T_RtnForQuote *quote_notify);

		/*
		 * market making.
		 * quote
		 */
		void rev_rep_of_quote(const T_InsertQuoteRespond *, const T_PositionData *);
		void rev_rtn_of_quote(const T_QuoteReturn *, const T_PositionData *);
		void rev_trade_rtn_of_quote(const T_QuoteTrade *, const T_PositionData *);
		void rev_rep_of_cancel_quote(const T_CancelQuoteRespond *);


		/*
		 * market making.
		 * processes response of request quote
		 */
		void process_rep_of_req_quote(const int &cur);
		void process_quote_response(const int &cur);

		state_options convert_from(char state);
		void trace(const string &invoker,const my_order &ord);
		void trace(const string &invoker,const QuoteOrder &ord);

		void init(const long &model);
		T_ContractInfoReturn query_ContractInfo();

//		string get_semkey_for_strategyunit(long model);
//		string get_semkey_for_cancel_processor(long model);

	private:

		// 该函数负责从交易通道接收回报数据并进行处理
		void process_reports(void);

		void rev_ord_response(const T_OrderRespond *, const T_PositionData *);
		void rev_cancel_ord_response(const T_CancelRespond *);
		void rev_ord_return(const T_OrderReturn *, const T_PositionData * pos);
		void rev_trade_return(const T_TradeReturn *, const T_PositionData * pos);

		void process_ord_response(const int &cur);
		void process_cancel_response(const int &cur);
		void process_ord_rtn(const int &cur);
		void process_trade_rtn(const int &cur);
		void process_quote_return(const int &cur);
		void process_quote_trade_return(const int &cur);
		void process_cancel_quote_response(const int &cur);

		void cancel_orders();
		bool is_normal();

		void query_ContractInfo_result_handler(const T_ContractInfoReturn *contracts);
	};
}
