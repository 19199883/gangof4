#pragma once

#include <list>
#include <map>
#include <boost/function.hpp>
#include <boost/function_equal.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/thread_time.hpp>
#include "tca_setting.h"
#include "tcs.h"
#include <vector>
#include "my_order.h"
#include <set>
#include <condition_variable> // std::condition_variable
#include "../my_exchange.h"
#include "tbb/include/tbb/spin_mutex.h"

using namespace std;

namespace trading_channel_agent
{
	typedef pair<long,exchange_names> TcsTableKeyT;
	typedef map<TcsTableKeyT,tcs*> TcsTableT;
	/**
	tcaʱtrading channel agent����д������һ������tca������Ϊte�ṩһ���򵥡�ͳһ�ķ��ʽӿ�,��Ҫ���ܰ�����
	(1) ·��ί��\����������ȷ��ͨ��
	(2) ���ͻر�֪ͨ��������
	(3) �ṩ�ر�������Ĳ�ѯ����
	(4) ����te���͵�ί��\��������
	*/
	class tca
	{
	public:
		/**
		��������������ͬ��������ղ���������Ĵ������.
		*/
		condition_variable cond_for_request;
		static speculative_spin_mutex mut_for_request;
		static speculative_spin_mutex mu_for_request_counter;

		map<pair<long,exchange_names>,tcs*> tcs_map;
		// tcs�б�
		list<tcs*> sources;
	private:
		// ���ֶδ洢qa�����ö���
		tca_setting setting;

		

		// �����������ÿ��ϵͳ�����󶼴�0��ʼ
		static long request_counter;

		vector<int> _split_cache;
	public:
		tca(void);
		virtual ~tca(void);
		void initialize(void);
		void finalize(void);

		/**
		��ȡָ��ģ������δ��ɵ�ί��\��������
		*/
		void get_not_final_requests(const long &model_id,vector<my_order> &ords,int &count);

		vector<tcs*> get_tcs(const long &model_id);

		/**
		����ģ��id���ź�id���Լ�ȫ��˳��ã�����clordid
		*/
		static long generate_cl_ord_id(const long& model_id , const long& signal_id);
		void place_orders(my_order &ord,set<long> &new_ords);

		/**
		���س��������cl_ord_id
		*/
		void cancel_ord(my_order &ord);

		void register_strategy(const long &model_id, const set<exchange_names> &exchange);

		/*
		 * market making
		 */
		void req_quote(my_order &ord);
		void quote(QuoteOrder &ord);
		void cancel_quote(QuoteOrder &ord);
		tcs* get_tcs(const long &model_id,const exchange_names &exchange);

	private:

		tcs* get_tcs(const pair<long,exchange_names> &key);
		int split(string symbol,int ori_vol,vector<int> &split_vols);
	};
}

