#ifndef QUOTE_CZCE_UDP_H
#define QUOTE_CZCE_UDP_H

#include <sys/types.h>
#include <stdint.h>
#include <dirent.h>
#include <string>
#include <vector>

#include <thread>
#include <boost/atomic.hpp>

#include "my_cmn_util_funcs.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include "quote_interface_czce_level2.h"
#include "quote_interface_tap.h"
#include "ZceLevel2ApiStruct.h"
#include <map>
#include <string>
#include <mutex>

using namespace std;

class CzceUdpMD
{
public:
    // 构造函数
    CzceUdpMD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

	// added by wangying on 20161028
    // 数据处理回调函数设置
	 void SetQuoteDataHandler(std::function<void(const ZCEL2QuotSnapshotField_MY *)> quote_data_handler)
     {
	     l2_quote_handler_ = quote_data_handler;
	 }
	void SetQuoteDataHandler(std::function<void(const ZCEQuotCMBQuotField_MY *)> quote_data_handler)
	{
        MY_LOG_WARN("CZCE_UDP - ZCEQuotCMBQuotField_MY not supported");
	}
	
    virtual ~CzceUdpMD();

private:
    // quote data handlers
    void UdpDataHandler();
    int CreateUdpFD(const std::string &addr_ip, unsigned short port);

    // 数据处理函数对象
    std::function<void(const ZCEL2QuotSnapshotField_MY*)> l2_quote_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    char qtm_name_[32];

    // save assistant object
    QuoteDataSave<ZCEL2QuotSnapshotField_MY> *p_save_zcel2_quote_snap_snapshot_;

	void OnTapAPIQuoteWhole_MY(const TapAPIQuoteWhole_MY *);

	std::string ToString(const ZCEL2QuotSnapshotField_MY* p);
	ZCEL2QuotSnapshotField_MY Convert(const StdQuote5 &other,TapAPIQuoteWhole_MY *tap_data );

    // receive threads
    volatile bool running_flag_;
    std::thread *p_md_handler_;

	TAP::MYQuoteData *lvl1_provider_;
	// the first market data for TapAPIQuoteWhole_MY type
	map<string,TapAPIQuoteWhole_MY> first_data_each_contract_;
	mutex first_data_each_contract_lock_;
	/*
	 * get the first level1 data, which type is TapAPIQuoteWhole_MY, using the given 
	 * contract(its format is, for example, SR1701) from the field first_data_each_contract._
	 */
	TapAPIQuoteWhole_MY *get_data_by_udp_contr(string &contract);

	TapAPIQuoteWhole_MY *get_data_by_tap_contr(string &contract);
};

#endif
