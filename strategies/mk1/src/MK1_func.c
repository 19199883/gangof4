#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <memory.h>
#include "common.h"
#include "common_mod.h"
#include "common_func.h"
#include "common_mm.h"
#include "interface_v30.h"
#include "MK1_var_def.h"
#include "MK1_func.h"



int giSingleOrderMaxVol = 1;
int giStopLossTickNum = 2;
double garAveBSV1[100000];
int giBSV1Num;
double garActBuy[100000];
double garActSel[100000];
int giActBuyNum;
int giActSelNum;

int giPreActNum = 5;
double giPreActBuy[5];
double giPreActSel[5];

//  day
//double grParameter1 = 100;  // ta
//double grParameter1 = 50;  // sr
//double grParameter1 = 50;  // cf
//double grParameter1 = 400;  // ma
//double grParameter1 = 250;  // rm
//double grParameter1 = 12;  // oi


// night
//double grParameter1 = 150;  // ta
//double grParameter1 = 90;  // sr
//double grParameter1 = 50;  // cf
//double grParameter1 = 450;  // ma
//double grParameter1 = 220;  // rm
//double grParameter1 = 12;  // oi



//  day
//double grParameter1 = 100;  // ta
//double grParameter1 = 50;  // sr
//double grParameter1 = 50;  // cf
//double grParameter1 = 150;  // ma
//double grParameter1 = 250;  // rm
//double grParameter1 = 12;  // oi


// night
//double grParameter1 = 150;  // ta
//double grParameter1 = 90;  // sr
//double grParameter1 = 50;  // cf
//double grParameter1 = 450;  // ma
//double grParameter1 = 220;  // rm
//double grParameter1 = 12;  // oi



int *giPriceMarkBuy, *giPriceMarkSel;


struct struct_mm_outdata gastMK1OutData[iOUT_LEN];
int giMK1OutDataIndex, giMK1TraceInit;
struct struct_MK1_paras gstMK1_Paras;

int gilastThreeRec[3];
double grlastThreeRecPrice[3];
int gilastThreeRecTime[3];
int giZhangDieTingTag = 0;

//DLi related
int giStatus; //0: 无任何摆单， 1：摆单买2卖2
double grLastBP[MK1_MM_NUM];
double grLastSP[MK1_MM_NUM];
int giLastBV[MK1_MM_NUM];
int giLastSV[MK1_MM_NUM];
double grLastPrice[MK1_MM_NUM];
int giCoverTime;
double grHitBuyP, grHitBuyP2;
double grHitSelP, grHitSelP2;
int giStopLoss;
double grActiveVolBuy[MK1_MM_NUM];
double grActiveVolSel[MK1_MM_NUM];
int giBuyOrSell;
//DLi related end

int *giWaterflow, giSumWaterflow, giWaterflowMax, giWaterflowMin;

void s_init_MK1_parameters()
{

	memset(&gstLocTradeInfo, 0, sizeof(st_loc_trade_info));
	memset(&gastMK1OutData, 0, sizeof(struct struct_mm_outdata)*iOUT_LEN);

	giMK1OutDataIndex = 0;
	giMK1TraceInit = 0;

	giPriceMarkBuy = (int*)malloc(sizeof(int)*(giMaxMMlevel+2));
	giPriceMarkSel = (int*)malloc(sizeof(int)*(giMaxMMlevel+2));

	gstMK1_Paras.rBVSum = 0;
	gstMK1_Paras.rSVSum = 0;
	gstMK1_Paras.rStopLossThreshold = 0.1;
	gstMK1_Paras.rVolBuy = 0;
	gstMK1_Paras.rVolSel = 0 ;
	gstMK1_Paras.iStop = 0;
	gstMK1_Paras.iUPstop = 0;
	gstMK1_Paras.iDNstop = 0 ;

	if (gcExch == cCFFE)
	{
		gstMK1_Paras.iLimitRange = 10;
		gstMK1_Paras.iLimitSqRange = 8 ;
	}
	else
	{
		gstMK1_Paras.iLimitRange = 5;
		gstMK1_Paras.iLimitSqRange = 10;
	}
	gstMK1_Paras.iLimitSqFlg = 0;
	gstMK1_Paras.iWithClose = 1;

	gaiFullOrderBookBuy = NULL;
	gaiFullOrderBookSel = NULL;
	giWaterflow = NULL;

	gilastThreeRec[0] = 0;
	gilastThreeRec[1] = 0;
	gilastThreeRec[2] = 0;
	grlastThreeRecPrice[0] = 0;
	grlastThreeRecPrice[1] = 0;
	grlastThreeRecPrice[2] = 0;
	gilastThreeRecTime[0] = 0;
	gilastThreeRecTime[1] = 0;
	gilastThreeRecTime[2] = 0;

	giZhangDieTingTag = 0;
}

void s_destroy_MK1_parameters()
{
    RELEASE(giPriceMarkBuy);
    RELEASE(giPriceMarkSel);
    RELEASE(gaiFullOrderBookBuy);
    RELEASE(gaiFullOrderBookSel);
    RELEASE(giWaterflow);
}

int update_lastThreeRec(int iBS, double rPrice)
{
    int i;

    if(iBS == gilastThreeRec[0] && fabs(rPrice - grlastThreeRecPrice[0]) < 0.1*gstGlobalPara.stSec.rTickSize) return 0;

    for(i=2; i>0; i--)
    {
        gilastThreeRec[i] = gilastThreeRec[i-1];
        grlastThreeRecPrice[i] = grlastThreeRecPrice[i-1];
        gilastThreeRecTime[i] = gilastThreeRecTime[i-1];
    }
    gilastThreeRec[0] = iBS;
    grlastThreeRecPrice[0] = rPrice;
    gilastThreeRecTime[0] = gstCurrIn.iTime;

    return 0;
}

int s_divide_curr_volumn(struct in_data *p_stDataIn, struct in_data *p_stPreDataIn,
                         struct all_parameter *p_stGlobalPara,
                         double *pr_PriceLo, double *pr_PriceHi,int *pVolLo, int *pVolHi)
{
    int liVLo, liVHi;
    double lrTradeP, lrPriceL, lrPriceH;

    double lrTick = p_stGlobalPara->stSec.rTickSize ;

    if (gcExch == cCZCE)
	{
		p_stDataIn->dVal  = 0;
		p_stDataIn->dAvgP = p_stDataIn->dP;
	}
	else
	{
		p_stDataIn->dVal = (p_stDataIn->dTotVal - p_stPreDataIn->dTotVal)/p_stGlobalPara->stExch.iVolDivider;
		p_stDataIn->dAvgP = f_calc_curr_avgp(p_stDataIn->dVal, p_stDataIn->iVol, p_stPreDataIn->dAvgP)/
			p_stGlobalPara->stSec.iUnitPerContract;
	}
	lrTradeP = p_stDataIn->dAvgP;

    if(lrTradeP >= p_stDataIn->dSP[0])
    {
        lrPriceL = p_stDataIn->dSP[0] - lrTick;
        lrPriceH = p_stDataIn->dSP[0];
        liVLo = 0;
        liVHi = p_stDataIn->iVol;
    }
    else if(lrTradeP <= p_stDataIn->dBP[0])
    {
        lrPriceL = p_stDataIn->dBP[0];
        lrPriceH = p_stDataIn->dBP[0]+lrTick;
        liVLo = p_stDataIn->iVol;
        liVHi = 0;
    }
    else
    {
        lrPriceL = (int)(lrTradeP/lrTick)*lrTick ;   // immediate tick price below tradep
        lrPriceH = lrPriceL + lrTick ;             // immediate tick price over tradep
        liVLo = (int)(p_stDataIn->iVol * (lrPriceH - lrTradeP) / lrTick + 0.5);   // last trade vol @ priceL
        liVHi = p_stDataIn->iVol - liVLo;   // last trade vol @ priceH
    }

    *pr_PriceLo = lrPriceL;
    *pr_PriceHi = lrPriceH;
    *pVolLo = liVLo;
    *pVolHi = liVHi;

    return 0;
}

int s_update_vol_waterflow(struct in_data *lstCurrInData, struct in_data *lstPreInData,
	struct all_parameter *lstGlobalPara)
{
    int liIndex;
    int iVolLo, iVolHi;
	double dPriceLo, dPriceHi;
	double lrTick;

	lrTick = lstGlobalPara->stSec.rTickSize;

    s_divide_curr_volumn(lstCurrInData, lstPreInData, lstGlobalPara, &dPriceLo, &dPriceHi, &iVolLo, &iVolHi);
    liIndex = (int)((dPriceLo-lstCurrInData->dPLmtDn)/lrTick+0.5);
    giWaterflow[liIndex] += iVolLo;
    if(giWaterflowMin == 0)
    {
        giWaterflowMin = liIndex;
    }
    else
        giWaterflowMin = MIN(giWaterflowMin, liIndex);

    liIndex = (int)((dPriceHi-lstCurrInData->dPLmtDn)/lrTick+0.5);
    giWaterflow[liIndex] += iVolHi;
    if(giWaterflowMax == 0)
    {
        giWaterflowMax = liIndex;
    }
    else
        giWaterflowMax = MAX(giWaterflowMax, liIndex);


    giSumWaterflow += (iVolLo+iVolHi);

    return 0;
}

//----------------------------------------------------
// subroutine s_hi5_sig_gen()
// description: set Hi78 model signal generation
//----------------------------------------------------
int s_MK1_sig_gen(struct in_data *lstCurrInData, struct in_data *lstPreInData,
	struct all_parameter *lstGlobalPara, st_loc_trade_info *p_stLocTradeInfo,
	int liTickNo, int *laiFullOrderBookBuy, int *laiFullOrderBookSel,
	int liLenFullOrderBook, st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder)
{
#ifdef WRITE_SYSLOG_
    syslog(LOG_ERR, "do s_MK1_sig_gen() %i %i\n",lstCurrInData->iTime, lstGlobalPara->stSec.iSecSqTime);
#endif // WRITE_SYSLOG_


    s_update_vol_waterflow(lstCurrInData, lstPreInData, lstGlobalPara);


    if(lstCurrInData->iTime > lstGlobalPara->stSec.iSecFinalSqTime * 1000)
    {
        s_MK1_eod_final_square(lstCurrInData, lstGlobalPara, p_stLocTradeInfo, liTickNo,
            stCurUsrOrder, iCurUsrOrder, stNewUsrOrder, iMaxUsrOrder, piNewUsrOrder);
    }
    else if((lstGlobalPara->stSec.iSecTradeEndTime02 == -1 && lstCurrInData->iTime >= lstGlobalPara->stSec.iSecTradeEndTime *1000)
            || (lstGlobalPara->stSec.iSecTradeEndTime02 > 0 && lstCurrInData->iTime >= lstGlobalPara->stSec.iSecTradeEndTime02 *1000)
            || giZhangDieTingTag == 1)
    {
        s_MK1_eod_square(lstCurrInData, lstGlobalPara, p_stLocTradeInfo, liTickNo,
            stCurUsrOrder, iCurUsrOrder, stNewUsrOrder, iMaxUsrOrder, piNewUsrOrder);
    }
    else if((lstGlobalPara->stSec.iSecTradeStartTime02 != -1
            && lstCurrInData->iTime >= lstGlobalPara->stSec.iSecTradeEndTime *1000
            && lstCurrInData->iTime < lstGlobalPara->stSec.iSecTradeStartTime02 * 1000))
    {
        s_MK1_eod_square_flat_netpos(lstCurrInData, lstGlobalPara, p_stLocTradeInfo, liTickNo,
            stCurUsrOrder, iCurUsrOrder, stNewUsrOrder, iMaxUsrOrder, piNewUsrOrder);
    }
    else
    {
        s_MK1_sig_gen_normal(lstCurrInData, lstPreInData, lstGlobalPara, p_stLocTradeInfo,
             liTickNo, laiFullOrderBookBuy, laiFullOrderBookSel, liLenFullOrderBook,
             stCurUsrOrder, iCurUsrOrder, stNewUsrOrder, iMaxUsrOrder, piNewUsrOrder);
    }

	return 0;
}


int s_MK1_sig_gen_normal(struct in_data *lstCurrInData, struct in_data *lstPreInData,
	struct all_parameter *lstGlobalPara, st_loc_trade_info *p_stLocTradeInfo,
	int liTickNo, int *laiFullOrderBookBuy, int *laiFullOrderBookSel,
	int liLenFullOrderBook, st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder)
{

	double larBP[30], larSP[30], lrTick; //, lrOI;
	//double lrOrderP;
	int laiBV[30], laiSV[30], laiImpBV[30], laiImpSV[30];
	int  i, i1, i2;
	int inew;
	int iCurSelOrder, iCurBuyOrder;
	//int iRemainBuyOrder, iRemainSelOrder;
	//int iDisplaceBuy, iDisplaceSel;
	//int iMark;
	//int liNetPos;
	int iFlatVol;
	//int liTagTemp, liTagTempB, liTagTempS;
	//double lrTradeP;
	//double lrVolBuy, lrVolSel;
	//int liTotBuyVol, liTotSelVol;
	//int liActBuyTag, liActSelTag;

	//data prepare begin
	*piNewUsrOrder = 0;
	inew = 0;
	memset(&(stNewUsrOrder[0]), 0, sizeof(st_usr_order) * iMaxUsrOrder);

	lrTick = lstGlobalPara->stSec.rTickSize;
	memcpy(larBP, lstCurrInData->dBP, sizeof(double)*30);
	memcpy(larSP, lstCurrInData->dSP, sizeof(double)*30);
	memcpy(laiBV, lstCurrInData->iBV, sizeof(int)*30);
	memcpy(laiSV, lstCurrInData->iSV, sizeof(int)*30);
	memcpy(laiImpBV, lstCurrInData->iImpBV, sizeof(int)*30);
	memcpy(laiImpSV, lstCurrInData->iImpSV, sizeof(int)*30);

    s_update_full_order_book(lstCurrInData, lstPreInData, lstGlobalPara,
           laiFullOrderBookBuy, laiFullOrderBookSel, liLenFullOrderBook);

    iCurBuyOrder = 0;
    iCurSelOrder = 0;
    for(i1=0; i1<iCurUsrOrder; i1++)
    {
        if(stCurUsrOrder[i1].iBS == USER_ORDER_SELL) iCurSelOrder ++;
        else if(stCurUsrOrder[i1].iBS == USER_ORDER_BUY) iCurBuyOrder ++;
    }
	// data prepare end


	if(lstCurrInData->dBP[0] > lrTick && lstCurrInData->dSP[0] > lrTick)
	{
		if(giBSV1Num > 0)
		{
			for(i = 0; i < giBSV1Num; i++)
			{
				if((lstCurrInData->iBV[0] + lstCurrInData->iSV[0])/2.0 < garAveBSV1[i]) break;
			}
			if(i != giBSV1Num) memcpy(&garAveBSV1[i+1], &garAveBSV1[i], sizeof(double)*(giBSV1Num-i));
			garAveBSV1[i] = (lstCurrInData->iBV[0] + lstCurrInData->iSV[0])/2.0;
		}
		else
		{
			garAveBSV1[giBSV1Num] = (lstCurrInData->iBV[0] + lstCurrInData->iSV[0])/2.0;
		}
		giBSV1Num ++;
	}

    // divide the order to last BP1 and SP1
	//lrTradeP = lstCurrInData->dAvgP;
	//if(fabs(lstCurrInData->dBP[0] - lstPreInData->dBP[0]) < 0.2 * lrTick && fabs(lstCurrInData->dSP[0] - lstPreInData->dSP[0]) < 0.2 * lrTick)
	//{
	//	if (lrTradeP > lstPreInData->dSP[0]) lrTradeP = lstPreInData->dSP[0];
	//	if (lrTradeP < lstPreInData->dBP[0]) lrTradeP = lstPreInData->dBP[0];
	//	if (fabs(lstPreInData->dSP[0] - lstPreInData->dBP[0]) < 0.2 * lrTick)
	//	{
	//		lrVolBuy = 0;
	//		lrVolSel = 0;
	//	}else
	//	{
	//		lrVolBuy = lstCurrInData->iVol * (lstPreInData->dSP[0] - lrTradeP)/(lstPreInData->dSP[0] - lstPreInData->dBP[0]);
	//		lrVolSel = lstCurrInData->iVol - lrVolBuy;
	//	}
	//}
	//else
	//{
	//	lrVolBuy = 0;
	//	lrVolSel = 0;
	//}

 //   if (liTickNo == 1)
	//{
	//	lrVolBuy = 0;
	//	lrVolSel = 0;
	//}

	//if(lrVolBuy > 0.5)
	//{
	//	if(giActBuyNum > 0)
	//	{
	//		for(i = 0; i < giActBuyNum; i++)
	//		{
	//			if(lrVolBuy > garActBuy[i]) break;
	//		}
	//		if(i != giActBuyNum) memcpy(&garActBuy[i+1], &garActBuy[i], sizeof(double)*(giActBuyNum-i));
	//		garActBuy[i] = lrVolBuy;
	//	}
	//	else
	//	{
	//		garActBuy[giActBuyNum] = lrVolBuy;
	//	}
	//	giActBuyNum ++;
	//}

	//if(lrVolSel > 0.5)
	//{
	//	if(giActSelNum > 0)
	//	{
	//		for(i = 0; i < giActSelNum; i++)
	//		{
	//			if(lrVolSel > garActSel[i]) break;
	//		}
	//		if(i != giActSelNum) memcpy(&garActSel[i+1], &garActSel[i], sizeof(double)*(giActSelNum-i));
	//		garActSel[i] = lrVolSel;
	//	}
	//	else
	//	{
	//		garActSel[giActSelNum] = lrVolSel;
	//	}
	//	giActSelNum ++;
	//}

	//for(i = (giPreActNum - 1); i > 0; i--)
	//{
	//	giPreActBuy[i] = giPreActBuy[i-1];
	//	giPreActSel[i] = giPreActSel[i-1];
	//}
	//giPreActBuy[0] = lrVolBuy;
	//giPreActSel[0] = lrVolSel;


	//for(i = MK1_MM_NUM-1 ; i>0; i--)
	//{
	//	grActiveVolBuy[i] = grActiveVolBuy[i-1];
	//	grActiveVolSel[i] = grActiveVolSel[i-1];
	//}
	//grActiveVolBuy[0] = lrVolSel;
	//grActiveVolSel[0] = lrVolBuy;
	// divide the order to last BP1 and SP1 end

    gstMK1_Paras.iLimitSqFlg = iNO;
	//if (fabs(larBP[4] - lstCurrInData->dPLmtDn) < gstMK1_Paras.iLimitSqRange * lrTick || larBP[4]<lrTick)
	if(fabs(lstCurrInData->dBP1 - lstCurrInData->dPLmtDn)<(gstMK1_Paras.iLimitSqRange+3)*lrTick ||
        lstCurrInData->dBP1 < lrTick)
	{
	    gstMK1_Paras.iLimitSqFlg = iYES;
		giZhangDieTingTag = 1;
        goto END;
	}
	//else if (fabs(larSP[4] - lstCurrInData->dPLmtUp) < gstMK1_Paras.iLimitSqRange * lrTick || larSP[4] < lrTick)
    else if (fabs(lstCurrInData->dSP1 - lstCurrInData->dPLmtUp) < (gstMK1_Paras.iLimitSqRange+3) * lrTick ||
        lstCurrInData->dSP1 < lrTick)
    {
        gstMK1_Paras.iLimitSqFlg = iYES ;
        giZhangDieTingTag = 1;
        goto END;
	}


    if (lstCurrInData->iTime < lstGlobalPara->stSec.iSecTradeStartTime * 1000)
    {
        //printf("%d %d\n",lstCurrInData->iTime,lstGlobalPara->stSec.iSecTradeStartTime);
        goto END;
    }


	//liActBuyTag = 0; liActSelTag = 0;
	//for(i=0; i<giPreActNum; i++)
	//{
	//	if(giPreActBuy[i] > MIN(garActBuy[giActBuyNum/500], garActSel[giActSelNum/500]) && giActBuyNum > 2000) liActBuyTag = 1;
	//	if(giPreActSel[i] > MIN(garActBuy[giActBuyNum/500], garActSel[giActSelNum/500]) && giActSelNum > 2000) liActSelTag = 1;
	//}

	if(iCurBuyOrder == 0 && iCurSelOrder == 0 && (p_stLocTradeInfo->iPosK - p_stLocTradeInfo->iPosD) == 0)  giStatus = 0;

	if(giStatus == 0)
	{
		//if(laiBV[0] < grParameter1 && laiSV[0] < grParameter1 && larBP[0] > lrTick && larSP[0] > lrTick)
		if(laiBV[0] < garAveBSV1[giBSV1Num/15] && laiSV[0] < garAveBSV1[giBSV1Num/15] && larBP[0] > lrTick && larSP[0] > lrTick && giBSV1Num > 200)
		{
			//if(liActBuyTag == 0 && liActSelTag == 0)
			//{
				stNewUsrOrder[inew].dPrice = larBP[0];
				//stNewUsrOrder[inew].iVol = giSingleOrderMaxVol;
				stNewUsrOrder[inew].iVol = giMaxPos;
				stNewUsrOrder[inew].iBS = USER_ORDER_BUY;
				stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;
				inew ++;
				grHitBuyP = stNewUsrOrder[inew-1].dPrice;

				stNewUsrOrder[inew].dPrice = larSP[0];
				//stNewUsrOrder[inew].iVol = giSingleOrderMaxVol;
				stNewUsrOrder[inew].iVol = giMaxPos;
				stNewUsrOrder[inew].iBS = USER_ORDER_SELL;
				stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;
				inew ++;
				grHitSelP = stNewUsrOrder[inew-1].dPrice;

				giStatus = 1;
			//}

			goto END;
		}
	}

	//if(giStatus == 0)
	//{
	//	if(laiBV[0] < 100 && laiSV[0]+laiSV[1] < 200 && larBP[0] > lrTick && larSP[1] > lrTick & lrVolSel > lrVolBuy)
	//	{
	//		stNewUsrOrder[inew].dPrice = larBP[0];
	//		stNewUsrOrder[inew].iVol = giSingleOrderMaxVol;
	//		stNewUsrOrder[inew].iBS = USER_ORDER_BUY;
	//		stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;
	//		inew ++;
	//		grHitBuyP = stNewUsrOrder[inew-1].dPrice;

	//		stNewUsrOrder[inew].dPrice = larSP[1];
	//		stNewUsrOrder[inew].iVol = giSingleOrderMaxVol;
	//		stNewUsrOrder[inew].iBS = USER_ORDER_SELL;
	//		stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;
	//		inew ++;
	//		grHitSelP = stNewUsrOrder[inew-1].dPrice;

	//		giStatus = 1;
	//		goto END;
	//	}


	//	if(laiSV[0] < 100 && laiBV[0]+laiBV[1] < 200 && larSP[0] > lrTick && larBP[1] > lrTick & lrVolBuy > lrVolSel)
	//	{
	//		stNewUsrOrder[inew].dPrice = larBP[1];
	//		stNewUsrOrder[inew].iVol = giSingleOrderMaxVol;
	//		stNewUsrOrder[inew].iBS = USER_ORDER_BUY;
	//		stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;
	//		inew ++;
	//		grHitBuyP = stNewUsrOrder[inew-1].dPrice;

	//		stNewUsrOrder[inew].dPrice = larSP[0];
	//		stNewUsrOrder[inew].iVol = giSingleOrderMaxVol;
	//		stNewUsrOrder[inew].iBS = USER_ORDER_SELL;
	//		stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;
	//		inew ++;
	//		grHitSelP = stNewUsrOrder[inew-1].dPrice;

	//		giStatus = 1;
	//		goto END;
	//	}

	//}


	if(giStatus == 1)
	{
		if(iCurBuyOrder == 0 && iCurSelOrder == 0 && (p_stLocTradeInfo->iPosK - p_stLocTradeInfo->iPosD) == 0)
		{
			giStatus = 0;
			goto END;
		}
		//if(iCurBuyOrder == 0 && iCurSelOrder != 0 && liActBuyTag == 1)
		//{
		//	giStatus = 2;
		//}
		//if(iCurBuyOrder != 0 && iCurSelOrder == 0 && liActSelTag == 1)
		//{
		//	giStatus = 2;
		//}
		if(grHitBuyP - larSP[0] > (giStopLossTickNum + 0.2) * lrTick)
		{
			giStatus = 2;
		}else if(larBP[0] - grHitSelP > (giStopLossTickNum + 0.2) * lrTick)
		{
			giStatus = 2;
		}
	}

	if(giStatus == 2)
	{
		if(iCurBuyOrder == 0 && iCurSelOrder == 0 && (p_stLocTradeInfo->iPosK - p_stLocTradeInfo->iPosD) == 0)
		{
			giStatus = 0;
			goto END;
		}

		iFlatVol = 0;
		for(i=0; i<iCurUsrOrder; i++)
		{
			if(stCurUsrOrder[i].iBS == USER_ORDER_SELL)
			{
				if(fabs(stCurUsrOrder[i].dPrice - lstCurrInData->dSP[0]) > 0.01 || stCurUsrOrder[i].iOpenFlat != USER_ORDER_OPT_FLAT)
				{
					stNewUsrOrder[inew].dPrice = stCurUsrOrder[i].dPrice;
					stNewUsrOrder[inew].iBS = USER_ORDER_CANCEL;
					stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;

					inew ++;
				}
				else
				{
					iFlatVol = stCurUsrOrder[i].iVol;
				}
			}
		}
		if((p_stLocTradeInfo->iPosD - p_stLocTradeInfo->iPosK)- iFlatVol > 0)
		{
			stNewUsrOrder[inew].dPrice = lstCurrInData->dSP[0];
			stNewUsrOrder[inew].iBS = USER_ORDER_SELL;
			stNewUsrOrder[inew].iVol = (p_stLocTradeInfo->iPosD - p_stLocTradeInfo->iPosK)- iFlatVol;
			stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_FLAT;

			inew ++;
		}

		iFlatVol = 0;
		for(i=0; i<iCurUsrOrder; i++)
		{
			if(stCurUsrOrder[i].iBS == USER_ORDER_BUY)
			{
				if(fabs(stCurUsrOrder[i].dPrice - lstCurrInData->dBP[0]) > 0.01 || stCurUsrOrder[i].iOpenFlat != USER_ORDER_OPT_FLAT)
				{
					stNewUsrOrder[inew].dPrice = stCurUsrOrder[i].dPrice;
					stNewUsrOrder[inew].iBS = USER_ORDER_CANCEL;
					stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;

					inew ++;
				}
				else
				{
					iFlatVol = stCurUsrOrder[i].iVol;
				}
			}
		}
		if((p_stLocTradeInfo->iPosK - p_stLocTradeInfo->iPosD)- iFlatVol > 0)
		{
			stNewUsrOrder[inew].dPrice = lstCurrInData->dBP[0];
			stNewUsrOrder[inew].iBS = USER_ORDER_BUY;
			stNewUsrOrder[inew].iVol = (p_stLocTradeInfo->iPosK - p_stLocTradeInfo->iPosD)- iFlatVol;
			stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_FLAT;

			inew ++;
		}

	}



END:    *piNewUsrOrder = inew;

    return 0;
}




int s_generate_Mk1_cancel_orders(double *larBP, double *larSP, double lrTick, int iCurTime,
    st_loc_trade_info *p_stLocTradeInfo, st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder)
{

    *piNewUsrOrder = 0;

    return 0;
}


int transfer_2_seconds(int itime)
{
    int isec;
    int ihour, imin;
    ihour = itime/10000000;
    imin = (itime%10000000)/100000;
    isec = (itime%100000)/1000;
    return ihour*3600 + imin*60 + isec;
}


//----------------------------------------------------
// subroutine s_hi5_eod_square()
// description: square at the end of the market
//----------------------------------------------------
int s_MK1_eod_square(const struct in_data *lstCurrInData, const struct all_parameter *lstGlobalPara,
	st_loc_trade_info *p_stLocTradeInfo, const int liTickNo,
	st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder)
{
	int i;
	int inew;
	int iFlatVol;

	*piNewUsrOrder = 0;
	inew = 0;

	iFlatVol = 0;
    for(i=0; i<iCurUsrOrder; i++)
    {
        if(stCurUsrOrder[i].iBS == USER_ORDER_SELL)
        {
            if(fabs(stCurUsrOrder[i].dPrice - lstCurrInData->dSP[0]) > 0.01 || stCurUsrOrder[i].iOpenFlat != USER_ORDER_OPT_FLAT)
            {
                stNewUsrOrder[inew].dPrice = stCurUsrOrder[i].dPrice;
                stNewUsrOrder[inew].iBS = USER_ORDER_CANCEL;
                stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;

                inew ++;
            }
            else
            {
                iFlatVol = stCurUsrOrder[i].iVol;
            }
        }
    }
    if(p_stLocTradeInfo->iPosD - iFlatVol > 0)
    {
        stNewUsrOrder[inew].dPrice = lstCurrInData->dSP[0];
        stNewUsrOrder[inew].iBS = USER_ORDER_SELL;
        stNewUsrOrder[inew].iVol = p_stLocTradeInfo->iPosD - iFlatVol;
        stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_FLAT;

        inew ++;
    }

    iFlatVol = 0;
    for(i=0; i<iCurUsrOrder; i++)
    {
        if(stCurUsrOrder[i].iBS == USER_ORDER_BUY)
        {
            if(fabs(stCurUsrOrder[i].dPrice - lstCurrInData->dBP[0]) > 0.01 || stCurUsrOrder[i].iOpenFlat != USER_ORDER_OPT_FLAT)
            {
                stNewUsrOrder[inew].dPrice = stCurUsrOrder[i].dPrice;
                stNewUsrOrder[inew].iBS = USER_ORDER_CANCEL;
                stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;

                inew ++;
            }
            else
            {
                iFlatVol = stCurUsrOrder[i].iVol;
            }
        }
    }
    if(p_stLocTradeInfo->iPosK - iFlatVol > 0)
    {
        stNewUsrOrder[inew].dPrice = lstCurrInData->dBP[0];
        stNewUsrOrder[inew].iBS = USER_ORDER_BUY;
        stNewUsrOrder[inew].iVol = p_stLocTradeInfo->iPosK - iFlatVol;
        stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_FLAT;

        inew ++;
    }

    *piNewUsrOrder = inew;
	return 0;
}

int s_MK1_eod_square_flat_netpos(const struct in_data *lstCurrInData, const struct all_parameter *lstGlobalPara,
	st_loc_trade_info *p_stLocTradeInfo, const int liTickNo,
	st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder)
{
	int i;
	int inew;
	int iFlatVol;

	*piNewUsrOrder = 0;
	inew = 0;

	iFlatVol = 0;
    for(i=0; i<iCurUsrOrder; i++)
    {
        if(stCurUsrOrder[i].iBS == USER_ORDER_SELL)
        {
            if(fabs(stCurUsrOrder[i].dPrice - lstCurrInData->dSP[0]) > 0.01 || stCurUsrOrder[i].iOpenFlat != USER_ORDER_OPT_FLAT)
            {
                stNewUsrOrder[inew].dPrice = stCurUsrOrder[i].dPrice;
                stNewUsrOrder[inew].iBS = USER_ORDER_CANCEL;
                stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;

                inew ++;
            }
            else
            {
                iFlatVol = stCurUsrOrder[i].iVol;
            }
        }
    }
    if((p_stLocTradeInfo->iPosD - p_stLocTradeInfo->iPosK)- iFlatVol > 0)
    {
        stNewUsrOrder[inew].dPrice = lstCurrInData->dSP[0];
        stNewUsrOrder[inew].iBS = USER_ORDER_SELL;
        stNewUsrOrder[inew].iVol = (p_stLocTradeInfo->iPosD - p_stLocTradeInfo->iPosK)- iFlatVol;
        stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_FLAT;

        inew ++;
    }

    iFlatVol = 0;
    for(i=0; i<iCurUsrOrder; i++)
    {
        if(stCurUsrOrder[i].iBS == USER_ORDER_BUY)
        {
            if(fabs(stCurUsrOrder[i].dPrice - lstCurrInData->dBP[0]) > 0.01 || stCurUsrOrder[i].iOpenFlat != USER_ORDER_OPT_FLAT)
            {
                stNewUsrOrder[inew].dPrice = stCurUsrOrder[i].dPrice;
                stNewUsrOrder[inew].iBS = USER_ORDER_CANCEL;
                stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;

                inew ++;
            }
            else
            {
                iFlatVol = stCurUsrOrder[i].iVol;
            }
        }
    }
    if((p_stLocTradeInfo->iPosK - p_stLocTradeInfo->iPosD)- iFlatVol > 0)
    {
        stNewUsrOrder[inew].dPrice = lstCurrInData->dBP[0];
        stNewUsrOrder[inew].iBS = USER_ORDER_BUY;
        stNewUsrOrder[inew].iVol = (p_stLocTradeInfo->iPosK - p_stLocTradeInfo->iPosD)- iFlatVol;
        stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_FLAT;

        inew ++;
    }

    *piNewUsrOrder = inew;
	return 0;
}


int s_MK1_eod_final_square(const struct in_data *lstCurrInData, const struct all_parameter *lstGlobalPara,
	st_loc_trade_info *p_stLocTradeInfo, const int liTickNo,
	st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder)
{
	int i;
	int inew;
	int iFlatVol;

	*piNewUsrOrder = 0;
	inew = 0;

	iFlatVol = 0;
    for(i=0; i<iCurUsrOrder; i++)
    {
        if(stCurUsrOrder[i].iBS == USER_ORDER_SELL)
        {
            stNewUsrOrder[inew].dPrice = stCurUsrOrder[i].dPrice;
            stNewUsrOrder[inew].iBS = USER_ORDER_CANCEL;
            stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;

            inew ++;
        }
    }
    if(p_stLocTradeInfo->iPosD - iFlatVol > 0)
    {
        stNewUsrOrder[inew].dPrice = lstCurrInData->dBP[0];
        stNewUsrOrder[inew].iBS = USER_ORDER_SELL;
        stNewUsrOrder[inew].iVol = p_stLocTradeInfo->iPosD - iFlatVol;
        stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_FLAT;

        inew ++;
    }

    iFlatVol = 0;
    for(i=0; i<iCurUsrOrder; i++)
    {
        if(stCurUsrOrder[i].iBS == USER_ORDER_BUY)
        {
            stNewUsrOrder[inew].dPrice = stCurUsrOrder[i].dPrice;
            stNewUsrOrder[inew].iBS = USER_ORDER_CANCEL;
            stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_NONE;

            inew ++;
        }
    }
    if(p_stLocTradeInfo->iPosK - iFlatVol > 0)
    {
        stNewUsrOrder[inew].dPrice = lstCurrInData->dSP[0];
        stNewUsrOrder[inew].iBS = USER_ORDER_BUY;
        stNewUsrOrder[inew].iVol = p_stLocTradeInfo->iPosK - iFlatVol;
        stNewUsrOrder[inew].iOpenFlat = USER_ORDER_OPT_FLAT;

        inew ++;
    }

    *piNewUsrOrder = inew;
	return 0;
}

int s_MK1_calc_profitloss(st_order_info *p_stOrder, struct sec_parameter *p_stSecPara,
    st_loc_trade_info *p_stLocTradeInfo)
{
	int liTradedSize;
	double lrCost, lrPrice;

    if(p_stOrder->exec_vol == 0) return 0;

	liTradedSize = p_stOrder->exec_vol;
	lrPrice = p_stOrder->exec_price;

	lrCost = p_stSecPara->rRssTCostByVol *liTradedSize + p_stSecPara->rRssTCostByVal* liTradedSize * lrPrice;

	if(p_stOrder->direct == SIG_DIRECTION_BUY)
    {
        p_stLocTradeInfo->rCash = p_stLocTradeInfo->rCash - lrPrice * liTradedSize - lrCost;
        p_stLocTradeInfo->rTotalCost = p_stLocTradeInfo->rTotalCost + lrCost;
    }
    else if(p_stOrder->direct == SIG_DIRECTION_SELL)
    {
        p_stLocTradeInfo->rCash = p_stLocTradeInfo->rCash + lrPrice * liTradedSize - lrCost;
        p_stLocTradeInfo->rTotalCost = p_stLocTradeInfo->rTotalCost + lrCost;
    }

    p_stLocTradeInfo->iTotalVol += p_stOrder->exec_vol;

    //update order info
    p_stOrder->exec_vol = 0;
    p_stOrder->exec_price = 0;
    if(p_stOrder->exec_acc_vol == p_stOrder->volume || p_stOrder->status == SIG_STATUS_SUCCESS)
        memset(p_stOrder, 0, sizeof(st_order_info));

	return 0;
}


int s_MK1_run_each_record(struct signal_t *lastStructTSignal, int *piNoTSignal)
{
#ifdef WRITE_SYSLOG_
    syslog(LOG_ERR, "do s_MK1_run_each_record() giInitFlg=%d\n",giInitFlg);
#endif // WRITE_SYSLOG_
    int iTemp;
	int i;

    *piNoTSignal = 0;
	// init  variables
	if (giInitFlg == 0)
	{
		if (gstCurrIn.dPLmtUp < 0.001 || gstCurrIn.dPLmtDn < 0.001) goto END;
		if (gcExch == cSHFE && gstCurrIn.iData_Flag == 2) goto END;
		f_find_security_name(gcSecName,gstCurrIn.cTicker,gcExch);
		s_set_global_parameters();
		s_MK1_variable_init(&gstCurrIn, &gstGlobalPara);

		memcpy(&gstPreIn, (void*)(&gstCurrIn), sizeof(struct in_data));
		memcpy(&gstRecPreIn, (void*)(&gstRecCurrIn), sizeof(struct in_data));
		giInitFlg = 1;
	}

	if (gcExch != cCZCE)
	{
		gstCurrIn.iVol = (int)(gstCurrIn.dTotV - gstPreIn.dTotV)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dVal = (gstCurrIn.dTotVal - gstPreIn.dTotVal)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dOIChg = (gstCurrIn.dOI - gstPreIn.dOI)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dAvgP = f_calc_curr_avgp(gstCurrIn.dVal, gstCurrIn.iVol, gstPreIn.dAvgP)/gstGlobalPara.stSec.iUnitPerContract;
	}
	else
	{
		gstCurrIn.iVol = (int)(gstCurrIn.dTotV - gstPreIn.dTotV)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dOIChg = (gstCurrIn.dOI - gstPreIn.dOI)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dAvgP = gstCurrIn.dP ;
		gstCurrIn.dVal = 0;
	}
	if (f_check_valid_data(&gstCurrIn, &gstRecPreIn, giTickNo, &gstGlobalPara) == iNO) goto END;   // check whether data is valid
	if (gstCurrIn.iData_Flag != 3 && gcExch == cSHFE) goto END;
	giTickNo = giTickNo + 1;  // tick number add 1


	for(i = MK1_MM_NUM-1 ; i>0; i--)
	{
		grLastBP[i] = grLastBP[i-1];
		grLastSP[i] = grLastSP[i-1];
		giLastBV[i] = giLastBV[i-1];
		giLastSV[i] = giLastSV[i-1];
		grLastPrice[i] = grLastPrice[i-1];
	}
	grLastBP[0] = gstCurrIn.dBP[0];
	grLastSP[0] = gstCurrIn.dSP[0];
	giLastBV[0] = gstCurrIn.iBV[0];
	giLastSV[0] = gstCurrIn.iSV[0];
	grLastPrice[0] = gstCurrIn.dP;

    //if(giTickNo%100 == 1)
	//printf("%d %0.2f %d %0.2f %d %0.2f %d %0.2f %d\n",gstCurrIn.iTime, gstCurrIn.dBP[0],gstCurrIn.iBV[0], gstCurrIn.dSP[0],gstCurrIn.iSV[0]
    //    , gstCurrIn.dBP[1],gstCurrIn.iBV[1], gstCurrIn.dSP[1],gstCurrIn.iSV[1]);

#ifdef WRITE_SYSLOG_
    syslog(LOG_ERR, "%d gstCurrIn iV=%d BP1=%0.2f SP1=%0.2f BV1=%d SV1=%d\n", giTickNo, gstCurrIn.iVol, gstCurrIn.dBP[0],gstCurrIn.dSP[0],gstCurrIn.iBV[0],gstCurrIn.iSV[0]);
#endif // WRITE_SYSLOG_

	if ((gstCurrIn.iTime < gstGlobalPara.stExch.iMktStartTime * 1000) ||
		(gstCurrIn.iTime > gstGlobalPara.stExch.iMktEndTime * 1000)) goto END;
	if (giTickNo == 1 && gstCurrIn.iTime >= gstGlobalPara.stExch.iMktStartTime * 1000) goto END; // used data is from second tick
	gstLocTradeInfo.iBV1Sum = gstLocTradeInfo.iBV1Sum + gstCurrIn.iBV[0];
	gstLocTradeInfo.iSV1Sum = gstLocTradeInfo.iSV1Sum + gstCurrIn.iSV[0];
	gstLocTradeInfo.iTotalCancelSize = gstLocTradeInfo.iTotalCancelSize + gstLocTradeInfo.iCurrCancelSize;
	gstLocTradeInfo.iTotalOrderSize = gstLocTradeInfo.iTotalOrderSize + gstLocTradeInfo.iCurrOrderSize;

	gstLocTradeInfo.iCurrCancelSize = 0;
    gstLocTradeInfo.iCurrOrderSize = 0;

    gstLocTradeInfo.rLivePL = gstLocTradeInfo.rCash + (gstLocTradeInfo.iPosD - gstLocTradeInfo.iPosK)* gstCurrIn.dP;
    gstLocTradeInfo.rMaxLivePL = MAX(gstLocTradeInfo.rMaxLivePL, gstLocTradeInfo.rLivePL);
    gstLocTradeInfo.rLiveDD = gstLocTradeInfo.rLivePL - gstLocTradeInfo.rMaxLivePL;
    gstLocTradeInfo.rMaxLiveDD = MIN(gstLocTradeInfo.rMaxLiveDD, gstLocTradeInfo.rLiveDD);

    gstLocTradeInfo.iMaxSidePos = MAX(MAX(gstLocTradeInfo.iPosD, gstLocTradeInfo.iPosK),gstLocTradeInfo.iMaxSidePos);

    iTemp = gstLocTradeInfo.iPosD - gstLocTradeInfo.iPosK;
    if(abs(gstLocTradeInfo.iMaxNetPos) < abs(iTemp)) gstLocTradeInfo.iMaxNetPos = iTemp;

	// calculate PNL
	if (giRunModeFlg == 1)  // for real run mode
	{
		//gstLocTradeInfo.rLivePL = gstLocTradeInfo.rCash + (gstLocTradeInfo.iPosD - gstLocTradeInfo.iPosK)* gstCurrIn.dP;
	}
	else  // for simulation
	{
		//gstLocTradeInfo.rLivePL = gstLocTradeInfo.rCash + (gstLocTradeInfo.iPosD - gstLocTradeInfo.iPosK)* gstCurrIn.dP;

		grSimuValueTemp = grSimuValueTot + gstLocTradeInfo.rLivePL;
		grSimuValueMax = MAX(grSimuValueMax, grSimuValueTemp);
		grSimuDrawDown = MIN(grSimuDrawDown, grSimuValueTemp - grSimuValueMax);
		grSimuDrawDownRatio = MIN(grSimuDrawDownRatio, grSimuDrawDown/(grSimuValueMax + grTotBench));
	}

	//Update user current order list
	s_update_current_usr_order(gstUserCurOrder, giMaxUsrOrder, &giUserCurOrder);

	// generate signal
	s_MK1_sig_gen(&gstCurrIn, &gstPreIn, &gstGlobalPara, &gstLocTradeInfo, giTickNo, gaiFullOrderBookBuy, gaiFullOrderBookSel,
        giLenFullOrderBook, gstUserCurOrder, giUserCurOrder, gstUserNewOrder, giMaxUsrOrder, &giUserNewOrder);

    s_update_my_order_from_usr_order(gstUserNewOrder, giUserNewOrder, &gstCurrIn,
        gaiFullOrderBookBuy, gaiFullOrderBookSel, &gstGlobalPara, &gstLocTradeInfo);

    // send signal
	s_mm_assign_out_tsignal(lastStructTSignal, piNoTSignal, &gstLocTradeInfo);

	// write log file
	if (giWriteOutputFileFlg == 1)
	{
		s_output_log(giTickNo, pfDayLogFile, gcFileName);
	}
END:
	memcpy(&gstPreIn, (void*)&gstCurrIn, sizeof(struct in_data));
	memcpy(&gstRecPreIn, (void*)&gstRecCurrIn, sizeof(struct in_data));
	//reset_exe_order_rec();
	return 0;
}

int s_MK1_run_each_record_1lvl(struct signal_t *lastStructTSignal, int *piNoTSignal)
{
#ifdef WRITE_SYSLOG_
    syslog(LOG_ERR, "do s_MK1_run_each_record_1lvl() giInitFlg=%d\n",giInitFlg);
#endif // WRITE_SYSLOG_
    int iTemp;

    *piNoTSignal = 0;
	// init  variables
	if (giInitFlg == 0)
	{
		if (gstCurrIn.dPLmtUp < 0.001 || gstCurrIn.dPLmtDn < 0.001) goto END;
		f_find_security_name2(gstCurrIn.cTicker,gcSecName,&gcExch);
		s_set_global_parameters();
		s_MK1_variable_init(&gstCurrIn, &gstGlobalPara);

		memcpy(&gstPreIn, (void*)(&gstCurrIn), sizeof(struct in_data));
		memcpy(&gstRecPreIn, (void*)(&gstRecCurrIn), sizeof(struct in_data));
		giInitFlg = 1;
	}

#ifdef WRITE_SYSLOG_
    syslog(LOG_ERR, "%s %c %d gstCurrIn iV=%d BP1=%0.2f SP1=%0.2f BV1=%d SV1=%d\n", gcSecName,gcExch, giTickNo, gstCurrIn.iVol, gstCurrIn.dBP[0],gstCurrIn.dSP[0],gstCurrIn.iBV[0],gstCurrIn.iSV[0]);
#endif // WRITE_SYSLOG_

	if (gcExch != cCZCE)
	{
		gstCurrIn.iVol = (int)(gstCurrIn.dTotV - gstPreIn.dTotV)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dVal = (gstCurrIn.dTotVal - gstPreIn.dTotVal)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dOIChg = (gstCurrIn.dOI - gstPreIn.dOI)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dAvgP = f_calc_curr_avgp(gstCurrIn.dVal, gstCurrIn.iVol, gstPreIn.dAvgP)/gstGlobalPara.stSec.iUnitPerContract;
	}
	else
	{
		gstCurrIn.iVol = (int)(gstCurrIn.dTotV - gstPreIn.dTotV)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dOIChg = (gstCurrIn.dOI - gstPreIn.dOI)/gstGlobalPara.stExch.iVolDivider;
		gstCurrIn.dAvgP = gstCurrIn.dP ;
		gstCurrIn.dVal = 0;
	}
	if (f_check_valid_data_1lvl(&gstCurrIn, &gstRecPreIn, giTickNo, &gstGlobalPara) == iNO) goto END;   // check whether data is valid
	giTickNo = giTickNo + 1;  // tick number add 1

#ifdef WRITE_SYSLOG_
    syslog(LOG_ERR, "%d gstCurrIn iV=%d BP1=%0.2f SP1=%0.2f BV1=%d SV1=%d\n", giTickNo, gstCurrIn.iVol, gstCurrIn.dBP[0],gstCurrIn.dSP[0],gstCurrIn.iBV[0],gstCurrIn.iSV[0]);
#endif // WRITE_SYSLOG_

	if ((gstCurrIn.iTime < gstGlobalPara.stExch.iMktStartTime * 1000) ||
		(gstCurrIn.iTime > gstGlobalPara.stExch.iMktEndTime * 1000)) goto END;
	if (giTickNo == 1 && gstCurrIn.iTime >= gstGlobalPara.stExch.iMktStartTime * 1000) goto END; // used data is from second tick
	gstLocTradeInfo.iBV1Sum = gstLocTradeInfo.iBV1Sum + gstCurrIn.iBV[0];
	gstLocTradeInfo.iSV1Sum = gstLocTradeInfo.iSV1Sum + gstCurrIn.iSV[0];
	gstLocTradeInfo.iTotalCancelSize = gstLocTradeInfo.iTotalCancelSize + gstLocTradeInfo.iCurrCancelSize;
	gstLocTradeInfo.iTotalOrderSize = gstLocTradeInfo.iTotalOrderSize + gstLocTradeInfo.iCurrOrderSize;


	gstLocTradeInfo.iCurrCancelSize = 0;
    gstLocTradeInfo.iCurrOrderSize = 0;

    gstLocTradeInfo.rLivePL = gstLocTradeInfo.rCash + (gstLocTradeInfo.iPosD - gstLocTradeInfo.iPosK)* gstCurrIn.dP;
    gstLocTradeInfo.rMaxLivePL = MAX(gstLocTradeInfo.rMaxLivePL, gstLocTradeInfo.rLivePL);
    gstLocTradeInfo.rLiveDD = gstLocTradeInfo.rLivePL - gstLocTradeInfo.rMaxLivePL;
    gstLocTradeInfo.rMaxLiveDD = MIN(gstLocTradeInfo.rMaxLiveDD, gstLocTradeInfo.rLiveDD);

    gstLocTradeInfo.iMaxSidePos = MAX(MAX(gstLocTradeInfo.iPosD, gstLocTradeInfo.iPosK),gstLocTradeInfo.iMaxSidePos);

    iTemp = gstLocTradeInfo.iPosD - gstLocTradeInfo.iPosK;
    if(abs(gstLocTradeInfo.iMaxNetPos) < abs(iTemp)) gstLocTradeInfo.iMaxNetPos = iTemp;

	// calculate PNL
	if (giRunModeFlg == 1)  // for real run mode
	{
		//gstLocTradeInfo.rLivePL = gstLocTradeInfo.rCash + (gstLocTradeInfo.iPosD - gstLocTradeInfo.iPosK)* gstCurrIn.dP;
	}
	else  // for simulation
	{
		//gstLocTradeInfo.rLivePL = gstLocTradeInfo.rCash + (gstLocTradeInfo.iPosD - gstLocTradeInfo.iPosK)* gstCurrIn.dP;

		grSimuValueTemp = grSimuValueTot + gstLocTradeInfo.rLivePL;
		grSimuValueMax = MAX(grSimuValueMax, grSimuValueTemp);
		grSimuDrawDown = MIN(grSimuDrawDown, grSimuValueTemp - grSimuValueMax);
		grSimuDrawDownRatio = MIN(grSimuDrawDownRatio, grSimuDrawDown/(grSimuValueMax + grTotBench));
	}

	//Update user current order list
	s_update_current_usr_order(gstUserCurOrder, giMaxUsrOrder, &giUserCurOrder);


	// generate signal
	s_MK1_sig_gen(&gstCurrIn, &gstPreIn, &gstGlobalPara, &gstLocTradeInfo, giTickNo, gaiFullOrderBookBuy, gaiFullOrderBookSel,
        giLenFullOrderBook, gstUserCurOrder, giUserCurOrder, gstUserNewOrder, giMaxUsrOrder, &giUserNewOrder);

    s_update_my_order_from_usr_order(gstUserNewOrder, giUserNewOrder, &gstCurrIn,
        gaiFullOrderBookBuy, gaiFullOrderBookSel, &gstGlobalPara, &gstLocTradeInfo);

    // send signal
	s_mm_assign_out_tsignal(lastStructTSignal, piNoTSignal, &gstLocTradeInfo);

	// write log file
	if (giWriteOutputFileFlg == 1)
	{
		s_output_log(giTickNo, pfDayLogFile, gcFileName);
	}
END:
	memcpy(&gstPreIn, (void*)&gstCurrIn, sizeof(struct in_data));
	memcpy(&gstRecPreIn, (void*)&gstRecCurrIn, sizeof(struct in_data));
	//reset_exe_order_rec();
	return 0;
}

int s_MK1_variable_init(struct in_data *p_stCurrInData, struct all_parameter *p_stGlobalPara)
{
    grTotBench = 1;
	s_MK1_common_variable_init(p_stCurrInData, p_stGlobalPara);

	return 0;
}


int s_MK1_common_variable_init(struct in_data *p_stCurrInData, struct all_parameter *p_stGlobalPara)
{
	giLenFullOrderBook = (int)((p_stCurrInData->dPLmtUp - p_stCurrInData->dPLmtDn)/ p_stGlobalPara->stSec.rTickSize + 0.5)+1;

	if (gaiFullOrderBookBuy != NULL)
	{
		free(gaiFullOrderBookBuy); gaiFullOrderBookBuy = NULL;
	}
	gaiFullOrderBookBuy = (int*)malloc(giLenFullOrderBook * sizeof(int));
	memset(gaiFullOrderBookBuy,0,giLenFullOrderBook * sizeof(int));

	if (gaiFullOrderBookSel != NULL)
	{
		free(gaiFullOrderBookSel); gaiFullOrderBookSel = NULL;
	}
	gaiFullOrderBookSel = (int*)malloc(giLenFullOrderBook * sizeof(int));
	memset(gaiFullOrderBookSel,0,giLenFullOrderBook * sizeof(int));

	if (giWaterflow != NULL)
	{
		free(giWaterflow); giWaterflow = NULL;
	}
	giWaterflow = (int*)malloc(giLenFullOrderBook * sizeof(int));
	memset(giWaterflow,0,giLenFullOrderBook * sizeof(int));

	giSumWaterflow = 0;
	giWaterflowMax = 0;
	giWaterflowMin = 0;

	p_stCurrInData->iTotalBuyVol = 0;
	p_stCurrInData->iTotalSelVol = 0;
	p_stCurrInData->dWeightedBuyOrderP = 0;
	p_stCurrInData->dWeightedSelOrderP = 0;

	return 0;
}


int s_MK1_output_log(int liTickNo, FILE *liFileNumber)
{
	int liIndex;
	int i, i0,i1;
	//char  cStrFmt[10], cOutFmt[512];

	// common output
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iTime = gstCurrIn.iTime;
	strncpy(gastMK1OutData[giMK1OutDataIndex].stCommOutData.cTicker, gstCurrIn.cTicker, 7);
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iRecordNo = liTickNo;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.rP = gstCurrIn.dP;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iV = gstCurrIn.iVol;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.rBP1 = gstCurrIn.dBP[0];
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iBV1 = gstRecCurrIn.iBV[0];
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.rSP1 = gstCurrIn.dSP[0];
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iSV1 = gstRecCurrIn.iSV[0];
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.rVal = gstCurrIn.dVal;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.rOI = gstCurrIn.dOI;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iHaveOrder = f_sign(giNoTradeItemEachRun);
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iTS = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iCancel = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.rBuyPrice = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.rSelPrice = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iOpenSize = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iCloseSize = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.rBuyPrice = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.rSelPrice = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iOpenSize = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iCloseSize = 0;
	if(gstLastRec.iBS == 1)
    {
        gastMK1OutData[giMK1OutDataIndex].stCommOutData.rBuyPrice = gstLastRec.rPrice;
        gastMK1OutData[giMK1OutDataIndex].stCommOutData.rSelPrice = 0;
    }
    else if(gstLastRec.iBS == -1)
    {
        gastMK1OutData[giMK1OutDataIndex].stCommOutData.rBuyPrice = 0;
        gastMK1OutData[giMK1OutDataIndex].stCommOutData.rSelPrice = gstLastRec.rPrice;
    }
    if(gstLastRec.iOpenFlat==SIG_ACTION_OPEN)
    {
        gastMK1OutData[giMK1OutDataIndex].stCommOutData.iOpenSize = gstLastRec.iVol;
    }
    else if(gstLastRec.iOpenFlat==SIG_ACTION_FLAT)
    {
        gastMK1OutData[giMK1OutDataIndex].stCommOutData.iCloseSize = gstLastRec.iVol;
    }

	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iToTradeSize = 0;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iSidePos = MAX(gstLocTradeInfo.iPosD, gstLocTradeInfo.iPosK);
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iLongPos = gstLocTradeInfo.iPosD;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iShortPos = gstLocTradeInfo.iPosK;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iTotalOrderSize = gstLocTradeInfo.iTotalOrderSize;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iTotalCancelSize = gstLocTradeInfo.iTotalCancelSize;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iTotalOrderCount = gstLocTradeInfo.iTotalOrderCount;
	gastMK1OutData[giMK1OutDataIndex].stCommOutData.iTotalCancelCount = gstLocTradeInfo.iTotalCancelCount;
	// specific output
	gastMK1OutData[giMK1OutDataIndex].iBV1 = gstCurrIn.iBV[0] ;
	gastMK1OutData[giMK1OutDataIndex].iSV1 = gstCurrIn.iSV[0] ;
	gastMK1OutData[giMK1OutDataIndex].iNewPOS = 0;
	gastMK1OutData[giMK1OutDataIndex].rCash = gstLocTradeInfo.rCash;
	gastMK1OutData[giMK1OutDataIndex].rLivePL = gstLocTradeInfo.rLivePL ;
	gastMK1OutData[giMK1OutDataIndex].rMaxDD = gstLocTradeInfo.rMaxLiveDD;

	gastMK1OutData[giMK1OutDataIndex].iNetPOS = gstLocTradeInfo.iPosD - gstLocTradeInfo.iPosK;
	gastMK1OutData[giMK1OutDataIndex].iRound = gstLocTradeInfo.iTotalVol;
	gastMK1OutData[giMK1OutDataIndex].iCanCelOrder = gstLocTradeInfo.iCurrCancelSize;
	gastMK1OutData[giMK1OutDataIndex].rPNet = 0;
	gastMK1OutData[giMK1OutDataIndex].rBVSVRatio =  0;
	gastMK1OutData[giMK1OutDataIndex].rBigVolScore = 0;
	gastMK1OutData[giMK1OutDataIndex].iMaxNetPos = gstLocTradeInfo.iMaxNetPos;
	gastMK1OutData[giMK1OutDataIndex].iMaxSidePos = gstLocTradeInfo.iMaxSidePos;

    memset(&(gastMK1OutData[giMK1OutDataIndex].out_order[0]), 0, sizeof(struct struct_out_order)*10);

    if(giUserCurOrder > 0)
    {
        i0 =0;
        for(i=0; i<giUserCurOrder; i++)
        {
            if(i>0 && gstUserCurOrder[i].iBS == USER_ORDER_BUY && gstUserCurOrder[i-1].iBS == USER_ORDER_SELL )
            {
                i0 = i; break;
            }
            else if(i==0 && gstUserCurOrder[i].iBS == USER_ORDER_BUY)
            {
                i0 = 0; break;
            }
            else if(i==giUserCurOrder-1 && gstUserCurOrder[i].iBS == USER_ORDER_SELL)
            {
                i0 = i+1; break;
            }
        }


        i1 = 4;
        for(i=i0-1; i>=MAX(0,i0-5); i--)
        {
            if(i<0 || i1<0) break;
            //printf("%d %d %d %d %d\n",giTickNo,giMK1OutDataIndex, i1, i, i0);
            gastMK1OutData[giMK1OutDataIndex].out_order[i1].ivol = - gstUserCurOrder[i].iVol;
            gastMK1OutData[giMK1OutDataIndex].out_order[i1].price = gstUserCurOrder[i].dPrice;
            gastMK1OutData[giMK1OutDataIndex].out_order[i1].vol2pass = gstUserCurOrder[i].vol2pass;
            i1 --;
        }
        i1=5;
        for(i=i0; i<MIN(giUserCurOrder,i0+5); i++)
        {
            if(gstUserCurOrder[i].iBS != USER_ORDER_BUY) break;
            gastMK1OutData[giMK1OutDataIndex].out_order[i1].ivol = gstUserCurOrder[i].iVol;
            gastMK1OutData[giMK1OutDataIndex].out_order[i1].price = gstUserCurOrder[i].dPrice;
            gastMK1OutData[giMK1OutDataIndex].out_order[i1].vol2pass = gstUserCurOrder[i].vol2pass;
            i1 ++;
        }
    }


	if (giNoTradeItemEachRun == 0 || giMK1OutDataIndex == iOUT_LEN-1)
	{

		if (giMK1TraceInit == 0)
		{
			fprintf(liFileNumber, "VersionNumber: %s\n", gcVersion);
			fprintf(liFileNumber,"ExchCode:%c SecName: %s StratID1: %d StratID2: %d Parameter: %5.1f-%5.1f-%5.1f MaxPos:%i NightFlg:%i\n",
				gcExch, gcSecName, giStrateType, 0,
					0.0, 0.0, 0.0,
					giMaxPos, giNightMarket);


			{
			   fprintf (liFileNumber, "exch_time  contract  n_tick  price  vol  bv1  bp1  sp1  sv1  amt  ");
			   fprintf (liFileNumber, "oi buy_price  sell_price  open_vol  close_vol  ");
			   fprintf (liFileNumber, "long_pos  short_pos  total_ordervol  total_cancelvol order_count cancel_count ");
			   fprintf (liFileNumber, "cash live net_pos  side_pos round  livePL max_dd max_netpos max_side_pos cancel_number  ");
			   fprintf (liFileNumber, "order1.p order1.v order1.v2pass order2.p order2.v order2.v2pass ");
			   fprintf (liFileNumber, "order3.p order3.v order3.v2pass order4.p order4.v order4.v2pass ");
			   fprintf (liFileNumber, "order5.p order5.v order5.v2pass order6.p order6.v order6.v2pass ");
			   fprintf (liFileNumber, "order7.p order7.v order7.v2pass order8.p order8.v order8.v2pass ");
			   fprintf (liFileNumber, "order9.p order9.v order9.v2pass order10.p order10.v order10.v2pass\n");
			}

			giMK1TraceInit = 1;
		}


		for(liIndex = 0; liIndex <= giMK1OutDataIndex; liIndex++)
		{

            fprintf(liFileNumber,"%d %6s %d %14.2f %d ",
                        gastMK1OutData[liIndex].stCommOutData.iTime,
                        gastMK1OutData[liIndex].stCommOutData.cTicker,
                        gastMK1OutData[liIndex].stCommOutData.iRecordNo,
                        gastMK1OutData[liIndex].stCommOutData.rP,
                        gastMK1OutData[liIndex].stCommOutData.iV);

            fprintf(liFileNumber,"%d %12.4f %12.4f %d %d %d ",
                      gastMK1OutData[liIndex].stCommOutData.iBV1,
                      gastMK1OutData[liIndex].stCommOutData.rBP1,
                      gastMK1OutData[liIndex].stCommOutData.rSP1,
                      gastMK1OutData[liIndex].stCommOutData.iSV1,
                      (int)(gastMK1OutData[liIndex].stCommOutData.rVal),
                      (int)(gastMK1OutData[liIndex].stCommOutData.rOI));

            fprintf(liFileNumber,"%12.4f %12.4f %d %d ",
                      gastMK1OutData[liIndex].stCommOutData.rBuyPrice,
                      gastMK1OutData[liIndex].stCommOutData.rSelPrice,
                      gastMK1OutData[liIndex].stCommOutData.iOpenSize,
                      gastMK1OutData[liIndex].stCommOutData.iCloseSize);

            fprintf(liFileNumber,"%d %d %d %d %d %d ",
                      gastMK1OutData[liIndex].stCommOutData.iLongPos,
                      gastMK1OutData[liIndex].stCommOutData.iShortPos,
                      gastMK1OutData[liIndex].stCommOutData.iTotalOrderSize,
                      gastMK1OutData[liIndex].stCommOutData.iTotalCancelSize,
                      gastMK1OutData[liIndex].stCommOutData.iTotalOrderCount,
                      gastMK1OutData[liIndex].stCommOutData.iTotalCancelCount);

            fprintf(liFileNumber,"%16.2f %16.2f %d %d %d %16.2f %16.2f %d %d %d ",
                      gastMK1OutData[liIndex].rCash,
                      gastMK1OutData[liIndex].rLivePL,
                      gastMK1OutData[liIndex].iNetPOS,
                      gastMK1OutData[liIndex].stCommOutData.iSidePos,
                      gastMK1OutData[liIndex].iRound,
                      gastMK1OutData[liIndex].rLivePL,
                      gastMK1OutData[liIndex].rMaxDD,
                      gastMK1OutData[liIndex].iMaxNetPos,
                      gastMK1OutData[liIndex].iMaxSidePos,
                      gastMK1OutData[liIndex].iCanCelOrder);

            for(i=0; i<9; i++)
            {
                fprintf(liFileNumber, "%0.2f %d %d ",
                    gastMK1OutData[liIndex].out_order[i].price,
                    gastMK1OutData[liIndex].out_order[i].ivol,
                    gastMK1OutData[liIndex].out_order[i].vol2pass);
            }
            fprintf(liFileNumber, "%0.2f %d %d\n",
                gastMK1OutData[liIndex].out_order[i].price,
                gastMK1OutData[liIndex].out_order[i].ivol,
                gastMK1OutData[liIndex].out_order[i].vol2pass);

		}
		//giHi5TraceDone = 1
		giMK1OutDataIndex = -1;
	}
	giMK1OutDataIndex ++;
	if(giMK1OutDataIndex >=iOUT_LEN) giMK1OutDataIndex = giMK1OutDataIndex - iOUT_LEN;
	return 0;
}


int s_MK1_set_parameters()
{
	if( giMK1Type == iMK101)
		gstGlobalPara.stSec.iSecSqTime = gstGlobalPara.stSec.iSecEndTime + 25;

	if ( (int)(gstGlobalPara.stSec.iSecSqTime/100)% 100 == 0)
		gstGlobalPara.stSec.iSecOpenEndTime = gstGlobalPara.stSec.iSecSqTime - 4100;
	else
		gstGlobalPara.stSec.iSecOpenEndTime = gstGlobalPara.stSec.iSecSqTime - 100;

	if (giInnerPar1Flg == iNO) gstGlobalPara.stHi5Para.rMKTThreshold   = giSimuPara1 * 0.1;
	if (giInnerPar2Flg == iNO) gstGlobalPara.stHi5Para.rOpenThreshold  = giSimuPara2 * 0.1;
	if (giInnerPar3Flg == iNO) gstGlobalPara.stHi5Para.rCloseThreshold = giSimuPara3 * 0.1;

	return 0;
}

