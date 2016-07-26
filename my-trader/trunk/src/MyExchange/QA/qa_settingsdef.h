#include "quotecategoryoptions.h"
#include "../my_exchange.h"

using namespace quote_agent;

// 构造函数
qa_settings::qa_settings(void):
	config_path("my_capital.config")
{
}

qa_settings::~qa_settings(void)
{
}

void qa_settings::Initialize(void){
	//创建一个XML的文档对象。
	TiXmlDocument config = TiXmlDocument(qa_settings::config_path.c_str());
    config.LoadFile();
    //获得根元素，即MyExchange
    TiXmlElement *RootElement = config.RootElement();    
    //获得第一个Quote节点
    TiXmlElement *quote = RootElement->FirstChildElement("quote");
	if (quote != 0){
		TiXmlElement *quote_src = quote->FirstChildElement();
		while (quote_src != 0){
			quote_source_setting source = this->create_quote_source(quote_src);
			quote_sources.push_back(source);
			quote_src = quote_src->NextSiblingElement();			
		}
	}    
}

quote_source_setting qa_settings::create_quote_source(TiXmlElement* xml_ele){
	quote_source_setting source;
	source.name = xml_ele->Attribute("name");
	source.exchange = (exchange_names)((int)xml_ele->Attribute("exchange")[0]);

	string category_str = xml_ele->Attribute("category");
	if (category_str == "fulldepth"){
		source.category = quote_category_options::FullDepth;
	}
	else if (category_str == "spif"){
		source.category = quote_category_options::SPIF;
	}
	else if (category_str == "cf"){
		source.category = quote_category_options::CF;
	}
	else if (category_str == "stock"){
		source.category = quote_category_options::Stock;
	}
	else if (category_str == "MDOrderStatistic"){
		source.category = quote_category_options::MDOrderStatistic;
	}
	else{
		LOG4CXX_WARN(log4cxx::Logger::getRootLogger(),"do NOT support " << category_str
				<< "type of quote");
	}

#ifdef rss
	xml_ele->QueryBoolAttribute("is_match_quote",&(source.is_match_quote));
#endif

	string type_str = xml_ele->Attribute("type");
	if (type_str == "local"){
		source.quote_type = quote_type_options::local;
	}
	else if (type_str == "forwarder"){
		source.quote_type = quote_type_options::forwarder;
	}

	string tmp = xml_ele->Attribute("shmdatakey");
	source.shmdatakeyfile = xml_ele->Attribute("shmdatakey");
	source.shmdataflagkeyfile = xml_ele->Attribute("shmdataflagkey");
	source.semkeyfile = xml_ele->Attribute("semkey");

	return source;
}

void qa_settings::finalize(void)
{
}
