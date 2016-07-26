#include "quote_shfe_my.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "quote_cmn_utility.h"
#include "zeusing_udp_datatype.h"
#include "qtm.h"

using namespace std;
using namespace my_cmn;

QuoteInterface_MY_SHFE_MD::QuoteInterface_MY_SHFE_MD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_shfe_deep_save_(NULL), p_shfe_ex_save_(NULL), p_my_shfe_md_save_(NULL),
        p_mbl_handler_(NULL), p_shfe_depth_md_handler_(NULL), p_data_handler_(NULL), my_shfe_md_inf_(cfg)
{
    udp_bind_cnt_ = 0;

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

    sprintf(qtm_name_, "my_shfe_md_zz_%u", getpid());
    QuoteUpdateState(qtm_name_, QtmState::INIT);

    p_shfe_deep_save_ = new QuoteDataSave<SHFEQuote>(cfg_, qtm_name_, "shfe_deep", SHFE_DEEP_QUOTE_TYPE);
    p_shfe_ex_save_ = new QuoteDataSave<CDepthMarketDataField>(cfg_, qtm_name_, "quote_level1", SHFE_EX_QUOTE_TYPE);
    p_my_shfe_md_save_ = new QuoteDataSave<MYShfeMarketData>(cfg_, qtm_name_, "my_shfe_md", MY_SHFE_MD_QUOTE_TYPE);

    my_shfe_md_inf_.SetDataHandler(this);

    //ShfeDataHandler();
    // start recv threads
    //p_mbl_handler_ = new boost::thread(boost::bind(&QuoteInterface_MY_SHFE_MD::ShfeMBLHandler, this));
    //p_shfe_depth_md_handler_ = new boost::thread(boost::bind(&QuoteInterface_MY_SHFE_MD::ShfeDepthMarketDataHandler, this));
    p_data_handler_ = new boost::thread(boost::bind(&QuoteInterface_MY_SHFE_MD::ShfeDataHandler, this));
}

QuoteInterface_MY_SHFE_MD::~QuoteInterface_MY_SHFE_MD()
{
    // terminate all threads
    running_flag_ = false;
    if (p_mbl_handler_)
    {
        p_mbl_handler_->interrupt();
    }
    if (p_shfe_depth_md_handler_)
    {
        p_shfe_depth_md_handler_->interrupt();
    }
    if (p_data_handler_)
    {
        p_data_handler_->interrupt();
    }

    // destroy all save object
    if (p_shfe_deep_save_)
        delete p_shfe_deep_save_;

    if (p_shfe_ex_save_)
        delete p_shfe_ex_save_;

    if (p_my_shfe_md_save_)
        delete p_my_shfe_md_save_;

    qtm_finish();
}

void QuoteInterface_MY_SHFE_MD::ShfeMBLHandler()
{
    if (cfg_.Logon_config().mbl_data_addrs.size() != 1)
    {
        QuoteUpdateState(qtm_name_, QtmState::QUOTE_INIT_FAIL);
        MY_LOG_ERROR("MY_SHFE_MD - address of mbl is wrong, please check");
        return;
    }

    IPAndPortNum ip_and_port = ParseIPAndPortNum(cfg_.Logon_config().mbl_data_addrs.front());

    int udp_fd = CreateUdpFD(ip_and_port.first, ip_and_port.second);
    if (udp_fd < 0)
    {
        QuoteUpdateState(qtm_name_, QtmState::CREATE_UDP_FD_FAIL);
        MY_LOG_ERROR("MY_SHFE_MD - MBL CreateUdpFD failed.");
        return;
    }
    QuoteDataSaveDirect *p_save_direct = new QuoteDataSaveDirect("udp_raw_" + boost::lexical_cast<std::string>(ip_and_port.second));

    SHFEQuote data;
    SHFEQuote empty_data;
    memset(&empty_data, 0, sizeof(empty_data));
    //char prev_dir = SHFE_FTDC_D_Buy;
    char buffer[4096];
    ++udp_bind_cnt_;
    if (udp_bind_cnt_ == 2)
    {
        QuoteUpdateState(qtm_name_, QtmState::API_READY);
    }

    while (running_flag_)
    {
        sockaddr_in fromAddr;
        int nFromLen = sizeof(sockaddr_in);
        int nCount = recvfrom(udp_fd, buffer, 4000, 0, (sockaddr *) &fromAddr, (socklen_t *) &nFromLen);
        if (nCount == 0)
            continue;
        if (nCount == -1)
        {
            int error_no = errno;
            if (error_no == 0 || error_no == 251 || error_no == EWOULDBLOCK) /*251 for PARisk */ //20060224 IA64 add 0
            {
                continue;
            }
            else
            {
                MY_LOG_ERROR("ZEUSING_UDP - recvfrom failed, error_no=%d.", error_no);
                continue;
            }
        }

        if (nCount != sizeof(CMBLMarketDataField))
        {
            MY_LOG_ERROR("MYShfeMBLHandler - receive data in wrong length: %d.", nCount);
        }
        else
        {
            CMBLMarketDataField *p = (CMBLMarketDataField *) buffer;
            //MY_LOG_INFO("CMBLMarketDataField: %s %c %.3f %d", p->InstrumentID, p->Direction, p->Price, p->Volume);

            timeval t;
            gettimeofday(&t, NULL);

            // 加一个instrument=last的包
            if (p->InstrumentID[2] == 's'
                && p->InstrumentID[3] == 't'
                //&& p->InstrumentID[0] == 'l'
                && p->InstrumentID[1] == 'a'
                    )
            {
                my_shfe_md_inf_.OnMBLData(&empty_data.field, true);
            }
            else
            {
                memcpy(data.field.InstrumentID, p->InstrumentID, sizeof(data.field.InstrumentID));
                data.field.Direction = p->Direction;
                data.field.Price = p->Price;
                data.field.Volume = p->Volume;
                //data.isLast = false;

                if (shfe_deep_data_handler_
                    && (subscribe_contracts_.empty()
                        || subscribe_contracts_.find(data.field.InstrumentID) != subscribe_contracts_.end()))
                {
                    shfe_deep_data_handler_(&data);
                }

                my_shfe_md_inf_.OnMBLData(&data.field, false);
            }
            p_shfe_deep_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
            p_save_direct->OnQuoteData((long long) t.tv_sec * 1000000 + t.tv_usec, buffer, nCount);
        }
    }
}

void QuoteInterface_MY_SHFE_MD::ShfeDepthMarketDataHandler()
{
    if (cfg_.Logon_config().depth_market_data_addrs.size() != 1)
    {
        QuoteUpdateState(qtm_name_, QtmState::QUOTE_INIT_FAIL);
        MY_LOG_ERROR("MY_SHFE_MD - address of normal data is wrong, please check");
        return;
    }

    IPAndPortNum ip_and_port = ParseIPAndPortNum(cfg_.Logon_config().depth_market_data_addrs.front());

    int udp_fd = CreateUdpFD(ip_and_port.first, ip_and_port.second);
    if (udp_fd < 0)
    {
        QuoteUpdateState(qtm_name_, QtmState::CREATE_UDP_FD_FAIL);
        MY_LOG_ERROR("MY_SHFE_MD - CreateUdpFD failed.");
        return;
    }
    QuoteDataSaveDirect *p_save_direct = new QuoteDataSaveDirect("udp_raw_" + boost::lexical_cast<std::string>(ip_and_port.second));

    SHFEQuote data;
    char buffer[4096];

    ++udp_bind_cnt_;
    if (udp_bind_cnt_ == 2)
    {
        QuoteUpdateState(qtm_name_, QtmState::API_READY);
    }

    while (running_flag_)
    {
        sockaddr_in fromAddr;
        int nFromLen = sizeof(sockaddr_in);
        int nCount = recvfrom(udp_fd, buffer, 4000, 0, (sockaddr *) &fromAddr, (socklen_t *) &nFromLen);
        if (nCount == 0)
            continue;
        if (nCount == -1)
        {
            int error_no = errno;
            if (error_no == 0 || error_no == 251 || error_no == EWOULDBLOCK) /*251 for PARisk */ //20060224 IA64 add 0
            {
                continue;
            }
            else
            {
                MY_LOG_ERROR("ZEUSING_UDP - recvfrom failed, error_no=%d.", error_no);
                continue;
            }
        }

        ParseMarketData(buffer, nCount, p_save_direct);
    }
}

void QuoteInterface_MY_SHFE_MD::ShfeDataHandler()
{
    if (cfg_.Logon_config().depth_market_data_addrs.size() != 1)
    {
        QuoteUpdateState(qtm_name_, QtmState::QUOTE_INIT_FAIL);
        MY_LOG_ERROR("MY_SHFE_MD - address of normal data is wrong, please check");
        return;
    }

    if (cfg_.Logon_config().mbl_data_addrs.size() != 1)
    {
        QuoteUpdateState(qtm_name_, QtmState::QUOTE_INIT_FAIL);
        MY_LOG_ERROR("MY_SHFE_MD - address of mbl is wrong, please check");
        return;
    }

    IPAndPortNum market_data_ip_and_port = ParseIPAndPortNum(cfg_.Logon_config().depth_market_data_addrs.front());
    IPAndPortNum mbl_ip_and_port = ParseIPAndPortNum(cfg_.Logon_config().mbl_data_addrs.front());

    int market_data_udp_fd = CreateUdpFD(market_data_ip_and_port.first, market_data_ip_and_port.second);
    int mbl_data_udp_fd = CreateUdpFD(mbl_ip_and_port.first, mbl_ip_and_port.second);

    if (market_data_udp_fd < 0)
    {
        QuoteUpdateState(qtm_name_, QtmState::CREATE_UDP_FD_FAIL);
        MY_LOG_ERROR("MY_SHFE_MD - CreateUdpFD failed.");
        return;
    }
    if (mbl_data_udp_fd < 0)
    {
        QuoteUpdateState(qtm_name_, QtmState::CREATE_UDP_FD_FAIL);
        MY_LOG_ERROR("MY_SHFE_MD - MBL CreateUdpFD failed.");
        return;
    }

    QuoteDataSaveDirect *p_save_direct_market_data = NULL;//new QuoteDataSaveDirect("udp_raw_" + boost::lexical_cast<std::string>(market_data_ip_and_port.second));
    //QuoteDataSaveDirect *p_save_direct_mbl = new QuoteDataSaveDirect("udp_raw_" + boost::lexical_cast<std::string>(mbl_ip_and_port.second));

    QuoteUpdateState(qtm_name_, QtmState::API_READY);

    SHFEQuote data;
    SHFEQuote empty_data;
    memset(&empty_data, 0, sizeof(empty_data));

    char market_data_buffer[4096];
    char mbl_buffer[4096];

    while (running_flag_)
    {
        sockaddr_in fromAddr;
        int nFromLen = sizeof(sockaddr_in);
        int market_data_count = recvfrom(market_data_udp_fd, market_data_buffer, 4000, 0, (sockaddr *) &fromAddr, (socklen_t *) &nFromLen);
        if (market_data_count == -1)
        {
            int error_no = errno;
            if (error_no != 0 && error_no != 251 && error_no != EWOULDBLOCK) /*251 for PARisk */ //20060224 IA64 add 0
            {
                MY_LOG_ERROR("ZEUSING_UDP - recvfrom failed, error_no=%d.", error_no);
            }
        }
        else if (market_data_count > 0)
        {
            ParseMarketData(market_data_buffer, market_data_count, p_save_direct_market_data);
        }

        int mbl_count = recvfrom(mbl_data_udp_fd, mbl_buffer, 4000, 0, (sockaddr *) &fromAddr, (socklen_t *) &nFromLen);
        if (mbl_count == -1)
        {
            int error_no = errno;
            if (error_no != 0 && error_no != 251 && error_no != EWOULDBLOCK) /*251 for PARisk */ //20060224 IA64 add 0
            {
                MY_LOG_ERROR("ZEUSING_UDP - recvfrom failed, error_no=%d.", error_no);
            }
        }
        else if (mbl_count > 0)
        {
            if (mbl_count != sizeof(CMBLMarketDataField))
            {
                MY_LOG_ERROR("ShfeDataHandler - mbl receive data in wrong length: %d.", mbl_count);
            }
            else
            {
                CMBLMarketDataField *p = (CMBLMarketDataField *) mbl_buffer;
                //MY_LOG_INFO("CMBLMarketDataField: %s %c %.3f %d", p->InstrumentID, p->Direction, p->Price, p->Volume);

                timeval t;
                gettimeofday(&t, NULL);

                // 加一个instrument=last的包
                if (p->InstrumentID[2] == 's'
                    && p->InstrumentID[3] == 't'
                    && p->InstrumentID[0] == 'l'
                    && p->InstrumentID[1] == 'a'
                        )
                {
                    my_shfe_md_inf_.OnMBLData(&empty_data.field, true);
                    p_shfe_deep_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &empty_data);
                }
                else
                {
                    memcpy(data.field.InstrumentID, p->InstrumentID, sizeof(data.field.InstrumentID));
                    data.field.Direction = p->Direction;
                    data.field.Price = p->Price;
                    data.field.Volume = p->Volume;
                    //data.isLast = false;

                    if (shfe_deep_data_handler_
                        && (subscribe_contracts_.empty()
                            || subscribe_contracts_.find(data.field.InstrumentID) != subscribe_contracts_.end()))
                    {
                        shfe_deep_data_handler_(&data);
                    }

                    my_shfe_md_inf_.OnMBLData(&data.field, false);
                    p_shfe_deep_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
                }

                //p_save_direct_mbl->OnQuoteData((long long) t.tv_sec * 1000000 + t.tv_usec, mbl_buffer, mbl_count);
            }

            my_shfe_md_inf_.ProcessData(&my_shfe_md_inf_);
        }
    }
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
        QuoteUpdateState(qtm_name_, QtmState::CREATE_UDP_FD_FAIL);
        MY_LOG_FATAL("ZEUSING_UDP - bind failed: %s:%d", addr_ip.c_str(), port);
        return -1;
    }

    // set nonblock flag
    int socket_ctl_flag = fcntl(udp_client_fd, F_GETFL);
    if (socket_ctl_flag < 0)
    {
        MY_LOG_WARN("ZEUSING_UDP - get socket control flag failed.");
    }
    if (fcntl(udp_client_fd, F_SETFL, socket_ctl_flag | O_NONBLOCK) < 0)
    {
        MY_LOG_WARN("ZEUSING_UDP - set socket control flag with nonblock failed.");
    }

    // set buffer length
    int rcvbufsize = 1 * 1024 * 1024;
    int ret = setsockopt(udp_client_fd, SOL_SOCKET, SO_RCVBUF, (const void *) &rcvbufsize, sizeof(rcvbufsize));
    if (ret != 0)
    {
        MY_LOG_WARN("ZEUSING_UDP - set SO_RCVBUF failed.");
    }

    int broadcast_on = 1;
    ret = setsockopt(udp_client_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_on, sizeof(broadcast_on));
    if (ret != 0)
    {
        MY_LOG_WARN("ZEUSING_UDP - set SO_RCVBUF failed.");
    }

    return udp_client_fd;
}

inline void CheckAndSetInvalidToZeroD(double &v)
{
    v = ((v > MIN_PURE_DBL ) && (v < MAX_PURE_DBL )) ? v : 0;
}
void CheckShfeDepthMD(CDepthMarketDataField* p)
{
    CheckAndSetInvalidToZeroD(p->LastPrice);            // 成交价
    CheckAndSetInvalidToZeroD(p->BidPrice1);            // 买一价
    CheckAndSetInvalidToZeroD(p->AskPrice1);            // 卖一价
    CheckAndSetInvalidToZeroD(p->OpenInterest);         // 持仓量
    CheckAndSetInvalidToZeroD(p->PreSettlementPrice);   // 昨结算价

//    CheckAndSetInvalidToZeroD(p->BidPrice2);            // 买二价
//    CheckAndSetInvalidToZeroD(p->BidPrice3);            // 买三价
//    CheckAndSetInvalidToZeroD(p->BidPrice4);            // 买四价
//    CheckAndSetInvalidToZeroD(p->BidPrice5);            // 买五价
//
//    CheckAndSetInvalidToZeroD(p->AskPrice2);            // 卖二价
//    CheckAndSetInvalidToZeroD(p->AskPrice3);            // 卖三价
//    CheckAndSetInvalidToZeroD(p->AskPrice4);            // 卖四价
//    CheckAndSetInvalidToZeroD(p->AskPrice5);            // 卖五价

    CheckAndSetInvalidToZeroD(p->UpperLimitPrice);      // 涨停价
    CheckAndSetInvalidToZeroD(p->LowerLimitPrice);      // 跌停价
    CheckAndSetInvalidToZeroD(p->OpenPrice);            // 开盘价
    CheckAndSetInvalidToZeroD(p->HighestPrice);         // 当日最高价
    CheckAndSetInvalidToZeroD(p->LowestPrice);          // 当日最低价
    CheckAndSetInvalidToZeroD(p->PreClosePrice);        // 昨收盘价
    CheckAndSetInvalidToZeroD(p->PreOpenInterest);      // 昨持仓价

    CheckAndSetInvalidToZeroD(p->Turnover);        // 成交金额
    CheckAndSetInvalidToZeroD(p->ClosePrice);      // 今收盘
    CheckAndSetInvalidToZeroD(p->SettlementPrice); // 今结算
    CheckAndSetInvalidToZeroD(p->PreDelta);        // 昨虚实度
    CheckAndSetInvalidToZeroD(p->CurrDelta);       // 今虚实度
}

bool QuoteInterface_MY_SHFE_MD::ParseMarketData(char* buffer, int nCount, QuoteDataSaveDirect *p_save_direct)
{
    if (nCount < (int) sizeof(TFTDCHeader))
    {
        MY_LOG_ERROR("ZEUSING_UDP - Invalid Package!");
        return false;
    }
    TFTDCHeader *header = (TFTDCHeader *) buffer;

    ///校验一下包是否正确
    if (header->Chain != 'L' && header->Chain != 'C')
    {
        MY_LOG_ERROR("Invalid Package!,Invalid Header[%c]", header->Chain);
        return false;
    }

    if (header->FTDCContentLength != (nCount - sizeof(TFTDCHeader)))
    {
        MY_LOG_ERROR("Invalid Package Length, Content Len[%d],Expected Len[%d]", nCount - sizeof(TFTDCHeader));
        return false;
    }

    if (header->TransactionId != 0x0000F103)
    {
        MY_LOG_ERROR("Not a Market Package!");
        return false;
    }

    CDepthMarketDataField data;
    memset(&data, 0, sizeof(data));

    TFieldHeader *fieldHeader;
    ///遍历Field
    char* pz = buffer + sizeof(TFTDCHeader);
    for (int i = 0; i < header->FieldCount; i++)
    {
        fieldHeader = (TFieldHeader *) pz;
        pz += sizeof(TFieldHeader);
        switch (fieldHeader->FieldID)
        {
            case 0x2439:
                {
                //memcpy(&updateTimeField, pz, fieldHeader->Size);
                CMarketDataUpdateTimeField *p = (CMarketDataUpdateTimeField *) pz;
                memcpy(data.InstrumentID, p->InstrumentID, 31);
                memcpy(data.UpdateTime, p->UpdateTime, 9);
                data.UpdateMillisec = p->UpdateMillisec;
                memcpy(data.ActionDay, p->ActionDay, 9);
                pz += fieldHeader->Size;
                //MY_LOG_INFO("CMarketDataUpdateTimeField: %s %s.%d %s", p->InstrumentID, p->UpdateTime, p->UpdateMillisec,
                //    p->ActionDay);
            }
                break;
            case 0x2434:
                {
                //memcpy(&bestPriceField, pz, fieldHeader->Size);
                CMarketDataBestPriceField *p = (CMarketDataBestPriceField *) pz;
                data.BidPrice1 = p->BidPrice1;
                data.BidVolume1 = p->BidVolume1;
                data.AskPrice1 = p->AskPrice1;
                data.AskVolume1 = p->AskVolume1;
                pz += fieldHeader->Size;
                //MY_LOG_INFO("CMarketDataBestPriceField: %.4f %d %.4f %d", p->BidPrice1, p->BidVolume1, p->AskPrice1, p->AskVolume1);
            }
                break;
            case 0x2432:
                {
                //memcpy(&staticField, pz, fieldHeader->Size);
                CMarketDataStaticField *p = (CMarketDataStaticField *) pz;
                data.OpenPrice = p->OpenPrice;
                data.HighestPrice = p->HighestPrice;
                data.LowestPrice = p->LowestPrice;
                data.ClosePrice = p->ClosePrice;
                data.UpperLimitPrice = p->UpperLimitPrice;
                data.LowerLimitPrice = p->LowerLimitPrice;
                data.SettlementPrice = p->SettlementPrice;
                data.CurrDelta = p->CurrDelta;
                pz += fieldHeader->Size;
                //MY_LOG_INFO("CMarketDataStaticField: %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f", p->OpenPrice, p->HighestPrice,
                //    p->LowestPrice, p->ClosePrice, p->UpperLimitPrice, p->LowerLimitPrice, p->SettlementPrice, p->CurrDelta);
            }
                break;
            case 0x2433:
                {
                //memcpy(&lastMatchField, pz, fieldHeader->Size);
                CMarketDataLastMatchField * p = (CMarketDataLastMatchField *) pz;
                data.LastPrice = p->LastPrice;
                data.Volume = p->Volume;
                data.Turnover = p->Turnover;
                data.OpenInterest = p->OpenInterest;
                pz += fieldHeader->Size;
                //MY_LOG_INFO("CMarketDataLastMatchField: %.4f %d %.4f %.2f", p->LastPrice, p->Volume, p->Turnover, p->OpenInterest);
            }
                break;
            case 0x2435:
                {
                //memcpy(&bid23Field, pz, fieldHeader->Size);
//                CMarketDataBid23Field *p = (CMarketDataBid23Field *) pz;
//                data.BidPrice2 = p->BidPrice2;
//                data.BidVolume2 = p->BidVolume2;
//                data.BidPrice3 = p->BidPrice3;
//                data.BidVolume3 = p->BidVolume3;
                pz += fieldHeader->Size;
                //MY_LOG_INFO("CMarketDataBid23Field: %.4f %d %.4f %d", p->BidPrice2, p->BidVolume2, p->BidPrice3, p->BidVolume3);
            }
                break;
            case 0x2436:
                {
                //memcpy(&ask23Field, pz, fieldHeader->Size);
//                CMarketDataAsk23Field *p = (CMarketDataAsk23Field *) pz;
//                data.AskPrice2 = p->AskPrice2;
//                data.AskVolume2 = p->AskVolume2;
//                data.AskPrice3 = p->AskPrice3;
//                data.AskVolume3 = p->AskVolume3;
                pz += fieldHeader->Size;
                //MY_LOG_INFO("CMarketDataAsk23Field: %.4f %d %.4f %d", p->AskPrice2, p->AskVolume2, p->AskPrice3, p->AskVolume3);
            }
                break;
            case 0x2437:
                {
                //memcpy(&bid45Field, pz, fieldHeader->Size);
//                CMarketDataBid45Field *p = (CMarketDataBid45Field *) pz;
//                data.BidPrice4 = p->BidPrice4;
//                data.BidVolume4 = p->BidVolume4;
//                data.BidPrice5 = p->BidPrice5;
//                data.BidVolume5 = p->BidVolume5;
                pz += fieldHeader->Size;
                //MY_LOG_INFO("CMarketDataBid45Field: %.4f %d %.4f %d", p->BidPrice4, p->BidVolume4, p->BidPrice5, p->BidVolume5);
            }
                break;
            case 0x2438:
                {
                //memcpy(&ask45Field, pz, fieldHeader->Size);
//                CMarketDataAsk45Field *p = (CMarketDataAsk45Field *) pz;
//                data.AskPrice4 = p->AskPrice4;
//                data.AskVolume4 = p->AskVolume4;
//                data.AskPrice5 = p->AskPrice5;
//                data.AskVolume5 = p->AskVolume5;
                pz += fieldHeader->Size;
                //MY_LOG_INFO("CMarketDataAsk45Field: %.4f %d %.4f %d", p->AskPrice4, p->AskVolume4, p->AskPrice5, p->AskVolume5);
            }
                break;
            case 0x2431:
                {
                //memcpy(&databaseField, pz, fieldHeader.Size);
                CFTDMarketDataBaseField *p = (CFTDMarketDataBaseField *) pz;
                memcpy(data.TradingDay, p->TradingDay, 9);
                data.PreSettlementPrice = p->PreSettlementPrice;
                data.PreClosePrice = p->PreClosePrice;
                data.PreOpenInterest = p->PreOpenInterest;
                data.PreDelta = p->PreDelta;
                pz += fieldHeader->Size;
                //MY_LOG_INFO("CFTDMarketDataBaseField: %s %.4f %.4f %.4f %.4f", p->TradingDay, p->PreSettlementPrice, p->PreClosePrice, p->PreOpenInterest, p->PreDelta);
            }
                break;
        }
    }

    CheckShfeDepthMD(&data);

    timeval t;
    gettimeofday(&t, NULL);
    if (shfe_ex_handler_
        && (subscribe_contracts_.empty()
            || subscribe_contracts_.find(data.InstrumentID) != subscribe_contracts_.end()))
    {
        shfe_ex_handler_(&data);
    }
    my_shfe_md_inf_.OnDepthMarketData(&data);
    p_shfe_ex_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
    //p_save_direct->OnQuoteData((long long) t.tv_sec * 1000000 + t.tv_usec, buffer, nCount);

    return true;
}
