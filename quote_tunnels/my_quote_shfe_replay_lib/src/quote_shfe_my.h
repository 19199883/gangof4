#pragma once

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

class QuoteInterface_MY_SHFE_MD
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

    void OnMYShfeMDData(MYShfeMarketData *data);
    bool running_flag_;

    void Replay();
    std::string md_file_name;
    int wait_seconds;
    int interval_us;
};
