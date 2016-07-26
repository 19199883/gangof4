#ifndef FEMAS_MULTICAST_QUOTE_INTERFACE_H_
#define FEMAS_MULTICAST_QUOTE_INTERFACE_H_

#include <string>
#include <sstream>
#include <list>
#include <unordered_map>

#include "UstpMdApi.h"

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "quote_interface_cffex_level2.h"
#include "quote_datatype_cffex_level2.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

typedef std::unordered_map<std::string, CUstpFtdcDepthMarketDataField> MDSnapshotMap;

class FemasMulticastMDHandler
{
public:
    FemasMulticastMDHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler);

    virtual ~FemasMulticastMDHandler(void);

private:
	void *handle_;
	bool is_ready_;
	volatile bool run_flag_;
	int heartbeat_check_interval_seconds_;
	boost::thread *p_hbt_check_;
	boost::thread *p_md_handler_;
	bool Init();
	void Login();
	void HeartBeatCheckThread();
	void DataHandle();

    // 数据处理函数对象
    boost::function<void(const CFfexFtdcDepthMarketData *)> quote_data_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    char qtm_name_[32];

    // save assistant object
    QuoteDataSave<CFfexFtdcDepthMarketData> *p_save_;
    void RalaceInvalidValue_Femas(CUstpFtdcDepthMarketDataField &d);
    CFfexFtdcDepthMarketData Convert(const CUstpFtdcDepthMarketDataField &femas_data);

    // login in thread and notify work thread
    boost::mutex login_sync_;
    boost::condition_variable login_con_;

    MDSnapshotMap snapshot_map_;
};

#endif
