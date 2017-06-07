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
	qa时quote agent的缩写。它是QA模块对外的接口，它对外隐藏了多个quote_source，对外
	呈现唯一的、统一的服务接口
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
		构造函数
		*/
		qa(void);

		/**
		析构函数
		*/
		~qa(void);

		/**
		初始化QA模块，如：根据配置创建quote_source实例等
		*/
		void Initialize(void);

		/**
		释放QA模块的所有资源，如释放qa_source对象
		*/
		void finalize(void);

		/**
		该方法用于记录哪个模型订阅了哪些合约的行情
		*/
		void subscribe_to_quote(const long &model_id, map<string,quote_category_options> symbols);
			
	private:
		
		// 处理行情通知事件
		void process_quote_notify(int quote);

		// TODO: improve 2
		MYQuoteData* build_quote_provider(SubscribeContracts &subscription) {
			return new MYQuoteData(&subs_, setting.MarketdataConfig);
		}

	private:
		// 该字段存储qa的配置对象
		qa_settings setting;

		/**
		该字段记录qa模块当前状态，如果为true，则表示该模块处于非工作状态
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
