// this type define should share in whole trade system

#ifndef _MY_EXCHANGE_DATATYPE_H_
#define  _MY_EXCHANGE_DATATYPE_H_

#include <unordered_map>
#include <unordered_set>
#include <vector>
//#include "my_tunnel_lib.h"

#define DLL_PUBLIC __attribute__ ((visibility ("default")))

typedef char StockCodeType[64];
typedef long SerialNoType;
typedef long VolumeType;
typedef long EntrustNoType;

///MYExchange 新增的数据类型定义 begin///////////////////////////////////////////////////////////////////////

//struct DLL_PUBLIC T_PositionData
//{
//    mutable int update_flag;    // 仓位是否更新标识; 0:no; 1:yes
//    int long_position;          // 总多仓
//    double long_avg_price;      // average price of long position
//    int short_position;         // 总空仓
//    double short_avg_price;      // average price of short position
//
//    // TODO: Improvement
//   int today_buy_volume; /* 今天的总买量 */
//   double today_aver_price_buy; /* 今天的买平均价格 */
//   int today_sell_volume; /* 今天的总卖量 */
//   double today_aver_price_sell; /* 今天卖平均价格 */
//
//   // TODO: imprvoment
//   char exchange;
//
//    T_PositionData()
//    {
//        update_flag = 0;
//        long_position = 0;
//        long_avg_price = 0;
//        short_position = 0;
//        short_avg_price = 0;
//        today_buy_volume = 0;
//        today_aver_price_buy = 0;
//        today_sell_volume = 0;
//        today_aver_price_sell = 0;
//    }
//};

///MYExchange 新增的数据类型定义 end///////////////////////////////////////////////////////////////////////

//****************业务数据字典项定义***********************//
namespace BUSSINESS_DEF
{
//买卖方向
const char ENTRUST_BUY = '0';   	//买入
const char ENTRUST_SELL = '1';   	//卖出

//开平仓标志
const char ENTRUST_OPEN = '0';  			//开仓
const char ENTRUST_CLOSE = '1';  			//平仓
const char ENTRUST_CLOSETODAY = '2';  		//平今仓
const char ENTRUST_CLOSEYESTERDAY = '3';  	//平昨仓

//委托属性
const char ENTRUST_PROP_LIMIT_PRICE = '0';    	//限价
const char ENTRUST_PROP_MARKET_PRICE = '1';    	//市价，

//投保标志
const char SHFLAG_TOU = '0';  //投机
const char SHFLAG_BAO = '1';  //保值
const char SHFLAG_TAO = '2';  //套利

//报单类型
const char ENTRUSTKIND_NORMAL = '0';  	//普通
const char ENTRUSTKIND_FAK = '1';  		//FAK
const char ENTRUSTKIND_FOK = '2';       //FOK

//委托状态
const char ENTRUST_STATUS_UNREPORT = '0'; 		//未报
const char ENTRUST_STATUS_REPORDED = 'a'; 		//已经报入
const char ENTRUST_STATUS_PARTIALCOM = 'p'; 	//部分成交
const char ENTRUST_STATUS_COMPLETED = 'c'; 		//全部成交
const char ENTRUST_STATUS_WITHDRAWING = 'f'; 	//等待撤除
const char ENTRUST_STATUS_ERROR = 'e'; 			//错误委托
const char ENTRUST_STATUS_WITHDRAWED = 'd'; 	//已经撤销

// added for my_exchange, by cys 20140619
const char ENTRUST_STATUS_INNERMATCHING = 'm'; //内部撮合中

//交易所标志
const char EXCH_CODE_SHFE = 'A';  	//上海期货
const char EXCH_CODE_DCE = 'B';  	//大连期货
const char EXCH_CODE_CZCE = 'C';  	//郑州期货
const char EXCH_CODE_CFFEX = 'G';  	//中金所
const char EXCH_CODE_SZEX = '0';  	//深交所
const char EXCH_CODE_SHEX = '1';  	//上交所
const char EXCH_CODE_NULL = ' ';
const std::string EXCH_NAME_SHFE = "SHFE";
const std::string EXCH_NAME_DCE = "DCE";
const std::string EXCH_NAME_CZCE = "CZCE";
const std::string EXCH_NAME_CFFEX = "CFFEX";
const std::string EXCH_NAME_NULL = " ";

//订单类型
const char ENTRUSTKIND_XJ = '0';  	//限价
const char ENTRUSTKIND_SJ = '1';  	//市价
const char ENTRUSTKIND_ZYJ = '2';  	//最优价
const char ENTRUSTKIND_SJZS = '3';  //市价止损
const char ENTRUSTKIND_SJZY = '4';  //市价止盈
const char ENTRUSTKIND_XJZS = '5';  //限价止损
const char ENTRUSTKIND_XJZY = '6';  //限价止盈
const char ENTRUSTKIND_CD = 'A';  	//撤单

//DFITCOrderAnswerStatusType:委托回报类型
const int SPD_CANCELED = 1; 			///全部撤单
const int SPD_FILLED = 2;				///全部成交
const int SPD_IN_QUEUE = 3;				///未成交还在队列中
const int SPD_PARTIAL = 4;				///部分成交还在队列中
const int SPD_PAPRIAL_CANCELED = 5;		///部成部撤
const int SPD_IN_CANCELING = 6;			///撤单中
const int SPD_ERROR = 7;				///错误(废单错误)
const int SPD_PLACED = 8;				///交易所已接受，但尚未成交
const int SPD_STARTED = 9;				///报单的初始状态，表示单子刚刚开始，尚未报到柜台。
const int SPD_TRIGGERED = 10;			///柜台已接收，但尚未到交易所
const int SPD_SUCCESS_CANCELED = 11;	///撤单成功
const int SPD_SUCCESS_FILLED = 12;		///成交成功

//合约类型
const int TYPE_COMM = 0;   //期货
const int TYPE_OPT = 1;    //期权

//套利策略代码数据类型
const std::string SP = "SP";   			//跨期套利
const std::string SP_SPC = "SPC";  		//两腿跨品种套利
const std::string SP_SPX = "SPX";    	//压榨套利
const std::string SP_CALL = "CSPR"; 	//Call Spread
const std::string SP_PUT = "PSPR";		//Put Spread
const std::string SP_COMBO = "COMBO";	//Combo
const std::string SP_STRADDLE = "STD";  //Straddle
const std::string SP_STRANGLE = "STG";	//Strangle
const std::string SP_GUTS = "GUTS";  	//Guts
const std::string SP_SYNUND = "SYN";	//Synthetic Underlying

//止损标志
const int STOPLOSS_NO = 0;      //不止损
const int STOPLOSS_YES = 1;     //止损

//止盈标志
const int STOPPROFIT_NO = 0;    //不止盈
const int STOPPROFIT_YES = 1;   //止盈

//看涨看跌
const int OPT_LOOK_UP = 1;	    //看涨
const int OPT_LOOK_DOWN = 2;	//看跌

//
const std::string INVALID_STR = "";
const char INVALID_CHAR = ' ';
}

// 功能命令的错误码定义
//namespace TUNNEL_ERR_CODE
//{
//const int RESULT_FAIL = -1;                        //执行功能失败
//const int RESULT_SUCCESS = 0;                      //执行功能成功
//const int UNSUPPORTED_FUNCTION_NO = 1;             //无此功能号
//const int NO_VALID_CONNECT_AVAILABLE = 2;          //交易通道未连接期货商
//const int ERROR_REQUEST = 3;                       //错误的请求指令
//const int CFFEX_EXCEED_LIMIT = 4;                  //股指期货累计开仓超出最大限制
//const int POSSIBLE_SELF_TRADE = 5;                 //可能自成交
//const int CANCEL_TIMES_REACH_WARN_THRETHOLD = 6;   //撤单次数达到告警阈值
//const int CANCEL_REACH_LIMIT = 7;                  //撤单次数达到上限
//
//const int UNSUPPORTED_FUNCTION = 100;              //不支持的功能
//const int NO_PRIVILEGE = 101;                      //无此权限
//const int NO_TRADING_RIGHT = 102;                  //没有报单交易权限
//const int NO_VALID_TRADER_AVAILABLE = 103;         //该交易席位未连接到交易所
//const int MARKET_NOT_OPEN = 104;                   //该席位目前没有处于开盘状态
//const int CFFEX_OVER_REQUEST = 105;                //交易所未处理请求超过许可数
//const int CFFEX_OVER_REQUEST_PER_SECOND = 106;     //交易所每秒发送请求数超过许可数
//const int SETTLEMENT_INFO_NOT_CONFIRMED = 107;     //结算结果未确认
//
//const int INSTRUMENT_NOT_FOUND = 200;              //找不到合约
//const int INSTRUMENT_NOT_TRADING = 201;            //合约不能交易
//const int BAD_FIELD = 202;                         //报单字段有误
//const int BAD_ORDER_ACTION_FIELD = 203;            //错误的报单操作字段
//const int DUPLICATE_ORDER_REF = 204;               //不允许重复报单
//const int DUPLICATE_ORDER_ACTION_REF = 205;        //不允许重复撤单
//const int ORDER_NOT_FOUND = 206;                   //撤单找不到相应报单
//const int UNSUITABLE_ORDER_STATUS = 207;           //当前报单状态不允许撤单
//const int CLOSE_ONLY = 208;                        //只能平仓
//const int OVER_CLOSE_POSITION = 209;               //平仓量超过持仓量
//const int INSUFFICIENT_MONEY = 210;                //资金不足
//const int SHORT_SELL = 211;                        //现货交易不能卖空
//const int OVER_CLOSETODAY_POSITION = 212;          //平今仓位不足
//const int OVER_CLOSEYESTERDAY_POSITION = 213;      //平昨仓位不足
//const int PRICE_OVER_LIMIT = 214;                  //委托价格超出涨跌幅限制
//const int STOCK_CANUSEAMT_NOTENOUGH = 215;         //证券可用数量不足
//
//// added on 20141203, for v2.5.6, control open position
//const int REACH_UPPER_LIMIT_POSITION = 216;         //reach upper limit,forbid open position
//}

#endif
