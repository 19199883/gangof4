#include "quote_interface_cffex_level2.h"

#include <string>
#include <boost/thread.hpp>

#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "qtm_with_code.h"

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "quote_cffex_multi.h"
#include "quote_femas.h"
#include "quote_femas_multicast.h"
#include "quote_net_xele.h"
#include "quote_loc_xele.h"
#include "quote_my_fpga.h"

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

        TNL_LOG_INFO("CFFEX MD Version : %s - %s\n", __DATE__, __TIME__);
    }
}

MYQuoteData::MYQuoteData(const SubscribeContracts *subscribe_contracts, const std::string &provider_config_file)
{
    interface_ = NULL;
    quote_provider_type_ = 0;

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
        case QuoteProviderType::PROVIDERTYPE_MYFPGA:
            interface_ = new MYFPGACffexDataHandler(subscribe_contracts, cfg);
            break;
        case QuoteProviderType::PROVIDERTYPE_NET_XELE:
            interface_ = new NetXeleDataHandler(subscribe_contracts, cfg);
            break;
        case QuoteProviderType::PROVIDERTYPE_LOC_XELE:
            interface_ = new LocXeleDataHandler(subscribe_contracts, cfg);
            break;
        case QuoteProviderType::PROVIDERTYPE_FEMAS:
            interface_ = new MYFEMASDataHandler(subscribe_contracts, cfg);
            break;
        case QuoteProviderType::PROVIDERTYPE_GTA_MULTI:
            interface_ = new MDCffexMultiResouce(subscribe_contracts, cfg);
            break;
        case QuoteProviderType::PROVIDERTYPE_FEMAS_MULTICAST:
            interface_ = new FemasMulticastMDHandler(subscribe_contracts, cfg);
            break;
        default:
            MY_LOG_ERROR("not support quote provider type(%d), please check config file.", quote_provider_type_);
            break;
    }

    // 连接服务
    MY_LOG_INFO("prepare to start quote source data transfer");

    return true;
}

void MYQuoteData::SetQuoteDataHandler(boost::function<void(const CFfexFtdcDepthMarketData*)> quote_handler)
{
    if (interface_)
    {
        switch (quote_provider_type_)
        {
            case QuoteProviderType::PROVIDERTYPE_MYFPGA:
                ((MYFPGACffexDataHandler *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            case QuoteProviderType::PROVIDERTYPE_GTA_MULTI:
                ((MDCffexMultiResouce *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            case QuoteProviderType::PROVIDERTYPE_FEMAS:
                ((MYFEMASDataHandler *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            case QuoteProviderType::PROVIDERTYPE_FEMAS_MULTICAST:
                ((FemasMulticastMDHandler *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            case QuoteProviderType::PROVIDERTYPE_NET_XELE:
                ((NetXeleDataHandler *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            case QuoteProviderType::PROVIDERTYPE_LOC_XELE:
                ((LocXeleDataHandler *) interface_)->SetQuoteDataHandler(quote_handler);
                break;
            default:
                MY_LOG_ERROR("not support quote provider type(%d) for SetQuoteDataHandler.", quote_provider_type_);
                break;
        }
    }
    else
    {
        MY_LOG_ERROR("cffex level2 quote handler function not match quote provider.");
    }
}

MYQuoteData::~MYQuoteData()
{
    if (interface_)
    {
        switch (quote_provider_type_)
        {
            case QuoteProviderType::PROVIDERTYPE_MYFPGA:
                delete ((MYFPGACffexDataHandler *) interface_);
                break;
            case QuoteProviderType::PROVIDERTYPE_GTA_MULTI:
                delete ((MDCffexMultiResouce *) interface_);
                break;
            case QuoteProviderType::PROVIDERTYPE_FEMAS:
                delete ((MYFEMASDataHandler *) interface_);
                break;
            case QuoteProviderType::PROVIDERTYPE_FEMAS_MULTICAST:
                delete ((FemasMulticastMDHandler *) interface_);
                break;
            case QuoteProviderType::PROVIDERTYPE_NET_XELE:
                delete ((NetXeleDataHandler *) interface_);
                break;
            case QuoteProviderType::PROVIDERTYPE_LOC_XELE:
                delete ((LocXeleDataHandler *) interface_);
                break;
            default:
                MY_LOG_ERROR("not support quote provider type(%d) for destroy interface.", quote_provider_type_);
                break;
        }
    }
    qtm_finish();
}
