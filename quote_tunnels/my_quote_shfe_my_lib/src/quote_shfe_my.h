#ifndef MY_SHFE_MD_QUOTE_INTERFACE_H_
#define MY_SHFE_MD_QUOTE_INTERFACE_H_

#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include "quote_datatype_shfe_my.h"
#include "quote_datatype_shfe_deep.h"
#include "quote_datatype_level1.h"

#include "quote_interface_shfe_my.h"

#include "shfe_my_data_manager.h"

#include <set>
struct DepthMarketDataID
{
    char time_code[40];
    int update_milisecond;

    DepthMarketDataID(const CDepthMarketDataField &md)
    {
        memcpy(time_code, md.UpdateTime, 9);        // char    UpdateTime[9];
        memcpy(time_code + 9, md.InstrumentID, 31); // char   InstrumentID[31];
        update_milisecond = md.UpdateMillisec;
    }

    bool operator==(const DepthMarketDataID &rhs) const
        {
        return update_milisecond == rhs.update_milisecond
            && memcmp(time_code, rhs.time_code, 40) == 0;
    }
    bool operator<(const DepthMarketDataID &rhs) const
        {
        return update_milisecond < rhs.update_milisecond
            || (update_milisecond == rhs.update_milisecond
                && memcmp(time_code, rhs.time_code, 40) < 0);
    }
};
typedef std::set<DepthMarketDataID> DepthMarketDataIDCollection;

struct ShfeProviderStatus
{
    int id;
    int status;
    char desc[80];
};

class QuoteInterface_MY_SHFE_MD: public MYShfeMDHandler
{
public:
    // 构造函数
    QuoteInterface_MY_SHFE_MD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const SHFEQuote *)> quote_data_handler)
    {
        shfe_deep_data_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const CDepthMarketDataField *)> quote_data_handler)
    {
        shfe_ex_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const MYShfeMarketData *)> quote_data_handler)
    {
        my_shfe_md_handler_ = quote_data_handler;
    }

    virtual ~QuoteInterface_MY_SHFE_MD();

private:
    // quote data handlers
    void OnMYShfeMDData(MYShfeMarketData *data);
    void ShfeDepthMarketDataHandler();
    void ShfeMBLHandler();
    void ProviderStatusHandler();

    // 数据处理函数对象
    boost::function<void(const SHFEQuote *)> shfe_deep_data_handler_;
    boost::function<void(const CDepthMarketDataField *)> shfe_ex_handler_;
    boost::function<void(const MYShfeMarketData *)> my_shfe_md_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;

    // save assistant object
    QuoteDataSave<SHFEQuote> *p_shfe_deep_save_;
    QuoteDataSave<CDepthMarketDataField> *p_shfe_ex_save_;
    QuoteDataSave<MYShfeMarketData> *p_my_shfe_md_save_;

    // receive threads
    volatile bool running_flag_;
    boost::thread *p_mbl_handler_;
    boost::thread *p_shfe_depth_md_handler_;
    boost::thread *p_shfe_status_handler_;
    char qtm_name_[32];

    MYShfeMDManager my_shfe_md_inf_;

    // communication
    void *zmq_context_;

    // id collection for filter duplicate depth md
    DepthMarketDataIDCollection md_ids_;

    // socket monitor thread
    static void *rep_socket_monitor(void *vp, const std::string &pair_addr, int *connect_counter);
    boost::thread *t_mbl_socket_monitor_;
    boost::thread *t_md_socket_monitor_;
    boost::mutex status_sync_;
    int mbl_connect_cnt_;
    int mbl_connect_cnt_cfg_;
    int md_connect_cnt_;
    int md_connect_cnt_cfg_;
    std::set<int> provider_normal_set_;
    int provider_cnt_cfg_;
};

#endif
