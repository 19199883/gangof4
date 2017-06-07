#pragma once

#include <tinyxml.h>
#include <tinystr.h>
#include <list>
#include "quote_source_setting.h"
#include "general_depth_quote_source.h"

using namespace std;

namespace quote_agent
{
	/**
	qa_settings负责为qa类关联配置信息，如：在初始化时，从配置文件中加载配置信息等
	*/
	class qa_settings
	{
	public:
		qa_settings(void);
		~qa_settings(void);
		
		// 加载QA模块配置信息
		void Initialize(void);		

		void finalize(void);
	
		// 行情源列表
		list<quote_source_setting> quote_sources;

		string MarketdataConfig;

	private:
		
	public:
		// 该常量定义配置文件的路径
		const string config_path;

		// 根据行情源配置构建配置对象
		quote_source_setting create_quote_source(TiXmlElement* xml_ele);
	};
}

#include "qa_settingsdef.h"
