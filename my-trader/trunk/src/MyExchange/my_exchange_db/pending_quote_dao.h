#pragma once

#include "../my_exchange.h"
#include <unordered_map>
#include <string>
#include <map>
#include <list>
#include "quote_entity.h"
#include <sstream>
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>
#include <ctime>
#include <string>
#include "exchange_names.h"
#include "security_type.h"
#include "quote_entity.h"
#include "tbb/include/tbb/spin_mutex.h"

#include "quote_datatype_stock_tdf.h"
//#include "my_marketdata.h"
#include "quote_datatype_cffex_level2.h"
#include "quote_datatype_level1.h"
#include "quote_datatype_shfe_my.h"
#include "quote_datatype_shfe_deep.h"
#include "quote_datatype_level1.h"
#include "quote_datatype_dce_level2.h"
#include "quote_datatype_czce_level2.h"
#include "quote_datatype_sec_kmds.h"
#include "quote_datatype_datasource.h"
#include "my_trade_tunnel_struct.h"

#include "quotecategoryoptions.h"
#include "tbb/include/tbb/spin_mutex.h"
#include <ctime>
#include <ratio>
#include <chrono>
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable> 

#define HASH_MAP_SIZE 128

using namespace std::chrono;
using namespace tbb;
using namespace std;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

// 订阅的合约，集合类型
typedef std::set<std::string> SubscribeContracts;
typedef map<long,bool> QuoteStateT;

inline int get_hash_index(const char* p, size_t len)
{
	int index = 0;
    if(len > 3){
    	index = *((int *)p);
    	p = p + 4;
    }
    if(*p != 0)
        index += ((*p++) ^ index);
    if(*p != 0)
        index += ((*p++) ^ index);
    if(*p != 0)
        index += ((*p++) ^ index);
	while (*p){
		index += ((*p++) ^ index);
	}

	return index &= (HASH_MAP_SIZE - 1);
}

namespace quote_agent
{
	struct module_node{
		long module_id;
		module_node * next;
	};

	struct contract_node{
		char contract[64];
		module_node* module;
		contract_node  *next;
	};

	/**
	pending_quote_daoàà·a×°á???pending_quote_table±í?áD′·??ê￡?è?￡o2é?ˉ???¨?￡Dí?′?áμ?DD?éêy?Y
	*/
	template<typename QuoteT>
	class pending_quote_dao
	{
	public:
		typedef long QuoteTableKeyT;
		typedef vector<QuoteT> QuoteTableRecordT;
		typedef map<QuoteTableKeyT, QuoteTableRecordT> QuoteTableT;
		typedef contract_node ContractNode;
		typedef module_node ModuleNode;
	private:
		static speculative_spin_mutex lock_ro_quote;
		
		/*
		 * the following fields cache SPIF,CF,stock,full depth quote
		 * key:model id
		 */
		static QuoteTableT quote_cache;

		/*
		 * value:record count
		 */
		static map<QuoteTableKeyT,int> cache_record_count;

		/**
		±ê?????¨μ??????aò??á
		*/
		static void mark_read(const long & model_id);

	public:

		/**
		??·?·¨ó?óú?????????￡Dí????á???D?o???μ?DD?é.
		key: ?￡DíID
		value: subscription info
		*/
		static map<long, SubscribeContracts > quote_subscribtion;

		//key contract, vector model id
		static unordered_map<string, set<long> > contract_model_map;
        static set<long> all_model;
    
        static ContractNode** contract_model_hash_map;

		/*
		 * key: quote ID
		 * value:timestamp
		 */
		static map <string,std::chrono::high_resolution_clock::time_point> timestamp_cache;

		/*****************************
		 * get symbol
		 ************************************/
		static string get_symbol(const QuoteT *quote)
		{
			throw invalid_argument("Need to implement get_symbol method.");
		}

		/*******************************
		 * get update time of quote
		 */
		static string get_quote_time(const QuoteT *quote)
		{
			return "";
		}

		static void set_quote_time(const QuoteT *quote,char *time)
		{
		}

		/*****************************
		 * get quote ID.
		 */
		static string get_id(const QuoteT *quote)
		{
			return "";
		}

		/*************************************
		 * get quote local timestamp
		 *************************************/
		static std::chrono::high_resolution_clock::time_point get_local_timestamp(const QuoteT *quote)
		{
			string id = pending_quote_dao<QuoteT>::get_id(*quote);
			return timestamp_cache[id];
		}

		/****************************************
		 * set local time stamp at the current time for the given quote
		 *****************************************/
		static void set_local_timestamp(const QuoteT *quote)
		{
		    string id = pending_quote_dao<QuoteT>::get_id(quote);
		    timestamp_cache[id] = high_resolution_clock::now();
		}

		pending_quote_dao(void)
		{			
		}

		~pending_quote_dao(void)
		{
			// TO IMPROVE: Dèòa??qa11?¨?aμ￥ày?￡ê?￡??a?ù￡?quote_cache?í2?Dèòa′ó??·???????
		}

		static void init(const long & model_id);
		/**
		2?è?DD?éêy?Y
		*/
		static void insert_quote(const long &model_id,const QuoteT *quote);

		/**
		2é?ˉ???¨?￡Díμ??ùóD?′?áμ?DD?é￡?2￠°′ê±??éyDò??áD
		*/
		static void query(const long & model_id_str,int &count,
				typename pending_quote_dao<QuoteT>::QuoteTableRecordT &new_cache);
	};



	/*****************************************
	 * MDBestAndDeep_MY.DCE exchange cf quote
	 */
	template<>
	string pending_quote_dao<MDBestAndDeep_MY>::get_quote_time(const MDBestAndDeep_MY *quote)
	{
		return quote->GenTime;
	}

	template<>
	void pending_quote_dao<MDBestAndDeep_MY>
	::set_quote_time(const MDBestAndDeep_MY *quote,char *time)
	{
	    strcpy(const_cast<char*>(quote->GenTime),time);
	}

	template<>
	string pending_quote_dao<MDBestAndDeep_MY>::get_symbol(const MDBestAndDeep_MY *quote)
	{
		return quote->Contract;
	}

	/*****************************************
	 * MDArbi_MY.DCE exchange cf quote
	 */
	template<>
	string pending_quote_dao<MDArbi_MY>::get_quote_time(const MDArbi_MY *quote)
	{
		return quote->GenTime;
	}

	template<>
	void pending_quote_dao<MDArbi_MY>
	::set_quote_time(const MDArbi_MY *quote,char *time)
	{
	    strcpy(const_cast<char*>(quote->GenTime),time);
	}

	template<>
	string pending_quote_dao<MDArbi_MY>::get_symbol(const MDArbi_MY *quote)
	{
		return quote->Contract;
	}

	template<>
	string pending_quote_dao<MDArbi_MY>::get_id(const MDArbi_MY *quote)
	{
	    char key[93];
	    strcpy(key,quote->Contract);
	    strcpy(key+strlen(quote->Contract),quote->GenTime);
	    return key;
	}

	/*****************************************
	 * MDTenEntrust_MY.DCE exchange cf quote
	 */
	template<>
	string pending_quote_dao<MDTenEntrust_MY>::get_quote_time(const MDTenEntrust_MY *quote)
	{
		return quote->GenTime;
	}

	template<>
	void pending_quote_dao<MDTenEntrust_MY>
	::set_quote_time(const MDTenEntrust_MY *quote,char *time)
	{
		strcpy(const_cast<char*>(quote->GenTime),time);
	}

	template<>
	string pending_quote_dao<MDTenEntrust_MY>::get_symbol(const MDTenEntrust_MY *quote)
	{
		return quote->Contract;
	}

	template<>
	string pending_quote_dao<MDTenEntrust_MY>::get_id(const MDTenEntrust_MY *quote)
	{
	    char key[93];
	    strcpy(key,quote->Contract);
	    strcpy(key+strlen(quote->Contract),quote->GenTime);
	    return key;
	}

	/*****************************************
	 * MDOrderStatistic_MY.DCE exchange cf quote
	 */
	template<>
	string pending_quote_dao<MDOrderStatistic_MY>::get_quote_time(const MDOrderStatistic_MY *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<MDOrderStatistic_MY>
	::set_quote_time(const MDOrderStatistic_MY *quote,char *time)
	{
		//strcpy(quote->GenTime,time);
	}

	template<>
	string pending_quote_dao<MDOrderStatistic_MY>::get_symbol(const MDOrderStatistic_MY *quote)
	{
		return quote->ContractID;
	}

	template<>
	string pending_quote_dao<MDOrderStatistic_MY>::get_id(const MDOrderStatistic_MY *quote)
	{
	    char key[93];
	    strcpy(key,quote->ContractID);
	    strcpy(key+strlen(quote->ContractID),"");
	    return key;
	}

	/*****************************************
	 * MDRealTimePrice_MY.DCE exchange cf quote
	 */
	template<>
	string pending_quote_dao<MDRealTimePrice_MY>::get_quote_time(const MDRealTimePrice_MY *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<MDRealTimePrice_MY>
	::set_quote_time(const MDRealTimePrice_MY *quote,char *time)
	{
		//strcpy(quote->GenTime,time);
	}

	template<>
	string pending_quote_dao<MDRealTimePrice_MY>::get_symbol(const MDRealTimePrice_MY *quote)
	{
		return quote->ContractID;
	}

	template<>
	string pending_quote_dao<MDRealTimePrice_MY>::get_id(const MDRealTimePrice_MY *quote)
	{
	    char key[93];
	    strcpy(key,quote->ContractID);
	    strcpy(key+strlen(quote->ContractID),"");
	    return key;
	}

	/*****************************************
	 * MDMarchPriceQty_MY.DCE exchange cf quote
	 */
	template<>
	string pending_quote_dao<MDMarchPriceQty_MY>::get_quote_time(const MDMarchPriceQty_MY *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<MDMarchPriceQty_MY>
	::set_quote_time(const MDMarchPriceQty_MY *quote,char *time)
	{
		//strcpy(quote->GenTime,time);
	}

	template<>
	string pending_quote_dao<MDMarchPriceQty_MY>::get_symbol(const MDMarchPriceQty_MY *quote)
	{
		return quote->ContractID;
	}

	template<>
	string pending_quote_dao<MDMarchPriceQty_MY>::get_id(const MDMarchPriceQty_MY *quote)
	{
	    char key[93];
	    strcpy(key,quote->ContractID);
	    strcpy(key+strlen(quote->ContractID),"");
	    return key;
	}



	/*****************************************
	 * CShfeFtdcMBLMarketDataField. full depth quote->
	 */
	template<>
	string pending_quote_dao<SHFEQuote>::get_quote_time(const SHFEQuote *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<SHFEQuote>
	::set_quote_time(const SHFEQuote *quote,char *time)
	{

	}

	template<>
	string pending_quote_dao<SHFEQuote>::get_symbol(const SHFEQuote *quote)
	{
		return quote->field.InstrumentID;
	}

	template<>
	string pending_quote_dao<SHFEQuote>::get_id(const SHFEQuote *quote)
	{
	    char key[40];
	    strcpy(key,quote->field.InstrumentID);
	    strcpy(key+strlen(quote->field.InstrumentID),"");
	    return key;
	}


	/*****************************************
	 * MDBestAndDeep_MY.ctp quote, cf.
	 */
	/*template<>
	string pending_quote_dao<CThostFtdcDepthMarketDataField>::get_quote_time(CThostFtdcDepthMarketDataField *quote)
	{
		return quote->UpdateTime;
	}

	template<>
	string pending_quote_dao<CThostFtdcDepthMarketDataField>::get_symbol(CThostFtdcDepthMarketDataField *quote)
	{
		return quote->InstrumentID;
	}

	template<>
	string pending_quote_dao<CThostFtdcDepthMarketDataField>::get_id(CThostFtdcDepthMarketDataField *quote)
	{
		char key[40];
		strcpy(key,quote->InstrumentID);
		strcpy(key+strlen(quote->InstrumentID),quote->UpdateTime);
		return key;
	}
*/
	/*****************************************
	 * quote_datatype_gta_udp, i.e.CFfexFtdcDepthMarketData. spif.
	 */
	template<>
	string pending_quote_dao<CFfexFtdcDepthMarketData>::get_quote_time(const CFfexFtdcDepthMarketData *quote)
	{
		return quote->szUpdateTime;
	}

	template<>
	void pending_quote_dao<CFfexFtdcDepthMarketData>
	::set_quote_time(const CFfexFtdcDepthMarketData *quote,char *time)
	{
		strcpy(const_cast<char*>(quote->szUpdateTime),time);
	}

	template<>
	string pending_quote_dao<CFfexFtdcDepthMarketData>::get_symbol(const CFfexFtdcDepthMarketData *quote)
	{
		return quote->szInstrumentID;
	}

	template<>
	string pending_quote_dao<CFfexFtdcDepthMarketData>::get_id(const CFfexFtdcDepthMarketData *quote)
	{
	    char key[40];
	    strcpy(key,quote->szInstrumentID);
	    strcpy(key+strlen(quote->szInstrumentID),quote->szUpdateTime);
	    return key;
	}

	/*****************************************
	* MYShfeMarketData
	*/
	template<>
	string pending_quote_dao<MYShfeMarketData>::get_quote_time(const MYShfeMarketData *quote)
	{
		return quote->UpdateTime;
	}

	template<>
	void pending_quote_dao<MYShfeMarketData>
	::set_quote_time(const MYShfeMarketData *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<MYShfeMarketData>::get_symbol(const MYShfeMarketData *quote)
	{
		return quote->InstrumentID;
	}

	template<>
	string pending_quote_dao<MYShfeMarketData>::get_id(const MYShfeMarketData *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->InstrumentID);
	    strcpy(key+strlen(quote->InstrumentID),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* CDepthMarketDataField
	*/
	template<>
	string pending_quote_dao<CDepthMarketDataField>::get_quote_time(const CDepthMarketDataField *quote)
	{
		return quote->UpdateTime;
	}

	template<>
	void pending_quote_dao<CDepthMarketDataField>
	::set_quote_time(const CDepthMarketDataField *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<CDepthMarketDataField>::get_symbol(const CDepthMarketDataField *quote)
	{
		return quote->InstrumentID;
	}

	template<>
	string pending_quote_dao<CDepthMarketDataField>::get_id(const CDepthMarketDataField *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->InstrumentID);
	    strcpy(key+strlen(quote->InstrumentID),get_quote_time(quote).c_str());
	    return key;
	}


	/*****************************************
	* ZCEQuotCMBQuotField_MY
	*/
	template<>
	string pending_quote_dao<ZCEQuotCMBQuotField_MY>::get_quote_time(const ZCEQuotCMBQuotField_MY *quote)
	{
		return quote->TimeStamp;
	}

	template<>
	void pending_quote_dao<ZCEQuotCMBQuotField_MY>
	::set_quote_time(const ZCEQuotCMBQuotField_MY *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<ZCEQuotCMBQuotField_MY>::get_symbol(const ZCEQuotCMBQuotField_MY *quote)
	{
		return quote->ContractID;
	}

	template<>
	string pending_quote_dao<ZCEQuotCMBQuotField_MY>::get_id(const ZCEQuotCMBQuotField_MY *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->ContractID);
	    strcpy(key+strlen(quote->ContractID),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* ZCEL2QuotSnapshotField_MY
	*/
	template<>
	string pending_quote_dao<ZCEL2QuotSnapshotField_MY>::get_quote_time(const ZCEL2QuotSnapshotField_MY *quote)
	{
		return quote->ContractID;
	}

	template<>
	void pending_quote_dao<ZCEL2QuotSnapshotField_MY>
	::set_quote_time(const ZCEL2QuotSnapshotField_MY *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<ZCEL2QuotSnapshotField_MY>::get_symbol(const ZCEL2QuotSnapshotField_MY *quote)
	{
		return quote->ContractID;
	}

	template<>
	string pending_quote_dao<ZCEL2QuotSnapshotField_MY>::get_id(const ZCEL2QuotSnapshotField_MY *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->ContractID);
	    strcpy(key+strlen(quote->ContractID),get_quote_time(quote).c_str());
	    return key;
	}


	/*****************************************
	* TDF_MARKET_DATA_MY
	*/
	template<>
	string pending_quote_dao<TDF_MARKET_DATA_MY>::get_quote_time(const TDF_MARKET_DATA_MY *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<TDF_MARKET_DATA_MY>
	::set_quote_time(const TDF_MARKET_DATA_MY *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<TDF_MARKET_DATA_MY>::get_symbol(const TDF_MARKET_DATA_MY *quote)
	{
		return quote->szCode;
	}

	template<>
	string pending_quote_dao<TDF_MARKET_DATA_MY>::get_id(const TDF_MARKET_DATA_MY *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->szCode);
	    strcpy(key+strlen(quote->szCode),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* TDF_MARKET_DATA_MY
	*/
	template<>
	string pending_quote_dao<TDF_INDEX_DATA_MY>::get_quote_time(const TDF_INDEX_DATA_MY *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<TDF_INDEX_DATA_MY>
	::set_quote_time(const TDF_INDEX_DATA_MY *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<TDF_INDEX_DATA_MY>::get_symbol(const TDF_INDEX_DATA_MY *quote)
	{
		return quote->szCode;
	}

	template<>
	string pending_quote_dao<TDF_INDEX_DATA_MY>::get_id(const TDF_INDEX_DATA_MY *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->szCode);
	    strcpy(key+strlen(quote->szCode),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* T_OptionQuote
	*/
	template<>
	string pending_quote_dao<T_OptionQuote>::get_quote_time(const T_OptionQuote *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<T_OptionQuote>
	::set_quote_time(const T_OptionQuote *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<T_OptionQuote>::get_symbol(const T_OptionQuote *quote)
	{
		return quote->contract_code;
	}

	template<>
	string pending_quote_dao<T_OptionQuote>::get_id(const T_OptionQuote *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->contract_code);
	    strcpy(key+strlen(quote->contract_code),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* T_OrderQueue
	*/
	template<>
	string pending_quote_dao<T_OrderQueue>::get_quote_time(const T_OrderQueue *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<T_OrderQueue>
	::set_quote_time(const T_OrderQueue *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<T_OrderQueue>::get_symbol(const T_OrderQueue *quote)
	{
		return quote->scr_code;
	}

	template<>
	string pending_quote_dao<T_OrderQueue>::get_id(const T_OrderQueue *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->scr_code);
	    strcpy(key+strlen(quote->scr_code),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* T_PerEntrust
	*/
	template<>
	string pending_quote_dao<T_PerEntrust>::get_quote_time(const T_PerEntrust *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<T_PerEntrust>
	::set_quote_time(const T_PerEntrust *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<T_PerEntrust>::get_symbol(const T_PerEntrust *quote)
	{
		return quote->scr_code;
	}

	template<>
	string pending_quote_dao<T_PerEntrust>::get_id(const T_PerEntrust *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->scr_code);
	    strcpy(key+strlen(quote->scr_code),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* T_PerBargain
	*/
	template<>
	string pending_quote_dao<T_PerBargain>::get_quote_time(const T_PerBargain *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<T_PerBargain>
	::set_quote_time(const T_PerBargain *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<T_PerBargain>::get_symbol(const T_PerBargain *quote)
	{
		return quote->scr_code;
	}

	template<>
	string pending_quote_dao<T_PerBargain>::get_id(const T_PerBargain *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->scr_code);
	    strcpy(key+strlen(quote->scr_code),get_quote_time(quote).c_str());
	    return key;
	}


	/*****************************************
	* TapAPIQuoteWhole_MY
	*/
	template<>
	string pending_quote_dao<TapAPIQuoteWhole_MY>::get_quote_time(const TapAPIQuoteWhole_MY *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<TapAPIQuoteWhole_MY>
	::set_quote_time(const TapAPIQuoteWhole_MY *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<TapAPIQuoteWhole_MY>::get_symbol(const TapAPIQuoteWhole_MY *quote)
	{
		return quote->ContractNo1;
	}

	template<>
	string pending_quote_dao<TapAPIQuoteWhole_MY>::get_id(const TapAPIQuoteWhole_MY *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->ContractNo1);
	    strcpy(key+strlen(quote->ContractNo1),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* my_datasource_data change to inner_quote format
	*/
	template<>
	string pending_quote_dao<inner_quote_fmt>::get_quote_time(const inner_quote_fmt *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<inner_quote_fmt>
	::set_quote_time(const inner_quote_fmt *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<inner_quote_fmt>::get_symbol(const inner_quote_fmt *quote)
	{
		return quote->inner_symbol;
	}

	template<>
	string pending_quote_dao<inner_quote_fmt>::get_id(const inner_quote_fmt *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->inner_symbol);
	    strcpy(key+strlen(quote->inner_symbol),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* IBDepth
	*/
	template<>
	string pending_quote_dao<IBDepth>::get_quote_time(const IBDepth *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<IBDepth>
	::set_quote_time(const IBDepth *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<IBDepth>::get_symbol(const IBDepth *quote)
	{
		return quote->Symbol;
	}

	template<>
	string pending_quote_dao<IBDepth>::get_id(const IBDepth *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->Symbol);
	    strcpy(key+strlen(quote->Symbol),get_quote_time(quote).c_str());
	    return key;
	}

	/*****************************************
	* IBTick
	*/
	template<>
	string pending_quote_dao<IBTick>::get_quote_time(const IBTick *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<IBTick>
	::set_quote_time(const IBTick *quote,char *time)
	{
//		if(true==strcmp(time, "finish")){
//			quote->time = -1;
//		}
	}

	template<>
	string pending_quote_dao<IBTick>::get_symbol(const IBTick *quote)
	{
		return quote->Symbol;
	}

	template<>
	string pending_quote_dao<IBTick>::get_id(const IBTick *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->Symbol);
	    strcpy(key+strlen(quote->Symbol),get_quote_time(quote).c_str());
	    return key;
	}

	/*
	 * SGIT_Depth_Market_Data_Field
	 */
	template<>
	string pending_quote_dao<SGIT_Depth_Market_Data_Field>::get_quote_time(const SGIT_Depth_Market_Data_Field *quote)
	{
		return "";
	}

	template<>
	void pending_quote_dao<SGIT_Depth_Market_Data_Field>
	::set_quote_time(const SGIT_Depth_Market_Data_Field *quote,char *time)
	{
	//		if(true==strcmp(time, "finish")){
	//			quote->time = -1;
	//		}
	}

	template<>
	string pending_quote_dao<SGIT_Depth_Market_Data_Field>::get_symbol(const SGIT_Depth_Market_Data_Field *quote)
	{
		return quote->InstrumentID;
	}

	template<>
	string pending_quote_dao<SGIT_Depth_Market_Data_Field>::get_id(const SGIT_Depth_Market_Data_Field *quote)
	{
	    char key[140];
	    memset(key,0x00,140);
	    strcpy(key,quote->InstrumentID);
	    strcpy(key+strlen(quote->InstrumentID),get_quote_time(quote).c_str());
	    return key;
	}
}


using namespace quote_agent;
#include "pending_quote_daodef.h"

