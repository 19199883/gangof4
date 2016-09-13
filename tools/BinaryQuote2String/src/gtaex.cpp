
#include "gtaex.h"

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <Windows.h>

InstrumentToAmountMap instrument_to_cffex_quoteinfo_;

// ������Ϣ��GTA������ʱ��û�к���������ָ��ǰ��1s�������飬
// ���ݵ�ǰ���Ƿ��״��յ������Ӻ�����Ϣ
struct QuoteInfo
{
	std::string security_code;
	std::string update_time;

	QuoteInfo(const std::string &s, const std::string &t)
		:security_code(s), update_time(t)
	{

	}

	bool operator==(const QuoteInfo &rhs)
	{
		return this->security_code == rhs.security_code;
	}
};
typedef std::vector<QuoteInfo> QuoteDatas;
static QuoteDatas s_quotes_;
static boost::mutex s_quote_mutex_;

using namespace std;
#include "gtaex.h"

std::string GTAQuoteToString( const SaveData_GTAEX * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const ZJS_Future_Input_MY * const p = &(p_data->data_);
	char whole_quote_str[2048];
	bool first_of_this_second = false;
	std::string code(p->szStockNo, 6);
	std::string time_str;
	{
		boost::mutex::scoped_lock lock(s_quote_mutex_);
		QuoteDatas::iterator it;
		for (it = s_quotes_.begin(); it != s_quotes_.end(); ++it)
		{
			if (it->security_code == code)
			{
				break;
			}
		}

		if (it == s_quotes_.end())
		{
			s_quotes_.push_back(QuoteInfo(code, p->szUpdateTime));
			it = s_quotes_.end() - 1;
			first_of_this_second = true;
		}

		time_str.append(p->szUpdateTime, 2);
		time_str.append(p->szUpdateTime + 3, 2);
		time_str.append(p->szUpdateTime + 6, 2);
		if (first_of_this_second || it->update_time != p->szUpdateTime)
		{
			time_str.append("000");
			it->update_time = p->szUpdateTime;
		}
		else
		{
			time_str.append("500");
		}
	}

	// ��ǰ�ɽ����
	double cur_amount = 0;
	InstrumentToAmountMap::iterator it = instrument_to_cffex_quoteinfo_.find(code);
	if (it != instrument_to_cffex_quoteinfo_.end())
	{
		cur_amount = p->dAmount - it->second;
		it->second = p->dAmount;
	}
	else
	{
		cur_amount = p->dAmount;
		instrument_to_cffex_quoteinfo_.insert(std::make_pair(code, p->dAmount));
	}
	std::string trade_day(p->szTradingDay, 8); 

	//R|||134500500|G|IF1309|2182.8|41533|2182.8|4|2183.0|1|22549|2231|7|2182.2|2182.0|2181.8|2181.6|13|9|2|5|2184.0|2184.6|2184.8|2185.0|6|5|18|14|2454.0|2008.0|2230.8|2235.2|2164.0|2236.6|21357|2199.05|
	// ��ծ�ڻ����۸�Ҫ����С�����4λ
	sprintf_s(whole_quote_str, "R|||%s|G|%s|%.4f|%.0f|%.4f|%.0f|%.4f|%.0f|%.0f|%.4f|%.0f|"
		"%.4f|%.4f|%.4f|%.4f|%.0f|%.0f|%.0f|%.0f|%.4f|%.4f|%.4f|%.4f|%.0f|%.0f|%.0f|%.0f|"
		"%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%.0f|%.4f|"
		"%.4f|%.4f|"
		"%.4f|%.4f|%.4f|%.4f|%s"
		"|||%.4f|%d|%.6f|%.6f"
		"|||||||"
		"||||||%llu|",
		time_str.c_str(),	// ʱ��               
		code.c_str(),		// stock       
		p->dLastTrade,		// �ɽ���               
		p->dVolume,			// �ɽ�����������       
		p->dBuyprice1,		// ��һ��               
		p->dBuyvol1,		// ��һ��               
		p->dSellprice1,		// ��һ��           
		p->dSellvol1,		// ��һ��               
		p->dOpenInterest,	// �ֲ���           
		p->dPreSettlementPrice,	// ������ 
		p->dLastVolume,// ��ǰ��               

		p->dBuyprice2,	// �����               
		p->dBuyprice3,	// ������               
		p->dBuyprice4,	// ���ļ�               
		p->dBuyprice5,	// �����               
		p->dBuyvol2,	// �����               
		p->dBuyvol3,	// ������               
		p->dBuyvol4,	// ������               
		p->dBuyvol5,	// ������               
		p->dSellprice2,	// ������           
		p->dSellprice3,	// ������           
		p->dSellprice4,	// ���ļ�           
		p->dSellprice5,	// �����           
		p->dSellvol2,	// ������               
		p->dSellvol3,	// ������               
		p->dSellvol4,	// ������               
		p->dSellvol5,	// ������     

		p->dUpB,		// ��ͣ��               
		p->dLowB,		// ��ͣ��               
		p->dOpen,		// ����                 
		p->dHigh,		// �������             
		p->dLow,		// �������             
		p->dYclose,		// ����                 
		p->dPreOpenInterest,	// ��ֲ�       
		p->dAvgPrice,	// ����

		// 20131015�����������ֶ�
		p->dAmount,		// �ۼƳɽ����
		cur_amount,		// ��ǰ�ɽ����

		p->dChg, //�ǵ�
		p->dChgPct, //�ǵ���
		p->dPriceGap, //����
		InvalidToZeroD(p->dSettlementPrice), //���ν����
		trade_day.c_str(), //��������

		//���������
		//������
		0.0, //������
		0, //�ɽ�����
		0.0, //����ʵ��
		0.0, //����ʵ��

		//��Լ�ڽ������Ĵ���
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

	return whole_quote_str;
}
