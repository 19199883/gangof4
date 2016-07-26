#ifndef FIELD_CONVERT_H_
#define FIELD_CONVERT_H_

#include "trade_data_type.h"
#include "my_trade_tunnel_data_type.h"
#include "ThostFtdcTraderApi.h"
#include <string>
#include <iomanip>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>


class CTPFieldConvert
{
public:
    // 转换成CTP字段取值
    inline static char GetCTPOCFlag(const char exch_code, const char open_close);
    inline static char GetCTPHedgeType(const char hedge_flag);
    inline static char GetCTPPriceType(const char price_type);
    inline static char GetCTPTimeCondition(const char price_type);
    inline static const char * ExchCodeToExchName(const char cExchCode);
    inline static const char * GetCurrencyID();
    inline static void GetMacAndIPAddress(const char *ethname, char *mac, char *ip);

    // 从CTP转换会MY协议取值
    inline static char EntrustStatusTrans(char OrderSubmitStatus, char OrderStatus);
    inline static char EntrustTbFlagTrans(char HedgeFlag);
    inline static char PriceTypeTrans(char PriceType);
    inline static char ExchNameToExchCode(const char * ex_name);

    // 格式化
    inline static void SysOrderIDToCTPFormat(EntrustNoType entrust_no, char * const &ctp_orderid);

};

char CTPFieldConvert::GetCTPOCFlag(const char exch_code, const char open_close)
{
    if ((exch_code == MY_TNL_EC_SHFE) &&
        ((open_close == MY_TNL_D_CLOSE) ||
            (open_close == MY_TNL_D_CLOSETODAY)))
    {
        return THOST_FTDC_OF_CloseToday;
    }
    else if ((exch_code == MY_TNL_EC_SHFE)
        && (open_close == MY_TNL_D_CLOSEYESTERDAY))
    {
        return THOST_FTDC_OF_CloseYesterday;
    }

    return open_close;
}

char CTPFieldConvert::GetCTPHedgeType(const char hedge_flag)
{
    if (hedge_flag == MY_TNL_HF_SPECULATION)
    {
        return THOST_FTDC_HF_Speculation;
    }
    else if (hedge_flag == MY_TNL_HF_ARBITRAGE)
    {
        return THOST_FTDC_HF_Arbitrage;
    }
    else if (hedge_flag == MY_TNL_HF_HEDGE)
    {
        return THOST_FTDC_HF_Hedge;
    }

    return THOST_FTDC_HF_Speculation;
}

char CTPFieldConvert::GetCTPPriceType(const char price_type)
{
    if (price_type == MY_TNL_OPT_ANY_PRICE)
    {
        return THOST_FTDC_OPT_AnyPrice;
    }
    return THOST_FTDC_OPT_LimitPrice;
}

char CTPFieldConvert::GetCTPTimeCondition(const char order_type)
{
    if (order_type == MY_TNL_HF_FAK || order_type == MY_TNL_HF_FOK)
    {
        return THOST_FTDC_TC_IOC;
    }
    return THOST_FTDC_TC_GFD;
}

const char * CTPFieldConvert::ExchCodeToExchName(const char cExchCode)
{
    if (cExchCode == MY_TNL_EC_SHFE)
    {
        return "SHFE";
    }
    else if (cExchCode == MY_TNL_EC_DCE)
    {
        return "DCE";
    }
    else if (cExchCode == MY_TNL_EC_CZCE)
    {
        return "CZCE";
    }
    else if (cExchCode == MY_TNL_EC_CFFEX)
    {
        return "CFFEX";
    }
    else
    {
        return "";
    }
}

char CTPFieldConvert::ExchNameToExchCode(const char * ex_name)
{
    if (strcmp(ex_name, "CFFEX") == 0)
        return MY_TNL_EC_CFFEX;

    if (strcmp(ex_name, "SHFE") == 0)
        return MY_TNL_EC_SHFE;

    if (strcmp(ex_name, "DCE") == 0)
        return MY_TNL_EC_DCE;

    if (strcmp(ex_name, "CZCE") == 0)
        return MY_TNL_EC_CZCE;

    return ' ';
}

//将CTP的委托状态转换为统一的委托状态
char CTPFieldConvert::EntrustStatusTrans(char OrderSubmitStatus, char OrderStatus)
{
    if (OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted
        || OrderSubmitStatus == THOST_FTDC_OSS_CancelSubmitted
        || OrderSubmitStatus == THOST_FTDC_OSS_ModifySubmitted
        || OrderSubmitStatus == THOST_FTDC_OSS_Accepted)
    {
        switch (OrderStatus)
        {
            //全部成交 --> 全部成交
            case THOST_FTDC_OST_AllTraded:
                return MY_TNL_OS_COMPLETED;

                //部分成交还在队列中 -->  全部成交
            case THOST_FTDC_OST_PartTradedQueueing:
                return MY_TNL_OS_PARTIALCOM;

                //部分成交不在队列中 -->  部成部撤
            case THOST_FTDC_OST_PartTradedNotQueueing:
                return MY_TNL_OS_WITHDRAWED;

                //未成交还在队列中  -->  已经报入
            case THOST_FTDC_OST_NoTradeQueueing:
                return MY_TNL_OS_REPORDED;

                //未成交不在队列中  -->  已经报入
            case THOST_FTDC_OST_NoTradeNotQueueing:
                return MY_TNL_OS_REPORDED;

                //撤单 -->  已经撤销
            case THOST_FTDC_OST_Canceled:
                return MY_TNL_OS_WITHDRAWED;

            default:
                return MY_TNL_OS_REPORDED;
        }
    }
    else if (OrderSubmitStatus == THOST_FTDC_OSS_InsertRejected)
    {
        switch (OrderStatus)
        {
            case THOST_FTDC_OST_Canceled:
                return MY_TNL_OS_WITHDRAWED;
            default:
                //错误委托
                return MY_TNL_OS_ERROR;
        }
    }

    //错误委托
    return MY_TNL_OS_ERROR;
}

//将CTP的投机、套保标志转换为统一的委托状态
char CTPFieldConvert::EntrustTbFlagTrans(char HedgeFlag)
{
    switch (HedgeFlag)
    {
        //套保
        case THOST_FTDC_ECIDT_Hedge:
            return MY_TNL_HF_HEDGE;
            //投机
        case THOST_FTDC_ECIDT_Speculation:
            return MY_TNL_HF_SPECULATION;
            //套利 20130125 caodl 增加
        case THOST_FTDC_ECIDT_Arbitrage:
            return MY_TNL_HF_ARBITRAGE;
        default:
            return MY_TNL_HF_SPECULATION;
    }
}

char CTPFieldConvert::PriceTypeTrans(char PriceType)
{
    switch (PriceType)
    {
        case THOST_FTDC_OPT_AnyPrice: ///任意价
            return MY_TNL_OPT_ANY_PRICE;
        case THOST_FTDC_OPT_LimitPrice: ///限价
            return MY_TNL_OPT_LIMIT_PRICE;
        case THOST_FTDC_OPT_BestPrice: ///最优价
            return MY_TNL_OPT_BEST_PRICE;
        default:
            return 'n';
    }
}

// ctp对接中金所/上期/大连，不足12位时，前面补空格，达到12位
void CTPFieldConvert::SysOrderIDToCTPFormat(EntrustNoType entrust_no, char * const &ctp_orderid)
{
    char my_orderid[64];
    sprintf(my_orderid, "%ld", entrust_no);
    std::size_t sysid_len = strlen(my_orderid);
    std::size_t start_pos = 0;
    if (sysid_len < 12)
    {
        start_pos = 12 - sysid_len;
        memset(ctp_orderid, ' ', start_pos);
    }

    memcpy(ctp_orderid + start_pos, my_orderid, sysid_len + 1);
}

const char * CTPFieldConvert::GetCurrencyID() {
	return "RMB";
}

void CTPFieldConvert::GetMacAndIPAddress(const char *ethname, char *mac, char *ip) {
	struct ifreq ifreq;
	int sock;
	if((sock=socket(AF_INET,SOCK_STREAM,0)) <0)  {
		perror( "socket ");
		return;
	}
	strcpy(ifreq.ifr_name, ethname);
	if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0) {
		perror( "ioctl ");
	} else {
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
				(unsigned   char)ifreq.ifr_hwaddr.sa_data[0],
				(unsigned   char)ifreq.ifr_hwaddr.sa_data[1],
				(unsigned   char)ifreq.ifr_hwaddr.sa_data[2],
				(unsigned   char)ifreq.ifr_hwaddr.sa_data[3],
				(unsigned   char)ifreq.ifr_hwaddr.sa_data[4],
				(unsigned   char)ifreq.ifr_hwaddr.sa_data[5]);
	}
	if (ioctl(sock, SIOCGIFADDR, &ifreq) < 0) {
		perror( "ioctl ");
	} else {
		sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in*)&(ifreq.ifr_addr))->sin_addr));
	}
}
#endif // FIELD_CONVERT_H_
