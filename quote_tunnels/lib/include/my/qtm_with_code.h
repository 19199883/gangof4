#pragma once

#include "qtm.h"
#include <stdio.h>
#include <string.h>

enum QtmState {
	INIT = 0,
	SET_ADDRADRESS_SUCCESS = 1,
	CONNECT_SUCCESS = 2,
	LOG_ON_SUCCESS = 3,
	QUERY_ACCOUNT_SUCCESS = 4,
	QUERY_POSITION_SUCCESS = 5,
	QUERY_ORDER_SUCCESS = 6,
	QUERY_TRADE_SUCCESS = 7,
	QUERY_CONTRACT_SUCCESS = 8,
	QUERY_CANCEL_TIMES_SUCCESS = 9,
	HANDLED_UNTERMINATED_ORDERS_SUCCESS = 10,
	
	API_READY = 10001,
	
	UNDEFINED_API_ERROR = -10000,

	SET_ADDRADRESS_FAIL = -1,
	CONNECT_FAIL = -2,
	LOG_ON_FAIL = -3,
	QUERY_ACCOUNT_FAIL = -4,
	QUERY_CONTRACT_FAIL = -5,
	QUERY_ORDER_FAIL = -6,

	QUOTE_INIT_FAIL = -11,
	CREATE_UDP_FD_FAIL = -12,

	DISCONNECT = -21,
	LOG_OUT = -22,

	TUNNEL_NOT_READY = -100,
	REQUEST_ORDER_ERROR = -101,
	NOT_SUPPORT_FUNCTION = -102,
	UNAUTHORIZED = -103,
	ORDER_UNAUTHORIZED = -104,
	NOT_CONNECT_TO_EXCHANGE = -105,
	OUT_OF_TRADING_TIME = -106,
	UNHANDLED_REQUESTS_EXCEED_LIMIT = -107,
	REQUESTS_PER_SECOND_EXCEED_LIMIT = -108,
	SETTLEMENT_INFO_UNCONFIRMED = -109,
	CONTRACT_CAN_NOT_FIND = -110,
	CONTRACT_CAN_NOT_TRADE = -111,
	ORDER_FIELD_ERROR = -112,
	IMPLEMENT_ORDER_FIELD_ERROR = -113,
	ORDER_REPEATEDLY = -114,
	CANCEL_REPEATEDLY = -115,
	CAN_NOT_FIND_ENTRUST_NO = -116,
	CAN_NOT_CANCEL_WITH_CURRENT_STATE = -117,
	OFFSET_SQUARE_LIQUIDATE_PERMITTED_ONLY = -118,
	CLOSE_POSITION_TOO_LARGER = -119,
	NOT_ENOUGH_MONEY = -120,
	NOT_ALLOW_SHORT_SELLING = -121,
	TODAY_POSITION_NOT_ENOUGH = -122,
	YESTERDAY_POSITION_NOT_ENOUGH = -123,
	ORDER_PRICE_EXCEED_MAXIMUM_PRICE_FLUCTUATUIONS = -124,
	SHARES_NOT_ENOUGH = -125,
	ORDER_VOLUME_UNPERMITTED = -126,
};

enum QtmComplianceState {
	INIT_COMPLIANCE_CHECK = 1,
	OPEN_POSITIONS_EXCEED_LIMITS = -11,
	OPEN_POSITIONS_EQUAL_LIMITS = -12,

	SELF_TRADE = -21,

	CANCEL_TIME_OVER_WARNING_THRESHOLD = -31,
	CANCEL_TIME_OVER_MAXIMUN = -32,
	CANCEL_TIME_EQUAL_WARNING_THRESHOLD = -33
};

static std::string GetDescriptionWithState(const int state) {
	switch (state) {
	case QtmState::INIT:
		return "Init";
	case QtmState::SET_ADDRADRESS_SUCCESS:
		return "Set addr adress success";
	case QtmState::CONNECT_SUCCESS:
		return "Connect success";
	case QtmState::LOG_ON_SUCCESS:
		return "Log on success";
	case QtmState::QUERY_ACCOUNT_SUCCESS:
		return "Query account success";
    case QtmState::QUERY_POSITION_SUCCESS:
        return "Query position success";
    case QtmState::QUERY_ORDER_SUCCESS:
        return "Query order detail success";
    case QtmState::QUERY_TRADE_SUCCESS:
        return "Query trade detail success";
	case QtmState::QUERY_CONTRACT_SUCCESS:
		return "Query contract info success";
	case QtmState::QUERY_CANCEL_TIMES_SUCCESS:
		return "Query cancel times success";
	case QtmState::HANDLED_UNTERMINATED_ORDERS_SUCCESS:
		return "Handled unterminated orders success";
	case QtmState::API_READY:
		return "API Ready";

	case QtmState::SET_ADDRADRESS_FAIL:
		return "Set addr adress fail";
	case QtmState::CONNECT_FAIL:
		return "Connect fail";
	case QtmState::LOG_ON_FAIL:
		return "Log on fail";
	case QtmState::QUERY_ACCOUNT_FAIL:
		return "Query account fail";
	case QtmState::QUERY_CONTRACT_FAIL:
		return "Query contract fail";

	case QtmState::QUOTE_INIT_FAIL:
		return "Quote init fail";
	case QtmState::CREATE_UDP_FD_FAIL:
	    return "Create udp fd fail";

	case QtmState::DISCONNECT:
		return "Disconnected";
	case QtmState::LOG_OUT:
		return "Log out";

	case QtmState::TUNNEL_NOT_READY:
		return "Tunnel not yet connect to broker";
	case QtmState::REQUEST_ORDER_ERROR:
		return "Request order error";
	case QtmState::NOT_SUPPORT_FUNCTION:
		return "The function requested is not supported";
	case QtmState::UNAUTHORIZED:
		return "Unauthorized";
	case QtmState::ORDER_UNAUTHORIZED:
		return "Trade order unauthorized";
	case QtmState::NOT_CONNECT_TO_EXCHANGE:
		return "SEAT not yet connect to Exchange";
	case QtmState::OUT_OF_TRADING_TIME:
		return "SEAT is out of the time of trading";
	case QtmState::UNHANDLED_REQUESTS_EXCEED_LIMIT:
		return "Unhandled requests exceed permitted number limit";
	case QtmState::REQUESTS_PER_SECOND_EXCEED_LIMIT:
		return "Requests per second exceed permitted number limit";
	case QtmState::SETTLEMENT_INFO_UNCONFIRMED:
		return "Settlement info unconfirmed";
	case QtmState::CONTRACT_CAN_NOT_FIND:
		return "Find contract fail";
	case QtmState::CONTRACT_CAN_NOT_TRADE:
		return "Contract can not be traded";
	case QtmState::ORDER_FIELD_ERROR:
		return "Order field error";
	case QtmState::IMPLEMENT_ORDER_FIELD_ERROR:
		return "implement Order field error";
	case QtmState::ORDER_REPEATEDLY:
		return "Order repeatedly";
	case QtmState::CANCEL_REPEATEDLY:
		return "Cancel repeatedly";
	case QtmState::CAN_NOT_FIND_ENTRUST_NO:
		return "Can not find order info according to entrust no within cancel info";
	case QtmState::CAN_NOT_CANCEL_WITH_CURRENT_STATE:
		return "Can not cancel in current status of order";
	case QtmState::OFFSET_SQUARE_LIQUIDATE_PERMITTED_ONLY:
		return "Offset Square Liquidate permitted only";
	case QtmState::CLOSE_POSITION_TOO_LARGER:
		return "Close positions are larger than open positions";
	case QtmState::NOT_ENOUGH_MONEY:
		return "Money is not enough";
	case QtmState::NOT_ALLOW_SHORT_SELLING:
		return "Short-selling is not allowed on cash market";
	case QtmState::TODAY_POSITION_NOT_ENOUGH:
		return "Today positions are not enough";
	case QtmState::YESTERDAY_POSITION_NOT_ENOUGH:
		return "Yesterday positions are not enough";
	case QtmState::ORDER_PRICE_EXCEED_MAXIMUM_PRICE_FLUCTUATUIONS:
		return "Order price exceed maximum price fluctuations";
	case QtmState::SHARES_NOT_ENOUGH:
		return "Shares are not enough";
	case QtmState::ORDER_VOLUME_UNPERMITTED:
		return "Order volume unpermitted";
	default:
		return "unknown state";
	}
}

static std::string GetComplianceDescriptionWithState(const int state, const char* account, const char* contract) {
	char err_msg[1024];
	memset(err_msg, 0, strlen(err_msg));
	switch (state) {
	case QtmComplianceState::INIT_COMPLIANCE_CHECK:
		sprintf(err_msg, "%s_%s: Init compliance check", account, contract);
		break;
	case QtmComplianceState::OPEN_POSITIONS_EXCEED_LIMITS:
		sprintf(err_msg, "%s_%s: Open positions exceed limits", account, contract);
		break;
	case QtmComplianceState::OPEN_POSITIONS_EQUAL_LIMITS:
		sprintf(err_msg, "%s_%s: Open positions equal limits", account, contract);
		break;
	case QtmComplianceState::SELF_TRADE:
		sprintf(err_msg, "%s_%s: Maybe trade self, insert fail", account, contract);
		break;
	case QtmComplianceState::CANCEL_TIME_OVER_WARNING_THRESHOLD:
		sprintf(err_msg, "%s_%s: Cancel times approach warning threshold, open limited", account, contract);
		break;
	case QtmComplianceState::CANCEL_TIME_OVER_MAXIMUN:
		sprintf(err_msg, "%s_%s: Cancel times approach maximum", account, contract);
		break;
	case QtmComplianceState::CANCEL_TIME_EQUAL_WARNING_THRESHOLD:
		sprintf(err_msg, "%s_%s: Cancel time equal warning threshold", account, contract);
		break;
	default:
		sprintf(err_msg, "%s_%s: UnknownState", account, contract);
	}
	return err_msg;
}
