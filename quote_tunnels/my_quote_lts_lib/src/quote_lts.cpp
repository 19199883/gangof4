#include "quote_lts.h"
#include <dirent.h>
#include <iomanip>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "query_lts.h"

using namespace my_cmn;
using namespace std;

static void TrimTailSpace(char p[], std::size_t len)
{
    for (std::size_t i = 0; i < len; ++i)
    {
        if (p[i] == ' ')
        {
            p[i] = '\0';
            break;
        }
    }
}

inline long long GetMicrosFromEpoch()
{
    // get us(micro seconds) from Unix Epoch
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

MYLTSDataHandler::MYLTSDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_save_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    qtm_name = "quote_lts_opt_" + boost::lexical_cast<std::string>(getpid());

    // 初始化
    logoned_ = false;
    api_ = NULL;
    const LogonConfig &logon_info = cfg.Logon_config();
    broker_id_ = new char[logon_info.broker_id.size() + 1];
    memcpy(broker_id_, logon_info.broker_id.data(), logon_info.broker_id.size() + 1);

    user_ = new char[logon_info.account.size() + 1];
    memcpy(user_, logon_info.account.data(), logon_info.account.size() + 1);

    password_ = new char[logon_info.password.size() + 1];
    memcpy(password_, logon_info.password.data(), logon_info.password.size() + 1);

    // get information of contracts
    {
        MY_LOG_INFO("LTS - try to get instruments with query interface");
        MYLTSQueryInterface *query_interface = new MYLTSQueryInterface(cfg_);
        // 等待交易接口获取到合约列表
        while (!query_interface->TaskFinished())
        {
            usleep(50 * 1000);
        }
        usleep(5 * 1000);

        // pick option instruments
        for (const MYSecurityCollection::value_type &value : MYLTSQueryInterface::securities_exchcode)
        {
            const CSecurityFtdcInstrumentField &o = value.second;
            if (o.ProductClass == SECURITY_FTDC_PC_Options)
            {
                options_[o.ExchangeID].push_back(o);
                OptionOfContract::iterator it = option_info_.insert(std::make_pair(o.InstrumentID, o)).first;
                TrimTailSpace(it->second.ExchangeInstID, sizeof(o.ExchangeInstID));
            }
        }

        MY_LOG_INFO("LTS - get %d instruments, %d are option type. try to destroy query interface.",
            MYLTSQueryInterface::securities_exchcode.size(), option_info_.size());
        delete query_interface;
    }

    p_save_ = new QuoteDataSave<T_OptionQuote>(cfg_, qtm_name, "quote_lts_option", KMDS_OPTION_QUOTE_TYPE);
    const LogonConfig &logon_cfg = cfg_.Logon_config();

    pp_instruments_ = NULL;
    p_ex_ = NULL;
    sub_count_ = 0;

    //Create Directory
    char cur_path[256];
    char full_path[256];
    getcwd(cur_path, sizeof(cur_path));
    sprintf(full_path, "%s/flow_md_%s/", cur_path, cfg_.Logon_config().account.c_str());

    // check whether dir exist
    if (opendir(full_path) == NULL)
    {
        mkdir(full_path, 0755);
    }

    // 初始化
    api_ = CSecurityFtdcMdApi::CreateFtdcMdApi(full_path);
    api_->RegisterSpi(this);

    // set front address
    for (const std::string &v : logon_cfg.quote_provider_addrs)
    {
        char *addr_tmp = new char[v.size() + 1];
        memcpy(addr_tmp, v.c_str(), v.size() + 1);
        api_->RegisterFront(addr_tmp);
        QuoteUpdateState(qtm_name.c_str(), QtmState::SET_ADDRADRESS_SUCCESS);
        MY_LOG_INFO("LTS - RegisterFront, addr: %s", addr_tmp);
        delete[] addr_tmp;
    }

    api_->Init();
}

MYLTSDataHandler::~MYLTSDataHandler(void)
{
    if (api_)
    {
        api_->RegisterSpi(NULL);
        api_->Release();
        api_ = NULL;
    }

    if (p_save_)
        delete p_save_;
}

void MYLTSDataHandler::ReqLogin()
{
    static bool first_login = true;
    if (first_login)
    {
        first_login = false;
    }
    else
    {
        sleep(30); // wait 30 seconds for retry login
    }

    CSecurityFtdcReqUserLoginField req_login;
    memset(&req_login, 0, sizeof(CSecurityFtdcReqUserLoginField));
    strncpy(req_login.BrokerID, broker_id_, sizeof(req_login.BrokerID));
    strncpy(req_login.UserID, user_, sizeof(req_login.UserID));
    strncpy(req_login.Password, password_, sizeof(req_login.Password));
    api_->ReqUserLogin(&req_login, 0);

    MY_LOG_INFO("LTS - request logon");
}

void MYLTSDataHandler::OnFrontConnected()
{
    MY_LOG_INFO("LTS: OnFrontConnected");
    QuoteUpdateState(qtm_name.c_str(), QtmState::CONNECT_SUCCESS);

    boost::thread req_login(&MYLTSDataHandler::ReqLogin, this);
}

void MYLTSDataHandler::OnFrontDisconnected(int nReason)
{
    logoned_ = false;
    QuoteUpdateState(qtm_name.c_str(), QtmState::DISCONNECT);
    MY_LOG_ERROR("LTS - OnFrontDisconnected, nReason: %d", nReason);
}

void MYLTSDataHandler::OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
{
    int error_code = pRspInfo ? pRspInfo->ErrorID : 0;
    MY_LOG_INFO("LTS - OnRspUserLogin, error code: %d", error_code);

    if (error_code == 0)
    {
        logoned_ = true;
        QuoteUpdateState(qtm_name.c_str(), QtmState::LOG_ON_SUCCESS);

        for (const OptionsOfEx::value_type &value : options_)
        {
            const std::list<CSecurityFtdcInstrumentField> &o_list = value.second;
            const std::string &ex_id = value.first;
            sub_count_ = o_list.size();
            if (sub_count_ == 0)
            {
                MY_LOG_WARN("LTS - no option in exchange %s", ex_id.c_str());
                continue;
            }
            pp_instruments_ = new char *[sub_count_];
            p_ex_ = new char[ex_id.size() + 1];
            memcpy(p_ex_, ex_id.data(), ex_id.size() + 1);

            int idx = 0;
            instruments_.clear();
            for (const CSecurityFtdcInstrumentField &o : o_list)
            {
                instruments_.append(std::string(o.InstrumentID) + "|");
                pp_instruments_[idx] = new char[strlen(o.InstrumentID) + 1];
                memcpy(pp_instruments_[idx], o.InstrumentID, strlen(o.InstrumentID) + 1);
                ++idx;
            }
            int sub_ret = api_->SubscribeMarketData(pp_instruments_, sub_count_, p_ex_);
            MY_LOG_INFO("LTS - SubMarketData, return:%d, count:%d, codelist: %s", sub_ret, sub_count_, instruments_.c_str());
            QuoteUpdateState(qtm_name.c_str(), QtmState::API_READY);

            // release memory
            delete[] p_ex_;
            for (int j = 0; j < idx; ++j)
            {
                delete[] pp_instruments_[j];
            }
            delete[] pp_instruments_;
        }
    }
    else
    {
        std::string err_str("null");
        if (pRspInfo && pRspInfo->ErrorMsg[0] != '\0')
        {
            err_str = pRspInfo->ErrorMsg;
        }
        QuoteUpdateState(qtm_name.c_str(), QtmState::LOG_ON_FAIL);
        MY_LOG_WARN("LTS - Logon fail, error code: %d; error info: %s", error_code, err_str.c_str());

        // 登录失败，不断重试
        boost::thread req_login(&MYLTSDataHandler::ReqLogin, this);
    }
}

void MYLTSDataHandler::OnRspSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, CSecurityFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    MY_LOG_DEBUG("LTS - OnRspSubMarketData, code: %s", pSpecificInstrument->InstrumentID);
}

void MYLTSDataHandler::OnRspUnSubMarketData(CSecurityFtdcSpecificInstrumentField *pSpecificInstrument,
    CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    MY_LOG_DEBUG("LTS - OnRspUnSubMarketData, code: %s", pSpecificInstrument->InstrumentID);
}

void MYLTSDataHandler::OnRtnDepthMarketData(CSecurityFtdcDepthMarketDataField *p)
{
    try
    {
        long long cur_tp = GetMicrosFromEpoch();

        RalaceInvalidValue_LTS(*p);
        T_OptionQuote option = Convert(*p);

        if (quote_data_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->InstrumentID) != subscribe_contracts_.end()))
        {
            quote_data_handler_(&option);
        }

        // 存起来
        p_save_->OnQuoteData(cur_tp, &option);

//        std::stringstream ss;
//        ss << "OnRtnDepthMarketData" << endl;
//        ss << "\tTradingDay=" << p->TradingDay << endl;
//        ss << "\tInstrumentID=" << p->InstrumentID << endl;
//        ss << "\tExchangeID=" << p->ExchangeID << endl;
//        ss << "\tExchangeInstID=" << p->ExchangeInstID << endl;
//        ss << "\tLastPrice=" << p->LastPrice << endl;
//        ss << "\tPreSettlementPrice=" << p->PreSettlementPrice << endl;
//        ss << "\tPreClosePrice=" << p->PreClosePrice << endl;
//        ss << "\tPreOpenInterest=" << p->PreOpenInterest << endl;
//        ss << "\tOpenPrice=" << p->OpenPrice << endl;
//        ss << "\tHighestPrice=" << p->HighestPrice << endl;
//        ss << "\tLowestPrice=" << p->LowestPrice << endl;
//        ss << "\tVolume=" << p->Volume << endl;
//        ss << "\tTurnover=" << p->Turnover << endl;
//        ss << "\tOpenInterest=" << p->OpenInterest << endl;
//        ss << "\tClosePrice=" << p->ClosePrice << endl;
//        ss << "\tSettlementPrice=" << p->SettlementPrice << endl;
//        ss << "\tUpperLimitPrice=" << p->UpperLimitPrice << endl;
//        ss << "\tLowerLimitPrice=" << p->LowerLimitPrice << endl;
//        ss << "\tPreDelta=" << p->PreDelta << endl;
//        ss << "\tCurrDelta=" << p->CurrDelta << endl;
//        ss << "\tPreIOPV=" << p->PreIOPV << endl;
//        ss << "\tIOPV=" << p->IOPV << endl;
//        ss << "\tAuctionPrice=" << p->AuctionPrice << endl;
//        ss << "\tUpdateTime=" << p->UpdateTime << endl;
//        ss << "\tUpdateMillisec=" << p->UpdateMillisec << endl;
//        ss << "\tBidPrice1=" << p->BidPrice1 << endl;
//        ss << "\tBidVolume1=" << p->BidVolume1 << endl;
//        ss << "\tAskPrice1=" << p->AskPrice1 << endl;
//        ss << "\tAskVolume1=" << p->AskVolume1 << endl;
//        ss << "\tBidPrice2=" << p->BidPrice2 << endl;
//        ss << "\tBidVolume2=" << p->BidVolume2 << endl;
//        ss << "\tAskPrice2=" << p->AskPrice2 << endl;
//        ss << "\tAskVolume2=" << p->AskVolume2 << endl;
//        ss << "\tBidPrice3=" << p->BidPrice3 << endl;
//        ss << "\tBidVolume3=" << p->BidVolume3 << endl;
//        ss << "\tAskPrice3=" << p->AskPrice3 << endl;
//        ss << "\tAskVolume3=" << p->AskVolume3 << endl;
//        ss << "\tBidPrice4=" << p->BidPrice4 << endl;
//        ss << "\tBidVolume4=" << p->BidVolume4 << endl;
//        ss << "\tAskPrice4=" << p->AskPrice4 << endl;
//        ss << "\tAskVolume4=" << p->AskVolume4 << endl;
//        ss << "\tBidPrice5=" << p->BidPrice5 << endl;
//        ss << "\tBidVolume5=" << p->BidVolume5 << endl;
//        ss << "\tAskPrice5=" << p->AskPrice5 << endl;
//        ss << "\tAskVolume5=" << p->AskVolume5 << endl;
//        ss << "\tAveragePrice=" << p->AveragePrice << endl;
//        ss << "\tActionDay=" << p->ActionDay << endl;
//        ss << "\tTradingPhase=" << p->TradingPhase << endl;
//        ss << "\tOpenRestriction=" << p->OpenRestriction << endl;
//
//        MY_LOG_DEBUG("%s", ss.str().c_str());
    }
    catch (...)
    {
        MY_LOG_FATAL("LTS - Unknown exception in OnRtnDepthMarketData.");
    }
}

void MYLTSDataHandler::OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int error_code = pRspInfo ? 0 : pRspInfo->ErrorID;
    if (error_code != 0)
    {
        MY_LOG_INFO("LTS - OnRspError, code: %d; info: %s", error_code, pRspInfo->ErrorMsg);
    }
}

void MYLTSDataHandler::SetQuoteDataHandler(boost::function<void(const T_OptionQuote *)> quote_data_handler)
{
    quote_data_handler_ = quote_data_handler;
}

void MYLTSDataHandler::RalaceInvalidValue_LTS(CSecurityFtdcDepthMarketDataField &d)
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
    d.AveragePrice = InvalidToZeroD(d.AveragePrice);

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

inline char GetMYExchangeIDByName(const char *ex_name)
{
    if (memcmp(ex_name, "SSE", 3) == 0)
    {
        return '1';
    }
    if (memcmp(ex_name, "SZE", 3) == 0)
    {
        return '0';
    }

    return '?';
}

inline static unsigned int DoubleToLong10000(double d)
{
    return (INT64) (d * 10000 + 0.5);
}

T_OptionQuote MYLTSDataHandler::Convert(const CSecurityFtdcDepthMarketDataField &d)
{
    T_OptionQuote oq;
    memset(&oq, 0, sizeof(oq));

    strncpy(oq.scr_code, d.InstrumentID, sizeof(oq.scr_code));

    //UpdateTime=13:54:13
    //UpdateMillisec=770
    oq.time = atoi(d.UpdateTime) * 10000000 + atoi(d.UpdateTime + 3) * 100000 + atoi(d.UpdateTime + 6) * 1000 + d.UpdateMillisec;  // 行情时间 HHMMSSmmm

    // ExchangeID=SSE
    oq.market = GetMYExchangeIDByName(d.ExchangeID);
    //oq.scr_code[SCR_CODE_LEN];
    //oq.seq_id;                   // 快照序号
    oq.high_price =DoubleToLong10000(d.HighestPrice);
    oq.low_price = DoubleToLong10000(d.LowestPrice);
    oq.last_price = DoubleToLong10000(d.LastPrice);
    oq.bgn_total_vol = d.Volume;
    oq.bgn_total_amt = DoubleToLong10000(d.Turnover);
    oq.auction_price = DoubleToLong10000(d.AuctionPrice);            // 动态参考价格
    //oq.auction_qty;              // 虚拟匹配数量

    oq.sell_price[0] = DoubleToLong10000(d.AskPrice1);
    oq.sell_price[1] = DoubleToLong10000(d.AskPrice2);
    oq.sell_price[2] = DoubleToLong10000(d.AskPrice3);
    oq.sell_price[3] = DoubleToLong10000(d.AskPrice4);
    oq.sell_price[4] = DoubleToLong10000(d.AskPrice5);

    oq.bid_price[0] = DoubleToLong10000(d.BidPrice1);
    oq.bid_price[1] = DoubleToLong10000(d.BidPrice2);
    oq.bid_price[2] = DoubleToLong10000(d.BidPrice3);
    oq.bid_price[3] = DoubleToLong10000(d.BidPrice4);
    oq.bid_price[4] = DoubleToLong10000(d.BidPrice5);

    oq.sell_volume[0] = d.AskVolume1;
    oq.sell_volume[1] = d.AskVolume2;
    oq.sell_volume[2] = d.AskVolume3;
    oq.sell_volume[3] = d.AskVolume4;
    oq.sell_volume[4] = d.AskVolume5;

    oq.bid_volume[0] = d.BidVolume1;
    oq.bid_volume[1] = d.BidVolume2;
    oq.bid_volume[2] = d.BidVolume3;
    oq.bid_volume[3] = d.BidVolume4;
    oq.bid_volume[4] = d.BidVolume5;

    oq.long_position = d.OpenInterest;            // 当前合约未平仓数

//    INT64 error_mark;               // 错误字段域
//    INT64 publish_tm1;              // 一级发布时间
//    INT64 publish_tm2;              // 二级发布时间
//    INT64 pub_num;                  // 行情信息编号
//    CHAR8 trad_phase[4];            // 产品实施阶段及标志
//    CHAR8 md_report_id[12];         // 行情信息编号

    // added on 20150709 (static fields from code table)
    //oq.contract_code[SCR_CODE_LEN];  // 510050C1512M02650
    oq.high_limit_price = DoubleToLong10000(d.UpperLimitPrice);             // 涨停价 * 10000
    oq.low_limit_price = DoubleToLong10000(d.LowerLimitPrice);              // 跌停价 * 10000
    oq.pre_close_price = DoubleToLong10000(d.PreClosePrice);              // 昨收盘价 * 10000
    oq.open_price = DoubleToLong10000(d.OpenPrice);                   // 开盘价 * 10000

    OptionOfContract::const_iterator cit = option_info_.find(oq.scr_code);
    if (cit != option_info_.end())
    {
        strncpy(oq.contract_code, cit->second.ExchangeInstID, sizeof(oq.contract_code));
    }

    return oq;
}
