#pragma once

#include <string>
#include <sstream>
#include <list>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "SecurityFtdcTraderApi.h"
#include "SecurityFtdcQueryApi.h"

#include "config_data.h"
#include "trade_data_type.h"
#include "lts_trade_context.h"
#include "my_cmn_util_funcs.h"
#include "my_tunnel_lib.h"
#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"
#include "lts_field_convert.h"
#include "lts_data_formater.h"
#include "my_trade_tunnel_data_type.h"
#include "qtm_with_code.h"

#ifdef USE_LTS_API_2
#include "SecurityFtdcTraderApi2.h"
#include "SecurityFtdcQueryApi2.h"

#define CSecurityFtdcTraderApi CSecurityFtdcTraderApi2
#define CSecurityFtdcQueryApi CSecurityFtdcQueryApi2
#endif

struct OriginalReqInfo;
class LTSQueryInf;

class LTSTradeInf: public CSecurityFtdcTraderSpi
{
public:
    LTSTradeInf(const TunnelConfigData &cfg);
    virtual ~LTSTradeInf(void);

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

    // overriding functions from CSecurityFtdcTraderSpi
    virtual void OnFrontConnected();
    virtual void OnFrontDisconnected(int nReason);
    virtual void OnHeartBeatWarning(int nTimeLapse);
    virtual void OnRspError(CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    virtual void OnRspUserLogin(CSecurityFtdcRspUserLoginField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    virtual void OnRspUserLogout(CSecurityFtdcUserLogoutField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    virtual void OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);

    virtual void OnRspOrderInsert(CSecurityFtdcInputOrderField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    virtual void OnRspOrderAction(CSecurityFtdcInputOrderActionField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    virtual void OnRtnOrder(CSecurityFtdcOrderField *pf);
    virtual void OnRtnTrade(CSecurityFtdcTradeField *pf);
    virtual void OnErrRtnOrderInsert(CSecurityFtdcInputOrderField *pf, CSecurityFtdcRspInfoField *rsp);
    virtual void OnErrRtnOrderAction(CSecurityFtdcOrderActionField *pf, CSecurityFtdcRspInfoField *rsp);

    // implement functions for CSecurityFtdcQuerySpi
    void OnQueryFrontConnected();
    void OnQueryFrontDisconnected(int reason);
    void OnQueryHeartBeatWarning(int time_lapse);
    void OnQueryRspError(CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnQueryRspUserLogin(CSecurityFtdcRspUserLoginField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnQueryRspUserLogout(CSecurityFtdcUserLogoutField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnQueryRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);

    void OnRspQryExchange(CSecurityFtdcExchangeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryInstrument(CSecurityFtdcInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryInvestor(CSecurityFtdcInvestorField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryTradingCode(CSecurityFtdcTradingCodeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryTradingAccount(CSecurityFtdcTradingAccountField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryMarketRationInfo(CSecurityFtdcMarketRationInfoField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryInstrumentCommissionRate(CSecurityFtdcInstrumentCommissionRateField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryETFInstrument(CSecurityFtdcETFInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryETFBasket(CSecurityFtdcETFBasketField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryOFInstrument(CSecurityFtdcOFInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQrySFInstrument(CSecurityFtdcSFInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryInstrumentUnitMargin(CSecurityFtdcInstrumentUnitMarginField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryMarketDataStaticInfo(CSecurityFtdcMarketDataStaticInfoField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryOrder(CSecurityFtdcOrderField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryTrade(CSecurityFtdcTradeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    void OnRspQryInvestorPosition(CSecurityFtdcInvestorPositionField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);

    // 下发指令接口
    int ReqOrderInsert(CSecurityFtdcInputOrderField *pInputOrder, int req_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderInsert when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqOrderInsert(pInputOrder, req_id);
            if (ret != 0)
            {
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
    int ReqOrderAction(CSecurityFtdcInputOrderActionField *pInputOrderAction, int req_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("ReqOrderAction when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }
        int ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        try
        {
            ret = api_->ReqOrderAction(pInputOrderAction, req_id);
            if (ret != 0)
            {
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("unknown exception in ReqOrderAction.");
        }

        return ret;
    }

    int QryPosition(CSecurityFtdcQryInvestorPositionField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryPosition when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = query_api_->ReqQryInvestorPosition(p, request_id);
        TNL_LOG_INFO("ReqQryInvestorPositionDetail - request_id:%d, return:%d", request_id, ret);

        return ret;
    }
    int QryOrderDetail(CSecurityFtdcQryOrderField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryOrderDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = query_api_->ReqQryOrder(p, request_id);
        TNL_LOG_INFO("ReqQryOrder - request_id:%d, return:%d", request_id, ret);

        return ret;
    }
    int QryTradeDetail(CSecurityFtdcQryTradeField *p, int request_id)
    {
        if (!TunnelIsReady())
        {
            TNL_LOG_WARN("QryTradeDetail when tunnel not ready");
            return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        }

        int ret = query_api_->ReqQryTrade(p, request_id);
        TNL_LOG_INFO("ReqQryTrade - request_id:%d, return:%d", request_id, ret);

        return ret;
    }
    int QryInstrument(CSecurityFtdcQryInstrumentField *p, int request_id)
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
            contract_info.datas.assign(ci_buffer_.begin(), ci_buffer_.end());
            contract_info.error_no = 0;

            if (QryContractInfoHandler_)
            {
                QryContractInfoHandler_(&contract_info);
            }

            LogUtil::OnContractInfoRtn(&contract_info, tunnel_info_);

            return 0;
        }
        else
        {
            int ret = query_api_->ReqQryInstrument(p, request_id);
            TNL_LOG_INFO("ReqQryInstrument - request_id:%d, return:%d", request_id, ret);

            return ret;
        }
    }

    bool TunnelIsReady()
    {
        return tunnel_finish_init_;
    }
    bool HaveFinishQueryOrders()
    {
        return have_handled_unterminated_orders_ && finish_query_canceltimes_;
    }
    int Front_id() const
    {
        return front_id_;
    }
    int Session_id() const
    {
        return session_id_;
    }

    // 外部接口对象使用，为避免修改接口，新增对象放到此处
    LTSTradeContext lts_trade_context_;
    std::mutex client_sync;
    std::mutex rsp_sync;
    std::condition_variable rsp_con;

private:
    void InitQueryInterface();
    void InitTradeInterface();
    CSecurityFtdcTraderApi *api_;
    CSecurityFtdcQueryApi *query_api_;
    LTSQueryInf *query_inf_;

    // status
    bool logoned_;
    bool query_logoned_;
    bool in_init_state_; // clear after login
    bool have_handled_unterminated_orders_;
    bool finish_query_canceltimes_;
    bool finish_query_contracts_;

    std::mutex api_mutex_;
    bool tunnel_finish_init_;
    bool query_finish_init_;
    bool trade_finish_init_;
    void SetQueryFinished();
    void SetTradeFinished();

    // properties
    int front_id_;
    int session_id_;
    int max_order_ref_;
    TSecurityFtdcDateType trade_day;
    double available_fund;
    PositionDetail PackageAvailableFunc();
    TSecurityFtdcAuthCodeType trade_authcode_;
    TSecurityFtdcAuthCodeType query_authcode_;

    // login parameters and functions
    void ParseConfig();
    Tunnel_Info tunnel_info_;
    std::string pswd_;
    void ReqLogin();
    void QueryReqLogin();
    void QueryAccount();
    void QueryContractInfo();
    void ReportErrorState(int api_error_no, const std::string &error_msg);

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
    std::mutex contract_sync_;
    std::unordered_map<std::string, int> contract_multiple_;
    int GetContractMultiple(const std::string &contract);

    // variables and functions for cancel all unterminated orders automatically
    std::vector<CSecurityFtdcOrderField> unterminated_orders_;
    std::mutex cancel_sync_;
    std::condition_variable qry_order_finish_cond_;
    CSecurityFtdcInputOrderActionField CreatCancelParam(const CSecurityFtdcOrderField & order_field);
    bool IsOrderTerminate(const CSecurityFtdcOrderField & order_field);
    void QueryAndHandleOrders();

    // variables for stats cancel times
    std::mutex stats_canceltimes_sync_;
    std::map<std::string, int> cancel_times_of_contract;

    int time_to_query_pos_int;
    int query_position_step; // 0:init; 1:start query; 2:finish query
    std::vector<PositionDetail> pos_buffer_query_on_time_;
    void RecordPositionAtSpecTime();
    void SaveCurrentPosition();
};

inline void LTSTradeInf::SetQueryFinished()
{
    std::lock_guard<std::mutex> lock(api_mutex_);
    query_finish_init_ = true;
    if (trade_finish_init_)
    {
        tunnel_finish_init_ = true;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
    }
}

inline void LTSTradeInf::SetTradeFinished()
{
    std::lock_guard<std::mutex> lock(api_mutex_);
    trade_finish_init_ = true;
    if (query_finish_init_)
    {
        tunnel_finish_init_ = true;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
    }
}
