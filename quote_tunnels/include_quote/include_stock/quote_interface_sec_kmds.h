#ifndef MY_QUOTE_INTERFACE_KMDS_SEC_H_
#define MY_QUOTE_INTERFACE_KMDS_SEC_H_

#include "quote_datatype_stock_tdf.h"
#include "quote_datatype_sec_kmds.h"

#include <set>
#include <string>
#include <boost/function.hpp>
//#include <functional>

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
     * @param quote_handler 行情处理的函数对象。stock market/index data of TDF
     */
    void SetQuoteDataHandler(boost::function<void(const TDF_MARKET_DATA_MY *)> quote_handler);
    void SetQuoteDataHandler(boost::function<void(const TDF_INDEX_DATA_MY *)> quote_handler);

    /// 个股期权行情
    void SetQuoteDataHandler(boost::function<void(const T_OptionQuote *)> quote_handler);

    /// 实时订单队列
    void SetQuoteDataHandler(boost::function<void(const T_OrderQueue *)> quote_handler);

    /// 实时逐笔委托
    void SetQuoteDataHandler(boost::function<void(const T_PerEntrust *)> quote_handler);

    /// 实时逐笔成交
    void SetQuoteDataHandler(boost::function<void(const T_PerBargain *)> quote_handler);
	
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

#endif  //MY_QUOTE_INTERFACE_STOCK_TDF_H_
