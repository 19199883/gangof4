#include "tdf_stock.h"
#include "util.h"
#include <string>

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

std::string TDF_Index_Data_QuoteToString( const SaveData_TDF_INDEX_DATA * const p_data )
{
	const TDF_INDEX_DATA_MY * const p = &(p_data->data_);
	char quote_string[1024];

	try
	{
		char exch_code = '0';
		// only two exchange: sz or sh
		if (p->szWindCode[strlen(p->szWindCode) - 1] == 'H'
			|| p->szWindCode[strlen(p->szWindCode) - 1] == 'h')
		{
			exch_code = '1';
		}
		sprintf_s(quote_string, sizeof(quote_string),
			"R||2|%09d|%c|%s|%0.4f|%0.4f|%0.4f|%0.4f|%0.4f|%lld|%lld|%llu|",
			p->nTime,
			exch_code,
			p->szCode,
			p->nLastIndex / 10000.0,
			p->nOpenIndex / 10000.0,
			p->nHighIndex / 10000.0,
			p->nLowIndex / 10000.0,
			p->nPreCloseIndex / 10000.0,
			p->iTotalVolume,
			p->iTurnover,
			p_data->t_
			);
	}
	catch (...)
	{
		cout << "error occurs when format TDF_INDEX_DATA." << std::endl;
	}

	return quote_string;
}

std::string TDF_Market_Data_QuoteToString( const SaveData_TDF_MARKET_DATA * const p_data )
{
	if (!p_data)
	{
		return "";
	}
	char quote_string[1024];
	const TDF_MARKET_DATA_MY * const p = &(p_data->data_);

	try
	{
		char exch_code = '-';
		// only two exchange: sz or sh
		int tail_pos = strlen(p->szWindCode) - 1;
		if (p->szWindCode[tail_pos] == 'H'
			|| p->szWindCode[tail_pos] == 'h')
		{
			exch_code = '1';
		}
		else if (p->szWindCode[tail_pos] == 'Z'
			|| p->szWindCode[tail_pos] == 'z')
		{
			exch_code = '0';
		}
		//ʱ��,������,��Ʊ����,״̬,���¼�,�ܳɽ���,�ɽ��ܽ��,
		//��һ��,�����,������,���ļ�,�����,������,���߼�,��˼�,��ż�,��ʮ��,
		//��һ��,�����,������,������,������,������,������,�����,�����,��ʮ��,
		//��һ��,������,������,���ļ�,�����,������,���߼�,���˼�,���ż�,��ʮ��,
		//��һ��,������,������,������,������,������,������,������,������,��ʮ��,
		//ǰ���̼�,���̼�,��߼�,��ͼ�,��ͣ��,��ͣ��,�ɽ�����,ί����������,ί����������,
		//��Ȩƽ��ί��۸�,��Ȩƽ��ί���۸�,IOPV��ֵ��ֵ,����������,��ӯ��1,��ӯ��2,
		sprintf_s(quote_string, sizeof(quote_string),
			"R|%s|1|%09d|%c|%s|%c|%.4f|%lld|%.4f|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|"
			"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|"
			"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%u|%lld|%lld|"
			"%.4f|%.4f|%d|%d|%d|%d|%llu|",
			ConvertToShortTimval(p_data->t_).c_str(),
			p->nTime,
			exch_code,
			p->szCode,
			p->nStatus == 0 ? '-' : p->nStatus,
			p->nMatch / 10000.0,
			p->iVolume,
			(double)p->iTurnover,
			p->nBidPrice[0] / 10000.0,
			p->nBidPrice[1] / 10000.0,
			p->nBidPrice[2] / 10000.0,
			p->nBidPrice[3] / 10000.0,
			p->nBidPrice[4] / 10000.0,
			p->nBidPrice[5] / 10000.0,
			p->nBidPrice[6] / 10000.0,
			p->nBidPrice[7] / 10000.0,
			p->nBidPrice[8] / 10000.0,
			p->nBidPrice[9] / 10000.0,
			p->nBidVol[0],
			p->nBidVol[1],
			p->nBidVol[2],
			p->nBidVol[3],
			p->nBidVol[4],
			p->nBidVol[5],
			p->nBidVol[6],
			p->nBidVol[7],
			p->nBidVol[8],
			p->nBidVol[9],
			p->nAskPrice[0] / 10000.0,
			p->nAskPrice[1] / 10000.0,
			p->nAskPrice[2] / 10000.0,
			p->nAskPrice[3] / 10000.0,
			p->nAskPrice[4] / 10000.0,
			p->nAskPrice[5] / 10000.0,
			p->nAskPrice[6] / 10000.0,
			p->nAskPrice[7] / 10000.0,
			p->nAskPrice[8] / 10000.0,
			p->nAskPrice[9] / 10000.0,
			p->nAskVol[0],
			p->nAskVol[1],
			p->nAskVol[2],
			p->nAskVol[3],
			p->nAskVol[4],
			p->nAskVol[5],
			p->nAskVol[6],
			p->nAskVol[7],
			p->nAskVol[8],
			p->nAskVol[9],
			p->nPreClose / 10000.0,
			p->nOpen / 10000.0,
			p->nHigh / 10000.0,
			p->nLow / 10000.0,
			p->nHighLimited / 10000.0,
			p->nLowLimited / 10000.0,
			p->nNumTrades,
			p->nTotalBidVol,
			p->nTotalAskVol,
			p->nWeightedAvgBidPrice / 10000.0,
			p->nWeightedAvgAskPrice / 10000.0,
			p->nIOPV,
			p->nYieldToMaturity,
			p->nSyl1,
			p->nSyl2,
			p_data->t_
			);
	}
	catch (...)
	{
		cout << "error occurs when format TDF_MARKET_DATA." << std::endl;
	}

	return quote_string;
}
