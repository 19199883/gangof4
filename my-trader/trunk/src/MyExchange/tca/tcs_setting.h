#pragma once

#include <string>
#include <set>
#include "exchange_names.h"
#include "my_trade_tunnel_api.h"

using namespace std;
using namespace quote_agent;

namespace trading_channel_agent
{
	enum channel_type_options
	{
		ctp = 1,
		dce = 2,
		femas = 3,
		zeusing = 4,
		citics_uft = 5,
		ks_option = 6,
		ib = 7,
		esunny = 8,
		kgi = 9,
		lts_option = 10,
	};

	/**
	���ඨ��tcs�����á��磺
	(1) �뽻��ͨ�����ӵ�ip��port
	(2) ��ͨ�����Դ����ĸ���������ί�е�
	*/
	class tcs_setting
	{
	public:
		tcs_setting(void);
		~tcs_setting(void);

		string name;

		int id;

		/**
		���ֶα�ʾ��ͨ�����Դ�����ĸ���������ί�е�
		*/
		set<exchange_names> exchanges;

		/*
		 * model id list, it represents these models place orders by this channel.
		 */
		set<long> models;

		channel_type_options channel_type;

		// channel configuration file to load
		string config;
		string so_file;
		string tunnel_so_file;

		struct my_xchg_cfg my_xchg_cfg_;
	};
}
