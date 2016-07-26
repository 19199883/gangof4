#include "quote_interface_option_lts.h"

#include <string>
#include <boost/thread.hpp>

#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "quote_lts.h"
#include "qtm_with_code.h"

using namespace std;
using namespace my_cmn;

static void InitOnce()
{
    static volatile bool s_have_init = false;
    static boost::mutex s_init_sync;

    if (s_have_init)
    {
        return;
    }
    else
    {
        boost::mutex::scoped_lock lock(s_init_sync);
        if (s_have_init)
        {
            return;
        }
        std::string log_file_name = "my_quote_lib_lts_" + my_cmn::GetCurrentDateTimeString();
        (void) my_log::instance(log_file_name.c_str());
        MY_LOG_INFO("start init lts quote library.");

        // initialize quote monitor
        qtm_init(TYPE_QUOTE);
        std::string qtm_name = "quote_lts_opt_" + boost::lexical_cast<std::string>(getpid());
        QuoteUpdateState(qtm_name.c_str(), QtmState::INIT);

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
    if (provider_type == QuoteProviderType::PROVIDERTYPE_LTS)
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
    interface_ = new MYLTSDataHandler(subscribe_contracts, cfg);

    return true;
}

/// 个股期权行情
void MYQuoteData::SetQuoteDataHandler(boost::function<void(const T_OptionQuote *)> quote_handler)
{
    if (interface_)
    {
        ((MYLTSDataHandler *) interface_)->SetQuoteDataHandler(quote_handler);
    }
    else
    {
        MY_LOG_ERROR("set option quote data handler failed.");
    }
}

MYQuoteData::~MYQuoteData()
{
    if (interface_) delete ((MYLTSDataHandler *) interface_);
}
