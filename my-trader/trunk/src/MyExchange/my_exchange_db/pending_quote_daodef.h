/******************the fields caching subscription ***************** */
template<typename QuoteT>
map<long, SubscribeContracts>
pending_quote_dao<QuoteT>::quote_subscribtion;

template<typename QuoteT>
set<long> 
pending_quote_dao<QuoteT>::all_model;

template<typename QuoteT>
map<string,high_resolution_clock::time_point>
pending_quote_dao<QuoteT>::timestamp_cache;

template<typename QuoteT>
map<typename pending_quote_dao<QuoteT>::QuoteTableKeyT,int>
pending_quote_dao<QuoteT>::cache_record_count;

template<typename QuoteT>
speculative_spin_mutex
pending_quote_dao<QuoteT>::lock_ro_quote;

/************************ the following fields caching quotes ****/
template<typename QuoteT>
typename pending_quote_dao<QuoteT>::QuoteTableT pending_quote_dao<QuoteT>::quote_cache;
/************************ the following fields caching quotes ****/
template<typename QuoteT>
typename pending_quote_dao<QuoteT>::ContractNode**
pending_quote_dao<QuoteT>::contract_model_hash_map = NULL;

template<typename QuoteT>
void
pending_quote_dao<QuoteT>::init(const long & model_id)
 {
	if (!contract_model_hash_map){
		contract_model_hash_map = (ContractNode**)malloc(sizeof(contract_node*)* HASH_MAP_SIZE);
		for (int i = 0; i < HASH_MAP_SIZE; ++i){
			contract_model_hash_map[i] = 0;
		}
	}    
    
	quote_cache[model_id] = vector<QuoteT>(50000);
	quote_cache.find(model_id)->second.reserve(50000);
	cache_record_count[model_id] = 0;


 }

/***********************************************************************************/
/*********************** insert quote **********************************************
 *
 */
template<typename QuoteT>
void
pending_quote_dao<QuoteT>::insert_quote(const long &model_id, const QuoteT *quote)
{
	int cur_count = cache_record_count[model_id];

	quote_cache.find(model_id)->second[cur_count] = *quote;
	cache_record_count[model_id] = cur_count + 1;
}

/*************************************************************************************
 * query quote
 */
template<typename QuoteT>
void
pending_quote_dao<QuoteT>::query(const long &key,int &count,
		typename pending_quote_dao<QuoteT>::QuoteTableRecordT &new_cache)
{
	typename pending_quote_dao<QuoteT>::QuoteTableRecordT &records = quote_cache.find(key)->second;
	records.swap(new_cache);
	count = cache_record_count[key];
	pending_quote_dao<QuoteT>::mark_read(key);
}


/************************************************
* mark the specified quotes in read status
*/
template<typename QuoteT>
void
pending_quote_dao<QuoteT>::mark_read(const long & key)
{
	cache_record_count[key] = 0;
}

