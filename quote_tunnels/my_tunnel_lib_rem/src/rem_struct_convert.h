#ifndef REM_STRUCT_CONVERT_H_
#define REM_STRUCT_CONVERT_H_

#include "EesTraderDefine.h"
#include "my_trade_tunnel_struct.h"
#include "rem_trade_context.h"

namespace RemStructConvert
{
    void OrderRespond(int err, long sn, long en_no, char entrust_status, T_OrderRespond &rsp);
    void CancelRespond(int err, long sn, long en_no, T_CancelRespond &rsp);
    void OrderReturn(const RemReqInfo *req, T_OrderReturn &order_return);
    void TradeReturn(const RemReqInfo *req, const EES_OrderExecutionField* match_info, T_TradeReturn &trade_return);

    void ReqOrderInsert(const T_PlaceOrder *p, EES_ClientToken tok, const char *acc, EES_EnterOrderField &req);
};

#endif
