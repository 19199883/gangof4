#ifndef TRADE_LOG_UTIL_H_
#define TRADE_LOG_UTIL_H_

#include <string>
#include <fstream>
#include <vector>
#include <tuple>
#include <thread>
#include <unordered_map>

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
struct Tunnel_Info {
	Tunnel_Info() {
		qtm_name = "";
		account = "";
	}

	Tunnel_Info(const Tunnel_Info& other) {
		qtm_name = other.qtm_name;
		account = other.account;
	}

	std::string qtm_name;
	std::string account;
};

typedef std::vector<std::tuple<long long, T_PlaceOrder, Tunnel_Info> > OrderAndTimes;
typedef std::vector<std::tuple<long long, T_CancelOrder, Tunnel_Info> > CancelAndTimes;
typedef std::vector<std::tuple<long long, T_OrderRespond, Tunnel_Info> > OrderRspAndTimes;
typedef std::vector<std::tuple<long long, T_CancelRespond, Tunnel_Info> > CancelRspAndTimes;
typedef std::vector<std::tuple<long long, T_OrderReturn, Tunnel_Info> > OrderRtnAndTimes;
typedef std::vector<std::tuple<long long, T_TradeReturn, Tunnel_Info> > TradeRtnAndTimes;

typedef std::vector<std::tuple<long long, T_QryPosition, Tunnel_Info> > QryPositionAndTimes;
typedef std::vector<std::tuple<long long, T_PositionReturn, Tunnel_Info> > PositionRtnAndTimes;
typedef std::vector<std::tuple<long long, T_QryOrderDetail, Tunnel_Info> > QryOrderDetailAndTimes;
typedef std::vector<std::tuple<long long, T_OrderDetailReturn, Tunnel_Info> > OrderDetailRtnAndTimes;
typedef std::vector<std::tuple<long long, T_QryTradeDetail, Tunnel_Info> > QryTradeDetailAndTimes;
typedef std::vector<std::tuple<long long, T_TradeDetailReturn, Tunnel_Info> > TradeDetailRtnAndTimes;
typedef std::vector<std::tuple<long long, T_QryContractInfo, Tunnel_Info> > QryContractInfoAndTimes;
typedef std::vector<std::tuple<long long, T_ContractInfoReturn, Tunnel_Info> > ContractInfoRtnAndTimes;

struct T_ReqForQuote ;       //  询价请求      模型 -> 通道
struct T_InsertQuote;        //  报价请求      模型 -> 通道
struct T_CancelQuote;        //  撤销报价      模型 -> 通道
struct T_RspOfReqForQuote;   //  询价响应      模型 <- 通道
struct T_RtnForQuote;        //  询价通知      模型 <- 通道
struct T_InsertQuoteRespond; //  报价应答      模型 <- 通道
struct T_CancelQuoteRespond; //  撤销报价应答  模型 <- 通道
struct T_QuoteReturn;        //  报价状态回报  模型 <- 通道
struct T_QuoteTrade;         //  报价成交回报  模型 <- 通道
typedef std::vector<std::tuple<long long, T_ReqForQuote, Tunnel_Info> > ReqForQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_InsertQuote, Tunnel_Info> > InsertQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_CancelQuote, Tunnel_Info> > CancelQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_RspOfReqForQuote, Tunnel_Info> > RspOfReqForQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_RtnForQuote, Tunnel_Info> > RtnForQuoteAndTimes;
typedef std::vector<std::tuple<long long, T_InsertQuoteRespond, Tunnel_Info> > InsertQuoteRespondAndTimes;
typedef std::vector<std::tuple<long long, T_CancelQuoteRespond, Tunnel_Info> > CancelQuoteRespondAndTimes;
typedef std::vector<std::tuple<long long, T_QuoteReturn, Tunnel_Info> > QuoteReturnAndTimes;
typedef std::vector<std::tuple<long long, T_QuoteTrade, Tunnel_Info> > QuoteTradeAndTimes;

class LogUtil
{
public:
    static void Start(const std::string &log_file_name, int shm_key);

    static void Stop();

    static void OnPlaceOrder(const T_PlaceOrder *p, const Tunnel_Info &tunnel_info);
    static void OnCancelOrder(const T_CancelOrder *p, const Tunnel_Info &tunnel_info);
    static void OnOrderRespond(const T_OrderRespond *p, const Tunnel_Info &tunnel_info);
    static void OnCancelRespond(const T_CancelRespond *p, const Tunnel_Info &tunnel_info);
    static void OnOrderReturn(const T_OrderReturn *p, const Tunnel_Info &tunnel_info);
    static void OnTradeReturn(const T_TradeReturn *p, const Tunnel_Info &tunnel_info);

    static void OnQryPosition(const T_QryPosition *p, const Tunnel_Info &tunnel_info);
    static void OnPositionRtn(const T_PositionReturn *p, const Tunnel_Info &tunnel_info);
    static void OnQryOrderDetail(const T_QryOrderDetail *p, const Tunnel_Info &tunnel_info);
    static void OnOrderDetailRtn(const T_OrderDetailReturn *p, const Tunnel_Info &tunnel_info);
    static void OnQryTradeDetail(const T_QryTradeDetail *p, const Tunnel_Info &tunnel_info);
    static void OnTradeDetailRtn(const T_TradeDetailReturn *p, const Tunnel_Info &tunnel_info);
    static void OnQryContractInfo(const T_QryContractInfo *p, const Tunnel_Info &tunnel_info);
    static void OnContractInfoRtn(const T_ContractInfoReturn *p, const Tunnel_Info &tunnel_info);

    static void OnReqForQuote(const T_ReqForQuote *p, const Tunnel_Info &tunnel_info);
    static void OnInsertQuote(const T_InsertQuote *p, const Tunnel_Info &tunnel_info);
    static void OnCancelQuote(const T_CancelQuote *p, const Tunnel_Info &tunnel_info);
    static void OnRspOfReqForQuote(const T_RspOfReqForQuote *p, const Tunnel_Info &tunnel_info);
    static void OnRtnForQuote(const T_RtnForQuote *p, const Tunnel_Info &tunnel_info);
    static void OnInsertQuoteRespond(const T_InsertQuoteRespond *p, const Tunnel_Info &tunnel_info);
    static void OnCancelQuoteRespond(const T_CancelQuoteRespond *p, const Tunnel_Info &tunnel_info);
    static void OnQuoteReturn(const T_QuoteReturn *p, const Tunnel_Info &tunnel_info);
    static void OnQuoteTrade(const T_QuoteTrade *p, const Tunnel_Info &tunnel_info);

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

    // add map to save sn to cancel_sn
    static std::unordered_map<long long, long long> cancel_sn_to_sn_;
};

#endif // TRADE_LOG_UTIL_H_
