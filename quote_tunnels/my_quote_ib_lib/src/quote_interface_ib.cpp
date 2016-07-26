#include "quote_interface_ib.h"
#include <string>
#include <boost/thread.hpp>

#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "qtm_with_code.h"

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"
#include "quote_ib.h"

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

    int provider_type = cfg.App_cfg().provider_type;
    list<string> ip_list = cfg.Logon_config().quote_provider_addrs;
    for (auto it = ip_list.begin(); it != ip_list.end(); it++)
    {
        if (provider_type == QuoteProviderType::PROVIDERTYPE_IB)
        {
            InitInterface(subscribe_contracts, cfg, *it);
        }
        else
        {
            MY_LOG_ERROR("not support quote provider type(%d), please check config file.", provider_type);
        }
    }
}

bool MYQuoteData::InitInterface(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg, string id)
{
    /* 连接服务 */
    MY_LOG_INFO("prepare to start quote source data transfer");
    interface_ = new MYIBDataHandler(subscribe_contracts, cfg, id);
    return true;
}

void MYQuoteData::SetQuoteDataHandler(boost::function<void(const IBDepth*)> depth_handler)
{
    if (interface_)
    {
        ((MYIBDataHandler *) interface_)->SetQuoteDataHandler(depth_handler);
    }
    else
    {
        MY_LOG_ERROR("IBMktDataDepth depth handler function not match quote provider.");
    }
}

void MYQuoteData::SetQuoteDataHandler(boost::function<void(const IBTick*)> tick_handler)
{
    if (interface_)
    {
        ((MYIBDataHandler *) interface_)->SetQuoteDataHandler(tick_handler);
    }
    else
    {
        MY_LOG_ERROR("IBMktDataDepth tick handler function not match quote provider.");
    }
}

MYQuoteData::~MYQuoteData()
{
    if (interface_)
    {
        delete ((MYIBDataHandler *) interface_);
    }
    else
    {
        MY_LOG_ERROR("not support quote provider type(%d) for destroy interface.", quote_provider_type_);
    }
    qtm_finish();
    interface_ = NULL;
}
