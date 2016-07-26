#ifndef EX_TRADE_LOG_UTIL_H_
#define EX_TRADE_LOG_UTIL_H_

#include <string>
#include <fstream>
#include <vector>
#include <tuple>
#include <thread>

// forward declare
struct T_PlaceOrder;
struct T_CancelOrder;
struct T_OrderRespond;
struct T_CancelRespond;
struct T_OrderReturn;
struct T_TradeReturn;

struct T_QryPosition;
struct T_PositionReturn;
struct T_QryOrderDetail;
struct T_OrderDetailReturn;
struct T_QryTradeDetail;
struct T_TradeDetailReturn;
struct T_QryContractInfo;
struct T_ContractInfoReturn;

typedef std::vector<std::tuple<long long, T_PlaceOrder, std::string> > OrderAndTimes;
typedef std::vector<std::tuple<long long, T_CancelOrder, std::string> > CancelAndTimes;
typedef std::vector<std::tuple<long long, T_OrderRespond, std::string> > OrderRspAndTimes;
typedef std::vector<std::tuple<long long, T_CancelRespond, std::string> > CancelRspAndTimes;
typedef std::vector<std::tuple<long long, T_OrderReturn, std::string> > OrderRtnAndTimes;
typedef std::vector<std::tuple<long long, T_TradeReturn, std::string> > TradeRtnAndTimes;

typedef std::vector<std::tuple<long long, T_QryPosition, std::string> > QryPositionAndTimes;
typedef std::vector<std::tuple<long long, T_PositionReturn, std::string> > PositionRtnAndTimes;
typedef std::vector<std::tuple<long long, T_QryOrderDetail, std::string> > QryOrderDetailAndTimes;
typedef std::vector<std::tuple<long long, T_OrderDetailReturn, std::string> > OrderDetailRtnAndTimes;
typedef std::vector<std::tuple<long long, T_QryTradeDetail, std::string> > QryTradeDetailAndTimes;
typedef std::vector<std::tuple<long long, T_TradeDetailReturn, std::string> > TradeDetailRtnAndTimes;
typedef std::vector<std::tuple<long long, T_QryContractInfo, std::string> > QryContractInfoAndTimes;
typedef std::vector<std::tuple<long long, T_ContractInfoReturn, std::string> > ContractInfoRtnAndTimes;

struct T_ReqForQuote ;       //  询价请求      模型 -> 通道
struct T_InsertQuote;        //  报价请求      模型 -> 通道
struct T_CancelQuote;        //  撤销报价      模型 -> 通道
struct T_RspOfReqForQuote;   //  询价响应      模型 <- 通道
struct T_RtnForQuote;        //  询价通知      模型 <- 通道
struct T_InsertQuoteRespond; //  报价应答      模型 <- 通道
struct T_CancelQuoteRespond; //  撤销报价应答  模型 <- 通道
struct T_QuoteReturn;        //  报价状态回报  模型 <- 通道
struct T_QuoteTrade;         //  报价成交回报  模型 <- 通道
typedef std::vector<std::tuple<long long, T_ReqForQuote, std::string> > ReqForQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_InsertQuote, std::string> > InsertQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_CancelQuote, std::string> > CancelQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_RspOfReqForQuote, std::string> > RspOfReqForQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_RtnForQuote, std::string> > RtnForQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_InsertQuoteRespond, std::string> > InsertQuoteRespondAndTimes;
typedef std::vector<std::tuple<long long, T_CancelQuoteRespond, std::string> > CancelQuoteRespondAndTimes;
typedef std::vector<std::tuple<long long, T_QuoteReturn, std::string> > QuoteReturnAndTimes;
typedef std::vector<std::tuple<long long, T_QuoteTrade, std::string> > QuoteTradeAndTimes;

class MYExchangeLogUtil
{
public:
    static void Start(const std::string &log_file_name, int shm_key);

    static void Stop();

    static void OnPlaceOrder(const T_PlaceOrder *p, const std::string &account);
    static void OnCancelOrder(const T_CancelOrder *p, const std::string &account);
    static void OnOrderRespond(const T_OrderRespond *p, const std::string &account);
    static void OnCancelRespond(const T_CancelRespond *p, const std::string &account);
    static void OnOrderReturn(const T_OrderReturn *p, const std::string &account);
    static void OnTradeReturn(const T_TradeReturn *p, const std::string &account);

    static void OnQryPosition(const T_QryPosition *p, const std::string &account);
    static void OnPositionRtn(const T_PositionReturn *p, const std::string &account);
    static void OnQryOrderDetail(const T_QryOrderDetail *p, const std::string &account);
    static void OnOrderDetailRtn(const T_OrderDetailReturn *p, const std::string &account);
    static void OnQryTradeDetail(const T_QryTradeDetail *p, const std::string &account);
    static void OnTradeDetailRtn(const T_TradeDetailReturn *p, const std::string &account);
    static void OnQryContractInfo(const T_QryContractInfo *p, const std::string &account);
    static void OnContractInfoRtn(const T_ContractInfoReturn *p, const std::string &account);

    static void OnReqForQuote(const T_ReqForQuote *p, const std::string &account);
    static void OnInsertQuote(const T_InsertQuote *p, const std::string &account);
    static void OnCancelQuote(const T_CancelQuote *p, const std::string &account);
    static void OnRspOfReqForQuote(const T_RspOfReqForQuote *p, const std::string &account);
    static void OnRtnForQuote(const T_RtnForQuote *p, const std::string &account);
    static void OnInsertQuoteRespond(const T_InsertQuoteRespond *p, const std::string &account);
    static void OnCancelQuoteRespond(const T_CancelQuoteRespond *p, const std::string &account);
    static void OnQuoteReturn(const T_QuoteReturn *p, const std::string &account);
    static void OnQuoteTrade(const T_QuoteTrade *p, const std::string &account);

private:
    static void SaveImp();

    static void CreateFileHandle();
    static std::string GetLocalVPNIP();

    static void SaveAll(
        const OrderAndTimes &place_orders_t,
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
        const ReqForQuoteAndTimes req_forquote_t,
        const InsertQuoteAndTimes insert_quote_t,
        const CancelQuoteAndTimes cancel_quote_t,
        const RspOfReqForQuoteAndTimes rsp_req_forquote_t,
        const RtnForQuoteAndTimes rtn_forquote_t,
        const InsertQuoteRespondAndTimes insert_quote_rsp_t,
        const CancelQuoteRespondAndTimes cancel_quote_rsp_t,
        const QuoteReturnAndTimes quote_rtn_t,
        const QuoteTradeAndTimes quote_trade_rtn_t,
        const QryContractInfoAndTimes qry_contract_t,
        const ContractInfoRtnAndTimes contract_rtn_t
        );

    static std::thread *save_thread;

    // 文件存储
    static std::ofstream log_file_;
    static std::ofstream inner_log_file_;
    static std::string file_name_;

    // share memory for write inner quote data
    static char *shm_addr;

    static long long GetTimeFromEpoch();
    static long long GetTimeAndPublish(int data_type, const void *pdata, std::size_t data_size);
};

#endif
