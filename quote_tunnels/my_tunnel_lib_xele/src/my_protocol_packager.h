#ifndef MY_PROTOCOL_PACKAGER_H_
#define MY_PROTOCOL_PACKAGER_H_

#include "CXeleFtdcUserApiStruct.h"

#include "trade_data_type.h"
#include "my_tunnel_lib.h"
#include "xele_trade_context.h"

// Xele Packer
class XELEPacker
{
public:

    static void OrderRequest(const TunnelConfigData &cfg, const T_PlaceOrder *req,
        OrderRefDataType new_order_ref, CXeleFtdcInputOrderField &insert_order);

    static void OrderRespond(int error_no, SerialNoType serial_no, const char* entrust_no,
        char entrust_status, T_OrderRespond &order_respond);

    static void CancelRequest(const TunnelConfigData &cfg, const T_CancelOrder *req, OrderRefDataType order_ref,
        OrderRefDataType org_order_ref, CXeleFtdcOrderActionField &cancle_order);

    static void CancelRespond(int error_no, SerialNoType serial_no, const char* entrust_no, T_CancelRespond &cancle_order);

    static void OrderReturn(SerialNoType serial_no, const CXeleFtdcOrderField *rsp, T_OrderReturn &order_return);

    static void TradeReturn(const XeleOriginalReqInfo *p_req, const CXeleFtdcTradeField *rsp, T_TradeReturn &trade_return);
};

#endif // MY_PROTOCOL_Packer_H_
