#include "quote_dce_udp.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "quote_cmn_utility.h"
#include "qtm.h"

#include "DFITCL2ApiDataType.h"

using namespace std;
using namespace my_cmn;
using namespace DFITC_L2;

enum EDataType
{
    eMDBestAndDeep = 0,
    eArbiBestAndDeep,
    eMDTenEntrust,
    eMDRealTimePrice,
    eMDOrderStatistic,
    eMDMarchPriceQty,
};

static MDBestAndDeep_MY Convert(const MDBestAndDeep &other)
{
    MDBestAndDeep_MY data;

    data.Type = other.Type;                                     // Type;
    data.Length = other.Length;                                 // Length;                         //报文长度
    data.Version = other.Version;                               // Version;                        //版本从1开始
    data.Time = other.Time;                                     // Time;                           //预留字段
    memcpy(data.Exchange, other.Exchange, 3);                   // Exchange[3];                    //交易所
    memcpy(data.Contract, other.Contract, 80);                  // Contract[80];                   //合约代码
    data.SuspensionSign = other.SuspensionSign;                 // SuspensionSign;                 //停牌标志
    data.LastClearPrice = InvalidToZeroF(other.LastClearPrice); // LastClearPrice;                 //昨结算价
    data.ClearPrice = InvalidToZeroF(other.ClearPrice);         // ClearPrice;                     //今结算价
    data.AvgPrice = InvalidToZeroF(other.AvgPrice);             // AvgPrice;                       //成交均价
    data.LastClose = InvalidToZeroF(other.LastClose);           // LastClose;                      //昨收盘
    data.Close = InvalidToZeroF(other.Close);                   // Close;                          //今收盘
    data.OpenPrice = InvalidToZeroF(other.OpenPrice);           // OpenPrice;                      //今开盘
    data.LastOpenInterest = other.LastOpenInterest;             // LastOpenInterest;               //昨持仓量
    data.OpenInterest = other.OpenInterest;                     // OpenInterest;                   //持仓量
    data.LastPrice = InvalidToZeroF(other.LastPrice);           // LastPrice;                      //最新价
    data.MatchTotQty = other.MatchTotQty;                       // MatchTotQty;                    //成交数量
    data.Turnover = InvalidToZeroD(other.Turnover);             // Turnover;                       //成交金额
    data.RiseLimit = InvalidToZeroF(other.RiseLimit);           // RiseLimit;                      //最高报价
    data.FallLimit = InvalidToZeroF(other.FallLimit);           // FallLimit;                      //最低报价
    data.HighPrice = InvalidToZeroF(other.HighPrice);           // HighPrice;                      //最高价
    data.LowPrice = InvalidToZeroF(other.LowPrice);             // LowPrice;                       //最低价
    data.PreDelta = InvalidToZeroF(other.PreDelta);             // PreDelta;                       //昨虚实度
    data.CurrDelta = InvalidToZeroF(other.CurrDelta);           // CurrDelta;                      //今虚实度

    data.BuyPriceOne = InvalidToZeroF(other.BuyPriceOne);       // BuyPriceOne;                    //买入价格1
    data.BuyQtyOne = other.BuyQtyOne;                           // BuyQtyOne;                      //买入数量1
    data.BuyImplyQtyOne = other.BuyImplyQtyOne;
    data.BuyPriceTwo = InvalidToZeroF(other.BuyPriceTwo);       // BuyPriceTwo;
    data.BuyQtyTwo = other.BuyQtyTwo;                           // BuyQtyTwo;
    data.BuyImplyQtyTwo = other.BuyImplyQtyTwo;
    data.BuyPriceThree = InvalidToZeroF(other.BuyPriceThree);   // BuyPriceThree;
    data.BuyQtyThree = other.BuyQtyThree;                       // BuyQtyThree;
    data.BuyImplyQtyThree = other.BuyImplyQtyThree;
    data.BuyPriceFour = InvalidToZeroF(other.BuyPriceFour);     // BuyPriceFour;
    data.BuyQtyFour = other.BuyQtyFour;                         // BuyQtyFour;
    data.BuyImplyQtyFour = other.BuyImplyQtyFour;
    data.BuyPriceFive = InvalidToZeroF(other.BuyPriceFive);     // BuyPriceFive;
    data.BuyQtyFive = other.BuyQtyFive;                         // BuyQtyFive;
    data.BuyImplyQtyFive = other.BuyImplyQtyFive;
    data.SellPriceOne = InvalidToZeroF(other.SellPriceOne);     // SellPriceOne;                   //卖出价格1
    data.SellQtyOne = other.SellQtyOne;                         // SellQtyOne;                     //买出数量1
    data.SellImplyQtyOne = other.SellImplyQtyOne;
    data.SellPriceTwo = InvalidToZeroF(other.SellPriceTwo);     // SellPriceTwo;
    data.SellQtyTwo = other.SellQtyTwo;                         // SellQtyTwo;
    data.SellImplyQtyTwo = other.SellImplyQtyTwo;
    data.SellPriceThree = InvalidToZeroF(other.SellPriceThree); // SellPriceThree;
    data.SellQtyThree = other.SellQtyThree;                     // SellQtyThree;
    data.SellImplyQtyThree = other.SellImplyQtyThree;
    data.SellPriceFour = InvalidToZeroF(other.SellPriceFour);   // SellPriceFour;
    data.SellQtyFour = other.SellQtyFour;                       // SellQtyFour;
    data.SellImplyQtyFour = other.SellImplyQtyFour;
    data.SellPriceFive = InvalidToZeroF(other.SellPriceFive);   // SellPriceFive;
    data.SellQtyFive = other.SellQtyFive;                       // SellQtyFive;
    data.SellImplyQtyFive = other.SellImplyQtyFive;

    memcpy(data.GenTime, other.GenTime, 13);                    // GenTime[13];                    //行情产生时间
    data.LastMatchQty = other.LastMatchQty;                     // LastMatchQty;                   //最新成交量
    data.InterestChg = other.InterestChg;                       // InterestChg;                    //持仓量变化
    data.LifeLow = InvalidToZeroF(other.LifeLow);               // LifeLow;                        //历史最低价
    data.LifeHigh = InvalidToZeroF(other.LifeHigh);             // LifeHigh;                       //历史最高价
    data.Delta = InvalidToZeroD(other.Delta);                   // Delta;                          //delta
    data.Gamma = InvalidToZeroD(other.Gamma);                   // Gamma;                          //gama
    data.Rho = InvalidToZeroD(other.Rho);                       // Rho;                            //rho
    data.Theta = InvalidToZeroD(other.Theta);                   // Theta;                          //theta
    data.Vega = InvalidToZeroD(other.Vega);                     // Vega;                           //vega
    memcpy(data.TradeDate, other.TradeDate, 9);                 // TradeDate[9];                   //行情日期
    memcpy(data.LocalDate, other.LocalDate, 9);                 // LocalDate[9];                   //本地日期

    return data;
}

static MDOrderStatistic_MY Convert(const MDOrderStatistic &other)
{
    MDOrderStatistic_MY data;

    data.Type = other.Type;                                     // Type;                           //行情域标识
    data.Len = other.Len;                                       // Len;
    memcpy(data.ContractID, other.ContractID, 80);              // ContractID[80];                 //合约号
    data.TotalBuyOrderNum = other.TotalBuyOrderNum;             // TotalBuyOrderNum;               //买委托总量
    data.TotalSellOrderNum = other.TotalSellOrderNum;           // TotalSellOrderNum;              //卖委托总量
    data.WeightedAverageBuyOrderPrice = InvalidToZeroD(other.WeightedAverageBuyOrderPrice);   // WeightedAverageBuyOrderPrice;   //加权平均委买价格
    data.WeightedAverageSellOrderPrice = InvalidToZeroD(other.WeightedAverageSellOrderPrice); // WeightedAverageSellOrderPrice;  //加权平均委卖价格

    return data;
}

DceUdpMD::DceUdpMD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_save_best_and_deep_(NULL), p_save_order_statistic_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    if (subscribe_contracts_.empty())
    {
        MY_LOG_INFO("DCE_UDP - subscribe all contract");
    }
    else
    {
        for (const std::string &v : subscribe_contracts_)
        {
            MY_LOG_INFO("DCE_UDP - subscribe: %s", v.c_str());
        }
    }

    running_flag_ = true;

    sprintf(qtm_name_, "dce_udp_%s_%u", cfg_.Logon_config().account.c_str(), getpid());

    p_save_best_and_deep_ = new QuoteDataSave<MDBestAndDeep_MY>(cfg_, qtm_name_, "bestanddeepquote", DCE_MDBESTANDDEEP_QUOTE_TYPE);
    p_save_order_statistic_ = new QuoteDataSave<MDOrderStatistic_MY>(cfg_, qtm_name_, "orderstatistic", DCE_MDORDERSTATISTIC_QUOTE_TYPE, false);

    // start recv threads
    p_md_handler_ = new boost::thread(std::bind(&DceUdpMD::UdpDataHandler, this));
}

DceUdpMD::~DceUdpMD()
{
    // terminate all threads
    running_flag_ = false;
    if (p_md_handler_)
    {
        p_md_handler_->interrupt();
    }

    // destroy all save object
    if (p_save_best_and_deep_)
        delete p_save_best_and_deep_;

    if (p_save_order_statistic_)
        delete p_save_order_statistic_;
}

void DceUdpMD::UdpDataHandler()
{
    if (cfg_.Logon_config().quote_provider_addrs.size() != 1)
    {
        MY_LOG_ERROR("DCE_UDP - quote front address is wrong, please check");
        return;
    }

    IPAndPortNum ip_and_port = ParseIPAndPortNum(cfg_.Logon_config().quote_provider_addrs.front());

    int udp_fd = CreateUdpFD(ip_and_port.first, ip_and_port.second);
    if (udp_fd < 0)
    {
        MY_LOG_ERROR("DCE_UDP - CreateUdpFD failed.");
        return;
    }

    MY_LOG_INFO("DCE_UDP - sizeof(MDBestAndDeep):%u", sizeof(MDBestAndDeep));
    MY_LOG_INFO("DCE_UDP - sizeof(MDOrderStatistic):%u", sizeof(MDOrderStatistic));

    char buf[2048];
    int data_len = 0;
    sockaddr_in src_addr;
    unsigned int addr_len = sizeof(sockaddr_in);

    while (running_flag_)
    {
        data_len = recvfrom(udp_fd, buf, 2048, 0, (sockaddr *) &src_addr, &addr_len);

        if (data_len > 0)
        {
            int type = (int) buf[0];

            if (type == EDataType::eMDBestAndDeep)
            {
                if ((data_len - 1) != sizeof(MDBestAndDeep))
                {
                    static bool output_md_bd_flag = true;
                    if (output_md_bd_flag)
                    {
                        output_md_bd_flag = false;
                        MY_LOG_ERROR("DCE_UDP - recv data len:%d,type:%d,", data_len, type);
                    }
                    continue;
                }
                timeval t;
                gettimeofday(&t, NULL);

                MDBestAndDeep * p = (MDBestAndDeep *) (buf + 1);
                MDBestAndDeep_MY data_my = Convert(*p);

                // 发出去
                if (best_and_deep_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->Contract) != subscribe_contracts_.end()))
                {
                    best_and_deep_handler_(&data_my);
                }

                // 存起来
                p_save_best_and_deep_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
            }
            else if (type == EDataType::eMDOrderStatistic)
            {
                if ((data_len - 1) != sizeof(MDOrderStatistic))
                {
                    static bool output_md_orderstat_flag = true;
                    if (output_md_orderstat_flag)
                    {
                        output_md_orderstat_flag = false;
                        MY_LOG_ERROR("DCE_UDP - recv data len:%d,type:%d,", data_len, type);
                    }
                    continue;
                }
                timeval t;
                gettimeofday(&t, NULL);

                MDOrderStatistic * p = (MDOrderStatistic *) (buf + 1);
                MDOrderStatistic_MY data_my = Convert(*p);

                // 发出去
                if (order_statistic_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->ContractID) != subscribe_contracts_.end()))
                {
                    order_statistic_handler_(&data_my);
                }

                // 存起来
                p_save_order_statistic_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
            }
        }
    }
}

int DceUdpMD::CreateUdpFD(const std::string& addr_ip, unsigned short port)
{
    // init udp socket
    int udp_client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    /* set reuse and non block for this socket */
    int son = 1;
    setsockopt(udp_client_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &son, sizeof(son));

    // bind address and port
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; //IPv4协议
    servaddr.sin_addr.s_addr = inet_addr(addr_ip.c_str());
    servaddr.sin_port = htons(port);

    if (bind(udp_client_fd, (sockaddr *) &servaddr, sizeof(servaddr)) != 0)
    {
        MY_LOG_FATAL("UDP - bind failed: %s:%d", addr_ip.c_str(), port);
        return -1;
    }

    // set nonblock flag
//    int socket_ctl_flag = fcntl(udp_client_fd, F_GETFL);
//    if (socket_ctl_flag < 0)
//    {
//        MY_LOG_WARN("UDP - get socket control flag failed.");
//    }
//    if (fcntl(udp_client_fd, F_SETFL, socket_ctl_flag | O_NONBLOCK) < 0)
//    {
//        MY_LOG_WARN("UDP - set socket control flag with nonblock failed.");
//    }

    // set buffer length
    int rcvbufsize = 1 * 1024 * 1024;
    int ret = setsockopt(udp_client_fd, SOL_SOCKET, SO_RCVBUF, (const void *) &rcvbufsize, sizeof(rcvbufsize));
    if (ret != 0)
    {
        MY_LOG_WARN("UDP - set SO_RCVBUF failed.");
    }

    int broadcast_on = 1;
    ret = setsockopt(udp_client_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_on, sizeof(broadcast_on));
    if (ret != 0)
    {
        MY_LOG_WARN("UDP - set SO_BROADCAST failed.");
    }

    return udp_client_fd;
}
