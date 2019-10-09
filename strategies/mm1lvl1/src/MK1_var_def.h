#ifndef MK1_VAR_DEF_H
#define MK1_VAR_DEF_H

#define MK1_MAX_POS 50
#define MK1_MM_LEVEL 1
#define MK1_MM_NUM 1
#define MK2_MAX_MM_TIME 300

struct struct_out_order
{
    double price;
    int ivol;
    int vol2pass;
};

struct struct_mm_outdata
{
	struct comm_out_data stCommOutData;
    int iTime, iVol, iBV1, iSV1, iSignalType, iVol2Trade, iPos, iPOSD, iPOSK, iRounds, iVol2Pass, iTickNo, iNewPOS, iNetPOS;
    int iRound, iCanCelOrder, iMaxNetPos, iMaxSidePos;
    double rP, rBP1, rSP1, rBVSVRatio, rTradeBP, rTradeSP, rCash, rVround, rPnl,rLivePL, rAvgP, rVolBuy, rVolSel, rPNet;
    char cTradeSig[4], cTime[10];
    double rBigVolScore, rMaxDD;
    struct struct_out_order out_order[10];
};

struct struct_MK1_paras
{
	double rBVSum, rSVSum, rStopLossThreshold, rVolBuy, rVolSel, rVolBuyPre, rVolSelPre, rPNet;
    int iStop, iUPstop, iDNstop, iWithClose;
    int iLimitRange, iLimitStop, iLimitSqRange, iLimitSqFlg, iPreviousSigType, iRecPreviousSigType;
};


static const int giMaxMMlevel = MK1_MM_LEVEL, giKeepOrderTime = MK2_MAX_MM_TIME;


extern struct struct_mm_outdata gastMK1OutData[iOUT_LEN];
extern int giMK1OutDataIndex, giMK1TraceInit;
extern struct struct_MK1_paras gstMK1_Paras;
extern int gilastThreeRec[3];
extern int giSingleOrderMaxVol;
extern int giStopLossTickNum;
extern double garAveBSV1[100000];
extern double garActBuy[100000], garActSel[100000];
extern int giActBuyNum, giActSelNum;
extern int giPreActNum;
extern double giPreActBuy[5];
extern double giPreActSel[5];
extern int giBSV1Num;
extern int giStatus;
extern double grLastBP[MK1_MM_NUM];
extern double grLastSP[MK1_MM_NUM];
extern double grLastPrice[MK1_MM_NUM];
extern int giLastBV[MK1_MM_NUM];
extern int giLastSV[MK1_MM_NUM];
extern int giCoverTime;
extern double grHitBuyP, grHitBuyP2;
extern double grHitSelP, grHitSelP2;
extern int giStopLoss;
extern double grActiveVolBuy[MK1_MM_NUM];
extern double grActiveVolSel[MK1_MM_NUM];
extern int giBuyOrSell;

#endif




