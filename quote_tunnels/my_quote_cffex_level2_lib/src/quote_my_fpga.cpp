#include "quote_my_fpga.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

#include "USTPFtdcUserApiDataType.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"
#include "toe_calc_const.h"

using namespace std;

int MYFPGACffexDataHandler::md_seq_no = -1;
std::mutex MYFPGACffexDataHandler::seq_lock;

#pragma pack(push, 1)
///深度行情
struct MYFpgaDataField
{
    char reserve[8];
    int seq_no;
    char reserve2[11];
    ///交易日
    TUstpFtdcDateType TradingDay;
    ///结算组代码
    //TUstpFtdcSettlementGroupIDType  SettlementGroupID;
    ///结算编号
    //TUstpFtdcSettlementIDType   SettlementID;
    ///昨结算
    TUstpFtdcPriceType PreSettlementPrice;
    ///昨收盘
    TUstpFtdcPriceType PreClosePrice;
    ///昨持仓量
    TUstpFtdcLargeVolumeType PreOpenInterest;
    ///昨虚实度
    TUstpFtdcRatioType PreDelta;
    ///今开盘
    TUstpFtdcPriceType OpenPrice;
    ///最高价
    TUstpFtdcPriceType HighestPrice;
    ///最低价
    TUstpFtdcPriceType LowestPrice;
    ///今收盘
    TUstpFtdcPriceType ClosePrice;
    ///涨停板价
    TUstpFtdcPriceType UpperLimitPrice;
    ///跌停板价
    TUstpFtdcPriceType LowerLimitPrice;
    ///今结算
    TUstpFtdcPriceType SettlementPrice;
    ///今虚实度
    //TUstpFtdcRatioType  CurrDelta;
    ///最新价
    TUstpFtdcPriceType LastPrice;
    ///数量
    TUstpFtdcVolumeType Volume;
    ///成交金额
    TUstpFtdcMoneyType Turnover;
    ///持仓量
    TUstpFtdcLargeVolumeType OpenInterest;
    ///申买价一
    TUstpFtdcPriceType BidPrice1;
    ///申买量一
    TUstpFtdcVolumeType BidVolume1;
    ///申卖价一
    TUstpFtdcPriceType AskPrice1;
    ///申卖量一
    TUstpFtdcVolumeType AskVolume1;
    ///申买价二
    TUstpFtdcPriceType BidPrice2;
    ///申买量二
    TUstpFtdcVolumeType BidVolume2;
    ///申买价三
    TUstpFtdcPriceType BidPrice3;
    ///申买量三
    TUstpFtdcVolumeType BidVolume3;
    ///申卖价二
    TUstpFtdcPriceType AskPrice2;
    ///申卖量二
    TUstpFtdcVolumeType AskVolume2;
    ///申卖价三
    TUstpFtdcPriceType AskPrice3;
    ///申卖量三
    TUstpFtdcVolumeType AskVolume3;
    ///申买价四
    TUstpFtdcPriceType BidPrice4;
    ///申买量四
    TUstpFtdcVolumeType BidVolume4;
    ///申买价五
    TUstpFtdcPriceType BidPrice5;
    ///申买量五
    TUstpFtdcVolumeType BidVolume5;
    ///申卖价四
    TUstpFtdcPriceType AskPrice4;
    ///申卖量四
    TUstpFtdcVolumeType AskVolume4;
    ///申卖价五
    TUstpFtdcPriceType AskPrice5;
    ///申卖量五
    TUstpFtdcVolumeType AskVolume5;
    ///合约代码
    TUstpFtdcInstrumentIDType InstrumentID;
    ///最后修改时间
    TUstpFtdcTimeType UpdateTime;
    ///最后修改毫秒
    TUstpFtdcMillisecType UpdateMillisec;
};
#pragma pack(pop)

inline long long GetMicrosFromEpoch()
{
    // get ns(nano seconds) from Unix Epoch
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}
inline long long GetNanosFromEpoch()
{
    // get ns(nano seconds) from Unix Epoch
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static CFfexFtdcDepthMarketData Convert(const MYFpgaDataField &xele_data)
{
    CFfexFtdcDepthMarketData q2;
    memset(&q2, 0, sizeof(CFfexFtdcDepthMarketData));

//    memcpy(q2.szTradingDay, xele_data.TradingDay, 9);
//    memcpy(q2.szSettlementGroupID, xele_data.SettlementGroupID, 9);
//    q2.nSettlementID = xele_data.SettlementID;
//    q2.dPreSettlementPrice = xele_data.PreSettlementPrice;
//    q2.dPreClosePrice = xele_data.PreClosePrice;
//    q2.dPreOpenInterest = xele_data.PreOpenInterest;
    q2.dLastPrice = xele_data.LastPrice;
    q2.dOpenPrice = xele_data.OpenPrice;
    q2.dHighestPrice = xele_data.HighestPrice;
    q2.dLowestPrice = xele_data.LowestPrice;
    q2.nVolume = xele_data.Volume;
    q2.dTurnover = xele_data.Turnover;
    q2.dOpenInterest = xele_data.OpenInterest;
    q2.dClosePrice = xele_data.ClosePrice;
    q2.dSettlementPrice = xele_data.SettlementPrice;
    q2.dUpperLimitPrice = xele_data.UpperLimitPrice;
    q2.dLowerLimitPrice = xele_data.LowerLimitPrice;
    //q2.dPreDelta = xele_data.PreDelta;
    //q2.dCurrDelta = xele_data.CurrDelta;
    memcpy(q2.szUpdateTime, xele_data.UpdateTime, 9);
    q2.nUpdateMillisec = xele_data.UpdateMillisec;
    memcpy(q2.szInstrumentID, xele_data.InstrumentID, 31);
    q2.dBidPrice1 = xele_data.BidPrice1;
    q2.nBidVolume1 = xele_data.BidVolume1;
    q2.dAskPrice1 = xele_data.AskPrice1;
    q2.nAskVolume1 = xele_data.AskVolume1;
    q2.dBidPrice2 = xele_data.BidPrice2;
    q2.nBidVolume2 = xele_data.BidVolume2;
    q2.dAskPrice2 = xele_data.AskPrice2;
    q2.nAskVolume2 = xele_data.AskVolume2;
    q2.dBidPrice3 = xele_data.BidPrice3;
    q2.nBidVolume3 = xele_data.BidVolume3;
    q2.dAskPrice3 = xele_data.AskPrice3;
    q2.nAskVolume3 = xele_data.AskVolume3;
    q2.dBidPrice4 = xele_data.BidPrice4;
    q2.nBidVolume4 = xele_data.BidVolume4;
    q2.dAskPrice4 = xele_data.AskPrice4;
    q2.nAskVolume4 = xele_data.AskVolume4;
    q2.dBidPrice5 = xele_data.BidPrice5;
    q2.nBidVolume5 = xele_data.BidVolume5;
    q2.dAskPrice5 = xele_data.AskPrice5;
    q2.nAskVolume5 = xele_data.AskVolume5;

    return q2;
}
static void RalaceInvalidValue(MYFpgaDataField &d)
{
    std::reverse((char *) &d.Turnover, (char *) &d.Turnover + sizeof(d.Turnover));
    std::reverse((char *) &d.LastPrice, (char *) &d.LastPrice + sizeof(d.LastPrice));
    std::reverse((char *) &d.UpperLimitPrice, (char *) &d.UpperLimitPrice + sizeof(d.UpperLimitPrice));
    std::reverse((char *) &d.LowerLimitPrice, (char *) &d.LowerLimitPrice + sizeof(d.LowerLimitPrice));
    std::reverse((char *) &d.HighestPrice, (char *) &d.HighestPrice + sizeof(d.HighestPrice));
    std::reverse((char *) &d.LowestPrice, (char *) &d.LowestPrice + sizeof(d.LowestPrice));
    std::reverse((char *) &d.OpenPrice, (char *) &d.OpenPrice + sizeof(d.OpenPrice));
    std::reverse((char *) &d.ClosePrice, (char *) &d.ClosePrice + sizeof(d.ClosePrice));
    std::reverse((char *) &d.OpenInterest, (char *) &d.OpenInterest + sizeof(d.OpenInterest));
    std::reverse((char *) &d.BidPrice1, (char *) &d.BidPrice1 + sizeof(d.BidPrice1));
    std::reverse((char *) &d.BidPrice2, (char *) &d.BidPrice2 + sizeof(d.BidPrice2));
    std::reverse((char *) &d.BidPrice3, (char *) &d.BidPrice3 + sizeof(d.BidPrice3));
    std::reverse((char *) &d.BidPrice4, (char *) &d.BidPrice4 + sizeof(d.BidPrice4));
    std::reverse((char *) &d.BidPrice5, (char *) &d.BidPrice5 + sizeof(d.BidPrice5));
    std::reverse((char *) &d.AskPrice1, (char *) &d.AskPrice1 + sizeof(d.AskPrice1));
    std::reverse((char *) &d.AskPrice2, (char *) &d.AskPrice2 + sizeof(d.AskPrice2));
    std::reverse((char *) &d.AskPrice3, (char *) &d.AskPrice3 + sizeof(d.AskPrice3));
    std::reverse((char *) &d.AskPrice4, (char *) &d.AskPrice4 + sizeof(d.AskPrice4));
    std::reverse((char *) &d.AskPrice5, (char *) &d.AskPrice5 + sizeof(d.AskPrice5));
    std::reverse((char *) &d.SettlementPrice, (char *) &d.SettlementPrice + sizeof(d.SettlementPrice));

    std::reverse((char *) &d.BidVolume1, (char *) &d.BidVolume1 + sizeof(d.BidVolume1));
    std::reverse((char *) &d.BidVolume2, (char *) &d.BidVolume2 + sizeof(d.BidVolume2));
    std::reverse((char *) &d.BidVolume3, (char *) &d.BidVolume3 + sizeof(d.BidVolume3));
    std::reverse((char *) &d.BidVolume4, (char *) &d.BidVolume4 + sizeof(d.BidVolume4));
    std::reverse((char *) &d.BidVolume5, (char *) &d.BidVolume5 + sizeof(d.BidVolume5));

    std::reverse((char *) &d.AskVolume1, (char *) &d.AskVolume1 + sizeof(d.AskVolume1));
    std::reverse((char *) &d.AskVolume2, (char *) &d.AskVolume2 + sizeof(d.AskVolume2));
    std::reverse((char *) &d.AskVolume3, (char *) &d.AskVolume3 + sizeof(d.AskVolume3));
    std::reverse((char *) &d.AskVolume4, (char *) &d.AskVolume4 + sizeof(d.AskVolume4));
    std::reverse((char *) &d.AskVolume5, (char *) &d.AskVolume5 + sizeof(d.AskVolume5));

    std::reverse((char *) &d.Volume, (char *) &d.Volume + sizeof(d.Volume));
    std::reverse((char *) &d.UpdateMillisec, (char *) &d.UpdateMillisec + sizeof(d.UpdateMillisec));

    d.Turnover = InvalidToZeroD(d.Turnover);
    d.LastPrice = InvalidToZeroD(d.LastPrice);
    d.UpperLimitPrice = InvalidToZeroD(d.UpperLimitPrice);
    d.LowerLimitPrice = InvalidToZeroD(d.LowerLimitPrice);
    d.HighestPrice = InvalidToZeroD(d.HighestPrice);
    d.LowestPrice = InvalidToZeroD(d.LowestPrice);
    d.OpenPrice = InvalidToZeroD(d.OpenPrice);
    d.ClosePrice = InvalidToZeroD(d.ClosePrice);
    //d.PreClosePrice = InvalidToZeroD(d.PreClosePrice);
    d.OpenInterest = InvalidToZeroD(d.OpenInterest);
    //d.PreOpenInterest = InvalidToZeroD(d.PreOpenInterest);

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
    //d.PreSettlementPrice = InvalidToZeroD(d.PreSettlementPrice);

    //d.PreDelta = InvalidToZeroD(d.PreDelta);
    //d.CurrDelta = InvalidToZeroD(d.CurrDelta);
}

//static void cleanup_handler(void *arg)
//{
//    MY_LOG_INFO("close toe in cleanup_handler");
//    struct session_init_struct *session = (struct session_init_struct *) arg;
//    toe_close(session);
//    session = NULL;
//}


struct RecvThreadParam
{
    RecvThreadParam(struct pthread_event & ev_,
        boost::function<void(const CFfexFtdcDepthMarketData *)> & h,
        SubscribeContracts &sub,
        volatile bool & r)
        : ev(ev_), data_handler(h), sub_contracts(sub), r_flag(r)
    {
        s = NULL;
        save = NULL;
        cpu_id = 0;
    }
    struct pthread_event &ev;
    boost::function<void(const CFfexFtdcDepthMarketData *)> &data_handler;
    SubscribeContracts & sub_contracts;
    volatile bool &r_flag;
    struct session_init_struct *s;
    QuoteDataSave<CFfexFtdcDepthMarketData> *save;
    int cpu_id;
};

//#define FPGA_TEST

#ifdef FPGA_TEST
    #define IOCTL_TIMER 0x8080
    static int s_stats_count = 0;
    static int s_stats_max = 100000;
    static int s_stats_result[100000];
    static int s_recv_length = 0;
#endif

void *MYFPGACffexDataHandler::toe_recv_data(void *param)
{
    RecvThreadParam *h = (RecvThreadParam *) param;
    boost::function<void(const CFfexFtdcDepthMarketData *)> &quote_data_handler = h->data_handler;
    SubscribeContracts &subscribe_contracts = h->sub_contracts;

    struct pthread_event &ev = h->ev;
    struct session_init_struct *session = h->s;

#ifdef FPGA_TEST
    int fd = open("/dev/mytest", O_RDWR);
    if (fd < 0)
    {
        printf("open /dev/mytest failed");
    }
    int high_level_v = 0x0;
    int low_level_v = 0xff;

    int da_counter = 0;
    int counter_1 = 0;
#endif

    //pthread_cleanup_push(cleanup_handler, session);

    unsigned char *buffer = NULL;
    long rbytes;
    unsigned long size = 2 * 1024;
    buffer = new unsigned char[size];

    PTHREAD_EVENT_SET(ev);
    toe_setaffinity(h->cpu_id);

    MY_LOG_INFO("toe data size:%d", sizeof(MYFpgaDataField));

    while (h->r_flag)
    {
        rbytes = toe_recv(session, buffer, size);
#ifdef FPGA_TEST
        s_recv_length += rbytes;
#endif

        if (rbytes <= 0)
        {
            continue;
        }

        long handled_bytes = 0;
        while ((rbytes - handled_bytes) >= (int) sizeof(MYFpgaDataField))
        {
            long long cur_tp = GetNanosFromEpoch();
            MYFpgaDataField *pd = (MYFpgaDataField *) (buffer + handled_bytes);
            handled_bytes += sizeof(MYFpgaDataField);

#ifdef FPGA_TEST
            // for performance compare
            std::reverse((char *) &pd->UpdateMillisec, (char *) &pd->UpdateMillisec + sizeof(pd->UpdateMillisec));
            if (pd->UpdateMillisec < 500)
            {
                ++counter_1;
                if (counter_1 == 16) ioctl(fd, IOCTL_TIMER, &high_level_v);
            }
            else
            {
                ioctl(fd, IOCTL_TIMER, &low_level_v);
                counter_1 = 0;
            }
            std::reverse((char *) &pd->UpdateMillisec, (char *) &pd->UpdateMillisec + sizeof(pd->UpdateMillisec));

            ioctl(fd, IOCTL_TIMER, &high_level_v);
            usleep(10);
            ioctl(fd, IOCTL_TIMER, &low_level_v);
            ++da_counter;
            printf("counter:%d, rbytes:%ld\n", da_counter, rbytes);
#endif

            RalaceInvalidValue(*pd);

            CFfexFtdcDepthMarketData q_cffex_level2 = Convert(*pd);

            // send to client when it has subscribed the contract
            if (quote_data_handler
                && (subscribe_contracts.empty() || subscribe_contracts.find(q_cffex_level2.szInstrumentID) != subscribe_contracts.end()))
            {
                quote_data_handler(&q_cffex_level2);
            }

            // save to disk
            h->save->OnQuoteData(cur_tp / 1000, &q_cffex_level2);

#ifdef FPGA_TEST
            if (s_stats_count < s_stats_max)
            {
                long long fin_tp = GetNanosFromEpoch();
                s_stats_result[s_stats_count] = (int) (fin_tp - cur_tp);
                ++s_stats_count;
            }
#endif
        }

        // have invalid package, write log
        if (rbytes != handled_bytes)
        {
            MY_LOG_WARN("recv package length:%ld, handled length:%ld", rbytes, handled_bytes);
        }
    }

#ifdef FPGA_TEST
    if (fd >= 0) close(fd);
#endif

    return NULL;
}

MYFPGACffexDataHandler::MYFPGACffexDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_save_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    sprintf(qtm_name_, "myfpga_cffex_%s_%u", cfg.Logon_config().account.c_str(), getpid());
    QuoteUpdateState(qtm_name_, QtmState::INIT);

    // 初始化
    p_save_ = new QuoteDataSave<CFfexFtdcDepthMarketData>(cfg_, qtm_name_, "cffex_toe", GTA_UDP_CFFEX_QUOTE_TYPE);

    md_seq_no = -1;
    //pthread_spin_init(&seq_lock, 0);
    pthread_attr_init(&thread_attr_);
    pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_DETACHED);
    pthread_attr_setscope(&thread_attr_, PTHREAD_SCOPE_SYSTEM);
    running_flag_ = true;

    for (const TOEConfig &toe_cfg : cfg_.Logon_config().toe_cfg)
    {
        uint8_t cpu_id = (uint8_t) atoi(toe_cfg.connect_cpu_id.c_str());
        uint8_t recv_cpu = (uint8_t) atoi(toe_cfg.data_recv_cpu_id.c_str());
        uint8_t toe_id = (uint8_t) atoi(toe_cfg.toe_id.c_str());
        uint8_t pro_type = (uint8_t) atoi(toe_cfg.pro_type.c_str());
        int local_ip = inet_addr(toe_cfg.local_ip.c_str());
        unsigned short local_port = (unsigned short) atoi(toe_cfg.local_port.c_str());
        int remote_ip = inet_addr(toe_cfg.remote_ip.c_str());
        unsigned short remote_port = (unsigned short) atoi(toe_cfg.remote_port.c_str());

        MY_LOG_INFO("call toe_connect: local - %s:%s; remote - %s:%s",
            toe_cfg.local_ip.c_str(), toe_cfg.local_port.c_str(),
            toe_cfg.remote_ip.c_str(), toe_cfg.remote_port.c_str());
        session_init_struct * session = toe_connect(cpu_id, toe_id, pro_type, local_port, remote_port, local_ip, remote_ip, -1);
        MY_LOG_INFO("toe_connect return, session_id:%d", (session ? session->out_sid : 0));
        if (NULL == session)
        {
            MY_LOG_ERROR("toe_connect failed, return NULL");
            QuoteUpdateState(qtm_name_, QtmState::CONNECT_FAIL);
            continue;
        }
        QuoteUpdateState(qtm_name_, QtmState::CONNECT_SUCCESS);

        sessions_.push_back(session);
        set_init_parameters(toe_id, session->out_sid, toe_cfg.multicast_ip);

        pthread_t thread_t = -1;
        struct RecvThreadParam *param = new RecvThreadParam(ev_, quote_data_handler_, subscribe_contracts_, running_flag_);
        param->s = session;
        param->save = p_save_;
        param->cpu_id = recv_cpu;
        PTHREAD_EVENT_INIT(ev_);
        pthread_create(&thread_t, &thread_attr_, &MYFPGACffexDataHandler::toe_recv_data, param);
        PTHREAD_EVENT_WAIT(ev_);
        QuoteUpdateState(qtm_name_, QtmState::API_READY);
    }
}

MYFPGACffexDataHandler::~MYFPGACffexDataHandler(void)
{
    running_flag_ = false;
    for (session_init_struct * s : sessions_)
    {
        toe_close(s);
    }
    sessions_.clear();

    if (p_save_)
    {
        delete p_save_;
    }

#ifdef FPGA_TEST
    // output handle used time
    MY_LOG_INFO("recv length: %d", s_recv_length);
    MY_LOG_INFO("s_stats_count: %d", s_stats_count);
    for (int i = 0; i < s_stats_count; ++i)
    {
        MY_LOG_INFO("used(ns):%d", s_stats_result[i]);
        if (i % 8888 == 0) usleep(50 * 1000);
    }
    if (s_stats_count > 0) usleep(200 * 1000);
#endif

    MY_LOG_INFO("exit toe data handler");
}

void MYFPGACffexDataHandler::SetQuoteDataHandler(boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler)
{
    quote_data_handler_ = quote_data_handler;
}

void MYFPGACffexDataHandler::set_init_parameters(uint8_t toe_id, uint8_t session_id, const std::string &mc_ip)
{
    MY_LOG_INFO("set_init_parameters");
    long const_price, const_volume;
    int ret = get_price_valume(&const_price, &const_volume);
    if (ret != 0)
    {
        MY_LOG_ERROR("get_price_valume failed, return:%d", ret);
        return;
    }

    struct quote_config mkt_cfg;

    // set price const
    mkt_cfg.type = CONF_MKT_PRICE1;
    mkt_cfg.data.mkt_price1 = const_price;
    MY_LOG_INFO("mkt_price1:0x%lX", mkt_cfg.data.mkt_price1);
    ret = toe_quote_config(&mkt_cfg);
    if (ret != 0)
    {
        MY_LOG_ERROR("toe_quote_config, set price const failed, return:%d", ret);
        return;
    }

    // set volume const
    mkt_cfg.type = CONF_MKT_VOL1;
    mkt_cfg.data.mkt_vol1 = (u_int32_t) const_volume;
    MY_LOG_INFO("mkt_vol1:0x%X", mkt_cfg.data.mkt_vol1);
    ret = toe_quote_config(&mkt_cfg);
    if (ret != 0)
    {
        MY_LOG_ERROR("toe_quote_config, set volume const failed, return:%d", ret);
        return;
    }

    // set trade day
    std::string cur_day = my_cmn::GetCurrentDateString();
    if (cur_day.size() == 8)
    {
        mkt_cfg.type = CONF_TRADE_DAY;
        mkt_cfg.data.day.high8 = cur_day[0];
        mkt_cfg.data.day.mid32 = (cur_day[1] << 24) + (cur_day[2] << 16) + (cur_day[3] << 8) + cur_day[4];
        mkt_cfg.data.day.low32 = (cur_day[5] << 24) + (cur_day[6] << 16) + (cur_day[7] << 8) + '\0';
        ret = toe_quote_config(&mkt_cfg);
        if (ret != 0)
        {
            MY_LOG_ERROR("toe_quote_config, set trade day failed, return:%d", ret);
            return;
        }
    }
    else
    {
        MY_LOG_ERROR("GetCurrentDateString return wrong format, return:%s", cur_day.c_str());
    }

    // set multicast address
    mkt_cfg.type = CONF_MULTICAST;
    mkt_cfg.data.mcast.toe_id = toe_id;
    mkt_cfg.data.mcast.session_id = session_id;

    int multicast_ip = inet_addr(mc_ip.c_str());
    std::reverse((char *) &multicast_ip, (char *) &multicast_ip + sizeof(multicast_ip));
    mkt_cfg.data.mcast.mcast_ip = multicast_ip;
    MY_LOG_INFO("session_id:0x%X, mcast_ip:%X", mkt_cfg.data.mcast.session_id, mkt_cfg.data.mcast.mcast_ip);
    ret = toe_quote_config(&mkt_cfg);
    if (ret != 0)
    {
        MY_LOG_ERROR("toe_quote_config, set multicast address failed, return:%d", ret);
        return;
    }
}
