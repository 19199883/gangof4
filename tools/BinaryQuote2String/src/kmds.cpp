#include "kmds.h"
#include "util.h"
#include <string>

#include <sstream>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

static char GetExchCodeByMarket(int market)
{
	//1 �Ͻ���
	//2 ���
	//4 �н���
	//5 ������
	//6 ������
	//7 ֣����
	//const char CONST_EXCHCODE_SHFE = 'A';  //�Ϻ��ڻ�
	//const char CONST_EXCHCODE_DCE = 'B';  //�����ڻ�
	//const char CONST_EXCHCODE_CZCE = 'C';  //֣���ڻ�
	//const char CONST_EXCHCODE_CFFEX = 'G';  //�н���
	//const char CONST_EXCHCODE_SZEX = '0';  //���
	//const char CONST_EXCHCODE_SHEX = '1';  //�Ͻ���
	char ex = ' ';
	switch(market)
	{
	case 1:ex ='1';break;
	case 2:ex ='0';break;
	case 4:ex ='G';break;
	case 5:ex ='A';break;
	case 6:ex ='B';break;
	case 7:ex ='C';break;
	case 15:ex ='O';break;
	case 19:ex ='O';break;
	default:break;
	}

	return ex;
}

std::string KMDS_QuoteToString( const SaveData_StockQuote_KMDS * const p )
{
	if (!p)
	{
		return "";
	}
	char quote_string[1024];
	const T_StockQuote * const pd = &(p->data_);

	try
	{
		char exch_code = GetExchCodeByMarket(pd->market);
		//ʱ��,������,��Ʊ����,״̬,���¼�,�ܳɽ���,�ɽ��ܽ��,
		//��һ��,�����,������,���ļ�,�����,������,���߼�,��˼�,��ż�,��ʮ��,
		//��һ��,�����,������,������,������,������,������,�����,�����,��ʮ��,
		//��һ��,������,������,���ļ�,�����,������,���߼�,���˼�,���ż�,��ʮ��,
		//��һ��,������,������,������,������,������,������,������,������,��ʮ��,
		//ǰ���̼�,���̼�,��߼�,��ͼ�,��ͣ��,��ͣ��,�ɽ�����,ί����������,ί����������,
		//��Ȩƽ��ί��۸�,��Ȩƽ��ί���۸�,IOPV��ֵ��ֵ,����������,��ӯ��1,��ӯ��2,
		sprintf_s(quote_string, sizeof(quote_string),
			"R||1|%09d|%c|%s|%c|%.4f|%lld|%.4f|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|"
			"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|"
			"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%u|%lld|%lld|"
			"%.4f|%.4f|%d|%d|%d|%d|%llu|",
			pd->time,
			exch_code,
			pd->scr_code,
			pd->status == 0 ? '-' : pd->status,
			pd->last_price / 10000.0,
			pd->bgn_total_vol,
			pd->bgn_total_amt,
			pd->bid_price[0] / 10000.0,
			pd->bid_price[1] / 10000.0,
			pd->bid_price[2] / 10000.0,
			pd->bid_price[3] / 10000.0,
			pd->bid_price[4] / 10000.0,
			pd->bid_price[5] / 10000.0,
			pd->bid_price[6] / 10000.0,
			pd->bid_price[7] / 10000.0,
			pd->bid_price[8] / 10000.0,
			pd->bid_price[9] / 10000.0,
			pd->bid_volume[0],
			pd->bid_volume[1],
			pd->bid_volume[2],
			pd->bid_volume[3],
			pd->bid_volume[4],
			pd->bid_volume[5],
			pd->bid_volume[6],
			pd->bid_volume[7],
			pd->bid_volume[8],
			pd->bid_volume[9],
			pd->sell_price[0] / 10000.0,
			pd->sell_price[1] / 10000.0,
			pd->sell_price[2] / 10000.0,
			pd->sell_price[3] / 10000.0,
			pd->sell_price[4] / 10000.0,
			pd->sell_price[5] / 10000.0,
			pd->sell_price[6] / 10000.0,
			pd->sell_price[7] / 10000.0,
			pd->sell_price[8] / 10000.0,
			pd->sell_price[9] / 10000.0,
			pd->sell_volume[0],
			pd->sell_volume[1],
			pd->sell_volume[2],
			pd->sell_volume[3],
			pd->sell_volume[4],
			pd->sell_volume[5],
			pd->sell_volume[6],
			pd->sell_volume[7],
			pd->sell_volume[8],
			pd->sell_volume[9],
			0.0,
			0.0,
			pd->high_price / 10000.0,
			pd->low_price / 10000.0,
			0.0,
			0.0,
			pd->bgn_count,
			pd->total_bid_lot,
			pd->total_sell_lot,
			pd->wght_avl_bid_price / 10000.0,
			pd->wght_avl_sell_price / 10000.0,
			pd->iopv_net_val_valt,
			pd->mtur_yld,
			pd->pe_ratio_1,
			pd->pe_ratio_2,
			p->t_
			);	}
	catch (...)
	{
		cout << "error occurs when format SaveData_StockQuote_KMDS." << std::endl;
	}

	static bool kmds_stock_flag = true;
	if(kmds_stock_flag)
	{
		kmds_stock_flag = false;
		return std::string("R||1|ʱ��|������|��Ʊ����|״̬|���¼�|�ܳɽ���|�ɽ��ܽ��|��һ��|�����|������|���ļ�|�����|������|���߼�|��˼�|��ż�|��ʮ��|��һ��|�����|������|������|������|������|������|�����|�����|��ʮ��|��һ��|������|������|���ļ�|�����|������|���߼�|���˼�|���ż�|��ʮ��|��һ��|������|������|������|������|������|������|������|������|��ʮ��|ǰ���̼�|���̼�|��߼�|��ͼ�|��ͣ��|��ͣ��|�ɽ�����|ί����������|ί����������|��Ȩƽ��ί��۸�|��Ȩƽ��ί���۸�|IOPV��ֵ��ֵ|����������|��ӯ��1|��ӯ��2|ʱ���|\n") 
			+ std::string(quote_string);
	}

	return quote_string;
}

std::string KMDS_QuoteToString( const SaveData_IndexQuote_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	char quote_string[1024];
	const T_IndexQuote * const p = &(pd->data_);

	try
	{
		char exch_code = GetExchCodeByMarket(p->market);

		sprintf_s(quote_string, sizeof(quote_string),
			"R||2|%09d|%c|%s|%0.4f|%0.4f|%0.4f|%0.4f|%0.4f|%lld|%lld|%llu|",
			p->time,
			exch_code,
			p->scr_code,
			p->last_price / 10000.0,
			0.0,
			p->high_price / 10000.0,
			p->low_price / 10000.0,
			0.0,
			p->bgn_total_vol,
			p->bgn_total_amt,
			pd->t_
			);
	}
	catch (...)
	{
		cout << "error occurs when format SaveData_IndexQuote_KMDS." << std::endl;
	}

	static bool kmds_idx_flag = true;
	if(kmds_idx_flag)
	{
		kmds_idx_flag = false;
		return std::string("R||2|ʱ��|������|��Ʊ����|���¼�|0.0|��߼�|��ͼ�|0.0|�ɽ�����|�ɽ��ܽ��|ʱ���|\n") 
			+ std::string(quote_string);
	}

	return quote_string;
}

std::string KMDS_QuoteToString( const SaveData_Option_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	char quote_string[10240];
	const T_OptionQuote * const p = &(pd->data_);

	try
	{
		char exch_code = GetExchCodeByMarket(p->market);
		//R|Ԥ��|Ԥ��|ʱ��|����������|��Լ����|�ɽ���|�ۼƳɽ���|��һ��|��һ��|��һ��|��һ��|�ֲ���|������|��ǰ��
		//|�����|������|���ļ�|�����|�����|������|������|������
		//|������|������|���ļ�|�����|������|������|������|������
		//|��ͣ��|��ͣ��|���տ��̼�|�������|�������|������|��ֲ�|���ճɽ�����
		//|�ۼƳɽ����|��ǰ�ɽ����\n

		sprintf(quote_string,
			"R|||%09d|%c|%s|%.4f|%lld|%.4f|%lld|%.4f|%lld|%lld|%.4f|%d|"
			"%.4f|%.4f|%.4f|%.4f|%lld|%lld|%lld|%lld|"
			"%.4f|%.4f|%.4f|%.4f|%lld|%lld|%lld|%lld|"
			"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.0f|%.4f|"
			"%.4f|%.4f|"
			"%.4f|%.4f|%.4f|%.4f|"
			"|||%.4f|%d|%.6f|%.6f"
			"|%s||||||"
			"||||||"
			"%lld|",
			(int)p->time,                       // ʱ��
			exch_code,// ����������
			std::string(p->scr_code,8).c_str(),                                  // ��Լ����
			p->last_price / 10000.0,            // �ɽ���
			p->bgn_total_vol,                            // �ɽ�����������
			p->bid_price[0] / 10000.0,            // ��һ��
			p->bid_volume[0],                        // ��һ��
			p->sell_price[0] / 10000.0,            // ��һ��
			p->sell_volume[0],                        // ��һ��
			p->long_position,         // �ֲ���
			0.0,   // ������
			0,                           // ��ǰ��

			p->bid_price[1] / 10000.0,            // �����
			p->bid_price[2] / 10000.0,            // ������
			p->bid_price[3] / 10000.0,            // ���ļ�
			p->bid_price[4] / 10000.0,            // �����
			p->bid_volume[1],                        // �����
			p->bid_volume[2],                        // ������
			p->bid_volume[3],                        // ������
			p->bid_volume[4],                        // ������

			p->sell_price[1] / 10000.0,            // ������
			p->sell_price[2] / 10000.0,            // ������
			p->sell_price[3] / 10000.0,            // ���ļ�
			p->sell_price[4] / 10000.0,            // �����
			p->sell_volume[1],                        // ������
			p->sell_volume[2],                        // ������
			p->sell_volume[3],                        // ������
			p->sell_volume[4],                        // ������

			p->high_limit_price / 10000.0,      // ��ͣ��
			p->low_limit_price / 10000.0,      // ��ͣ��
			p->open_price / 10000.0,            // ����
			p->high_price / 10000.0,         // �������
			p->low_price / 10000.0,          // �������
			p->pre_close_price / 10000.0,        // ����
			0.0,      // ��ֲ�
			0.0,                            // ����

			// 20131015�����������ֶ�
			p->bgn_total_amt / 10000.0,                         // �ۼƳɽ����
			0.0,                           // ��ǰ�ɽ����

			0.0, //�ǵ�
			0.0, //�ǵ���
			0.0, //����
			0.0, //���ν����
			//��������

			//���������
			//������
			0.0, //������
			0, //�ɽ�����
			0.0, //����ʵ��
			0.0, //����ʵ��

			p->contract_code, //��Լ�ڽ������Ĵ���
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
			pd->t_
			);
	}
	catch (...)
	{
		cout << "error occurs when format SaveData_Option_KMDS." << std::endl;
	}

	return quote_string;
}

std::string KMDS_QuoteToString( const SaveData_FutureQuote_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	char quote_string[1024];
	const T_FutureQuote * const p = &(pd->data_);

	try
	{
	}
	catch (...)
	{
		cout << "error occurs when format SaveData_FutureQuote_KMDS." << std::endl;
	}

	return quote_string;
}

std::string KMDS_QuoteToString( const SaveData_OrderQueue_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	const T_OrderQueue * const p = &(pd->data_);

	char time_str[64];
	sprintf(time_str, "%09d", p->time);

	std::stringstream ss;
	// ʱ�䣬�г������룬�����۸񣬶�����������ϸ������������ϸ����
	ss << std::fixed << std::setprecision(2)
		<< ConvertToShortTimval(pd->t_) << ","
		<< time_str << ","
		<< p->market << ","
		<< p->scr_code << ","
		<< p->insr_txn_tp_code << ","
		<< p->ord_price << ","
		<< p->ord_qty << ","
		<< p->ord_nbr << ",";
	for (int i = 0; i < p->ord_nbr; ++i)
	{
		ss << p->ord_detail_vol[i] << "|";
	}
	ss << "," << pd->t_;

	static bool kmds_queue_flag = true;
	if(kmds_queue_flag)
	{
		kmds_queue_flag = false;
		return std::string("local_time,ʱ��,������,��Ʊ����,��������,�۸�,��������,��ϸ����,(��ϸ����),ʱ���,\n") 
			+ ss.str();
	}

	return ss.str();
}

std::string KMDS_QuoteToString( const SaveData_PerEntrust_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}
	const T_PerEntrust * const p = &(pd->data_);

	char time_str[64];
	sprintf(time_str, "%09d", p->entrt_time);

	std::stringstream ss;
	// ί��ʱ�䣬�г������룬ί�мۣ�ί�б�ţ�ί������ָ������ͣ�B��S����ί�����
	ss << std::fixed << std::setprecision(2)
		<< ConvertToShortTimval(pd->t_) << ","
		<< time_str << ","
		<< p->market << ","
		<< p->scr_code << ","
		<< p->entrt_id << ","
		<< p->entrt_price << ","
		<< p->entrt_vol << ","
		<< std::string(p->insr_txn_tp_code) << ","
		<< std::string(p->entrt_tp) << ","
		<< pd->t_ << ","
		;

	static bool kmds_perentrust_flag = true;
	if(kmds_perentrust_flag)
	{
		kmds_perentrust_flag = false;
		return std::string("local_time,ʱ��,������,��Ʊ����,ί�б��,ί�м�,ί����,��������,ί�����,ʱ���,\n") 
			+ ss.str();
	}

	return ss.str();
}

std::string KMDS_QuoteToString( const SaveData_PerBargain_KMDS * const pd )
{
	if (!pd)
	{
		return "";
	}

	const T_PerBargain * const p = &(pd->data_);

	char time_str[64];
	sprintf(time_str, "%09d", p->time);

	std::stringstream ss;
	// �ɽ�ʱ�䣬�г������룬�ɽ��ۣ��ɽ���ţ��ɽ������ɽ����ɽ����ָ�������
	ss << std::fixed << std::setprecision(2)
		<< ConvertToShortTimval(pd->t_) << ","
		<< time_str << ","
		<< p->market << ","
		<< p->scr_code << ","
		<< p->bgn_price << ","
		<< p->bgn_id << ","
		<< p->bgn_qty << ","
		<< p->bgn_amt << ","
		<< std::string(p->nsr_txn_tp_code) << ","
		<< std::string(p->bgn_flg) << ","
		<< pd->t_ << ","
		;

	static bool kmds_perbargain_flag = true;
	if(kmds_perbargain_flag)
	{
		kmds_perbargain_flag = false;
		return std::string("local_time,ʱ��,������,��Ʊ����,�ɽ���,�ɽ����,�ɽ���,�ɽ����,��������,�ɽ����,ʱ���,\n") 
			+ ss.str();
	}

	return ss.str();
}
