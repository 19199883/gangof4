#ifndef TDF_STOCK_QUOTE_INTERFACE_H_
#define TDF_STOCK_QUOTE_INTERFACE_H_

#include "quote_cmn_utility.h"
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "TDFAPIStruct.h"

#include "quote_cmn_config.h"
#include "quote_interface_sec_kmds.h"
#include "quote_datatype_stock_tdf.h"
#include "quote_datatype_sec_kmds.h"
#include "quote_cmn_save.h"

struct TDF_OPEN_SETTING;

class  QuoteInterface_TDF : private boost::noncopyable
{
public:
    QuoteInterface_TDF(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const TDF_MARKET_DATA_MY *)> quote_data_handler)
    {
        md_data_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const TDF_INDEX_DATA_MY *)> quote_data_handler)
    {
        id_data_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_OptionQuote*)> quote_data_handler)
    {
        option_quote_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_OrderQueue*)> quote_data_handler)
    {
        order_queue_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_PerEntrust*)> quote_data_handler)
    {
        perentrust_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const T_PerBargain*)> quote_data_handler)
    {
        perbargain_handler_ = quote_data_handler;
    }

    virtual ~QuoteInterface_TDF();

private:
    TDF_OPEN_SETTING *open_settings;
    THANDLE g_hTDF;

    static void InitOpenSetting(TDF_OPEN_SETTING *settings, const ConfigData &cfg);
    static void QuoteDataHandler(THANDLE hTdf, TDF_MSG* pMsgHead);
    static void SystemMsgHandler(THANDLE hTdf, TDF_MSG* pMsgHead);

    // 数据处理函数对象
    static boost::function<void(const TDF_MARKET_DATA_MY *)> md_data_handler_;
    static boost::function<void(const TDF_INDEX_DATA_MY *)> id_data_handler_;
    static boost::function<void(const T_OptionQuote*)> option_quote_handler_;
    static boost::function<void(const T_OrderQueue*)> order_queue_handler_;
    static boost::function<void(const T_PerEntrust*)> perentrust_handler_;
    static boost::function<void(const T_PerBargain*)> perbargain_handler_;

    // 订阅合约集合
    static SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    static char qtm_name_[32];

    // save assistant object
    QuoteDataSave<TDF_MARKET_DATA_MY> *p_stock_md_save_;
    QuoteDataSave<TDF_INDEX_DATA_MY> *p_stock_idx_save_;
    QuoteDataSave<T_OrderQueue> *p_stock_oq_save_;
    QuoteDataSave<T_PerEntrust> *p_stock_pe_save_;
    QuoteDataSave<T_PerBargain> *p_stock_pb_save_;
};

#endif // TDF_STOCK_QUOTE_INTERFACE_H_
