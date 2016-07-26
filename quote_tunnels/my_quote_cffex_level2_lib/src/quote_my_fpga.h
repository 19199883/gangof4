#pragma once

#include <pthread.h>
#include <string>
#include <sstream>
#include <list>
#include <unordered_map>
#include <map>
#include <mutex>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "ac_xele_md.h"
#include "quote_interface_cffex_level2.h"
#include "quote_datatype_cffex_level2.h"
#include "my_cmn_util_funcs.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"
#include "toe_app.h"

class MYFPGACffexDataHandler
{
public:
    MYFPGACffexDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler);

    virtual ~MYFPGACffexDataHandler(void);

private:
    // 数据处理函数对象
    boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    char qtm_name_[32];

    // save assistant object
    QuoteDataSave<CFfexFtdcDepthMarketData> *p_save_;

    // connect and receive variables
    struct pthread_event ev_;
    pthread_attr_t thread_attr_;
    std::list<session_init_struct *> sessions_;
    void set_init_parameters(uint8_t toe_id, uint8_t session_id, const std::string &mc_ip);
    static void *toe_recv_data(void *param);
    volatile bool running_flag_;

    static int md_seq_no;
    static std::mutex seq_lock;
    inline static bool UpdateSeqNo(int new_seq);
};

inline bool MYFPGACffexDataHandler::UpdateSeqNo(int new_seq)
{
    //_TTSpinLockGuard lock(seq_lock);
    std::lock_guard<std::mutex> lock(seq_lock);
    //printf("new_seq:%d, md_seq_no:%d\n", new_seq, md_seq_no);
    if (new_seq > md_seq_no)
    {
        md_seq_no = new_seq;
        return true;
    }
    return false;
}
