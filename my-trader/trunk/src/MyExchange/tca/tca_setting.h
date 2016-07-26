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
	���ඨ��tca��������Ϣ���磺
	(1) ��������Щͨ��
	*/
	class tca_setting
	{
	public:
		tca_setting(void);
		~tca_setting(void);

		// ����QAģ��������Ϣ
		void Initialize(void);		

		void finalize(void);

		// ͨ���б�
		list<tcs_setting> tcs_sources;

		// �ó������������ļ���·��
		const string config_path;

		map<string,int> ord_vol_limits;

	private:
		// ����tca���ù������ö���
		tcs_setting create_tcs(TiXmlElement* xml_ele);
	};
}
