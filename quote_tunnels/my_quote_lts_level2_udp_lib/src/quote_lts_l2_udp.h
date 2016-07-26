#pragma once

#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <list>
#include "SecurityFtdcUserApiStruct.h"

#include <boost/function.hpp>
#include "Level2BroadCast.h"
#include "quote_interface_lts_level2.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"
#include "query_lts.h"

struct IdxCacheData
{
    int nOpenIndex;		//今开盘指数
    int nHighIndex;		//最高指数
    int nLowIndex;		//最低指数
    int nLastIndex;		//最新指数
    long long iTotalVolume;	//参与计算相应指数的交易数量
    long long iTurnover;		//参与计算相应指数的成交金额

    IdxCacheData(const CL2FAST_INDEX & idx)
    {
        nOpenIndex = (int) (InvalidToZeroD(idx.OpenIndex) * 10000 + 0.5);         //今开盘指数
        nHighIndex = (int) (InvalidToZeroD(idx.HighIndex) * 10000 + 0.5);         //最高指数
        nLowIndex = (int) (InvalidToZeroD(idx.LowIndex) * 10000 + 0.5);           //最低指数
        nLastIndex = (int) (InvalidToZeroD(idx.LastIndex) * 10000 + 0.5);         //最新指数
        iTotalVolume = (long long) (InvalidToZeroD(idx.TotalVolume));             //参与计算相应指数的交易数量
        iTurnover = (long long) (InvalidToZeroD(idx.TurnOver) * 10000 + 0.5);     //参与计算相应指数的成交金额
    }

    void Update(const TDF_INDEX_DATA_MY &md)
    {
        nOpenIndex = md.nOpenIndex;
        nHighIndex = md.nHighIndex;
        nLowIndex = md.nLowIndex;
        nLastIndex = md.nLastIndex;
        iTotalVolume = md.iTotalVolume;
        iTurnover = md.iTurnover;
    }
};

typedef std::unordered_map<std::string, IdxCacheData> IdxCacheMap;

class MYLTSMDL2
{
public:
    // 构造函数
    MYLTSMDL2(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);
    virtual ~MYLTSMDL2();

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const TDF_MARKET_DATA_MY *)> handler)
    {
        md_l2_handler_ = handler;
    }
    void SetQuoteDataHandler(boost::function<void(const TDF_INDEX_DATA_MY *)> handler)
    {
        md_idx_handler_ = handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_PerEntrust *)> handler)
    {
        per_entrust_handler_ = handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_PerBargain *)> handler)
    {
        per_bargain_handler_ = handler;
    }

private:
    // stocks/indices for subsribe, queried by query interface
    MYMDStaticInfo static_info_of_stocks_;

    volatile bool run_flag_;
    int current_date;

    // 配置数据对象
    ConfigData cfg_;

    int CreateUdpFD(const std::string& addr_ip, unsigned short port);
    void StartRecv();

    // recv thread and sync var
    boost::thread *recv_thread_;

    // 数据处理函数对象
    boost::function<void(const TDF_MARKET_DATA_MY *)> md_l2_handler_;
    boost::function<void(const TDF_INDEX_DATA_MY *)> md_idx_handler_;
    boost::function<void(const T_PerEntrust *)> per_entrust_handler_;
    boost::function<void(const T_PerBargain *)> per_bargain_handler_;

    // data convertors
    const CSecurityFtdcMarketDataStaticInfoField * GetMDStaticInfo(const char *ex_id, const char *code);
    TDF_MARKET_DATA_MY Convert(const CL2FAST_MD &pd);
    bool Convert(const CL2FAST_INDEX &pd, TDF_INDEX_DATA_MY &md);
    T_PerEntrust Convert(const CL2FAST_ORDER &pd);
    T_PerBargain Convert(const CL2FAST_TRADE &pd);

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // save assistant object
    QuoteDataSave<TDF_MARKET_DATA_MY> *p_md_l2_save_;
    QuoteDataSave<TDF_INDEX_DATA_MY> *p_md_idx_save_;
    QuoteDataSave<T_PerEntrust> *p_per_entrust_save_;
    QuoteDataSave<T_PerBargain> *p_per_bargain_save_;

    // variables and functions for cache index data and filter process
    IdxCacheMap idx_cache_md_;

    std::string qtm_name;
};
