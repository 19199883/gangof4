#include "gta_udp.h"
#include "util.h"
#include <string>

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

std::string GTA_UDP_QuoteToString( const SaveData_GTA_UDP * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const CFfexFtdcDepthMarketData * const p = &(p_data->data_);	
	char buffer[64];
	char total_quote[2048];

	// ��ǰ���������л�
	std::string csTemp = p->szUpdateTime;
	boost::erase_all(csTemp, ":");
	sprintf(buffer, "%03d", p->nUpdateMillisec);
	csTemp.append(buffer);


	double total_amount = InvalidToZeroD(p->dTurnover);
	double avg_price = 0;
	if (p->nVolume > 0)
	{
		avg_price = total_amount / p->nVolume;
	}

	// ��ǰ��, ��ǰ�ɽ����
	int cur_volumn = 0;
	double cur_amount = 0;
	//InstrumentToVolumnAndAmountMap::iterator it = instrument_to_quoteinfo_.find(p->InstrumentID);
	//if (it != instrument_to_quoteinfo_.end())
	//{
	//	cur_volumn = p->Volume - it->second.first;
	//	cur_amount = total_amount - it->second.second;
	//	it->second = std::make_pair(p->Volume, total_amount);
	//}
	//else
	//{
	//	cur_volumn = p->Volume;
	//	cur_amount = total_amount;
	//	instrument_to_quoteinfo_.insert(std::make_pair(p->InstrumentID,
	//		std::make_pair(p->Volume, total_amount)));
	//}

	std::string trade_day(p->szTradingDay, 8);

	//R|Ԥ��|Ԥ��|ʱ��|����������|��Լ����|�ɽ���|�ۼƳɽ���|��һ��|��һ��|��һ��|��һ��|�ֲ���|������|��ǰ��
	//|�����|������|���ļ�|�����|�����|������|������|������
	//|������|������|���ļ�|�����|������|������|������|������
	//|��ͣ��|��ͣ��|���տ��̼�|�������|�������|������|��ֲ�|���ճɽ�����
	//|�ۼƳɽ����|��ǰ�ɽ����\n

	sprintf(total_quote,
		"R|%s||%s|G|%s|%.4f|%d|%.4f|%d|%.4f|%d|%.0f|%.4f|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.0f|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%s"
		"|||%.4f|%d|%.6f|%.6f"
		"|%s||||||"
		"||||||"
		"%lld|",
		ConvertToShortTimval(p_data->t_).c_str(),
		csTemp.c_str(),                       // ʱ��
		// ����������
		p->szInstrumentID,                                  // ��Լ����
		InvalidToZeroD(p->dLastPrice),            // �ɽ���
		p->nVolume,                            // �ɽ�����������
		InvalidToZeroD(p->dBidPrice1),            // ��һ��
		p->nBidVolume1,                        // ��һ��
		InvalidToZeroD(p->dAskPrice1),            // ��һ��
		p->nAskVolume1,                        // ��һ��
		InvalidToZeroD(p->dOpenInterest),         // �ֲ���
		InvalidToZeroD(p->dPreSettlementPrice),   // ������
		cur_volumn,                           // ��ǰ��

		InvalidToZeroD(p->dBidPrice2),            // �����
		InvalidToZeroD(p->dBidPrice3),            // ������
		InvalidToZeroD(p->dBidPrice4),            // ���ļ�
		InvalidToZeroD(p->dBidPrice5),            // �����
		p->nBidVolume2,                        // �����
		p->nBidVolume3,                        // ������
		p->nBidVolume4,                        // ������
		p->nBidVolume5,                        // ������

		InvalidToZeroD(p->dAskPrice2),            // ������
		InvalidToZeroD(p->dAskPrice3),            // ������
		InvalidToZeroD(p->dAskPrice4),            // ���ļ�
		InvalidToZeroD(p->dAskPrice5),            // �����
		p->nAskVolume2,                        // ������
		p->nAskVolume3,                        // ������
		p->nAskVolume4,                        // ������
		p->nAskVolume5,                        // ������

		InvalidToZeroD(p->dUpperLimitPrice),      // ��ͣ��
		InvalidToZeroD(p->dLowerLimitPrice),      // ��ͣ��
		InvalidToZeroD(p->dOpenPrice),            // ����
		InvalidToZeroD(p->dHighestPrice),         // �������
		InvalidToZeroD(p->dLowestPrice),          // �������
		InvalidToZeroD(p->dPreClosePrice),        // ����
		InvalidToZeroD(p->dPreOpenInterest),      // ��ֲ�
		avg_price,                            // ����

		// 20131015�����������ֶ�
		total_amount,                         // �ۼƳɽ����
		cur_amount,                           // ��ǰ�ɽ����

		0.0, //�ǵ�
		0.0, //�ǵ���
		0.0, //����
		InvalidToZeroD(p->dSettlementPrice), //���ν����
		trade_day.c_str(), //��������

		//���������
		//������
		InvalidToZeroD(p->dClosePrice), //������
		0, //�ɽ�����
		InvalidToZeroD(p->dPreDelta), //����ʵ��
		InvalidToZeroD(p->dCurrDelta), //����ʵ��

		"", //��Լ�ڽ������Ĵ���
		//ͣ�Ʊ�־
		//�ֲ����仯
		//��ʷ��ͼ�
		//��ʷ��߼�
		//�����Ƶ���
		//�����Ƶ���

		//delta����Ȩ�ã�
		//gama����Ȩ�ã�
		//rho����Ȩ�ã�
		//theta����Ȩ�ã�
		//vega����Ȩ�ã�
		p_data->t_
		);

	return total_quote;
}
