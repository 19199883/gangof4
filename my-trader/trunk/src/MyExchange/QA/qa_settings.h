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
	qa_settings����Ϊqa�����������Ϣ���磺�ڳ�ʼ��ʱ���������ļ��м���������Ϣ��
	*/
	class qa_settings
	{
	public:
		qa_settings(void);
		~qa_settings(void);
		
		// ����QAģ��������Ϣ
		void Initialize(void);		

		void finalize(void);
	
		// ����Դ�б�
		list<quote_source_setting> quote_sources;

		string MarketdataConfig;

	private:
		
	public:
		// �ó������������ļ���·��
		const string config_path;

		// ��������Դ���ù������ö���
		quote_source_setting create_quote_source(TiXmlElement* xml_ele);
	};
}

#include "qa_settingsdef.h"
