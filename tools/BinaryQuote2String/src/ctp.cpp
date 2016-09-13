#include "ctp.h"
#include <string>

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
std::string CTPQuoteToString( const SaveData_CTP * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const CThostFtdcDepthMarketDataField * const p = &(p_data->data_);	
	
	char buffer[64];
	char total_quote[2048];

	std::string quote_time = p->UpdateTime;
	boost::erase_all(quote_time, ":");
	sprintf(buffer, "%03d", p->UpdateMillisec);
	quote_time.append(buffer);

	double total_amount = InvalidToZeroD(p->Turnover);
	double avg_price = InvalidToZeroD(p->AveragePrice);
	int cur_volumn = 0;
	double cur_amount = 0;
	std::string exchange_code("U");
	std::string trade_day(p->TradingDay, 8);

	//R|Ԥ��|Ԥ��|ʱ��|����������|��Լ����|�ɽ���|�ۼƳɽ���|��һ��|��һ��|��һ��|��һ��|�ֲ���|������|��ǰ��
	//|�����|������|���ļ�|�����|�����|������|������|������
	//|������|������|���ļ�|�����|������|������|������|������
	//|��ͣ��|��ͣ��|���տ��̼�|�������|�������|������|��ֲ�|���ճɽ�����
	//|�ۼƳɽ����|��ǰ�ɽ����\n

	sprintf(total_quote,
		"R|||%s|%s|%s|%.4f|%d|%.4f|%d|%.4f|%d|%.0f|%.4f|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%d|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.0f|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%s"
		"|||%.4f|%d|%.6f|%.6f"
		"|%s||||||"
		"||||||"
		"%lld|",
		quote_time.c_str(),                   // ʱ��
		exchange_code.c_str(),                // ����������
		p->InstrumentID,			          // ��Լ����
		InvalidToZeroD(p->LastPrice),            // �ɽ���
		p->Volume,                            // �ɽ�����������
		InvalidToZeroD(p->BidPrice1),            // ��һ��
		p->BidVolume1,                        // ��һ��
		InvalidToZeroD(p->AskPrice1),            // ��һ��
		p->AskVolume1,                        // ��һ��
		InvalidToZeroD(p->OpenInterest),         // �ֲ���
		InvalidToZeroD(p->PreSettlementPrice),   // ������
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

		InvalidToZeroD(p->UpperLimitPrice),      // ��ͣ��
		InvalidToZeroD(p->LowerLimitPrice),      // ��ͣ��
		InvalidToZeroD(p->OpenPrice),            // ����
		InvalidToZeroD(p->HighestPrice),         // �������
		InvalidToZeroD(p->LowestPrice),          // �������
		InvalidToZeroD(p->PreClosePrice),        // ����
		InvalidToZeroD(p->PreOpenInterest),      // ��ֲ�
		avg_price,                            // ����

		// 20131015�����������ֶ�
		total_amount,                         // �ۼƳɽ����
		cur_amount,                           // ��ǰ�ɽ����

		0.0, //�ǵ�
		0.0, //�ǵ���
		0.0, //����
		InvalidToZeroD(p->SettlementPrice), //���ν����
		trade_day.c_str(), //��������

		//���������
		//������
		InvalidToZeroD(p->ClosePrice), //������
		0, //�ɽ�����
		InvalidToZeroD(p->PreDelta), //����ʵ��
		InvalidToZeroD(p->CurrDelta), //����ʵ��

		p->ExchangeInstID, //��Լ�ڽ������Ĵ���
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
