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
#include "tcs.h"

using namespace std;
using namespace trading_channel_agent;

/*
 * example
 * contract exchange dir pos model_id
 * IF1506   G            100     1000092
 * 000001   0            1000     1000093
 *
 * return value:
 *
 */
int output_positions(string pos_file)
{
	tca *tca_ins = new tca();
	tca_ins->initialize();
	list<tcs*>::iterator it = tca_ins->sources.begin();
	list<tcs*>::iterator end = tca_ins->sources.end();
	std::filebuf fb;
	filebuf *fb_p = fb.open (pos_file,std::ios::out);
	for(; it!=end; ++it){
		T_PositionReturn pos_rtn = (*it)->query_position();
		while(0 != pos_rtn.error_no){
			this_thread::sleep_for(std::chrono::milliseconds(500));
			pos_rtn = (*it)->query_position();
		} // end while(0 != pos_rtn.error_no){

		if (fb_p!=NULL){
			std::ostream os(&fb);
			std::vector<PositionDetail>::iterator it = pos_rtn.datas.begin();
			std::vector<PositionDetail>::iterator end = pos_rtn.datas.end();
			map<string,list<PositionDetail> > group;
			for( ; it!=end; ++it){
				os << it->stock_code << " " << it->exchange_type << " " << it->direction << " "
						<< it->position << endl;
			}
		}	// end if (fb_p!=NULL){
	}	// end for(; it!=end; ++it){
	fb.close();

	//tca_ins.finalize();
}

/*
 * this function reads position data from pos_file file and store them
 * into set object.
 */
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
	TiXmlDocument config = TiXmlDocument("trasev.config");
	config.LoadFile();
	//获得根元素，即MyExchange
	TiXmlElement *RootElement = config.RootElement();
	//获得第一个strategies节点
	TiXmlElement *strategies = RootElement->FirstChildElement("strategies");
	if (strategies != 0){
		TiXmlElement *strategy = strategies->FirstChildElement();
		while (strategy != 0){
			TiXmlElement* symbol_ele = strategy->FirstChildElement();
			// first, remove all symbol elements other than 'placeholder'
			while (symbol_ele != 0){
				string symbol = symbol_ele->Attribute("name");
				string exchange = symbol_ele->Attribute("exchange");
				if(symbol != "placeholder" &&
						(exchange=="1" || exchange=="0")){
					strategy->RemoveChild(symbol_ele);
				}
				symbol_ele = symbol_ele->NextSiblingElement();
			}

			TiXmlElement *placeholder_ele = strategy->FirstChildElement();
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

					TiXmlElement *symbolTmp = (TiXmlElement*)placeholder_ele->Clone();
					symbolTmp->SetAttribute("name", symbol.c_str());

					string ex = line.substr(27,1);
					if(ex=="e"){
						symbolTmp->SetAttribute("exchange", "0");
					}else{
						symbolTmp->SetAttribute("exchange", "1");
					}
					strategy->LinkEndChild(symbolTmp);
				} // end while (is){
				fb.close();
			} // end if (fb_p!=NULL){

			for(map<string,string>::iterator it=pos.begin(); it!=pos.end(); ++it){
				if(it->second != "0" && it->second != "1") continue;

				TiXmlElement *symbolTmp = (TiXmlElement*)placeholder_ele->Clone();
				symbolTmp->SetAttribute("name", it->first.c_str());
				symbolTmp->SetAttribute("exchange", it->second.c_str());
				strategy->LinkEndChild(symbolTmp);
			}

			strategy = strategy->NextSiblingElement();
		}	// end while (strategy != 0){
	}

	config.SaveFile("trasev.config");
	return 0;
}
