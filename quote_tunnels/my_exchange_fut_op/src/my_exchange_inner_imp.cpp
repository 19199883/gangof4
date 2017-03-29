#include "my_exchange_inner_imp.h"
#include <fstream>
#include <functional>
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

void* MYExchangeInnerImp::QryPosForInit()
{
    // query position of account
    T_QryPosition qry_pos;
    usleep(150 * 1000); // wait for login state
    while (!have_send_qry_pos_flag)
    {
        p_tunnel->QueryPosition(&qry_pos);
        usleep(1001 * 1000); // wait 1.001s for flow control
    }
    return NULL;
}

namespace
{
struct UselessCharPred
{
    bool operator()(const char &l)
    {
        return l == ' ' || l == '\t';
    }
};
}

// TODO: pos_calcu, wangying, modify this function to support for saving each strategy's position respectively
void MYExchangeInnerImp::InitModelPosByEvFile()
{
    for (const EvFileNameOfModel::value_type &v : cfg_.Position_policy().ev_file_name_of_model) {
        int mod_id = v.first;
        string  mod_name = v.second;
		if (exists(mod_name)){ 
			int long_pos = 0;
			int short_pos = 0;
			string cont = "";
			get_pos(mod_name, long_pos, short_pos, cont);
			model_pos.InitPosition(mod_id, cont.c_str(), MY_TNL_D_BUY,long_pos);
			model_pos.InitPosition(mod_id, cont.c_str(), MY_TNL_D_SELL, short_pos);
			EX_LOG_INFO("model(%d) pos(L:%d;S:%d)", mod_id, long_pos,short_pos);
		}
    }
}

void MYExchangeInnerImp::get_pos(string &stra, int &long_pos, int &short_pos, string &cont)
{
	string pos_file = stra + ".pos";
	char buf[1024];
	std::ifstream is;
	is.open (pos_file);
	if (is) {
		is.getline(buf, sizeof(buf));
		string line = buf;
		
		int cur_pos = 0;
		int next_pos = 0;
		next_pos = line.find(';', cur_pos);
		string stra_id = line.substr(cur_pos, next_pos-cur_pos);
		
		cur_pos = next_pos + 1;
		next_pos = line.find(';', cur_pos);
		// commented by wangying on 2017-03-23
		// stra = line.substr(cur_pos, next_pos-cur_pos);
		
		cur_pos = next_pos + 1;
		next_pos = line.find(';', cur_pos);
		cont = line.substr(cur_pos, next_pos-cur_pos);

		cur_pos = next_pos + 1;
		next_pos = line.find(';', cur_pos);
		long_pos = stoi(line.substr(cur_pos, next_pos-cur_pos));

		cur_pos = next_pos + 1;
		short_pos = stoi(line.substr(cur_pos));

	}

	is.close();
}

