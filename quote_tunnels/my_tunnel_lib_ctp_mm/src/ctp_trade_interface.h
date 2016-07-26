#ifndef CTP_TRADE_INTERFACE_H_
#define CTP_TRADE_INTERFACE_H_

#include <string>
#include <sstream>
#include <list>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "ThostFtdcTraderApi.h"

#include "config_data.h"
#include "trade_data_type.h"
#include "ctp_trade_context.h"
#include "my_tunnel_lib.h"
#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"
#include "field_convert.h"
#include "ctp_data_formater.h"
#include "my_trade_tunnel_data_type.h"

#include "ctp_forquote.h"

struct OriginalReqInfo;

class MYForquote;

class MyCtpTradeSpi: public CThostFtdcTraderSpi
{
public:
    MyCtpTradeSpi(const TunnelConfigData &cfg);
    virtual ~MyCtpTradeSpi(void);

    void SetCallbackHandler(std::function<void(const T_OrderRespond *)> handler)
    {
        order_rsp_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_CancelRespond *)> handler)
    {
        cancel_rsp_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_OrderReturn *)> handler)
    {
        order_rtn_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_TradeReturn *)> handler)
    {
        trade_rtn_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
    {
        qry_pos_rtn_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler)
    {
        qry_order_rtn_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
    {
        qry_trade_rtn_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_ContractInfoReturn *)> handler)
    {
        qry_contract_rtn_handler_ = handler;
    }

    void SetCallbackHandler(std::function<void(const T_RspOfReqForQuote *)> handler)
    {
        reqforquote_rsp_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_RtnForQuote *)> handler)
    {
        forquote_rtn_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_InsertQuoteRespond *)> handler)
    {
        quote_rsp_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_CancelQuoteRespond *)> handler)
    {
        cancelquote_rsp_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_QuoteReturn *)> handler)
    {
        quote_rtn_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_QuoteTrade *)> handler)
    {
        quotetrade_rtn_handler_ = handler;
    }

    bool ParseConfig();

    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected();

    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param nReason 错误原因
    ///        0x1001 网络读失败
    ///        0x1002 网络写失败
    ///        0x2001 接收心跳超时
    ///        0x2002 发送心跳失败
    ///        0x2003 收到错误报文
    virtual void OnFrontDisconnected(int nReason);

    ///心跳超时警告。当长时间未收到报文时，该方法被调用。
    ///@param nTimeLapse 距离上次接收报文的时间
    virtual void OnHeartBeatWarning(int nTimeLapse);

    ///登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast);

    ///登出请求响应
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast);

    ///报单录入请求响应
    virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast);

    ///报单操作请求响应
    virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    ///投资者结算结果确认响应
    virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询投资者结算结果响应
    virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    ///请求查询结算信息确认响应
    virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///错误应答
    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///报单通知
    virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

    ///成交通知
    virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

    ///报单录入错误回报
    virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

    ///报单操作错误回报
    virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);

    ///请求查询报单响应
    virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询成交响应
    virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询投资者持仓响应
    virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    ///请求查询投资者持仓明细响应
    virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询合约响应
    virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast);

    ///请求查询资金账户响应
    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    // added on 20141216 for support market making
    ///询价录入请求响应
    virtual void OnRspForQuoteInsert(CThostFtdcInputForQuoteField *p_data, CThostFtdcRspInfoField *p_rsp, int req_id, bool bIsLast);

    ///报价录入请求响应
    virtual void OnRspQuoteInsert(CThostFtdcInputQuoteField *p_data, CThostFtdcRspInfoField *p_rsp, int req_id,
    bool bIsLast);

    ///报价操作请求响应
    virtual void OnRspQuoteAction(CThostFtdcInputQuoteActionField *p_data, CThostFtdcRspInfoField *p_rsp, int req_id, bool bIsLast);

    ///请求查询报价响应
    virtual void OnRspQryQuote(CThostFtdcQuoteField *p_data, CThostFtdcRspInfoField *p_rsp, int req_id, bool bIsLast);

    ///询价录入错误回报
    virtual void OnErrRtnForQuoteInsert(CThostFtdcInputForQuoteField *p_data, CThostFtdcRspInfoField *p_rsp);

    ///报价通知
    virtual void OnRtnQuote(CThostFtdcQuoteField *p_data);

    ///报价录入错误回报
    virtual void OnErrRtnQuoteInsert(CThostFtdcInputQuoteField *p_data, CThostFtdcRspInfoField *p_rsp);

    ///报价操作错误回报
    virtual void OnErrRtnQuoteAction(CThostFtdcQuoteActionField *p_data, CThostFtdcRspInfoField *p_rsp);

    ///询价通知
    virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *p_data);
    void ForQuoteRtn(const T_RtnForQuote * p)
    {
        LogUtil::OnRtnForQuote(p, tunnel_info_);

        if (forquote_rtn_handler_) forquote_rtn_handler_(p);
    }

public:
    // 下发指令接口
    int ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqOrderInsert(pInputOrder, nRequestID);
            if (ret != 0)
            {
                // -2，表示未处理请求超过许可数；
                // -3，表示每秒发送请求数超过许可数。
                if (ret == -2)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
                }
                if (ret == -3)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST_PER_SECOND;
                }
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqOrderInsert.");
        }

        return ret;
    }

    //报单操作请求
    int ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderAction when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqOrderAction(pInputOrderAction, nRequestID);
            if (ret != 0)
            {
                // -2，表示未处理请求超过许可数；
                // -3，表示每秒发送请求数超过许可数。
                if (ret == -2)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
                }
                if (ret == -3)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST_PER_SECOND;
                }
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqOrderAction.");
        }

        return ret;
    }

    int QryPosition(CThostFtdcQryInvestorPositionDetailField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryPosition when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = api_->ReqQryInvestorPositionDetail(p, request_id);
        TNL_LOG_INFO("ReqQryInvestorPositionDetail - request_id:%d, return:%d", request_id, ret);

        return ret;
    }
    int QryOrderDetail(CThostFtdcQryOrderField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryOrderDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = api_->ReqQryOrder(p, request_id);
        TNL_LOG_INFO("ReqQryOrder - request_id:%d, return:%d", request_id, ret);

        return ret;
    }
    int QryTradeDetail(CThostFtdcQryTradeField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryTradeDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = api_->ReqQryTrade(p, request_id);
        TNL_LOG_INFO("ReqQryTrade - request_id:%d, return:%d", request_id, ret);

        return ret;
    }
    int QryInstrument(CThostFtdcQryInstrumentField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryInstrument when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        if (finish_query_contracts_)
        {
            // use cached datas
            T_ContractInfoReturn contract_info;
            contract_info.datas.swap(ci_buffer_);
            contract_info.error_no = 0;

            if (qry_contract_rtn_handler_)
            {
                qry_contract_rtn_handler_(&contract_info);
            }

            LogUtil::OnContractInfoRtn(&contract_info, tunnel_info_);

            return 0;
        }
        else
        {
            int ret = api_->ReqQryInstrument(p, request_id);
            TNL_LOG_INFO("ReqQryInstrument - request_id:%d, return:%d", request_id, ret);

            return ret;
        }
    }

    ///询价录入请求
    int ReqForQuoteInsert(CThostFtdcInputForQuoteField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqForQuoteInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqForQuoteInsert(p, request_id);
            if (ret != 0)
            {
                // -2，表示未处理请求超过许可数；
                // -3，表示每秒发送请求数超过许可数。
                if (ret == -2)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
                }
                if (ret == -3)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST_PER_SECOND;
                }
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqForQuoteInsert.");
        }

        return ret;
    }

    ///报价录入请求
    int ReqQuoteInsert(CThostFtdcInputQuoteField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqQuoteInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqQuoteInsert(p, request_id);
            TNL_LOG_DEBUG("ReqQuoteInsert - %s", CTPDatatypeFormater::ToString(p).c_str());
            if (ret != 0)
            {
                // -2，表示未处理请求超过许可数；
                // -3，表示每秒发送请求数超过许可数。
                if (ret == -2)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
                }
                if (ret == -3)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST_PER_SECOND;
                }
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqQuoteInsert.");
        }

        return ret;
    }

    ///报价操作请求
    int ReqQuoteAction(CThostFtdcInputQuoteActionField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqQuoteAction when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqQuoteAction(p, request_id);
            TNL_LOG_DEBUG("ReqQuoteAction - %s", CTPDatatypeFormater::ToString(p).c_str());
            if (ret != 0)
            {
                // -2，表示未处理请求超过许可数；
                // -3，表示每秒发送请求数超过许可数。
                if (ret == -2)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
                }
                if (ret == -3)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST_PER_SECOND;
                }
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqQuoteAction.");
        }

        return ret;
    }

    ///请求查询报价
    int ReqQryQuote(CThostFtdcQryQuoteField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqQryQuote when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqQryQuote(p, request_id);
            if (ret != 0)
            {
                // -2，表示未处理请求超过许可数；
                // -3，表示每秒发送请求数超过许可数。
                if (ret == -2)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
                }
                if (ret == -3)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST_PER_SECOND;
                }
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqQryQuote.");
        }

        return ret;
    }

    bool TunnelIsReady()
    {
        return logoned_ && HaveFinishQueryOrders() && have_handled_unterminated_quotes_ && finish_query_contracts_;
    }
    int Front_id() const
    {
        return front_id_;
    }
    int Session_id() const
    {
        return session_id_;
    }
    void StartPositionBackUp();

private:
    // 委托回报处理
    void OnRtnOrderNormal(CThostFtdcOrderField * pOrder, const OriginalReqInfo * p, OrderRefDataType order_ref);
    void OnRtnOrderFak(CThostFtdcOrderField * pOrder, const OriginalReqInfo * p, OrderRefDataType order_ref);
    void OnRtnQuoteNormal(CThostFtdcOrderField * pOrder, const CTPQuoteInfo * p, OrderRefDataType quote_ref);

    void ReqLogin();
    void SaveToFile();

public:
    CTPTradeContext ctp_trade_context_;
    std::mutex client_sync;

    // 外部接口对象使用，为避免修改接口，新增对象放到此处
    std::mutex rsp_sync;
    std::condition_variable rsp_con;

private:
    CThostFtdcTraderApi *api_;
    int front_id_;
    int session_id_;
    int max_order_ref_;

    Tunnel_Info tunnel_info_;
    std::string pswd_;
    std::string quote_addr_;
    std::string exchange_code_;
    TThostFtdcDateType trade_day;
    double available_fund;
    void QueryAccount();
    void QueryContractInfo();
    PositionDetail PackageAvailableFunc();

    std::function<void(const T_OrderRespond *)> order_rsp_handler_;
    std::function<void(const T_CancelRespond *)> cancel_rsp_handler_;
    std::function<void(const T_OrderReturn *)> order_rtn_handler_;
    std::function<void(const T_TradeReturn *)> trade_rtn_handler_;

    std::function<void(const T_PositionReturn *)> qry_pos_rtn_handler_;
    std::function<void(const T_OrderDetailReturn *)> qry_order_rtn_handler_;
    std::function<void(const T_TradeDetailReturn *)> qry_trade_rtn_handler_;
    std::function<void(const T_ContractInfoReturn *)> qry_contract_rtn_handler_;

    // added for support market making interface
    std::function<void(const T_RspOfReqForQuote *)> reqforquote_rsp_handler_;
    std::function<void(const T_RtnForQuote *)> forquote_rtn_handler_;
    std::function<void(const T_InsertQuoteRespond *)> quote_rsp_handler_;
    std::function<void(const T_CancelQuoteRespond *)> cancelquote_rsp_handler_;
    std::function<void(const T_QuoteReturn *)> quote_rtn_handler_;
    std::function<void(const T_QuoteTrade *)> quotetrade_rtn_handler_;

    // 配置数据对象
    TunnelConfigData cfg_;
    volatile bool connected_;
    std::atomic_bool logoned_;
    std::atomic_bool backup_;
    std::vector<CThostFtdcInvestorPositionDetailField> pos_info_;

    // query position control variables
    std::vector<PositionDetail> pos_buffer_;
    void CheckAndSaveYestodayPosition();
    void LoadYestodayPositionFromFile(const std::string &file);
    void SaveYestodayPositionToFile(const std::string &file);

    // query order detail control variables
    std::vector<OrderDetail> od_buffer_;

    // query trade detail control variables
    std::vector<TradeDetail> td_buffer_;

    // query contract info control variables
    std::vector<ContractInfo> ci_buffer_;

    // variables and functions for cancel all unterminated orders automatically
    std::atomic_bool have_handled_unterminated_orders_;
    std::vector<CThostFtdcOrderField> unterminated_orders_;
    std::mutex cancel_sync_;
    std::condition_variable qry_order_finish_cond_;
    CThostFtdcInputOrderActionField CreatCancelParam(const CThostFtdcOrderField & order_field);
    bool IsOrderTerminate(const CThostFtdcOrderField & order_field);
    void QueryAndHandleOrders();

    // variables and functions for cancel all unterminated quotes automatically
    std::atomic_bool have_handled_unterminated_quotes_;
    std::vector<CThostFtdcQuoteField> unterminated_quotes_;
    std::mutex cancel_quote_sync_;
    std::condition_variable qry_quote_finish_cond_;
    CThostFtdcInputQuoteActionField CreatCancelQuoteParam(const CThostFtdcQuoteField & quote_field);
    bool IsQuoteTerminate(const CThostFtdcQuoteField & quote_field);
    void CancelUnterminatedQuotes();
    void ReportErrorState(int api_error_no, const std::string &error_msg);

    // variables for stats cancel times
    std::mutex stats_canceltimes_sync_;
    std::atomic_bool finish_query_canceltimes_;
    std::map<std::string, int> cancel_times_of_contract;
    bool HaveFinishQueryOrders()
    {
        return have_handled_unterminated_orders_ && finish_query_canceltimes_;
    }

    // variables for query all contracts
    std::atomic_bool finish_query_contracts_;
    Contracts all_contract_;

    MYCTPDataHandler *forquote_inf_;

    volatile bool in_init_state_; // clear after login
};

#endif //
