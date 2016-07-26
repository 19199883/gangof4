#include "quote_femas_multicast.h"
#include <iomanip>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

using namespace my_cmn;
using namespace std;

FemasMulticastMDHandler::FemasMulticastMDHandler(const SubscribeContracts* subscribe_contracts, const ConfigData& cfg)
    : cfg_(cfg)
{
    handle_ = NULL;
    is_ready_ = false;
    heartbeat_check_interval_seconds_ = 180; // 3 minutes (must less than 10 minutes)
    run_flag_ = true;

    sprintf(qtm_name_, "cffex_femas_multicast_%s_%u", cfg_.Logon_config().account.c_str(), getpid());
    QuoteUpdateState(qtm_name_, QtmState::INIT);

    p_save_ = new QuoteDataSave<CFfexFtdcDepthMarketData>(cfg_, qtm_name_, "cffex_l2", GTA_UDP_CFFEX_QUOTE_TYPE);
    p_hbt_check_ = NULL;
    p_md_handler_ = NULL;



    if (!Init())
    {
        return;
    }
    MY_LOG_INFO("Init api success.");

    // should wait and retry, so put in another thread
    boost::thread t(&FemasMulticastMDHandler::Login, this);

    // create heartbeat check thread
    p_hbt_check_ = new boost::thread(&FemasMulticastMDHandler::HeartBeatCheckThread, this);

    // create market data handle thread
    p_md_handler_ = new boost::thread(&FemasMulticastMDHandler::DataHandle, this);
}

void FemasMulticastMDHandler::SetQuoteDataHandler(boost::function<void(const CFfexFtdcDepthMarketData*)> quote_data_handler)
{
    quote_data_handler_ = quote_data_handler;
}

FemasMulticastMDHandler::~FemasMulticastMDHandler(void)
{
    run_flag_ = false;
    if (p_md_handler_) p_md_handler_->join();

    CUstpRspInfo rspInfo;
    if (handle_ && CheckValidateHandle(handle_, &rspInfo))
    {
        if (ReqUserLogout(handle_, &rspInfo) != true)
        {
            MY_LOG_WARN("logout failed, ErrorID:%d, ErrorMsg:%s", rspInfo.ErrorID, rspInfo.ErrorMsg);
        }
        QuoteUpdateState(qtm_name_, QtmState::LOG_OUT);
        DestroyUserHandle(handle_);
    }
    handle_ = NULL;
    DestroyAPI();
}

bool FemasMulticastMDHandler::Init()
{
    CUstpRspInfo rspInfo;
    if (!InitAPI(5000, &rspInfo))
    {
        QuoteUpdateState(qtm_name_, QtmState::QUOTE_INIT_FAIL);
        MY_LOG_WARN("InitAPI failed, ErrorID:%d, ErrorMsg:%s", rspInfo.ErrorID, rspInfo.ErrorMsg);
        return false;
    }

    for (const std::string &v : cfg_.Logon_config().quote_provider_addrs)
    {
        char *addr_tmp = new char[v.size() + 1];
        memcpy(addr_tmp, v.c_str(), v.size() + 1);
        RegisterFront(addr_tmp);
        QuoteUpdateState(qtm_name_, QtmState::SET_ADDRADRESS_SUCCESS);
        MY_LOG_INFO("femas_multicast - RegisterFront, addr: %s", addr_tmp);
        delete[] addr_tmp;
    }

    return true;
}

void FemasMulticastMDHandler::Login()
{
    CUstpRspInfo rspInfo;
    int retry_wait_seconds = 3;

    while (ConnectFront(&rspInfo) == false)
    {
        MY_LOG_ERROR("ConnectFront failed, ErrorID:%d, ErrorMsg:%s", rspInfo.ErrorID, rspInfo.ErrorMsg);
        sleep(retry_wait_seconds);
    }
    QuoteUpdateState(qtm_name_, QtmState::CONNECT_SUCCESS);
    MY_LOG_INFO("ConnectFront success");

    CUstpFtdcReqUserLoginField_MC loginField;
    sprintf(loginField.UserID, cfg_.Logon_config().account.c_str());
    sprintf(loginField.Password, cfg_.Logon_config().password.c_str());
    sprintf(loginField.IP, cfg_.Logon_config().trade_server_addr.c_str());
    loginField.TopicID = atoi(cfg_.Logon_config().topic.c_str());

    while (ReqUserLogin(&loginField, true, handle_, &rspInfo) == false)
    {
        MY_LOG_ERROR("ReqUserLogin failed, ErrorID:%d, ErrorMsg:%s", rspInfo.ErrorID, rspInfo.ErrorMsg);
        sleep(retry_wait_seconds);
    }
    QuoteUpdateState(qtm_name_, QtmState::LOG_ON_SUCCESS);
    MY_LOG_INFO("ReqUserLogin success");

    // wait and query snapshot
    sleep(1);
    vector<CUstpFtdcDepthMarketDataField> snapshot;
    TUstpFtdcInstrumentIDType req_instrument;
    memset(req_instrument, 0 , sizeof(req_instrument));
    while (ReqQrySnapshotDepthMarket(handle_, req_instrument, snapshot, &rspInfo) == false)
    {
        MY_LOG_ERROR("ReqQrySnapshotDepthMarket failed, ErrorID:%d, ErrorMsg:%s", rspInfo.ErrorID, rspInfo.ErrorMsg);
        snapshot.clear();
        sleep(1);
    }
    MY_LOG_INFO("ReqQrySnapshotDepthMarket success, data count:%d", snapshot.size());

    // add in map
    for (CUstpFtdcDepthMarketDataField & v : snapshot)
    {
        MY_LOG_INFO("InstrumentID:%s,TradingDay:%s,PreClosePrice:%.4f,PreSettlePrice:%.4f,PreOpenInterest:%.4f,PreDelta:%.4f",
            v.InstrumentID, v.TradingDay, v.PreClosePrice, v.PreSettlementPrice, v.PreOpenInterest, v.PreDelta);

        MDSnapshotMap::iterator it = snapshot_map_.find(std::string(v.InstrumentID));
        if (it == snapshot_map_.end())
        {
            snapshot_map_.insert(std::make_pair(std::string(v.InstrumentID), v));
        }
        else
        {
            memcpy(&it->second, &v, sizeof(CUstpFtdcDepthMarketDataField));
        }
    }

    {
        boost::unique_lock<boost::mutex> lock(login_sync_);
        is_ready_ = true;
    }
    QuoteUpdateState(qtm_name_, QtmState::API_READY);
    login_con_.notify_all();
}

void FemasMulticastMDHandler::HeartBeatCheckThread()
{
    {
        boost::unique_lock<boost::mutex> lock(login_sync_);
        while (!is_ready_)
        {
            login_con_.wait(lock);
        }
    }

    MY_LOG_INFO("heartbeat check thread is running, interval is %d seconds.", heartbeat_check_interval_seconds_);
    CUstpRspInfo rspInfo;

    while (run_flag_)
    {
        ReqHeartCheck(handle_, &rspInfo);
        MY_LOG_INFO("ReqHeartCheck respond, ErrorID:%d, ErrorMsg:%s", rspInfo.ErrorID, rspInfo.ErrorMsg);
        sleep(heartbeat_check_interval_seconds_);
    }
}

void FemasMulticastMDHandler::DataHandle()
{
    {
        boost::unique_lock<boost::mutex> lock(login_sync_);
        while (!is_ready_)
        {
            login_con_.wait(lock);
        }
    }

    MY_LOG_INFO("market data handle thread is running.");
    CUstpRspInfo rspInfo;
    CUstpFtdcDepthMarketDataField d;
    long prev_seq = 0;
    long seqno = 0;
    while (run_flag_)
    {
        if (GetDepthMarketDataField(handle_, seqno, d, &rspInfo) != true)
        {
            //MY_LOG_INFO("GetDepthMarketDataField failed, seqno:%ld, ErrorID:%d, ErrorMsg:%s", seqno, rspInfo.ErrorID, rspInfo.ErrorMsg);
            continue;
        }

        timeval t;
        gettimeofday(&t, NULL);

        MDSnapshotMap::iterator it = snapshot_map_.find(std::string(d.InstrumentID));
        if (it != snapshot_map_.end())
        {
            CUstpFtdcDepthMarketDataField &s = it->second;
            memcpy(d.TradingDay, s.TradingDay, sizeof(d.TradingDay));
            d.PreClosePrice = s.PreClosePrice;
            d.PreSettlementPrice = s.PreSettlementPrice;
            d.PreOpenInterest = s.PreOpenInterest;
            d.PreDelta = s.PreDelta;
        }
        RalaceInvalidValue_Femas(d);
        CFfexFtdcDepthMarketData q_cffex_level2 = Convert(d);

        if (quote_data_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(d.InstrumentID) != subscribe_contracts_.end()))
        {
            quote_data_handler_(&q_cffex_level2);
        }

        // 存起来
        p_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &q_cffex_level2);

        if (prev_seq == 0)
        {
            MY_LOG_INFO("GetDepthMarketDataField, seqno start: %ld", seqno);
        }
        else if (prev_seq + 1 != seqno)
        {
            MY_LOG_WARN("GetDepthMarketDataField, maybe lost package, prev_seq:%ld, cur_seq:%ld", prev_seq, seqno);
        }
        prev_seq = seqno;
    }
}

void FemasMulticastMDHandler::RalaceInvalidValue_Femas(CUstpFtdcDepthMarketDataField& d)
{

    d.Turnover = InvalidToZeroD(d.Turnover);
    d.LastPrice = InvalidToZeroD(d.LastPrice);
    d.UpperLimitPrice = InvalidToZeroD(d.UpperLimitPrice);
    d.LowerLimitPrice = InvalidToZeroD(d.LowerLimitPrice);
    d.HighestPrice = InvalidToZeroD(d.HighestPrice);
    d.LowestPrice = InvalidToZeroD(d.LowestPrice);
    d.OpenPrice = InvalidToZeroD(d.OpenPrice);
    d.ClosePrice = InvalidToZeroD(d.ClosePrice);
    d.PreClosePrice = InvalidToZeroD(d.PreClosePrice);
    d.OpenInterest = InvalidToZeroD(d.OpenInterest);
    d.PreOpenInterest = InvalidToZeroD(d.PreOpenInterest);

    d.BidPrice1 = InvalidToZeroD(d.BidPrice1);
    d.BidPrice2 = InvalidToZeroD(d.BidPrice2);
    d.BidPrice3 = InvalidToZeroD(d.BidPrice3);
    d.BidPrice4 = InvalidToZeroD(d.BidPrice4);
    d.BidPrice5 = InvalidToZeroD(d.BidPrice5);

    d.AskPrice1 = InvalidToZeroD(d.AskPrice1);
    d.AskPrice2 = InvalidToZeroD(d.AskPrice2);
    d.AskPrice3 = InvalidToZeroD(d.AskPrice3);
    d.AskPrice4 = InvalidToZeroD(d.AskPrice4);
    d.AskPrice5 = InvalidToZeroD(d.AskPrice5);

    d.SettlementPrice = InvalidToZeroD(d.SettlementPrice);
    d.PreSettlementPrice = InvalidToZeroD(d.PreSettlementPrice);

    d.PreDelta = InvalidToZeroD(d.PreDelta);
    d.CurrDelta = InvalidToZeroD(d.CurrDelta);
}

CFfexFtdcDepthMarketData FemasMulticastMDHandler::Convert(const CUstpFtdcDepthMarketDataField& femas_data)
{
    CFfexFtdcDepthMarketData q2;
    memset(&q2, 0, sizeof(CFfexFtdcDepthMarketData));

    memcpy(q2.szTradingDay, femas_data.TradingDay, 9); // typedef char TUstpFtdcDateType[9];
    memcpy(q2.szSettlementGroupID, femas_data.SettlementGroupID, 9); // typedef char TUstpFtdcSettlementGroupIDType[9];
    q2.nSettlementID = femas_data.SettlementID;
    q2.dLastPrice = femas_data.LastPrice;
    q2.dPreSettlementPrice = femas_data.PreSettlementPrice;
    q2.dPreClosePrice = femas_data.PreClosePrice;
    q2.dPreOpenInterest = femas_data.PreOpenInterest;
    q2.dOpenPrice = femas_data.OpenPrice;
    q2.dHighestPrice = femas_data.HighestPrice;
    q2.dLowestPrice = femas_data.LowestPrice;
    q2.nVolume = femas_data.Volume;
    q2.dTurnover = femas_data.Turnover;
    q2.dOpenInterest = femas_data.OpenInterest;
    q2.dClosePrice = femas_data.ClosePrice;
    q2.dSettlementPrice = femas_data.SettlementPrice;
    q2.dUpperLimitPrice = femas_data.UpperLimitPrice;
    q2.dLowerLimitPrice = femas_data.LowerLimitPrice;
    q2.dPreDelta = femas_data.PreDelta;
    q2.dCurrDelta = femas_data.CurrDelta;
    memcpy(q2.szUpdateTime, femas_data.UpdateTime, 9); // typedef char TUstpFtdcTimeType[9];
    q2.nUpdateMillisec = femas_data.UpdateMillisec;
    memcpy(q2.szInstrumentID, femas_data.InstrumentID, 31); // typedef char TUstpFtdcInstrumentIDType[31];
    q2.dBidPrice1 = femas_data.BidPrice1;
    q2.nBidVolume1 = femas_data.BidVolume1;
    q2.dAskPrice1 = femas_data.AskPrice1;
    q2.nAskVolume1 = femas_data.AskVolume1;
    q2.dBidPrice2 = femas_data.BidPrice2;
    q2.nBidVolume2 = femas_data.BidVolume2;
    q2.dAskPrice2 = femas_data.AskPrice2;
    q2.nAskVolume2 = femas_data.AskVolume2;
    q2.dBidPrice3 = femas_data.BidPrice3;
    q2.nBidVolume3 = femas_data.BidVolume3;
    q2.dAskPrice3 = femas_data.AskPrice3;
    q2.nAskVolume3 = femas_data.AskVolume3;
    q2.dBidPrice4 = femas_data.BidPrice4;
    q2.nBidVolume4 = femas_data.BidVolume4;
    q2.dAskPrice4 = femas_data.AskPrice4;
    q2.nAskVolume4 = femas_data.AskVolume4;
    q2.dBidPrice5 = femas_data.BidPrice5;
    q2.nBidVolume5 = femas_data.BidVolume5;
    q2.dAskPrice5 = femas_data.AskPrice5;
    q2.nAskVolume5 = femas_data.AskVolume5;

    return q2;
}
