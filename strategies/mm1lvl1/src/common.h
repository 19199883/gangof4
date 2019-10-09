#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <string.h>

//#define WRITE_SYSLOG_

#ifdef WRITE_SYSLOG_
    #include <syslog.h>
    #include <errno.h>
#endif // WRITE_SYSLOG_

#define MY_SHFE_QUOTE_PRICE_POS_COUNT 30

#define MAX(a,b)  ((a)>(b)?(a):(b))
#define MIN(a,b)  ((a)>(b)?(b):(a))
#define RELEASE(mem) if(mem != NULL) {free(mem); mem = NULL;}

typedef enum {false=0,true=!false} BOOL;

#define iSIGNAL_LEN 1000
#define iOUT_LEN 100

#define iSTRATEGY_MK1 100
#define iMK101 101

static const char gcVersion[50]= "0.1.1.1tb";
extern int giNightMarket;
extern int giRunModeFlg;
extern int giStrateType, giMK1Type;

extern char gcExch, gcSecName[8], gcTicker[8];


static const int iSigStatusSuccess = 0, iSigStatusPart = 1,
				 iSigStatusCancel = 2, iSigStatusRejected = 3;
static const int iSigInstructionROD = 0, iSigInstructionFOK = 1,
				 iSigInstructionFAK = 2, iSigInstructionMRK = 3;
static const int iSigInstructionQuote = 4, iSigInstructionMAX = 5;

static const int iBUY = 1, iSELL = -1, iZERO = 0, iLONG = 1;
static const int iSHORT = -1, iUP = 1, iDN = -1, iFLAT = 0, iYES = 1, iNO = 0;
static const int iBID = 1, iOFFER = -1, iCD = 2, iXD = 1;
static const int iBPK = 1, iSPK = 2, iBP = 3, iSP = 4, iBK = 5, iSK = 6;

static const char cSHFE = 'A', cDCE = 'B', cCZCE = 'C', cCFFE = 'G', cFUT = 'F',
				  cBLANK ='\0';

struct CDepthMarketDataField
{
    char TradingDay[9];

    char SettlementGroupID[9];
    int SettlementID;

    double LastPrice;
    double PreSettlementPrice;
    double PreClosePrice;
    double PreOpenInterest;
    double OpenPrice;
    double HighestPrice;
    double LowestPrice;
    int Volume;
    double Turnover;
    double OpenInterest;
    double ClosePrice;
    double SettlementPrice;
    double UpperLimitPrice;
    double LowerLimitPrice;
    double PreDelta;
    double CurrDelta;
    char UpdateTime[9];
    int UpdateMillisec;
    char InstrumentID[31];

    double BidPrice1;
    int BidVolume1;
    double AskPrice1;
    int AskVolume1;
    double BidPrice2;
    int BidVolume2;
    double AskPrice2;
    int AskVolume2;
    double BidPrice3;
    int BidVolume3;
    double AskPrice3;
    int AskVolume3;
    double BidPrice4;
    int BidVolume4;
    double AskPrice4;
    int AskVolume4;
    double BidPrice5;
    int BidVolume5;
    double AskPrice5;
    int AskVolume5;

    char ActionDay[9];

};

struct cfex_deep_quote {
	char szTradingDay[9];
	char szSettlementGroupID[9];
	int nSettlementID;
	double dLastPrice;
	double dPreSettlementPrice;
	double dPreClosePrice;
	double dPreOpenInterest;
	double dOpenPrice;
	double dHighestPrice;
	double dLowestPrice;
	int nVolume;
	double dTurnover;
	double dOpenInterest;
	double dClosePrice;
	double dSettlementPrice;
	double dUpperLimitPrice;
	double dLowerLimitPrice;
	double dPreDelta;
	double dCurrDelta;
	char szUpdateTime[9];
	int nUpdateMillisec;
	char szInstrumentID[31];
	double dBidPrice1;
	int nBidVolume1;
	double dAskPrice1;
	int nAskVolume1;
	double dBidPrice2;
	int nBidVolume2;
	double dAskPrice2;
	int nAskVolume2;
	double dBidPrice3;
	int nBidVolume3;
	double dAskPrice3;
	int nAskVolume3;
	double dBidPrice4;
	int nBidVolume4;
	double dAskPrice4;
	int nAskVolume4;
	double dBidPrice5;
	int nBidVolume5;
	double dAskPrice5;
	int nAskVolume5;
};




struct czce_my_deep_quote {
	char        ContractID[50];
	char  ContractIDType;
	double    PreSettle;
	double    PreClose;
	int    PreOpenInterest;
	double    OpenPrice;
	double    HighPrice;
	double    LowPrice;
	double    LastPrice;
	double    BidPrice[5];
	double    AskPrice[5];
	int    BidLot[5];
	int    AskLot[5];
	int    TotalVolume;
	int    OpenInterest;
	double    ClosePrice;
	double    SettlePrice;
	double    AveragePrice;
	double    LifeHigh;
	double    LifeLow;
	double    HighLimit;
	double    LowLimit;
	int    TotalBidLot;
	int    TotalAskLot;
	char   TimeStamp[24];
};

struct cStruct_dl_quote {
	char cType;
	int iLength, iVersion, iTime;
	char cExch[3], cContract[80];
	int iSuspensionSign;
	float rLastClearPrice, rClearPrice, rAvgPrice, rLastClose, rClose, rOpenPrice;
	int iLastOpenInterest, iOpenInterest;
	float rLastPrice;
	int iMatchTotQty;
	double rTurnover;
	float rRiseLimit, rFallLimit, rHighPrice, rLowPrice, rPreDelta, rCurrDelta,
		  rBuyPriceOne;
	int iBuyQtyOne, iBuyImplyQtyOne;
	float rBuyPriceTwo;
	int iBuyQtyTwo, iBuyImplyQtyTwo;
	float rBuyPriceThree;
	int iBuyQtyThree, iBuyImplyQtyThree;
	float rBuyPriceFour;
	int iBuyQtyFour, iBuyImplyQtyFour;
	float rBuyPriceFive;
	int iBuyQtyFive, iBuyImplyQtyFive;
	float rSellPriceOne;
	int iSellQtyOne, iSellImplyQtyOne;
	float rSellPriceTwo;
	int iSellQtyTwo, iSellImplyQtyTwo;
	float rSellPriceThree;
	int iSellQtyThree, iSellImplyQtyThree;
	float rSellPriceFour;
	int iSellQtyFour, iSellImplyQtyFour;
	float rSellPriceFive;
	int iSellQtyFive, iSellImplyQtyFive;
	char cGenTime[13];
	int iLastMatchQty, iInterestChg;
	float rLifeLow, rLifeHigh;
	double rDelta, rGamma, rRho, rTheta, rVega;
	char cTradeDate[9], cLocalDate[9];
};

struct    cStruct_dl_orderstat {
	char cType;
	int iLen;
	char cContractID[80];
	int iTotalBuyOrderNum;
	int iTotalSellOrderNum;
	double     rWeightedAverageBuyOrderPrice;
	double     rWeightedAverageSellOrderPrice;
};




struct czce_my_snapshot_quote {
	char     CmbType;
	char    ContractID[50];
	double    BidPrice;
	double    AskPrice;
	int    BidLot;
	int    AskLot;
	int    VolBidLot;
	int    VolAskLot;
	char        TimeStamp[24];
};


struct shfe_my_quote {

	char    TradingDay[9];
	char    SettlementGroupID[9];
	int    SettlementID;
	double    LastPrice;
	double    PreSettlementPrice;
	double    PreClosePrice;
	double    PreOpenInterest;
	double    OpenPrice;
	double    HighestPrice;
	double    LowestPrice;
	int    Volume;
	double    Turnover;
	double    OpenInterest;
	double    ClosePrice;
	double    SettlementPrice;
	double    UpperLimitPrice;
	double    LowerLimitPrice;
	double    PreDelta;
	double    CurrDelta;

	char    UpdateTime[9];
	int    UpdateMillisec;
	char    InstrumentID[31];
	double    BidPrice1;
	int    BidVolume1;
	double    AskPrice1;
	int    AskVolume1;
	double    BidPrice2;
	int    BidVolume2;
	double    AskPrice2;
	int    AskVolume2;
	double    BidPrice3;
	int    BidVolume3;
	double    AskPrice3;
	int    AskVolume3;
	double    BidPrice4;
	int    BidVolume4;
	double    AskPrice4;
	int    AskVolume4;
	double    BidPrice5;
	int    BidVolume5;
	double    AskPrice5;
	int    AskVolume5;

	char    ActionDay[9];
	int   data_flag;


	unsigned short  start_index;
	unsigned short  data_count;
	double buy_price[MY_SHFE_QUOTE_PRICE_POS_COUNT];
	int    buy_volume[MY_SHFE_QUOTE_PRICE_POS_COUNT];
	double sell_price[MY_SHFE_QUOTE_PRICE_POS_COUNT];
	int    sell_volume[MY_SHFE_QUOTE_PRICE_POS_COUNT];


	unsigned int   buy_total_volume;
	unsigned int   sell_total_volume;
	double         buy_weighted_avg_price;
	double         sell_weighted_avg_price;
};


struct in_data {
	char  cTime[10], cTicker[8], cSecType, cExchCode;
	double  dYestP, dP, dV, dOI, dTotV, dPLmtUp, dPLmtDn, dPOpen, dPHi, dPLo,
			dPYestCl, dPClose;
	double  dYestOI, dPMean, dTotVal, dVal, dPSettle;
	double  dBP1, dBP2, dBP3, dBP4, dBP5, dSP1, dSP2, dSP3, dSP4, dSP5;
	int     iBV1, iBV2, iBV3, iBV4, iBV5, iSV1, iSV2, iSV3, iSV4, iSV5, iTime;
	double  dBP[30], dSP[30];
	int  iBV[30], iSV[30], iImpBV[30], iImpSV[30];
	int  iVol, iTotalBuyVol, iTotalSelVol;
	double  dAvgP, dOIChg, dWeightedBuyOrderP, dWeightedSelOrderP, dPLifeHigh,
			dPLifeLow;
	int  iData_Flag;
};

struct struct_symbol_pos {
	char  ticker[64];
	int long_volume;
	double long_price;
	int short_volume;
	double short_price;
	int pos_change;
};

struct struct_position {
	int count;
	struct struct_symbol_pos pos[10000];
};



struct cStruct_each_pos_in {
	char symbol[30];
	int exchange, maxPos, PDLongPos, PDShortPos, TDLongPos, TDShortPos;
	double availFund, commRatio;
	int longPosIncr, shortPosIncr;
	BOOL feedPos;
	int quoteCat;
};

struct cStruct_each_pending_vol_in {
	char symbol[30];
	double    price;
	int side, volume;
};

struct sec_parameter {
	char cExchCode;
	int iSecMktOpenTime, iSecTradeStartTime, iSecTradeEndTime, iSecOpenEndTime;
	int iSecTradeStartTime02, iSecTradeEndTime02;
	int iSecSqTime, iSecEndTime, iSecFinalSqTime, iSecTraceTime;
	int iUnitPerContract, iWrongVol, iCostSide, iPMultiplier, iTickSizeMulti,
		iDecimalPt, iRatioLevel;
	int iMinTradeCount, iMinTradeSec, iWithCancel, iUseLastV, iLookBackRecordNo;
	int iBaseSize, iMaxSizePerOrder;
	double rTCostByVol, rTCostByVal, rPctLmt, rBrokerPct, rTickSize, rOVCompRatio;
	double rTVCompRatio, rExchSameDayDisc, rTOverPct;
	double rRssTCostByVol, rRssTCostByVal;
};

struct exch_parameter {
	int iVolDivider, iPInLevel, iPreMktAucTime, iMktStartTime, iMktEndTime;
};

struct hi5_parameter {
	double    rMKTThreshold, rOpenThreshold, rCloseThreshold;
};

struct all_parameter {
	struct exch_parameter stExch;
	struct sec_parameter stSec;
	struct hi5_parameter  stHi5Para;
};


struct myTSignal {
	int iStID, iSigID, iBV, iSV, iBP, iSP;
	double rBP, rSP;
	char cExchCode, cTicker[7];
	int iSigAction, iInstruction, iCancelSigID;
};




struct comm_out_data {
	char cTicker[8];
	int iTime, iRecordNo, iV;
	double rP, rBP1, rSP1, rVal, rOI, rBuyPrice, rSelPrice;
	int iBV1, iSV1, iHaveOrder, iTS, iCancel, iOpenSize, iCloseSize;
	int iToTradeSize, iLongPos, iShortPos, iSidePos, iTotalOrderSize,
		iTotalCancelSize;
    int iTotalOrderCount, iTotalCancelCount;
};

typedef struct struct_order_info{
	int sig_id;
	int itime;
	int cancel_sig_id;
	double price;
	int volume;
	int direct;  //1: buy;  2: sell  4: cancel
	int kaiping;  //0:ping;  1: kai
	int vol2Pass;
	double exec_avg_price;
	int exec_acc_vol;
	double exec_price;
	int exec_vol;
	int status;
} st_order_info;

//st_order_info.status
enum {
		 SIG_STATUS_INIT = -1,			 //new order
	 	 SIG_STATUS_SUCCESS, 	 	     /* 报单全部成交 */
	 	 SIG_STATUS_ENTRUSTED, 	 	   /* 报单委托成功 */
	 	 SIG_STATUS_PARTED, 	 	 	 /* 报单部分成交 */
	 	 SIG_STATUS_CANCEL, 	 	 	 /* 报单被撤销 */
	 	 SIG_STATUS_REJECTED, 	 	 	 /* 报单被拒绝 */
};

//st_order_info.kaiping
enum {
    SIG_ACTION_OPEN = 1,
    SIG_ACTION_FLAT = 2,
};

//st_order_info.direct
enum {
    SIG_DIRECTION_BUY = 1,
    SIG_DIRECTION_SELL= 2,
    SIG_DIRECTION_CANCEL = 4,
};


typedef struct struct_local_trade_info
{
    int iPosD, iPosK;
    double rCash, rTotalCost;
    double rLivePL;
    int iLastTradeTime;
    int iTotalVol;
    int iBV1Sum, iSV1Sum;
    int iTotalOrderSize, iTotalCancelSize;
    int iCurrOrderSize, iCurrCancelSize;
    int iTotalOrderCount, iTotalCancelCount;
    double rMaxLivePL, rMaxLiveDD, rLiveDD;
    int iMaxSidePos, iMaxNetPos;
}st_loc_trade_info;



extern int  giNoTradeItemEachRun, giLb, giLen;
extern int  giPosLb, giPosLen, giPendVLb, giPendVLen;

extern struct in_data  gstCurrIn;
extern struct in_data  gstPreIn;
extern struct in_data  gstRecCurrIn;
extern struct in_data  gstRecPreIn;
extern struct cfex_deep_quote     *gstcStructCFFEIn;
extern struct cStruct_dl_quote     *gstcStructDCEIn;
extern struct shfe_my_quote     *gstcStructSHFEIn;
extern struct czce_my_deep_quote     *gstcStructCZCEIn;

extern struct all_parameter    gstGlobalPara;
extern int  giTickNo;

extern int  *gaiFullOrderBookBuy, *gaiFullOrderBookSel;

extern int  giFullOrderBookL, giFullOrderBookU, giLenFullOrderBook;
extern int  giSimuPara1, giSimuPara2, giSimuPara3;
extern int  giSizePerTrade;
extern FILE  *pfDayLogFile;
extern char  gcFileName[512];
extern int  giInitFlg;
extern int  giWriteOutputFileFlg, giWriteOutputFileInitFlg;
extern int  giInnerPar1Flg, giInnerPar2Flg, giInnerPar3Flg;

extern double grSimuValueTot, grSimuValueTemp, grSimuValueMax, grSimuDrawDown;
extern double grTotBench, grSimuDrawDownRatio;
extern int  giMaxTime;

extern int giStID, giMaxPos;

extern st_loc_trade_info gstLocTradeInfo;

#endif
