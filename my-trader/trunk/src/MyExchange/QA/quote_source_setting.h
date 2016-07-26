#pragma once

#include <string>
#include "exchange_names.h"
#include "quotecategoryoptions.h"

using namespace std;

namespace quote_agent
{
	enum quote_type_options{
		local = 0,
		forwarder = 1,
	};

	/*
	quote_source_setting定义了quote_source需要的一些配置信息，如：与行情源连接的ip,port等
	*/
	class quote_source_setting
	{
	public:
		quote_source_setting(void);
		~quote_source_setting(void);
		
		// 行情源的IP地址
		std::string name;

		// 行情源的IP地址
		std::string ip;

		// 行情源的端口号
		int port;

		exchange_names exchange;

		// 行情类别
		quote_category_options category;

		quote_type_options quote_type;

		string shmdatakeyfile;
		string shmdataflagkeyfile;
		string semkeyfile;
		bool is_match_quote;

	};
}

#include "quote_source_settingdef.h"
