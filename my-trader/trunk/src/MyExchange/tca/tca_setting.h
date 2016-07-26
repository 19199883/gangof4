#pragma once

#include <tinyxml.h>
#include <tinystr.h>
#include <list>
#include "tcs_setting.h"
#include <string>
#include <map>

using namespace std;

namespace trading_channel_agent
{
	/**
	该类定义tca的配置信息，如：
	(1) 连接了哪些通道
	*/
	class tca_setting
	{
	public:
		tca_setting(void);
		~tca_setting(void);

		// 加载QA模块配置信息
		void Initialize(void);		

		void finalize(void);

		// 通道列表
		list<tcs_setting> tcs_sources;

		// 该常量定义配置文件的路径
		const string config_path;

		map<string,int> ord_vol_limits;

	private:
		// 根据tca配置构建配置对象
		tcs_setting create_tcs(TiXmlElement* xml_ele);
	};
}
