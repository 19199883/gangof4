#include "cme.h"
#include <string>
#include "util.h"

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

std::string CME_QuoteToString(const SaveData_depthMarketData * const p_data)
{
	if (!p_data)
	{
		return "";
	}

	const depthMarketData * const p = &(p_data->data_);	
	
	
	return "";
}

std::string CME_QuoteToString(const SaveData_realTimeData * const p_data)
{
	if (!p_data)
	{
		return "";
	}

	const realTimeData * const p = &(p_data->data_);	
	
	
	return "";
}

std::string CME_QuoteToString(const SaveData_orderbookData * const p_data)
{
	if (!p_data)
	{
		return "";
	}

	const orderbookData * const p = &(p_data->data_);	
	
	
	return "";
}

std::string CME_QuoteToString(const SaveData_tradeVolume * const p_data)
{
	if (!p_data)
	{
		return "";
	}

	const tradeVolume * const p = &(p_data->data_);	
	
	
	return "";
}