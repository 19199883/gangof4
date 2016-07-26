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

void MYExchangeInnerImp::InitModelPosByEvFile()
{
    for (const EvFileNameOfModel::value_type &v : cfg_.Position_policy().ev_file_name_of_model)
    {
        int mod_id = v.first;
        std::string ev_file_name = v.second;

        // read
        std::ifstream evf_stream(ev_file_name.c_str());
        if (!evf_stream)
        {
            EX_LOG_ERROR("InitModelPosByEvFile - open evfile(%s) failed", ev_file_name.c_str());
            return;
        }

        // read and parse
        std::string line;
        bool start_flag = false;
        int format_type = 0; // 1:stock option; 2:underlying future; 3:future option
//        Class.  Yesterday symbols information: symbol,   buy_volume, buy_price, sell_volume, sell_price
//           510050C1511M02300,         1,    0.0000,         0,    0.0000
//           510050P1511M02300,         1,    0.0000,         0,    0.0000
//           510050P1511M02400,         1,    0.0000,         0,    0.0000
//        End,0,0.0,0,0.0
//        Class.  Yesterday underlying symbols, net positions, and fair prices
//         End, 0, 0
//        Class.  Yesterday option symbols, yesterday last price, impli price, fair prices and net positions
//        End,0,0,0,0

        while (std::getline(evf_stream, line))
        {
            if (start_flag)
            {
                // session end flag
                if (line.find("End") != std::string::npos)
                {
                    start_flag = false;
                    continue;
                }

                std::vector<std::string> fields;
                std::string new_line(line.begin(), std::remove_if(line.begin(), line.end(), UselessCharPred()));
                EX_LOG_INFO("parse pos: %s", new_line.c_str());
                my_cmn::MYStringSplit(new_line.c_str(), fields, ',');

                switch (format_type)
                {
                    case 1: // 1:stock option
                    {
                        if (fields.size() >= 5)
                        {
                            // symbol, symbol-2,   buy_volume, buy_price, sell_volume, sell_price
                            model_pos.InitPosition(mod_id, fields[0].c_str(), MY_TNL_D_BUY, atoi(fields[2].c_str()));
                            model_pos.InitPosition(mod_id, fields[0].c_str(), MY_TNL_D_SELL, atoi(fields[4].c_str()));
                        }
                        else
                        {
                            EX_LOG_ERROR("InitModelPosByEvFile - parse one line failed:%s", new_line.c_str());
                        }
                    }
                        break;
                    case 2: // 2:underlying future
                    {
                        if (fields.size() >= 2)
                        {
                            // underlying 0, 1
                            int pos_vol = atoi(fields[1].c_str());
                            if (pos_vol > 0) model_pos.InitPosition(mod_id, fields[0].c_str(), MY_TNL_D_BUY, pos_vol);
                            if (pos_vol < 0) model_pos.InitPosition(mod_id, fields[0].c_str(), MY_TNL_D_SELL, -pos_vol);
                        }
                        else
                        {
                            EX_LOG_ERROR("InitModelPosByEvFile - parse one line failed:%s", new_line.c_str());
                        }
                    }
                        break;
                    case 3: // 3:future option
                    {
                        if (fields.size() >= 10)
                        {
                            // call 1,5
                            int pos_vol = atoi(fields[4].c_str());
                            if (pos_vol > 0) model_pos.InitPosition(mod_id, fields[0].c_str(), MY_TNL_D_BUY, pos_vol);
                            if (pos_vol < 0) model_pos.InitPosition(mod_id, fields[0].c_str(), MY_TNL_D_SELL, -pos_vol);

                            // put 6,10
                            pos_vol = atoi(fields[9].c_str());
                            if (pos_vol > 0) model_pos.InitPosition(mod_id, fields[5].c_str(), MY_TNL_D_BUY, pos_vol);
                            if (pos_vol < 0) model_pos.InitPosition(mod_id, fields[5].c_str(), MY_TNL_D_SELL, -pos_vol);
                        }
                        else
                        {
                            EX_LOG_ERROR("InitModelPosByEvFile - parse one line failed:%s", new_line.c_str());
                        }
                    }
                        break;
                    default:
                        // unknown type
                        break;
                }
            }
            else
            {
                if (line.find("Yesterday") != std::string::npos)
                {
                    start_flag = true;

                    if (line.find("symbols information") != std::string::npos)
                    {
                        format_type = 1;
                    }
                    else if (line.find("underlying symbols") != std::string::npos)
                    {
                        format_type = 2;
                    }
                    else if (line.find("option symbols") != std::string::npos)
                    {
                        format_type = 3;
                    }
                    else
                    {
                        EX_LOG_ERROR("parse evfile, format_type is unknown: %s", line.c_str());
                        start_flag = false;
                    }

                    EX_LOG_INFO("parse position start flag line: %s, format_type:%d(1:stock option; 2:underlying future; 3:future option)",
                        line.c_str(), format_type);

                    continue;
                }
            }
        }

        evf_stream.close();
    }
}
