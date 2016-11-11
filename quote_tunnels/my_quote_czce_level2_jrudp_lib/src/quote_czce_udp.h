#ifndef QUOTE_CZCE_UDP_H
#define QUOTE_CZCE_UDP_H

#include <sys/types.h>
#include <stdint.h>
#include <dirent.h>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include "my_cmn_util_funcs.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include "quote_interface_czce_level2.h"

class CzceUdpMD
{
public:
    // 构造函数
    CzceUdpMD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

	// added by wangying on 20161028
    // 数据处理回调函数设置
	 void SetQuoteDataHandler(boost::function<void(const ZCEL2QuotSnapshotField_MY *)> quote_data_handler)
     {
	     l2_quote_handler_ = quote_data_handler;
	 }
	void SetQuoteDataHandler(boost::function<void(const ZCEQuotCMBQuotField_MY *)> quote_data_handler)
	{
        MY_LOG_WARN("CZCE_UDP - ZCEQuotCMBQuotField_MY not supported");
	}
	
    virtual ~CzceUdpMD();

private:
    // quote data handlers
    void UdpDataHandler();
    int CreateUdpFD(const std::string &addr_ip, unsigned short port);

    // 数据处理函数对象
    boost::function<void(const ZCEL2QuotSnapshotField_MY*)> l2_quote_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    char qtm_name_[32];

    // save assistant object
    QuoteDataSave<ZCEL2QuotSnapshotField_MY> *p_save_zcel2_quote_snap_snapshot_;

    // receive threads
    volatile bool running_flag_;
    boost::thread *p_md_handler_;
};

#endif
