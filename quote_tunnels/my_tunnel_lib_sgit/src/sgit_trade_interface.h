#pragma once

#include <string>
#include <sstream>
#include <list>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "SgitFtdcTraderApi.h"

#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"
#include "trade_data_type.h"
#include "config_data.h"
#include "my_tunnel_lib.h"
#include "sgit_trade_context.h"
#include "sgit_field_convert.h"
#include "sgit_data_formater.h"

#include "sgit_tunnel_initializer.h"

struct OriginalReqInfo;

class SgitTradeSpi: public CSgitFtdcTraderSpi
{
public:
    SgitTradeSpi(const TunnelConfigData &cfg);
    virtual ~SgitTradeSpi(void);

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

    void ParseConfig();

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
    virtual void OnRspUserLogin(CSgitFtdcRspUserLoginField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///登出请求响应
    virtual void OnRspUserLogout(CSgitFtdcUserLogoutField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///报单录入请求响应
    virtual void OnRspOrderInsert(CSgitFtdcInputOrderField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///报单操作请求响应
    virtual void OnRspOrderAction(CSgitFtdcInputOrderActionField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///投资者结算结果确认响应
    virtual void OnRspSettlementInfoConfirm(CSgitFtdcSettlementInfoConfirmField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///请求查询投资者结算结果响应
    virtual void OnRspQrySettlementInfo(CSgitFtdcSettlementInfoField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///请求查询结算信息确认响应
    virtual void OnRspQrySettlementInfoConfirm(CSgitFtdcSettlementInfoConfirmField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///错误应答
    virtual void OnRspError(CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///报单通知
    virtual void OnRtnOrder(CSgitFtdcOrderField *pf, CSgitFtdcRspInfoField *rsp);

    ///成交通知
    virtual void OnRtnTrade(CSgitFtdcTradeField *pf);

    ///报单录入错误回报
    virtual void OnErrRtnOrderInsert(CSgitFtdcInputOrderField *pf, CSgitFtdcRspInfoField *rsp);

    ///报单操作错误回报
    virtual void OnErrRtnOrderAction(CSgitFtdcOrderActionField *pf, CSgitFtdcRspInfoField *rsp);

    ///请求查询报单响应
    virtual void OnRspQryOrder(CSgitFtdcOrderField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///请求查询成交响应
    virtual void OnRspQryTrade(CSgitFtdcTradeField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///请求查询投资者持仓响应
    virtual void OnRspQryInvestorPosition(CSgitFtdcInvestorPositionField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///请求查询投资者持仓明细响应
    virtual void OnRspQryInvestorPositionDetail(CSgitFtdcInvestorPositionDetailField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///请求查询合约响应
    virtual void OnRspQryInstrument(CSgitFtdcInstrumentField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    ///请求查询资金账户响应
    virtual void OnRspQryTradingAccount(CSgitFtdcTradingAccountField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);

    // 下发指令接口
    int ReqOrderInsert(CSgitFtdcInputOrderField* pf, int req_id);

    //报单操作请求
    int ReqOrderAction(CSgitFtdcInputOrderActionField* pf, int req_id);

    int QryPosition(CSgitFtdcQryInvestorPositionField* p, int request_id);

    int QryOrderDetail(CSgitFtdcQryOrderField* p, int request_id);

    int QryTradeDetail(CSgitFtdcQryTradeField* p, int request_id);

    int QryInstrument(CSgitFtdcQryInstrumentField* p, int request_id);

    bool TunnelIsReady()
    {
        return logoned_ && tunnel_initializer_ && tunnel_initializer_->Finished();
    }

    TradeContext trade_context_;
    std::mutex client_sync;

    // 外部接口对象使用，为避免修改接口，新增对象放到此处
    std::mutex rsp_sync;
    std::condition_variable rsp_con;

private:
    CSgitFtdcTraderApi *api_;
    TunnelInitializer *tunnel_initializer_;
    int max_order_ref_;

    // variable for the session
    std::string user_;
    std::size_t user_id_len_;
    Tunnel_Info tunnel_info_;
    std::string pswd_;
    std::string quote_addr_;
    TSgitFtdcDateType trade_day;
    char exchange_code_;

    bool IsMineReturn(const char *user_id);
    void ReportErrorState(int api_error_no, const std::string &error_msg);

    void ReqLogin();
    PositionDetail PackageAvailableFunc();

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
    volatile bool in_init_state_; // clear after login
};

inline int SgitTradeSpi::ReqOrderInsert(CSgitFtdcInputOrderField* pf, int req_id)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("ReqOrderInsert when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }

    int ret = TUNNEL_ERR_CODE::RESULT_FAIL;
    try
    {
        ret = api_->ReqOrderInsert(pf, req_id);
        TNL_LOG_DEBUG("ReqOrderInsert, return:%d,  %s", ret, DatatypeFormater::ToString(pf).c_str());
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

inline int SgitTradeSpi::ReqOrderAction(CSgitFtdcInputOrderActionField* pf, int req_id)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("ReqOrderAction when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }

    int ret = TUNNEL_ERR_CODE::RESULT_FAIL;
    try
    {
        ret = api_->ReqOrderAction(pf, req_id);
        TNL_LOG_DEBUG("ReqOrderAction, return:%d  %s", ret, DatatypeFormater::ToString(pf).c_str());
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

inline int SgitTradeSpi::QryPosition(CSgitFtdcQryInvestorPositionField* p, int request_id)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("QryPosition when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    // get data from initializer
    T_PositionReturn query_result;
    const std::vector<PositionDetail>& init_buf = tunnel_initializer_->GetPosition();
    query_result.datas.assign(init_buf.begin(), init_buf.end());
    query_result.error_no = 0;
    if (QryPosReturnHandler_)
    {
        QryPosReturnHandler_(&query_result);
    }
    LogUtil::OnPositionRtn(&query_result, tunnel_info_);
    return 0;
}

inline int SgitTradeSpi::QryOrderDetail(CSgitFtdcQryOrderField* p, int request_id)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("QryOrderDetail when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    // get data from initializer
    T_OrderDetailReturn query_result;
    const std::vector<OrderDetail>& init_buf = tunnel_initializer_->GetOrderInfo();
    query_result.datas.assign(init_buf.begin(), init_buf.end());
    query_result.error_no = 0;
    if (QryOrderDetailReturnHandler_)
    {
        QryOrderDetailReturnHandler_(&query_result);
    }
    LogUtil::OnOrderDetailRtn(&query_result, tunnel_info_);
    return 0;
}

inline int SgitTradeSpi::QryTradeDetail(CSgitFtdcQryTradeField* p, int request_id)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("QryTradeDetail when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    // get data from initializer
    T_TradeDetailReturn query_result;
    const std::vector<TradeDetail>& init_buf = tunnel_initializer_->GetTradeInfo();
    query_result.datas.assign(init_buf.begin(), init_buf.end());
    query_result.error_no = 0;
    if (QryTradeDetailReturnHandler_)
    {
        QryTradeDetailReturnHandler_(&query_result);
    }
    LogUtil::OnTradeDetailRtn(&query_result, tunnel_info_);
    return 0;
}

inline int SgitTradeSpi::QryInstrument(CSgitFtdcQryInstrumentField* p, int request_id)
{
    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("QryInstrument when tunnel not ready");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    // get data from initializer
    T_ContractInfoReturn contract_info;
    const std::vector<ContractInfo>& init_buf = tunnel_initializer_->GetContractInfo();
    contract_info.datas.assign(init_buf.begin(), init_buf.end());
    contract_info.error_no = 0;
    if (QryContractInfoHandler_)
    {
        QryContractInfoHandler_(&contract_info);
    }
    LogUtil::OnContractInfoRtn(&contract_info, tunnel_info_);
    return 0;
}
