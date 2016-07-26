#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "ac_xele_md_struct.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"
#include "quote_datatype_shfe_my.h"
#include "quote_datatype_shfe_deep.h"
#include "quote_datatype_level1.h"
#include "quote_interface_shfe_my.h"
#include "shfe_my_data_manager.h"

struct ShfeMDAndState
{
    CDepthMarketDataField md;
    int f_flag;

    ShfeMDAndState(const char *contract)
    {
        bzero(&md, sizeof(md));
        strncpy(md.InstrumentID, contract, sizeof(md.InstrumentID));
        f_flag = 0;
    }

    inline void reset()
    {
        f_flag = 0;
    }
    inline void set_l1(const LevelOneMarketDataField &ld)
    {
        strncpy(md.UpdateTime, ld.UpdateTime, sizeof(md.UpdateTime));
        md.UpdateMillisec = ld.UpdateMillisec;
        md.Volume = ld.Volume;
        md.LastPrice = InvalidToZeroD(ld.LastPrice);
        md.Turnover = ld.Turnover;
        md.OpenInterest = ld.OpenInterest;
        md.BidPrice1 = InvalidToZeroD(ld.BidPrice);
        md.AskPrice1 = InvalidToZeroD(ld.AskPrice);
        md.BidVolume1 = ld.BidVolume;
        md.AskVolume1 = ld.AskVolume;

        f_flag |= 0x1;
    }
    inline void set_lowl1(const LowLevelOneMarketDataField &ld)
    {
        strncpy(md.UpdateTime, ld.UpdateTime, sizeof(md.UpdateTime));
        md.OpenPrice = InvalidToZeroD(ld.OpenPrice);
        md.HighestPrice = InvalidToZeroD(ld.HighestPrice);
        md.LowestPrice = InvalidToZeroD(ld.LowestPrice);
        md.ClosePrice = InvalidToZeroD(ld.ClosePrice);
        md.UpperLimitPrice = ld.UpperLimitPrice;
        md.LowerLimitPrice = ld.LowerLimitPrice;
        md.SettlementPrice = InvalidToZeroD(ld.SettlementPrice);
        md.CurrDelta = InvalidToZeroD(ld.CurrDelta);

        f_flag |= 0x2;
    }
    inline void set_snapshot_l1(const QuickStartMarketDataField &ld)
    {
        strncpy(md.UpdateTime, ld.UpdateTime, sizeof(md.UpdateTime));
        md.OpenPrice = InvalidToZeroD(ld.OpenPrice);
        md.HighestPrice = InvalidToZeroD(ld.HighestPrice);
        md.LowestPrice = InvalidToZeroD(ld.LowestPrice);
        md.ClosePrice = InvalidToZeroD(ld.ClosePrice);
        md.UpperLimitPrice = ld.UpperLimitPrice;
        md.LowerLimitPrice = ld.LowerLimitPrice;
        md.SettlementPrice = InvalidToZeroD(ld.SettlementPrice);
        md.CurrDelta = InvalidToZeroD(ld.CurrDelta);

        f_flag |= 0x2;
    }
    inline bool have_quote_time()
    {
        return md.UpdateTime[0] != '\0';
    }
    inline bool is_finish()
    {
        return f_flag == 0x3 && have_quote_time();
    }
    inline bool is_valid()
    {
        return ((f_flag & 0x1) == 0x1) && (md.UpperLimitPrice > 0.001) && have_quote_time();
    }
};

typedef std::unordered_map<std::string, ShfeMDAndState> ContractToMDMap;

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
    // 数据处理函数对象
    boost::function<void(const SHFEQuote *)> shfe_deep_data_handler_;
    boost::function<void(const CDepthMarketDataField *)> shfe_ex_handler_;
    boost::function<void(const MYShfeMarketData *)> my_shfe_md_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    std::string qtm_name_;

    // save assistant object
    QuoteDataSave<SHFEQuote> *p_shfe_deep_save_;
    QuoteDataSave<CDepthMarketDataField> *p_shfe_ex_save_;
    QuoteDataSave<MYShfeMarketData> *p_my_shfe_md_save_;

    // receive data handlers
    void OnMYShfeMDData(MYShfeMarketData *data);
    void XeleDataHandler();
    void SnapshotDataHandler(const MarketData * md, int current_count);

    // receive threads
    volatile bool running_flag_;
    boost::thread *p_xele_handler_;

    // data manager (to merge all data received to my data structure)
    MYShfeMDManager my_shfe_md_inf_;
    ContractToMDMap md_buffer_;
    ShfeMDAndState &GetL1MD(const char *contract);
    void SendAllValidL1();
};
