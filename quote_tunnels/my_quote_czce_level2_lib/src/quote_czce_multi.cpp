#include "quote_czce_multi.h"
#include "my_cmn_util_funcs.h"
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

using namespace my_cmn;

CZCE_Multi_Source::CZCE_Multi_Source(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : p_save_(NULL)
{
    filter_flag = cfg.Transfer_policy().filter_duplicate_flag;
    qtm_name = "zce_level2_multi";

    p_save_ = new QuoteDataSave<ZCEL2QuotSnapshotField_MY>(cfg, qtm_name.c_str(), "czce_level2", CZCE_LEVEL2_QUOTE_TYPE);
    p_save_cmb_ = new QuoteDataSave<ZCEQuotCMBQuotField_MY>(cfg, qtm_name.c_str(), "czce_cmb", CZCE_CMB_QUOTE_TYPE);
    cur_used_line_idx_ = -1;
    line_count_ = 0;
    p_md_c = NULL;

    bool is_first_line = true;
    for (const std::string &cfg_file : cfg.Quote_cfg_files())
    {
        ConfigData *cfg = new ConfigData();
        cfg->Load(cfg_file);
        int provider_type = cfg->App_cfg().provider_type;
        if (provider_type == QuoteProviderType::PROVIDERTYPE_CZCE)
        {
            MYCZCEDataHandler * t = new MYCZCEDataHandler(subscribe_contracts, *cfg);

            boost::function<void(const ZCEL2QuotSnapshotField_MY*)> quote_handler = boost::bind(&CZCE_Multi_Source::DataHandler,
                this, _1);
            t->SetQuoteDataHandler(quote_handler);

            boost::function<void(const ZCEQuotCMBQuotField_MY*)> cmb_handler = boost::bind(&CZCE_Multi_Source::CMBDataHandler, this,
                _1);
            t->SetQuoteDataHandler(cmb_handler);

            if (is_first_line)
            {
                is_first_line = false;
                t->SetPublish();
                cur_used_line_idx_ = 0;
            }
            else
            {
                t->SetUnPublish();
            }

            cece_level2_interfaces_.push_back(t);
            recv_data_flag_.push_back(0);
            ++line_count_;
        }
        else
        {
            MY_LOG_ERROR("not support quote provider type for czce multi, configure file:%s, provider type:%d.",
                cfg_file.c_str(),
                provider_type);
        }
    }

    // start md monitor thread
    MY_LOG_INFO("md lines number:%d, used:%d", line_count_, cur_used_line_idx_);
    p_md_c = new boost::thread(&CZCE_Multi_Source::RecvDataCheck, this);
}

void CZCE_Multi_Source::SetQuoteDataHandler(boost::function<void(const ZCEL2QuotSnapshotField_MY*)> quote_data_handler)
{
    quote_data_handler_ = quote_data_handler;
}

void CZCE_Multi_Source::SetQuoteDataHandler(boost::function<void(const ZCEQuotCMBQuotField_MY*)> quote_data_handler)
{
    cmb_data_handler_ = quote_data_handler;
}

CZCE_Multi_Source::~CZCE_Multi_Source()
{
    for (MYCZCEDataHandler * p : cece_level2_interfaces_)
    {
        delete p;
    }

    if (p_save_) delete p_save_;
    if (p_save_cmb_) delete p_save_cmb_;
}

void CZCE_Multi_Source::DataHandler(const ZCEL2QuotSnapshotField_MY* data)
{
    timeval t;
    gettimeofday(&t, NULL);
    if (quote_data_handler_)
    {
        quote_data_handler_(data);
    }

    // 存起来
    p_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, data);
}

void CZCE_Multi_Source::RecvDataCheck()
{
    if (cur_used_line_idx_ < 0 || line_count_ <= 0)
    {
        MY_LOG_WARN("no valid md line, no need check");
    }

    const int check_interval_seconds = 1;
    const int total_check_times = 3;
    bool have_data_flag = true;
    bool switch_flag = false;
    while (true)
    {
        switch_flag = false;
        if (!cece_level2_interfaces_[cur_used_line_idx_]->GetAndClearRecvState())
        {
            // consider change md line
            for (int i = 0; i < line_count_; ++i)
            {
                recv_data_flag_[i] = 0;
            }

            int check_times = total_check_times;
            while (check_times > 0)
            {
                sleep(check_interval_seconds);
                for (int i = 0; i < line_count_; ++i)
                {
                    if (cece_level2_interfaces_[i]->GetAndClearRecvState())
                    {
                        ++recv_data_flag_[i];
                    }
                }
                --check_times;
            }

            // no data on current used line
            if (recv_data_flag_[cur_used_line_idx_] == 0)
            {
                for (int i = 0; i < line_count_; ++i)
                {
                    if (have_data_flag)
                    {
                        MY_LOG_WARN("line:%d, recv cnt:%d, check_times:%d", i, recv_data_flag_[i], total_check_times);
                    }
                }
                for (int i = 0; i < line_count_; ++i)
                {
                    // switch to a good line
                    if (line_count_ != cur_used_line_idx_
                        && recv_data_flag_[i] == total_check_times)
                    {
                        MY_LOG_WARN("no data on line:%d, switch to:%d", cur_used_line_idx_, i);
                        cece_level2_interfaces_[cur_used_line_idx_]->SetUnPublish();
                        cur_used_line_idx_ = i;
                        cece_level2_interfaces_[cur_used_line_idx_]->SetPublish();

                        switch_flag = true;
                        break;
                    }
                }
            }

            if (have_data_flag)
            {
                if (!switch_flag)
                {
                    MY_LOG_WARN("no data on line:%d, and no data on other line", cur_used_line_idx_);
                }
                have_data_flag = false;
            }
        }
        else
        {
            if (!have_data_flag)
            {
                MY_LOG_WARN("recv data on line:%d", cur_used_line_idx_);
                have_data_flag = true;
            }
        }

        sleep(check_interval_seconds);
    }
}

void CZCE_Multi_Source::CMBDataHandler(const ZCEQuotCMBQuotField_MY* data)
{
    timeval t;

    gettimeofday(&t, NULL);
    if (cmb_data_handler_)
    {
        cmb_data_handler_(data);
    }

    // 存起来
    p_save_cmb_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, data);
}
