﻿#ifndef QUOTE_INTERFACE_DATASOURCE_H
#define QUOTE_INTERFACE_DATASOURCE_H

#include "quote_datatype_datasource.h"

#include <set>
#include <string>
#include <boost/function.hpp>
//#include <functional>

#define DLL_PUBLIC  __attribute__ ((visibility("default")))

// 订阅的合约，集合类型
typedef std::set<std::string> SubscribeContracts;

// forward declare
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
     * @param quote_handler 行情处理的函数对象。stock derivative data of MY
     */
    void SetQuoteDataHandler(boost::function<void(const inner_quote_fmt *)> quote_handler);

    void SetQuoteDataHandler(boost::function<void(const int *)> quote_handler){}

    ~MYQuoteData();

private:
    // 禁止拷贝和赋值
    MYQuoteData(const MYQuoteData & other);
    MYQuoteData operator=(const MYQuoteData & other);

    // 内部实现接口
    bool InitInterface(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

private:
    void *interface_;
};

#endif
