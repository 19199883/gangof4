#include "ksg.h"
#include <string>

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "util.h"

using namespace std;

std::string KSG_QuoteToString(const SaveData_SH_GOLD * const p_data)
{
	if (!p_data)
	{
		return "";
	}

	const CKSG_DepthMarketDataField * const p = &(p_data->data_);	
	char total_quote[2048];

	// ��ǰ���������л� 
	std::string quote_time = ConvertToShortTimval(p_data->t_); // HHMMSSmmm

	double total_amount = p->Turnover;
	double avg_price = p->AveragePrice;

	// ��ǰ��, ��ǰ�ɽ����
	int cur_volumn = 0;
	double cur_amount = 0;

	//R|Ԥ��|Ԥ��|ʱ��|����������|��Լ����|�ɽ���|�ۼƳɽ���|��һ��|��һ��|��һ��|��һ��|�ֲ���|������|��ǰ��
	//|�����|������|���ļ�|�����|�����|������|������|������
	//|������|������|���ļ�|�����|������|������|������|������
	//|��ͣ��|��ͣ��|���տ��̼�|�������|�������|������|��ֲ�|���ճɽ�����
	//|�ۼƳɽ����|��ǰ�ɽ����\n

	sprintf(total_quote,
		"R|||%s|G|%s|%.4f|%d|%.4f|%d|%.4f|%d|%.0f|%.4f|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.0f|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%s"
		"|||%.4f|%d|%.6f|%.6f"
		"|%s||||||"
		"||||||"
		"%lld|",
		quote_time.c_str(),                       // ʱ��
		// ����������
		TrimAndToString(p->ContractID, 8).c_str(),                                  // ��Լ����
		InvalidToZeroD(p->LastPrice),            // �ɽ���
		p->MatchTotQty,                            // �ɽ�����������
		InvalidToZeroD(p->BidPrice1),            // ��һ��
		p->BidVolume1,                        // ��һ��
		InvalidToZeroD(p->AskPrice1),            // ��һ��
		p->AskVolume1,                        // ��һ��
		InvalidToZeroD(p->OpenInterest),         // �ֲ���
		InvalidToZeroD(p->LastClearPrice),   // ������
		cur_volumn,                           // ��ǰ��

		InvalidToZeroD(p->BidPrice2),            // �����
		InvalidToZeroD(p->BidPrice3),            // ������
		InvalidToZeroD(p->BidPrice4),            // ���ļ�
		InvalidToZeroD(p->BidPrice5),            // �����
		p->BidVolume2,                        // �����
		p->BidVolume3,                        // ������
		p->BidVolume4,                        // ������
		p->BidVolume5,                        // ������

		InvalidToZeroD(p->AskPrice2),            // ������
		InvalidToZeroD(p->AskPrice3),            // ������
		InvalidToZeroD(p->AskPrice4),            // ���ļ�
		InvalidToZeroD(p->AskPrice5),            // �����
		p->AskVolume2,                        // ������
		p->AskVolume3,                        // ������
		p->AskVolume4,                        // ������
		p->AskVolume5,                        // ������

		InvalidToZeroD(p->RiseLimit),      // ��ͣ��
		InvalidToZeroD(p->FallLimit),      // ��ͣ��
		InvalidToZeroD(p->OpenPrice),            // ����
		InvalidToZeroD(p->HighPrice),         // �������
		InvalidToZeroD(p->LowPrice),          // �������
		InvalidToZeroD(p->LastClose),        // ����
		0.0,      // ��ֲ�
		avg_price,                            // ����

		// 20131015�����������ֶ�
		total_amount,                         // �ۼƳɽ����
		cur_amount,                           // ��ǰ�ɽ����

		0.0, //�ǵ�
		0.0, //�ǵ���
		0.0, //����
		InvalidToZeroD(p->ClearPrice), //���ν����
		std::string(p->TradingDay, 8).c_str(), //��������

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
