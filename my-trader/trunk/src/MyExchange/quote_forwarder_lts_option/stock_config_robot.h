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


using namespace std;

int read_pos(string pos_file, map<string,string> &stock_pos)
{
	const string SEPARATOR = " ";
	string line;
	std::ifstream is;
	std::filebuf * fb = is.rdbuf();
	fb->open (pos_file,std::ios::in);
	while (is){
		getline(is,line);
		if(line.length() < 2) break;

		int pos = 0;
		int len = line.find(SEPARATOR);
		string symbol = line.substr(pos,len);

		pos = len+1;
		len = line.find(SEPARATOR,pos);
		string ex = line.substr(pos,len-pos);

		stock_pos[symbol] = ex;
	} // end while (is){
	fb->close();
}

int update_symbols_in_config(string hs300File, map<string,string> &pos)
{
	//创建一个XML的文档对象。
	TiXmlDocument config = TiXmlDocument("tdf_quote_forwarder.xml");
	config.LoadFile();
	//获得根元素，即MyExchange
	TiXmlElement *RootElement = config.RootElement();
	//获得第一个strategies节点
	TiXmlElement *subscriptionE = RootElement->FirstChildElement("subscription");
	if (subscriptionE != 0){
		TiXmlElement *itemE = subscriptionE->FirstChildElement();
		while (itemE != 0){
			string name = itemE->Attribute("value");
			if(name != "placeholder"){
				subscriptionE->RemoveChild(itemE);
			}
			itemE = itemE->NextSiblingElement();
		}

		TiXmlElement *placeholder_element = subscriptionE->FirstChildElement();
		std::filebuf fb;
		filebuf *fb_p = fb.open (hs300File,std::ios::in);
		if (fb_p!=NULL){
			string line;
			std::istream is(&fb);
			while (is){
				getline(is,line);
				if(line.length() < 10) break;

				string symbol = line.substr(18,6);
				if (pos.end()!=pos.find(symbol)){
					pos.erase(symbol);
				}

				TiXmlElement *itemTmp = (TiXmlElement*)placeholder_element->Clone();
				itemTmp->SetAttribute("value", symbol.c_str());
				subscriptionE->LinkEndChild(itemTmp);
			} //end while (is){
			fb.close();
		}	// end if (fb_p!=NULL){

		for(map<string,string>::iterator it=pos.begin(); it!=pos.end(); ++it){
			if(it->second=="G" || it->first=="#CASH") continue;

			TiXmlElement *symbolTmp = (TiXmlElement*)placeholder_element->Clone();
			symbolTmp->SetAttribute("value", it->first.c_str());
			subscriptionE->LinkEndChild(symbolTmp);
		}
	}	// end if (subscriptionE != 0){

	config.SaveFile("tdf_quote_forwarder.xml");
	return 0;
}



/*
 * first argument:the path of mytrader configuration file 'my_capital'
 * second argument:the path of forwarder configuration file
 */
//int main(int argc, char  *argv[])
//{
//	for(int i=0; i<argc; i++){//int main(int argc, char  *argv[])
//{
//	for(int i=0; i<argc; i++){
//		string tmp = argv[i];
//		 std::cout << tmp << endl;
//	}
//
//	tca tca_inc;
//	long model = 0;
//	tcs *tcs_inc = tca_inc.get_tcs(model)[0];
//	// fetch position data from counter and output them to file 'pos_data.txt'
//	// fetch previous day position
////	T_PositionReturn pos_rtn = tcs_inc->query_position();
////	while(0 != pos_rtn.error_no){
////
////		this_thread::sleep_for(std::chrono::milliseconds(500));
////		pos_rtn = tcs_inc->query_position();
////	}
//
//	// read file 'pos_data.txt' and hs300.txt,
//	// and then update mytrader configuration file and forwarder configuration file
//
//#ifdef STOCK_TRADING
//	update_symbols_in_config();
//#endif
//
//	ifstream in("test.txt");
//	char* errstr = strerror(errno);
//	bool good = in.good();
//	std::filebuf fb;
//	filebuf *fb_p = fb.open ("./test.txt",std::ios::in);
//	if (fb_p!=NULL)
//	{
//		string line;
//		std::istream is(&fb);
//		while (is){
//			getline(is,line);
//		  std::cout << line << endl;
//		}
//		fb.close();
//	}else{
//
//		//int err = stderr;
//		//fb.
//	}
//}
//		string tmp = argv[i];
//		 std::cout << tmp << endl;
//	}
//
//	tca tca_inc;
//	long model = 0;
//	tcs *tcs_inc = tca_inc.get_tcs(model)[0];
//	// fetch position data from counter and output them to file 'pos_data.txt'
//	// fetch previous day position
////	T_PositionReturn pos_rtn = tcs_inc->query_position();
////	while(0 != pos_rtn.error_no){
////
////		this_thread::sleep_for(std::chrono::milliseconds(500));
////		pos_rtn = tcs_inc->query_position();
////	}
//
//	// read file 'pos_data.txt' and hs300.txt,
//	// and then update mytrader configuration file and forwarder configuration file
//
//#ifdef STOCK_TRADING
//	update_symbols_in_config();
//#endif
//
//	ifstream in("test.txt");
//	char* errstr = strerror(errno);
//	bool good = in.good();
//	std::filebuf fb;
//	filebuf *fb_p = fb.open ("./test.txt",std::ios::in);
//	if (fb_p!=NULL)
//	{
//		string line;
//		std::istream is(&fb);
//		while (is){
//			getline(is,line);
//		  std::cout << line << endl;
//		}
//		fb.close();
//	}else{
//
//		//int err = stderr;
//		//fb.
//	}
//}
