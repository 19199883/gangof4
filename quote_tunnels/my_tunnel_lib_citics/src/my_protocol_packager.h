#ifndef MY_PROTOCOL_PACKAGER_H_
#define MY_PROTOCOL_PACKAGER_H_

#include "trade_data_type.h"
#include "my_trade_tunnel_api.h"
#include "CITICs_HsT2Hlp.h"
#include "my_tunnel_lib.h"

class CITTCsUFTPacker
{
public:
	static void OrderRespond(const int error_no, const long serial_no,
			long entrust_no,const char utf_status, T_OrderRespond &order_respond,char ex);

	static void CancelRespond(const int error_no, const long serial_no,
			long entrust_no, const char internal_status,T_CancelRespond &cancle_order);


	static void OrderReturn(const T_InternalOrder *req,T_OrderReturn &order_return,char ex);


	static void TradeReturn(const T_InternalOrder *rsp,T_TradeReturn &trade_return,char ex);
};

#endif // MY_PROTOCOL_Packer_H_
