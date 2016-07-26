#ifndef HS_UTIL_H_
#define HS_UTIL_H_

#include <string>
#include "CITICs_HsT2Hlp.h"

namespace HSUtil
{
// 证券交易统一接口的参数信息
//登陆 331100
const int func_id_login = 331100;
//查股东信息 331300
const int func_id_qry_stockholder = 331300;
//客户资金精确查询 333612
const int func_client_fund_all_qry = 333612;
//查证券持仓 333104
const int func_id_qry_stock_position = 333620;
//期权持仓查询 338023
const int func_id_qry_option_position = 338023;
//委托 333002
const int func_id_placeorder = 333002;
//委托撤单 333017
const int func_id_cancelorder = 333017;
//查持仓   333620
const int func_secu_stock_qry = 333620;
//成交查询 333102
const int func_secu_realtime_qry = 333102;
//订单查询 333100
const int func_secu_en_with_ent_qry = 333100;



//查询委托 333101
const int func_id_qry_order = 333101;
//期权可交易数据获取 338010
const int func_id_qry_option_data = 338010;

//期权撤单 338012
const int func_id_cancelorder_option = 338012;
// 期权委托查询
const int func_id_ary_option = 338020;
//订阅成交回报 620001 (股票,期权)
const int func_id_subscribe_match_rtn = 620001;
//订阅取消-证券成交回报主推 620002 (股票,期权)
const int func_id_cancelsubscribe_match_rtn = 620002;

//打印返回值。如果遇到和文档不一致的情况，以测试结果为准。
void ShowOneRowData(const std::string &desp, HSHLPHANDLE HlpHandle,int iRow);
void ShowAnsData(const std::string &desp, HSHLPHANDLE HlpHandle);
void ShowAnsData(const std::string &desp, HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl);
void ShowOneRowData(const std::string &desp, HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl,int iRow);
void ShowErrMsg(HSHLPHANDLE HlpHandle, int iFunc, int nErr);

}
;

#endif
