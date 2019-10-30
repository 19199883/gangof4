#if !defined(MY_QUOTE_SAVE_H_)
#define MY_QUOTE_SAVE_H_

#include "quote_datatype_common.h"
#include "quote_datatype_ctp.h"
#include "quote_datatype_gtaex.h"
#include "quote_datatype_gta_udp.h"
#include "quote_datatype_dce.h"
#include "quote_datatype_shfe_deep.h"
#include "quote_datatype_shfe_ex.h"
#include "quote_datatype_my_shfe_md.h"
#include "quote_datatype_tdf.h"
#include "quote_datatype_czce_level2.h"
#include "quote_datatype_sec_kmds.h"
#include "KSGUserApiStruct.h"
#include "md_datatype_taifex.h"
#include "quote_datatype_cme.h"
#include "YaoQuote.h"
#include "ThostFtdcUserApiStruct.h"

// 行情类型标识定义
#define GTAEX_CFFEX_QUOTE_TYPE          1

#define DCE_MDBESTANDDEEP_QUOTE_TYPE    2
#define DCE_ARBI_QUOTE_TYPE             3
#define DCE_MDTENENTRUST_QUOTE_TYPE     4
#define DCE_MDORDERSTATISTIC_QUOTE_TYPE 5
#define DCE_MDREALTIMEPRICE_QUOTE_TYPE  6
#define DCE_MDMARCHPRICEQTY_QUOTE_TYPE  7

#define CTP_MARKETDATA_QUOTE_TYPE       8

#define SHFE_DEEP_QUOTE_TYPE            9

#define SHFE_EX_QUOTE_TYPE              10

#define GTA_UDP_CFFEX_QUOTE_TYPE        11
#define MY_SHFE_MD_QUOTE_TYPE           12

// TDF 股票行情和指数行情类型
#define TDF_STOCK_QUOTE_TYPE            13
#define TDF_INDEX_QUOTE_TYPE            14

// my derivative data of stock market
#define MY_STOCK_QUOTE_TYPE             15

// CZCE Market data id
#define CZCE_LEVEL2_QUOTE_TYPE          16
#define CZCE_CMB_QUOTE_TYPE             17

// ksg gold of sh
#define SH_GOLD_QUOTE_TYPE              18

// 台湾合并行情类型
#define TAI_FEX_MD_TYPE                 19

// CME 芝加哥交易所
#define DEPTHMARKETDATA_QUOTE_TYPE             30
#define REALTIMEDATA_QUOTE_TYPE             31
#define ORDERBOOKDATA_QUOTE_TYPE             32
#define TRADEVOLUMEDATA_QUOTE_TYPE             33

#define SHFE_LEV2_DATA_QUOTE_TYPE 70

#define YAO_QUOTE_TYPE             125

// data identities of kmds {"stockcode", "stockquote", "indexquote", "optionquote", "ordqueue", "perentrust", "perbargain"};
#define KMDS_CODETABLE_TYPE         0xc0
#define KMDS_STOCK_SNAPSHOT_TYPE    0xc1
#define KMDS_INDEX_TYPE             0xc2
#define KMDS_OPTION_QUOTE_TYPE      0xc3
#define KMDS_ORDER_QUEUE_TYPE       0xc4
#define KMDS_PER_ENTRUST_TYPE       0xc5
#define KMDS_PER_BARGAIN_TYPE       0xc6
#define KMDS_FUTURE_QUOTE_TYPE      0xc7

#pragma pack(1)
#define MAX_PAIR 120
struct PVPair
{
	double price;
	int volume;
};
struct MDPack
{
	char instrument[10];
	char islast;
	int seqno;
	char direction;
	short count;
	PVPair data[MAX_PAIR];
};
#pragma pack()

// 为兼容，对齐方式统一使用8字节对齐
#pragma pack(push)
#pragma pack(8)

// 二进制存储的文件头信息
struct SaveFileHeaderStruct
{
    int data_count;     // number of quote data items
    short data_type;    // 行情类型标识
    short data_length;  // length of one item (byte)
};

// 发送数据，也使用该结构做消息头
typedef SaveFileHeaderStruct SendQuoteDataHeader;

// 国泰安 - 股指期货 五档行情
struct SaveData_GTAEX
{
    long long t_;					// 时间戳
    ZJS_Future_Input_MY data_;		// 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_GTAEX()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_GTAEX(long long t, const ZJS_Future_Input &d)
        : t_(t), data_(d)
    {
    }
};

struct SaveData_GTA_UDP
{
    long long t_;
    CFfexFtdcDepthMarketData data_;
    // 缺省构造
    SaveData_GTA_UDP()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_GTA_UDP(long long t, const CFfexFtdcDepthMarketData &d)
        : t_(t), data_(d)
    {
    }
};

// 飞创-大商 MDBestAndDeep 最优与五档深度行情
struct SaveData_MDBestAndDeep
{
    long long t_;					// 时间戳
    MDBestAndDeep_MY data_;		// 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_MDBestAndDeep()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_MDBestAndDeep(long long t, const DFITC_L2::MDBestAndDeep &d)
        : t_(t), data_(d)
    {
    }
};

// 飞创-大商 MDBestAndDeep 套利行情 - 最优与五档深度行情
struct SaveData_Arbi
{
    long long t_;                   // 时间戳
    MDBestAndDeep_MY data_;     // 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_Arbi()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_Arbi(long long t, const DFITC_L2::MDBestAndDeep &d)
        : t_(t), data_(d)
    {
    }
};

// 飞创-大商 MDTenEntrust 最优价位上十笔委托
struct SaveData_MDTenEntrust
{
    long long t_;					// 时间戳
    MDTenEntrust_MY data_;		// 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_MDTenEntrust()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_MDTenEntrust(long long t, const DFITC_L2::MDTenEntrust &d)
        : t_(t), data_(d)
    {
    }
};

// 飞创-大商 MDOrderStatistic 加权平均以及委托总量行情
struct SaveData_MDOrderStatistic
{
    long long t_;					// 时间戳
    MDOrderStatistic_MY data_;		// 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_MDOrderStatistic()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_MDOrderStatistic(long long t, const DFITC_L2::MDOrderStatistic &d)
        : t_(t), data_(d)
    {
    }
};

// 飞创-大商 MDRealTimePrice 实时结算价
struct SaveData_MDRealTimePrice
{
    long long t_;					// 时间戳
    MDRealTimePrice_MY data_;		// 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_MDRealTimePrice()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_MDRealTimePrice(long long t, const DFITC_L2::MDRealTimePrice &d)
        : t_(t), data_(d)
    {
    }
};

// 飞创-大商 MDMarchPriceQty 分价位成交
struct SaveData_MDMarchPriceQty
{
    long long t_;					// 时间戳
    MDMarchPriceQty_MY data_;		// 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_MDMarchPriceQty()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_MDMarchPriceQty(long long t, const DFITC_L2::MDMarchPriceQty &d)
        : t_(t), data_(d)
    {
    }
};

// CTP MarketData
struct SaveData_CTP
{
    long long t_;								// 时间戳
    CThostFtdcDepthMarketDataField data_;		// 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_CTP()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_CTP(long long t, const CThostFtdcDepthMarketDataField &d)
        : t_(t), data_(d)
    {
    }
};

// SHFE DEEP Data
struct SaveData_SHFE_DEEP
{
    long long t_;                               // 时间戳
    SHFEQuote data_;                            // 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_SHFE_DEEP()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_SHFE_DEEP(long long t, const SHFEQuote &d)
        : t_(t), data_(d)
    {
    }
};

// SHFE Express Data
struct SaveData_SHFE_EX
{
    long long t_;                               // 时间戳
    CDepthMarketDataField data_;                // 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_SHFE_EX()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_SHFE_EX(long long t, const CDepthMarketDataField &d)
        : t_(t), data_(d)
    {
    }
};

// MY shfe market data
struct SaveData_MY_SHFE_MD
{
    long long t_;                               // 时间戳
    MYShfeMarketData data_;                // 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_MY_SHFE_MD()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_MY_SHFE_MD(long long t, const MYShfeMarketData &d)
        : t_(t), data_(d)
    {
    }
};
// tdf stock market data
struct SaveData_TDF_MARKET_DATA
{
    long long t_;                               // 时间戳
    TDF_MARKET_DATA_MY data_;                // 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_TDF_MARKET_DATA()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_TDF_MARKET_DATA(long long t, const TDF_MARKET_DATA_MY &d)
        : t_(t), data_(d)
    {
    }
};

// tdf index data
struct SaveData_TDF_INDEX_DATA
{
    long long t_;                               // 时间戳
    TDF_INDEX_DATA_MY data_;                // 数据对象 （转换为8字节对齐）

    // 缺省构造
    SaveData_TDF_INDEX_DATA()
    {
        t_ = 0;
    }

    // 通过时间戳、和网络数据包构造
    SaveData_TDF_INDEX_DATA(long long t, const TDF_INDEX_DATA_MY &d)
        : t_(t), data_(d)
    {
    }
};

struct SaveData_CZCE_L2_DATA
{
	long long t_;                               // 时间戳
	ZCEL2QuotSnapshotField_MY data_;                // 数据对象 （转换为8字节对齐）

	// 缺省构造
	SaveData_CZCE_L2_DATA()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_CZCE_L2_DATA(long long t, const ZCEL2QuotSnapshotField_MY &d)
		: t_(t), data_(d)
	{
	}
};


struct SaveData_CZCE_CMB_DATA
{
	long long t_;                               // 时间戳
	ZCEQuotCMBQuotField_MY data_;                // 数据对象 （转换为8字节对齐）

	// 缺省构造
	SaveData_CZCE_CMB_DATA()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_CZCE_CMB_DATA(long long t, const ZCEQuotCMBQuotField_MY &d)
		: t_(t), data_(d)
	{
	}
};

struct SaveData_SH_GOLD
{
	long long t_;
	CKSG_DepthMarketDataField data_;
	// 缺省构造
	SaveData_SH_GOLD()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_SH_GOLD(long long t, const CKSG_DepthMarketDataField &d)
		: t_(t), data_(d)
	{
	}
};

struct SaveData_TaiFexMD
{
	long long t_;
	TaiFexMarketDataField data_;
	// 缺省构造
	SaveData_TaiFexMD()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_TaiFexMD(long long t, const TaiFexMarketDataField &d)
		: t_(t), data_(d)
	{
	}
};

// 中信证券使用金证接口的相关数据存储类型
///金证股票
struct SaveData_StockQuote_KMDS
{
	long long t_;
	T_StockQuote data_;
	// 缺省构造
	SaveData_StockQuote_KMDS()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_StockQuote_KMDS(long long t, const T_StockQuote &d)
		: t_(t), data_(d)
	{
	}
};
///金证指数
struct SaveData_IndexQuote_KMDS
{
	long long t_;
	T_IndexQuote data_;
	// 缺省构造
	SaveData_IndexQuote_KMDS()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_IndexQuote_KMDS(long long t, const T_IndexQuote &d)
		: t_(t), data_(d)
	{
	}
};
///金证期权
struct SaveData_Option_KMDS
{
	long long t_;
	T_OptionQuote data_;
	// 缺省构造
	SaveData_Option_KMDS()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_Option_KMDS(long long t, const T_OptionQuote &d)
		: t_(t), data_(d)
	{
	}
};
///金证期货
struct SaveData_FutureQuote_KMDS
{
	long long t_;
	T_FutureQuote data_;
	// 缺省构造
	SaveData_FutureQuote_KMDS()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_FutureQuote_KMDS(long long t, const T_FutureQuote &d)
		: t_(t), data_(d)
	{
	}
};
///金证报单队列
struct SaveData_OrderQueue_KMDS
{
	long long t_;
	T_OrderQueue data_;
	// 缺省构造
	SaveData_OrderQueue_KMDS()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_OrderQueue_KMDS(long long t, const T_OrderQueue &d)
		: t_(t), data_(d)
	{
	}
};
///金证逐笔委托
struct SaveData_PerEntrust_KMDS
{
	long long t_;
	T_PerEntrust data_;
	// 缺省构造
	SaveData_PerEntrust_KMDS()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_PerEntrust_KMDS(long long t, const T_PerEntrust &d)
		: t_(t), data_(d)
	{
	}
};
///金证逐笔成交
struct SaveData_PerBargain_KMDS
{
	long long t_;
	T_PerBargain data_;
	// 缺省构造
	SaveData_PerBargain_KMDS()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_PerBargain_KMDS(long long t, const T_PerBargain &d)
		: t_(t), data_(d)
	{
	}
};

///////// 芝加哥交易所
struct SaveData_depthMarketData
{
	long long t_;                               // 时间戳
	depthMarketData data_;                // 数据对象 （转换为8字节对齐）

	// 缺省构造
	SaveData_depthMarketData()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_depthMarketData(long long t, const depthMarketData &d)
		: t_(t), data_(d)
	{
	}
};


struct SaveData_realTimeData
{
	long long t_;
	realTimeData data_;
	// 缺省构造
	SaveData_realTimeData()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_realTimeData(long long t, const realTimeData &d)
		: t_(t), data_(d)
	{
	}
};

struct SaveData_orderbookData
{
	long long t_;
	orderbookData data_;
	// 缺省构造
	SaveData_orderbookData()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_orderbookData(long long t, const orderbookData &d)
		: t_(t), data_(d)
	{
	}
};

struct SaveData_tradeVolume
{
	long long t_;
	tradeVolume data_;
	// 缺省构造
	SaveData_tradeVolume()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_tradeVolume(long long t, const tradeVolume &d)
		: t_(t), data_(d)
	{
	}
};


struct SaveData_Lev2Data
{
	long long t_;
	CThostFtdcDepthMarketDataField data_;
	// 缺省构造
	SaveData_Lev2Data()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_Lev2Data(long long t, const CThostFtdcDepthMarketDataField &d)
		: t_(t), data_(d)
	{
	}
};

struct SaveData_YaoQuote
{
	long long t_;
	YaoQuote data_;
	// 缺省构造
	SaveData_YaoQuote()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveData_YaoQuote(long long t, const YaoQuote &d)
		: t_(t), data_(d)
	{
	}
};

#pragma pack(pop)

#endif  //MY_QUOTE_SAVE_H_
