/*
 * 		quote_interface_tap.cpp
 *
 * 		the implement of quote_interface_tap.h
 *
 * 		Copyright(c) 2007-2015, by MY Capital Inc.
 */

#include "quote_interface_tap.h"

#include <string>
#include <thread>
#include <mutex>

#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "quote_tap.h"

using namespace std;
using namespace my_cmn;
using namespace TAP;
void
InitOnce()
{
    static volatile bool s_have_init = false;
    static std::mutex s_init_sync;

    std::lock_guard<std::mutex> lock(s_init_sync);

    if (s_have_init)
    {
        return;
    }
    else
    {
        if (s_have_init)
        {
            return;
        }
        else
        {
            std::string log_file_name = "my_quote_lib_" + my_cmn::GetCurrentDateTimeString();

            (void) my_log::instance(log_file_name.c_str());
            MY_LOG_INFO("start init quote library.");

            s_have_init = true;
        }
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

    MY_LOG_INFO("create MYQuoteData object with configure file: %s.", provider_config_file.c_str());

    ConfigData cfg;
    cfg.Load(provider_config_file);

    int provider_type = cfg.App_cfg().provider_type;
    if (provider_type == QuoteProviderType::PROVIDERTYPE_TAP)
    {
        InitInterface(subscribe_contracts, cfg);
    }
    else
    {
        MY_LOG_ERROR("not support quote provider type(%d), please check config file.", provider_type);
    }
}

bool
MYQuoteData::InitInterface(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
{
    /* 连接服务 */
    MY_LOG_INFO("prepare to start tap quote source data transfer");
    interface_ = new MYTAPDataHandler(subscribe_contracts, cfg);

    return true;
}

void
MYQuoteData::SetQuoteDataHandler(std::function<void(const TapAPIQuoteWhole_MY *)> quote_handler)
{
    if (interface_)
    {
        ((MYTAPDataHandler*) interface_)->SetQuoteDataHandler(quote_handler);
    }
    else
    {
        MY_LOG_ERROR("TapAPIContract_MY quote handler function not match quote provider.");
    }
}

MYQuoteData::~MYQuoteData()
{
    if (interface_)
    {
        delete ((MYTAPDataHandler*) interface_);
    }
    else
    {
        MY_LOG_ERROR("not support quote provider type(%d) for destroy interface.", quote_provider_type_);
    }
    interface_ = NULL;
}
