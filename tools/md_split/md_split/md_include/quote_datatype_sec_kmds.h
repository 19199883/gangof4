#if !defined(QUOTE_DATATYPE_SEC_KMDS_H_)
#define QUOTE_DATATYPE_SEC_KMDS_H_

#ifndef DLL_PUBLIC
#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

#include <string>

// 按8字节对齐的转换后数据结构
#pragma pack(push)
#pragma pack(8)

typedef unsigned char UCHAR8;
typedef unsigned short UINT16;
typedef long long INT64;
typedef char CHAR8;
typedef short INT16;
typedef int INT32;
typedef unsigned int UINT32;
typedef double REAL8;
typedef int BOOL;

#define SCR_CODE_LEN    32
#define SCR_NAME_LEN    32

// 实时证券代码表
struct T_StockCode
{
    INT32 market;                   // 交易市场
    CHAR8 scr_code[SCR_CODE_LEN];   // 证券代码
    CHAR8 scr_name[SCR_NAME_LEN];   // 证券名称
    INT64 pre_close;                // 昨收盘价
    INT64 open_price;               // 开盘价
    INT64 high_price;               // 涨停价
    INT64 low_price;                // 跌停价
    INT64 pre_delta;                // 昨虚实度
    INT64 pre_open_interest;        // 昨持仓
    INT64 settle_price;             // 今日结算价
    INT64 pre_settle_price;         // 昨结算价

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        return "";
    }
};

// 实时股票行情
struct T_StockQuote
{
    INT32 time;                     // 行情时间
    INT32 market;                   // 交易市场
    CHAR8 scr_code[SCR_CODE_LEN];   // 证券代码
    INT64 seq_id;                   // 快照序号
    INT32 high_price;               // 最高价
    INT32 low_price;                // 最低价
    INT32 last_price;               // 最新价
    INT32 sell_price[10];           // 申卖价
    UINT32 sell_volume[10];         // 申卖量
    INT32 bid_price[10];            // 申买价
    UINT32 bid_volume[10];          // 申买量
    INT32 bgn_count;                // 成交笔数
    INT64 bgn_total_vol;            // 成交总量
    INT64 bgn_total_amt;            // 成交总金额
    INT64 total_bid_lot;            // 委买总量
    INT64 total_sell_lot;           // 委卖总量
    INT32 wght_avl_bid_price;       // 加权平均委买价格
    INT32 wght_avl_sell_price;      // 加权平均委卖价格

    INT32 iopv_net_val_valt;        // IOPV 净值估值
    INT32 mtur_yld;                 // 到期收益率
    INT32 status;                   // 分笔快照数据类型码
    INT32 pe_ratio_1;               // 市盈率1
    INT32 pe_ratio_2;               // 市盈率2
    INT64 error_mark;               // 错误字段域
    INT64 publish_tm1;              // 一级发布时间
    INT64 publish_tm2;              // 二级发布时间
    CHAR8 pre_fix[8];               // 证券信息前缀 （宏汇特有,宏汇接口定义4个字节）

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        int t = time;                  //时间(HHMMSSmmm)
        sprintf(buf, "%02d:%02d:%02d.%03d", t / 10000000,
            (t / 100000) % 100,
            (t / 1000) % 100,
            t % 1000);
        return std::string(buf);
    }
};

// 实时指数行情
struct T_IndexQuote
{
    INT32 time;
    INT32 market;
    CHAR8 scr_code[SCR_CODE_LEN];
    INT64 seq_id;
    INT32 high_price;               // 最高价
    INT32 low_price;                // 最低价
    INT32 last_price;               // 当前价
    INT64 bgn_total_vol;            // 总成交量
    INT64 bgn_total_amt;            // 总成交金额
    INT64 error_mark;               // 错误字段域
    INT64 publish_tm1;              // 一级发布时间
    INT64 publish_tm2;              // 二级发布时间

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        int t = time;                  //时间(HHMMSSmmm)
        sprintf(buf, "%02d:%02d:%02d.%03d", t / 10000000,
            (t / 100000) % 100,
            (t / 1000) % 100,
            t % 1000);
        return std::string(buf);
    }
};

// 期货/股指期权行情
struct T_FutureQuote
{
    INT32 time;
    INT32 market;
    CHAR8 scr_code[SCR_CODE_LEN];
    INT64 seq_id;
    INT32 high_price;       // 最高价
    INT32 low_price;        // 最低价
    INT32 last_price;       // 当前价
    INT32 today_close;      // 今收盘
    INT32 today_delta;
    INT32 sell_price[5];    // 卖1-5价
    UINT32 sell_volume[5];  // 卖1-5量
    INT32 bid_price[5];     // 买1-5价
    UINT32 bid_volume[5];   // 买1-5量
    INT64 bgn_total_vol;    // 成交总量
    INT64 bgn_total_amt;    // 成交总金额
    INT64 today_pos;        // 持仓总量
    INT64 error_mark;       // 错误字段域
    INT64 publish_tm1;      // 一级发布时间
    INT64 publish_tm2;      // 二级发布时间

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        int t = time;                  //时间(HHMMSSmmm)
        sprintf(buf, "%02d:%02d:%02d.%03d", t / 10000000,
            (t / 100000) % 100,
            (t / 1000) % 100,
            t % 1000);
        return std::string(buf);
    }
};

// 个股期权行情
struct T_OptionQuote
{
    INT64 time;                     // 行情时间 HHMMSSmmm
    INT32 market;
    CHAR8 scr_code[SCR_CODE_LEN];
    INT64 seq_id;                   // 快照序号
    INT64 high_price;
    INT64 low_price;
    INT64 last_price;
    INT64 bgn_total_vol;
    INT64 bgn_total_amt;
    INT64 auction_price;            // 动态参考价格
    INT64 auction_qty;              // 虚拟匹配数量
    INT64 sell_price[5];
    INT64 bid_price[5];
    INT64 sell_volume[5];
    INT64 bid_volume[5];

    INT64 long_position;            // 当前合约未平仓数

    INT64 error_mark;               // 错误字段域
    INT64 publish_tm1;              // 一级发布时间
    INT64 publish_tm2;              // 二级发布时间
    INT64 pub_num;                  // 行情信息编号
    CHAR8 trad_phase[4];            // 产品实施阶段及标志
    CHAR8 md_report_id[12];         // 行情信息编号

    // added on 20150709 (static fields from code table)
    CHAR8 contract_code[SCR_CODE_LEN];  // 510050C1512M02650
    INT64 high_limit_price;             // 涨停价 * 10000
    INT64 low_limit_price;              // 跌停价 * 10000
    INT64 pre_close_price;              // 昨收盘价 * 10000
    INT64 open_price;                   // 开盘价 * 10000

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        int t = (int)time;                  //时间(HHMMSSmmm)
        sprintf(buf, "%02d:%02d:%02d.%03d", t / 10000000,
            (t / 100000) % 100,
            (t / 1000) % 100,
            t % 1000);
        return std::string(buf);
    }
};

// 实时订单队列
struct T_OrderQueue
{
    INT32 time;
    INT32 market;
    CHAR8 scr_code[SCR_CODE_LEN];
    CHAR8 insr_txn_tp_code[4];      // 指令交易类型 'B','S'
    INT32 ord_price;                // 订单价格
    INT32 ord_qty;                  // 订单数量
    INT32 ord_nbr;                  // 明细个数
    INT32 ord_detail_vol[200];      // 订单明细数量 （参考宏汇接口，定义成200个整数数组）

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        int t = time;                  //时间(HHMMSSmmm)
        sprintf(buf, "%02d:%02d:%02d.%03d", t / 10000000,
            (t / 100000) % 100,
            (t / 1000) % 100,
            t % 1000);

        return std::string(buf);
    }
};

// 实时逐笔委托
struct T_PerEntrust
{
    INT32 market;
    CHAR8 scr_code[SCR_CODE_LEN];
    INT32 entrt_time;               // 委托时间
    INT32 entrt_price;              // 委托价格
    INT64 entrt_id;                 // 委托编号
    INT64 entrt_vol;                // 委托数量

    CHAR8 insr_txn_tp_code[4];      // 指令交易类型 'B','S'
    CHAR8 entrt_tp[4];              // 委托类别

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        int t = entrt_time;                  //时间(HHMMSSmmm)
        sprintf(buf, "%02d:%02d:%02d.%03d", t / 10000000,
            (t / 100000) % 100,
            (t / 1000) % 100,
            t % 1000);

        return std::string(buf);
    }
};

// 实时逐笔成交
struct T_PerBargain
{
    INT32 time;                     // 成交时间
    INT32 market;
    CHAR8 scr_code[SCR_CODE_LEN];
    INT64 bgn_id;                   // 成交编号
    INT32 bgn_price;                // 成交价格
    INT64 bgn_qty;                  // 成交数量
    INT64 bgn_amt;                  // 成交金额

    CHAR8 bgn_flg[4];               // 成交类别
    CHAR8 nsr_txn_tp_code[4];       // 指令交易类型

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        int t = time;                  //时间(HHMMSSmmm)
        sprintf(buf, "%02d:%02d:%02d.%03d", t / 10000000,
            (t / 100000) % 100,
            (t / 1000) % 100,
            t % 1000);

        return std::string(buf);
    }
};

#pragma pack(pop)

#endif
