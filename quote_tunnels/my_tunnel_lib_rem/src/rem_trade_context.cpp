#include "rem_trade_context.h"

#include "config_data.h"
#include "tunnel_cmn_utility.h"

using namespace std;

RemTradeContext::RemTradeContext()
{
    cur_client_token_ = 0;
}

void RemTradeContext::SetClientToken(EES_ClientToken cur_max_token)
{
    lock_guard<mutex> lock(token_sync_);
    cur_client_token_ = cur_max_token;
}

EES_ClientToken RemTradeContext::GetNewClientToken()
{
    lock_guard<mutex> lock(token_sync_);
    return ++cur_client_token_;
}

void RemTradeContext::SaveReqInfo(RemReqInfo * order_info)
{
    lock_guard<mutex> lock(po_mutex_);

    // insert request info of client token
    ReqInfoOfCliToken::iterator cli_it = req_of_clienttoken_.find(order_info->client_token);
    if (cli_it == req_of_clienttoken_.end())
    {
        bool insert_res = req_of_clienttoken_.insert(std::make_pair(order_info->client_token, order_info)).second;
        if (insert_res == false)
        {
            TNL_LOG_ERROR("cache request info failed, client_token: %d", order_info->client_token);
        }
    }
    else
    {
        TNL_LOG_ERROR("duplicate client token: %d", order_info->client_token);
    }

    // insert request info of serial no
    ReqInfoOfSn::iterator sn_it = req_of_sn_.find(order_info->po.serial_no);
    if (sn_it == req_of_sn_.end())
    {
        bool insert_res = req_of_sn_.insert(std::make_pair(order_info->po.serial_no, order_info)).second;
        if (insert_res == false)
        {
            TNL_LOG_ERROR("cache request info failed, placeorder.sn: %ld", order_info->po.serial_no);
        }
    }
    else
    {
        TNL_LOG_ERROR("duplicate serial no of place order: %ld", order_info->po.serial_no);
    }
}

void RemTradeContext::SaveReqInfoOfMarketToken(EES_MarketToken market_token, const RemReqInfo *order_info)
{
    lock_guard<mutex> lock(po_mutex_);

    // insert request info of market token
    ReqInfoOfMarketToken::iterator cli_it = req_of_markettoken_.find(market_token);
    if (cli_it == req_of_markettoken_.end())
    {
        bool insert_res = req_of_markettoken_.insert(std::make_pair(market_token, order_info)).second;
        if (insert_res == false)
        {
            TNL_LOG_ERROR("cache request info failed, market: %lld", market_token);
        }
    }
    else
    {
        TNL_LOG_ERROR("duplicate market token: %lld", market_token);
    }
}

const RemReqInfo* RemTradeContext::GetReqInfoByClientToken(EES_ClientToken client_token)
{
    lock_guard<mutex> lock(po_mutex_);
    ReqInfoOfCliToken::iterator cli_it = req_of_clienttoken_.find(client_token);
    if (cli_it != req_of_clienttoken_.end())
    {
        return cli_it->second;
    }
    return NULL;
}

const RemReqInfo* RemTradeContext::GetReqInfoByMarketToken(EES_MarketToken market_token)
{
    lock_guard<mutex> lock(po_mutex_);
    ReqInfoOfMarketToken::iterator cli_it = req_of_markettoken_.find(market_token);
    if (cli_it != req_of_markettoken_.end())
    {
        return cli_it->second;
    }
    return NULL;
}

EES_MarketToken RemTradeContext::GetMarketOrderTokenOfSn(SerialNoType sn)
{
    lock_guard<mutex> lock(po_mutex_);
    ReqInfoOfSn::const_iterator cit = req_of_sn_.find(sn);
    if (cit != req_of_sn_.end())
    {
        return cit->second->market_token;
    }

    return 0;
}

void RemTradeContext::SaveCancelInfo(EES_MarketToken market_token, SerialNoType cancel_sn)
{
    lock_guard<mutex> lock(cancel_mutex_);

    CancelSnsOfMarketToken::iterator it = cancel_sns_of_markettoken_.find(market_token);
    if (it == cancel_sns_of_markettoken_.end())
    {
        std::pair<CancelSnsOfMarketToken::iterator, bool> res =
            cancel_sns_of_markettoken_.insert(std::make_pair(market_token, std::list<SerialNoType>()));
        if (res.second == false)
        {
            TNL_LOG_ERROR("cache cancel sn failed, market: %lld, cancel_sn: %ld", market_token, cancel_sn);
        }
        else
        {
            res.first->second.push_back(cancel_sn);
        }
    }
    else
    {
        it->second.push_back(cancel_sn);
    }
}

SerialNoType RemTradeContext::GetCancelSn(EES_MarketToken market_token)
{
    SerialNoType sn = 0;

    lock_guard<mutex> lock(cancel_mutex_);
    CancelSnsOfMarketToken::iterator it = cancel_sns_of_markettoken_.find(market_token);
    if (it != cancel_sns_of_markettoken_.end())
    {
        if (!it->second.empty())
        {
            sn = it->second.front();
            it->second.pop_front();
        }
    }
    return sn;
}
