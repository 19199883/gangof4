#include "trade_log_util.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include <syslog.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>

#include <iomanip>
#include <sstream>
#include <list>
#include <mutex>

#include "my_trade_tunnel_struct.h"
#include "tunnel_cmn_utility.h"
#include "share_mem.h"

using namespace std;

enum OrderTypes
{
    ePlace_Order = 1,
    eOrder_Respond,
    eOrder_Return,
    eTrade_Return,
    eCancel_Order,
    eCancel_Respond,
};

struct OrderInfoInTunnel
{
    union
    {
        T_PlaceOrder pl_ord;
        T_OrderRespond res_ord;
        T_OrderReturn ret_ord;
        T_TradeReturn ret_tra;
        T_CancelOrder can_ord;
        T_CancelRespond can_res;
    } ord;

    int ord_type;
    long time;

    OrderInfoInTunnel(int type, long t, const void *pdata, size_t data_size)
        : ord_type(type), time(t)
    {
        memcpy(&ord, pdata, data_size);
    }
};
static std::mutex log_sync;
static volatile bool s_run_flag = true;
static volatile bool s_init_flag = false;
char *LogUtil::shm_addr = NULL;

// data buffers
static OrderAndTimes place_orders;
static CancelAndTimes cancel_orders;
static OrderRspAndTimes order_responds;
static CancelRspAndTimes cancel_responds;
static OrderRtnAndTimes order_returns;
static TradeRtnAndTimes trade_returns;

static QryPositionAndTimes qry_position;
static PositionRtnAndTimes pos_rtn;
static QryOrderDetailAndTimes qry_order_detail;
static OrderDetailRtnAndTimes order_detail_rtn;
static QryTradeDetailAndTimes qry_trade_detail;
static TradeDetailRtnAndTimes trade_detail_rtn;

static QryContractInfoAndTimes qry_contract;
static ContractInfoRtnAndTimes contract_rtn;

static ReqForQuoteAndTimes req_forquote;       			//  询价请求      模型 -> 通道
static InsertQuoteAndTimes insert_quote;        		//  报价请求      模型 -> 通道
static CancelQuoteAndTimes cancel_quote;        		//  撤销报价      模型 -> 通道
static RspOfReqForQuoteAndTimes rsp_req_forquote;   	//  询价响应      模型 <- 通道
static RtnForQuoteAndTimes rtn_forquote;        		//  询价通知      模型 <- 通道
static InsertQuoteRespondAndTimes insert_quote_rsp; 	//  报价应答      模型 <- 通道
static CancelQuoteRespondAndTimes cancel_quote_rsp; 	//  撤销报价应答  模型 <- 通道
static QuoteReturnAndTimes quote_rtn;        			//  报价状态回报  模型 <- 通道
static QuoteTradeAndTimes quote_trade_rtn;         		//  报价成交回报  模型 <- 通道

// serialize functions
static std::string ToFormatedDatetime(long long t)
{
    char tm_buff[32];
    long int loc_t_sec = t / 1000000000;
    struct tm localtime_result;
    strftime(tm_buff, 32, "%Y-%m-%d %T", localtime_r(&loc_t_sec, &localtime_result));
    return tm_buff;
}

static std::string ToString(long long t, const T_PlaceOrder *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[PlaceOrder] ";
    ss << "serial_no: " << p->serial_no << "; ";
    ss << "stock_code: " << p->stock_code << "; ";
    ss << "limit_price: " << p->limit_price << "; ";
    ss << "direction: " << p->direction << "; ";
    ss << "open_close: " << p->open_close << "; ";
    ss << "speculator: " << p->speculator << "; ";
    ss << "volume: " << p->volume << "; ";
    ss << "order_kind: " << p->order_kind << "; ";
    ss << "order_type: " << p->order_type << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_CancelOrder *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[CancelOrder] ";
    ss << "serial_no: " << p->org_serial_no << "; ";
    ss << "stock_code: " << p->stock_code << "; ";
    ss << "limit_price: " << p->limit_price << "; ";
    ss << "direction: " << p->direction << "; ";
    ss << "open_close: " << p->open_close << "; ";
    ss << "speculator: " << p->speculator << "; ";
    ss << "volume: " << p->volume << "; ";
    ss << "entrust_no: " << p->entrust_no << "; ";
    ss << "cancel_serial_no: " << p->serial_no << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_OrderRespond *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[OrderRespond] ";
    ss << "serial_no: " << p->serial_no << "; ";
    ss << "error_no: " << p->error_no << "; ";
    ss << "entrust_no: " << p->entrust_no << "; ";
    ss << "entrust_status: " << p->entrust_status << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_CancelRespond *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[CancelRespond] ";
    ss << "cancel_serial_no: " << p->serial_no << "; ";
    ss << "error_no: " << p->error_no << "; ";
    ss << "entrust_no: " << p->entrust_no << "; ";
    ss << "entrust_status: " << p->entrust_status << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_OrderReturn *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[OrderReturn] ";
    ss << "serial_no: " << p->serial_no << "; ";
    ss << "stock_code: " << p->stock_code << "; ";
    ss << "limit_price: " << p->limit_price << "; ";
    ss << "direction: " << p->direction << "; ";
    ss << "open_close: " << p->open_close << "; ";
    ss << "speculator: " << p->speculator << "; ";
    ss << "volume: " << p->volume << "; ";
    ss << "entrust_no: " << p->entrust_no << "; ";
    ss << "entrust_status: " << p->entrust_status << "; ";

    ss << "volume_remain: " << p->volume_remain << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_TradeReturn *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[TradeReturn] ";
    ss << "serial_no: " << p->serial_no << "; ";
    ss << "entrust_no: " << p->entrust_no << "; ";
    ss << "business_volume: " << p->business_volume << "; ";
    ss << "business_price: " << p->business_price << "; ";
    ss << "business_no: " << p->business_no << "; ";

    ss << "stock_code: " << p->stock_code << "; ";
    ss << "direction: " << p->direction << "; ";
    ss << "open_close: " << p->open_close << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_QryPosition *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[QryPosition] ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_PositionReturn *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream sst;
    sst << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - " << "[PositionReturn] ";
    std::string prefix = sst.str();

    stringstream ss;
    ss << fixed << setprecision(4);
    ss << prefix << "error_no: " << p->error_no << "; ";
    ss << "account_no: " << acc << "; ";
    for (const PositionDetail &v : p->datas)
    {
        ss << std::endl << prefix;
        ss << "code: " << v.stock_code << "; ";
        ss << "dir: " << v.direction << "; ";
        ss << "pos: " << v.position << "; ";
        ss << "avg_price: " << v.position_avg_price << "; ";
        ss << "yd_pos: " << v.yestoday_position << "; ";
        ss << "yd_avg_price: " << v.yd_position_avg_price << "; ";
        ss << "exchange: " << v.exchange_type << "; ";
        ss << "account_no: " << acc << "; ";
    }

    return ss.str();
}

static std::string ToString(long long t, const T_QryOrderDetail *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[QryOrderDetail] ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_OrderDetailReturn *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream sst;
    sst << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - " << "[OrderDetailReturn] ";
    std::string prefix = sst.str();

    stringstream ss;
    ss << fixed << setprecision(4);
    ss << prefix << "error_no: " << p->error_no << "; ";
    ss << "account_no: " << acc << "; ";

    for (const OrderDetail &v : p->datas)
    {
        ss << std::endl << prefix;
        ss << "code: " << v.stock_code << "; ";
        ss << "entrust_no: " << v.entrust_no << "; ";
        ss << "order_kind: " << v.order_kind << "; ";
        ss << "dir: " << v.direction << "; ";
        ss << "oc: " << v.open_close << "; ";
        ss << "speculator: " << v.speculator << "; ";
        ss << "status: " << v.entrust_status << "; ";
        ss << "limit_price: " << v.limit_price << "; ";
        ss << "volume: " << v.volume << "; ";
        ss << "volume_traded: " << v.volume_traded << "; ";
        ss << "volume_remain: " << v.volume_remain << "; ";
        ss << "account_no: " << acc << "; ";
    }

    return ss.str();
}

static std::string ToString(long long t, const T_QryTradeDetail *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[QryTradeDetail] ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_TradeDetailReturn *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream sst;
    sst << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - " << "[TradeDetailReturn] ";
    std::string prefix = sst.str();

    stringstream ss;
    ss << fixed << setprecision(4);
    ss << prefix << "error_no: " << p->error_no << "; ";
    ss << "account_no: " << acc << "; ";

    for (const TradeDetail &v : p->datas)
    {
        ss << std::endl << prefix;
        ss << "code: " << v.stock_code << "; ";
        ss << "entrust_no: " << v.entrust_no << "; ";
        ss << "dir: " << v.direction << "; ";
        ss << "oc: " << v.open_close << "; ";
        ss << "speculator: " << v.speculator << "; ";
        ss << "trade_price: " << v.trade_price << "; ";
        ss << "trade_volume: " << v.trade_volume << "; ";
        ss << "trade_time: " << v.trade_time << "; ";
        ss << "account_no: " << acc << "; ";
    }

    return ss.str();
}

static std::string ToString(long long t, const T_QryContractInfo *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[QryContractInfo] ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

static std::string ToString(long long t, const T_ContractInfoReturn *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream sst;
    sst << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - " << "[ContractInfoReturn] ";
    std::string prefix = sst.str();

    stringstream ss;
    ss << fixed << setprecision(4);
    ss << prefix << "error_no: " << p->error_no << "; ";
    ss << "account_no: " << acc << "; ";
    for (const ContractInfo &v : p->datas)
    {
        ss << std::endl << prefix;
        ss << "stock_code: " << v.stock_code << "; ";
        ss << "TradeDate: " << v.TradeDate << "; ";
        ss << "ExpireDate: " << v.ExpireDate << "; ";
        ss << "account_no: " << acc << "; ";
    }

    return ss.str();
}

static std::string ToString(long long t, const T_ReqForQuote *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[ReqForQuote] ";

    ss << "serial_no: " << p->serial_no << "; ";
    ss << "stock_code: " << p->stock_code << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}
static std::string ToString(long long t, const T_InsertQuote *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[InsertQuote] ";
    ss << "serial_no: " << p->serial_no << "; ";
    ss << "stock_code: " << p->stock_code << "; ";
    ss << "buy_limit_price: " << p->buy_limit_price << "; ";
    ss << "buy_volume: " << p->buy_volume << "; ";
    ss << "buy_open_close: " << p->buy_open_close << "; ";
    ss << "buy_speculator: " << p->buy_speculator << "; ";
    ss << "sell_limit_price: " << p->sell_limit_price << "; ";
    ss << "sell_volume: " << p->sell_volume << "; ";
    ss << "sell_open_close: " << p->sell_open_close << "; ";
    ss << "sell_speculator: " << p->sell_speculator << "; ";
    ss << "for_quote_id: " << p->for_quote_id << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}
static std::string ToString(long long t, const T_CancelQuote *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[CancelQuote] ";
    ss << "serial_no: " << p->serial_no << "; ";
    ss << "stock_code: " << p->stock_code << "; ";
    ss << "entrust_no: " << p->entrust_no << "; ";
    ss << "org_serial_no: " << p->org_serial_no << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}
static std::string ToString(long long t, const T_RspOfReqForQuote *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[RspOfReqForQuote] ";

    ss << "serial_no: " << p->serial_no << "; ";
    ss << "error_no: " << p->error_no << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}
static std::string ToString(long long t, const T_RtnForQuote *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[RtnForQuote] ";

    ss << "stock_code: " << p->stock_code << "; ";
    ss << "for_quote_id: " << p->for_quote_id << "; ";
    ss << "for_quote_time: " << p->for_quote_time << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}
static std::string ToString(long long t, const T_InsertQuoteRespond *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[InsertQuoteRespond] ";

    ss << "serial_no: " << p->serial_no << "; ";
    ss << "error_no: " << p->error_no << "; ";
    ss << "entrust_no: " << p->entrust_no << "; ";
    ss << "entrust_status: " << p->entrust_status << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}
static std::string ToString(long long t, const T_CancelQuoteRespond *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[CancelQuoteRespond] ";

    ss << "serial_no: " << p->serial_no << "; ";
    ss << "error_no: " << p->error_no << "; ";
    ss << "entrust_no: " << p->entrust_no << "; ";
    ss << "entrust_status: " << p->entrust_status << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}
static std::string ToString(long long t, const T_QuoteReturn *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[QuoteReturn] ";

    ss << "serial_no: " << p->serial_no << "; ";
    ss << "stock_code: " << p->stock_code << "; ";
    ss << "entrust_no: " << p->entrust_no << "; ";
    ss << "entrust_status: " << p->entrust_status << "; ";
    ss << "direction: " << p->direction << "; ";
    ss << "open_close: " << p->open_close << "; ";
    ss << "speculator: " << p->speculator << "; ";
    ss << "volume: " << p->volume << "; ";
    ss << "limit_price: " << p->limit_price << "; ";
    ss << "volume_remain: " << p->volume_remain << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}
static std::string ToString(long long t, const T_QuoteTrade *p, const std::string &acc)
{
    int usec = (int) (t % 1000000000) / 1000;
    stringstream ss;
    ss << fixed << setprecision(4);
    ss << ToFormatedDatetime(t) << "." << std::setw(6) << std::setfill('0') << usec << " - ";
    ss << "[QuoteTrade] ";

    ss << "serial_no: " << p->serial_no << "; ";
    ss << "business_volume: " << p->business_volume << "; ";
    ss << "business_price: " << p->business_price << "; ";
    ss << "business_no: " << p->business_no << "; ";
    ss << "stock_code: " << p->stock_code << "; ";
    ss << "direction: " << p->direction << "; ";
    ss << "open_close: " << p->open_close << "; ";
    ss << "account_no: " << acc << "; ";

    return ss.str();
}

std::thread * LogUtil::save_thread = NULL;
std::string LogUtil::file_name_;
std::ofstream LogUtil::log_file_;
std::ofstream LogUtil::inner_log_file_;
std::unordered_map<long long, long long> LogUtil::cancel_sn_to_sn_;

void LogUtil::Start(const std::string &log_file_name, int shm_key)
{
    if (s_init_flag)
    {
        return;
    }

    lock_guard<mutex> lock(log_sync);
    if (s_init_flag)
    {
        return;
    }

    s_run_flag = true;
    if (!log_file_name.empty())
    {
        file_name_ = log_file_name;
    }
    else
    {
        file_name_ = "my_tunnel_lib_cmn";
    }

    // get share memory
//    if (shm_key != 0)
//    {
//        shm_addr = (char *) env_init(shm_key);
//        TNL_LOG_INFO("address of share memory: %p", shm_addr);
//    }
//    else
//    {
//        TNL_LOG_INFO("close inner quote source feature");
//    }

    // 文件存储
    CreateFileHandle();

    save_thread = new std::thread(&LogUtil::SaveImp);
    s_init_flag = true;
    TNL_LOG_INFO("create log save file thread");
}

void LogUtil::Stop()
{
    TNL_LOG_INFO("stop log thread");
    if (!save_thread)
    {
        return;
    }

    if (save_thread->joinable())
    {
        // 把任务做完
        while (false)
        {
            usleep(30 * 1000);
        }

        s_run_flag = false;
        save_thread->join();
        save_thread = NULL;
    }
}

void LogUtil::SaveImp()
{
    // local buffers
    OrderAndTimes place_orders_t;
    CancelAndTimes cancel_orders_t;
    OrderRspAndTimes order_responds_t;
    CancelRspAndTimes cancel_responds_t;
    OrderRtnAndTimes order_returns_t;
    TradeRtnAndTimes trade_returns_t;

    QryPositionAndTimes qry_position_t;
    PositionRtnAndTimes pos_rtn_t;
    QryOrderDetailAndTimes qry_order_detail_t;
    OrderDetailRtnAndTimes order_detail_rtn_t;
    QryTradeDetailAndTimes qry_trade_detail_t;
    TradeDetailRtnAndTimes trade_detail_rtn_t;

    QryContractInfoAndTimes qry_contract_t;
    ContractInfoRtnAndTimes contract_rtn_t;

    ReqForQuoteAndTimes req_forquote_t;
    InsertQuoteAndTimes insert_quote_t;
    CancelQuoteAndTimes cancel_quote_t;
    RspOfReqForQuoteAndTimes rsp_req_forquote_t;
    RtnForQuoteAndTimes rtn_forquote_t;
    InsertQuoteRespondAndTimes insert_quote_rsp_t;
    CancelQuoteRespondAndTimes cancel_quote_rsp_t;
    QuoteReturnAndTimes quote_rtn_t;
    QuoteTradeAndTimes quote_trade_rtn_t;

    TNL_LOG_INFO("enter log thread, s_run_flag: %d", s_run_flag);
    while (s_run_flag)
    {
        // 将数据交换到本地
        {
            lock_guard<mutex> lock(log_sync);
            place_orders.swap(place_orders_t);
            cancel_orders.swap(cancel_orders_t);
            order_responds.swap(order_responds_t);
            cancel_responds.swap(cancel_responds_t);
            order_returns.swap(order_returns_t);
            trade_returns.swap(trade_returns_t);

            qry_position.swap(qry_position_t);
            pos_rtn.swap(pos_rtn_t);
            qry_order_detail.swap(qry_order_detail_t);
            order_detail_rtn.swap(order_detail_rtn_t);
            qry_trade_detail.swap(qry_trade_detail_t);
            trade_detail_rtn.swap(trade_detail_rtn_t);
            qry_contract.swap(qry_contract_t);
            contract_rtn.swap(contract_rtn_t);

            req_forquote.swap(req_forquote_t);
            insert_quote.swap(insert_quote_t);
            cancel_quote.swap(cancel_quote_t);
            rsp_req_forquote.swap(rsp_req_forquote_t);
            rtn_forquote.swap(rtn_forquote_t);
            insert_quote_rsp.swap(insert_quote_rsp_t);
            cancel_quote_rsp.swap(cancel_quote_rsp_t);
            quote_rtn.swap(quote_rtn_t);
            quote_trade_rtn.swap(quote_trade_rtn_t);
        }

        // 没事做，让出CPU
        if (place_orders_t.empty()
            && cancel_orders_t.empty()
            && order_responds_t.empty()
            && cancel_responds_t.empty()
            && order_returns_t.empty()
            && trade_returns_t.empty()
            && qry_position_t.empty()
            && pos_rtn_t.empty()
            && qry_order_detail_t.empty()
            && order_detail_rtn_t.empty()
            && qry_trade_detail_t.empty()
            && trade_detail_rtn_t.empty()
            && req_forquote_t.empty()
            && insert_quote_t.empty()
            && cancel_quote_t.empty()
            && rsp_req_forquote_t.empty()
            && rtn_forquote_t.empty()
            && insert_quote_rsp_t.empty()
            && cancel_quote_rsp_t.empty()
            && quote_rtn_t.empty()
            && quote_trade_rtn_t.empty()
            && qry_contract_t.empty()
            && contract_rtn_t.empty()
            )
        {
            usleep(100 * 1000);
            continue;
        }

        // 自己慢慢存
        try
        {
            //TNL_LOG_DEBUG("save log to file.");
            SaveAll(place_orders_t, cancel_orders_t, order_responds_t, cancel_responds_t, order_returns_t, trade_returns_t,
                qry_position_t, pos_rtn_t, qry_order_detail_t, order_detail_rtn_t, qry_trade_detail_t, trade_detail_rtn_t,
                req_forquote_t, insert_quote_t, cancel_quote_t, rsp_req_forquote_t, rtn_forquote_t,
                insert_quote_rsp_t, cancel_quote_rsp_t, quote_rtn_t, quote_trade_rtn_t,
                qry_contract_t, contract_rtn_t);
        }
        catch (...)
        {
            syslog(LOG_ERR, "exception occur when save tunnel log\n");
        }

        // 存完清空
        place_orders_t.clear();
        cancel_orders_t.clear();
        order_responds_t.clear();
        cancel_responds_t.clear();
        order_returns_t.clear();
        trade_returns_t.clear();
        qry_position_t.clear();
        pos_rtn_t.clear();
        qry_order_detail_t.clear();
        order_detail_rtn_t.clear();
        qry_trade_detail_t.clear();
        trade_detail_rtn_t.clear();

        req_forquote_t.clear();
        insert_quote_t.clear();
        cancel_quote_t.clear();
        rsp_req_forquote_t.clear();
        rtn_forquote_t.clear();
        insert_quote_rsp_t.clear();
        cancel_quote_rsp_t.clear();
        quote_rtn_t.clear();
        quote_trade_rtn_t.clear();

        qry_contract_t.clear();
        contract_rtn_t.clear();
    }
}

long long LogUtil::GetTimeFromEpoch()
{
    // get ns(nano seconds) from Unix Epoch
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

long long LogUtil::GetTimeAndPublish(int data_type, const void *pdata, std::size_t data_size)
{
    long long t = GetTimeFromEpoch();

    // write to share memory
//    if (shm_addr)
//    {
//        OrderInfoInTunnel d(data_type, t, pdata, data_size);
//        write_share_mem(shm_addr, &d);
//    }

    return t;
}

void LogUtil::OnPlaceOrder(const T_PlaceOrder* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);

    // get us(micro seconds) from Unix Epoch, and publish data
    long long t = GetTimeAndPublish(OrderTypes::ePlace_Order, p, sizeof(T_PlaceOrder));

    // cache for write to file
    place_orders.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnCancelOrder(const T_CancelOrder* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);

    // get us(micro seconds) from Unix Epoch, and publish data
    long long t = GetTimeAndPublish(OrderTypes::eCancel_Order, p, sizeof(T_CancelOrder));

    // cache for write to file
    cancel_orders.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnOrderRespond(const T_OrderRespond* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);

    // get us(micro seconds) from Unix Epoch, and publish data
    long long t = GetTimeAndPublish(OrderTypes::eOrder_Respond, p, sizeof(T_OrderRespond));

    // cache for write to file
    order_responds.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnCancelRespond(const T_CancelRespond* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);

    // get us(micro seconds) from Unix Epoch, and publish data
    long long t = GetTimeAndPublish(OrderTypes::eCancel_Respond, p, sizeof(T_CancelRespond));

    // cache for write to file
    cancel_responds.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnOrderReturn(const T_OrderReturn* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);

    // get us(micro seconds) from Unix Epoch, and publish data
    long long t = GetTimeAndPublish(OrderTypes::eOrder_Return, p, sizeof(T_OrderReturn));

    // cache for write to file
    order_returns.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnTradeReturn(const T_TradeReturn* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);

    // get us(micro seconds) from Unix Epoch, and publish data
    long long t = GetTimeAndPublish(OrderTypes::eTrade_Return, p, sizeof(T_TradeReturn));

    // cache for write to file
    trade_returns.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnQryPosition(const T_QryPosition* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    qry_position.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnPositionRtn(const T_PositionReturn *p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    pos_rtn.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnQryOrderDetail(const T_QryOrderDetail *p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    qry_order_detail.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnOrderDetailRtn(const T_OrderDetailReturn *p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    order_detail_rtn.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnQryTradeDetail(const T_QryTradeDetail *p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    qry_trade_detail.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnTradeDetailRtn(const T_TradeDetailReturn *p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    trade_detail_rtn.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnQryContractInfo(const T_QryContractInfo *p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    qry_contract.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnContractInfoRtn(const T_ContractInfoReturn *p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    contract_rtn.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::CreateFileHandle()
{
    //Create Directory
    char cur_path[256];
    getcwd(cur_path, sizeof(cur_path));
    std::string full_path = std::string(cur_path) + "/log";

    // check whether dir exist
    if (opendir(full_path.c_str()) == NULL)
    {
        mkdir(full_path.c_str(), 0755);
    }

    // open log file
    std::string strTime = my_cmn::GetCurrentDateString();
    std::string str;

    str = full_path + "/" + file_name_ + "_" + strTime + ".log";
    log_file_.open(str.c_str(), std::ios_base::out | std::ios_base::app);
    log_file_ << GetLocalVPNIP() << std::endl;

    str = full_path + "/" + file_name_ + "_inner_" + my_cmn::GetCurrentDateTimeString() + ".log";
    inner_log_file_.open(str.c_str(), std::ios_base::out | std::ios_base::app);
    inner_log_file_ << GetLocalVPNIP() << std::endl;
}

std::string LogUtil::GetLocalVPNIP()
{
    std::string vpn_ip("0.0.0.0");
    struct ifaddrs *ifap = NULL;
    int res = getifaddrs(&ifap);
    if (res == 0 && ifap)
    {
        struct ifaddrs *cur_if = ifap;

        while (cur_if != NULL)
        {
            // only get IP4 address
            if (cur_if->ifa_addr->sa_family == AF_INET)
            {
                void * tmpAddrPtr = &((struct sockaddr_in *) cur_if->ifa_addr)->sin_addr;
                char ip_addr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, ip_addr, INET_ADDRSTRLEN);

                // check whether vpn address
                std::vector<std::string> fields;
                my_cmn::MYStringSplit(ip_addr, fields, '.');
                if (fields.size() >= 4 && atoi(fields[2].c_str()) == 30)
                {
                    vpn_ip = ip_addr;
                    break;
                }
            }
            cur_if = cur_if->ifa_next;
        }

        freeifaddrs(ifap);
    }

    return vpn_ip;
}

struct NoTypeDataAndTime
{
    long long time;
    char type;
    const void * p_data;
    Tunnel_Info tunnel_info;

    NoTypeDataAndTime(long long t, char c, const void *d, const Tunnel_Info &t_info)
        : time(t), type(c), p_data(d), tunnel_info(t_info)
    {
    }

    bool operator <(const NoTypeDataAndTime &other)
    {
        return time < other.time;
    }
};
typedef std::list<NoTypeDataAndTime> NoTypeDataAndTimeCollection;

template<typename T>
void MergeAllTypeData(const T &d, char c,
    NoTypeDataAndTimeCollection &all_datas)
{
    typename T::const_iterator p(d.begin());
    for (; p != d.end(); ++p)
    {
        all_datas.push_back(NoTypeDataAndTime(std::get<0>(*p), c, &(std::get<1>(*p)), std::get<2>(*p)));
    }
}

void LogUtil::OnReqForQuote(const T_ReqForQuote* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    req_forquote.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnInsertQuote(const T_InsertQuote* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    insert_quote.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnCancelQuote(const T_CancelQuote* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    cancel_quote.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnRspOfReqForQuote(const T_RspOfReqForQuote* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    rsp_req_forquote.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnRtnForQuote(const T_RtnForQuote* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    rtn_forquote.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnInsertQuoteRespond(const T_InsertQuoteRespond* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    insert_quote_rsp.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnCancelQuoteRespond(const T_CancelQuoteRespond* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    cancel_quote_rsp.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnQuoteReturn(const T_QuoteReturn* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    quote_rtn.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::OnQuoteTrade(const T_QuoteTrade* p, const Tunnel_Info &tunnel_info)
{
    lock_guard<mutex> lock(log_sync);
    long long t = GetTimeFromEpoch();
    quote_trade_rtn.push_back(std::make_tuple(t, *p, tunnel_info));
}

void LogUtil::SaveAll(const OrderAndTimes &place_orders_t,
    const CancelAndTimes &cancel_orders_t,
    const OrderRspAndTimes &order_responds_t,
    const CancelRspAndTimes &cancel_responds_t,
    const OrderRtnAndTimes &order_returns_t,
    const TradeRtnAndTimes &trade_returns_t,
    const QryPositionAndTimes &qry_position_t,
    const PositionRtnAndTimes &pos_rtn_t,
    const QryOrderDetailAndTimes &qry_order_detail_t,
    const OrderDetailRtnAndTimes &order_detail_rtn_t,
    const QryTradeDetailAndTimes &qry_trade_detail_t,
    const TradeDetailRtnAndTimes &trade_detail_rtn_t,
    const ReqForQuoteAndTimes &req_forquote_t,
    const InsertQuoteAndTimes &insert_quote_t,
    const CancelQuoteAndTimes &cancel_quote_t,
    const RspOfReqForQuoteAndTimes &rsp_req_forquote_t,
    const RtnForQuoteAndTimes &rtn_forquote_t,
    const InsertQuoteRespondAndTimes &insert_quote_rsp_t,
    const CancelQuoteRespondAndTimes &cancel_quote_rsp_t,
    const QuoteReturnAndTimes &quote_rtn_t,
    const QuoteTradeAndTimes &quote_trade_rtn_t,
    const QryContractInfoAndTimes &qry_contract_t,
    const ContractInfoRtnAndTimes &contract_rtn_t
    )
{
    NoTypeDataAndTimeCollection all_datas;

    // merge all type data
    MergeAllTypeData(place_orders_t, '1', all_datas);
    MergeAllTypeData(cancel_orders_t, '2', all_datas);
    MergeAllTypeData(order_responds_t, '3', all_datas);
    MergeAllTypeData(cancel_responds_t, '4', all_datas);
    MergeAllTypeData(order_returns_t, '5', all_datas);
    MergeAllTypeData(trade_returns_t, '6', all_datas);

    MergeAllTypeData(qry_position_t, '7', all_datas);
    MergeAllTypeData(pos_rtn_t, '8', all_datas);
    MergeAllTypeData(qry_order_detail_t, '9', all_datas);
    MergeAllTypeData(order_detail_rtn_t, 'a', all_datas);
    MergeAllTypeData(qry_trade_detail_t, 'b', all_datas);
    MergeAllTypeData(trade_detail_rtn_t, 'c', all_datas);

    MergeAllTypeData(req_forquote_t, 'd', all_datas);
    MergeAllTypeData(insert_quote_t, 'e', all_datas);
    MergeAllTypeData(cancel_quote_t, 'f', all_datas);
    MergeAllTypeData(rsp_req_forquote_t, 'g', all_datas);
    MergeAllTypeData(rtn_forquote_t, 'h', all_datas);
    MergeAllTypeData(insert_quote_rsp_t, 'i', all_datas);
    MergeAllTypeData(cancel_quote_rsp_t, 'j', all_datas);
    MergeAllTypeData(quote_rtn_t, 'k', all_datas);
    MergeAllTypeData(quote_trade_rtn_t, 'l', all_datas);

    MergeAllTypeData(qry_contract_t, 'm', all_datas);
    MergeAllTypeData(contract_rtn_t, 'n', all_datas);

    // sort by time
    all_datas.sort();

    // output to file
    for (NoTypeDataAndTimeCollection::const_iterator p = all_datas.begin(); p != all_datas.end(); ++p)
    {
        switch (p->type)
        {
            case '1':
                {
                log_file_ << ToString(p->time, (const T_PlaceOrder *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case '2':
                {
                // report serial no of place order instead of cancel serial no. modified on 20160426
                const T_CancelOrder * co = (const T_CancelOrder *) p->p_data;
                cancel_sn_to_sn_.insert(std::make_pair(co->serial_no, co->org_serial_no));
                log_file_ << ToString(p->time, (const T_CancelOrder *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case '3':
                {
                log_file_ << ToString(p->time, (const T_OrderRespond *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case '4':
                {
                // report serial no of place order instead of cancel serial no. modified on 20160426
                SerialNoType po_sn = 0;
                const T_CancelRespond * co_rsp = (const T_CancelRespond *) p->p_data;
                std::unordered_map<long long, long long>::const_iterator cit = cancel_sn_to_sn_.find(co_rsp->serial_no);
                if (cit != cancel_sn_to_sn_.end())
                {
                    po_sn = cit->second;
                }
                log_file_ << ToString(p->time, (const T_CancelRespond *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case '5':
                {
                log_file_ << ToString(p->time, (const T_OrderReturn *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case '6':
                {
                log_file_ << ToString(p->time, (const T_TradeReturn *) p->p_data, p->tunnel_info.account) << std::endl;
                break;

            }

            case '7':
                {
                inner_log_file_ << ToString(p->time, (const T_QryPosition *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case '8':
                {
                inner_log_file_ << ToString(p->time, (const T_PositionReturn *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case '9':
                {
                inner_log_file_ << ToString(p->time, (const T_QryOrderDetail *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'a':
                {
                inner_log_file_ << ToString(p->time, (const T_OrderDetailReturn *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'b':
                {
                inner_log_file_ << ToString(p->time, (const T_QryTradeDetail *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'c':
                {
                inner_log_file_ << ToString(p->time, (const T_TradeDetailReturn *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }

            case 'd':
                {
                log_file_ << ToString(p->time, (const T_ReqForQuote *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'e':
                {
                log_file_ << ToString(p->time, (const T_InsertQuote *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'f':
                {
                log_file_ << ToString(p->time, (const T_CancelQuote *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'g':
                {
                log_file_ << ToString(p->time, (const T_RspOfReqForQuote *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'h':
                {
                log_file_ << ToString(p->time, (const T_RtnForQuote *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'i':
                {
                log_file_ << ToString(p->time, (const T_InsertQuoteRespond *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'j':
                {
                log_file_ << ToString(p->time, (const T_CancelQuoteRespond *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'k':
                {
                log_file_ << ToString(p->time, (const T_QuoteReturn *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'l':
                {
                log_file_ << ToString(p->time, (const T_QuoteTrade *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'm':
                {
                inner_log_file_ << ToString(p->time, (const T_QryContractInfo *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            case 'n':
                {
                inner_log_file_ << ToString(p->time, (const T_ContractInfoReturn *) p->p_data, p->tunnel_info.account) << std::endl;
                break;
            }
            default:
                {
                TNL_LOG_ERROR("unknown data type of log data, type: %d", p->type);
                break;
            }
        }
    }
}
