#include "quote_lts_l2_udp.h"

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <iomanip>
#include <vector>
#include <thread>

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "query_lts.h"

using namespace my_cmn;
using namespace std;

inline long long GetMicrosFromEpoch()
{
    // get us(micro seconds) from Unix Epoch
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

inline static std::string CreateWindCode(const char *code, const char *ex_id)
{
    std::string sz_wcode(code);
    sz_wcode.append(".");
    if (memcmp(ex_id, "SSE", 3) == 0)
    {
        sz_wcode.append("SH"); // shanghai
    }
    else
    {
        sz_wcode.append("SZ");
    }

    return sz_wcode;
}

inline static int CreateIntTime(const char *t)
{
    //09:30:52.000
    int nt = atoi(t) * 10000000 + atoi(t + 3) * 100000 + atoi(t + 6) * 1000;
    if (t[8] == '.') nt += atoi(t + 9);

    return nt;
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

MYLTSMDL2::MYLTSMDL2(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), recv_thread_(NULL), p_md_l2_save_(NULL), p_md_idx_save_(NULL), p_per_entrust_save_(NULL), p_per_bargain_save_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    // 初始化
    run_flag_ = true;
    current_date = my_cmn::GetCurrentDateInt();
    char qtm_name_buf[64];
    sprintf(qtm_name_buf, "stock_l2_lts_udp_%lu", getpid());
    qtm_name = qtm_name_buf;
    QuoteUpdateState(qtm_name.c_str(), QtmState::INIT);

    // TODO get information of contracts
    {
        MY_LOG_INFO("LTS - try to get static info by query interface");
        MYLTSQueryInterface *query_interface = new MYLTSQueryInterface(cfg_);
        // 等待交易接口获取到合约列表
        int wait_times = 30 * 20; // 30s
        int w = 0;
        bool query_res = true;
        while (!query_interface->TaskFinished())
        {
            usleep(50 * 1000);
            ++w;
            if (w > wait_times)
            {
                query_res = false;
                MY_LOG_ERROR("LTS - query static info not finish in 30 seconds, try again.");
                // break;
                w = 0;
                query_res = true;
            }
        }
        usleep(5 * 1000);

        if (query_res)
        {
            // copy to local
            static_info_of_stocks_.insert(MYLTSQueryInterface::md_staticinfo.begin(), MYLTSQueryInterface::md_staticinfo.end());
            MY_LOG_INFO("LTS - static info total count (%u). try to destroy query interface.", static_info_of_stocks_.size());
        }
        else
        {
            MY_LOG_ERROR("LTS - query static info failed. try to destroy query interface.");
        }

        delete query_interface;
    }

    p_md_l2_save_ = new QuoteDataSave<TDF_MARKET_DATA_MY>(cfg_, qtm_name, "tdf_stock_l2_lts", TDF_STOCK_QUOTE_TYPE, true);
    p_md_idx_save_ = new QuoteDataSave<TDF_INDEX_DATA_MY>(cfg_, qtm_name, "tdf_idx_lts", TDF_INDEX_QUOTE_TYPE, false);
    p_per_entrust_save_ = new QuoteDataSave<T_PerEntrust>(cfg_, qtm_name, "kmds_per_entrust_lts", KMDS_PER_ENTRUST_TYPE, false);
    p_per_bargain_save_ = new QuoteDataSave<T_PerBargain>(cfg_, qtm_name, "kmds_per_bargin_lts", KMDS_PER_BARGAIN_TYPE, false);

    // start receive thread
    recv_thread_ = new boost::thread(&MYLTSMDL2::StartRecv, this);
}

MYLTSMDL2::~MYLTSMDL2(void)
{
    run_flag_ = false;

    if (recv_thread_)
    {
        if (recv_thread_->joinable())
        {
            recv_thread_->join();
        }
    }

    if (p_md_l2_save_) delete p_md_l2_save_;
    if (p_md_idx_save_) delete p_md_idx_save_;
    if (p_per_entrust_save_) delete p_per_entrust_save_;
    if (p_per_bargain_save_) delete p_per_bargain_save_;
}

int MYLTSMDL2::CreateUdpFD(const std::string& addr_ip, unsigned short port)
{
    int flag_n = 1;
    sockaddr_in listen_addr;

    int udp_client_id = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > udp_client_id)
    {
        MY_LOG_ERROR("Failed to Create Socket");
        return -1;
    }

    if (0 != setsockopt(udp_client_id, SOL_SOCKET, SO_REUSEADDR, (char *) &flag_n, sizeof(flag_n)))
    {
        MY_LOG_ERROR("Failed to Reuse Address");
        return -1;
    }

    if (ioctl(udp_client_id, FIONBIO, &flag_n) < 0)
    {
        MY_LOG_ERROR("Failed to Set FIONBIO");
        return -1;
    }

    flag_n = 1024 * 1024;
    if (0 != setsockopt(udp_client_id, SOL_SOCKET, SO_RCVBUF, (const char *) &flag_n, sizeof(flag_n)))
    {
        MY_LOG_ERROR("Failed to Set Receive Buffer Size");
        return -1;
    }

    memset(&listen_addr, 0x00, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(port);
    listen_addr.sin_addr.s_addr = inet_addr(addr_ip.c_str());

    if (0 != bind(udp_client_id, (sockaddr *) &listen_addr, sizeof(listen_addr)))
    {
        MY_LOG_ERROR("Failed to Bind Socket");
        return -1;
    }

    int Broad = 1;
    if (0 != setsockopt(udp_client_id, SOL_SOCKET, SO_BROADCAST, &Broad, sizeof(Broad)))
    {
        MY_LOG_ERROR("Failed to Set Broadcast");
        return -1;
    }

    return udp_client_id;
}

void MYLTSMDL2::StartRecv()
{
    const char md_level2[3] = "MD";
    const char md_idx[3] = "ID";
    const char md_order[3] = "OD";
    const char md_trade[3] = "TD";

    std::string multicast_addr = cfg_.Logon_config().quote_provider_addrs.front();
    MY_LOG_INFO("multicast_addr:%s", multicast_addr.c_str());
    IPAndPortNum ip_and_port = ParseIPAndPortNum(multicast_addr);
    int udp_fd = CreateUdpFD(ip_and_port.first, ip_and_port.second);
    if (udp_fd < 0)
    {
        MY_LOG_ERROR("LTS_Level2_UDP - CreateUdpFD failed.");
        return;
    }
    QuoteUpdateState(qtm_name.c_str(), QtmState::API_READY);

    int offset;
    sockaddr_in src_addr;
    socklen_t addr_len = sizeof(sockaddr_in);
    int recv_length;
    char recv_buffer[2048];
    char *md_type;

    unsigned short md_len;
    int header_len = sizeof(md_len);
    int md_type_len = 2;

    while (run_flag_)
    {
        addr_len = sizeof(sockaddr_in);
        recv_length = recvfrom(udp_fd, recv_buffer, 2048, 0, (sockaddr *) &src_addr, &addr_len);
        if (recv_length > header_len)
        {
            memcpy(&md_len, recv_buffer, header_len);
            if ((recv_length - header_len - md_type_len) % md_len != 0)
            {
                MY_LOG_WARN("Invalid Fast Market Data, len:%d", recv_length);
                continue;
            }
//            MY_LOG_DEBUG("Receive[len: %d] From [%s][%d]", recv_length, inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port));
            offset = header_len;
            md_type = (char*) (recv_buffer + offset);           //数据类型
            offset = offset + md_type_len;                 //指针指向数据起始
            while (offset < recv_length)
            {
                //循环解包
                if (md_type[0] == md_level2[0])
                {
                    CL2FAST_MD *p_l2 = (CL2FAST_MD*) (recv_buffer + offset);

                    long long cur_tp = GetMicrosFromEpoch();
                    TDF_MARKET_DATA_MY data = Convert(*p_l2);
                    // send to client
                    if (md_l2_handler_
                        && (subscribe_contracts_.empty() || subscribe_contracts_.find(data.szCode) != subscribe_contracts_.end()))
                    {
                        md_l2_handler_(&data);
                    }
                    // save it
                    p_md_l2_save_->OnQuoteData(cur_tp, &data);

//#ifdef OUTPUT_DBG_LOG
//                    MY_LOG_DEBUG("MD_L2 - InstrumentID:%s,TimeStamp:%s,OpenPrice:%.4f,HighPrice:%.4f,"
//                        "BidPriceLevel:%d,OfferPriceLevel:%d,OfferVolumeA:%.4f,"
//                        "BidVolume1:%.4f,BidPrice1:%.4f,BidCount1:%.4f,TotalTradeVolume:%.4f,TotalTradeValue:%.4f",
//                        p_l2->InstrumentID, p_l2->TimeStamp, InvalidToZeroD(p_l2->OpenPrice), InvalidToZeroD(p_l2->HighPrice),
//                        p_l2->BidPriceLevel,p_l2->OfferPriceLevel,p_l2->OfferVolumeA,
//                        p_l2->BidVolume1, InvalidToZeroD(p_l2->BidPrice1), p_l2->BidCount1, InvalidToZeroD(p_l2->TotalTradeVolume),
//                        InvalidToZeroD(p_l2->TotalTradeValue));
//#endif

                    offset += md_len;
                }
                else if (md_type[0] == md_idx[0])
                {
                    CL2FAST_INDEX *p_idx = (CL2FAST_INDEX*) (recv_buffer + offset);

                    long long cur_tp = GetMicrosFromEpoch();
                    TDF_INDEX_DATA_MY data;
                    bool convert_res = Convert(*p_idx, data);

                    if (convert_res)
                    {
                        // send to client
                        if (md_idx_handler_)
                        {
                            md_idx_handler_(&data);
                        }
                        // save it
                        p_md_idx_save_->OnQuoteData(cur_tp, &data);
                    }

#ifdef OUTPUT_DBG_LOG
                    MY_LOG_DEBUG("MD_IDX - ExchangeID:%s,InstrumentID:%s,TimeStamp:%s,TotalVolume:%.4f,TurnOver:%.4f,"
                        "LastIndex:%.4f,OpenIndex:%.4f,convert_res:%d",
                        p_idx->ExchangeID, p_idx->InstrumentID, p_idx->TimeStamp, InvalidToZeroD(p_idx->TotalVolume), InvalidToZeroD(p_idx->TurnOver),
                        InvalidToZeroD(p_idx->LastIndex), InvalidToZeroD(p_idx->OpenIndex), convert_res);
#endif

                    offset += md_len;
                }
                else if (md_type[0] == md_order[0])
                {
                    CL2FAST_ORDER *p_ord = (CL2FAST_ORDER*) (recv_buffer + offset);

                    long long cur_tp = GetMicrosFromEpoch();
                    T_PerEntrust data = Convert(*p_ord);
                    // send to client
                    if (per_entrust_handler_)
                    {
                        per_entrust_handler_(&data);
                    }
                    // save it
                    p_per_entrust_save_->OnQuoteData(cur_tp, &data);

//#ifdef OUTPUT_DBG_LOG
//                    MY_LOG_DEBUG("MD_ORD - ExchangeID:%s,FunctionCode:%c,InstrumentID:%s,OrderTime:%s,"
//                        "Volume:%.4f,Price:%.4f,"
//                        "OrderKind:%c,OrderGroupID:%d,OrderIndex:%d",
//                        p_ord->ExchangeID, p_ord->FunctionCode, p_ord->InstrumentID, p_ord->OrderTime,
//                        InvalidToZeroD(p_ord->Volume), InvalidToZeroD(p_ord->Price),
//                        p_ord->OrderKind, p_ord->OrderGroupID, p_ord->OrderIndex);
//#endif

                    offset += md_len;
                }
                else if (md_type[0] == md_trade[0])
                {
                    CL2FAST_TRADE *p_td = (CL2FAST_TRADE*) (recv_buffer + offset);

                    long long cur_tp = GetMicrosFromEpoch();
                    T_PerBargain data = Convert(*p_td);
                    // send to client
                    if (per_bargain_handler_)
                    {
                        per_bargain_handler_(&data);
                    }
                    // save it
                    p_per_bargain_save_->OnQuoteData(cur_tp, &data);

//#ifdef OUTPUT_DBG_LOG
//                    MY_LOG_DEBUG("MD_TRD - ExchangeID:%s,FunctionCode:%c,InstrumentID:%s,TradeTime:%s,"
//                        "Volume:%.4f,Price:%.4f,"
//                        "OrderKind:%c,TradeGroupID:%d,TradeIndex:%d,BuyIndex:%d,SellIndex:%d",
//                        p_td->ExchangeID, p_td->FunctionCode, p_td->InstrumentID, p_td->TradeTime,
//                        InvalidToZeroD(p_td->Volume), InvalidToZeroD(p_td->Price),
//                        p_td->OrderKind, p_td->TradeGroupID, p_td->TradeIndex, p_td->BuyIndex, p_td->SellIndex);
//#endif

                    offset += md_len;
                }
                else
                {
                    MY_LOG_WARN("Invalid L2 Data: offset:%d,recv_length:%d", offset, recv_length);
                }
            }
        }
    }
}

const CSecurityFtdcMarketDataStaticInfoField * MYLTSMDL2::GetMDStaticInfo(const char *ex_id, const char *code)
{
    std::string ex_code(ex_id);
    ex_code.append(".");
    ex_code.append(code);

    MYMDStaticInfo::iterator it = static_info_of_stocks_.find(ex_code);
    if (it != static_info_of_stocks_.end())
    {
        return &it->second;
    }
    return NULL;
}

inline static unsigned int DoubleCkAndToUInt10000(double d)
{
    return (unsigned int) (InvalidToZeroD(d) * 10000 + 0.5);
}

inline static int DoubleCkAndToInt10000(double d)
{
    return (int) (InvalidToZeroD(d) * 10000 + 0.5);
}
TDF_MARKET_DATA_MY MYLTSMDL2::Convert(const CL2FAST_MD& pd)
{
    TDF_MARKET_DATA_MY data;
    memset(&data, 0, sizeof(data));

    strncpy(data.szWindCode, CreateWindCode(pd.InstrumentID, pd.ExchangeID).c_str(), sizeof(data.szWindCode));
    strncpy(data.szCode, pd.InstrumentID, sizeof(data.szCode));
    data.nActionDay = current_date;             //业务发生日(自然日)
    data.nTradingDay = current_date;            //交易日
    data.nTime = CreateIntTime(pd.TimeStamp);                  //时间(HHMMSSmmm)
//    data.nStatus;                //状态
    data.nOpen = DoubleCkAndToUInt10000(pd.OpenPrice);                 //开盘价 * 10000
    data.nHigh = DoubleCkAndToUInt10000(pd.HighPrice);                 //最高价 * 10000
    data.nLow = DoubleCkAndToUInt10000(pd.LowPrice);                  //最低价 * 10000
    data.nMatch = DoubleCkAndToUInt10000(pd.LastPrice);                //最新价 * 10000

    //data.nAskPrice[10];         //申卖价 * 10000
    data.nAskPrice[0] = DoubleCkAndToUInt10000(pd.OfferPrice1);
    data.nAskPrice[1] = DoubleCkAndToUInt10000(pd.OfferPrice2);
    data.nAskPrice[2] = DoubleCkAndToUInt10000(pd.OfferPrice3);
    data.nAskPrice[3] = DoubleCkAndToUInt10000(pd.OfferPrice4);
    data.nAskPrice[4] = DoubleCkAndToUInt10000(pd.OfferPrice5);
    data.nAskPrice[5] = DoubleCkAndToUInt10000(pd.OfferPrice6);
    data.nAskPrice[6] = DoubleCkAndToUInt10000(pd.OfferPrice7);
    data.nAskPrice[7] = DoubleCkAndToUInt10000(pd.OfferPrice8);
    data.nAskPrice[8] = DoubleCkAndToUInt10000(pd.OfferPrice9);
    data.nAskPrice[9] = DoubleCkAndToUInt10000(pd.OfferPriceA);

    //data.nAskVol[10];           //申卖量
    data.nAskVol[0] = (unsigned int) pd.OfferVolume1;
    data.nAskVol[1] = (unsigned int) pd.OfferVolume2;
    data.nAskVol[2] = (unsigned int) pd.OfferVolume3;
    data.nAskVol[3] = (unsigned int) pd.OfferVolume4;
    data.nAskVol[4] = (unsigned int) pd.OfferVolume5;
    data.nAskVol[5] = (unsigned int) pd.OfferVolume6;
    data.nAskVol[6] = (unsigned int) pd.OfferVolume7;
    data.nAskVol[7] = (unsigned int) pd.OfferVolume8;
    data.nAskVol[8] = (unsigned int) pd.OfferVolume9;
    data.nAskVol[9] = (unsigned int) pd.OfferVolumeA;

    //data.nBidPrice[10];         //申买价 * 10000
    data.nBidPrice[0] = DoubleCkAndToUInt10000(pd.BidPrice1);
    data.nBidPrice[1] = DoubleCkAndToUInt10000(pd.BidPrice2);
    data.nBidPrice[2] = DoubleCkAndToUInt10000(pd.BidPrice3);
    data.nBidPrice[3] = DoubleCkAndToUInt10000(pd.BidPrice4);
    data.nBidPrice[4] = DoubleCkAndToUInt10000(pd.BidPrice5);
    data.nBidPrice[5] = DoubleCkAndToUInt10000(pd.BidPrice6);
    data.nBidPrice[6] = DoubleCkAndToUInt10000(pd.BidPrice7);
    data.nBidPrice[7] = DoubleCkAndToUInt10000(pd.BidPrice8);
    data.nBidPrice[8] = DoubleCkAndToUInt10000(pd.BidPrice9);
    data.nBidPrice[9] = DoubleCkAndToUInt10000(pd.BidPriceA);

    //data.nBidVol[10];           //申买量
    data.nBidVol[0] = (unsigned int) pd.BidVolume1;
    data.nBidVol[1] = (unsigned int) pd.BidVolume2;
    data.nBidVol[2] = (unsigned int) pd.BidVolume3;
    data.nBidVol[3] = (unsigned int) pd.BidVolume4;
    data.nBidVol[4] = (unsigned int) pd.BidVolume5;
    data.nBidVol[5] = (unsigned int) pd.BidVolume6;
    data.nBidVol[6] = (unsigned int) pd.BidVolume7;
    data.nBidVol[7] = (unsigned int) pd.BidVolume8;
    data.nBidVol[8] = (unsigned int) pd.BidVolume9;
    data.nBidVol[9] = (unsigned int) pd.BidVolumeA;

    data.nNumTrades = (unsigned int) (InvalidToZeroD(pd.TradeCount));            //成交笔数
    data.iVolume = (long long) (InvalidToZeroD(pd.TotalTradeVolume));              //成交总量
    data.iTurnover = (long long) (InvalidToZeroD(pd.TotalTradeValue));                //成交总金额
    data.nTotalBidVol = (long long) (InvalidToZeroD(pd.TotalBidVolume));         //委托买入总量
    data.nTotalAskVol = (long long) (InvalidToZeroD(pd.TotalOfferVolume));         //委托卖出总量
    data.nWeightedAvgBidPrice = DoubleCkAndToUInt10000(pd.WeightedAvgBidPrice);  //加权平均委买价格 * 10000
    data.nWeightedAvgAskPrice = DoubleCkAndToUInt10000(pd.WeightedAvgOfferPrice);  //加权平均委卖价格 * 10000
    data.nIOPV = DoubleCkAndToInt10000(pd.IOPV);                  //IOPV净值估值  * 10000 （基金）
    data.nYieldToMaturity = DoubleCkAndToInt10000(pd.YieldToMaturity);       //到期收益率 * 10000 （债券）
//    data.chPrefix[4];           //证券信息前缀
//    data.nSyl1;                  //市盈率1  未使用（当前值为0）
//    data.nSyl2;                  //市盈率2  未使用（当前值为0）
//    data.nSD2;                   //升跌2（对比上一笔）    未使用（当前值为0）

    const CSecurityFtdcMarketDataStaticInfoField *p = GetMDStaticInfo(pd.ExchangeID, pd.InstrumentID);
    if (p)
    {
        data.nPreClose = DoubleCkAndToUInt10000(p->PreClosePrice);             // 前收盘价 * 10000
        data.nHighLimited = DoubleCkAndToUInt10000(p->UpperLimitPrice);          // 涨停价 * 10000
        data.nLowLimited = DoubleCkAndToUInt10000(p->LowerLimitPrice);           // 跌停价 * 10000
    }

    return data;
}

bool MYLTSMDL2::Convert(const CL2FAST_INDEX &pd, TDF_INDEX_DATA_MY &data)
{
    std::string idx_key = pd.ExchangeID;
    idx_key.append(".");
    idx_key.append(pd.InstrumentID);
    IdxCacheMap::iterator it = idx_cache_md_.find(idx_key);

    if (it == idx_cache_md_.end())
    {
        it = idx_cache_md_.insert(std::make_pair(idx_key, IdxCacheData(pd))).first;
    }

    memset(&data, 0, sizeof(data));

    strncpy(data.szWindCode, CreateWindCode(pd.InstrumentID, pd.ExchangeID).c_str(), sizeof(data.szWindCode));
    strncpy(data.szCode, pd.InstrumentID, sizeof(data.szCode));
    data.nActionDay = current_date;             //业务发生日(自然日)
    data.nTradingDay = current_date;            //交易日
    data.nTime = CreateIntTime(pd.TimeStamp);          //时间(HHMMSSmmm)

    data.nOpenIndex = DoubleCkAndToInt10000(pd.OpenIndex);     //今开盘指数
    if (data.nOpenIndex == 0) data.nOpenIndex = it->second.nOpenIndex;

    data.nHighIndex = DoubleCkAndToInt10000(pd.HighIndex);     //最高指数
    if (data.nHighIndex == 0) data.nHighIndex = it->second.nHighIndex;

    data.nLowIndex = DoubleCkAndToInt10000(pd.LowIndex);      //最低指数
    if (data.nLowIndex == 0) data.nLowIndex = it->second.nLowIndex;

    data.nLastIndex = DoubleCkAndToInt10000(pd.LastIndex);     //最新指数
    if (data.nLastIndex == 0) data.nLastIndex = it->second.nLastIndex;

    data.iTotalVolume = (long long) (InvalidToZeroD(pd.TotalVolume)); //参与计算相应指数的交易数量
    if (data.iTotalVolume == 0) data.iTotalVolume = it->second.iTotalVolume;

    data.iTurnover = (long long) (InvalidToZeroD(pd.TurnOver) * 10000 + 0.5);        //参与计算相应指数的成交金额
    if (data.iTurnover == 0) data.iTurnover = it->second.iTurnover;

    const CSecurityFtdcMarketDataStaticInfoField *p = GetMDStaticInfo(pd.ExchangeID, pd.InstrumentID);
    if (p)
    {
        data.nPreCloseIndex = DoubleCkAndToInt10000(p->PreClosePrice); // 前盘指数
    }

    it->second.Update(data);
    return true;
}

T_PerEntrust MYLTSMDL2::Convert(const CL2FAST_ORDER& pd)
{
    T_PerEntrust data;
    memset(&data, 0, sizeof(data));

    data.market = GetMYExchangeIDByName(pd.ExchangeID);
    strncpy(data.scr_code, pd.InstrumentID, sizeof(data.scr_code));
    data.entrt_time = CreateIntTime(pd.OrderTime);        // 委托时间
    data.entrt_price = (int) (pd.Price * 10000 + 0.5);
    data.entrt_id = pd.OrderIndex;                        // 委托编号
    data.entrt_vol = (long long) pd.Volume;               // 委托数量
    data.insr_txn_tp_code[0] = pd.FunctionCode;           // 指令交易类型 'B','S'
    data.entrt_tp[0] = pd.OrderKind;                      // 委托类别

    return data;
}

T_PerBargain MYLTSMDL2::Convert(const CL2FAST_TRADE& pd)
{
    T_PerBargain data;
    memset(&data, 0, sizeof(data));

    data.market = GetMYExchangeIDByName(pd.ExchangeID);
    strncpy(data.scr_code, pd.InstrumentID, sizeof(data.scr_code));
    data.time = CreateIntTime(pd.TradeTime);
    data.bgn_id = pd.TradeIndex;                        // 成交编号
    data.bgn_price = (int) (pd.Price * 10000 + 0.5);          // 成交价格
    data.bgn_qty = (long long) pd.Volume;                // 成交数量
    data.bgn_amt = data.bgn_qty * data.bgn_price;       // 成交金额
    data.bgn_flg[0] = pd.OrderKind;                     // 成交类别
    data.nsr_txn_tp_code[0] = pd.FunctionCode;          // 指令交易类型

    return data;
}
