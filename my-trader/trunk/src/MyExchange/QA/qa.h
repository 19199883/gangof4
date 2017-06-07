#ifndef QA_H_
#define QA_H_

#include "../my_exchange.h"
#include <list>
#include <map>
#include <boost/function.hpp>
#include <boost/function_equal.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/thread_time.hpp>
#include "qa_settings.h"
#include "general_depth_quote_source.h"
#include "exchange_names.h"

#include "quote_datatype_cffex_level2.h"
#include "quote_datatype_level1.h"
#include "quote_datatype_shfe_my.h"
#include "quote_datatype_shfe_deep.h"
#include "quote_datatype_level1.h"
#include "quote_datatype_dce_level2.h"

#include "quote_interface_shfe_my.h"

#include <memory>

using namespace std;

namespace quote_agent
{
	/**
	qaʱquote agent����д������QAģ�����Ľӿڣ������������˶��quote_source������
	����Ψһ�ġ�ͳһ�ķ���ӿ�
	*/
	template<
	typename SPIFQuoteT,
	typename CFQuoteT = int,
	typename StockQuoteT = int,
	typename FullDepthQuoteT = int,
	typename QuoteT5 = int>
	class qa
	{
	public :
		/**
		 * four categories of quote sources
		 */
		shared_ptr<quote_source<SPIFQuoteT> > spif_quote_source_ptr;
		shared_ptr<quote_source<CFQuoteT> > cf_quote_source_ptr;
		shared_ptr<quote_source<StockQuoteT> > stock_quote_source_ptr;
		shared_ptr<quote_source<FullDepthQuoteT> > full_depth_quote_source_ptr;
		shared_ptr<quote_source<QuoteT5> > quote_source5_ptr;

		/**
		���캯��
		*/
		qa(void);

		/**
		��������
		*/
		~qa(void);

		/**
		��ʼ��QAģ�飬�磺�������ô���quote_sourceʵ����
		*/
		void Initialize(void);

		/**
		�ͷ�QAģ���������Դ�����ͷ�qa_source����
		*/
		void finalize(void);

		/**
		�÷������ڼ�¼�ĸ�ģ�Ͷ�������Щ��Լ������
		*/
		void subscribe_to_quote(const long &model_id, map<string,quote_category_options> symbols);
			
	private:
		
		// ��������֪ͨ�¼�
		void process_quote_notify(int quote);

		// TODO: improve 2
		MYQuoteData* build_quote_provider(SubscribeContracts &subscription) {
			return new MYQuoteData(&subs_, setting.MarketdataConfig);
		}

	private:
		// ���ֶδ洢qa�����ö���
		qa_settings setting;

		/**
		���ֶμ�¼qaģ�鵱ǰ״̬�����Ϊtrue�����ʾ��ģ�鴦�ڷǹ���״̬
		*/
		bool stopped;
		//
		// TODO: improve 2
		MYQuoteData *md_provider_;
		SubscribeContracts subs_;
	};
}

using namespace quote_agent;
#include  "qadef.h"
#endif
