//============================================================================
// Name        : stock_config_robot.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::filebuf
#include <string>
#include <stdio.h>
#include <cerrno>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "tinyxml.h"
#include "tinystr.h"

#include "tca.h"
#include "my_exchange_stock_include_v2.5.7/include/my_exchange_datatype.h"

using namespace std;
using namespace trading_channel_agent;

void update_symbols_in_config()
{
	//创建一个XML的文档对象。
	TiXmlDocument config = TiXmlDocument("trasev.config");
	config.LoadFile();
	//获得根元素，即MyExchange
	TiXmlElement *RootElement = config.RootElement();
	//获得第一个strategies节点
	TiXmlElement *strategies = RootElement->FirstChildElement("strategies");
	if (strategies != 0){
		TiXmlElement *strategy = strategies->FirstChildElement();
		const char * ev_name = strategy->Attribute("ev_name");
		while (strategy != 0){
			TiXmlElement* symbol_ele = strategy->FirstChildElement();

			std::filebuf fb;
			filebuf *fb_p = fb.open (ev_name,std::ios::in);
			if (fb_p!=NULL){
				string line;
				std::istream is(&fb);
				while (is){
					getline(is,line);
					if(line.length() < 10) break;
					string symbol = line.substr(18,6);

					TiXmlElement *symbolTmp = (TiXmlElement*)symbol_ele->Clone();
					symbolTmp->SetAttribute("name", symbol.c_str());

					string ex = line.substr(27,1);
					if(ex=="e"){
						symbolTmp->SetAttribute("exchange", "0");
					}else{
						symbolTmp->SetAttribute("exchange", "1");
					}
					strategy->LinkEndChild(symbolTmp);
				}
				fb.close();
			}

			strategy = strategy->NextSiblingElement();
		}
	}

//	//创建一个XML的文档对象。
//	TiXmlDocument *myDocument = new TiXmlDocument();
//	//创建一个根元素并连接。
//	TiXmlElement *RootElement = new TiXmlElement("Persons");
//	myDocument->LinkEndChild(RootElement);
//	//创建一个Person元素并连接。
//	TiXmlElement *PersonElement = new TiXmlElement("Person");
//	RootElement->LinkEndChild(PersonElement);
//	//设置Person元素的属性。
//	PersonElement->SetAttribute("ID", "1");
//	//创建name元素、age元素并连接。
//	TiXmlElement *NameElement = new TiXmlElement("name");
//	TiXmlElement *AgeElement = new TiXmlElement("age");
//	PersonElement->LinkEndChild(NameElement);
//	PersonElement->LinkEndChild(AgeElement);
//	//设置name元素和age元素的内容并连接。
//	TiXmlText *NameContent = new TiXmlText("周星星");
//	TiXmlText *AgeContent = new TiXmlText("22");
//	NameElement->LinkEndChild(NameContent);
//	AgeElement->LinkEndChild(AgeContent);
//
//	TiXmlElement *NameElement1 = new TiXmlElement("周星星11");
//	PersonElement->LinkEndChild(NameElement1);
//	TiXmlElement *NameElement2 = (TiXmlElement*)NameElement1->Clone();
//	PersonElement->LinkEndChild(NameElement2);


//	string fullPath = "OUT.TXT";
	config.SaveFile("trasev.config");
}



/*
 * first argument:the path of mytrader configuration file 'my_capital'
 * second argument:the path of forwarder configuration file
 */
int main(int argc, char  *argv[])
{
	for(int i=0; i<argc; i++){
		string tmp = argv[i];
		 std::cout << tmp << endl;
	}

	tca tca_inc;
	long model = 0;
	tcs *tcs_inc = tca_inc.get_tcs(model)[0];
	// fetch position data from counter and output them to file 'pos_data.txt'
	// fetch previous day position
//	T_PositionReturn pos_rtn = tcs_inc->query_position();
//	while(0 != pos_rtn.error_no){
//
//		this_thread::sleep_for(std::chrono::milliseconds(500));
//		pos_rtn = tcs_inc->query_position();
//	}

	// read file 'pos_data.txt' and hs300.txt,
	// and then update mytrader configuration file and forwarder configuration file

#ifdef STOCK_TRADING
	update_symbols_in_config();
#endif

	ifstream in("test.txt");
	char* errstr = strerror(errno);
	bool good = in.good();
	std::filebuf fb;
	filebuf *fb_p = fb.open ("./test.txt",std::ios::in);
	if (fb_p!=NULL)
	{
		string line;
		std::istream is(&fb);
		while (is){
			getline(is,line);
		  std::cout << line << endl;
		}
		fb.close();
	}else{

		//int err = stderr;
		//fb.
	}
}
