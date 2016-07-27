#include "tca_setting.h"
#include "tcs_setting.h"

using namespace trading_channel_agent;

tca_setting::tca_setting(void):
	config_path("trasev.config")
{
}


tca_setting::~tca_setting(void)
{
}


void tca_setting::Initialize(void)
{
	//����һ��XML���ĵ�����
	TiXmlDocument config = TiXmlDocument(tca_setting::config_path.c_str());
    config.LoadFile();
    //��ø�Ԫ�أ���MyExchange
    TiXmlElement *RootElement = config.RootElement();    
    TiXmlElement *orderVolLimitE = RootElement->FirstChildElement("OrderVolLimit");
    TiXmlElement *itemE = orderVolLimitE->FirstChildElement();
	while (itemE != NULL){
		int maxVol = 0;
		string symbol = itemE->Attribute("symbol");
		itemE->QueryIntAttribute("maxVol",&maxVol);
		this->ord_vol_limits[symbol] = maxVol;
		itemE = itemE->NextSiblingElement();
	}

    //��õ�һ��tca�ڵ�
    TiXmlElement *tca = RootElement->FirstChildElement("tca");
	if (tca != 0)
	{
		TiXmlElement *src = tca->FirstChildElement();
		while (src != 0)
		{
			tcs_setting source = this->create_tcs(src);
			tcs_sources.push_back(source);
			src = src->NextSiblingElement();			
		}
	}  
}

void tca_setting::finalize(void)
{
}

tcs_setting tca_setting::create_tcs(TiXmlElement* xml_ele)
{
	tcs_setting source;
	source.name = xml_ele->Attribute("name");
	source.config = xml_ele->Attribute("config");
	source.so_file = xml_ele->Attribute("so_file");
	source.tunnel_so_file = xml_ele->Attribute("tunnel_so_file");

	string chn_type = xml_ele->Attribute("channel_type");
	if (chn_type == "ctp"){
		source.channel_type = channel_type_options::ctp;
	}else if (chn_type == "dce"){
		source.channel_type = channel_type_options::dce;
	}else if (chn_type == "femas"){
		source.channel_type = channel_type_options::femas;
	}else if (chn_type == "zeusing"){
		source.channel_type = channel_type_options::zeusing;
	}else if (chn_type == "kingstar_option"){
		source.channel_type = channel_type_options::ks_option;
	}else if (chn_type == "ib"){
		source.channel_type = channel_type_options::ib;
	}else if (chn_type == "esunny"){
		source.channel_type = channel_type_options::esunny;
	}else if (chn_type == "kgi"){
		source.channel_type = channel_type_options::kgi;
	}else if (chn_type == "lts_option"){
		source.channel_type = channel_type_options::lts_option;
	}

	string exchanges_str = xml_ele->Attribute("exchange");
	exchanges_str = exchanges_str + ",";
	size_t pos = 0;
	size_t cur_pos = 0;
	while((pos=exchanges_str.find(",",cur_pos)) != string::npos){
		exchange_names ex_name = static_cast<exchange_names>(exchanges_str.substr(cur_pos,pos-cur_pos)[0]);
		source.exchanges.insert(ex_name);
		cur_pos = pos+1;
	}

	string model_str = xml_ele->Attribute("models");
	model_str = model_str + ",";
	pos = 0;
	cur_pos = 0;
	while((pos=model_str.find(",",cur_pos)) != string::npos){
		string model_id_str = model_str.substr(cur_pos,pos-cur_pos);
		source.models.insert(stol(model_id_str));
		cur_pos = pos+1;
	}

	xml_ele->QueryIntAttribute("init_pos_at_start",&source.my_xchg_cfg_.init_pos_at_start);
	xml_ele->QueryIntAttribute("model_ctrl_oc",&source.my_xchg_cfg_.st_ctrl_oc);
	xml_ele->QueryIntAttribute("change_oc_flag",&source.my_xchg_cfg_.change_oc_flag);
	xml_ele->QueryIntAttribute("init_pos_from_ev",&source.my_xchg_cfg_.init_pos_from_ev);	
	strcpy(source.my_xchg_cfg_.tunnel_cfg_path,source.config.c_str());
	strcpy(source.my_xchg_cfg_.tunnel_so_path,source.tunnel_so_file.c_str());
	strcpy(source.my_xchg_cfg_.tunnel_so_path,source.tunnel_so_file.c_str());

	return source;
}
