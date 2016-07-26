#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <iomanip>
#include <vector>
#include <thread>
#include <string.h>

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "czce_multicast_quote.h"
using namespace my_cmn;
using namespace std;

inline long long GetMicrosFromEpoch()
{
    // get us(micro seconds) from Unix Epoch
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

MYLTSMDL2::MYLTSMDL2(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), recv_real_time_thread_(NULL), recv_recovery_thread_(NULL), p_md_l2_save_(NULL), snap_original_save_(NULL), recovery_original_save_(
    NULL), realtime_cmb_save_(NULL), recovery_cmb_save_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    // 初始化
    qtm_name = "zce_level2_mc_" + boost::lexical_cast<std::string>(getpid());
    run_flag_ = true;
    current_date = my_cmn::GetCurrentDateInt();

    p_md_l2_save_ = new QuoteDataSave<ZCEL2QuotSnapshotField_MY>(cfg_, qtm_name.c_str(), "czce_level2", TAP_QUOTE_TYPE, true);
    snap_original_save_ = new QuoteDataSave<quote_snap_shot_t>(cfg_, qtm_name.c_str(), "snap_original_quote", TAP_QUOTE_TYPE, false);
    recovery_original_save_ = new QuoteDataSave<ZCEL2QuotSnapshotField_MY>(cfg_, qtm_name.c_str(), "recovery_czce_level2", TAP_QUOTE_TYPE, false);
    realtime_cmb_save_ = new QuoteDataSave<ZCEQuotCMBQuotField_MY>(cfg_, qtm_name.c_str(), "czce_cmb", TAP_QUOTE_TYPE, false);
    recovery_cmb_save_ = new QuoteDataSave<ZCEQuotCMBQuotField_MY>(cfg_, qtm_name.c_str(), "recovery_czce_cmb", TAP_QUOTE_TYPE, false);

    // start receive thread
    recv_real_time_thread_ = new boost::thread(&MYLTSMDL2::Start_Real_Time_Recv, this);
    recv_recovery_thread_ = new boost::thread(&MYLTSMDL2::Start_Recovery_Recv, this);
}

MYLTSMDL2::~MYLTSMDL2(void)
{
    run_flag_ = false;

    if (recv_real_time_thread_)
    {
        if (recv_real_time_thread_->joinable())
        {
            recv_real_time_thread_->join();
        }
    }

    if (p_md_l2_save_) delete p_md_l2_save_;

    if (recv_recovery_thread_)
    {
        if (recv_recovery_thread_->joinable())
        {
            recv_recovery_thread_->join();
        }
    }
    if (snap_original_save_) delete snap_original_save_;
    if (recovery_original_save_) delete recovery_original_save_;
}

int MYLTSMDL2::CreateUdpFD(const std::string& addr_ip, const std::string& gourp_ip, unsigned short int port)
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
    //  listen_addr.sin_addr.s_addr = inet_addr(addr_ip.c_str());
    inet_pton(AF_INET, addr_ip.c_str(), &listen_addr.sin_addr);

    if (0 != bind(udp_client_id, (sockaddr *) &listen_addr, sizeof(listen_addr)))
    {
        MY_LOG_ERROR("Failed to Bind Socket");
        return -1;
    }

    struct ip_mreq m_mreq;
    memset(&m_mreq, 0, sizeof(m_mreq));
    inet_pton(AF_INET, addr_ip.c_str(), &m_mreq.imr_multiaddr.s_addr);

    inet_pton(AF_INET, gourp_ip.c_str(), &m_mreq.imr_interface.s_addr);

    if (0 != setsockopt(udp_client_id, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &m_mreq, sizeof(m_mreq)))
    {
        MY_LOG_ERROR("Failed to join multicast");
        return -1;
    }

    return udp_client_id;
}

int split_by_char(char *contr_set[], char *base_addr, char *spl_ele)
{
    int idx = 0, ret_idx = 0;
    if (base_addr[0] != *spl_ele)
    {
        contr_set[ret_idx++] = base_addr;
    }

    while (base_addr[idx] != '\0')
    {
        if (base_addr[idx] == *spl_ele)
        {
            base_addr[idx] = '\0';
            contr_set[ret_idx++] = &base_addr[idx + 1];
        }
        idx++;
    };
    return ret_idx;
}

int convert_inner(quote_snap_shot_t *snap_data, char *recv_buffer)
{
    snap_data->seq_num = *(unsigned int*) (recv_buffer + SNAP_SEQ_NUM_OFFSET);
    snap_data->field_cnt = *(unsigned char *) (recv_buffer + SNAP_FIELD_CNT_OFFSET);
    snap_data->update_time = *(unsigned long long *) (recv_buffer + SNAP_UPDATE_TIME_OFFSET);
    /* extract contract  */
    char *contr_set[MAX_CONTR_TYPE_LEN];
    char spl_ele = '|';
    int ret = split_by_char(contr_set, (recv_buffer + SNAP_CONTR_SET_OFFSET), &spl_ele);
    if (ret < 3)
    {
        MY_LOG_ERROR(" split_by_char() failed \n");
    }

    if (0 == strncmp(contr_set[F_CONTR_SET_TYPE_IDX], "F", STR_CMP_NUM))
    {
        snap_data->contr_type = 'F';
    }
    else if (0 == strncmp(contr_set[F_CONTR_SET_TYPE_IDX], "O", STR_CMP_NUM))
    {
        snap_data->contr_type = 'O';
    }
    else if (0 == strncmp(contr_set[F_CONTR_SET_TYPE_IDX], "S", STR_CMP_NUM))
    {
        snap_data->contr_type = 'S';
    }
    else if (0 == strncmp(contr_set[F_CONTR_SET_TYPE_IDX], "M", STR_CMP_NUM))
    {
        snap_data->contr_type = 'M';
    }
    else
    {
        MY_LOG_ERROR(" contr_set  contract type extracted failed \n");
    }

    int item_len = strlen(contr_set[F_CONTR_SET_ITEM_IDX]);
    int date_len = strlen(contr_set[F_CONTR_SET_DATE_IDX]);
    //int end_date_len = strlen(contr_set[F_CONTR_SET_END_DATE_IDX]);
    /* connect contract */
    strncpy(snap_data->contr, contr_set[F_CONTR_SET_ITEM_IDX], item_len);
    strncpy((snap_data->contr + item_len), contr_set[F_CONTR_SET_DATE_IDX], date_len + 1);
    if (snap_data->contr_type == 'S')
    {
        strncpy((snap_data->contr + item_len + date_len), "-", sizeof("-"));
        strncpy((snap_data->contr + item_len + date_len + CONNECT_CAL_SPR_ABR_LEN), snap_data->contr, item_len + date_len);
    }

    /* obtain quote field  */
    for (int idx = 0; idx < snap_data->field_cnt; idx++)
    {
        char *base = (recv_buffer + SNAP_QUOTE_AR_OFFSET + SNAP_QUOTE_AR_EACH_LEN * idx);
        unsigned char each_field_idx = *(unsigned char *) base;
        //MY_LOG_INFO("each_file_idx = %d , ",each_field_idx);
        memcpy(((char *) (&(snap_data->quote_field)) + FIELD_LEN * each_field_idx), (base + SNAP_QUOTE_FID_MEAN), FIELD_LEN);
    }

    return 0;
}

struct ZCEQuotCMBQuotField_MY convert_cmb_quote(quote_snap_shot_t *pd)
{
    struct ZCEQuotCMBQuotField_MY data;
    memset(&data, 0, sizeof(data));

    strncpy(data.ContractID, pd->contr, sizeof(pd->contr));
    data.CmbType = pd->contr_type;/*合约类型, 合约类型： M->跨品种套利  S->跨期套利*/

    data.BidPrice = pd->quote_field.bid_price[0]; /*买入价格*/
    data.AskPrice = pd->quote_field.ask_price[0]; /*卖出价*/

    data.BidLot = pd->quote_field.bid_qty[0]; /*买入数量*/
    data.AskLot = pd->quote_field.ask_qty[0]; /*卖出数量*/

    data.VolBidLot = pd->quote_field.tot_bid_qty; /*委买总量*/
    data.VolAskLot = pd->quote_field.tot_ask_qty; /*委卖总量*/

    /* 格式：integer YYYYMMDDhhmmss000 to string YYYY-MM-DD hh:mm:ss.000 */
    snprintf(data.TimeStamp, sizeof(data.TimeStamp), "%d-%02d-%02d %02d:%02d:%02d.%03d",
        (int) (pd->update_time / 10000000000000),
        (int) ((pd->update_time / 100000000000) % 100),
        (int) ((pd->update_time / 1000000000) % 100),
        (int) ((pd->update_time / 10000000) % 100),
        (int) ((pd->update_time / 100000) % 100),
        (int) ((pd->update_time / 1000) % 100),
        (int) (pd->update_time % 1000));

    return data;
}

void MYLTSMDL2::Start_Real_Time_Recv()
{
    const unsigned short int SNAP_QUOTE_UINT16 = 0x1001;
    const unsigned short int MSG_RECOVERY_END = 0x2001;

    std::string realtime_ip, recovery_ip, gourp_ip;
    unsigned short int realtime_port, recovery_port;
    for (const zce_multi_config &zce_multi_cfg : cfg_.Logon_config().zce_multi_cfg)
    {
        realtime_port = (unsigned short int) atoi(zce_multi_cfg.realtime_port.c_str());
        realtime_ip = zce_multi_cfg.realtime_ip;

        recovery_port = (unsigned short int) atoi(zce_multi_cfg.recovery_port.c_str());
        recovery_ip = zce_multi_cfg.recovery_ip;

        gourp_ip = zce_multi_cfg.gourp_ip;

        MY_LOG_INFO("Start_Real_Time_Recv - realtime_ip : %s , port : %d , gourp_ip: %s", realtime_ip.c_str(), realtime_port, gourp_ip.c_str());
    }

    int udp_fd = CreateUdpFD(realtime_ip, gourp_ip, realtime_port);
    if (udp_fd < 0)
    {
        MY_LOG_ERROR("Multi_czce_Level2_UDP - CreateUdpFD failed.");
        return;
    }

    QuoteUpdateState(qtm_name.c_str(), QtmState::API_READY);

    sockaddr_in src_addr;
    socklen_t addr_len = sizeof(sockaddr_in);
    int recv_length;
    char recv_buffer[RECV_BUF_LEN];

    while (run_flag_)
    {
        addr_len = sizeof(sockaddr_in);
        recv_length = recvfrom(udp_fd, recv_buffer, RECV_BUF_LEN, 0, (sockaddr *) &src_addr, &addr_len);
        if ((recv_length == -1))
        {
            if ((errno == EAGAIN))
            {
                continue;
            }
            MY_LOG_ERROR("recv - recvfrom()  failed. errno :%s ", strerror(errno));
            //return ;
        }
        else
        {
            /* extract package head */
#if 0
            unsigned short int pkg_size = *(unsigned short int *)(recv_buffer + PKG_SIZE_OFFSET);
            unsigned long long pkg_seq_num = *(unsigned long long *)(recv_buffer + PKG_SEQ_NUM_OFFSET);
            unsigned char channle_id = *(unsigned char *)(recv_buffer + CHANNEL_ID_OFFSET);
            unsigned char channle_type = *(unsigned char *)(recv_buffer + CHANNEL_TYPE_OFFSET);
#endif
            unsigned char msg_cnt = *(unsigned char *) (recv_buffer + MSG_CNT_OFFSET);
            //MY_LOG_DEBUG("Receive[len: %d] From [%s][%d] , msg_cnt : %d", recv_length, inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port), msg_cnt);

            /*extract  message */
            int idx = 0, tot_msg_size = PKG_HEAD_LEN + MSG_HEAD_LEN;
            for (; idx < msg_cnt; idx++)
            {
                /* extract message head */
                unsigned short int msg_type = *(unsigned short int *) (recv_buffer + PKG_HEAD_LEN + MSG_TYPE_OFFSET);
                unsigned short int msg_size = *(unsigned short int *) (recv_buffer + PKG_HEAD_LEN + MSG_SIZE_OFFSET);

                /* extract  message body */
                if (msg_type == SNAP_QUOTE_UINT16)
                {

                    /* extract snap shot message  */
                    quote_snap_shot_t snap_data;
                    memset(&snap_data, 0, sizeof(quote_snap_shot_t));
                    convert_inner(&snap_data, (recv_buffer + tot_msg_size));

                    long long cur_tp = GetMicrosFromEpoch();

                    if (snap_data.contr_type == 'F' || snap_data.contr_type == 'O')
                    {
                        ZCEL2QuotSnapshotField_MY data = Convert(&snap_data);
                        // send to client
                        if (snap_l2_handler_
                            && (subscribe_contracts_.empty() || subscribe_contracts_.find(data.ContractID) != subscribe_contracts_.end()))
                        {
                            snap_l2_handler_(&data);
                        }
                        // save it

                        snap_original_save_->OnQuoteData(cur_tp, &snap_data);
                        p_md_l2_save_->OnQuoteData(cur_tp, &data);
                    }
                    else
                    {
                        /* snap_data.contr_type == 'M'  snap_data.contr_type== 'S'  */
                        ZCEQuotCMBQuotField_MY cmb_quote = convert_cmb_quote(&snap_data);
                        realtime_cmb_save_->OnQuoteData(cur_tp, &cmb_quote);
                    }

                    /*update tot_msg_size*/
                    tot_msg_size += (msg_size + MSG_HEAD_LEN);
                }
                else if (msg_type == MSG_RECOVERY_END)
                {
                    /* just msg recovery end */
                    tot_msg_size += (msg_size + MSG_HEAD_LEN);

                }
                else
                {
                    /* do nothing */
                    printf("something is bad happen \n");
                }
            }
        }
    }
}

void MYLTSMDL2::Start_Recovery_Recv()
{
    const unsigned short int SNAP_QUOTE_UINT16 = 0x1001;
    const unsigned short int MSG_RECOVERY_END = 0x2001;

    std::string realtime_ip, recovery_ip, gourp_ip;
    unsigned short int realtime_port, recovery_port;
    for (const zce_multi_config &zce_multi_cfg : cfg_.Logon_config().zce_multi_cfg)
    {
        realtime_port = (unsigned short int) atoi(zce_multi_cfg.realtime_port.c_str());
        realtime_ip = zce_multi_cfg.realtime_ip;

        recovery_port = (unsigned short int) atoi(zce_multi_cfg.recovery_port.c_str());
        recovery_ip = zce_multi_cfg.recovery_ip;

        gourp_ip = zce_multi_cfg.gourp_ip;

        MY_LOG_INFO(" Start_Recovery_Recv -recovery_ip : %s , port : %d , gourp_ip: %s ", recovery_ip.c_str(), recovery_port, gourp_ip.c_str());
    }

    int udp_fd = CreateUdpFD(recovery_ip, gourp_ip, recovery_port);
    if (udp_fd < 0)
    {
        MY_LOG_ERROR("Multi_czce_Level2_UDP - CreateUdpFD failed.");
        return;
    }

    sockaddr_in src_addr;
    socklen_t addr_len = sizeof(sockaddr_in);
    int recv_length;
    char recv_buffer[RECV_BUF_LEN];

    while (run_flag_)
    {
        addr_len = sizeof(sockaddr_in);
        recv_length = recvfrom(udp_fd, recv_buffer, RECV_BUF_LEN, 0, (sockaddr *) &src_addr, &addr_len);
        if ((recv_length == -1))
        {
            if ((errno == EAGAIN))
            {
                continue;
            }

            MY_LOG_ERROR("recv - recvfrom()  failed. errno :%s ", strerror(errno));
            //return ;
        }
        else
        {
            //    MY_LOG_DEBUG("Receive[len: %d] From [%s][%d]", recv_length, inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port));
            /* extract package head */
#if 0
            unsigned short int pkg_size = *(unsigned short int *)(recv_buffer + PKG_SIZE_OFFSET);
            unsigned long long pkg_seq_num = *(unsigned long long *)(recv_buffer + PKG_SEQ_NUM_OFFSET);
            unsigned char channle_id = *(unsigned char *)(recv_buffer + CHANNEL_ID_OFFSET);
            unsigned char channle_type = *(unsigned char *)(recv_buffer + CHANNEL_TYPE_OFFSET);
#endif
            unsigned char msg_cnt = *(unsigned char *) (recv_buffer + MSG_CNT_OFFSET);

            /*extract  message */
            int idx = 0, tot_msg_size = PKG_HEAD_LEN + MSG_HEAD_LEN;
            for (; idx < msg_cnt; idx++)
            {
                /* extract message head */
                unsigned short int msg_type = *(unsigned short int *) (recv_buffer + PKG_HEAD_LEN + MSG_TYPE_OFFSET);
                unsigned short int msg_size = *(unsigned short int *) (recv_buffer + PKG_HEAD_LEN + MSG_SIZE_OFFSET);

                /* extract  message body */
                if (msg_type == SNAP_QUOTE_UINT16)
                {
                    /* extract snap shot message  */
                    quote_snap_shot_t snap_data;
                    convert_inner(&snap_data, (recv_buffer + tot_msg_size));

                    long long cur_tp = GetMicrosFromEpoch();
                    if (snap_data.contr_type == 'F' || snap_data.contr_type == 'O')
                    {
                        //recovery_original_save_->OnQuoteData(cur_tp, &snap_data);
                        ZCEL2QuotSnapshotField_MY data = Convert(&snap_data);
                        // save it
                        recovery_original_save_->OnQuoteData(cur_tp, &data);
                    }
                    else
                    {
                        /* snap_data.contr_type == 'M'  snap_data.contr_type== 'S'  */
                        ZCEQuotCMBQuotField_MY recovery_cmb_quote = convert_cmb_quote(&snap_data);
                        recovery_cmb_save_->OnQuoteData(cur_tp, &recovery_cmb_quote);
                    }

                    /*update tot_msg_size*/
                    tot_msg_size += (msg_size + MSG_HEAD_LEN);
                }
                else if (msg_type == MSG_RECOVERY_END)
                {
                    /* just msg recovery end */
                    tot_msg_size += (msg_size + MSG_HEAD_LEN);
                }
                else
                {
                    /* do nothing */
                }
            }
        }
    }
}

struct ZCEL2QuotSnapshotField_MY MYLTSMDL2::Convert(const quote_snap_shot_t *pd)
{
    struct ZCEL2QuotSnapshotField_MY data;
    memset(&data, 0, sizeof(data));

    strncpy(data.ContractID, pd->contr, sizeof(pd->contr));
    data.ContractIDType = pd->contr_type;/*合约类型 0->目前应该为0， 扩充：0:期货,1:期权,2:组合*/

    data.PreSettle = pd->quote_field.pre_settle_price; /*前结算价格*/
    data.PreClose = pd->quote_field.pre_close_price; /*前收盘价格*/
    data.PreOpenInterest = pd->quote_field.pre_pos_qty; /*昨日昨日持仓量*/
    data.OpenPrice = pd->quote_field.open_price; /*开盘价*/
    data.HighPrice = pd->quote_field.high_price; /*最高价*/
    data.LowPrice = pd->quote_field.low_price; /*最低价*/
    data.LastPrice = pd->quote_field.last_price; /*最新价*/

    memcpy(&data.BidPrice, pd->quote_field.bid_price, sizeof(data.BidPrice)); /*买入价格 下标从0开始*/
    memcpy(&data.AskPrice, pd->quote_field.ask_price, sizeof(data.AskPrice)); /*卖出价 下标从0开始*/

    for (int idx = 0; idx < 5; idx++)
    {
        data.BidLot[idx] = pd->quote_field.bid_qty[idx];
        data.AskLot[idx] = pd->quote_field.ask_qty[idx];
    }
    //memcpy(&data.BidLot, pd->quote_field.bid_qty, sizeof(data.BidLot)); /*买入数量 下标从0开始*/
    //memcpy(&data.AskLot, pd->quote_field.ask_qty, sizeof(data.AskLot)); /*卖出数量 下标从0开始*/

    data.TotalVolume = pd->quote_field.tot_match_qty; /*总成交量*/
    data.OpenInterest = pd->quote_field.pos_qty; /*持仓量*/
    data.ClosePrice = pd->quote_field.close_price; /*收盘价*/
    data.SettlePrice = pd->quote_field.settle_price; /*结算价*/
    data.AveragePrice = pd->quote_field.aver_price; /*均价*/
    data.LifeHigh = pd->quote_field.his_high_price; /*历史最高成交价格*/
    data.LifeLow = pd->quote_field.his_low_price; /*历史最低成交价格*/
    data.HighLimit = pd->quote_field.lim_up_price; /*涨停板*/
    data.LowLimit = pd->quote_field.lim_down_price; /*跌停板*/
    data.TotalBidLot = pd->quote_field.tot_bid_qty; /*委买总量*/
    data.TotalAskLot = pd->quote_field.tot_ask_qty; /*委卖总量*/

    /* 格式：integer YYYYMMDDhhmmss000 to string YYYY-MM-DD hh:mm:ss.000 */
    snprintf(data.TimeStamp, sizeof(data.TimeStamp), "%d-%02d-%02d %02d:%02d:%02d.%03d",
        (int) (pd->update_time / 10000000000000),
        (int) ((pd->update_time / 100000000000) % 100),
        (int) ((pd->update_time / 1000000000) % 100),
        (int) ((pd->update_time / 10000000) % 100),
        (int) ((pd->update_time / 100000) % 100),
        (int) ((pd->update_time / 1000) % 100),
        (int) (pd->update_time % 1000));

    return data;
}

