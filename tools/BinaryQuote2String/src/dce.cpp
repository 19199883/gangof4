#include "dce.h"
#include <string>

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "util.h"

using namespace std;

#pragma pack(push)
#pragma pack(4)
	////////////////////////////////////////////////
	///MDBestAndDeep���������嵵�������
	////////////////////////////////////////////////
	struct bd_tt{
		INT1		Type;
		UINT4		Length;							//���ĳ���
		UINT4		Version;						//�汾��1��ʼ
		UINT4		Time;							//Ԥ���ֶ�
		INT1		Exchange[3];					//������
		INT1		Contract[80];					//��Լ����
		BOOL		SuspensionSign;					//ͣ�Ʊ�־
		REAL4		LastClearPrice;					//������
		REAL4		ClearPrice;						//������
		REAL4		AvgPrice;						//�ɽ�����
		REAL4		LastClose;						//������
		REAL4		Close;							//������
		REAL4		OpenPrice;						//����
		UINT4		LastOpenInterest;				//��ֲ���
		UINT4		OpenInterest;					//�ֲ���
		REAL4		LastPrice;						//���¼�
		UINT4		MatchTotQty;					//�ɽ�����
		REAL8		Turnover;						//�ɽ����
		REAL4		RiseLimit;						//��߱���
		REAL4		FallLimit;						//��ͱ���
		REAL4		HighPrice;						//��߼�
		REAL4		LowPrice;						//��ͼ�
		REAL4		PreDelta;						//����ʵ��
		REAL4		CurrDelta;						//����ʵ��
		REAL4		BuyPriceOne;					//����۸�1
		UINT4		BuyQtyOne;						//��������1
		UINT4		BuyImplyQtyOne;					//��1�Ƶ���
		REAL4		BuyPriceTwo;
		UINT4		BuyQtyTwo;
		UINT4		BuyImplyQtyTwo;
		REAL4		BuyPriceThree;
		UINT4		BuyQtyThree;
		UINT4		BuyImplyQtyThree;
		REAL4		BuyPriceFour;
		UINT4		BuyQtyFour;
		UINT4		BuyImplyQtyFour;
		REAL4		BuyPriceFive;
		UINT4		BuyQtyFive;
		UINT4		BuyImplyQtyFive;
		REAL4		SellPriceOne;					//�����۸�1
		UINT4		SellQtyOne;						//�������1
		UINT4		SellImplyQtyOne;				//��1�Ƶ���
		REAL4		SellPriceTwo;
		UINT4		SellQtyTwo;
		UINT4		SellImplyQtyTwo;
		REAL4		SellPriceThree;
		UINT4		SellQtyThree;
		UINT4		SellImplyQtyThree;
		REAL4		SellPriceFour;
		UINT4		SellQtyFour;
		UINT4		SellImplyQtyFour;
		REAL4		SellPriceFive;
		UINT4		SellQtyFive;
		UINT4		SellImplyQtyFive;
		INT1		GenTime[13];					//�������ʱ��
		UINT4		LastMatchQty;					//���³ɽ���
		INT4		InterestChg;					//�ֲ����仯
		REAL4		LifeLow;						//��ʷ��ͼ�
		REAL4		LifeHigh;						//��ʷ��߼�
		REAL8		Delta;							//delta
		REAL8		Gamma;							//gama
		REAL8		Rho;							//rho
		REAL8		Theta;							//theta
		REAL8		Vega;							//vega
		INT1		TradeDate[9];					//��������
		INT1		LocalDate[9];					//��������
	};
#pragma pack(pop)



std::string BestAndDeepToString( const SaveData_MDBestAndDeep * const p_data )
{
	if (!p_data)
	{
		return "";
	}
	double d1 = DBL_MAX;
	double d2 = DBL_MIN;
	float f1 = FLT_MAX;
	float f2 = FLT_MIN;
	const MDBestAndDeep_MY &quote = p_data->data_;

	const bd_tt * ttp = (const bd_tt *)&p_data->data_;
	char total_quote[2048];

	bool b1 = quote.ClearPrice > f1;
	bool b2 = quote.ClearPrice < f2;
	bool b3 = quote.Delta > d1;
	bool b4 = quote.Delta < d2;

	double d3 = InvalidToZeroD(quote.Delta);
	float f3 = InvalidToZeroF(quote.ClearPrice);

	// ��ǰ���������л�
	string quote_time = quote.GenTime;
	boost::erase_all(quote_time, ":");
	boost::erase_all(quote_time, ".");
	if (quote_time.empty())
	{
		quote_time = "0";
	}

	// ��ǰ��, ��ǰ�ɽ����
	double total_amount = InvalidToZeroD(quote.Turnover);
	unsigned int cur_volumn = quote.LastMatchQty;
	double cur_amount = 0;

	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);

	//InstrumentToVolumnAndAmountMap::iterator it = best_quoteinfo_.find(quote.Contract);
	//if (it != best_quoteinfo_.end())
	//{
	//	cur_volumn = quote.MatchTotQty - it->second.first;
	//	cur_amount = total_amount - it->second.second;
	//	it->second = std::make_pair(quote.MatchTotQty, total_amount);
	//}
	//else
	//{
	//	cur_volumn = quote.MatchTotQty;
	//	cur_amount = total_amount;
	//	best_quoteinfo_.insert(std::make_pair(quote.Contract, 
	//		std::make_pair(quote.MatchTotQty, total_amount)));
	//}

	//R|Ԥ��|Ԥ��|ʱ��|����������|��Լ����|�ɽ���|�ۼƳɽ���|��һ��|��һ��|��һ��|��һ��|�ֲ���|������|��ǰ��|
	//�����|������|���ļ�|�����|�����|������|������|������|
	//������|������|���ļ�|�����|������|������|������|������|
	//��ͣ��|��ͣ��|���տ��̼�|�������|�������|������|��ֲ�|���ճɽ�����|
	//�ۼƳɽ����|��ǰ�ɽ����\n

	sprintf(total_quote,
		"R|||%s|B|%s|%.4f|%u|%.4f|%u|%.4f|%u|%u|%.4f|%u|"
		"%.4f|%.4f|%.4f|%.4f|%u|%u|%u|%u|"
		"%.4f|%.4f|%.4f|%.4f|%u|%u|%u|%u|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%u|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%08d|"
		"||%.4f|%d|%.6f|%.6f|"
		"%s|%d|%d|%.6f|%.6f|%d|%d|"
		"%.6f|%.6f|%.6f|%.6f|%.6f|"
		"%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|"
		"%lld|",
		quote_time.c_str(),               // ʱ��
		// ����������
		quote.Contract,                      // ��Լ����
		InvalidToZeroF(quote.LastPrice),        // �ɽ���
		quote.MatchTotQty,                   // �ɽ�����������
		InvalidToZeroF(quote.BuyPriceOne),      // ��һ��
		quote.BuyQtyOne,                     // ��һ��
		InvalidToZeroF(quote.SellPriceOne),     // ��һ��
		quote.SellQtyOne,                    // ��һ��
		quote.OpenInterest,                  // �ֲ���
		InvalidToZeroF(quote.LastClearPrice),   // ������
		cur_volumn,                       // ��ǰ��

		InvalidToZeroF(quote.BuyPriceTwo),      // �����
		InvalidToZeroF(quote.BuyPriceThree),    // ������
		InvalidToZeroF(quote.BuyPriceFour),     // ���ļ�
		InvalidToZeroF(quote.BuyPriceFive),     // �����
		quote.BuyQtyTwo,                     // �����
		quote.BuyQtyThree,                   // ������
		quote.BuyQtyFour,                    // ������
		quote.BuyQtyFive,                    // ������

		InvalidToZeroF(quote.SellPriceTwo),     // ������
		InvalidToZeroF(quote.SellPriceThree),   // ������
		InvalidToZeroF(quote.SellPriceFour),    // ���ļ�
		InvalidToZeroF(quote.SellPriceFive),    // �����
		quote.SellQtyTwo,                    // ������
		quote.SellQtyThree,                  // ������
		quote.SellQtyFour,                   // ������
		quote.SellQtyFive,                   // ������

		quote.RiseLimit,                     // ��ͣ��
		quote.FallLimit,                     // ��ͣ��
		InvalidToZeroF(quote.OpenPrice),        // ����
		InvalidToZeroF(quote.HighPrice),        // �������
		InvalidToZeroF(quote.LowPrice),         // �������
		InvalidToZeroF(quote.LastClose),        // ����
		quote.LastOpenInterest,              // ��ֲ�
		InvalidToZeroF(quote.AvgPrice),         // ����

		// 20131015�����������ֶ�
		total_amount,                     // �ۼƳɽ����
		cur_amount,                       // ��ǰ�ɽ����

		0.0, //�ǵ�
		0.0, //�ǵ���
		0.0, //����
		InvalidToZeroD(quote.ClearPrice),		  //���ν����
		d_, //��������

		//���������
		//������
		InvalidToZeroD(quote.Close),					//������
		0,										//�ɽ�����
		InvalidToZeroD(quote.PreDelta),				//����ʵ��
		InvalidToZeroD(quote.CurrDelta),				//����ʵ��

		"",										//��Լ�ڽ������Ĵ���
		(int)quote.SuspensionSign,					//ͣ�Ʊ�־
		quote.InterestChg,							//�ֲ����仯
		InvalidToZeroD(quote.LifeLow),				//��ʷ��ͼ�
		InvalidToZeroD(quote.LifeHigh),				//��ʷ��߼�
		quote.BuyImplyQtyOne,			//�����Ƶ���
		quote.SellImplyQtyOne,			//�����Ƶ���

		InvalidToZeroD(quote.Delta),					//delta����Ȩ�ã�
		InvalidToZeroD(quote.Gamma),					//gama����Ȩ�ã�
		InvalidToZeroD(quote.Rho),					//rho����Ȩ�ã�
		InvalidToZeroD(quote.Theta),					//theta����Ȩ�ã�
		InvalidToZeroD(quote.Vega),					//vega����Ȩ�ã�	

		quote.BuyImplyQtyOne,
		quote.BuyImplyQtyTwo,
		quote.BuyImplyQtyThree,
		quote.BuyImplyQtyFour,
		quote.BuyImplyQtyFive,
		quote.SellImplyQtyOne,
		quote.SellImplyQtyTwo,
		quote.SellImplyQtyThree,
		quote.SellImplyQtyFour,
		quote.SellImplyQtyFive,

		p_data->t_
		);

	return total_quote;
}

std::string ArbiToString( const SaveData_Arbi * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDBestAndDeep_MY &quote = p_data->data_;
	char total_quote[2048];

	// ��ǰ���������л�
	string quote_time = quote.GenTime;
	boost::erase_all(quote_time, ":");
	boost::erase_all(quote_time, ".");
	if (quote_time.empty())
	{
		quote_time = "0";
	}

	// ��ǰ��, ��ǰ�ɽ����
	double total_amount = InvalidToZeroD(quote.Turnover);
	unsigned int cur_volumn = 0;
	double cur_amount = 0;
	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);
	//InstrumentToVolumnAndAmountMap::iterator it = best_quoteinfo_.find(quote.Contract);
	//if (it != best_quoteinfo_.end())
	//{
	//	cur_volumn = quote.MatchTotQty - it->second.first;
	//	cur_amount = total_amount - it->second.second;
	//	it->second = std::make_pair(quote.MatchTotQty, total_amount);
	//}
	//else
	//{
	//	cur_volumn = quote.MatchTotQty;
	//	cur_amount = total_amount;
	//	best_quoteinfo_.insert(std::make_pair(quote.Contract, 
	//		std::make_pair(quote.MatchTotQty, total_amount)));
	//}

	//R|Ԥ��|Ԥ��|ʱ��|����������|��Լ����|�ɽ���|�ۼƳɽ���|��һ��|��һ��|��һ��|��һ��|�ֲ���|������|��ǰ��|
	//�����|������|���ļ�|�����|�����|������|������|������|
	//������|������|���ļ�|�����|������|������|������|������|
	//��ͣ��|��ͣ��|���տ��̼�|�������|�������|������|��ֲ�|���ճɽ�����|
	//�ۼƳɽ����|��ǰ�ɽ����\n

	sprintf(total_quote,
		"R|||%s|B|%s|%.4f|%u|%.4f|%u|%.4f|%u|%u|%.4f|%u|"
		"%.4f|%.4f|%.4f|%.4f|%u|%u|%u|%u|"
		"%.4f|%.4f|%.4f|%.4f|%u|%u|%u|%u|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%u|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%08d|"
		"||%.4f|%d|%.6f|%.6f|"
		"%s|%d|%d|%.6f|%.6f|%.6f|%.6f|"
		"%.6f|%.6f|%.6f|%.6f|%.6f|"
		"%lld|",
		quote_time.c_str(),               // ʱ��
		// ����������
		quote.Contract,                      // ��Լ����
		InvalidToZeroF(quote.LastPrice),        // �ɽ���
		quote.MatchTotQty,                   // �ɽ�����������
		InvalidToZeroF(quote.BuyPriceOne),      // ��һ��
		quote.BuyQtyOne,                     // ��һ��
		InvalidToZeroF(quote.SellPriceOne),     // ��һ��
		quote.SellQtyOne,                    // ��һ��
		quote.OpenInterest,                  // �ֲ���
		InvalidToZeroF(quote.LastClearPrice),   // ������
		cur_volumn,                       // ��ǰ��

		InvalidToZeroF(quote.BuyPriceTwo),      // �����
		InvalidToZeroF(quote.BuyPriceThree),    // ������
		InvalidToZeroF(quote.BuyPriceFour),     // ���ļ�
		InvalidToZeroF(quote.BuyPriceFive),     // �����
		quote.BuyQtyTwo,                     // �����
		quote.BuyQtyThree,                   // ������
		quote.BuyQtyFour,                    // ������
		quote.BuyQtyFive,                    // ������

		InvalidToZeroF(quote.SellPriceTwo),     // ������
		InvalidToZeroF(quote.SellPriceThree),   // ������
		InvalidToZeroF(quote.SellPriceFour),    // ���ļ�
		InvalidToZeroF(quote.SellPriceFive),    // �����
		quote.SellQtyTwo,                    // ������
		quote.SellQtyThree,                  // ������
		quote.SellQtyFour,                   // ������
		quote.SellQtyFive,                   // ������

		quote.RiseLimit,                     // ��ͣ��
		quote.FallLimit,                     // ��ͣ��
		InvalidToZeroF(quote.OpenPrice),        // ����
		InvalidToZeroF(quote.HighPrice),        // �������
		InvalidToZeroF(quote.LowPrice),         // �������
		InvalidToZeroF(quote.LastClose),        // ����
		quote.LastOpenInterest,              // ��ֲ�
		InvalidToZeroF(quote.AvgPrice),         // ����

		// 20131015�����������ֶ�
		total_amount,                     // �ۼƳɽ����
		cur_amount,                       // ��ǰ�ɽ����

		0.0, //�ǵ�
		0.0, //�ǵ���
		0.0, //����
		InvalidToZeroD(quote.ClearPrice),		  //���ν����
		d_, //��������

		//���������
		//������
		InvalidToZeroD(quote.Close),					//������
		0,										//�ɽ�����
		InvalidToZeroD(quote.PreDelta),				//����ʵ��
		InvalidToZeroD(quote.CurrDelta),				//����ʵ��

		"",										//��Լ�ڽ������Ĵ���
		(int)quote.SuspensionSign,					//ͣ�Ʊ�־
		quote.InterestChg,							//�ֲ����仯
		InvalidToZeroD(quote.LifeHigh),				//��ʷ��ͼ�
		InvalidToZeroD(quote.LifeLow),				//��ʷ��߼�
		InvalidToZeroD(quote.BuyImplyQtyOne),			//�����Ƶ���
		InvalidToZeroD(quote.SellImplyQtyOne),			//�����Ƶ���

		InvalidToZeroD(quote.Delta),					//delta����Ȩ�ã�
		InvalidToZeroD(quote.Gamma),					//gama����Ȩ�ã�
		InvalidToZeroD(quote.Rho),					//rho����Ȩ�ã�
		InvalidToZeroD(quote.Theta),					//theta����Ȩ�ã�
		InvalidToZeroD(quote.Vega),					//vega����Ȩ�ã�				

		p_data->t_
		);

	return total_quote;
}

std::string TenEntrustToString( const SaveData_MDTenEntrust * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDTenEntrust_MY &quote = p_data->data_;
	char total_quote[2048];

	// ��ǰ���������л�
	string quote_time = quote.GenTime;
	boost::erase_all(quote_time, ":");
	boost::erase_all(quote_time, ".");
	if (quote_time.empty())
	{
		quote_time = "0";
	}
	//"R,Ԥ��,Ԥ��,ʱ��,����������,��Լ����,�������,��������,"
	//"ί����1,ί����2,ί����3,ί����4,ί����5,ί����6,ί����7,ί����8,ί����9,ί����10,"
	//"ί����1,ί����2,ί����3,ί����4,ί����5,ί����6,ί����7,ί����8,ί����9,ί����10"
	sprintf(total_quote,
		"R|||%s|B|%s|"
		"%.4f|%.4f|"
		"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
		"%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|"
		"%lld|",
		quote_time.c_str(),							// ʱ��
		// ����������
		quote.Contract,								// ��Լ����

		InvalidToZeroD(quote.BestBuyOrderPrice),			// �������
		InvalidToZeroD(quote.BestSellOrderPrice),			// ��������

		quote.BestBuyOrderQtyOne,			// ί����1
		quote.BestBuyOrderQtyTwo,			// ί����2
		quote.BestBuyOrderQtyThree,		// ί����3
		quote.BestBuyOrderQtyFour,			// ί����4
		quote.BestBuyOrderQtyFive,			// ί����5
		quote.BestBuyOrderQtySix,			// ί����6
		quote.BestBuyOrderQtySeven,		// ί����7
		quote.BestBuyOrderQtyEight,		// ί����8
		quote.BestBuyOrderQtyNine,			// ί����9
		quote.BestBuyOrderQtyTen,			// ί����10

		quote.BestSellOrderQtyOne,			// ί����1
		quote.BestSellOrderQtyTwo,			// ί����2
		quote.BestSellOrderQtyThree,		// ί����3
		quote.BestSellOrderQtyFour,		// ί����4
		quote.BestSellOrderQtyFive,		// ί����5
		quote.BestSellOrderQtySix,			// ί����6
		quote.BestSellOrderQtySeven,		// ί����7
		quote.BestSellOrderQtyEight,		// ί����8
		quote.BestSellOrderQtyNine,		// ί����9
		quote.BestSellOrderQtyTen,			// ί����10

		p_data->t_
		);

	return total_quote;
}

std::string OrderStatisticToString( const SaveData_MDOrderStatistic * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDOrderStatistic_MY &quote = p_data->data_;
	char total_quote[2048];
	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);

	//"R,Ԥ��,Ԥ��,ʱ��,����������,��Լ����,"
	//"��ί������,��ί������,��Ȩƽ��ί��۸�,��Ȩƽ��ί���۸�"
	sprintf(total_quote,
		"R|||%09d|B|%s|"
		"%u|%u|%.6f|%.6f|"
		"%lld|",
		t_,							// ʱ��
		// ����������
		quote.ContractID,					// ��Լ����

		quote.TotalBuyOrderNum,			// ��ί������
		quote.TotalSellOrderNum,			// ��ί������
		InvalidToZeroD(quote.WeightedAverageBuyOrderPrice),	// ��Ȩƽ��ί��۸�
		InvalidToZeroD(quote.WeightedAverageSellOrderPrice),	// ��Ȩƽ��ί���۸�

		p_data->t_
		);

	return total_quote;
}

std::string RealTimePriceToString( const SaveData_MDRealTimePrice * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDRealTimePrice_MY &quote = p_data->data_;
	char total_quote[2048];
	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);

	//"R,Ԥ��,Ԥ��,ʱ��,����������,��Լ����,ʵʱ�����"
	sprintf(total_quote,
		"R|||%09d|B|%s|%.4f|"
		"%lld|",
		t_,							// ʱ��
		// ����������
		quote.ContractID,					// ��Լ����
		InvalidToZeroD(quote.RealTimePrice),	// ʵʱ�����

		p_data->t_
		);

	return total_quote;
}

std::string MarchPriceQtyToString( const SaveData_MDMarchPriceQty * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const MDMarchPriceQty_MY &quote = p_data->data_;
	char total_quote[2048];
	int d_, t_;
	ConvertTimval(p_data->t_, d_, t_);

	//"R,Ԥ��,Ԥ��,ʱ��,����������,��Լ����,"
	//"�۸�1,������1,��ƽ����1,��������1,��ƽ����1,"
	//"�۸�2,������2,��ƽ����2,��������2,��ƽ����2,"
	//"�۸�3,������3,��ƽ����3,��������3,��ƽ����3,"
	//"�۸�4,������4,��ƽ����4,��������4,��ƽ����4,"
	//"�۸�5,������5,��ƽ����5,��������5,��ƽ����5,"
	sprintf(total_quote,
		"R|||%09d|B|%s|"
		"%.4f|%u|%u|%u|%u|"
		"%.4f|%u|%u|%u|%u|"
		"%.4f|%u|%u|%u|%u|"
		"%.4f|%u|%u|%u|%u|"
		"%.4f|%u|%u|%u|%u|"
		"%lld|",
		t_,							// ʱ��
		// ����������
		quote.ContractID,					// ��Լ����

		InvalidToZeroD(quote.PriceOne),	// �۸�1
		quote.PriceOneBOQty,			// ������1
		quote.PriceOneBEQty,			// ��ƽ����1
		quote.PriceOneSOQty,			// ��������1
		quote.PriceOneSEQty,			// ��ƽ����1

		InvalidToZeroD(quote.PriceTwo),	// �۸�2
		quote.PriceTwoBOQty,			// ������2
		quote.PriceTwoBEQty,			// ��ƽ����2
		quote.PriceTwoSOQty,			// ��������2
		quote.PriceTwoSEQty,			// ��ƽ����2

		InvalidToZeroD(quote.PriceThree),	// �۸�3
		quote.PriceThreeBOQty,			// ������3
		quote.PriceThreeBEQty,			// ��ƽ����3
		quote.PriceThreeSOQty,			// ��������3
		quote.PriceThreeSEQty,			// ��ƽ����3

		InvalidToZeroD(quote.PriceFour),	// �۸�4
		quote.PriceFourBOQty,			// ������4
		quote.PriceFourBEQty,			// ��ƽ����4
		quote.PriceFourSOQty,			// ��������4
		quote.PriceFourSEQty,			// ��ƽ����4

		InvalidToZeroD(quote.PriceFive),	// �۸�5
		quote.PriceFiveBOQty,			// ������5
		quote.PriceFiveBEQty,			// ��ƽ����5
		quote.PriceFiveSOQty,			// ��������5
		quote.PriceFiveSEQty,			// ��ƽ����5

		p_data->t_
		);

	return total_quote;
}
