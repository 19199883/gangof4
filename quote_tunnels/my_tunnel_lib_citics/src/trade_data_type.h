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

struct QuoteInfoInTnnl
{
    long serial_no;                 // 下单流水号
    int front_id;
    int session_id;
    long entrust_no;
    std::string cancel_serial_no;
    std::string Cancel_serial_no() const
    {
        std::size_t pos = cancel_serial_no.find(",");
        if (pos != std::string::npos)
        {
            return cancel_serial_no.substr(0, pos);
        }

        return cancel_serial_no;
    }

    QuoteInfoInTnnl(const int s_no, int f_id, int s_id)
        : serial_no(s_no),
          front_id(f_id),
          session_id(s_id),
          entrust_no(0)
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



//****************业务数据字典项定义***********************//
namespace FUNC_CONST_DEF
{
//买卖方向
const char CONST_ENTRUST_BUY = '0';   //买入
const char CONST_ENTRUST_SELL = '1';   //卖出

//开平仓标志
const char CONST_ENTRUST_OPEN = '0';  //开仓
const char CONST_ENTRUST_CLOSE = '1';  //平仓
const char CONST_ENTRUST_CLOSETODAY = '2';  //平今仓
const char CONST_ENTRUST_CLOSEYESTERDAY = '3';  //平昨仓

//委托属性
const char CONST_ENTRUSTPROP_BS = '0';    //限价
const char CONST_ENTRUSTPROP_MARKET_PRICE = '1';    //市价，

//投保标志
const char CONST_SHFLAG_TOU = '0';  //投机
const char CONST_SHFLAG_BAO = '1';  //保值
const char CONST_SHFLAG_TAO = '2';  //套利
const char CONST_SHFLAG_MarketMaker = '3';  //做市商

//合约类型
const int CONST_TYPE_COMM = 0;   //期货
const int CONST_TYPE_OPT = 1;   //期权

//套利策略代码数据类型
const std::string CONST_SP = "SP";     //跨期套利
const std::string CONST_SP_SPC = "SPC";    //两腿跨品种套利
const std::string CONST_SP_SPX = "SPX";    //压榨套利
const std::string CONST_SP_CALL = "CSPR";   //Call Spread
const std::string CONST_SP_PUT = "PSPR";   //Put Spread
const std::string CONST_SP_COMBO = "COMBO";  //Combo
const std::string CONST_SP_STRADDLE = "STD";    //Straddle
const std::string CONST_SP_STRANGLE = "STG";    //Strangle
const std::string CONST_SP_GUTS = "GUTS";   //Guts
const std::string CONST_SP_SYNUND = "SYN";    //Synthetic Underlying

//报单类型
const char CONST_ENTRUSTKIND_NORMAL = '0';  //普通
const char CONST_ENTRUSTKIND_FAK = '1';  //FAK
const char CONST_ENTRUSTKIND_FOK = '2';  //FOK

//成交属性
const char CONST_TRADEATTR_INTODAY = '0'; //当日有效
const char CONST_TRADEATTR_ALLDEAL = '1'; //全成或全撤
const char CONST_TRADEATTR_AUTOWITHDRAW = '2'; //剩余即撤销

//委托状态
const char CONST_ENTRUST_STATUS_UNREPORT = '0'; //未报
const char CONST_ENTRUST_STATUS_WAITING = 'n'; //等待发出
const char CONST_ENTRUST_STATUS_REPORTING = 's'; //正在申报
const char CONST_ENTRUST_STATUS_REPORDED = 'a'; //已经报入
const char CONST_ENTRUST_STATUS_PARTIALCOM = 'p'; //部分成交
const char CONST_ENTRUST_STATUS_COMPLETED = 'c'; //全部成交
const char CONST_ENTRUST_STATUS_WITHDRAWING = 'f'; //等待撤除
const char CONST_ENTRUST_STATUS_ERROR = 'e'; //错误委托
const char CONST_ENTRUST_STATUS_DISABLE = 'q'; //场内拒绝
const char CONST_ENTRUST_STATUS_WITHDRAWED = 'd'; //已经撤销
const char CONST_ENTRUST_STATUS_PARTIALWD = 'b'; //部成部撤

//DFITCOrderAnswerStatusType:委托回报类型
const int CONST_SPD_CANCELED = 1;    ///全部撤单
const int CONST_SPD_FILLED = 2;    ///全部成交
const int CONST_SPD_IN_QUEUE = 3;    ///未成交还在队列中
const int CONST_SPD_PARTIAL = 4;    ///部分成交还在队列中
const int CONST_SPD_PAPRIAL_CANCELED = 5;    ///部成部撤
const int CONST_SPD_IN_CANCELING = 6;    ///撤单中
const int CONST_SPD_ERROR = 7;    ///错误(废单错误)
const int CONST_SPD_PLACED = 8;    ///交易所已接受，但尚未成交
const int CONST_SPD_STARTED = 9;    ///报单的初始状态，表示单子刚刚开始，尚未报到柜台。
const int CONST_SPD_TRIGGERED = 10;   ///柜台已接收，但尚未到交易所
const int CONST_SPD_SUCCESS_CANCELED = 11;   ///撤单成功
const int CONST_SPD_SUCCESS_FILLED = 12;   ///成交成功

//止损标志
const int CONST_STOPLOSS_NO = 0;     //不止损
const int CONST_STOPLOSS_YES = 1;     //止损

//止盈标志
const int CONST_STOPPROFIT_NO = 0;   //不止盈
const int CONST_STOPPROFIT_YES = 1;  //止盈

//交易所标志
const char CONST_EXCHCODE_SHFE = 'A';  //上海期货
const char CONST_EXCHCODE_DCE = 'B';  //大连期货
const char CONST_EXCHCODE_CZCE = 'C';  //郑州期货
const char CONST_EXCHCODE_CFFEX = 'G';  //中金所
const char CONST_EXCHCODE_SZEX = '0';  //深交所
const char CONST_EXCHCODE_SHEX = '1';  //上交所
const char CONST_EXCHCODE_NULL = ' ';
const std::string CONST_EXCH_NAME_SHFE = "SHFE";  //上海期货
const std::string CONST_EXCH_NAME_DCE = "DCE";   //大连期货
const std::string CONST_EXCH_NAME_CZCE = "CZCE";  //郑州期货
const std::string CONST_EXCH_NAME_CFFEX = "CFFEX"; //中金所
const std::string CONST_EXCH_NAME_NULL = " ";

//看涨看跌
const int CONST_OPT_LOOK_UP = 1;    //看涨
const int CONST_OPT_LOOK_DOWN = 2;    //看跌

//订单类型
const char CONST_ENTRUSTKIND_XJ = '0';  //限价
const char CONST_ENTRUSTKIND_SJ = '1';  //市价
const char CONST_ENTRUSTKIND_ZYJ = '2';  //最优价
const char CONST_ENTRUSTKIND_SJZS = '3';  //市价止损
const char CONST_ENTRUSTKIND_SJZY = '4';  //市价止盈
const char CONST_ENTRUSTKIND_XJZS = '5';  //限价止损
const char CONST_ENTRUSTKIND_XJZY = '6';  //限价止盈
const char CONST_ENTRUSTKIND_CD = 'A';  //撤单

//
const std::string CONST_INVALID_STR = "";
const char CONST_INVALID_CHAR = ' ';
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
