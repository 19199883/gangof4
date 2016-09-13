#include "shfe_deep.h"
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

#pragma pack(push)
#pragma pack(8)

struct StructAlignTest
{
	///合约代码
	TShfeFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TShfeFtdcDirectionType	Direction;
	///价格
	TShfeFtdcPriceType	Price;
	///数量
	TShfeFtdcVolumeType	Volume;
	bool isLast;
};
#pragma pack(pop)

std::string SHFE_DEEP_QuoteToString( const SaveData_SHFE_DEEP * const p_data )
{
	if (!p_data)
	{
		return "";
	}
	//const StructAlignTest *const test_align = (const StructAlignTest *const)&(p_data->data_);
	const CShfeFtdcMBLMarketDataField * const p = &(p_data->data_.field);	
	stringstream ss;
	// ag1312|0|4225.0000|26|1384304089635957|
	if (p->InstrumentID[0] == '\0')
	{
		ss << "||||"
			<< p_data->data_.isLast << "|"
			<< p_data->t_ << "|";
	}
	else
	{
		ss << p->InstrumentID << "|"
			<< p->Direction << "|"
			<< std::fixed << std::setprecision(4) << p->Price << "|"
			<< p->Volume << "|"
			<< p_data->data_.isLast << "|"
			<< p_data->t_ << "|"<<ConvertToShortTimval(p_data->t_);

		if (p_data->data_.isLast)
		{
			ss << "\n||||"
				<< p_data->data_.isLast << "|"
				<< p_data->t_ << "|" << ConvertToShortTimval(p_data->t_);
		}
	}

	return ss.str();
}
