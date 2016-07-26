#ifndef XELE_TRADE_INTERFACE_H_
#define XELE_TRADE_INTERFACE_H_

#include <string>
#include <sstream>
#include <list>
#include <set>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "CXeleTraderApi.hpp"
#include "config_data.h"
#include "trade_data_type.h"
#include "my_tunnel_lib.h"
#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"
#include "xele_trade_context.h"
#include "field_convert.h"
#include "xele_data_formater.h"

struct OriginalReqInfo;

class MyXeleTradeSpi: public CXeleTraderSpi
{
public:
    MyXeleTradeSpi(const TunnelConfigData &cfg);
    virtual ~MyXeleTradeSpi(void);

    void SetCallbackHandler(std::function<void(const T_OrderRespond *)> handler);
    void SetCallbackHandler(std::function<void(const T_CancelRespond *)> handler);
    void SetCallbackHandler(std::function<void(const T_OrderReturn *)> handler);
    void SetCallbackHandler(std::function<void(const T_TradeReturn *)> handler);

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

    ///报文回调开始通知。当API收到一个报文后，首先调用本方法，然后是各数据域的回调，最后是报文回调结束通知。
    ///@param nTopicID 主题代码（如私有流、公共流、行情流等）
    ///@param nSequenceNo 报文序号
    virtual void OnPackageStart(int nTopicID, int nSequenceNo);

    ///报文回调结束通知。当API收到一个报文后，首先调用报文回调开始通知，然后是各数据域的回调，最后调用本方法。
    ///@param nTopicID 主题代码（如私有流、公共流、行情流等）
    ///@param nSequenceNo 报文序号
    virtual void OnPackageEnd(int nTopicID, int nSequenceNo);

    ///错误应答
    virtual void OnRspError(CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

    ///用户登录应答
    virtual void OnRspUserLogin(CXeleFtdcRspUserLoginField *pRspUserLogin,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

    ///用户退出应答
    virtual void OnRspUserLogout(CXeleFtdcRspUserLogoutField *pRspUserLogout,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

    ///报单录入应答
    virtual void OnRspOrderInsert(CXeleFtdcInputOrderField *pInputOrder,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

    ///报单操作应答
    virtual void OnRspOrderAction(CXeleFtdcOrderActionField *pOrderAction,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

//    ///会员资金查询应答
//    virtual void OnRspQryPartAccount(CXeleFtdcRspPartAccountField *pRspPartAccount,
//        CXeleFtdcRspInfoField *pRspInfo,
//        int nRequestID,
//        bool bIsLast);

    ///报单查询应答
    virtual void OnRspQryOrder(CXeleFtdcOrderField *pOrder,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

    ///成交单查询应答
    virtual void OnRspQryTrade(CXeleFtdcTradeField *pTrade,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

//    ///会员客户查询应答
//    virtual void OnRspQryClient(CXeleFtdcRspClientField *pRspClient,
//        CXeleFtdcRspInfoField *pRspInfo,
//        int nRequestID,
//        bool bIsLast);
//
//    ///会员持仓查询应答
//    virtual void OnRspQryPartPosition(CXeleFtdcRspPartPositionField *pRspPartPosition,
//        CXeleFtdcRspInfoField *pRspInfo,
//        int nRequestID,
//        bool bIsLast);

    ///客户持仓查询应答
    virtual void OnRspQryClientPosition(CXeleFtdcRspClientPositionField *pRspClientPosition,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

    ///合约查询应答
    virtual void OnRspQryInstrument(CXeleFtdcRspInstrumentField *pRspInstrument,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

    ///合约交易状态查询应答
    virtual void OnRspQryInstrumentStatus(CXeleFtdcInstrumentStatusField *pInstrumentStatus,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

    ///主题查询应答
    virtual void OnRspQryTopic(CXeleFtdcDisseminationField *pDissemination,
        CXeleFtdcRspInfoField *pRspInfo,
        int nRequestID,
        bool bIsLast);

    ///成交回报
    virtual void OnRtnTrade(CXeleFtdcTradeField *pTrade);

    ///报单回报
    virtual void OnRtnOrder(CXeleFtdcOrderField *pOrder);

    ///合约交易状态通知
    virtual void OnRtnInstrumentStatus(CXeleFtdcInstrumentStatusField *pInstrumentStatus);

    ///增加合约通知
    virtual void OnRtnInsInstrument(CXeleFtdcInstrumentField *pInstrument);

    ///删除合约通知
    virtual void OnRtnDelInstrument(CXeleFtdcInstrumentField *pInstrument);

    ///报单录入错误回报
    virtual void OnErrRtnOrderInsert(CXeleFtdcInputOrderField *pInputOrder,
        CXeleFtdcRspInfoField *pRspInfo);

    ///报单操作错误回报
    virtual void OnErrRtnOrderAction(CXeleFtdcOrderActionField *pOrderAction,
        CXeleFtdcRspInfoField *pRspInfo);

public:
    // 下发指令接口
    int ReqOrderInsert(CXeleFtdcInputOrderField *po, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqOrderInsert(po, request_id);

            TNL_LOG_DEBUG("ReqOrderInsert - ret:%d, req_id:%d - \n%s", ret, request_id,
                XeleDatatypeFormater::ToString(po).c_str());

            if (ret != 0)
            {
                // -2，表示未处理请求超过许可数；
                // -3，表示每秒发送请求数超过许可数。
                if (ret == -2)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST;
                }
                else if (ret == -3)
                {
                    return TUNNEL_ERR_CODE::CFFEX_OVER_REQUEST_PER_SECOND;
                }
                else
                {
                    return TUNNEL_ERR_CODE::RESULT_FAIL;
                }
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqOrderInsert.");
        }

        return ret;
    }

    //报单操作请求
    int ReqOrderAction(CXeleFtdcOrderActionField *co, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderAction when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqOrderAction(co, request_id);

            TNL_LOG_DEBUG("ReqOrderAction - ret:%d, req_id:%d - \n%s", ret, request_id,
                XeleDatatypeFormater::ToString(co).c_str());

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

    // query handle
    int QryPosition(CXeleFtdcQryClientPositionField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryPosition when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = api_->ReqQryClientPosition(p, request_id);

        TNL_LOG_DEBUG("ReqQryClientPosition - ret:%d, req_id:%d - \n%s", ret, request_id,
            XeleDatatypeFormater::ToString(p).c_str());

        return ret;
    }

//    // query handle
//    int QryPositionPart(CXeleFtdcQryPartPositionField *p, int request_id)
//    {
//        if (!TunnelIsReady())
//        {
//            TNL_LOG_WARN("QryPosition when tunnel not ready");
//            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
//        }
//
//        int ret = api_->ReqQryPartPosition(p, request_id);
//
//        TNL_LOG_DEBUG("ReqQryPartPosition - ret:%d, req_id:%d - \n%s", ret, request_id,
//            XeleDatatypeFormater::ToString(p).c_str());
//
//        return ret;
//    }

    int QryOrderDetail(CXeleFtdcQryOrderField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryOrderDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

//        int ret = api_->ReqQryOrder(p, request_id);
//
//        TNL_LOG_DEBUG("ReqQryOrder - ret:%d, req_id:%d - \n%s", ret, request_id,
//            XeleDatatypeFormater::ToString(p).c_str());

        int ret = TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION_NO;
        return ret;
    }
    int QryTradeDetail(CXeleFtdcQryTradeField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryTradeDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

//        int ret = api_->ReqQryTrade(p, request_id);
//
//        TNL_LOG_DEBUG("ReqQryTrade - ret:%d, req_id:%d - \n%s", ret, request_id,
//            XeleDatatypeFormater::ToString(p).c_str());

        int ret = TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION_NO;
        return ret;
    }

    XeleTradeContext xele_trade_context_;

    // 外部接口对象使用，为避免修改接口，新增对象放到此处
    std::mutex client_sync;

    std::mutex rsp_sync;
    std::condition_variable rsp_con;

private:
    bool TunnelIsReady()
    {
        return logoned_ && HaveFinishQueryOrders();
    }
    bool HaveFinishQueryOrders()
    {
        return have_handled_unterminated_orders_ && finish_query_canceltimes_;
    }

    void HandleFillupRsp(long entrust_no, SerialNoType serial_no);
    void HandleFillupRtn(long entrust_no, char order_status, const XeleOriginalReqInfo *p);

    void ReqLogin();

    CXeleTraderApi *api_;
    OrderRefDataType max_order_ref_;

    Tunnel_Info tunnel_info_;
    std::string user_;
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

    // 查询报单
    std::thread *qry_order_t_;
    void QueryAndHandleOrders();
    void ReportErrorState(int api_error_no, const std::string &error_msg);

    // variables and functions for cancel all unterminated orders automatically
    std::atomic_bool have_handled_unterminated_orders_;
    std::vector<CXeleFtdcOrderField> unterminated_orders_;
    std::mutex cancel_sync_;
    std::condition_variable qry_order_finish_cond_;
    CXeleFtdcOrderActionField CreatCancelParam(const CXeleFtdcOrderField & order_field);
    bool IsOrderTerminate(const CXeleFtdcOrderField & order_field);

    std::mutex stats_canceltimes_sync_;
    std::atomic_bool finish_query_canceltimes_;
    std::unordered_map<std::string, int> cancel_times_of_contract;

    volatile bool in_init_state_; // clear after login
};

#endif //