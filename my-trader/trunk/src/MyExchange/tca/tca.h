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
	tca时trading channel agent的缩写。它与一个或多个tca交互，为te提供一个简单、统一的访问接口,主要功能包括：
	(1) 路由委托\撤单请求到正确的通道
	(2) 发送回报通知给订阅者
	(3) 提供回报、请求的查询操作
	(4) 处理te发送的委托\撤单请求
	*/
	class tca
	{
	public:
		/**
		该条件变量负责同步请求接收操作与请求的处理操作.
		*/
		condition_variable cond_for_request;
		static speculative_spin_mutex mut_for_request;
		static speculative_spin_mutex mu_for_request_counter;

		map<pair<long,exchange_names>,tcs*> tcs_map;
		// tcs列表
		list<tcs*> sources;
	private:
		// 该字段存储qa的配置对象
		tca_setting setting;

		

		// 请求计数器，每次系统启动后都从0开始
		static long request_counter;

		vector<int> _split_cache;
	public:
		tca(void);
		virtual ~tca(void);
		void initialize(void);
		void finalize(void);

		/**
		获取指定模型所有未完成的委托\撤单请求
		*/
		void get_not_final_requests(const long &model_id,vector<my_order> &ords,int &count);

		vector<tcs*> get_tcs(const long &model_id);

		/**
		根据模型id、信号id，以及全局顺序好，产生clordid
		*/
		static long generate_cl_ord_id(const long& model_id , const long& signal_id);
		void place_orders(my_order &ord,set<long> &new_ords);

		/**
		返回撤单请求的cl_ord_id
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

