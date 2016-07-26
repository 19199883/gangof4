#if !defined(QUOTE_CMN_UTILITY_H_)
#define QUOTE_CMN_UTILITY_H_

#include <pthread.h>
#include <string>
#include <vector>
#include <set>
#include <float.h>

#include <boost/date_time.hpp>

#include "my_cmn_log.h"
#include "my_cmn_util_funcs.h"
#include "qtm_with_code.h"

#ifndef DLL_PUBLIC
#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

#define MAX_PURE_DBL          (double)9007199254740991.0
#define MIN_PURE_DBL          (double)-9007199254740991.0
#define MAX_PURE_FLT          (double)16777215
#define MIN_PURE_FLT          (double)-16777215
inline bool IsValidDouble(double lfValue)
{
    return ((lfValue > MIN_PURE_DBL) && (lfValue < MAX_PURE_DBL));
}

inline bool IsValidFloat(float lfValue)
{
    return ((lfValue > MIN_PURE_FLT) && (lfValue < MAX_PURE_FLT));
}

inline double InvalidToZeroD(double dVal)
{
    return IsValidDouble(dVal) ? dVal : 0.0;
}

inline float InvalidToZeroF(float fVal)
{
    return IsValidFloat(fVal) ? fVal : 0.0;
}

#define MD_LOG_DEBUG(format, args...) my_cmn::my_log::instance(NULL)->log(my_cmn::LOG_MOD_QUOTE, my_cmn::LOG_PRI_DEBUG, format, ##args)
#define MD_LOG_INFO(format, args...) my_cmn::my_log::instance(NULL)->log(my_cmn::LOG_MOD_QUOTE, my_cmn::LOG_PRI_INFO, format, ##args)
#define MD_LOG_WARN(format, args...) my_cmn::my_log::instance(NULL)->log(my_cmn::LOG_MOD_QUOTE, my_cmn::LOG_PRI_WARN, format, ##args)
#define MD_LOG_ERROR(format, args...) my_cmn::my_log::instance(NULL)->log(my_cmn::LOG_MOD_QUOTE, my_cmn::LOG_PRI_ERROR, format, ##args)
#define MD_LOG_FATAL(format, args...) my_cmn::my_log::instance(NULL)->log(my_cmn::LOG_MOD_QUOTE, my_cmn::LOG_PRI_CRIT, format, ##args)

//#define MD_LOG_DEBUG(format, args...)
//#define MD_LOG_INFO(format, args...)
//#define MD_LOG_WARN(format, args...)
//#define MD_LOG_ERROR(format, args...)
//#define MD_LOG_FATAL(format, args...)

namespace MY_QUOTE_CONST
{
const std::string SUBSCRIBE_ALL = "All"; // 订阅全部

const std::string EXCHCODE_SHFE = "A";  //上海期货
const std::string EXCHCODE_DCE = "B";  //大连期货
const std::string EXCHCODE_CZCE = "C";  //郑州期货
const std::string EXCHCODE_CFFEX = "G";  //中金所
const std::string EXCHCODE_SZEX = "0";  //深交所
const std::string EXCHCODE_SHEX = "1";  //上交所
const std::string EXCHCODE_SH_GOLD_EX = "2";  //上海黄金交易所
const std::string EXCHCODE_NULL = " ";
const std::string EXCH_NAME_SHFE = "SHFE";
const std::string EXCH_NAME_DCE = "DCE";
const std::string EXCH_NAME_CZCE = "CZCE";
const std::string EXCH_NAME_CFFEX = "CFFEX";
const std::string EXCH_NAME_NULL = " ";
}

class MYMutexGuard
{
public:
    MYMutexGuard(pthread_mutex_t &sync_mutex) : sync_mutex_(sync_mutex)
    {
        pthread_mutex_lock(&sync_mutex_);
    }
    ~MYMutexGuard()
    {
        pthread_mutex_unlock(&sync_mutex_);
    }

private:
    MYMutexGuard(const MYMutexGuard&);
    MYMutexGuard &operator=(const MYMutexGuard&);

    // 互斥量
    pthread_mutex_t &sync_mutex_;
};

inline static const std::string &ExchNameToExchCode(const std::string &sExchName)
{
    if (sExchName == MY_QUOTE_CONST::EXCH_NAME_CFFEX)
    {
        return MY_QUOTE_CONST::EXCHCODE_CFFEX;
    }
    else if (sExchName == MY_QUOTE_CONST::EXCH_NAME_SHFE)
    {
        return MY_QUOTE_CONST::EXCHCODE_SHFE;
    }
    else if (sExchName == MY_QUOTE_CONST::EXCH_NAME_DCE)
    {
        return MY_QUOTE_CONST::EXCHCODE_DCE;
    }
    else if (sExchName == MY_QUOTE_CONST::EXCH_NAME_CZCE)
    {
        return MY_QUOTE_CONST::EXCHCODE_CZCE;
    }
    else
    {
        return sExchName;
    }
}

template<typename DataType>
void MYUTIL_UpdateQuoteTime(const std::string &qtm_name, const std::vector<DataType> &datas)
{
    for (typename std::vector<DataType>::const_iterator it = datas.begin(); it != datas.end(); ++it)
    {
        std::string quote_time = it->data_.GetQuoteTime();
        if (quote_time.empty())
        {
        	acquire_quote_time_field(qtm_name.c_str(), my_cmn::GetCurrentTimeWithMilisecString().c_str());
        }
        else if(quote_time.size() == 12)
        {
        	acquire_quote_time_field(qtm_name.c_str(), quote_time.c_str());
        }
    }
}


template<typename DataType>
void MYUTIL_SaveDataToFile(const std::vector<DataType> &datas, int &data_count, FILE * pf)
{
    if (pf && !datas.empty())
    {
        fwrite(&(datas[0]), sizeof(DataType), datas.size(), pf);
        data_count += datas.size();
        fseek(pf, 0, SEEK_SET);
        fwrite(&data_count, sizeof(data_count), 1, pf);
        fseek(pf, 0, SEEK_END);
        fflush(pf);
    }
}

template<typename DataType, typename HeaderType>
void MYUTIL_SaveFileHeader(int data_type, FILE * pf)
{
    if (pf)
    {
        HeaderType header;
        header.data_count = 0;
        header.data_type = short(data_type);
        header.data_length = (short) (sizeof(DataType));
        fwrite(&header, sizeof(HeaderType), 1, pf);
    }
}

typedef std::pair<std::string, unsigned short> IPAndPortNum;
IPAndPortNum ParseIPAndPortNum(const std::string &addr_cfg);

typedef std::pair<std::string, std::string> IPAndPortStr;
IPAndPortStr ParseIPAndPortStr(const std::string &addr_cfg);

void QuoteUpdateState(const char *name, int s);

#endif  //
