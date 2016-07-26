#ifndef MYTRADEDATATYPE_H_
#define MYTRADEDATATYPE_H_

#include "my_trade_tunnel_data_type.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

typedef long long OrderRefDataType;
typedef long EntrustNoType;

struct OriginalReqInfo
{
    long serial_no;                 // 下单流水号
    int front_id;                   // 前置机 id
    int session_id;                 // session id
    char exchange_code;             // 交易所代码
    char buy_sell_flag;             // 买卖标志
    char hedge_type;                // 投机套保标识
    char open_close_flag;           // 开平标识
    char order_type;                // 报单类型；0：普通；1：FAK
    int order_volum;               // 委托数量
    char order_Kind;               // 委托类型
    std::string stock_code;         // 合约代码
    std::string cancel_serial_no;   // 撤单流水号

public:
    std::string Cancel_serial_no() const
    {
        std::size_t pos = cancel_serial_no.find(",");
        if (pos != std::string::npos)
        {
            return cancel_serial_no.substr(0, pos);
        }

        return cancel_serial_no;
    }
    OriginalReqInfo(const long s_no,
        int f_id, int s_id,
        char exch_code, char buy_sell_flg, char hedge_flag, char oc_flag, char order_t,
        int o_vlm, char o_kind, const std::string &s_code, const std::string &s_cancel_no)
        : serial_no(s_no),
            front_id(f_id),
            session_id(s_id),
            exchange_code(exch_code),
            buy_sell_flag(buy_sell_flg),
            hedge_type(hedge_flag),
            open_close_flag(oc_flag),
            order_type(order_t),
            order_volum(o_vlm), order_Kind(o_kind), stock_code(s_code), cancel_serial_no(s_cancel_no)
    {
    }
};

typedef std::unordered_map<OrderRefDataType, OriginalReqInfo> OrderRefToRequestInfoMap;
typedef std::unordered_map<long, OrderRefDataType> SerialNoToOrderRefMap;
typedef std::unordered_map<OrderRefDataType, long> OrderRefToSerialNoMap;
typedef std::unordered_set<long> SerialNoSet;
typedef std::unordered_map<long, long> SerialNoToOrderSysIDMap;
typedef std::unordered_set<OrderRefDataType> AnsweredOrderRefSet;

typedef OrderRefToRequestInfoMap::iterator OrderRefToRequestInfoMapIt;
typedef OrderRefToRequestInfoMap::const_iterator OrderRefToRequestInfoMapCit;
typedef OrderRefToRequestInfoMap::value_type OrderRefToRequestInfoMapValueType;

typedef SerialNoToOrderRefMap::iterator SerialNoToOrderRefMapIt;
typedef SerialNoToOrderRefMap::const_iterator SerialNoToOrderRefMapCit;
typedef SerialNoToOrderRefMap::value_type SerialNoToOrderRefMapValueType;

// 子系统内的常量定义
namespace TUNNEL_CONST_VAR
{
// 配置文件名
const std::string APP_CFG_FILE = "my_trade_tunnel.xml";
const std::string ERR_CFG_FILE_CTP = "error_code_ctp.xml";
const std::string ERR_CFG_FILE_DCE = "error_code_xspeed.xml";
const std::string ERR_CFG_FILE_FEMAS = "error_code_femas.xml";
const std::string ERR_CFG_FILE_ZEUSING = "error_code_zeusing.xml";
const std::string ERR_CFG_FILE_KSG = "error_code_ksg.xml";
const std::string ERR_CFG_FILE_CITICS_HS = "error_code_citics_hs.xml";
const std::string ERR_CFG_FILE_ESUNNY = "error_code_esunny.xml";
const std::string ERR_CFG_FILE_XELE = "error_code_xele.xml";
const std::string ERR_CFG_FILE_REM = "error_code_rem.xml";
const std::string ERR_CFG_FILE_IB_API = "error_code_ib_api.xml";
const std::string ERR_CFG_FILE_LTS = "error_code_lts.xml";
const std::string ERR_CFG_FILE_SGIT = "error_code_sgit.xml";

// 未知错误码和错误信息
const int UNKNOWN_ERROR_NO = -1;
const std::string UNKNOWN_ERROR_INFO = "unknown error.";

//浮点数零
const double CONST_FLOAT_ZERO = 1E-8;
}

struct OrderStatusInTunnel
{
    char status;
    int volume;
    int matched_volume;

    OrderStatusInTunnel(int v)
        : status(MY_TNL_OS_UNREPORT),
            volume(v),
            matched_volume(0)
    {
    }
};

enum StatusCheckResult
{
    none = 0,
    abandon,
    fillup_rsp,
    fillup_rtn_p,
    fillup_rtn_c,
    fillup_rsp_rtn_p,
    fillup_rsp_rtn_c,
};

enum FemasRespondType
{
    order_respond = 0,
    order_return,
    trade_return,
};

typedef std::unordered_map<OrderRefDataType, OrderStatusInTunnel> OrderRefToOrderStatusMap;
typedef OrderRefToOrderStatusMap::iterator OrderRefToOrderStatusMapIt;

#endif // MYTRADEDATATYPE_H_
