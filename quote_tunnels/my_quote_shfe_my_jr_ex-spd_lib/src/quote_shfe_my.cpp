#include "quote_shfe_my.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "quote_cmn_utility.h"
#include "qtm.h"

using namespace std;
using namespace my_cmn;

#pragma pack(1)
#define MAX_PAIR 120
struct PVPair
{
    double price;
    int volume;
};
struct MDPack
{
    char instrument[10];
    char islast;
    int seqno;
    char direction;
    short count;
    PVPair data[MAX_PAIR];
};
#pragma pack()

static std::string ToString(const MDPack &d) {
    char msg[10240];
    int rec = sprintf(msg, "MDPack Data: \ninstrument: %s\nislast: %d\nseqno: %d\ndirection: %c\ncount: %d\n", d.instrument, (int)d.islast, d.seqno, d.direction, d.count);
    int tmp = 0;
    for(int i = 0; i < d.count; i++) {
        rec += tmp;
        tmp = sprintf(msg + rec, "price%d: %lf, volume%d: %d\n", i, d.data[i].price, i, d.data[i].volume);
    }
    return msg;
}

QuoteInterface_MY_SHFE_MD::QuoteInterface_MY_SHFE_MD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), seq_no_(0), server_(0), p_shfe_deep_save_(NULL), p_shfe_ex_save_(NULL), p_my_shfe_md_save_(NULL),
        p_mbl_handler_(NULL), my_shfe_md_inf_(cfg)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    if (subscribe_contracts_.empty())
    {
        MY_LOG_INFO("MY_SHFE_MD - subscribe all contract");
    }
    else
    {
        for (const std::string &v : subscribe_contracts_)
        {
            MY_LOG_INFO("MY_SHFE_MD - subscribe: %s", v.c_str());
        }
    }

    running_flag_ = true;

    sprintf(qtm_name_, "shfe_jr_%s_%u", cfg_.Logon_config().account.c_str(), getpid());
    QuoteUpdateState(qtm_name_, QtmState::INIT);

    p_shfe_deep_save_ = new QuoteDataSave<SHFEQuote>(cfg_, qtm_name_, "shfe_deep", SHFE_DEEP_QUOTE_TYPE, false);
    p_shfe_ex_save_ = new QuoteDataSave<CDepthMarketDataField>(cfg_, qtm_name_, "quote_level1", SHFE_EX_QUOTE_TYPE, false);
    p_my_shfe_md_save_ = new QuoteDataSave<MYShfeMarketData>(cfg_, qtm_name_, "my_shfe_md", MY_SHFE_MD_QUOTE_TYPE);

    my_shfe_md_inf_.SetDataHandler(this);

    // start recv threads
    p_mbl_handler_ = new boost::thread(boost::bind(&QuoteInterface_MY_SHFE_MD::ShfeMBLHandler, this));

    femas_inf_ = new CMdclientHandler(NULL, cfg_);
    femas_inf_->SetQuoteDataHandler(boost::bind(&QuoteInterface_MY_SHFE_MD::ShfeDepthMarketDataHandler, this, _1));
}

QuoteInterface_MY_SHFE_MD::~QuoteInterface_MY_SHFE_MD()
{
    // terminate all threads
    running_flag_ = false;
    if (p_mbl_handler_)
    {
        p_mbl_handler_->interrupt();
    }

    if (femas_inf_)
    {
        delete femas_inf_;
    }

    // destroy all save object
    if (p_shfe_deep_save_)
        delete p_shfe_deep_save_;

    if (p_shfe_ex_save_)
        delete p_shfe_ex_save_;

    if (p_my_shfe_md_save_)
        delete p_my_shfe_md_save_;
}

void QuoteInterface_MY_SHFE_MD::ShfeMBLHandler()
{
    if (cfg_.Logon_config().mbl_data_addrs.size() != 1)
    {
        MY_LOG_ERROR("MY_SHFE_MD - address of mbl is wrong, please check");
        return;
    }

    IPAndPortNum ip_and_port = ParseIPAndPortNum(cfg_.Logon_config().mbl_data_addrs.front());

    int udp_fd = CreateUdpFD(ip_and_port.first, ip_and_port.second);
    if (udp_fd < 0)
    {
        QuoteUpdateState(qtm_name_, QtmState::CONNECT_FAIL);
        MY_LOG_ERROR("MY_SHFE_MD - CreateUdpFD failed.");
        return;
    }

    QuoteUpdateState(qtm_name_, QtmState::CONNECT_SUCCESS);

    int recv_struct_len = sizeof(MDPack);
    int ary_len = recv_struct_len;
    char *recv_buf = new char[ary_len];
    std::size_t recv_len = 0;
    QuoteUpdateState(qtm_name_, QtmState::API_READY);

    while (running_flag_)
    {
        sockaddr_in fromAddr;
        int nFromLen = sizeof(fromAddr);
        recv_len = recvfrom(udp_fd, recv_buf, ary_len, 0, (sockaddr *)&fromAddr,
            (socklen_t *)&nFromLen);

        if (recv_len == -1)
        {
            int error_no = errno;
            if (error_no == 0 || error_no == 251 || error_no == EWOULDBLOCK) /*251 for PARisk */ //20060224 IA64 add 0
            {
                continue;
            }
            else
            {
                MY_LOG_ERROR("UDP - recvfrom failed, error_no=%d.", error_no);
                continue;
            }
        }
        //MY_LOG_DEBUG("recv_len: %d", recv_len);

        // data handle
        MDPack *p = (MDPack *)recv_buf;
        //MY_LOG_DEBUG("%s", ToString(*p).c_str());

        if (p->seqno / 10 != seq_no_ + 1)
        {
            //update_state(qtm_name_, TYPE_QUOTE, QtmState::UNDEFINED_API_ERROR, "packet loss");
            MY_LOG_WARN("seq no from %d to %d, may have packet loss", seq_no_, p->seqno / 10);
        }

        if (p->seqno % 10 != server_)
        {
            MY_LOG_WARN("server frome %d to %d", server_, p->seqno % 10);
        }

        timeval t;
        gettimeofday(&t, NULL);

        SHFEQuote item;
        memset(&item, 0, sizeof(item));
        strcpy(item.field.InstrumentID, p->instrument);
        item.field.Direction = p->direction;
        for (int i = 0; i < p->count; i++)
        {
            item.field.Price = p->data[i].price;
            item.field.Volume = p->data[i].volume;
            if (p->islast == true && i == p->count - 1)
            {
                item.isLast = true;
            }
            else
            {
                item.isLast = false;
            }

            if (shfe_deep_data_handler_
                && (subscribe_contracts_.empty()
                    || subscribe_contracts_.find(p->instrument) != subscribe_contracts_.end()))
            {
                shfe_deep_data_handler_(&item);
            }
            my_shfe_md_inf_.OnMBLData(&item.field, item.isLast);
            p_shfe_deep_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &item);
        }

        seq_no_ = p->seqno / 10;
        server_ = p->seqno % 10;
    }
}

void QuoteInterface_MY_SHFE_MD::ShfeDepthMarketDataHandler(const CDepthMarketDataField * data)
{
    timeval t;
    gettimeofday(&t, NULL);
    if (shfe_ex_handler_
        && (subscribe_contracts_.empty()
            || subscribe_contracts_.find(data->InstrumentID) != subscribe_contracts_.end()))
    {
        shfe_ex_handler_(data);
    }

    // send to data manager object
    my_shfe_md_inf_.OnDepthMarketData(data);

    // save
    p_shfe_ex_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, data);
}

void QuoteInterface_MY_SHFE_MD::OnMYShfeMDData(MYShfeMarketData *data)
{
    timeval t;
    gettimeofday(&t, NULL);
    if (my_shfe_md_handler_
        && (subscribe_contracts_.empty()
            || subscribe_contracts_.find(data->InstrumentID) != subscribe_contracts_.end()))
    {
        my_shfe_md_handler_(data);
    }
    p_my_shfe_md_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, data);
}

int QuoteInterface_MY_SHFE_MD::CreateUdpFD(const std::string& addr_ip, unsigned short port)
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
        QuoteUpdateState(qtm_name_, QtmState::CONNECT_FAIL);
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
        MY_LOG_WARN("UDP - set SO_RCVBUF failed.");
    }

    return udp_client_fd;
}
