#pragma once

#include "quote_datatype_ib.h"

#include <set>
#include <map>
#include <string>
#include <boost/function.hpp>

#define DLL_PUBLIC  __attribute__ ((visibility("default")))

// 订阅的合约，集合类型
typedef std::set<std::string> SubscribeContracts;

class ConfigData;

class DLL_PUBLIC MYQuoteData
{
public:

    /**
     * @param subscribe_contracts 需要订阅的合约集合。(指针为空指针，或集合为空，将返回行情接口接收到的所有合约数据)
     * @param provider_config_file 行情的配置文件名
     */
    MYQuoteData(const SubscribeContracts *subscribe_contracts, const std::string &provider_config_file);

    /**
     * @param quote_handler 行情处理的函数对象。interactive brokers
     */
    void SetQuoteDataHandler(boost::function<void(const IBDepth*)> depth_handler);
    void SetQuoteDataHandler(boost::function<void(const IBTick*)> tick_handler);
	
	void SetQuoteDataHandler(boost::function<void(const int *)> quote_handler){}

    ~MYQuoteData();

private:
    // 禁止拷贝和赋值
    MYQuoteData(const MYQuoteData & other);
    MYQuoteData operator=(const MYQuoteData & other);

    // 内部实现接口
    bool InitInterface(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg, std::string id);

    void *interface_;
    int quote_provider_type_;
};
