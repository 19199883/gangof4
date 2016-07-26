#ifndef CITICS_HS_TRADE_INTERFACE_H_
#define CITICS_HS_TRADE_INTERFACE_H_

#include <string>
#include <sstream>
#include <list>
#include <atomic>
#include <boost/thread.hpp>
#include <map>
#include <set>

#include "CITICs_HsT2Hlp.h"

#include "config_data.h"
#include "trade_data_type.h"
#include "hs_trade_context.h"
#include "my_trade_tunnel_api.h"
#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"
#include "hs_field_convert.h"
#include "hs_util.h"

using namespace std;

class CiticsHsTradeInf
{
public:
    CiticsHsTradeInf(const TunnelConfigData &cfg, const Tunnel_Info &tunnel_info);
    virtual ~CiticsHsTradeInf(void);

    // 下发指令接口
    int PlaceOrder(const T_PlaceOrder *pPlaceOrder);
    int ProcessRejectResponse(int err, const T_PlaceOrder* p);

    int CancelOrder(const T_CancelOrder *pCancelOrder);
    int QueryPosition(const T_QryPosition *pQryPosition);
    int QueryTradeDetail(const T_QryTradeDetail *pQryParam);

    bool BuildInternalPosObj(HSHLPHANDLE HlpHandle, T_PositionReturn& internalPosObj, char *position_str);
    bool BuildTradeDetailObj(HSHLPHANDLE HlpHandle, T_TradeDetailReturn& ret, char *position_str);

    int ProcessCommitFailure(int err, const T_PlaceOrder* p);

    void ProcessAcceptedSubscribedMessage(
        HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, T_PlaceOrder *p);
    void ProcessSuccessfulSynResponse(HSHLPHANDLE HlpHandle, const T_PlaceOrder *p);
    void ProcessSubscribedMessage(HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl);

    void ProcessSubscribedCancelMessage(
        HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, const int &report_no);
    void ProcessAcceptedCancelSubscribedMessage(T_InternalOrder *p);
    void ProcessRejectedCancelSubscribedMessage(HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, T_InternalOrder *p);
    void ProcessSubscribedPlaceMessage(
        HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, const int report_no);
    int ProcessRejectResponse(int err, const T_CancelOrder* p);
    int ProcessSynCommitFailure(int err, const T_CancelOrder* p);

    void ProcessAcceptedPlaceSubscribedMessage(T_InternalOrder *p);
    void ProcessRejectedStatusMessage(HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl, T_InternalOrder *p);
    void ProcessFilledStatusMessage(T_InternalOrder *p);

    void ProcessOrderStatusData(T_InternalOrder *p);
    void ProcessFilledDataMessage(T_InternalOrder *p);

    void ProcessCancelledStatusMessage(T_InternalOrder *p);
    void ProcessPartialFilledStatusMessage(T_InternalOrder *p);

    // callback handle set interface
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

    void SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler)
    {
        QryTradeDetailReturnHandler_ = handler;
    }

    bool ParseConfig();

    bool TunnelIsReady()
    {
        return logoned_ && have_handled_unterminated_orders_;
    }

    HSTradeContext &Context()
    {
        return hs_trade_context_;
    }
public:
    HSTradeContext hs_trade_context_;

    const int PAGE_SIZE = 50;

private:
    HSHLPHANDLE rtn_handle_;
    HSHLPHANDLE hs_handle_;
    HSHLPCFGHANDLE hs_config_;

    Tunnel_Info tunnel_info_;
    std::string acc_pswd;
    std::string server_address;
    char client_id[32];
    char user_token[64];
    char branch_no[8];
    char asset_prop[4];
    char sysnode_id[4];
    char op_entrust_way[4];
    char op_station[128];
    char stock_account[128];
    char identity_type[2];
    char password_type[2];

    /*
     * key:exchange type
     * value:stock account
     */
    map<char, string> stock_accounts_;

    /*
     * key:asynchronous message request number(nReqNo)
     * value: cl_ord_id
     */
    map<int, long> asyn_msg_reqnos_;

    T_TradeDetailReturn TradeDetail_cache_;

    boost::mutex op_sync_;
    void Init();
    void InitTradeDetail();
    void fill_today_traded_info(PositionDetail &pos_detail);
    void Connect(HSHLPHANDLE &handle);
    void Login(HSHLPHANDLE &handle);
    void QryStockHolderInfo();
    void SetNecessaryParam(HSHLPHANDLE HlpHandle);
    int SubscribeOrderReturn();
    void UnsubscribeOrderReturn();
    void ProcessAsynAndSubscribedMessage();
    void ReportErrorState(int api_error_no, const std::string &error_msg);

    // respond process handler
    std::function<void(const T_OrderRespond *)> OrderRespond_call_back_handler_;
    std::function<void(const T_CancelRespond *)> CancelRespond_call_back_handler_;
    std::function<void(const T_OrderReturn *)> OrderReturn_call_back_handler_;
    std::function<void(const T_TradeReturn *)> TradeReturn_call_back_handler_;

    std::function<void(const T_PositionReturn *)> QryPosReturnHandler_;
    std::function<void(const T_OrderDetailReturn *)> QryOrderDetailReturnHandler_;
    std::function<void(const T_TradeDetailReturn *)> QryTradeDetailReturnHandler_;

    // 配置数据对象
    TunnelConfigData cfg_;
    std::atomic_bool connected_;
    std::atomic_bool logoned_;

    // query position control variables
    std::vector<PositionDetail> pos_buffer_;

    // query order detail control variables
    std::vector<OrderDetail> od_buffer_;

//    // query trade detail control variables
//    std::vector<TradeDetail> td_buffer_;

// variables and functions for cancel all unterminated orders automatically
    std::atomic_bool have_handled_unterminated_orders_;

    boost::mutex cancel_sync_;
    boost::condition_variable qry_order_finish_cond_;
    bool IsOrderTerminate(char status);
    int CancelUnterminatedOrders();
    void resubscribe();
    int QueryCancelableOrders(vector<T_CancelOrder> &cancelOrders);
    bool BuildTradeDetailObj(HSHLPHANDLE HlpHandle, vector<T_CancelOrder>& cancelOrders, char *position_str);
    int CancelOrderInternal(const T_CancelOrder* pCancelOrder);
};

#endif //
