#include "quote_interface_lts_level2.h"

#include <string>
#include <mutex>

#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "quote_lts_l2_udp.h"

using namespace std;
using namespace my_cmn;

void InitOnce()
{
    static volatile bool s_have_init = false;
    static std::mutex s_init_sync;

    if (s_have_init)
    {
        return;
    }
    else
    {
        std::lock_guard<std::mutex> lock(s_init_sync);
        if (s_have_init)
        {
            return;
        }
        std::string log_file_name = "my_quote_lib_lts_" + my_cmn::GetCurrentDateTimeString();
        (void) my_log::instance(log_file_name.c_str());
        MY_LOG_INFO("start init lts quote library.");

        // initialize quote monitor
        qtm_init(TYPE_QUOTE);

        s_have_init = true;
    }
}

MYQuoteData::MYQuoteData(const SubscribeContracts *subscribe_contracts, const std::string &provider_config_file)
{
    interface_ = NULL;

    InitOnce();

    if (provider_config_file.empty())
    {
        MY_LOG_ERROR("no quote provider config file");
    }

    MY_LOG_INFO("create MYQuoteData object with configure file: %s", provider_config_file.c_str());

    ConfigData cfg;
    cfg.Load(provider_config_file);
    int provider_type = cfg.App_cfg().provider_type;
    if (provider_type == QuoteProviderType::PROVIDERTYPE_LTS_L2)
    {
        InitInterface(subscribe_contracts, cfg);
    }
    else
    {
        MY_LOG_ERROR("not support quote provider type(%d), please check config file.", provider_type);
    }
}

bool MYQuoteData::InitInterface(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
{
    // 连接服务
    MY_LOG_INFO("prepare to start lts quote source data transfer");
    interface_ = new MYLTSMDL2(subscribe_contracts, cfg);

    return true;
}

/// stock level2行情
void MYQuoteData::SetQuoteDataHandler(boost::function<void(const TDF_MARKET_DATA_MY *)> quote_handler)
{
    if (interface_)
    {
        ((MYLTSMDL2 *) interface_)->SetQuoteDataHandler(quote_handler);
    }
    else
    {
        MY_LOG_ERROR("set stock level2 data handler failed.");
    }
}
void MYQuoteData::SetQuoteDataHandler(boost::function<void(const TDF_INDEX_DATA_MY *)> quote_handler)
{
    if (interface_)
    {
        ((MYLTSMDL2 *) interface_)->SetQuoteDataHandler(quote_handler);
    }
    else
    {
        MY_LOG_ERROR("set index data handler failed.");
    }
}
void MYQuoteData::SetQuoteDataHandler(boost::function<void(const T_PerEntrust *)> quote_handler)
{
    if (interface_)
    {
        ((MYLTSMDL2 *) interface_)->SetQuoteDataHandler(quote_handler);
    }
    else
    {
        MY_LOG_ERROR("set per entrust data handler failed.");
    }
}
void MYQuoteData::SetQuoteDataHandler(boost::function<void(const T_PerBargain *)> quote_handler)
{
    if (interface_)
    {
        ((MYLTSMDL2 *) interface_)->SetQuoteDataHandler(quote_handler);
    }
    else
    {
        MY_LOG_ERROR("set per match data handler failed.");
    }
}

MYQuoteData::~MYQuoteData()
{
    if (interface_) delete ((MYLTSMDL2 *) interface_);
}
