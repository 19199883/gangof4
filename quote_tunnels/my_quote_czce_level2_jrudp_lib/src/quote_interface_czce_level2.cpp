#include "quote_interface_czce_level2.h"

#include <string>
#include <thread>

#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

#include "quote_czce_udp.h"

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
        std::string log_file_name = "my_quote_lib_" + my_cmn::GetCurrentDateTimeString();
        (void) my_log::instance(log_file_name.c_str());
        MY_LOG_INFO("start init quote library.");

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

    InitInterface(subscribe_contracts, cfg);
}

bool MYQuoteData::InitInterface(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
{
    // 连接服务
    MY_LOG_INFO("prepare to start czce level2 quote source data transfer");
    interface_ = new CzceUdpMD(subscribe_contracts, cfg);

    return true;
}

void MYQuoteData::SetQuoteDataHandler(std::function<void(const ZCEL2QuotSnapshotField_MY*)> quote_handler)
{
    if (interface_)
    {
        ((CzceUdpMD *) interface_)->SetQuoteDataHandler(quote_handler);
    }
    else
    {
        MY_LOG_ERROR("ZCEL2QuotSnapshotField_MY quote handler function not match quote provider.");
    }
}

void MYQuoteData::SetQuoteDataHandler(std::function<void(const ZCEQuotCMBQuotField_MY*)> quote_handler)
{
    if (interface_)
    {
//        ((DceUdpMD *) interface_)->SetQuoteDataHandler(quote_handler);
    }
    else
    {
        MY_LOG_ERROR("ZCEQuotCMBQuotField_MY quote handler function not match quote provider.");
    }
}

MYQuoteData::~MYQuoteData()
{
    if (interface_) delete ((CzceUdpMD *) interface_);
}
