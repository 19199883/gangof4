#pragma once

#include <string>

#include <boost/thread.hpp>

#include "quote_datatype_czce_level2.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include "quote_czce.h"

class CZCE_Multi_Source: private boost::noncopyable
{
public:
    CZCE_Multi_Source(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const ZCEL2QuotSnapshotField_MY *)> quote_data_handler);
    void SetQuoteDataHandler(boost::function<void(const ZCEQuotCMBQuotField_MY *)> quote_data_handler);

    virtual ~CZCE_Multi_Source();

private:
    std::vector<MYCZCEDataHandler *> cece_level2_interfaces_;
    std::vector<int> recv_data_flag_;
    int cur_used_line_idx_;
    int line_count_;
    void RecvDataCheck();
    boost::thread *p_md_c;

    // 数据处理函数对象
    boost::function<void(const ZCEL2QuotSnapshotField_MY *)> quote_data_handler_;
    void DataHandler(const ZCEL2QuotSnapshotField_MY *data);
    boost::function<void(const ZCEQuotCMBQuotField_MY *)> cmb_data_handler_;
    void CMBDataHandler(const ZCEQuotCMBQuotField_MY *data);

    // save for valify
    QuoteDataSave<ZCEL2QuotSnapshotField_MY> *p_save_;
    QuoteDataSave<ZCEQuotCMBQuotField_MY> *p_save_cmb_;
    bool filter_flag;

    std::string qtm_name;
};
