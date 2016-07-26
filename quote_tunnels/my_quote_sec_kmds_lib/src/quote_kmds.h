#ifndef KMDS_QUOTE_INTERFACE_H_
#define KMDS_QUOTE_INTERFACE_H_

#include "quote_cmn_utility.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "ckmdsdef.h"
#include "ckmdsuserapi.h"
#include "cmsgdata.h"
#include "ctabledata.h"


#include "quote_cmn_config.h"
#include "quote_interface_sec_kmds.h"
#include "quote_datatype_stock_tdf.h"
#include "quote_datatype_sec_kmds.h"
#include "quote_cmn_save.h"

typedef std::unordered_map<std::string, T_StockCode> StockCodeMap;

class  QuoteInterface_KMDS : private boost::noncopyable
{
public:
    QuoteInterface_KMDS(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const TDF_MARKET_DATA_MY *)> quote_data_handler)
    {
        md_data_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const TDF_INDEX_DATA_MY *)> quote_data_handler)
    {
        id_data_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_OptionQuote *)> quote_data_handler)
    {
        st_option_data_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_OrderQueue *)> quote_data_handler)
    {
        st_order_queue_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_PerEntrust *)> quote_data_handler)
    {
        st_per_entrust_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_PerBargain *)> quote_data_handler)
    {
        st_per_bargain_handler_ = quote_data_handler;
    }

    virtual ~QuoteInterface_KMDS();

private:
    static KMDSUSERAPIHANDLE pclient;

    static void Init();
    static void OnKmdsMsg(void* pmsgdesc, void* pmsgopts,int ncmd ,MSGDATAHANDLE pmsg);
    static void GetCodeTable();

    // data handler for each type of market data
    // code table changed   推送
    static void OnCodeTableMsg(MSGDATAHANDLE pmsg);
    // 股票行情数据   推送
    static void OnStockMD(MSGDATAHANDLE pmsg);
    // 期货行情数据   推送
    static void OnFutureMD(MSGDATAHANDLE pmsg);
    // 指数行情数据   推送
    static void OnIndex(MSGDATAHANDLE pmsg);
    // 订单队列     推送
    static void OnOrderQueue(MSGDATAHANDLE pmsg);
    // 逐笔委托     推送
    static void OnPerEntrust(MSGDATAHANDLE pmsg);
    // 逐笔成交     推送
    static void OnTransaction(MSGDATAHANDLE pmsg);

    // stock option
    static void OnStockOptionMD(MSGDATAHANDLE pmsg);

    // 数据处理函数对象
    static boost::function<void(const TDF_MARKET_DATA_MY *)> md_data_handler_;
    static boost::function<void(const TDF_INDEX_DATA_MY *)> id_data_handler_;
    static boost::function<void(const T_OptionQuote *)> st_option_data_handler_;
    static TDF_MARKET_DATA_MY Convert(const T_StockQuote &kmds_data);
    static TDF_INDEX_DATA_MY Convert(const T_IndexQuote &kmds_data);

    static boost::function<void(const T_OrderQueue *)> st_order_queue_handler_;
    static boost::function<void(const T_PerEntrust *)> st_per_entrust_handler_;
    static boost::function<void(const T_PerBargain *)> st_per_bargain_handler_;

    // 订阅合约集合
    static SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    static ConfigData cfg_;
    static char qtm_name_[32];

    static QuoteDataSave<TDF_MARKET_DATA_MY> *s_p_stock_md_save;
    static QuoteDataSave<TDF_INDEX_DATA_MY> *s_p_stock_idx_save;
    static QuoteDataSave<T_StockQuote> *s_p_kmds_stock_save;
    static QuoteDataSave<T_FutureQuote> *s_p_kmds_future_save;
    static QuoteDataSave<T_OptionQuote> *s_p_kmds_option_save;
    static QuoteDataSave<T_IndexQuote> *s_p_kmds_index_save;
    static QuoteDataSave<T_OrderQueue> *s_p_kmds_ord_queue_save;
    static QuoteDataSave<T_PerEntrust> *s_p_kmds_per_entrust_save;
    static QuoteDataSave<T_PerBargain> *s_p_kmds_per_bgn_save;

    static boost::mutex md_sync_mutex_;
    static StockCodeMap stock_code_map_;
    static void AddCodeInfo(int market, const std::string &scr_code, const T_StockCode &stock_code);
    static bool GetCodeInfo(int market, const std::string &scr_code, T_StockCode *stock_code_info);
    static std::string BuildScrCode(int market, const std::string &scr_code);
    static volatile int ready_flag;
};
#endif
