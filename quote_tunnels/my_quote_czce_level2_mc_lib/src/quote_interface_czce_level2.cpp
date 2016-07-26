#include "quote_interface_czce_level2.h"

#include <string>
#include <boost/thread.hpp>

#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "czce_multicast_quote.h"

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

    std::string qtm_name = "zce_level2_mc_" + boost::lexical_cast<std::string>(getpid());
    QuoteUpdateState(qtm_name.c_str(), QtmState::INIT);

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
    interface_ = new MYLTSMDL2(subscribe_contracts, cfg);

    // 连接服务
    MY_LOG_INFO("prepare to start czce multicast level2 quote source data transfer");

    return true;
}

void MYQuoteData::SetQuoteDataHandler(boost::function<void(const ZCEL2QuotSnapshotField_MY*)> quote_handler)
{
    if (interface_)
    {
        ((MYLTSMDL2 *) interface_)->SetQuoteDataHandler(quote_handler);
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
        ((MYLTSMDL2 *) interface_)->SetQuoteDataHandler(quote_handler);
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
        delete ((MYLTSMDL2 *) interface_);
    }
    interface_ = NULL;
}
