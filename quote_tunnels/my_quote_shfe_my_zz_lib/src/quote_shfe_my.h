#ifndef MY_SHFE_MD_QUOTE_INTERFACE_H_
#define MY_SHFE_MD_QUOTE_INTERFACE_H_

#include <sys/types.h>
#include <stdint.h>
#include <dirent.h>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include "my_cmn_util_funcs.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include "quote_datatype_shfe_my.h"
#include "quote_datatype_shfe_deep.h"
#include "quote_datatype_level1.h"

#include "quote_interface_shfe_my.h"

#include "shfe_my_data_manager.h"

#include <set>
struct DepthMarketDataID
{
    char time_code[40];
    int update_milisecond;

    DepthMarketDataID(const CDepthMarketDataField &md)
    {
        memcpy(time_code, md.UpdateTime, 9);        // char    UpdateTime[9];
        memcpy(time_code + 9, md.InstrumentID, 31); // char   InstrumentID[31];
        update_milisecond = md.UpdateMillisec;
    }

    bool operator==(const DepthMarketDataID &rhs) const
        {
        return update_milisecond == rhs.update_milisecond
            && memcmp(time_code, rhs.time_code, 40) == 0;
    }
    bool operator<(const DepthMarketDataID &rhs) const
        {
        return update_milisecond < rhs.update_milisecond
            || (update_milisecond == rhs.update_milisecond
                && memcmp(time_code, rhs.time_code, 40) < 0);
    }
};
typedef std::set<DepthMarketDataID> DepthMarketDataIDCollection;

#define DIRECT_SAVE_BUFFER_LEN 1024000
class QuoteDataSaveDirect
{
public:
    QuoteDataSaveDirect(const std::string &file_name_prefix)
    {
        buffer_ = new unsigned char[DIRECT_SAVE_BUFFER_LEN];
        buf_used_len_ = 0;
        running_flag_ = true;
        pthread_save_ = 0;
        pthread_mutex_init(&save_sync_, NULL);

        pthread_create(&pthread_save_, NULL, (void * (*)(void *))SaveImp, this);

        std::string quote_name = file_name_prefix;
        if (!quote_name.empty())
        {
            quote_name.append("_");
        }

        quote_file_ = NULL;

        // 文件存储
        char cur_work_dir[256];
        getcwd(cur_work_dir, sizeof(cur_work_dir));
        std::string full_path = std::string(cur_work_dir) + "/Data";

        if (NULL == opendir(full_path.c_str()))
        {
            mkdir(full_path.c_str(), 0775);
        }

        //Create output file
        if (NULL != opendir(full_path.c_str()))
        {
            std::string str = full_path + "/" + quote_name + my_cmn::GetCurrentDateTimeString() + ".dat";
            quote_file_ = fopen(str.c_str(), "wb+");

            if (quote_file_)
            {
                MY_LOG_INFO("file <%s> open success.", str.c_str());
            }
            else
            {
                MY_LOG_ERROR("file <%s> open failed.", str.c_str());
            }
        }
    }

    ~QuoteDataSaveDirect()
    {
        // 把任务做完
        while (buf_used_len_ > 0)
        {
            usleep(30 * 1000);
        }

        running_flag_ = false;

        int ret = pthread_cancel(pthread_save_);
        if (ret)
        {
            MY_LOG_ERROR("pthread_cancel save thread failed.");
        }

        ret = pthread_join(pthread_save_, NULL);
        if (ret)
        {
            MY_LOG_ERROR("pthread_join save thread failed.");
        }

        if (quote_file_)
        {
            fflush(quote_file_);
            fclose(quote_file_);
        }

        pthread_mutex_destroy(&save_sync_);
    }

    /**
     * QuoteData Handler
     */
    void OnQuoteData(int64_t t, void *buf, int len)
    {
        if (!quote_file_ || !buf || len == 0)
        {
            MY_LOG_ERROR("QuoteDataSaveDirect::OnQuoteData error");
            return;
        }

//        fwrite(&t, sizeof(t), 1, quote_file_);
//        fwrite(&len, sizeof(len), 1, quote_file_);
//        fwrite(buf, 1, len, quote_file_);

        MYMutexGuard lock(save_sync_);
        if (DIRECT_SAVE_BUFFER_LEN < (buf_used_len_ + sizeof(t) + sizeof(len) + len))
        {
            MY_LOG_ERROR("no buffer for cache the message, abandon one.");
        }
        memcpy(buffer_ + buf_used_len_, &t, sizeof(t));
        buf_used_len_ += sizeof(t);
        memcpy(buffer_ + buf_used_len_, &len, sizeof(len));
        buf_used_len_ += sizeof(len);
        memcpy(buffer_ + buf_used_len_, buf, len);
        buf_used_len_ += len;
        if (buf_used_len_ > DIRECT_SAVE_BUFFER_LEN / 2)
        {
            MY_LOG_WARN("buffer used %d, more than half expected size", buf_used_len_);
        }
    }

private:
    // 文件存储
    FILE * quote_file_;

    unsigned char *buffer_;
    std::size_t buf_used_len_;
    volatile bool running_flag_;
    pthread_t pthread_save_;
    pthread_mutex_t save_sync_;

    static void * SaveImp(void *pv)
    {
        QuoteDataSaveDirect *p = (QuoteDataSaveDirect *) pv;
        MY_LOG_DEBUG("save thread started.");
        unsigned char *buffer_t = new unsigned char[DIRECT_SAVE_BUFFER_LEN];
        std::size_t buf_used_len_t = 0;

        while (p->running_flag_)
        {
            // 将数据交换到本地
            {
                MYMutexGuard lock(p->save_sync_);
                if (p->buf_used_len_ > 0)
                {
                    unsigned char * t = p->buffer_;
                    p->buffer_ = buffer_t;
                    buffer_t = t;
                    buf_used_len_t = p->buf_used_len_;
                    p->buf_used_len_ = 0;
                }
            }

            // 没事做，让出CPU
            if (buf_used_len_t == 0)
            {
                usleep(50 * 1000);
                continue;
            }

            // 自己慢慢存
            try
            {
                fwrite(buffer_t, 1, buf_used_len_t, p->quote_file_);
            }
            catch (...)
            {
                MY_LOG_ERROR("unknown error in SaveImp when save quote data.");
            }

            // 存完清空
            buf_used_len_t = 0;
        }

        return NULL;
    }
};

class QuoteInterface_MY_SHFE_MD: public MYShfeMDHandler
{
public:
    // 构造函数
    QuoteInterface_MY_SHFE_MD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const SHFEQuote *)> quote_data_handler)
    {
        shfe_deep_data_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const CDepthMarketDataField *)> quote_data_handler)
    {
        shfe_ex_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const MYShfeMarketData *)> quote_data_handler)
    {
        my_shfe_md_handler_ = quote_data_handler;
    }

    virtual ~QuoteInterface_MY_SHFE_MD();

private:
    // quote data handlers
    void OnMYShfeMDData(MYShfeMarketData *data);
    void ShfeDepthMarketDataHandler();
    void ShfeMBLHandler();
    void ShfeDataHandler();

    int CreateUdpFD(const std::string &addr_ip, unsigned short port);
    bool ParseMarketData(char* buffer, int nCount, QuoteDataSaveDirect *p_save_direct);

    // 数据处理函数对象
    boost::function<void(const SHFEQuote *)> shfe_deep_data_handler_;
    boost::function<void(const CDepthMarketDataField *)> shfe_ex_handler_;
    boost::function<void(const MYShfeMarketData *)> my_shfe_md_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    char qtm_name_[32];

    // save assistant object
    QuoteDataSave<SHFEQuote> *p_shfe_deep_save_;
    QuoteDataSave<CDepthMarketDataField> *p_shfe_ex_save_;
    QuoteDataSave<MYShfeMarketData> *p_my_shfe_md_save_;

    // receive threads
    volatile bool running_flag_;
    boost::thread *p_mbl_handler_;
    boost::thread *p_shfe_depth_md_handler_;
    boost::thread *p_data_handler_;

    MYShfeMDManager my_shfe_md_inf_;

    // id collection for filter duplicate depth md
    DepthMarketDataIDCollection md_ids_;

    // udp bind state
    boost::atomic<int> udp_bind_cnt_;
};

#endif
