#ifndef QUOTE_DCE_UDP_H
#define QUOTE_DCE_UDP_H


#include <sys/types.h>
#include <stdint.h>
#include <dirent.h>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include "my_cmn_util_funcs.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include "quote_interface_dce_level2.h"

class DceUdpMD
{
public:
    // 构造函数
    DceUdpMD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(std::function<void(const MDBestAndDeep_MY *)> quote_data_handler)
    {
        best_and_deep_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(std::function<void(const MDOrderStatistic_MY *)> quote_data_handler)
    {
        order_statistic_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(std::function<void(const MDTenEntrust_MY *)> quote_data_handler)
    {
        MY_LOG_WARN("DCE_UDP - not support datatype TenEntrust");
    }
    void SetQuoteDataHandler(std::function<void(const MDArbi_MY *)> quote_data_handler)
    {
        MY_LOG_WARN("DCE_UDP - not support datatype Arbi");
    }
    void SetQuoteDataHandler(std::function<void(const MDRealTimePrice_MY *)> quote_data_handler)
    {
        MY_LOG_WARN("DCE_UDP - not support datatype RealTimePrice");
    }
    void SetQuoteDataHandler(std::function<void(const MDMarchPriceQty_MY *)> quote_data_handler)
    {
        MY_LOG_WARN("DCE_UDP - not support datatype MarchPriceQty");
    }

    virtual ~DceUdpMD();

private:
    // quote data handlers
    void UdpDataHandler();
    int CreateUdpFD(const std::string &addr_ip, unsigned short port);

    // 数据处理函数对象
    std::function<void(const MDBestAndDeep_MY *)> best_and_deep_handler_;
    std::function<void(const MDOrderStatistic_MY *)> order_statistic_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    char qtm_name_[32];

    // save assistant object
    QuoteDataSave<MDBestAndDeep_MY> *p_save_best_and_deep_;
    QuoteDataSave<MDOrderStatistic_MY> *p_save_order_statistic_;

    // receive threads
    volatile bool running_flag_;
    boost::thread *p_md_handler_;
};

#endif
