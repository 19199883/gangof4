#pragma once

#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <list>

#include <boost/function.hpp>
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"
#include "quote_interface_czce_level2.h"

#include "multicast_pkg_struct.h"

class MYLTSMDL2
{
public:
    // 构造函数
    MYLTSMDL2(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);
    virtual ~MYLTSMDL2();

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const struct ZCEL2QuotSnapshotField_MY *)> handler)
    {
        snap_l2_handler_ = handler;
    }
    void SetQuoteDataHandler(boost::function<void(const ZCEQuotCMBQuotField_MY *)> handler)
    {
        cmb_quote_handler_ = handler;
    }

private:

    volatile bool run_flag_;
    int current_date;

    // 配置数据对象
    ConfigData cfg_;

    int CreateUdpFD(const std::string& addr_ip, const std::string& gourp_ip, unsigned short port);
    void Start_Real_Time_Recv();
    void Start_Recovery_Recv();

    // recv thread and sync var
    boost::thread *recv_real_time_thread_;
    boost::thread *recv_recovery_thread_;

    // 数据处理函数对象
    boost::function<void(const struct ZCEL2QuotSnapshotField_MY *)> snap_l2_handler_;
    boost::function<void(const ZCEQuotCMBQuotField_MY *)> cmb_quote_handler_;

    struct ZCEL2QuotSnapshotField_MY Convert(const quote_snap_shot_t *pd);

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // save assistant object
    QuoteDataSave<ZCEL2QuotSnapshotField_MY> *p_md_l2_save_;
    QuoteDataSave<quote_snap_shot_t> *snap_original_save_;
    QuoteDataSave<ZCEL2QuotSnapshotField_MY> *recovery_original_save_;
    QuoteDataSave<ZCEQuotCMBQuotField_MY> *realtime_cmb_save_;
    QuoteDataSave<ZCEQuotCMBQuotField_MY> *recovery_cmb_save_;

    std::string qtm_name;
};
