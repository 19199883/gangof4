#pragma once

#include <boost/thread.hpp>
#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <set>
#include "my_order.h"
#include <map>
#include <list>
#include <vector>
#include "tbb/include/tbb/spin_mutex.h"
#include <unordered_map>
#include <unordered_set>
#include "../my_exchange.h"

#include "my_trade_tunnel_api.h"

using namespace std;
using namespace tbb;

namespace trading_channel_agent
{
	/*
	 * key,model id;value,cancel_signal_processor or position_correct_signal
	 */
	typedef pair<long,string> ReportNotifyTableKeyT;

	typedef vector<my_order> ReportNotifyRecordT;

	typedef vector<QuoteOrder>QuoteOrderRecordT;
	/*
	 * pair<long,string>:key,model id;value,cancel_signal_processor or position_correct_signal
	 */
	typedef map<ReportNotifyTableKeyT, ReportNotifyRecordT> ReportNotifyTableT;

	/*
	 *
	 * value:true,ready;false,not ready
	 */
	typedef map<ReportNotifyTableKeyT,bool> ReportNotifyStateT;


	/*
	 * store all orders for this day including both final and non-final orders.
	 */
	typedef vector<my_order> OrderDatabaseT;

	/*
	 * a mapping table.
	 * using this table, you may find the position which the order is at
	 * in OrderDatabase by cl_ord_id.
	 * the table only stores index of non-final orders.
	 * key: cl_ord_id of my_order object
	 * value: the position which my_order object is at in OrderDatabaseT object.
	 */
	typedef unordered_map<long,size_t> OrderDatabaseClOrdIdIndexTableT;

	/*
	 * a mapping table.
	 * using this table, you can find the positions which all non-final orders are at
	 * in OrderDatabase for the specified model id.
	 * key: model id
	 * value:a set object which stores positions of orders in OrderDatabase.
	 */
	typedef unordered_map<long,unordered_set<size_t> > OrderDatabaseModelIndexTableT;

	/*
	 * a mapping table.
	 * using this table, you can find the positions which all non-final orders are at
	 * in OrderDatabase for the specified symbol.
	 * key: symbol
	 * value: a set object which stores positions of orders in OrderDatabase.
	 */
	typedef unordered_map<string,unordered_set<size_t> > OrderDatabaseSymbolTableT;

	/*
	 * market making
	 */
	/*
	 * store all orders for this day including both final and non-final orders.
	 */
	typedef vector<QuoteOrder> QuoteOrderDatabaseT;

	/*
	 * a mapping table.
	 * using this table, you may find the position which the order is at
	 * in OrderDatabase by cl_ord_id.
	 * the table only stores index of non-final orders.
	 * key: cl_ord_id of my_order object
	 * value: the position which my_order object is at in OrderDatabaseT object.
	 */
	typedef unordered_map<long,size_t> QuoteOrderDatabaseClOrdIdIndexTableT;

	/*
	 * a mapping table.
	 * using this table, you can find the positions which all non-final orders are at
	 * in OrderDatabase for the specified model id.
	 * key: model id
	 * value:a set object which stores positions of orders in OrderDatabase.
	 */
	typedef unordered_map<long,unordered_set<size_t> > QuoteOrderDatabaseModelIndexTableT;

	/*
	 * a mapping table.
	 * using this table, you can find the positions which all non-final orders are at
	 * in OrderDatabase for the specified symbol.
	 * key: symbol
	 * value: a set object which stores positions of orders in OrderDatabase.
	 */
	typedef unordered_map<string,unordered_set<size_t> > QuoteOrderDatabaseSymbolTableT;

	/*
	 * key: model id
	 * value:symbol
	 */
	typedef pair<long,string> PositionTableKeyT;
	typedef map<PositionTableKeyT,int> PositionTableT;

	/**
	pending_ord_request_dao��װ��pending_ord_request_table��ݵķ���
	*/
	class pending_ord_request_dao
	{
	public:
		// TODO: wangying 2017-02-17
		static int const ORDER_DATABASE_MAX_SIZE = 30000;
	private:
		static set<long> cl_ord_id_cache;
		static speculative_spin_mutex lock_local_request_cache;

		/*
		 * the following fields cache reports to notify to te.
		 * key:model id
		 * value:my_order list
		 */
		static ReportNotifyTableT rpt_notify_cache;
		static speculative_spin_mutex lock_rpt_notify_cache;

		// market making, quote
		static map<long,vector<T_RtnForQuote> > quote_notify_cache;
		static speculative_spin_mutex lock_quote_notify_cache;
		static map<long,int > cache_quote_notify_count;

		/*
		 * value:record count
		 */
		static map<ReportNotifyTableKeyT,int> cache_record_count;

		static OrderDatabaseT OrderDatabase;
		static OrderDatabaseClOrdIdIndexTableT OrderDatabaseClOrdIdIndexTable;
		static OrderDatabaseModelIndexTableT OrderDatabaseModelIndexTable;
		static OrderDatabaseSymbolTableT OrderDatabaseSymbolTable;
		static speculative_spin_mutex mtx_for_OrderDatabase;
		static size_t order_database_index;

		static QuoteOrderDatabaseT QuoteOrderDatabase;
		static QuoteOrderDatabaseClOrdIdIndexTableT QuoteOrderDatabaseClOrdIdIndexTable;
		static QuoteOrderDatabaseModelIndexTableT QuoteOrderDatabaseModelIndexTable;
		static QuoteOrderDatabaseSymbolTableT QuoteOrderDatabaseSymbolTable;
		static speculative_spin_mutex mtx_for_QuoteOrderDatabase;
		static size_t quote_order_database_index;

	public:
		/**
		* �ڱ������һ��ί������
		* local_ord:
		* 	true,the order was placed by current myexchange;
		* 	false,the order was placed other myexchange
		*/
		static void insert_request(const my_order &ord);

		/*
		 * cahce reports to notify te
		 */
		static void cache(const my_order &ord);
		static void clear_cache(const ReportNotifyTableKeyT &key);

		/*
		 * market making, quote request
		 */
		static void cache(const T_RtnForQuote &ord,map<long,bool> &state);
		static void clear_quote_notify_cache(const long &modle_id);
		static vector<T_RtnForQuote>& read_quote_notify(
				const long &key,int &count,vector<T_RtnForQuote>& result);

		static void update_ord(const long &cl_ord_id,const long &ord_id,const state_options &status,
				const bool &is_final,const channel_errors &err);

		/**
		��ѯָ��ģ�ͷ��͵�ί����������δ��������״̬��ί������
		*/
		static void query_not_final_ord_requests(const long& model_id,ReportNotifyRecordT &ords,int &count);

		static void query_not_final_ord_requests_by_symbol(const string &symbol,
				ReportNotifyRecordT &ords,int &count);

		/**
		�ж��Ƿ����ָ��cl_ord_id������
		*/
		static bool exist(const long &cl_ord_id);
		static void query_request(const long& cl_ord_id, my_order &ord);

		/*
		����ָ��ί�е��ĳɽ���Ϣ
		*/
		static void update_filled_info(const long &cl_ord_id,const double &px,
				const int &qty,const bool &is_final);


		static void init(const ReportNotifyTableKeyT &key);

		static ReportNotifyRecordT& read_report_notify(const ReportNotifyTableKeyT &key,
				int &count,ReportNotifyRecordT& result);

		static void reset(my_order &ord);
		static void init(const long& model_id,set<string> &symbols);

		// market making
		static set<long> cl_quote_ord_id_cache;
		static void insert_quote_order(const QuoteOrder &ord);
		static void query_quote_order(const long& cl_ord_id, QuoteOrder &ord);
		static bool exist_quote_order(const long &cl_ord_id);
		static void update_quote_order(const side_options &side,const long &cl_ord_id,
						const long &ord_id,const state_options &status,const bool &is_final,
						const channel_errors &err);
		static void update_quote_fill(side_options &side,const long& cl_ord_id, const double &px, const int &qty,const bool &is_final);
		static void reset_quote_order(QuoteOrder &ord);
		static void query_not_final_quote_order_by_symbol(const string &symbol,ReportNotifyRecordT &ords, int &count);
		static void query_not_final_quote_order(const long &model_id,QuoteOrderRecordT &ords, int &total);
	};
}
