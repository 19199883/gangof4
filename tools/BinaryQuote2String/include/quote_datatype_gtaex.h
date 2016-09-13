#if !defined(QUOTE_DATATYPE_GTAEX_H_)
#define QUOTE_DATATYPE_GTAEX_H_

#pragma pack(push)
#pragma pack(1)
#define GTAEX_HEATBEAT_TYPE 2 // 心跳消息
#define GTAEX_SOURCE_CFFEX	3  //3表示为股指期货
struct ZJSFuturePacketHeader
{
	int		nMsgSeqNum;				// 包序号（可判断丢包）
	char	cMsgType;				// 类型（1：价格信息，2: heartbeat，3：更新完成，0 删除）
	char	cDataType;				// GTA 保存
	int		nBodyLength;			// 此市场长度包括头，体，尾. 29+328+4=361
	char	szSendingDate[9];		// 日期
	char	szSendingTime[9];		// 时间
	char	cSourceCode;			// 数据来源编码 0x01 上海现货， 0x02 上海现货， 0x03 股指期货， 0x04 上海商品期货
};		//size:29

struct ZJS_Future_Input		//中金所股指期货
{
	char	szStockNo[6];		//	期货代码
	double	dChg;				//	涨跌
	double	dChgPct;			//	涨跌幅
	double	dVolume;			//	当日总量
	double	dAmount;			//	成交总金额
	double	dLastTrade;			//	成交价
	double	dLastVolume;		//	瞬量
	double	dUpB;				//	涨停价
	double	dLowB;				//	跌停价
	double	dHigh;				//	当日最高
	double	dLow;				//	当日最低
	double	dYclose;			//	昨收
	double	dOpen;				//	开盘
	double	dAvgPrice;			//	均价
	double	dPriceGap;			//	檔差
	double	dBuyprice1;			//	买价一
	double	dBuyprice2;			//	买价二
	double	dBuyprice3;			//	买价三
	double	dBuyprice4;			//	买价四
	double	dBuyprice5;			//	买价五
	double	dBuyvol1;			//	买量一
	double	dBuyvol2;			//	买量二
	double	dBuyvol3;			//	买量三
	double	dBuyvol4;			//	买量四
	double	dBuyvol5;			//	买量五
	double	dSellprice1;		//	卖价一
	double	dSellprice2;		//	卖价二
	double	dSellprice3;		//	卖价三
	double	dSellprice4;		//	卖价四
	double	dSellprice5;		//	卖价五
	double	dSellvol1;			//	卖量一
	double	dSellvol2;			//	卖量二
	double	dSellvol3;			//	卖量三
	double	dSellvol4;			//	卖量四
	double	dSellvol5;			//	卖量五
	double	dPreOpenInterest;
	double	dOpenInterest;
	double	dSettlementPrice;
	char	szTradingDay[9];
	char	szUpdateTime[9];
	double	dPreSettlementPrice;//	昨日结算价
	//double dClosePrice;	//今收盘
	//double dPreDelta;	//昨虚实度
	//double dDelta;	//今虚实度
	//char szSettlementGroupID[9]; //结算组代码
	//int nSettleID;	//结算编号
	//int nUpdateMillisec;	//最后修改毫秒     股指期货txt解析文档.docx有47个字段少这6个,
	int tail;	// 暂时无用的尾部，放到结构中，以便简化处理
};	//size:328
#pragma pack(pop)


// 按8字节对齐的转换后数据结构
#include <string.h>
#pragma pack(push)
#pragma pack(8)
struct ZJS_Future_Input_MY     //中金所股指期货
{
    char    szStockNo[6];       //  期货代码
    double  dChg;               //  涨跌
    double  dChgPct;            //  涨跌幅
    double  dVolume;            //  当日总量
    double  dAmount;            //  成交总金额
    double  dLastTrade;         //  成交价
    double  dLastVolume;        //  瞬量
    double  dUpB;               //  涨停价
    double  dLowB;              //  跌停价
    double  dHigh;              //  当日最高
    double  dLow;               //  当日最低
    double  dYclose;            //  昨收
    double  dOpen;              //  开盘
    double  dAvgPrice;          //  均价
    double  dPriceGap;          //  檔差
    double  dBuyprice1;         //  买价一
    double  dBuyprice2;         //  买价二
    double  dBuyprice3;         //  买价三
    double  dBuyprice4;         //  买价四
    double  dBuyprice5;         //  买价五
    double  dBuyvol1;           //  买量一
    double  dBuyvol2;           //  买量二
    double  dBuyvol3;           //  买量三
    double  dBuyvol4;           //  买量四
    double  dBuyvol5;           //  买量五
    double  dSellprice1;        //  卖价一
    double  dSellprice2;        //  卖价二
    double  dSellprice3;        //  卖价三
    double  dSellprice4;        //  卖价四
    double  dSellprice5;        //  卖价五
    double  dSellvol1;          //  卖量一
    double  dSellvol2;          //  卖量二
    double  dSellvol3;          //  卖量三
    double  dSellvol4;          //  卖量四
    double  dSellvol5;          //  卖量五
    double  dPreOpenInterest;
    double  dOpenInterest;
    double  dSettlementPrice;
    char    szTradingDay[9];
    char    szUpdateTime[9];
    double  dPreSettlementPrice;//  昨日结算价

    ZJS_Future_Input_MY(){};
    ZJS_Future_Input_MY (const ZJS_Future_Input &other)
    {
        memcpy(szStockNo, other.szStockNo, 6); //char    szStockNo[6];
        dChg = other.dChg;
        dChgPct = other.dChgPct;
        dVolume = other.dVolume;
        dAmount = other.dAmount;
        dLastTrade = other.dLastTrade;
        dLastVolume = other.dLastVolume;
        dUpB = other.dUpB;
        dLowB = other.dLowB;
        dHigh = other.dHigh;
        dLow = other.dLow;
        dYclose = other.dYclose;
        dOpen = other.dOpen;
        dAvgPrice = other.dAvgPrice;
        dPriceGap = other.dPriceGap;
        dBuyprice1 = other.dBuyprice1;
        dBuyprice2 = other.dBuyprice2;
        dBuyprice3 = other.dBuyprice3;
        dBuyprice4 = other.dBuyprice4;
        dBuyprice5 = other.dBuyprice5;
        dBuyvol1 = other.dBuyvol1;
        dBuyvol2 = other.dBuyvol2;
        dBuyvol3 = other.dBuyvol3;
        dBuyvol4 = other.dBuyvol4;
        dBuyvol5 = other.dBuyvol5;
        dSellprice1 = other.dSellprice1;
        dSellprice2 = other.dSellprice2;
        dSellprice3 = other.dSellprice3;
        dSellprice4 = other.dSellprice4;
        dSellprice5 = other.dSellprice5;
        dSellvol1 = other.dSellvol1;
        dSellvol2 = other.dSellvol2;
        dSellvol3 = other.dSellvol3;
        dSellvol4 = other.dSellvol4;
        dSellvol5 = other.dSellvol5;
        dPreOpenInterest = other.dPreOpenInterest;
        dOpenInterest = other.dOpenInterest;
        dSettlementPrice = other.dSettlementPrice;
        memcpy(szTradingDay, other.szTradingDay, 9); //char    szTradingDay[9];
        memcpy(szUpdateTime, other.szUpdateTime, 9); //char    szUpdateTime[9];
        dPreSettlementPrice = other.dPreSettlementPrice;
    }
};
#pragma pack(pop)

#endif  //QUOTE_DATATYPE_GTAEX_H_
