#include "quote_cffex_multi.h"
#include "my_cmn_util_funcs.h"
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

std::string ToStringID(const CFfexFtdcDepthMarketData &md)
{
    // char szUpdateTime[9];        ///< 最后修改时间
    // char szInstrumentID[31];     ///< 合约代码
    char buff[64];
    sprintf(buff, "%s%03d%s", md.szUpdateTime, md.nUpdateMillisec, md.szInstrumentID);
    return buff;
}

MDCffexMultiResouce::MDCffexMultiResouce(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : p_save_(NULL)
{
    filter_flag = cfg.Transfer_policy().filter_duplicate_flag;

    sprintf(qtm_name_, "cffex_multiresource_%s_%u", cfg.Logon_config().account.c_str(), getpid());
    QuoteUpdateState(qtm_name_, QtmState::INIT);

    p_save_ = new QuoteDataSave<CFfexFtdcDepthMarketData>(cfg, qtm_name_, "cffex_level2", GTA_UDP_CFFEX_QUOTE_TYPE);

    for (const std::string &cfg_file : cfg.Quote_cfg_files())
    {
        ConfigData *cfg = new ConfigData();
        cfg->Load(cfg_file);
        int provider_type = cfg->App_cfg().provider_type;
        if (provider_type == QuoteProviderType::PROVIDERTYPE_MYFPGA)
        {
            MYFPGACffexDataHandler * t = new MYFPGACffexDataHandler(subscribe_contracts, *cfg);
            t->SetQuoteDataHandler(boost::bind(&MDCffexMultiResouce::DataHandler, this, _1));
            my_fpga_interfaces_.push_back(t);
        }
        else if (provider_type == QuoteProviderType::PROVIDERTYPE_NET_XELE)
        {
            NetXeleDataHandler * t = new NetXeleDataHandler(subscribe_contracts, *cfg);
            t->SetQuoteDataHandler(boost::bind(&MDCffexMultiResouce::DataHandler, this, _1));
            net_xele_interfaces_.push_back(t);
        }
        else if (provider_type == QuoteProviderType::PROVIDERTYPE_LOC_XELE)
        {
            LocXeleDataHandler * t = new LocXeleDataHandler(subscribe_contracts, *cfg);
            t->SetQuoteDataHandler(boost::bind(&MDCffexMultiResouce::DataHandler, this, _1));
            loc_xele_interfaces_.push_back(t);
        }
        else if (provider_type == QuoteProviderType::PROVIDERTYPE_FEMAS)
        {
            MYFEMASDataHandler * t = new MYFEMASDataHandler(subscribe_contracts, *cfg);
            t->SetQuoteDataHandler(boost::bind(&MDCffexMultiResouce::DataHandler, this, _1));
            femas_interfaces_.push_back(t);
        }
        else if (provider_type == QuoteProviderType::PROVIDERTYPE_FEMAS_MULTICAST)
        {
            FemasMulticastMDHandler * t = new FemasMulticastMDHandler(subscribe_contracts, *cfg);
            t->SetQuoteDataHandler(boost::bind(&MDCffexMultiResouce::DataHandler, this, _1));
            femas_mc_interfaces_.push_back(t);
        }
        else
        {
            MY_LOG_ERROR("not support quote provider type for cffex multi, configure file:%s, provider type:%d.", cfg_file.c_str(), provider_type);
        }
    }
    QuoteUpdateState(qtm_name_, QtmState::API_READY);
}

MDCffexMultiResouce::~MDCffexMultiResouce()
{
for (NetXeleDataHandler * p : net_xele_interfaces_)
{
    delete p;
}

for (LocXeleDataHandler * p : loc_xele_interfaces_)
{
    delete p;
}

for (MYFEMASDataHandler * p : femas_interfaces_)
{
    delete p;
}

for (FemasMulticastMDHandler * p : femas_mc_interfaces_)
{
    delete p;
}

for (MYFPGACffexDataHandler * p : my_fpga_interfaces_)
{
    delete p;
}

if (p_save_)
    delete p_save_;
}

void MDCffexMultiResouce::DataHandler(const CFfexFtdcDepthMarketData *data)
{
timeval t;
{
    boost::mutex::scoped_lock lock(ids_sync_);
    if (filter_flag)
    {
        std::string id = ToStringID(*data);
        if (ids_.find(id) != ids_.end())
        {
            return;
        }
        ids_.insert(id);
    }

    gettimeofday(&t, NULL);
    if (quote_data_handler_)
    {
        quote_data_handler_(data);
    }
}

// 存起来
p_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, data);
}

void MDCffexMultiResouce::SetQuoteDataHandler(boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler)
{
quote_data_handler_ = quote_data_handler;
}
