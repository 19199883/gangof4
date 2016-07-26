#include "quote_interface_czce_level2.h"

#include <string>
#include <boost/thread.hpp>

#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"
#include "quote_czce.h"
#include "quote_czce_multi.h"

using namespace std;
using namespace my_cmn;

void InitOnce()
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
        std::string log_file_name = "my_quote_lib_" + my_cmn::GetCurrentDateTimeString();
        (void) my_log::instance(log_file_name.c_str());
        MY_LOG_INFO("start init quote library.");

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
    quote_provider_type_ = cfg.App_cfg().provider_type;
    InitInterface(subscribe_contracts, cfg);
}

bool MYQuoteData::InitInterface(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
{
    switch (quote_provider_type_)
    {
        case QuoteProviderType::PROVIDERTYPE_CZCE:
            interface_ = new MYCZCEDataHandler(subscribe_contracts, cfg);
            break;
        case QuoteProviderType::PROVIDERTYPE_CZCE_MULTI:
            interface_ = new CZCE_Multi_Source(subscribe_contracts, cfg);
            break;
        default:
            MY_LOG_ERROR("not support quote provider type(%d), please check config file.", quote_provider_type_);
            break;
    }

    // 连接服务
    MY_LOG_INFO("prepare to start czce level2 quote source data transfer");

    return true;
}

void MYQuoteData::SetQuoteDataHandler(boost::function<void(const ZCEL2QuotSnapshotField_MY*)> quote_handler)
{
    if (interface_)
    {
        switch (quote_provider_type_)
        {
            case QuoteProviderType::PROVIDERTYPE_CZCE:
                ((MYCZCEDataHandler *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            case QuoteProviderType::PROVIDERTYPE_CZCE_MULTI:
                ((CZCE_Multi_Source *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            default:
                MY_LOG_ERROR("not support quote provider type(%d) for SetQuoteDataHandler.", quote_provider_type_);
                break;
        }
    }
    else
    {
        MY_LOG_ERROR("ZCEL2QuotSnapshotField_MY quote handler function not match quote provider.");
    }
}

void MYQuoteData::SetQuoteDataHandler(boost::function<void(const ZCEQuotCMBQuotField_MY*)> quote_handler)
{
    if (interface_)
    {
        switch (quote_provider_type_)
        {
            case QuoteProviderType::PROVIDERTYPE_CZCE:
                ((MYCZCEDataHandler *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            case QuoteProviderType::PROVIDERTYPE_CZCE_MULTI:
                ((CZCE_Multi_Source *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            default:
                MY_LOG_ERROR("not support quote provider type(%d) for SetQuoteDataHandler.", quote_provider_type_);
                break;
        }
    }
    else
    {
        MY_LOG_ERROR("ZCEQuotCMBQuotField_MY quote handler function not match quote provider.");
    }
}

MYQuoteData::~MYQuoteData()
{
    if (interface_)
    {
        switch (quote_provider_type_)
        {
            case QuoteProviderType::PROVIDERTYPE_CZCE:
                delete ((MYCZCEDataHandler *) interface_);
                break;
            case QuoteProviderType::PROVIDERTYPE_CZCE_MULTI:
                delete ((CZCE_Multi_Source *) interface_);
                break;
            default:
                MY_LOG_ERROR("not support quote provider type(%d) for destroy interface.", quote_provider_type_);
                break;
        }
    }
    interface_ = NULL;
}
