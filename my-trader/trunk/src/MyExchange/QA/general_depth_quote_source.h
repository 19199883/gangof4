#ifndef general_depth_quote_source_H_
#define general_depth_quote_source_H_

#include "../my_exchange.h"
#include <list>
#include <cstdlib>
#include <list>
#include <chrono>
#include "quote_source_setting.h"
#include "quote_entity.h"
#include <map>
#include <climits>
#include "pending_quote_dao.h"
#include <condition_variable> // std::condition_variable
#include <memory>
#include <thread>
#include <vector>
#include <memory>
#include "tbb/include/tbb/spin_mutex.h"
#include "pending_quote_dao.h"

#include "quote_interface_shfe_my.h"

using namespace std::chrono;
using namespace std;
using namespace tbb;



namespace quote_agent{
	/**
	quote_source���װ������������Դ�Ľ�������ͨ��TCP������Դ�������ݣ����浽���ݿ��֪ͨqa�ࡣ
	*/
	template<typename QuoteT>
	class quote_source{
	private:

        /*
        * key:model id
        */
        typedef map<long,bool> QuoteStateT;
        
	public:
		quote_source(quote_source_setting setting,MYQuoteData *md_provider);
		~quote_source(void);
		void intialize(void);
		void finalize(void);		

	private:
		bool _subscribed;

		/**
		���ֶμ�¼qaģ�鵱ǰ״̬�����Ϊtrue�����ʾ��ģ�鴦�ڷǹ���״̬
		*/
		bool stopped;
		thread _forwarder_thread;
		
		// TODO: improve
		MYQuoteData *md_provider_;

	public:
		// ����һ����������
		void process_one_quote(const QuoteT *quote);

		void subscribe_to_symbols(SubscribeContracts subscription);

		void OnGTAQuoteData(const QuoteT *quote);

		void subscribe_to_quote(const long &model_id, bool isOption, SubscribeContracts contracts);

		bool match(string &lockup_value,SubscribeContracts &lookup_array);

		// ���ֶδ洢quote_source��������Ϣ
		quote_source_setting setting_;
		
		condition_variable quote_ready_cv;
		mutex quote_ready_mtx;

		QuoteStateT quote_state;
	};
}

#include "general_depth_quote_sourcedef.h"

#endif
