#ifndef REM_TRADE_CONTEXT_H_
#define REM_TRADE_CONTEXT_H_

#include "EesTraderDefine.h"
#include "my_trade_tunnel_struct.h"
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <mutex>
#include <string.h>

struct RemReqInfo
{
    T_PlaceOrder po;                // original po struct
    EES_ClientToken client_token;   // 下单 local id
    mutable EES_MarketToken market_token;   // order number in shengli system
    mutable EntrustNoType   entruct_no;     // entruct number in exchange
    mutable char order_status;
    mutable VolumeType volume_remain;

    RemReqInfo(const T_PlaceOrder &p, EES_ClientToken cli_token)
        : po(p), client_token(cli_token), market_token(0), entruct_no(0), order_status(MY_TNL_OS_UNREPORT)
    {
        volume_remain = po.volume;
    }
};

typedef std::unordered_map<EES_ClientToken, RemReqInfo*> ReqInfoOfCliToken;
typedef std::unordered_map<EES_MarketToken, const RemReqInfo*> ReqInfoOfMarketToken;
typedef std::unordered_map<SerialNoType, RemReqInfo*> ReqInfoOfSn;

typedef std::unordered_map<EES_MarketToken, std::list<SerialNoType> > CancelSnsOfMarketToken;

class RemTradeContext
{
public:
    RemTradeContext();

    // 报单client token 的维护
    void SetClientToken(EES_ClientToken cur_max_token);
    EES_ClientToken GetNewClientToken();

    // 报单client token 和报单struct的映射
    void SaveReqInfo(RemReqInfo *order_info);
    void SaveReqInfoOfMarketToken(EES_MarketToken market_token, const RemReqInfo *order_info);
    const RemReqInfo * GetReqInfoByClientToken(EES_ClientToken client_token);
    const RemReqInfo * GetReqInfoByMarketToken(EES_MarketToken market_token);
    EES_MarketToken GetMarketOrderTokenOfSn(SerialNoType sn);

    void SaveCancelInfo(EES_MarketToken market_token, SerialNoType cancel_sn);
    SerialNoType GetCancelSn(EES_MarketToken market_token);

private:
    std::mutex token_sync_;
    EES_ClientToken cur_client_token_;

    // token to request info
    std::mutex po_mutex_;
    ReqInfoOfCliToken req_of_clienttoken_;
    ReqInfoOfSn req_of_sn_;
    ReqInfoOfMarketToken req_of_markettoken_;

    std::mutex cancel_mutex_;
    CancelSnsOfMarketToken cancel_sns_of_markettoken_;
};

#endif
