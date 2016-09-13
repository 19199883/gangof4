#if !defined(MY_QUOTE_LIB_H_)
#define MY_QUOTE_LIB_H_

#include "quote_datatype_common.h"
#include "quote_datatype_ctp.h"
#include "quote_datatype_gtaex.h"
#include "quote_datatype_gta_udp.h"
#include "quote_datatype_dce.h"
#include "quote_datatype_shfe_deep.h"
#include "quote_datatype_shfe_ex.h"
#include "quote_datatype_my_shfe_md.h"

#include <boost/function.hpp>
//#include <functional>

// 前向声明
class GTAEXRecvInterface;
class QuoteInterface_GTA_UDP;
class MYDCEDataHandler;
class MYCTPDataHandler;
class QuoteInterface_SHFE_EX;
class QuoteInterface_SHFE_DEEP;
class QuoteInterface_MY_SHFE_MD;
class ConfigData;


class MYQuoteData
{
public:

	/**
	* @param subscribe_contracts 需要订阅的合约集合。
	*                  指针为空指针，或集合为空，将返回行情接口接收到的所有合约数据。
	* @param provider_config_file 行情的配置文件名
	*                  配置文件为空时，默认使用 my_quote_lib.config。
	*/
	MYQuoteData(const SubscribeContracts *subscribe_contracts,
		const std::string &provider_config_file);

	/**
	* @param quote_handler 行情处理的函数对象。
	*/
	// 国泰安的中转行情，tcp连接，无api
	//void SetQuoteDataHandler(boost::function<void (const ZJS_Future_Input_MY *)> quote_handler);

	// 国泰安的udp行情，直接转发交易所发送的数据结构
	void SetQuoteDataHandler(boost::function<void (const CFfexFtdcDepthMarketData *)> quote_handler);

	// 飞创的行情服务，包括五档行情、10笔委托，等6种类型数据
	void SetQuoteDataHandler(boost::function<void (const MDBestAndDeep_MY *)> quote_handler);
	void SetQuoteDataHandler(boost::function<void (const MDTenEntrust_MY *)> quote_handler);
	void SetQuoteDataHandler(boost::function<void (const MDArbi_MY *)> quote_handler);
    void SetQuoteDataHandler(boost::function<void (const MDOrderStatistic_MY *)> quote_handler);
    void SetQuoteDataHandler(boost::function<void (const MDRealTimePrice_MY *)> quote_handler);
    void SetQuoteDataHandler(boost::function<void (const MDMarchPriceQty_MY *)> quote_handler);

	// CTP 行情
	void SetQuoteDataHandler(boost::function<void (const CThostFtdcDepthMarketDataField *)> quote_handler);

	// 上期的快速行情（中信提供，只有1档）
	void SetQuoteDataHandler(boost::function<void (const CDepthMarketDataField *)> quote_handler);

	// 上期的深度数据（中信提供）
	void SetQuoteDataHandler(boost::function<void (const SHFEQuote *)> quote_handler);

	// MY上期的行情数据（xh提供）
	void SetQuoteDataHandler(boost::function<void (const MYShfeMarketData *)> quote_handler);

	~MYQuoteData();

private:
	// 禁止拷贝和赋值
	MYQuoteData(const MYQuoteData & other);
	MYQuoteData operator=(const MYQuoteData & other);

	// 内部实现接口
	bool InitGTAEX(const SubscribeContracts *subscribe_contracts,	const ConfigData &cfg);
	bool InitGTAUDP(const SubscribeContracts *subscribe_contracts,   const ConfigData &cfg);
	bool InitDCE(const SubscribeContracts *subscribe_contracts,		const ConfigData &cfg);
    bool InitCTP(const SubscribeContracts *subscribe_contracts,     const ConfigData &cfg);
    bool InitSHFEEX(const SubscribeContracts *subscribe_contracts,     const ConfigData &cfg);
    bool InitSHFEDEEP(const SubscribeContracts *subscribe_contracts,     const ConfigData &cfg);
    bool InitMYShfeMD(const SubscribeContracts *subscribe_contracts,     const ConfigData &cfg);
private:
	GTAEXRecvInterface *gta_interface_;
	QuoteInterface_GTA_UDP *gta_udp_interface_;
	MYDCEDataHandler *dce_interface_;
	MYCTPDataHandler *ctp_interface_;
	QuoteInterface_SHFE_EX *shfe_ex_interface_;
	QuoteInterface_SHFE_DEEP *shfe_deep_interface_;
	QuoteInterface_MY_SHFE_MD *my_shfe_md_interface_;
};

#endif  //MY_QUOTE_LIB_H_
