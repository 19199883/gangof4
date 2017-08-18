#ifndef CFFEX_LEVEL2_MULTI_INTERFACE_H_
#define CFFEX_LEVEL2_MULTI_INTERFACE_H_

#include <string>
#include <unordered_set>

#include <boost/thread.hpp>

#include "quote_interface_cffex_level2.h"
#include "quote_datatype_cffex_level2.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include "quote_femas.h"
#include "quote_femas_multicast.h"

typedef std::unordered_set<std::string> ID_Collection;

class MDCffexMultiResouce: private boost::noncopyable
{
public:
    MDCffexMultiResouce(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler);

    virtual ~MDCffexMultiResouce();

private:

    //std::vector<MYFPGACffexDataHandler *> my_fpga_interfaces_;
    //std::vector<NetXeleDataHandler *> net_xele_interfaces_;
    //std::vector<LocXeleDataHandler *> loc_xele_interfaces_;
    std::vector<MYFEMASDataHandler *> femas_interfaces_;
    std::vector<FemasMulticastMDHandler *> femas_mc_interfaces_;

    // 数据处理函数对象
    boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler_;
    void DataHandler(const CFfexFtdcDepthMarketData *data);

    char qtm_name_[32];

    // quote id, for filter duplicate data
    boost::mutex ids_sync_;
    ID_Collection ids_;

    // save for valify
    QuoteDataSave<CFfexFtdcDepthMarketData> *p_save_;
    bool filter_flag;
};

#endif
