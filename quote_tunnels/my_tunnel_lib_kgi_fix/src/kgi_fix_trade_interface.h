#pragma once

#include <mutex>
#include <condition_variable>
#include <map>
#include <unordered_map>

#include "kgi_fix_struct.h"
#include "position_log.h"

typedef std::unordered_map<long, std::string> SnToClOrdidMap;
typedef std::unordered_map<std::string, long> ClOrdidToSnMap;

class MYFixTrader
{
public:
    friend class MYFixSession;
    MYFixTrader(const std::string& cfg, const Tunnel_Info& tunnel_info);
    ~MYFixTrader()
    {
        if (mc_)
        {
            if (mc_->session_ptr()->is_shutdown() == false)
            {
                if (mc_->session_ptr()->get_connection())
                {
                    mc_->session_ptr()->stop();
                }
            }
        }
        if (position_)
        {
            delete position_;
        }
    }
    // 外部接口对象使用，为避免修改接口，新增对象放到此处
    std::mutex rsp_sync;
    std::condition_variable rsp_con;

    // 下发指令接口
    int ReqOrderInsert(const T_PlaceOrder *po);
    //报单操作请求
    int ReqOrderAction(const T_CancelOrder *co);
    // query interfaces
    int QryPosition(T_PositionReturn &p);
    int QryOrderDetail(const T_QryOrderDetail *p);
    int QryTradeDetail(const T_QryTradeDetail *p);
    int QryContractInfo(T_ContractInfoReturn &p);

    void SetCallbackHandler(
        std::function<void(const T_OrderRespond *)> handler)
    {
        OrderRespond_call_back_handler_ = handler;
    }
    void SetCallbackHandler(
        std::function<void(const T_CancelRespond *)> handler)
    {
        CancelRespond_call_back_handler_ = handler;
    }
    void SetCallbackHandler(
        std::function<void(const T_OrderReturn *)> handler)
    {
        OrderReturn_call_back_handler_ = handler;
    }
    void SetCallbackHandler(
        std::function<void(const T_TradeReturn *)> handler)
    {
        TradeReturn_call_back_handler_ = handler;
    }
    void SetCallbackHandler(
        std::function<void(const T_PositionReturn *)> handler)
    {
        QryPosReturnHandler_ = handler;
    }
    void SetCallbackHandler(
        std::function<void(const T_OrderDetailReturn *)> handler)
    {
        QryOrderDetailReturnHandler_ = handler;
    }
    void SetCallbackHandler(
        std::function<void(const T_TradeDetailReturn *)> handler)
    {
        QryTradeDetailReturnHandler_ = handler;
    }
    void SetCallbackHandler(
        std::function<void(const T_ContractInfoReturn *)> handler)
    {
        QryContractInfoHandler_ = handler;
    }

private:
    std::shared_ptr<FIX8::ClientSession<MYFixSession> > mc_;

    inline void SetTransactTimeAndOrderId(char *transact_time,
        char *cl_orderid)
    {
        time_t tt = time(NULL);
        tm* t = gmtime(&tt);
        sprintf(transact_time, "%04d%02d%02d-%02d:%02d:%02d", t->tm_year + 1900,
            t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
        sprintf(cl_orderid, "%s_%02d%02d%02d%04d", account_.c_str(), t->tm_hour,
            t->tm_min, t->tm_sec, order_id_);
    }

    SnToClOrdidMap order_book_;
    ClOrdidToSnMap serialno_book_;

    unsigned order_id_;
    string cfg_;
    string account_;
    Tunnel_Info tunnel_info_;
    position_log *position_;

    volatile bool loggoned_;
    std::mutex api_mutex_;

    std::function<void(const T_OrderRespond *)> OrderRespond_call_back_handler_;
    std::function<void(const T_CancelRespond *)> CancelRespond_call_back_handler_;
    std::function<void(const T_OrderReturn *)> OrderReturn_call_back_handler_;
    std::function<void(const T_TradeReturn *)> TradeReturn_call_back_handler_;
    std::function<void(const T_PositionReturn *)> QryPosReturnHandler_;
    std::function<void(const T_OrderDetailReturn *)> QryOrderDetailReturnHandler_;
    std::function<void(const T_TradeDetailReturn *)> QryTradeDetailReturnHandler_;
    std::function<void(const T_ContractInfoReturn *)> QryContractInfoHandler_;
};
