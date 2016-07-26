#pragma once

#include <atomic>
#include <string>
#include <sstream>
#include <list>
#include <iomanip>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "ZceLevel2ApiInterface.h"

#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include "quote_interface_czce_level2.h"

class MYCZCEDataHandler: public IZCEL2QuoteNotify
{
public:
    // 构造函数
    MYCZCEDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    virtual void ZCEL2QUOTE_CALL OnRspLogin(const ZCEL2QuotRspLoginField& loginMsg);
    virtual void ZCEL2QUOTE_CALL OnRecvQuote(const ZCEL2QuotSnapshotField& snapshot);
    virtual void ZCEL2QUOTE_CALL OnRecvCmbQuote(const ZCEL2QuotCMBQuotField& snapshot);
    virtual void ZCEL2QUOTE_CALL OnConnectClose();

    virtual ~MYCZCEDataHandler(void);

    void SetQuoteDataHandler(boost::function<void(const ZCEL2QuotSnapshotField_MY *)> quote_data_handler)
    {
        l2_quote_handler_ = quote_data_handler;
    }
    void SetQuoteDataHandler(boost::function<void(const ZCEQuotCMBQuotField_MY *)> quote_data_handler)
    {
        cmb_quote_handler_ = quote_data_handler;
    }
    // property
    bool Logoned() const
    {
        return logoned_;
    }

    void SetPublish()
    {
        publish_data_flag_ = true;
    }

    void SetUnPublish()
    {
        publish_data_flag_ = false;
    }

    bool GetAndClearRecvState()
    {
        bool cur_flag = recv_data_flag_;
        recv_data_flag_ = false;
        return cur_flag;
    }

private:
    IZCEL2QuoteClient *api_;

    bool logoned_;
    boost::mutex quote_mutex;
    bool in_login_flag;

    std::atomic_bool recv_data_flag_;
    std::atomic_bool publish_data_flag_;

    void SetServerAddr();
    void ReqLogin(int wait_seconds);

    // 数据处理函数对象
    boost::function<void(const ZCEL2QuotSnapshotField_MY *)> l2_quote_handler_;
    boost::function<void(const ZCEQuotCMBQuotField_MY *)> cmb_quote_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;

    // save assistant object
    QuoteDataSave<ZCEL2QuotSnapshotField_MY> *p_save_l2_quote_;
    QuoteDataSave<ZCEQuotCMBQuotField_MY> *p_save_cmb_quote_;

    // data convert functions
    ZCEL2QuotSnapshotField_MY Convert(const ZCEL2QuotSnapshotField &other);
    ZCEQuotCMBQuotField_MY Convert(const ZCEL2QuotCMBQuotField &other);

    std::string qtm_name;
};
