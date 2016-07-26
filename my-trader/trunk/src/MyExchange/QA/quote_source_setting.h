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
	quote_source_setting������quote_source��Ҫ��һЩ������Ϣ���磺������Դ���ӵ�ip,port��
	*/
	class quote_source_setting
	{
	public:
		quote_source_setting(void);
		~quote_source_setting(void);
		
		// ����Դ��IP��ַ
		std::string name;

		// ����Դ��IP��ַ
		std::string ip;

		// ����Դ�Ķ˿ں�
		int port;

		exchange_names exchange;

		// �������
		quote_category_options category;

		quote_type_options quote_type;

		string shmdatakeyfile;
		string shmdataflagkeyfile;
		string semkeyfile;
		bool is_match_quote;

	};
}

#include "quote_source_settingdef.h"
