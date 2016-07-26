#pragma once

#include "../my_exchange.h"
#include <string>
#include "exchange_names.h"

#include "my_trade_tunnel_api.h"


using namespace quote_agent;
using namespace std;

namespace trading_channel_agent
{
	enum report_types
	{
		new_ord_rpt = '0',

		cancel_rpt = '1',

		/*
		 * market maker, request quote
		 */
		req_quote_rpt = '9',
	};

	enum side_options
	{
		buy = '0',

		sell = '1',

		undefined1 = 'u',
	};

	/**
		锟斤拷1锟斤拷锟斤拷锟缴诧拷锟斤拷状态锟角诧拷锟斤拷要锟斤拷锟节的ｏ拷
		锟斤拷2锟斤拷rejected状态锟斤拷last_qty锟斤拷值锟斤拷
	*/
	enum state_options
	{
		// 未锟斤拷(n:锟饺达拷锟�; s:锟斤拷锟斤拷锟疥报)
		pending_new = '0',

		// 锟窖撅拷锟斤拷锟斤拷
		new_state = 'a',

		// 锟斤拷锟街成斤拷
		partial_filled = 'p',

		// 全锟斤拷锟缴斤拷
		filled = 'c',

		// 锟饺达拷锟�
		pending_cancel = 'f',

		// 锟斤拷锟节拒撅拷(e:锟斤拷锟斤拷委锟斤拷)
		rejected = 'q',

		// 锟窖撅拷锟斤拷锟斤拷(b:锟斤拷锟缴诧拷锟斤拷)
		cancelled = 'd',

		undefined2 = 'u'
	};

	enum position_effect_options
	{
		//	锟斤拷锟斤拷
		open_ = '0',

		// 平锟斤拷
		close_pos = '1',

		// 平锟斤拷锟�
		rolled = '2',

		// 	平锟斤拷锟�
		close_yesterday = '3',

		undefined = 'u'

	};

	enum price_options
	{
		/*
		锟睫硷拷
		*/
		limit = '0',

		/*
		锟叫硷拷
		*/
		market = '1'

	};

	enum request_types
	{
		place_order		= '0',
		cancel_order	= '1',
		req_quote		= '2',
		quote		= '3',
	};

	enum ord_types
	{
		general = '0',
		fak = '1',
		fok = '2',
	};

	enum sah_options
	{
		speculate 	= '0',
		hedge 		= '1',
		arbitrage 	= '2',
	};

	/*
	锟斤拷枚锟劫讹拷锟藉交锟斤拷通锟斤拷锟斤拷氐拇锟斤拷锟斤拷锟较�
	*/
	enum channel_errors
	{
		// 执锟叫癸拷锟斤拷失锟斤拷
		RESULT_FAIL = -1,

		// 	执锟叫癸拷锟杰成癸拷
		RESULT_SUCCESS = 0,

		// 锟睫此癸拷锟杰猴拷
		UNSUPPORTED_FUNCTION_NO = 1,

		// 锟斤拷锟斤拷通锟斤拷未锟斤拷锟斤拷锟节伙拷锟斤拷
		NO_VALID_CONNECT_AVAILABLE = 2,

		// 锟斤拷锟斤拷锟斤拷锟斤拷锟街革拷锟�
		ERROR_REQUEST = 3,

		// 锟斤拷指锟节伙拷锟桔计匡拷锟街筹拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
		CFFEX_EXCEED_LIMIT = 4,

		// 锟斤拷支锟街的癸拷锟斤拷
		UNSUPPORTED_FUNCTION = 100,

		// 锟睫达拷权锟斤拷
		NO_PRIVILEGE = 101,

		// 没锟叫憋拷锟斤拷锟斤拷锟斤拷权锟斤拷
		NO_TRADING_RIGHT = 102,

		// 锟矫斤拷锟斤拷席位未锟斤拷锟接碉拷锟斤拷锟斤拷锟斤拷
		NO_VALID_TRADER_AVAILABLE = 103,

		// 锟斤拷席位目前没锟叫达拷锟节匡拷锟斤拷状态
		MARKET_NOT_OPEN = 104,

		// 锟斤拷锟斤拷锟斤拷未锟斤拷锟斤拷锟斤拷锟襟超癸拷锟斤拷锟斤拷锟�
		CFFEX_OVER_REQUEST = 105,

		// 锟斤拷锟斤拷锟斤拷每锟诫发锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
		CFFEX_OVER_REQUEST_PER_SECOND = 106,

		// 锟斤拷锟斤拷锟斤拷未确锟斤拷
		SETTLEMENT_INFO_NOT_CONFIRMED = 107,

		// 锟揭诧拷锟斤拷锟斤拷约
		INSTRUMENT_NOT_FOUND = 200,

		// 锟斤拷约锟斤拷锟杰斤拷锟斤拷
		INSTRUMENT_NOT_TRADING = 201,

		// 锟斤拷锟斤拷锟街讹拷锟斤拷锟斤拷
		BAD_FIELD = 202,

		// 锟斤拷锟斤拷谋锟斤拷锟斤拷锟斤拷锟斤拷侄锟�
		BAD_ORDER_ACTION_FIELD = 203,

		// 锟斤拷锟斤拷锟斤拷锟截革拷锟斤拷锟斤拷
		DUPLICATE_ORDER_REF = 204,

		// 锟斤拷锟斤拷锟斤拷锟截革拷锟斤拷锟斤拷
		DUPLICATE_ORDER_ACTION_REF = 205,

		// 锟斤拷锟斤拷锟揭诧拷锟斤拷锟斤拷应锟斤拷锟斤拷
		ORDER_NOT_FOUND = 206,

		// 锟斤拷前锟斤拷锟斤拷状态锟斤拷锟斤拷锟�?锟斤拷
		UNSUITABLE_ORDER_STATUS = 207,

		// 只锟斤拷平锟斤拷
		CLOSE_ONLY = 208,

		// 平锟斤拷锟斤拷锟斤拷锟斤拷植锟斤拷锟�
		OVER_CLOSE_POSITION = 209,

		// 锟绞斤拷锟斤拷
		INSUFFICIENT_MONEY = 210,

		// 锟街伙拷锟斤拷锟阶诧拷锟斤拷锟斤拷锟斤拷
		SHORT_SELL = 211,

		// 平锟斤拷锟轿伙拷锟斤拷锟�
		OVER_CLOSETODAY_POSITION = 212,

		// 平锟斤拷锟轿伙拷锟斤拷锟�
		OVER_CLOSEYESTERDAY_POSITION = 213,

		// 委锟叫价格超筹拷锟角碉拷锟斤拷锟斤拷锟�
		PRICE_OVER_LIMIT = 214,

	};

	class my_order
	{
	public:
		my_order()
		{
			request_type = request_types::place_order;
			exchange = exchange_names::undefined;
			cl_ord_id = 0;
			ord_id = 0;
			model_id = 0;
			signal_id = 0;
			is_final = false;
			symbol = "";
			price = 0.0;
			volume = 0;
			side = side_options::undefined1;
			position_effect = position_effect_options::undefined;
			ord_type = ord_types::general;
			last_px = 0.0;
			last_qty = 0;
			error_no = channel_errors::RESULT_SUCCESS;
			price_type = price_options::limit;
			last_px = 0;
			last_qty = 0;
			this->state = state_options::undefined2;
			orig_cl_ord_id = 0;
			orig_ord_id = 0;

			cum_qty = 0;
			cum_amount = 0.0;
			sah_ = sah_options::speculate;
		}

	public:
		long cl_ord_id;

		/**
		委锟斤拷\锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷通锟斤拷锟斤拷傻锟轿ㄒ伙拷锟绞�
		*/
		long ord_id;

		/**
		模锟斤拷ID锟斤拷锟斤拷锟节憋拷识锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥革拷模锟酵诧拷锟斤拷锟�
		*/
		long model_id;

		/**
		锟脚猴拷ID锟斤拷锟斤拷锟节憋拷识锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥革拷锟脚号诧拷锟斤拷锟�
		*/
		long signal_id;

		/**
		锟斤拷锟街段硷拷录锟斤拷锟斤拷锟斤拷锟斤拷息
		*/
		exchange_names exchange;

		/**
		锟斤拷锟街段硷拷录锟斤拷锟斤拷锟斤拷锟酵ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟轿拷锟斤拷锟斤拷锟�
		*/
		request_types request_type;

		/**
		锟斤拷示锟斤拷锟斤拷锟角凤拷锟斤拷锟斤拷锟�
		*/
		bool is_final;

		state_options state;
		/**
		锟斤拷锟街段硷拷录锟斤拷约
		*/
		string symbol;

		/**
		锟斤拷锟街段硷拷录委锟斤拷锟斤拷鄹锟�
		*/
		double price;

		/**
		锟斤拷锟街段硷拷录委锟斤拷锟斤拷
		*/
		int volume;

		/**
		锟斤拷锟街段硷拷录锟斤拷锟斤拷锟斤拷锟斤拷
		*/
		side_options side;

		/**
		锟斤拷锟街段硷拷录锟斤拷平锟街诧拷锟斤拷
		*/
		position_effect_options position_effect;

		// 委锟叫凤拷式
		ord_types ord_type;

		/**
		锟斤拷锟街段硷拷录锟斤拷锟铰成斤拷锟桔革拷
		*/
		double last_px;

		/**
		锟斤拷锟街段硷拷录锟斤拷锟铰成斤拷锟斤拷
		*/
		int last_qty;

		int cum_qty;
		double cum_amount;

		channel_errors error_no;

		price_options price_type;
		long orig_cl_ord_id;
		long orig_ord_id;
		sah_options sah_;

		T_PositionData position_;
	};

	class QuoteOrder
	{
	public:
		QuoteOrder()
		{
			model_id = 0;
			signal_id = 0;
			cl_ord_id = 0;
			request_type = request_types::place_order;
			buy_is_final = false;
			sell_is_final = false;
			buy_state = state_options ::undefined2;
			sell_state = state_options ::undefined2;
			ord_id = 0;
			stock_code = "";
			buy_limit_price = 0.0;
			buy_volume = 0;
			buy_open_close = -1;            // '0': open; '1': close
			buy_speculator = -1;            // '0': speculation; '1': hedge; '2':arbitrage;
			sell_limit_price = 0.0;
			sell_volume = 0;
			sell_open_close = -1;
			sell_speculator = -1;
			memset(for_quote_id,0,21);
			sah_ = sah_options::speculate;
			orig_cl_ord_id = 0;
			orig_ord_id = 0;

			buy_cum_qty = 0;
			buy_cum_amount = 0.0;
			sell_cum_qty = 0;
			sell_cum_amount = 0.0;
			buy_last_px = 0.0;
			buy_last_qty = 0;
			sell_last_px = 0.0;
			sell_last_qty = 0;
			error_no = channel_errors::RESULT_SUCCESS;
			exchange = exchange_names::undefined;
		}
		/**
		模锟斤拷ID锟斤拷锟斤拷锟节憋拷识锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥革拷模锟酵诧拷锟斤拷锟�
		*/
		long model_id;

		/**
		锟脚猴拷ID锟斤拷锟斤拷锟节憋拷识锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥革拷锟脚号诧拷锟斤拷锟�
		*/
		long signal_id;

		long cl_ord_id;

		/**
		锟斤拷锟街段硷拷录锟斤拷锟斤拷锟斤拷锟酵ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟轿拷锟斤拷锟斤拷锟�
		*/
		request_types request_type;

		/**
		锟斤拷示锟斤拷锟斤拷锟角凤拷锟斤拷锟斤拷锟�
		*/
		bool buy_is_final;
		bool sell_is_final;

		state_options buy_state;
		state_options sell_state;

		/**
		委锟斤拷\锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷通锟斤拷锟斤拷傻锟轿ㄒ伙拷锟绞�
		*/
		long ord_id;

	    string stock_code;   // 锟斤拷约锟斤拷锟斤拷

	    double buy_limit_price;         // 锟桔革拷
	    VolumeType buy_volume;          // 锟斤拷锟斤拷
	    char buy_open_close;            // '0': open; '1': close
	    char buy_speculator;            // '0': speculation; '1': hedge; '2':arbitrage;
	    double buy_last_px;
	    int buy_last_qty;
	    int buy_cum_qty;
	    double buy_cum_amount;

	    double sell_limit_price;
	    VolumeType sell_volume;
	    char sell_open_close;
	    char sell_speculator;
	    double sell_last_px;
		int sell_last_qty;
		int sell_cum_qty;
		double sell_cum_amount;

	    char for_quote_id[21]; ///应锟桔憋拷锟�
	    sah_options sah_;
	    channel_errors error_no;
	    exchange_names exchange;
	    long orig_cl_ord_id;
	    long orig_ord_id;
	};
}
