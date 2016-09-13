#ifndef _QUOTE_DATATYPE_TAIFEX_
#define _QUOTE_DATATYPE_TAIFEX_

#define DLL_PUBLIC

#include <string>
#pragma pack(push)
#pragma pack(8)

/// 合并后的转发数据结构
struct DLL_PUBLIC TaiFexMarketDataField
{
	///最后修改时间 格式： HHMMSSMS 时 分 秒 百分秒
	int UpdateTime;
	///代码
	char PROD_ID[20];

	///最新价
    double LastPrice;
    ///今开盘
    double OpenPrice;
    ///最高价
    double HighestPrice;
    ///最低价
    double LowestPrice;
    ///总成交数量
    int Volume;
    ///总成交金额
    double Turnover;
    ///持仓量
    double OpenInterest;
    ///今收盘
    double ClosePrice;
    ///今结算
    double SettlementPrice;
    ///涨停板价
    double UpperLimitPrice;
    ///跌停板价
    double LowerLimitPrice;

    ///申买价一
    double BidPrice1;
    ///申买量一
    int BidVolume1;
    ///申卖价一
    double AskPrice1;
    ///申卖量一
    int AskVolume1;
    ///申买价二
    double BidPrice2;
    ///申买量二
    int BidVolume2;
    ///申卖价二
    double AskPrice2;
    ///申卖量二
    int AskVolume2;
    ///申买价三
    double BidPrice3;
    ///申买量三
    int BidVolume3;
    ///申卖价三
    double AskPrice3;
    ///申卖量三
    int AskVolume3;
    ///申买价四
    double BidPrice4;
    ///申买量四
    int BidVolume4;
    ///申卖价四
    double AskPrice4;
    ///申卖量四
    int AskVolume4;
    ///申买价五
    double BidPrice5;
    ///申买量五
    int BidVolume5;
    ///申卖价五
    double AskPrice5;
    ///申卖量五
    int AskVolume5;
};

// 和原始数据包对应的转发结构
// I010: 商品漲跌幅及基本資料訊息
struct DLL_PUBLIC I010
{
	// 	X(10)	10	商品代號	
	char  PROD_ID_S[10];
	// 	9(9)	5	第一漲停價 	
	double RISE_LIMIT_PRICE1;
	// 	9(9)	5	參考價 	
	double REFERENCE_PRICE;
	// 	9(9)	5	第一跌停價 	
	double FALL_LIMIT_PRICE1;
	// 	9(9)	5	第二漲停價 	
	double RISE_LIMIT_PRICE2;
	// 	9(9)	5	第二跌停價 	
	double FALL_LIMIT_PRICE2;
	// 	9(9)	5	第三漲停價 	
	double RISE_LIMIT_PRICE3;
	// 	9(9)	5	第三跌停價 	
	double FALL_LIMIT_PRICE3;
	// 	X(1)	1	契約種類	I:指數類  R:利率類  B:債券類  C:商品類  S:股票類
	char PROD_KIND;
	// 	9(1)	1	價格欄位小數位數	
	int DECIMAL_LOCATOR;
	// 	9(1)	1	選擇權商品代號之履約價格小數位數(期貨則固定為0)參考附錄五	
	int STRIKE_PRICE_DECIMAL_LOCATOR;
	// 	9(8)	4	上市日期 (YYYYMMDD)	
	int BEGIN_DATE;
	// 	9(8)	4	下市日期 (YYYYMMDD)	
	int END_DATE;
};

// I011	契約基本資料
struct DLL_PUBLIC  I011
{
	// 	X(4)	4	契約代碼	
	char KIND_ID[4];
	// 	X(30)	30	契約名稱 契約中文名稱	
	char NAME[30];
	// 	X(6)	6	現貨股票代碼	
	char STOCK_ID[6];
	// 	X(1)	1	契約類別	 I:指數類  R:利率類  B:債券類  C:商品類  S:股票類
	char SUBTYPE;
	// 	9(5)V9(4)	5	契約乘數	
	int CONTRACT_SIZE;
	// 	X(1)	1	狀態碼 N:正常 P:暫停交易;U:即將上市	
	char STATUS_CODE;
	// 	X(1)	1	幣別	
	char CURRENCY_TYPE;
	// 	9(1)	1	價格欄位小數位數 同 I010 商品價格小數位數	
	int DECIMAL_LOCATOR;
	// 	9(1)	1	選擇權商品代號之履約價格小數位數 (期貨則固定為0)同I010 選擇權商品代號之履約價格小數位數	
	int STRIKE_PRICE_DECIMAL_LOCATOR;
	// 	X(1)	1	是否可報價 Y:可報價; N:不可報價	
	char ACCEPT_QUOTE_FLAG;
	// 	X(8)	8	上市日期。日期格式為 YYYYMMDD 僅適用股票類契約	
	int BEGIN_DATE;
	// 	X(1)	1	是否可鉅額交易 Y:可; N:不可	
	char BLOCK_TRADE_FLAG;
	// 	X(1)	1	到期別 S:標準;W:週	
	char EXPIRY_TYPE;
};

// I020	成交價量揭示
struct DLL_PUBLIC I020
{
	// 	X(20)	20	商品代號	
	char PROD_ID[20];
	// 	9(8)	4	成交時間	
	int MATCH_TIME;
	// 	9(9)	5	第一成交價 	
	double FIRST_MATCH_PRICE;
	// 	9(8)	4	第一成交量	
	int FIRST_MATCH_QNTY;
	// 	X(01)	1	成交資料揭示項目註記（BIT MAP）	
	// 1:yes; 0:no
	int FirstPackageFlag;
	// 成交價量个数 (0 - 127)
	int MatchItemCount;
	// 	9(9)	5	成交價格 下标从0开始，个数 = MatchItemCount
	double MATCH_PRICE[127];
	// 	9(4)	2	成交數量 下标从0开始，个数 = MatchItemCount	
	int MATCH_QUANTITY[127];
	// 	9(8)	4	累計成交數量	
	int MATCH_TOTAL_QTY;
	// 	9(8)	4	累計買進成交筆數	
	int MATCH_BUY_CNT;
	// 	9(8)	4	累計賣出成交筆數	
	int MATCH_SELL_CNT;
	// 	9(2)	1	狀況標記 00:正常狀況;01~60:異常狀況時間(分鐘);98:異常狀況解除;99:異常狀況超過60分鐘	
	int STATUS_CODE;
};

// I022 試撮成交價量揭示
typedef I020 I022;

// I021	盤中最(高)低價揭示
struct DLL_PUBLIC I021
{
	// 	X(20)	20	商品代號	
	char PROD_ID[20];
	// 	9(9)	5	當日最高成交價格 	
	double DAY_HIGHT_PRICE;
	// 	9(9)	5	當日最低成交價格 	
	double DAY_LOW_PRICE;
	// 	9(8)	4	最後價格穿越時間	
	int SHOW_TIME;
};

// I023	定時開盤價量揭示
struct DLL_PUBLIC I023
{
	// 	X(20)	20	商品代號	
	char PROD_ID[20];
	// 	9(8)	4	成交時間	
	int MATCH_TIME;
	// 	9(9)	5	開盤價 	
	double FIRST_MATCH_PRICE;
	// 	9(8)	4	開盤量	
	int FIRST_MATCH_QNTY;
};

// I030	單一商品委託量累計
// 	X(20)	20	商品代號	
struct DLL_PUBLIC I030
{
	char PROD_ID[20];
	// 	9(8)	4	買進累計委託筆數	
	int BUY_ORDER;
	// 	9(8)	4	買進累計委託合約數	
	int BUY_QUANTITY;
	// 	9(8)	4	賣出累計委託筆數	
	int SELL_ORDER;
	// 	9(8)	4	賣出累計委託合約數	
	int SELL_QUANTITY;
};

// I050	公告訊息
struct DLL_PUBLIC I050
{
	// 	9(4)	2	本公告的編號(鍵值)	
	int BUILTIN_KEY;
	// 	X(80)	80	公告資料	
	char BUILTIN_DATA[80];
};

// 						
// I060	現貨標的資訊
struct DLL_PUBLIC I060
{
	// 	X(3)	3	類別代碼	
	char KIND[3];
	// 	9(8)	4	指數或價格時間	
	int TIME;
	// 	9(5)V999	4	標的指數或價格	
	double VALUE;
	// 	X(1)	1	狀態項目註記 (BIT MAP)	
	// Bit7-1 = 0 保留;
	// Bit0, 0:一般揭示; 1:個股延後收盤(此時 TIME 與 VALUE 欄位為最近一次成交的時間與價格)
	unsigned char STATUS_ITEM;
};

// I070/I071/I072 收盤行情資料訊息
struct DLL_PUBLIC I07X
{
	// 	X(10)	10	商品代號	
	char PROD_ID_S[10];
	// 	9(9)	5	該期最高價 	
	double TERM_HIGH_PRICE;
	// 	9(9)	5	該期最低價 	
	double TERM_LOW_PRICE;
	// 	9(9)	5	該日最高價 	
	double DAY_HIGH_PRICE;
	// 	9(9)	5	該日最低價 	
	double DAY_LOW_PRICE;
	// 	9(9)	5	開盤價 	
	double OPEN_PRICE;
	// 	9(9)	5	最後買價 	
	double BUY_PRICE;
	// 	9(9)	5	最後賣價 	
	double SELL_PRICE;
	// 	9(9)	5	收盤價 	
	double CLOSE_PRICE;
	// 	9(8)	4	委託買進總筆數	
	int BO_COUNT_TAL;
	// 	9(8)	4	委託買進總口數	
	int BO_QNTY_TAL;
	// 	9(8)	4	委託賣出總筆數	
	int SO_COUNT_TAL;
	// 	9(8)	4	委託賣出總口數	
	int SO_QNTY_TAL;
	// 	9(8)	4	總成交筆數	
	int TOTAL_COUNT;
	// 	9(8)	4	總成交量	
	int TOTAL_QNTY;
	// 	9(8)	4	合併委託買進總筆數	
	int COMBINE_BO_COUNT_TAL;
	// 	9(8)	4	合併委託買進總口數	
	int COMBINE_BO_QNTY_TAL;
	// 	9(8)	4	合併委託賣出總筆數	
	int COMBINE_SO_COUNT_TAL;
	// 	9(8)	4	合併委託賣出總口數	
	int COMBINE_SO_QNTY_TAL;
	// 	9(8)	4	合併總成交量	
	int COMBINE_TOTAL_QNTY;
	// 	9(9)	5	結算價 	(仅 I071,I072 有，没有时为0）
	double SETTLEMENT_PRICE;
	// 	9(8)	4	未平倉合約數	(仅 I072 有，没有时为0）
	int OPEN_INTEREST;
	// 	9(8)	4	鉅額交易總成交量	(仅 I072 有，没有时为0）
	int BLOCK_TRADE_QNTY;
};

// I073	複式商品收盤行情資料訊息
struct DLL_PUBLIC I073
{
	// 	X(20)	20	商品代號	
	char PROD_ID[20];
	// 	9(9)	5	該期最高價 	
	double TERM_HIGH_PRICE;
	// 	9(9)	5	該期最低價 	
	double TERM_LOW_PRICE;
	// 	9(9)	5	該日最高價 	
	double DAY_HIGH_PRICE;
	// 	9(9)	5	該日最低價 	
	double DAY_LOW_PRICE;
	// 	9(9)	5	開盤價 	
	double OPEN_PRICE;
	// 	9(9)	5	最後買價 	
	double BUY_PRICE;
	// 	9(9)	5	最後賣價 	
	double SELL_PRICE;
	// 	9(9)	5	收盤價 	
	double CLOSE_PRICE;
	// 	9(8)	4	委託買進總筆數	
	int BO_COUNT_TAL;
	// 	9(8)	4	委託買進總口數	
	int BO_QNTY_TAL;
	// 	9(8)	4	委託賣出總筆數	
	int SO_COUNT_TAL;
	// 	9(8)	4	委託賣出總口數	
	int SO_QNTY_TAL;
	// 	9(8)	4	總成交筆數	
	int TOTAL_COUNT;
	// 	9(8)	4	總成交量	
	int TOTAL_QNTY;
};

// 						
// I080	委託簿揭示訊息
struct DLL_PUBLIC I080
{
	// 	X(20)	20	商品代號	
	char PROD_ID[20];
	// 	9(9)	5	最佳買進價格 下标从0开始，数值为0表示没有该档位
	double BUY_PRICE[5];
	// 	9(8)	4	最佳買進價格數量 下标从0开始，数值为0表示没有该档位
	int BUY_QUANTITY[5];
	// 	9(9)	5	最佳賣出價格 下标从0开始，数值为0表示没有该档位
	double SELL_PRICE[5];
	// 	9(8)	4	最佳賣出價格數量 下标从0开始，数值为0表示没有该档位
	int SELL_QUANTITY[5];
	// 	9(2)	1	有無衍生委託單之旗標 01:解析下面四個欄位;00:不帶後面四個欄位	
	int DERIVED_FLAG;
	// 	9(9)	5	衍生委託單第一檔買進價格 	
	double FIRST_DERIVED_BUY_PRICE;
	// 	9(8)	4	衍生委託單第一檔買進價格數量	
	int FIRST_DERIVED_BUY_QUANTITY;
	// 	9(9)	5	衍生委託單第一檔賣出價格 	
	double FIRST_DERIVED_SELL_PRICE;
	// 	9(8)	4	衍生委託單第一檔賣出價格數量	
	int FIRST_DERIVED_SELL_QUANTITY;
};

// I082 試撮後剩餘委託簿揭示訊息
typedef I080 I082;

// I100	詢價揭示訊息
struct DLL_PUBLIC I100
{
	// 	X(10)	10	商品代號	
	char PROD_ID_S[10];
	// 	9(8)	4	詢價揭示時間 格式：HHMMSSMS (時:分:秒:百分秒)	
	int DISCLOSURE_TIME;
	// 	9(3)	2	詢價持續時間 0-999 (秒)	
	int DURATION_TIME;
};

// I120	股票選擇權/股票期貨與現貨標的對照表
struct DLL_PUBLIC  I120 
{
	// 	X(3)	3	股票選擇權代碼 or 股票期貨代碼	
	char INDEX_KIND[3];
	// 	X(6)	6	股票代碼	
	char INDEX_NUMBER[6];
	// 	9(5)V999	4	現貨標的開盤參考價	
	double INDEX_VALUE;
	// 	X(01)	1	狀態碼(P/N)	
	char INDEX_STATUS;
};

// I130	股票選擇權/股票期貨契約調整檔
struct DLL_PUBLIC I130
{
	// 	9(8)	4	調整基準日	
	int CADJ_BASE_DATE;
	// 	X(04)	4	調整前契約代碼	
	char CADJ_BF_KIND_ID[4];
	// 	X(06)	6	調整前約定標的物1_證券代號	
	char CADJ_BF_STOCK_ID[6];
	// 	9(6)V9999	5	調整前約定標的物1_股數	
	double CADJ_BF_STOCK_QNTY;
	// 	9(6)V9999	5	調整前約定標的物2_現金	
	double CADJ_BF_STOCK_CASH2;
	// 	9(6)V9999	5	調整前約定標的物3_現增價值	
	double CADJ_BF_STOCK_CASH3;
	// 	X(06)	6	調整前約定標的物4_證券代號	
	char CADJ_BF_STOCK_ID4[6];
	// 	9(6)V9999	5	調整前約定標的物4_股數	
	double CADJ_BF_STOCK_QNTY4;
	// 	X(04)	4	調整後契約代碼	
	char CADJ_AF_KIND_ID[4];
	// 	X(06)	6	調整後約定標的物1_證券代號	
	char CADJ_AF_STOCK_ID[6];
	// 	9(6)V9999	5	調整後約定標的物1_增減股數	
	double CADJ_AF_STOCK_QNTY;
	// 	9(6)V9999	5	調整後約定標的物2_現金	
	double CADJ_AF_STOCK_CASH2;
	// 	9(6)V9999	5	調整後約定標的物3_認購價	
	double CADJ_AF_STOCK_PRICE3;
	// 	9(6)V9999	5	調整後約定標的物3_認購股數	
	double CADJ_AF_STOCK_QNTY3;
	// 	9(8)	4	現金增資繳款截止日	
	int CADJ_AF_STOCK_DATE3;
	// 	X(06)	6	調整後約定標的物4_證券代號	
	char CADJ_AF_STOCK_ID4[6];
	// 	9(6)V9999	5	調整後約定標的物4_股數	
	double CADJ_AF_STOCK_QNTY4;
	// 	9(8)	4	除權除息交易日	
	int CADJ_DIVIDEND_DATE;
};

// 放寬漲跌幅狀況
struct DLL_PUBLIC  I140_1
{
	// 	9(4)	2	 100:放寬漲跌幅	
	// 	9(4)	2	 101:倒數集合競價時間	
	// 	9(4)	2	 102:集合競價開始	
	int FUNCTION_CODE;
	// 	X(10)	10	契約或商品代碼	
	char PROD_ID_S[10];
	// 	9(2)	1	漲跌階段	
	int RANGE;
	// 	9(6)	3	HHMMSS 放寬開始時間 FUNCTION_CODE 为 100 时有效
	int START_TIME;
	// 	9(6)	3	HHMMSS 集合競價時間 FUNCTION_CODE 为 100 时有效
	int COMPETITION_TIME;
	// 	9(4)	2	MMSS 倒數集合競價時間 FUNCTION_CODE 为 101 时有效
	int LEFT_TIME;
};

// 暫停交易狀況
struct DLL_PUBLIC I140_2
{
	// 	9(4)	2	 200:暫停交易	
	// 	9(4)	2	 201:恢復交易	
	int FUNCTION_CODE;
	// 	9(6)	3	HHMMSS 開始收單時間 FUNCTION_CODE 为201 时有效	
	int START_TIME;
	// 	9(6)	3	HHMMSS 集合競價時間 FUNCTION_CODE 为201 时有效	
	int COMPETITION_TIME;
	// 	9(2)	1	恢復/暫停交易契約筆數	
	int PDK_COUNT;
	// 	X(3)	3	恢復/暫停契約	
	char PDK_KIND_ID[3][99];
};

// 市場狀態通知
struct DLL_PUBLIC I140_3
{
	// I140_302	 開始收單
	// 	9(4)	2	 302:開始收單	
	// 	9(4)	2	 304:集合競價開始	
	// 	9(4)	2	 305: 不可刪單階段開始 Non_cancel period 開始	
	int FUNCTION_CODE;
	// 	9(2)	1	契約筆數	
	int PDK_COUNT;
	// 	X(3)	3	契約代號	
	char PDK_KIND_ID[3][99];
};

#pragma pack(pop)
#endif
