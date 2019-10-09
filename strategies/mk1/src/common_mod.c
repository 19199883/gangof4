#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <memory.h>
#include "common.h"
#include "common_mod.h"
#include "common_func.h"
#include "interface_v30.h"
#include "MK1_var_def.h"
#include "MK1_func.h"



int s_read_fut_struct(int liTickNo)
{

	if(gcExch == cCFFE)
        s_read_gta_fut_struct(gstcStructCFFEIn, &gstCurrIn);
	else if(gcExch == cDCE)
        s_read_dl_fut_struct(gstcStructDCEIn, &gstCurrIn);
	else if(gcExch == cSHFE)
        s_read_sh_fut_struct(gstcStructSHFEIn, &gstCurrIn);
	else if(gcExch == cCZCE)
        s_read_zz_fut_struct(gstcStructCZCEIn, &gstCurrIn);

	return 0;
}

int s_set_global_parameters()
{
#ifdef WRITE_SYSLOG_
    syslog(LOG_ERR, "do s_set_global_parameters() giInitFlg=%d\n",giInitFlg);
#endif // WRITE_SYSLOG_

	s_set_exch_parameters(gcExch, gcSecName, &gstGlobalPara);
	s_set_convertion_parameters(gcSecName, &gstGlobalPara.stSec);

	if (giNightMarket == 0)
    {
        if(giStrateType == iSTRATEGY_MK1)
            s_MK1_set_parameters();
    }
	return 0;
}

int f_check_valid_data(struct in_data *p_stCurrInData, struct in_data *p_stPreInData,
	int liTickNo, struct all_parameter *p_stGlobalPara)
{
	int ireturn = iYES;
	int i, isum1, isum2;
	double dsum1, dsum2, dsum3, dsum4;

	if ((int)(p_stCurrInData->dTotV + 0.5) % p_stGlobalPara->stExch.iVolDivider != 0)
	{	ireturn = iNO; }
	if ((p_stCurrInData->dBP[0] - p_stCurrInData->dSP[0]) > -p_stGlobalPara->stSec.rTickSize/2)
	{	ireturn = iNO; }

	if (abs(p_stCurrInData->iVol) > p_stGlobalPara->stSec.iWrongVol || p_stCurrInData->iVol < 0
		|| p_stCurrInData->iTime < p_stPreInData->iTime)
	{	ireturn = iNO; }

	if(liTickNo==1)
		giMaxTime = p_stCurrInData->iTime;
	else
	{
		if(p_stCurrInData->iTime < giMaxTime)
		{	ireturn = iNO; }
		else
		{
			giMaxTime = p_stCurrInData->iTime;
		}
	}

	if (p_stCurrInData->iVol == 0 && p_stCurrInData->dVal > 1.0 * p_stCurrInData->dP)
	{	ireturn = iNO;}

	if (p_stCurrInData->dVal > 1.0 && p_stCurrInData->iVol >= 1)
	{
		if (p_stCurrInData->dAvgP > (p_stCurrInData->dPLmtUp + p_stGlobalPara->stSec.rTickSize/2) ) ireturn = iNO;
		if (p_stCurrInData->dAvgP < (p_stCurrInData->dPLmtDn - p_stGlobalPara->stSec.rTickSize/2) ) ireturn = iNO;
	}


	isum1 = 0; isum2 = 0; dsum1=0; dsum2 = 0;
	dsum3 = 0; dsum4 = 0;
	for(i=0; i<5; i++)
	{
		dsum1 = dsum1 + fabs(p_stCurrInData->dBP[i] - p_stPreInData->dBP[i]);
		dsum2 = dsum2 + fabs(p_stCurrInData->dSP[i] - p_stPreInData->dSP[i]);
		isum1 = isum1 + abs(p_stCurrInData->iBV[i] - p_stPreInData->iBV[i]);
		isum2 = isum2 + abs(p_stCurrInData->iSV[i] - p_stPreInData->iSV[i]);
		if(i>0)
        {
            dsum3 += fabs(p_stCurrInData->dBP[i]);
            dsum4 += fabs(p_stCurrInData->dSP[i]);
        }
	}
	if((p_stCurrInData->dPLmtUp - p_stCurrInData->dSP[0] > 4*p_stGlobalPara->stSec.rTickSize
       && dsum3 < p_stGlobalPara->stSec.rTickSize) ||
       (p_stCurrInData->dBP[0] - p_stCurrInData->dPLmtDn > 4*p_stGlobalPara->stSec.rTickSize
       && dsum4 < p_stGlobalPara->stSec.rTickSize))
    {
        ireturn = iNO;
    }
	if (gcExch != cCZCE)
	{
		if (liTickNo > 1 && p_stCurrInData->iTime == p_stPreInData->iTime)
			ireturn = iNO;

		if ( fabs(dsum1) < 0.0001 && fabs(dsum2) <0.0001 && isum1 == 0 && isum2 == 0 &&
			p_stCurrInData->iVol == 0) ireturn = iNO;
	}
	else
	{
		{
			if (dsum1 < 0.01 && dsum2 < 0.01 && isum1 == 0 && isum2 == 0 &&
			(liTickNo > 1 && p_stCurrInData->iTime == p_stPreInData->iTime))
				ireturn = iNO;
		}
	}

	return ireturn;
}

int f_check_valid_data_1lvl(struct in_data *p_stCurrInData, struct in_data *p_stPreInData,
	int liTickNo, struct all_parameter *p_stGlobalPara)
{
	int ireturn = iYES;
	int i, isum1, isum2;
	double dsum1, dsum2;

	if ((int)(p_stCurrInData->dTotV + 0.5) % p_stGlobalPara->stExch.iVolDivider != 0)
	{	ireturn = iNO; }
	if ((p_stCurrInData->dBP1 - p_stCurrInData->dSP1) > -p_stGlobalPara->stSec.rTickSize/2)
	{	ireturn = iNO; }

	if (abs(p_stCurrInData->iVol) > p_stGlobalPara->stSec.iWrongVol || p_stCurrInData->iVol < 0
		|| p_stCurrInData->iTime < p_stPreInData->iTime)
	{	ireturn = iNO; }

	if(liTickNo==1)
		giMaxTime = p_stCurrInData->iTime;
	else
	{
		if(p_stCurrInData->iTime < giMaxTime)
		{	ireturn = iNO; }
		else
		{
			giMaxTime = p_stCurrInData->iTime;
		}
	}

	if (p_stCurrInData->iVol == 0 && p_stCurrInData->dVal > 1.0 * p_stCurrInData->dP)
	{	ireturn = iNO;}

	if (p_stCurrInData->dVal > 1.0 && p_stCurrInData->iVol >= 1)
	{
		if (p_stCurrInData->dAvgP > (p_stCurrInData->dPLmtUp + p_stGlobalPara->stSec.rTickSize/2) ) ireturn = iNO;
		if (p_stCurrInData->dAvgP < (p_stCurrInData->dPLmtDn - p_stGlobalPara->stSec.rTickSize/2) ) ireturn = iNO;
	}


	isum1 = 0; isum2 = 0; dsum1=0; dsum2 = 0;
	for(i=0; i<1; i++)
	{
		dsum1 = dsum1 + fabs(p_stCurrInData->dBP[i] - p_stPreInData->dBP[i]);
		dsum2 = dsum2 + fabs(p_stCurrInData->dSP[i] - p_stPreInData->dSP[i]);
		isum1 = isum1 + abs(p_stCurrInData->iBV[i] - p_stPreInData->iBV[i]);
		isum2 = isum2 + abs(p_stCurrInData->iSV[i] - p_stPreInData->iSV[i]);
	}

	if (gcExch != cCZCE)
	{
		if (liTickNo > 1 && p_stCurrInData->iTime == p_stPreInData->iTime)
			ireturn = iNO;

		if ( fabs(dsum1) < 0.0001 && fabs(dsum2) <0.0001 && isum1 == 0 && isum2 == 0 &&
			p_stCurrInData->iVol == 0) ireturn = iNO;
	}
	else
	{
		{
			if (dsum1 < 0.01 && dsum2 < 0.01 && isum1 == 0 && isum2 == 0 &&
			(liTickNo > 1 && p_stCurrInData->iTime == p_stPreInData->iTime))
				ireturn = iNO;
		}
	}

	return ireturn;
}

//----------------------------------------------------
// subroutine mysub0(liFileNumber, lcFileName)
// description: initialization function
// input: liFileNumber
//        lcFileName
//----------------------------------------------------
void mysub0(int liFileNumber, char *lcFileName)
{
	int i;
	//pfDayLogFile = liFileNumber;
	strncpy(gcFileName, lcFileName, 500);

	giInitFlg = 0;
	giSizePerTrade = 0;
	giSimuPara1 = 0;
	giSimuPara2 = 0;
	giSimuPara3 = 0;
	giWriteOutputFileFlg = 1;
	giWriteOutputFileInitFlg = 0;
	giRunModeFlg = 1;
	giMaxTime = 0;
	giBSV1Num = 0;
	giActBuyNum = 0;
	giActSelNum = 0;
	for(i=0; i<giPreActNum; i++)
	{
		giPreActBuy[i] = 0;
		giPreActSel[i] = 0;
	}
	giStatus = 0;
	giCoverTime = 0;
	giStopLoss = 0;
	for(i=0; i<MK1_MM_NUM; i++)
	{
		grLastBP[i] = 0;
		grLastSP[i] = 0;
		grLastPrice[i] = 0;
		giLastBV[i] = 0;
		giLastSV[i] = 0;
		grActiveVolBuy[i] = 0;
		grActiveVolSel[i] = 0;
	}
	giBuyOrSell = 0;

	s_init_MK1_parameters();
	s_initializer_usr_order();
	s_initializer_my_order();
}



int s_output_log(int liTickNo, FILE *p_iFileNumber, char *lcFileName)
{
	if (giWriteOutputFileFlg == 1 && giWriteOutputFileInitFlg == 0)
	{
		if (giRunModeFlg == 0)
		{
			pfDayLogFile = fopen(lcFileName, "w");
		}
		else
		{
			//open(liFileNumber,file=trim(lcFileName),status='unknown',  position = 'append')
			pfDayLogFile = fopen(lcFileName, "a");
		}
		giWriteOutputFileInitFlg = 1;
	}

    if(giStrateType == iSTRATEGY_MK1)
    {
        s_MK1_output_log(liTickNo, pfDayLogFile);
    }

	return 0;
}

int s_update_full_order_book(struct in_data *lstCurrInData, struct in_data *lstPreInData,
	struct all_parameter *lstGlobalPara,int *laiFullOrderBookBuy, int *laiFullOrderBookSel,
	int liLenFullOrderBook)
{
    double larBP[30], larSP[30], lrTick;
	int laiBV[30], laiSV[30], laiImpBV[30], laiImpSV[30];
	int liLevel,liIndex;
	int liRefreshBidFrom, liRefreshBidTo, liRefreshAskFrom, liRefreshAskTo;
	int i;

    lrTick = lstGlobalPara->stSec.rTickSize;
	memcpy(larBP, lstCurrInData->dBP, sizeof(double)*30);
	memcpy(larSP, lstCurrInData->dSP, sizeof(double)*30);
	memcpy(laiBV, lstCurrInData->iBV, sizeof(int)*30);
	memcpy(laiSV, lstCurrInData->iSV, sizeof(int)*30);
	memcpy(laiImpBV, lstCurrInData->iImpBV, sizeof(int)*30);
	memcpy(laiImpSV, lstCurrInData->iImpSV, sizeof(int)*30);

	liLevel = lstGlobalPara->stExch.iPInLevel;


	//lrOI = lstCurrInData->dOI;

	gstMK1_Paras.iLimitStop = iNO;
	if ((larBP[0] < lstCurrInData->dPLmtDn + gstMK1_Paras.iLimitRange * lrTick) ||
		(larSP[0] > lstCurrInData->dPLmtUp - gstMK1_Paras.iLimitRange * lrTick))
			gstMK1_Paras.iLimitStop = iYES;

	if (larBP[liLevel-1] < lrTick)
		liRefreshBidFrom = 0;
	else
		liRefreshBidFrom = MAX((int)((larBP[liLevel-1]-lstCurrInData->dPLmtDn)/lrTick+0.5), 0);

	liRefreshBidTo = MAX((int)((MAX(larBP[0], lstPreInData->dBP[0]) - lstCurrInData->dPLmtDn)/lrTick+0.5) , 0);


	if (larSP[liLevel-1] < lrTick)
		liRefreshAskTo = liLenFullOrderBook -1;
	else
		liRefreshAskTo = (int)((larSP[liLevel-1]-lstCurrInData->dPLmtDn)/lrTick+0.5);

	if (larSP[0] < lrTick && lstPreInData->dSP[0] < lrTick)
		liRefreshAskFrom = liLenFullOrderBook -1;
	else if (MIN(larSP[0], lstPreInData->dSP[0]) < lrTick)
		liRefreshAskFrom = (int)((MAX(larSP[0], lstPreInData->dSP[0])-lstCurrInData->dPLmtDn)/lrTick+0.5);
	else
		liRefreshAskFrom = (int)((MIN(larSP[0], lstPreInData->dSP[0])-lstCurrInData->dPLmtDn)/lrTick+0.5);

	// reset buffer
	for(i= liRefreshBidFrom; i<= liRefreshBidTo; i++)
		laiFullOrderBookBuy[i] = 0;
	for(i= liRefreshAskFrom; i<= liRefreshAskTo; i++)
		laiFullOrderBookSel[i] = 0;


	for(i=0; i<liLevel; i++)
	{
		if (larBP[i] > lrTick/2 && larBP[i] - lstCurrInData->dPLmtDn > -lrTick/2)
		{
			liIndex = (int)((larBP[i]-lstCurrInData->dPLmtDn)/lrTick+0.5);
			laiFullOrderBookBuy[liIndex] = laiBV[i] - laiImpBV[i];
		}
		if (larSP[i] > lrTick/2 && larSP[i] - lstCurrInData->dPLmtUp < lrTick/2)
		{
			liIndex = (int)((larSP[i]-lstCurrInData->dPLmtDn)/lrTick+0.5);
			laiFullOrderBookSel[liIndex] = laiSV[i] - laiImpSV[i];
		}
	}

    return 0;
}


















