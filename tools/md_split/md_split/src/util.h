#ifndef UTIL_H_
#define UTIL_H_

#include "../md_include/my_quote_save.h"

inline int GetSymbolOffset(int data_type, int data_len)
{
	int offset_in_data = -1;
	switch (data_type)
	{
	case MY_SHFE_MD_QUOTE_TYPE:
	{
		MYShfeMarketData md;
		offset_in_data = (int)((char *)&(md.InstrumentID) - (char *)&md);
	}
	break;
	case DCE_MDBESTANDDEEP_QUOTE_TYPE:
	{
		MDBestAndDeep_MY md;
		offset_in_data = (int)((char *)&(md.Contract) - (char *)&md);
	}
	break;
	case DCE_MDORDERSTATISTIC_QUOTE_TYPE:
	{
		MDOrderStatistic_MY md;
		offset_in_data = (int)((char *)&(md.ContractID) - (char *)&md);
	}
	break;
	// added by wangying on 20161109 for czce level2 market data
	case CZCE_LEVEL2_QUOTE_TYPE:
	{
		ZCEL2QuotSnapshotField_MY md;
		offset_in_data = (int)((char *)&(md.ContractID) - (char *)&md);
	}
	break;
	// add by wangying on 2017-08-21
	case GTA_UDP_CFFEX_QUOTE_TYPE:
	{
		CFfexFtdcDepthMarketData md; 
		offset_in_data = (int)((char *)&(md.szInstrumentID) - (char *)&md);
	}
	break;

	case SHFE_EX_QUOTE_TYPE:
	{
		CDepthMarketDataField md;
		offset_in_data = (int)((char *)&(md.InstrumentID) - (char *)&md);
	}
	break;
	// added by wangying on 2019-1-25
	case DEPTHMARKETDATA_QUOTE_TYPE:
	{
		depthMarketData md;
		offset_in_data = (int)((char *)&(md.name) - (char *)&md);
	}
	break;
	case REALTIMEDATA_QUOTE_TYPE:
	{
		realTimeData md;
		offset_in_data = (int)((char *)&(md.name) - (char *)&md);
	}
	break;
	case ORDERBOOKDATA_QUOTE_TYPE             :
	{
		orderbookData md;
		offset_in_data = (int)((char *)&(md.name) - (char *)&md);
	}
	break;
	case TRADEVOLUMEDATA_QUOTE_TYPE :
	{
		tradeVolume md;
		offset_in_data = (int)((char *)&(md.name) - (char *)&md);
	}
	break;


	default:
		break;
	}

	return 8 + offset_in_data;
}

template<typename DataType>
struct SaveDataStruct
{
	long long t_;                  // 时间戳 (nano seconds from epoch)
	DataType data_;                // 数据对象 （8字节对齐）

	// 缺省构造
	SaveDataStruct()
	{
		t_ = 0;
	}

	// 通过时间戳、和网络数据包构造
	SaveDataStruct(long long t, const DataType &d)
		: t_(t), data_(d)
	{
	}
};

inline void UTIL_SaveDataToFile(void *data, int data_len, int &data_count, FILE * pf)
{
	if (pf && data)
	{
		fwrite(data, data_len, 1, pf);
		++data_count;
		fseek(pf, 0, SEEK_SET);
		fwrite(&data_count, sizeof(data_count), 1, pf);
		fseek(pf, 0, SEEK_END);
	}
}

template<typename DataType>
void UTIL_SaveDataToFile(DataType *data, int &data_count, FILE * pf)
{
	if (pf && data)
	{
		fwrite(data, sizeof(DataType), 1, pf);
		++data_count;
		fseek(pf, 0, SEEK_SET);
		fwrite(&data_count, sizeof(data_count), 1, pf);
		fseek(pf, 0, SEEK_END);
	}
}

template<typename DataType>
void UTIL_SaveFileHeader(int data_type, FILE * pf)
{
	if (pf)
	{
		SaveFileHeaderStruct header;
		header.data_count = 0;
		header.data_type = short(data_type);
		header.data_length = (short)(sizeof(DataType));
		fwrite(&header, sizeof(SaveFileHeaderStruct), 1, pf);
	}
}

class test_progress_display
{
public:
	explicit test_progress_display(unsigned long expected_count_,
		std::ostream & os = std::cout,
		const std::string & s1 = "\n", //leading strings
		const std::string & s2 = "",
		const std::string & s3 = "")
		// os is hint; implementation may ignore, particularly in embedded systems
		: m_os(os), m_s1(s1), m_s2(s2), m_s3(s3) {
		restart(expected_count_);
	}

	void           restart(unsigned long expected_count_)
		//  Effects: display appropriate scale
		//  Postconditions: count()==0, expected_count()==expected_count_
	{
		_count = _next_tic_count = _tic = 0;
		_expected_count = expected_count_;

		m_os << m_s1 << "0%   10   20   30   40   50   60   70   80   90   100%\n"
			<< m_s2 << "|----|----|----|----|----|----|----|----|----|----|"
			<< std::endl  // endl implies flush, which ensures display
			<< m_s3;
		if (!_expected_count) _expected_count = 1;  // prevent divide by zero
	} // restart

	unsigned long  operator+=(unsigned long increment)
		//  Effects: Display appropriate progress tic if needed.
		//  Postconditions: count()== original count() + increment
		//  Returns: count().
	{
		if ((_count += increment) >= _next_tic_count) { display_tic(); }
		return _count;
	}

	unsigned long  operator++() { return operator+=(1); }
	unsigned long  count() const { return _count; }
	unsigned long  expected_count() const { return _expected_count; }

private:
	std::ostream &     m_os;  // may not be present in all imps
	const std::string  m_s1;  // string is more general, safer than 
	const std::string  m_s2;  //  const char *, and efficiency or size are
	const std::string  m_s3;  //  not issues

	unsigned long _count, _expected_count, _next_tic_count;
	unsigned int  _tic;
	void display_tic()
	{
		// use of floating point ensures that both large and small counts
		// work correctly.  static_cast<>() is also used several places
		// to suppress spurious compiler warnings. 
		unsigned int tics_needed = static_cast<unsigned int>((static_cast<double>(_count)
			/ static_cast<double>(_expected_count)) * 50.0);
		do { m_os << '*' << std::flush; } while (++_tic < tics_needed);
		_next_tic_count =
			static_cast<unsigned long>((_tic / 50.0) * static_cast<double>(_expected_count));
		if (_count == _expected_count) {
			if (_tic < 51) m_os << '*';
			m_os << std::endl;
		}
	} // display_tic
};

#endif // UTIL_H_
