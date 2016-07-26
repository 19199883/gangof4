#include "quote_shfe_my.h"
#include "quote_cmn_utility.h"

#include <fstream>

using namespace std;
using namespace my_cmn;

#define ST_NORMAL 0
#define ST_NET_DISCONNECTED -1
#define ST_LOGIN_FAILED -2
#define ST_LOGOUT -3

QuoteInterface_MY_SHFE_MD::QuoteInterface_MY_SHFE_MD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_shfe_deep_save_(NULL), p_shfe_ex_save_(NULL), p_my_shfe_md_save_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    if (subscribe_contracts_.empty())
    {
        MY_LOG_INFO("MY_SHFE_MD - subscribe all contract");
    }
    else
    {
        for (const std::string &v : subscribe_contracts_)
        {
            MY_LOG_INFO("MY_SHFE_MD - subscribe: %s", v.c_str());
        }
    }

    running_flag_ = true;

    p_shfe_deep_save_ = new QuoteDataSave<SHFEQuote>(cfg_, "", "shfe_deep", SHFE_DEEP_QUOTE_TYPE, false);
    p_shfe_ex_save_ = new QuoteDataSave<CDepthMarketDataField>(cfg_, "", "quote_level1", SHFE_EX_QUOTE_TYPE, false);
    p_my_shfe_md_save_ = new QuoteDataSave<MYShfeMarketData>(cfg_, "", "my_shfe_md", MY_SHFE_MD_QUOTE_TYPE, false);

    md_file_name = cfg_.Logon_config().broker_id;
    wait_seconds = atoi(cfg_.Logon_config().account.c_str());
    if (wait_seconds < 5) wait_seconds = 60;
    interval_us = atoi(cfg_.Logon_config().trade_server_addr.c_str());
    if (interval_us < 10) interval_us = 100;

    boost::thread(boost::bind(&QuoteInterface_MY_SHFE_MD::Replay, this));
}

QuoteInterface_MY_SHFE_MD::~QuoteInterface_MY_SHFE_MD()
{
    // terminate all threads
    running_flag_ = false;

    // destroy all save object
    if (p_shfe_deep_save_)
        delete p_shfe_deep_save_;

    if (p_shfe_ex_save_)
        delete p_shfe_ex_save_;

    if (p_my_shfe_md_save_)
        delete p_my_shfe_md_save_;
}

void QuoteInterface_MY_SHFE_MD::OnMYShfeMDData(MYShfeMarketData *data)
{
    timeval t;
    gettimeofday(&t, NULL);
    if (my_shfe_md_handler_
        && (subscribe_contracts_.empty()
            || subscribe_contracts_.find(data->InstrumentID) != subscribe_contracts_.end()))
    {
        my_shfe_md_handler_(data);
    }
    p_my_shfe_md_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, data);
}

void QuoteInterface_MY_SHFE_MD::Replay()
{
    // open md file
    ifstream f_in(md_file_name.c_str(), std::ios_base::binary);
    SaveFileHeaderStruct f_header;
    f_header.data_count = 0;
    f_header.data_length = 0;
    f_header.data_type = 0;

    f_in.read((char *) &f_header, sizeof(f_header));

    // wait some seconds, make sure trader started
    sleep(wait_seconds);

    // begin replay
    SaveDataStruct<MYShfeMarketData> d;
    if (f_header.data_length != (short)sizeof(d))
    {
        MY_LOG_INFO("data file format error,data_length:%d, sizeof:%d", f_header.data_length, (short)sizeof(d));
        cout << "format error of data file\n";
    }
    for (int i = 0; i < f_header.data_count; ++i)
    {
        f_in.read((char *)&d, sizeof(d));
        MYShfeMarketData *p = &d.data_;

        OnMYShfeMDData(p);
        usleep(100);
    }
}
