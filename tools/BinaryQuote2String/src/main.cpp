#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/progress.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>


#include "util.h"

#include "gtaex.h"
#include "ctp.h"
#include "dce.h"
#include "shfe_ex.h"
#include "shfe_deep.h"
#include "gta_udp.h"
#include "my_shfe_md.h"
#include "tdf_stock.h"
#include "czce_l2.h"
#include "ksg.h"
#include "taifex_md.h"
#include "kmds.h"
#include "cme.h"
#include "yao_quote.h"
#include "lev2_quote.h"

#include <iostream>
#include <chrono>


using namespace std;

void Pause() 
{
	std::cout << "input quit to quit" << std::endl;
	std::string value;
	while (value != "quit")
	{
		std::cin >> value;
		boost::this_thread::sleep(boost::posix_time::millisec(100));
	}
}

template<typename DataType>
void MYUTIL_SaveDataToFile(DataType *data, int &data_count, FILE * pf)
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
void MYUTIL_SaveFileHeader(int data_type, FILE * pf)
{
	if (pf)
	{
		SaveFileHeaderStruct header;
		header.data_count = 0;
		header.data_type = short(data_type);
		header.data_length = (short) (sizeof(DataType));
		fwrite(&header, sizeof(SaveFileHeaderStruct), 1, pf);
	}
}

int g_use_timestamp_flag = 1;
int main(int argc, char **argv)
{	
	std::cout << "Usage: BinaryQuote2String BinaryQuoteDataFile [use_timestamp_flag(1)]" << std::endl;
	if (argc < 2)
	{
		std::cout << "miss parameters" << std::endl;
		Pause();
	}
	g_use_timestamp_flag = 1;
	if (argc > 2)
	{
		g_use_timestamp_flag = atoi(argv[2]);
	}

	std::string file_name = argv[1];
	std::size_t real_count = 0;
	std::size_t calc_count = 0;

	struct _stat64 info;
	_stat64(file_name.c_str(), &info);
	long long file_size = info.st_size;
	long long handled_size = 0;

	ifstream f_in(file_name.c_str(), std::ios_base::binary);
	SaveFileHeaderStruct f_header;
	f_header.data_count = 0;
	f_header.data_length = 0;
	f_header.data_type = 0;

	f_in.read((char *)&f_header, sizeof(f_header));

	// 错误处理
	if (!f_in)
	{
		std::cout << file_name << " open failed. " << std::endl;
		//system("pause");
		return 1;
	}

	// 错误处理
	if (f_header.data_length == 0)
	{
		std::cout << file_name << " read header failed. " << std::endl;
		//system("pause");
		return 1;
	}

	calc_count = (std::size_t)((file_size - sizeof(f_header))/f_header.data_length);
	
	// 错误处理
	if (file_size != (calc_count * f_header.data_length + sizeof(f_header))
		&& f_header.data_count == 0)
	{
		std::cout << "WARNING: there are incomplete data item in " << file_name  << std::endl;
		//system("pause");
	}

	if (f_header.data_count == 0 || f_header.data_count == calc_count)
	{
		std::cout << file_name << " total count is: " << calc_count << std::endl;
	}
	else
	{
		std::cout << file_name << " f_header.data_count is: " << f_header.data_count 
			<< "; calc_count is: " << calc_count << std::endl;
	}

	// output file
	std::string quote_str_file = file_name;
	boost::replace_last(quote_str_file, ".dat", ".txt");
	ofstream f_out(quote_str_file.c_str(), std::ios_base::out | std::ios_base::app);

	std::map<std::string, ofstream *> f_out_of_sec;
	std::map<std::string, FILE *> f_out_of_binary;
	std::map<std::string, int> data_count_binary;
	std::string out_header("tmv,tma,btv,bav,stv,sav,bv1,bp1,bv2,vp2,bv3,bp3,bv4,bp4,bv5,bp5,bv6,bp6,bv7,bp7,bv8,bp8,bv9,bp9,bv10,bp10,"
		"sv1,sp1,sv2,vp2,sv3,sp3,sv4,sp4,sv5,sp5,sv6,sp6,sv7,sp7,sv8,sp8,sv9,sp9,sv10,sp10");

	// 修改头中的数据条数field
	if (f_header.data_count == 0)
	{
		FILE *f_mod;
		f_mod = fopen(file_name.c_str(), "rb+");
		if (f_mod)
		{
			std::cout <<  ftell(f_mod) << std::endl;
			fseek(f_mod, 0, SEEK_SET);
			std::cout <<  ftell(f_mod) << std::endl;
			rewind(f_mod);
			std::cout <<  ftell(f_mod) << std::endl;
			fwrite(&calc_count, sizeof(calc_count), 1, f_mod);
			fclose(f_mod);

			std::cout << "WARNING: data count field correct success. " << std::endl;
		}
		else
		{
			std::cout << "WARNING: data count field correct failed. " << std::endl;
		}

		f_header.data_count = calc_count;
	}
	
	// 进度显示
	std::cout << "begin convert, waiting..." << std::endl;

	boost::progress_display pd(f_header.data_count);
	//pd += sizeof(f_header);
	while (true)
	{
		for (int i = 0; i < f_header.data_count; ++i)
		{
			switch (f_header.data_type)
			{
			case GTAEX_CFFEX_QUOTE_TYPE:
				{
					SaveData_GTAEX t;
					f_in.read((char *)&t, sizeof(t));
					f_out << GTAQuoteToString(&t) << std::endl;
					break;
				}
			case CTP_MARKETDATA_QUOTE_TYPE:
				{
					SaveData_CTP t;
					f_in.read((char *)&t, sizeof(t));
					f_out << CTPQuoteToString(&t) << std::endl;
					break;
				}
			case DCE_MDBESTANDDEEP_QUOTE_TYPE:
				{
					SaveData_MDBestAndDeep t;
					f_in.read((char *)&t, sizeof(t));
					f_out << BestAndDeepToString(&t) << std::endl;
					break;
				}
			case DCE_ARBI_QUOTE_TYPE:
				{
					SaveData_Arbi t;
					f_in.read((char *)&t, sizeof(t));
					f_out << ArbiToString(&t) << std::endl;
					break;
				}
			case DCE_MDTENENTRUST_QUOTE_TYPE:
				{
					SaveData_MDTenEntrust t;
					f_in.read((char *)&t, sizeof(t));
					f_out << TenEntrustToString(&t) << std::endl;
					break;
				}
			case DCE_MDORDERSTATISTIC_QUOTE_TYPE:
				{
					SaveData_MDOrderStatistic t;
					f_in.read((char *)&t, sizeof(t));
					f_out << OrderStatisticToString(&t) << std::endl;
					break;
				}
			case DCE_MDREALTIMEPRICE_QUOTE_TYPE:
				{
					SaveData_MDRealTimePrice t;
					f_in.read((char *)&t, sizeof(t));
					f_out << RealTimePriceToString(&t) << std::endl;
					break;
				}
			case DCE_MDMARCHPRICEQTY_QUOTE_TYPE:
				{
					SaveData_MDMarchPriceQty t;
					f_in.read((char *)&t, sizeof(t));
					f_out << MarchPriceQtyToString(&t) << std::endl;
					break;
				}
			case SHFE_EX_QUOTE_TYPE:
				{
					SaveData_SHFE_EX t;
					f_in.read((char *)&t, sizeof(t));
					f_out << SHFE_EX_ToString(&t) << std::endl;
					break;
				}
			case SHFE_DEEP_QUOTE_TYPE:
				{
					SaveData_SHFE_DEEP t;
					f_in.read((char *)&t, sizeof(t));
					f_out << SHFE_DEEP_QuoteToString(&t) << std::endl;
					break;
				}
			case GTA_UDP_CFFEX_QUOTE_TYPE:
				{
					SaveData_GTA_UDP t;
					f_in.read((char *)&t, sizeof(t));
					f_out << GTA_UDP_QuoteToString(&t) << std::endl;
					break;
				}
			case MY_SHFE_MD_QUOTE_TYPE:
				{
					SaveData_MY_SHFE_MD t;
					f_in.read((char *)&t, sizeof(t));
					f_out << MY_SHFE_MD_QuoteToString(&t) << std::endl;
					break;
				}
			case TDF_STOCK_QUOTE_TYPE:
				{
					SaveData_TDF_MARKET_DATA t;
					f_in.read((char *)&t, sizeof(t));
					f_out << TDF_Market_Data_QuoteToString(&t) << std::endl;
					break;
				}
			case TDF_INDEX_QUOTE_TYPE:
				{
					SaveData_TDF_INDEX_DATA t;
					f_in.read((char *)&t, sizeof(t));
					f_out << TDF_Index_Data_QuoteToString(&t) << std::endl;
					break;
				}
			case CZCE_LEVEL2_QUOTE_TYPE:
				{
					SaveData_CZCE_L2_DATA t;
					f_in.read((char *)&t, sizeof(t));
					f_out << CZCE_L2_QuoteToString(&t) << std::endl;
					break;
				}
			case CZCE_CMB_QUOTE_TYPE:
				{
					SaveData_CZCE_CMB_DATA t;
					f_in.read((char *)&t, sizeof(t));
					f_out << CZCE_CMB_QuoteToString(&t) << std::endl;
					break;
				}
			case SH_GOLD_QUOTE_TYPE:
				{
					SaveData_SH_GOLD t;
					f_in.read((char *)&t, sizeof(t));
					f_out << KSG_QuoteToString(&t) << std::endl;
					break;
				}
			case TAI_FEX_MD_TYPE:
				{
					SaveData_TaiFexMD t;
					f_in.read((char *)&t, sizeof(t));
					f_out << TaiFexMD_QuoteToString(&t) << std::endl;
					break;
				}
			case KMDS_STOCK_SNAPSHOT_TYPE:
				{
					SaveData_StockQuote_KMDS t;
					f_in.read((char *)&t, sizeof(t));
					f_out << KMDS_QuoteToString(&t) << std::endl;
					break;
				}
			case KMDS_INDEX_TYPE:
				{
					SaveData_IndexQuote_KMDS t;
					f_in.read((char *)&t, sizeof(t));
					f_out << KMDS_QuoteToString(&t) << std::endl;
					break;
				}
			case KMDS_OPTION_QUOTE_TYPE:
				{
					SaveData_Option_KMDS t;
					f_in.read((char *)&t, sizeof(t));
					f_out << KMDS_QuoteToString(&t) << std::endl;
					break;
				}
			case KMDS_ORDER_QUEUE_TYPE:
				{
					SaveData_OrderQueue_KMDS t;
					f_in.read((char *)&t, sizeof(t));
					f_out << KMDS_QuoteToString(&t) << std::endl;
					break;
				}
			case KMDS_PER_ENTRUST_TYPE:
				{
					SaveData_PerEntrust_KMDS t;
					f_in.read((char *)&t, sizeof(t));
					f_out << KMDS_QuoteToString(&t) << std::endl;
					break;
				}
			case KMDS_PER_BARGAIN_TYPE:
				{
					SaveData_PerBargain_KMDS t;
					f_in.read((char *)&t, sizeof(t));
					f_out << KMDS_QuoteToString(&t) << std::endl;
					break;
				}
			case KMDS_FUTURE_QUOTE_TYPE:
				{
					SaveData_FutureQuote_KMDS t;
					f_in.read((char *)&t, sizeof(t));
					f_out << KMDS_QuoteToString(&t) << std::endl;
					break;
				}

			/////// CME
			case DEPTHMARKETDATA_QUOTE_TYPE:
				{
					SaveData_depthMarketData t;
					f_in.read((char *)&t, sizeof(t));
					f_out << CME_QuoteToString(&t) << std::endl;
					break;
				}
				case REALTIMEDATA_QUOTE_TYPE:
				{
					SaveData_realTimeData t;
					f_in.read((char *)&t, sizeof(t));
					f_out << CME_QuoteToString(&t) << std::endl;
					break;
				}
				case ORDERBOOKDATA_QUOTE_TYPE:
				{
					SaveData_orderbookData t;
					f_in.read((char *)&t, sizeof(t));
					f_out << CME_QuoteToString(&t) << std::endl;
					break;
				}
				case TRADEVOLUMEDATA_QUOTE_TYPE:
				{
					SaveData_tradeVolume t;
					f_in.read((char *)&t, sizeof(t));
					f_out << CME_QuoteToString(&t) << std::endl;
					break;
				}
				case YAO_QUOTE_TYPE:
				{
					SaveData_YaoQuote t;
					f_in.read((char *)&t, sizeof(t));
					f_out << YaoQuoteToString(&t) << std::endl;
					break;
				}
				case SaveData_Lev2Data:
				{
					SaveData_Lev2Data t;
					f_in.read((char *)&t, sizeof(t));
					f_out << Lev2QuoteToString(&t) << std::endl;
					break;
				}
			default:
				{
					std::cout << "can't handle quote data of type: " << f_header.data_type << std::endl;
					break;
				}
			}

			pd += 1;
			++real_count;
		}
		handled_size += ((long long)f_header.data_count * f_header.data_length + sizeof(f_header));

		if (handled_size + sizeof(f_header) < file_size)
		{
			f_in.read((char *)&f_header, sizeof(f_header));
			//pd += sizeof(f_header);
		}
		else
		{
			break;
		}

		if (f_header.data_count == 0) break;
	}

	f_in.close();
	f_out.close();
	std::cout << std::endl << file_name << " process success, convert count is: " << real_count << std::endl;
	//system("pause");
	return 0;
}
