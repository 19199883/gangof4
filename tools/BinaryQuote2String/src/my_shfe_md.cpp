#include "my_shfe_md.h"
#include "shfe_ex.h"
#include <string>

#include <sstream>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "util.h"

using namespace std;

extern int g_use_timestamp_flag;

std::string MY_SHFE_MD_QuoteToString(const SaveData_MY_SHFE_MD * const p_data )
{
	if (!p_data)
	{
		return "";
	}

	const SaveData_SHFE_EX * const p = (const SaveData_SHFE_EX * const)p_data;	
	int total_volume = 0;
	if (p_data->data_.data_flag != 2 &&
		p_data->data_.data_flag != 5)
	{
		total_volume = p->data_.Volume;
	}
	stringstream ss;
	ss << std::fixed << std::setprecision(4);

	ss << p_data->data_.InstrumentID << " - data_flag|" << p_data->data_.data_flag 
		<< "|" << p->t_ << "|" << total_volume << std::endl;
	if (p_data->data_.data_flag == 1 || 
		p_data->data_.data_flag == 3 || 
		p_data->data_.data_flag == 6)
	{
		std::string ex_str = SHFE_EX_ToString(p);
		ss << ex_str;
	}

	if (p_data->data_.data_flag == 3||
		p_data->data_.data_flag == 6)
	{
		ss << std::endl;
	}

	if (p_data->data_.data_flag == 2 || 
		p_data->data_.data_flag == 3 || 
		p_data->data_.data_flag == 5 || 
		p_data->data_.data_flag == 6)
	{
		ss <<p_data->data_.InstrumentID << " - stats|" << p_data->data_.buy_total_volume << "|" << p_data->data_.buy_weighted_avg_price << "|" 
			<< p_data->data_.sell_total_volume << "|" << p_data->data_.sell_weighted_avg_price << "|"
			<< std::endl;
		for (int i = 0; i < p_data->data_.data_count; ++i)
		{
			ss << p_data->data_.InstrumentID << "|0|" 
				<< p_data->data_.buy_price[i] << "|" 
				<< p_data->data_.buy_volume[i] << "|" 
				<< std::endl;
		}
		for (int i = 0; i < p_data->data_.data_count; ++i)
		{
			ss << p_data->data_.InstrumentID << "|1|" 
				<< p_data->data_.sell_price[i] << "|" 
				<< p_data->data_.sell_volume[i] << "|" 
				<< std::endl;
		}
	}

	return ss.str();
}
