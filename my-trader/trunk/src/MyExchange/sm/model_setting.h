#pragma once

#include <string>
#include <map>
#include "position_entity.h"
#include "model_setting.h"
#include "quotecategoryoptions.h"
#include "model_config.h"
#include "my_order.h"

using namespace std;

namespace strategy_manager{
	/**
	model_setting定义模型配置信息。
	*/
	class model_setting	{
	public:
		// the configuration to tell model
		st_config_t config_;

		string init_method;
		string feed_init_pos_method;
		string feed_pos_method;
		string feed_spif_quote_method;
		string feed_cf_quote_method;
		string feed_stock_quote_method;
		string feed_full_depth_quote_method;
		string MDOrderStatistic_method;
		string feed_sig_response_method;
		string feed_time_event_method;
		string feed_quote_notify_method;

		// option
		string init_option_strategy;
		string update_option_strategy;
		string get_monitor_info;
		bool isOption;

		/*
		 * T29
		 * this function check whether this strategy is valid
		 */
		bool is_valid(){
			return id < 100000000;
		}

		string destroy_method;
		string account;
		string name;
		sah_options sah_;
		int id;
		/*
		 * model file's relative path,include file name.
		 */
		string file;

		/*
		 * a time interval in microseconds after which rejected order will be re-sent.
		 */
		int cancel_timeout;

		/*
		 * key:quote_category_options
		 * value: symbols
		 */
		map<quote_category_options,set<string>> subscription;

	public:

		model_setting(void){}

		~model_setting(void){}
	};
}
