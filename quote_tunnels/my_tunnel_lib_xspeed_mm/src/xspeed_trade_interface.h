#pragma once

#include <string>
#include <sstream>
#include <list>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "DFITCTraderApi.h"
#include "DFITCMdApi.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "config_data.h"
#include "trade_data_type.h"
#include "xspeed_trade_context.h"
#include "my_tunnel_lib.h"
#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"
#include "field_convert.h"
#include "xspeed_data_formater.h"

struct OriginalReqInfo;
class MYMdSpi;

class MYXSpeedSpi: public DFITCXSPEEDAPI::DFITCTraderSpi
{
public:
    MYXSpeedSpi(const TunnelConfigData &cfg);
    virtual ~MYXSpeedSpi(void);

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

    void SetCallbackHandler(std::function<void(const T_RspOfReqForQuote *)> handler)
    {
        RspOfReqForQuoteHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_RtnForQuote *)> handler)
    {
        RtnForQuoteHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_InsertQuoteRespond *)> handler)
    {
        InsertQuoteRespondHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_CancelQuoteRespond *)> handler)
    {
        CancelQuoteRespondHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_QuoteReturn *)> handler)
    {
        QuoteReturnHandler_ = handler;
    }
    void SetCallbackHandler(std::function<void(const T_QuoteTrade *)> handler)
    {
        QuoteTradeHandler_ = handler;
    }

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

    //客户请求登录响应
    virtual void OnRspUserLogin(struct DFITCUserLoginInfoRtnField *pf, struct DFITCErrorRtnField *pe);
    //客户退出请求响应
    virtual void OnRspUserLogout(struct DFITCUserLogoutInfoRtnField *pf, struct DFITCErrorRtnField *pe);
    //委托下单响应
    virtual void OnRspInsertOrder(struct DFITCOrderRspDataRtnField *pf, struct DFITCErrorRtnField *pe);
    //委托撤单响应
    virtual void OnRspCancelOrder(struct DFITCOrderRspDataRtnField *pf, struct DFITCErrorRtnField *pe);
    //持仓查询响应
    virtual void OnRspQryPosition(struct DFITCPositionInfoRtnField *pf, struct DFITCErrorRtnField *pe, bool bIsLast);
    virtual void OnRspQryPositionDetail(struct DFITCPositionDetailRtnField * pf, struct DFITCErrorRtnField * pe, bool bIsLast);
    //客户资金查询响应
    virtual void OnRspCustomerCapital(struct DFITCCapitalInfoRtnField *pf, struct DFITCErrorRtnField *pe, bool bIsLast);
    //交易所合约查询响应
    virtual void OnRspQryExchangeInstrument(struct DFITCExchangeInstrumentRtnField *pf, struct DFITCErrorRtnField *pe, bool bIsLast);
    // 账单确认响应
    virtual void OnRspBillConfirm(struct DFITCBillConfirmRspField * pf, struct DFITCErrorRtnField * pe);
    //错误回报
    virtual void OnRtnErrorMsg(struct DFITCErrorRtnField *pf);
    //成交回报
    virtual void OnRtnMatchedInfo(struct DFITCMatchRtnField *pf);
    //委托回报
    virtual void OnRtnOrder(struct DFITCOrderRtnField *pf);
    //撤单回报
    virtual void OnRtnCancelOrder(struct DFITCOrderCanceledRtnField *pf);

    virtual void OnRspQryOrderInfo(struct DFITCOrderCommRtnField *pf, struct DFITCErrorRtnField * pe, bool bIsLast);
    virtual void OnRspQryMatchInfo(struct DFITCMatchedRtnField *pf, struct DFITCErrorRtnField * pe, bool bIsLast);
    //交易日确认响应:用于接收交易日信息。
    virtual void OnRspTradingDay(struct DFITCTradingDayRtnField * pf);

    // added on 20141216 for support market making
    /**
     * 做市商报单响应
     * @param pRspQuoteData:指向做市商报单响应地址的指针。
     */
    virtual void OnRspQuoteInsert(struct DFITCQuoteRspField * pf, struct DFITCErrorRtnField * pe);

    /**
     * 做市商报单回报
     * @param pRtnQuoteData:指向做市商报单回报地址的指针。
     */
    virtual void OnRtnQuoteInsert(struct DFITCQuoteRtnField * pf);

    /**
     * 做市商成交回报
     * @param pRtnQuoteMatchedData:指向做市商成交回报地址的指针。
     */
    virtual void OnRtnQuoteMatchedInfo(struct DFITCQuoteMatchRtnField * pf);

    /**
     * 做市商撤单响应
     * @param pRspQuoteCanceledData:指向做市商撤单响应地址的指针。
     */
    virtual void OnRspQuoteCancel(struct DFITCQuoteRspField * pf, struct DFITCErrorRtnField * pe);

    /**
     * 做市商撤单回报
     * @param pRtnQuoteCanceledData:指向做市商撤单回报地址的指针。
     */
    virtual void OnRtnQuoteCancel(struct DFITCQuoteCanceledRtnField * pf);

    /**
     * 交易所状态通知
     * @param pRtnExchangeStatusData:指向交易所状态通知地址的指针。
     */
    virtual void OnRtnExchangeStatus(struct DFITCExchangeStatusRtnField * pf);

    /**
     * 批量撤单响应
     * @param pRspStripCancelOrderData:指向批量撤单响应地址的指针。
     */
    virtual void OnRspCancelAllOrder(struct DFITCCancelAllOrderRspField *pf, struct DFITCErrorRtnField * pe);

    /**
     * 询价响应
     * @param pRspForQuoteData:询价请求结构地址。
     * @return 0 - 请求发送成功 -1 - 请求发送失败  -2 -检测异常。
     */
    virtual void OnRspForQuote(struct DFITCForQuoteRspField * pf, struct DFITCErrorRtnField * pe);

    /**
     * 询价回报
     * @param pRspForQuoteData:询价请求结构地址。
     * @return 0 - 请求发送成功 -1 - 请求发送失败  -2 -检测异常。
     */
    virtual void OnRtnForQuote(struct DFITCForQuoteRtnField * pf);

    /**
     * 查询当日报价委托响应:当用户发出做市商报价委托查询后，该方法会被调用。
     * @param pRtnQuoteOrderData:指向报价查询回报结构的地址。
     * @param bIsLast:表明是否是最后一条响应信息（0 -否   1 -是）。
     */
    virtual void OnRspQryQuoteOrderInfo(struct DFITCQuoteOrderRtnField * pf, struct DFITCErrorRtnField * pe, bool bIsLast);

    // following interface, moved from tradeapi to mdapi in version XSpeedAPI_V1.0.9.14_sp4
    void OnMDFrontConnected();
    void OnMDFrontDisconnected(int reason);
    void OnMDRspUserLogin(struct DFITCUserLoginInfoRtnField * pf, struct DFITCErrorRtnField * rsp);
    void OnMDRspUserLogout(struct DFITCUserLogoutInfoRtnField * pf, struct DFITCErrorRtnField * rsp);
    void OnMDRspError(struct DFITCErrorRtnField *rsp);
    // 订阅询价应答
    void OnMDRspSubForQuoteRsp(struct DFITCSpecificInstrumentField * pf, struct DFITCErrorRtnField * rsp);
    // 取消订阅询价应答
    void OnMDRspUnSubForQuoteRsp(struct DFITCSpecificInstrumentField * pf, struct DFITCErrorRtnField * rsp);
    // 询价通知
    void OnMDRtnForQuoteRsp(struct DFITCQuoteSubscribeRtnField * pf);

public:
    // 下发指令接口
    int ReqOrderInsert(DFITCInsertOrderField *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqInsertOrder(p);
            TNL_LOG_DEBUG("ReqInsertOrder - ret=%d - %s", ret, XSpeedDatatypeFormater::ToString(p).c_str());
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
    int ReqOrderAction(DFITCCancelOrderField *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderAction when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqCancelOrder(p);
            TNL_LOG_DEBUG("ReqCancelOrder - ret=%d - %s", ret, XSpeedDatatypeFormater::ToString(p).c_str());
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
            TNL_LOG_ERROR("unknown exception in ReqCancelOrder.");
        }

        return ret;
    }

    int QryPosition(const T_QryPosition *pQryPosition)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryPosition when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        if (pos_qry_step != 0)
        {
            TNL_LOG_WARN("QryPosition is executing now, need wait till it finished, pos_qry_step(0-init;1-fut;2-opt): %d", pos_qry_step.load());
            return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
        }

        position_detail_.clear();

        DFITCPositionDetailField qry_param;
        memset(&qry_param, 0, sizeof(DFITCPositionField));
        strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
        qry_param.instrumentType = DFITC_COMM_TYPE;

        pos_qry_step = 1;
        int ret = api_->ReqQryPositionDetail(&qry_param);
        if (ret != 0)
        {
            pos_qry_step = 0;
        }

        TNL_LOG_DEBUG("ReqQryPosition - ret=%d - %s", ret, XSpeedDatatypeFormater::ToString(&qry_param).c_str());

        return ret;
    }
    int QryOrderDetail(const T_QryOrderDetail *pQryParam)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryOrderDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        if (order_qry_step != 0)
        {
            TNL_LOG_WARN("QryOrderDetail is executing now, need wait till it finished, pos_qry_step(0-init;1-fut;2-opt): %d", order_qry_step.load());
            return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
        }

        order_detail_.clear();

        DFITCOrderField qry_param;
        memset(&qry_param, 0, sizeof(DFITCOrderField));
        strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
        qry_param.instrumentType = DFITC_COMM_TYPE;

        order_qry_step = 1;
        int ret = api_->ReqQryOrderInfo(&qry_param);
        if (ret != 0)
        {
            order_qry_step = 0;
        }

        TNL_LOG_DEBUG("ReqQryOrderInfo - ret=%d - %s", ret, XSpeedDatatypeFormater::ToString(&qry_param).c_str());
        return ret;
    }

    int QryContractInfo(const T_QryContractInfo* pQryParam)
    {
		if (!TunnelIsReady())
		{
			TNL_LOG_WARN("QryContractInfo when tunnel not ready");
			return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
		}
		int ret = TUNNEL_ERR_CODE::RESULT_FAIL;
		DFITCExchangeInstrumentField data;
		memset(&data, 0, sizeof(data));
		strncpy(data.accountID, tunnel_info_.account.c_str(), strlen(data.accountID));
		strcpy(data.exchangeID, DFITC_EXCHANGE_DCE);
		data.instrumentType = DFITC_OPT_TYPE;
		ret = api_->ReqQryExchangeInstrument(&data);

		return ret;
    }

    int QryTradeDetail(const T_QryTradeDetail *pQryParam)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryTradeDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        if (trade_qry_step != 0)
        {
            TNL_LOG_WARN("ReqQryMatchInfo is executing now, need wait till it finished, pos_qry_step(0-init;1-fut;2-opt): %d", trade_qry_step.load());
            return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
        }

        trade_detail_.clear();

        DFITCMatchField qry_param;
        memset(&qry_param, 0, sizeof(DFITCMatchField));
        strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
        qry_param.instrumentType = DFITC_COMM_TYPE;

        trade_qry_step = 1;
        int ret = api_->ReqQryMatchInfo(&qry_param);
        if (ret != 0)
        {
            trade_qry_step = 0;
        }

        TNL_LOG_DEBUG("ReqQryMatchInfo - ret=%d - %s", ret, XSpeedDatatypeFormater::ToString(&qry_param).c_str());

        return ret;
    }

    ///询价录入请求
    int ReqForQuoteInsert(DFITCForQuoteField *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqForQuoteInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqForQuote(p);
            TNL_LOG_DEBUG("ReqForQuote - ret=%d - %s", ret, XSpeedDatatypeFormater::ToString(p).c_str());
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
            TNL_LOG_ERROR("unknown exception in ReqForQuote.");
        }

        return ret;
    }

    ///报价录入请求
    int ReqQuoteInsert(DFITCQuoteInsertField *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqQuoteInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqQuoteInsert(p);
            TNL_LOG_DEBUG("ReqQuoteInsert - ret=%d - %s", ret, XSpeedDatatypeFormater::ToString(p).c_str());
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
    int ReqQuoteAction(DFITCCancelOrderField *p)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqQuoteAction when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqQuoteCancel(p);
            TNL_LOG_DEBUG("ReqQuoteCancel - ret=%d - %s", ret, XSpeedDatatypeFormater::ToString(p).c_str());
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
            TNL_LOG_ERROR("unknown exception in ReqQuoteCancel.");
        }

        return ret;
    }

    bool TunnelIsReady()
    {
        return is_ready_;
    }
    int Session_id() const
    {
        return session_id_;
    }
    void StartPositionBackUp();


    // 外部接口对象使用，为避免修改接口，新增对象放到此处
    XSpeedTradeContext xspeed_trade_context_;
    std::mutex client_sync;

    std::mutex rsp_sync;
    std::condition_variable rsp_con;

private:
    DFITCXSPEEDAPI::DFITCTraderApi *api_;
    long session_id_;
    long max_order_ref_;
    DFITCDateType trade_day;

    bool ParseConfig();
    void ReqLogin();
    void SaveToFile();

    Tunnel_Info tunnel_info_;
    std::string pswd_;
    std::string quote_addr_;
    std::string exchange_code_;

    std::function<void(const T_OrderRespond *)> OrderRespond_call_back_handler_;
    std::function<void(const T_CancelRespond *)> CancelRespond_call_back_handler_;
    std::function<void(const T_OrderReturn *)> OrderReturn_call_back_handler_;
    std::function<void(const T_TradeReturn *)> TradeReturn_call_back_handler_;

    std::function<void(const T_PositionReturn *)> QryPosReturnHandler_;
    std::function<void(const T_OrderDetailReturn *)> QryOrderDetailReturnHandler_;
    std::function<void(const T_TradeDetailReturn *)> QryTradeDetailReturnHandler_;
    std::function<void(const T_ContractInfoReturn *)> QryContractInfoHandler_;

    // added for support market making interface
    std::function<void(const T_RspOfReqForQuote *)> RspOfReqForQuoteHandler_;
    std::function<void(const T_RtnForQuote *)> RtnForQuoteHandler_;
    std::function<void(const T_InsertQuoteRespond *)> InsertQuoteRespondHandler_;
    std::function<void(const T_CancelQuoteRespond *)> CancelQuoteRespondHandler_;
    std::function<void(const T_QuoteReturn *)> QuoteReturnHandler_;
    std::function<void(const T_QuoteTrade *)> QuoteTradeHandler_;

    // 配置数据对象
    TunnelConfigData cfg_;
    volatile bool connected_;
    std::atomic_bool logoned_;
    std::atomic_bool backup_;
    std::vector<DFITCPositionDetailRtnField> pos_info_;
    bool is_ready_;
    std::vector<ContractInfo> contract_info_;

    // query position control variables
    std::vector<PositionDetail> position_detail_;
    std::atomic_int pos_qry_step; // 0:init; 1:query future; 2:query option
    void QueryOptionPosition();

    void CheckAndSaveYestodayPosition();
    void LoadYestodayPositionFromFile(const std::string &file);
    void SaveYestodayPositionToFile(const std::string &file);

    // query order detail control variables
    std::vector<OrderDetail> order_detail_;
    std::atomic_int order_qry_step; // 0:init; 1:query future; 2:query option
    void QueryOptionOrder();
    void ReportErrorState(int api_error_no, const std::string &error_msg);

    // query trade detail control variables
    std::vector<TradeDetail> trade_detail_;
    std::atomic_int trade_qry_step; // 0:init; 1:query future; 2:query option
    void QueryOptionTrade();

    // variables and functions for cancel all unterminated orders automatically
    std::atomic_bool have_handled_unterminated_orders_;
    std::mutex terminate_order_sync_;

    std::mutex stats_canceltimes_sync_;
    std::atomic_bool finish_query_canceltimes_;
    std::map<std::string, int> cancel_times_of_contract;

    std::condition_variable qry_order_finish_cond_;
    std::vector<DFITCOrderCommRtnField> unterminated_orders_;
    std::vector<DFITCQuoteOrderRtnField> unterminated_quotes_;
    void ProcessAfterGetAllQuotes();

    bool HaveFinishQueryOrders()
    {
        return have_handled_unterminated_orders_ && finish_query_canceltimes_;
    }
    void QueryOrdersForInitial();
    void HandleQueryOrders(struct DFITCOrderCommRtnField *pf, struct DFITCErrorRtnField * pe, bool bIsLast);
    bool IsOrderTerminate(struct DFITCOrderCommRtnField *pf);
    bool IsQuoteTerminate(struct DFITCQuoteOrderRtnField *pf);
    bool IsCancelOrder(struct DFITCOrderCommRtnField *pf);

    volatile bool in_init_state_; // clear after login

    // md interface
    MYMdSpi *md_spi;
    DFITCXSPEEDMDAPI::DFITCMdApi *md_api;
    bool md_in_init_stat;
    void InitMDInterface();
    void MDReqLogin();
};


class MYMdSpi: public DFITCXSPEEDMDAPI::DFITCMdSpi
{
public:
    MYMdSpi(MYXSpeedSpi *trade_inf)
        : pt_(trade_inf)
    {
    }
    virtual ~MYMdSpi()
    {
    }

    virtual void OnFrontConnected()
    {
        return pt_->OnMDFrontConnected();
    }
    virtual void OnFrontDisconnected(int reason)
    {
        return pt_->OnMDFrontDisconnected(reason);
    }
    virtual void OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pf, struct DFITCErrorRtnField * rsp)
    {
        return pt_->OnMDRspUserLogin(pf, rsp);
    }
    virtual void OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pf, struct DFITCErrorRtnField * rsp)
    {
        return pt_->OnMDRspUserLogout(pf, rsp);
    }
    virtual void OnRspError(struct DFITCErrorRtnField *rsp)
    {
        return pt_->OnMDRspError(rsp);
    }

    // 订阅询价应答
    virtual void OnRspSubForQuoteRsp(struct DFITCSpecificInstrumentField * pf, struct DFITCErrorRtnField * rsp)
    {
        return pt_->OnMDRspSubForQuoteRsp(pf, rsp);
    }
    virtual void OnRspUnSubForQuoteRsp(struct DFITCSpecificInstrumentField * pf, struct DFITCErrorRtnField * rsp)
    {
        return pt_->OnMDRspUnSubForQuoteRsp(pf, rsp);
    }
    // 询价通知
    virtual void OnRtnForQuoteRsp(struct DFITCQuoteSubscribeRtnField * pf)
    {
        return pt_->OnMDRtnForQuoteRsp(pf);
    }

private:
    MYXSpeedSpi *pt_;
};
