#ifndef REM_TRADE_INTERFACE_H_
#define REM_TRADE_INTERFACE_H_

#include <string>
#include <sstream>
#include <list>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "EesTraderApi.h"

#include "config_data.h"
#include "trade_data_type.h"
#include "rem_trade_context.h"
#include "rem_tunnel_lib.h"
#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"
#include "rem_field_convert.h"
#include "rem_struct_convert.h"
#include "rem_struct_formater.h"

class MYRemTradeSpi: public EESTraderEvent
{
public:
    MYRemTradeSpi(const TunnelConfigData &cfg);
    virtual ~MYRemTradeSpi(void);

    // interfaces of EESTraderEvent
    virtual void OnConnection(ERR_NO errNo, const char* pErrStr);
    virtual void OnDisConnection(ERR_NO errNo, const char* pErrStr);
    virtual void OnUserLogon(EES_LogonResponse* pLogon);
    virtual void OnQueryUserAccount(EES_AccountInfo * pAccoutnInfo, bool bFinish);
    virtual void OnQueryAccountPosition(const char* pAccount, EES_AccountPosition* pAccoutnPosition, int nReqId, bool bFinish);
    virtual void OnQueryAccountBP(const char* pAccount, EES_AccountBP* pAccoutnPosition, int nReqId);
    virtual void OnQuerySymbol(EES_SymbolField* pSymbol, bool bFinish);
    virtual void OnOrderAccept(EES_OrderAcceptField* pAccept);
    virtual void OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept);
    virtual void OnOrderReject(EES_OrderRejectField* pReject);
    virtual void OnOrderMarketReject(EES_OrderMarketRejectField* pReject);
    virtual void OnOrderExecution(EES_OrderExecutionField* pExec);
    virtual void OnOrderCxled(EES_OrderCxled* pCxled);
    virtual void OnCxlOrderReject(EES_CxlOrderRej* pReject);
    virtual void OnQueryTradeOrder(const char* pAccount, EES_QueryAccountOrder* pQueryOrder, bool bFinish);
    virtual void OnQueryTradeOrderExec(const char* pAccount, EES_QueryOrderExecution* pQueryOrderExec, bool bFinish);
    virtual void OnPostOrder(EES_PostOrder* pPostOrder);
    virtual void OnPostOrderExecution(EES_PostOrderExecution* pPostOrderExecution);
    virtual void OnQueryMarketSession(EES_ExchangeMarketSession* pMarketSession, bool bFinish);
    virtual void OnMarketSessionStatReport(EES_MarketSessionId MarketSessionId, bool ConnectionGood);
    virtual void OnSymbolStatusReport(EES_SymbolStatus* pSymbolStatus);
    virtual void OnQuerySymbolStatus(EES_SymbolStatus* pSymbolStatus, bool bFinish);

    void SetCallbackHandler(std::function<void(const T_OrderRespond *)> handler)
    {
        OrderRespond_call_back_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_CancelRespond *)> handler)
    {
        CancelRespond_call_back_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_OrderReturn *)> handler)
    {
        OrderReturn_call_back_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_TradeReturn *)> handler)
    {
        TradeReturn_call_back_handler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler)
    {
        QryPosReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler)
    {
        QryOrderDetailReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
    {
        QryTradeDetailReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_ContractInfoReturn *)> handler)
    {
        QryContractInfoHandler_ = handler;
    }

    // 下发指令接口
    int ReqOrderInsert(const T_PlaceOrder *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            std::lock_guard<std::mutex> lock(req_sequential_protect_);

            // convert to rem structure
            EES_EnterOrderField req;
            bzero(&req, sizeof(req));
            strncpy(req.m_Account, tunnel_info_.account.c_str(), sizeof(req.m_Account));           ///< 用户代码
            req.m_Side = RemFieldConvert::GetRemSide(p->direction, p->open_close);  ///< 买卖方向
            req.m_Exchange = EES_ExchangeID_cffex;                                  ///< 交易所 (cffex)
            strncpy(req.m_Symbol, p->stock_code, sizeof(req.m_Symbol));             ///< 合约代码
            req.m_SecType = EES_SecType_fut;                                        ///< 交易品种
            req.m_Price = p->limit_price;                                           ///< 价格
            req.m_Qty = p->volume;                                                  ///< 数量
            req.m_ForceCloseReason = EES_ForceCloseType_not_force_close;            ///< 强平原因
            req.m_ClientOrderToken = trade_context_.GetNewClientToken();            ///< 整型，必须保证，这次比上次的值大，并不一定需要保证连续
            req.m_Tif = RemFieldConvert::GetRemCondition(p->order_type);            ///< 当需要下FAK/FOK报单时，需要设置为EES_OrderTif_IOC
            req.m_MinQty = 0;                                                       ///< 当需要下FAK/FOK报单时，该值=0：映射交易所的FAK-任意数量；
            if (p->order_type == MY_TNL_HF_FOK) req.m_MinQty = p->volume;
            //req.m_CustomField;                                                    ///< 用户自定义字段，8个字节。用户在下单时指定的值，将会在OnOrderAccept，OnQueryTradeOrder事件中返回
            //req.m_MarketSessionId;                                                ///< 交易所席位代码，从OnResponseQueryMarketSessionId获取合法值，如果填入0或者其他非法值，REM系统将自行决定送单的席位
            req.m_HedgeFlag = RemFieldConvert::GetRemHedgeFlag(p->speculator);      ///< 投机套利标志

            // save req info
            trade_context_.SaveReqInfo(new RemReqInfo(*p, req.m_ClientOrderToken));

            // send request to counter
            ret = api_->EnterOrder(&req);
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in EnterOrder.");
        }

        return ret;
    }

    // 报单操作请求
    int ReqOrderAction(const T_CancelOrder *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderAction when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            // prepare request structure
            EES_CancelOrder cancel_order;
            bzero(&cancel_order, sizeof(cancel_order));
            strncpy(cancel_order.m_Account, tunnel_info_.account.c_str(), sizeof(cancel_order.m_Account));
            cancel_order.m_MarketOrderToken = trade_context_.GetMarketOrderTokenOfSn(p->org_serial_no);
            cancel_order.m_Quantity = 0;

            // save cancel req info
            trade_context_.SaveCancelInfo(cancel_order.m_MarketOrderToken, p->serial_no);

            // send request to counter
            ret = api_->CancelOrder(&cancel_order);
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in CancelOrder.");
        }

        return ret;
    }

    int QryPosition(const T_QryPosition *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryPosition when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        ret = api_->QueryAccountPosition(tunnel_info_.account.c_str(), 0);
        TNL_LOG_INFO("QueryAccountPosition - return:%d", ret);

        return ret;
    }
    int QryOrderDetail(const T_QryOrderDetail *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryOrderDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;
        ret = api_->QueryAccountOrder(tunnel_info_.account.c_str());
        TNL_LOG_INFO("QueryAccountOrder - return:%d", ret);

        return ret;
    }
    int QryTradeDetail(const T_QryTradeDetail *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryTradeDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;
        ret = api_->QueryAccountOrderExecution(tunnel_info_.account.c_str());
        TNL_LOG_INFO("QueryAccountOrderExecution - return:%d", ret);

        return ret;
    }
    int QryInstrument(const T_QryContractInfo *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryInstrument when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = api_->QuerySymbolList();
        TNL_LOG_INFO("QuerySymbolList - return:%d", ret);
        return ret;
    }

private:
    void ReqLogin();

    bool TunnelIsReady()
    {
        return logoned_ && HaveFinishQueryOrders();
    }
    bool HaveFinishQueryOrders()
    {
        return have_handled_unterminated_orders_ && finish_query_canceltimes_ && finish_query_orders_;
    }

public:
    RemTradeContext trade_context_;

    // 外部接口对象使用，为避免修改接口，新增对象放到此处
    std::mutex rsp_sync;
    std::condition_variable rsp_con;

private:
    EESTraderApi *api_;

    Tunnel_Info tunnel_info_;
    std::string user_;
    std::string pswd_;
    std::string exchange_code_;
    std::string mac_addr_;
    std::string program_name_;
    double available_fund;
    int trade_day;  // 交易日，格式为yyyyMMdd的int型值 (get after login)

    std::mutex req_sequential_protect_;
    void QueryAccount();
    PositionDetail PackageAvailableFund();

    std::function<void(const T_OrderRespond *)> OrderRespond_call_back_handler_;
    std::function<void(const T_CancelRespond *)> CancelRespond_call_back_handler_;
    std::function<void(const T_OrderReturn *)> OrderReturn_call_back_handler_;
    std::function<void(const T_TradeReturn *)> TradeReturn_call_back_handler_;

    std::function<void(const T_PositionReturn *)> QryPosReturnHandler_;
    std::function<void(const T_OrderDetailReturn *)> QryOrderDetailReturnHandler_;
    std::function<void(const T_TradeDetailReturn *)> QryTradeDetailReturnHandler_;
    std::function<void(const T_ContractInfoReturn *)> QryContractInfoHandler_;

    // 配置数据对象
    TunnelConfigData cfg_;
    volatile bool connected_;
    std::atomic_bool logoned_;

    // query position control variables
    std::vector<PositionDetail> pos_buffer_;

    // query order detail control variables
    std::vector<OrderDetail> od_buffer_;

    // query trade detail control variables
    std::vector<TradeDetail> td_buffer_;

    // query contract info control variables
    std::vector<ContractInfo> ci_buffer_;

    // 查询报单
    void QueryAndHandleOrders();
    void ReportErrorState(int api_error_no, const std::string &error_msg);

    // variables and functions for cancel all unterminated orders automatically
    std::atomic_bool have_handled_unterminated_orders_;
    std::vector<EES_MarketToken> unterminated_orders_;
    std::mutex cancel_sync_;
    std::condition_variable qry_order_finish_cond_;
    EES_CancelOrder CreatCancelParam(EES_MarketToken market_token);
    bool IsOrderTerminate(const EES_QueryAccountOrder & order_field);
    bool IsClientCanceledOrder(const EES_QueryAccountOrder & order_field);
    bool IsExecutedOrder(const EES_QueryAccountOrder & order_field);

    std::mutex stats_canceltimes_sync_;
    std::atomic_bool finish_query_canceltimes_;
    typedef std::unordered_map<std::string, int> ContractCancelTimesMap;
    ContractCancelTimesMap cancel_times_of_contract;

    std::atomic_bool finish_query_orders_;
    typedef std::unordered_map<EES_MarketToken, EES_QueryAccountOrder> OrderOfMTokenMap;
    OrderOfMTokenMap executed_orders_;

    volatile bool in_init_state_; // clear after login
};

#endif //
