#ifndef QUOTE_LOC_XELE_H_
#define QUOTE_LOC_XELE_H_

#include <string>
#include <sstream>
#include <list>
#include <unordered_map>
#include <map>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "ac_xele_md.h"
#include "quote_interface_cffex_level2.h"
#include "quote_datatype_cffex_level2.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

class LocXeleDataHandler
{
public:
    LocXeleDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler);

    virtual ~LocXeleDataHandler(void);

private:
    void QryDepthMarketData();

    // 数据处理函数对象
    boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    char qtm_name_[32];

    // save assistant object
    QuoteDataSave<CFfexFtdcDepthMarketData> *p_save_;
    void RalaceInvalidValue(DepthMarketDataField &d);
    CFfexFtdcDepthMarketData Convert(const DepthMarketDataField &xele_data);

    boost::thread *qry_md_thread_;
    volatile bool running_flag_;
};

#endif
