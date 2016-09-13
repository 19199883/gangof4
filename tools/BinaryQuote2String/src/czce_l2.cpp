#include "czce_l2.h"
#include <string>
#include "util.h"

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

std::string CZCE_L2_QuoteToString(const SaveData_CZCE_L2_DATA * const p_data)
{
	if (!p_data)
	{
		return "";
	}

	const ZCEL2QuotSnapshotField_MY * const p = &(p_data->data_);	
	char total_quote[2048];

	// ��ǰ���������л�
	std::string csTemp = p->TimeStamp; // 2014-02-03 13:23:45.180
	boost::erase_all(csTemp, ":");
	boost::erase_all(csTemp, ".");
	csTemp = csTemp.substr(11);


	double total_amount = 0;
	double avg_price = p->AveragePrice;

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

	std::string trade_day = p->TimeStamp;
	boost::erase_all(trade_day, "-");
	trade_day = trade_day.substr(0, 8);


	//R|Ԥ��|Ԥ��|ʱ��|����������|��Լ����|�ɽ���|�ۼƳɽ���|��һ��|��һ��|��һ��|��һ��|�ֲ���|������|��ǰ��
	//|�����|������|���ļ�|�����|�����|������|������|������
	//|������|������|���ļ�|�����|������|������|������|������
	//|��ͣ��|��ͣ��|���տ��̼�|�������|�������|������|��ֲ�|���ճɽ�����
	//|�ۼƳɽ����|��ǰ�ɽ����\n

	sprintf(total_quote,
		"R|%s||%s|C|%s|%.4f|%d|%.4f|%d|%.4f|%d|%.0f|%.4f|%d|"
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
		p->ContractID,                                  // ��Լ����
		InvalidToZeroD(p->LastPrice),            // �ɽ���
		p->TotalVolume,                            // �ɽ�����������
		InvalidToZeroD(p->BidPrice[0]),            // ��һ��
		p->BidLot[0],                        // ��һ��
		InvalidToZeroD(p->AskPrice[0]),            // ��һ��
		p->AskLot[0],                        // ��һ��
		InvalidToZeroD(p->OpenInterest),         // �ֲ���
		InvalidToZeroD(p->PreSettle),   // ������
		cur_volumn,                           // ��ǰ��

		InvalidToZeroD(p->BidPrice[1]),            // �����
		InvalidToZeroD(p->BidPrice[2]),            // ������
		InvalidToZeroD(p->BidPrice[3]),            // ���ļ�
		InvalidToZeroD(p->BidPrice[4]),            // �����
		p->BidLot[1],                        // �����
		p->BidLot[2],                        // ������
		p->BidLot[3],                        // ������
		p->BidLot[4],                        // ������

		InvalidToZeroD(p->AskPrice[1]),            // ������
		InvalidToZeroD(p->AskPrice[2]),            // ������
		InvalidToZeroD(p->AskPrice[3]),            // ���ļ�
		InvalidToZeroD(p->AskPrice[4]),            // �����
		p->AskLot[1],                        // ������
		p->AskLot[2],                        // ������
		p->AskLot[3],                        // ������
		p->AskLot[4],                        // ������

		InvalidToZeroD(p->HighLimit),      // ��ͣ��
		InvalidToZeroD(p->LowLimit),      // ��ͣ��
		InvalidToZeroD(p->OpenPrice),            // ����
		InvalidToZeroD(p->HighPrice),         // �������
		InvalidToZeroD(p->LowPrice),          // �������
		InvalidToZeroD(p->PreClose),        // ����
		InvalidToZeroD(p->PreOpenInterest),      // ��ֲ�
		avg_price,                            // ����

		// 20131015�����������ֶ�
		total_amount,                         // �ۼƳɽ����
		cur_amount,                           // ��ǰ�ɽ����

		0.0, //�ǵ�
		0.0, //�ǵ���
		0.0, //����
		InvalidToZeroD(p->SettlePrice), //���ν����
		trade_day.c_str(), //��������

		//���������
		//������
		InvalidToZeroD(p->ClosePrice), //������
		0, //�ɽ�����
		0.0, //����ʵ��
		0.0, //����ʵ��

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

std::string CZCE_CMB_QuoteToString( const SaveData_CZCE_CMB_DATA * const p_data )
{
	char total_quote[2048];

	const ZCEQuotCMBQuotField_MY * const p = &(p_data->data_);	

	sprintf(total_quote, "%s|%s|%c|"
		"%.4f|%d|%d|%.4f|%d|%d|%lld|",
		p->TimeStamp, p->ContractID, p->CmbType,
		p->BidPrice,p->BidLot, p->VolBidLot, p->AskPrice, p->AskLot, p->VolAskLot,p_data->t_);

	return total_quote;
}
