#pragma once

#include "KGIfix_router.hpp"

#include "config_data.h"
#include "tunnel_cmn_utility.h"
#include "trade_log_util.h"

const char KGI_FIX_HONGKONG_MARKET[3] = "HK";
const char KGI_FIX_SHENZHEN_B_STOCK[3] = "SZ";
const char KGI_FIX_SHANGHAI_B_STOCK[3] = "C1";

typedef std::unordered_map<std::string, long> ClOrdidToSnMap;
class MYFixTrader;
class MYFixSession;

class MYFixRouter: public FIX8::KGI::KGIfix_Router
{
public:
    MYFixRouter(MYFixSession& session)
        :
            session_(session)
    {
    }

    virtual bool operator()(const FIX8::KGI::ExecutionReport *msg) const;
    virtual bool operator()(
        const class FIX8::KGI::OrderCancelReject *msg) const;
    virtual bool operator()(const class FIX8::KGI::Reject *msg) const;
    virtual bool operator()(class FIX8::Message *msg) const
        {
        return false;
    }

    MYFixSession& session_;
};

class MYFixSession: public FIX8::Session
{
    MYFixRouter router_;

public:
    MYFixSession(const FIX8::F8MetaCntx& ctx, const FIX8::SessionID& sid,
        FIX8::Persister *persist = 0, FIX8::Logger *logger = 0,
        FIX8::Logger *plogger = 0)
        :
            FIX8::Session(ctx, sid, persist, logger, plogger), router_(*this), p_trd_(
            NULL), has_resend_(false)
    {
    }

    virtual ~MYFixSession()
    {
        send(generate_logout());
        if (p_trd_)
        {
            p_trd_ = NULL;
        }
        this->Session::~Session();
    }

    bool my_handle_logon(const unsigned seqnum, const FIX8::Message *msg);
    bool handle_logon(const unsigned seqnum, const FIX8::Message *msg);
    bool handle_logout(const unsigned seqnum, const FIX8::Message *msg);
    bool handle_reject(const unsigned seqnum, const FIX8::Message *msg);
    bool handle_heartbeat(const unsigned seqnum, const FIX8::Message *msg);
    bool handle_test_request(const unsigned seqnum, const FIX8::Message *msg);
    bool handle_application(const unsigned seqnum, const FIX8::Message *&msg);
    bool handle_resend_request(const unsigned seqnum, const FIX8::Message *msg);
    bool handle_sequence_reset(const unsigned seqnum, const FIX8::Message *msg);

    bool checkSematics(const unsigned seqnum, const FIX8::Message *msg);
    bool exchange_sequence_check(const unsigned seqnum,
        const FIX8::Message *msg);
    FIX8::Message *generate_resend_request(const unsigned begin,
        const unsigned end = 0);

    void set_trader_ptr(MYFixTrader* p)
    {
        p_trd_ = p;
    }

    ClOrdidToSnMap::iterator GetClOrdIdPos(std::string key);
    bool ItIsEnd(ClOrdidToSnMap::iterator it);
    const Tunnel_Info GetTunnelInfo();
    void SentToOrderRespondCallBackHandler(T_OrderRespond *rsp);
    void SentToCancelRespondCallBackHandler(T_CancelRespond *rsp);
    void SentToOrderReturnCallBackHandler(T_OrderReturn *rsp);
    void SentToTradeReturnCallBackHandler(T_TradeReturn *rsp);
    long GetEntrustNo(const std::string &order_id);
    void UpdatePositionData(T_TradeReturn &trade);

private:
    MYFixTrader *p_trd_;
    bool has_resend_;
};
