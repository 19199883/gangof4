#pragma once

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include "position_entity.h"
#include "tca.h"
#include <memory>
#include "position_entity.h"

#include "my_trade_tunnel_api.h"

//#include "my_tunnel_lib.h"

#include "position_entity.h"
#include "my_order.h"
#include "position_entity.h"

using namespace std;
using namespace strategy_manager;
using namespace trading_channel_agent;

namespace trading_engine{
	/**
	通锟斤拷直锟接诧拷询锟斤拷菘锟斤拷取锟斤拷锟秸成斤拷锟斤拷息锟斤拷锟斤拷锟斤拷锟绞硷拷植趾喜锟斤拷锟斤拷佣锟矫碉拷实时锟街诧拷锟斤拷息锟斤拷
	锟斤拷锟斤拷每锟斤拷模锟酵ｏ拷锟斤拷锟斤拷锟斤拷锟揭伙拷锟斤拷锟斤拷锟绞碉拷锟斤拷锟斤拷锟斤拷锟侥ｏ拷徒锟斤拷椎暮锟皆硷拷锟绞凳憋拷植锟斤拷锟较拷锟�
	TO IMPROVE:
		锟斤拷1锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷细锟斤拷锟角ｏ拷锟斤拷锟斤拷系统锟斤拷锟杰ｏ拷锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷菘锟�
	*/
	class position_manager{
	private:
		/**
		模锟斤拷ID
		*/
		long model_id;
		model_setting model_setting_ins_;

		shared_ptr<tca> tca_ptr;
		ReportNotifyRecordT _report_record_cache;

		ReportNotifyRecordT _quote_report_record_cache;

		vector<order_request_t> pending_vol_cache;
		int pending_vol_cache_size;

	private:
		void assign(strategy_init_pos_t &dest,T_PositionReturn &src,const exchange_names &ex);
		void assign(strategy_init_pos_t &dest,T_TradeDetailReturn &src,const exchange_names &ex);

		void sum (acc_volume_t &sum, map<side_options,list<TradeDetail>> &addend);

		bool is_subscribed_symbol(string symbol);
		void fill_today_traded_info(position_t today_pos,acc_volume_t *today_acc_volume,int count);

	public:
		/**
		锟斤拷锟届函锟斤拷
		*/
		position_manager(shared_ptr<tca> tca_ptr, model_setting model_setting_ins,const long &model_id);

		virtual ~position_manager(void);

		/**
		锟斤拷取指锟斤拷锟斤拷约锟斤拷锟斤拷锟铰的持诧拷锟斤拷息
		*/
		void get_position(st_config_t &config,strategy_init_pos_t &ini_pos,const exchange_names &exchange);

		void get_pending_vol(const string &symbol,const exchange_names &exchange,pending_order_t &table);

		T_ContractInfoReturn query_ContractInfo(st_config_t *config, exchange_names ex,int *ret_code);
	};
}

#include "position_managerdef.h"
