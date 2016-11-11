﻿#include "quote_czce_udp.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "quote_cmn_utility.h"
#include "qtm.h"

#include "ZceLevel2ApiStruct.h"

using namespace std;
using namespace my_cmn;

static ZCEL2QuotSnapshotField_MY  Convert(const StdQuote5 &other)
{
    ZCEL2QuotSnapshotField_MY data;

	memcpy(data.ContractID, other.instrument, 32);		/*合约编码*/
	data.ContractIDType = 0;							/*合约类型 0->目前应该为0， 扩充：0:期货,1:期权,2:组合*/
	data.PreSettle = 0;									/*前结算价格*/
	data.PreClose = 0;									/*前收盘价格*/
	data.PreOpenInterest = 0;							/*昨日空盘量*/
	data.OpenPrice = 0;									/*开盘价*/
	data.HighPrice = 0;									/*最高价*/
	data.LowPrice = 0;									/*最低价*/
	data.LastPrice = InvalidToZeroD(other.price);		/*最新价*/

	data.BidPrice[0] = InvalidToZeroD(other.bidPrice1);     /*买入价格 下标从0开始*/
	data.BidPrice[1] = InvalidToZeroD(other.bidPrice2);     /*买入价格 下标从0开始*/
	data.BidPrice[2] = InvalidToZeroD(other.bidPrice3);     /*买入价格 下标从0开始*/
	data.BidPrice[3] = InvalidToZeroD(other.bidPrice4);     /*买入价格 下标从0开始*/	
	data.BidPrice[4] = InvalidToZeroD(other.bidPrice5);     /*买入价格 下标从0开始*/

	data.AskPrice[0] = InvalidToZeroD(other.askPrice1);     /*卖出价 下标从0开始*/
	data.AskPrice[1] = InvalidToZeroD(other.askPrice2);     /*卖出价 下标从0开始*/
	data.AskPrice[2] = InvalidToZeroD(other.askPrice3);     /*卖出价 下标从0开始*/
	data.AskPrice[3] = InvalidToZeroD(other.askPrice4);     /*卖出价 下标从0开始*/
	data.AskPrice[4] = InvalidToZeroD(other.askPrice5);     /*卖出价 下标从0开始*/
	data.BidLot[0] = other.bidVolume1;          /*买入数量 下标从0开始*/
	data.BidLot[1] = other.bidVolume2;          /*买入数量 下标从0开始*/
	data.BidLot[2] = other.bidVolume3;          /*买入数量 下标从0开始*/
	data.BidLot[3] = other.bidVolume4;          /*买入数量 下标从0开始*/
	data.BidLot[4] = other.bidVolume5;          /*买入数量 下标从0开始*/

	data.AskLot[0] = other.askVolume1;          /*卖出数量 下标从0开始*/
	data.AskLot[1] = other.askVolume2;          /*卖出数量 下标从0开始*/
	data.AskLot[2] = other.askVolume3;          /*卖出数量 下标从0开始*/
	data.AskLot[3] = other.askVolume4;          /*卖出数量 下标从0开始*/
	data.AskLot[4] = other.askVolume5;          /*卖出数量 下标从0开始*/

	data.TotalVolume = 0			;			/*总成交量*/
	data.OpenInterest = other.openinterest;       /*持仓量*/
	data.ClosePrice = 0;						/*收盘价*/
	data.SettlePrice = 0;						/*结算价*/
	data.AveragePrice = 0;    /*均价*/
	data.LifeHigh = 0;        /*历史最高成交价格*/
	data.LifeLow = 0;         /*历史最低成交价格*/
	data.HighLimit = 0;       /*涨停板*/
	data.LowLimit = 0;        /*跌停板*/
	data.TotalBidLot = 0;        /*委买总量*/
	data.TotalAskLot = 0;        /*委卖总量*/
	
	// unkonwn value
//	data.TimeStamp[24];     //时间：如2014-02-03 13:23:45

    return data;
}

CzceUdpMD::CzceUdpMD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_save_zcel2_quote_snap_snapshot_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    if (subscribe_contracts_.empty())
    {
        MY_LOG_INFO("CZCE_UDP - subscribe all contract");
    }
    else
    {
        for (const std::string &v : subscribe_contracts_)
        {
            MY_LOG_INFO("CZCE_UDP - subscribe: %s", v.c_str());
        }
    }

    running_flag_ = true;

    sprintf(qtm_name_, "czce_udp_%s_%u", cfg_.Logon_config().account.c_str(), getpid());

    p_save_zcel2_quote_snap_snapshot_= new QuoteDataSave<ZCEL2QuotSnapshotField_MY>(cfg_, qtm_name_, "czce_level2", CZCE_LEVEL2_QUOTE_TYPE);

    // start recv threads
    p_md_handler_ = new boost::thread(boost::bind(&CzceUdpMD::UdpDataHandler, this));
}

CzceUdpMD::~CzceUdpMD()
{
    // terminate all threads
    running_flag_ = false;
    if (p_md_handler_)
    {
        p_md_handler_->interrupt();
    }

    // destroy all save object
    if (p_save_zcel2_quote_snap_snapshot_)
        delete p_save_zcel2_quote_snap_snapshot_;
}

void CzceUdpMD::UdpDataHandler()
{
    if (cfg_.Logon_config().quote_provider_addrs.size() != 1)
    {
        MY_LOG_ERROR("CZCE_UDP - quote front address is wrong, please check");
        return;
    }

    IPAndPortNum ip_and_port = ParseIPAndPortNum(cfg_.Logon_config().quote_provider_addrs.front());

    int udp_fd = CreateUdpFD(ip_and_port.first, ip_and_port.second);
    if (udp_fd < 0)
    {
        MY_LOG_ERROR("CZCE_UDP - CreateUdpFD failed.");
        return;
    }

    MY_LOG_INFO("CZCE_UDP - sizeof(StdQuote5):%u", sizeof(StdQuote5));

    char buf[2048];
    int data_len = 0;
    sockaddr_in src_addr;
    unsigned int addr_len = sizeof(sockaddr_in);

    while (running_flag_)
    {
        data_len = recvfrom(udp_fd, buf, 2048, 0, (sockaddr *) &src_addr, &addr_len);

        if (data_len > 0)
        {
			if (data_len != sizeof(StdQuote5))
			{
				static bool output_md_bd_flag = true;
				if (output_md_bd_flag)
				{
					output_md_bd_flag = false;
					MY_LOG_ERROR("CZCE_UDP - recv data len:%d", data_len);
				}

				// debug,wangying on 20161105
				//		continue;
			}
			timeval t;
			gettimeofday(&t, NULL);

			StdQuote5 * p = (StdQuote5 *) (buf);
			
			// debug, wangying on 20161105
			MY_LOG_ERROR("CZCE_UDP - recv data len:%d", data_len);
			MY_LOG_ERROR("CZCE_UDP - recv data:%s,%d,%d,%d,%f", p->instrument,p->bidVolume5,p->askVolume3,
			p->bidVolume1,p->askPrice2);
			
		    ZCEL2QuotSnapshotField_MY data_my = Convert(*p);

			// 发出去
			if (l2_quote_handler_
				&& (subscribe_contracts_.empty() || subscribe_contracts_.find(p->instrument) != subscribe_contracts_.end()))
			{
				l2_quote_handler_(&data_my);
			}

			// 存起来
			p_save_zcel2_quote_snap_snapshot_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
        }
    }
}

int CzceUdpMD::CreateUdpFD(const std::string& addr_ip, unsigned short port)
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
