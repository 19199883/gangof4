#include "quote_cmn_config.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>

#include "my_cmn_crypto.h"
#include "quote_cmn_utility.h"

using boost::property_tree::ptree;

void ConfigData::Load(const std::string &cfg_file)
{
    ptree pt;
    try
    {
        // Load the XML file into the property tree. If reading fails
        // (cannot open file, parse error), an exception is thrown.
        MY_LOG_INFO("read config file: %s", cfg_file.c_str());
        read_xml(cfg_file, pt);

        ParseAppConfig(pt);

        // 登录配置
        ParseLogonConfig(pt);

        // 订阅配置
        ParseSubscribeConfig(pt);

        // 存库配置
        ParseQuoteDataSaveParamConfig(pt);

        // 解析配置
        ParseQuoteCfgFiles(pt);

        ParseTransferPolicy(pt);
    }
    catch (...)
    {
        MY_LOG_FATAL("parse config file fail, file: %s", cfg_file.c_str());
    }
}

void ConfigData::ParseAppConfig(ptree &pt)
{
    try
    {
        // 应用程序配置
        app_cfg_.provider_type = pt.get<int>("matketdata_config.provider_type");
        MY_LOG_INFO("app_cfg_.provider_type: %d", app_cfg_.provider_type);
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("ParseAppConfig, info: %s", e.what());
    }
    catch (...)
    {
        MY_LOG_ERROR("unknown exception in ParseAppConfig.");
    }
}

void ConfigData::ParseLogonConfig(ptree &pt)
{
    try
    {
        for (ptree::value_type &v : pt.get_child("matketdata_config.login.serverlist"))
        {
            if (v.first == "addr")
            {
                logon_config_.quote_provider_addrs.push_back(v.second.data());
            }
        }
    }
    catch (...)
    {
        MY_LOG_INFO("parse <matketdata_config.login.serverlist> failed.");
    }

    try
    {
        logon_config_.trade_server_addr = pt.get<std::string>("matketdata_config.login.trade_server_addr");
    }
    catch (...)
    {
        MY_LOG_INFO("parse <matketdata_config.login.trade_server_addr> failed.");
    }

    try
    {
        logon_config_.broker_id = pt.get<std::string>("matketdata_config.login.brokerid");
        logon_config_.account = pt.get<std::string>("matketdata_config.login.account_id");
        logon_config_.password = pt.get<std::string>("matketdata_config.login.password");
    }
    catch (...)
    {
        MY_LOG_INFO("parse <matketdata_config.login.broker_id|account|password> failed.");
    }

    if (!logon_config_.password.empty())
    {
        logon_config_.password = my_cmn::MYDecrypt(logon_config_.password);
    }

    try
    {
        logon_config_.topic = pt.get<std::string>("matketdata_config.login.topic");
    }
    catch (...)
    {
        MY_LOG_INFO("parse <matketdata_config.login.topic> failed.");
    }

    // new added items, maybe miss in old config file
    try
    {
        for (ptree::value_type &v : pt.get_child("matketdata_config.login.mbl_data_addrs"))
        {
            if (v.first == "addr")
            {
                logon_config_.mbl_data_addrs.push_back(v.second.data());
            }
        }
        for (ptree::value_type &v : pt.get_child("matketdata_config.login.depth_market_data_addrs"))
        {
            if (v.first == "addr")
            {
                logon_config_.depth_market_data_addrs.push_back(v.second.data());
            }
        }
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("parse data publish address failed, it used in shfe, info: %s", e.what());
    }

    // new added parameter on 20141113 for monitor xh provider
    try
    {
        for (ptree::value_type &v : pt.get_child("matketdata_config.login.status_data_addrs"))
        {
            if (v.first == "addr")
            {
                logon_config_.status_data_addrs.push_back(v.second.data());
            }
        }
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("parse status publish address failed, it used in shfe_xh, info: %s", e.what());
    }

    // toe parameters added on 20151222 by chenyongshun
    try
    {
        for (ptree::value_type &v : pt.get_child("matketdata_config.login.toe_config"))
        {
            if (v.first == "item")
            {
                ptree t = v.second;
                TOEConfig toe_cfg;

                // <item toe_id="0" cnn_cpu="1" recv_cpu="1" pro_type="0" loc_ip="67.153.0.8" loc_port="18110" remote_ip="67.153.0.100" remote_port="18111" mc_ip="232.8.0.18"/>
                toe_cfg.toe_id = t.get<std::string>("<xmlattr>.toe_id");
                toe_cfg.connect_cpu_id = t.get<std::string>("<xmlattr>.cnn_cpu");
                toe_cfg.data_recv_cpu_id = t.get<std::string>("<xmlattr>.recv_cpu");
                toe_cfg.pro_type = t.get<std::string>("<xmlattr>.pro_type");
                toe_cfg.local_ip = t.get<std::string>("<xmlattr>.loc_ip");
                toe_cfg.local_port = t.get<std::string>("<xmlattr>.loc_port");
                toe_cfg.remote_ip = t.get<std::string>("<xmlattr>.remote_ip");
                toe_cfg.remote_port = t.get<std::string>("<xmlattr>.remote_port");
                toe_cfg.multicast_ip = t.get<std::string>("<xmlattr>.mc_ip");

                logon_config_.toe_cfg.push_back(toe_cfg);
            }
        }

    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("parse toe parameters failed(only used in toe), info: %s", e.what());
    }

    // lts parameters added on 20151225 by chenyongshun
    try
    {
        logon_config_.query_server_addr = pt.get<std::string>("matketdata_config.login.query_server_addr");
        logon_config_.query_brokerid = pt.get<std::string>("matketdata_config.login.query_brokerid");
        logon_config_.query_account = pt.get<std::string>("matketdata_config.login.query_account");
        logon_config_.query_password = pt.get<std::string>("matketdata_config.login.query_password");
        logon_config_.user_productinfo = pt.get<std::string>("matketdata_config.login.user_productinfo");
        logon_config_.auth_code = pt.get<std::string>("matketdata_config.login.auth_code");
        if (!logon_config_.query_password.empty())
        {
            logon_config_.query_password = my_cmn::MYDecrypt(logon_config_.query_password);
        }
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("parse lts parameters failed(only used in lts), info: %s", e.what());
    }
    //add by hwg zce_multi
    try
    {
        for (ptree::value_type &v : pt.get_child("matketdata_config.login.zce_multi_config"))
        {
            if (v.first == "item")
            {
                ptree t = v.second;
                zce_multi_config zce_multi_cfg;

                // <item toe_id="0" cnn_cpu="1" recv_cpu="1" pro_type="0" loc_ip="67.153.0.8" loc_port="18110" remote_ip="67.153.0.100" remote_port="18111" mc_ip="232.8.0.18"/>

                zce_multi_cfg.realtime_ip = t.get<std::string>("<xmlattr>.realtime_ip");
                zce_multi_cfg.realtime_port = t.get<std::string>("<xmlattr>.realtime_port");
                zce_multi_cfg.recovery_ip = t.get<std::string>("<xmlattr>.recovery_ip");
                zce_multi_cfg.recovery_port = t.get<std::string>("<xmlattr>.recovery_port");
                zce_multi_cfg.gourp_ip = t.get<std::string>("<xmlattr>.gourp_ip");
                logon_config_.zce_multi_cfg.push_back(zce_multi_cfg);
            }
        }

    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("parse toe parameters failed(only used in toe), info: %s", e.what());
    }

    try
    {
        // show config information
        for (const std::string &v : logon_config_.quote_provider_addrs)
        {
            MY_LOG_INFO("quote provider address: %s", v.c_str());
        }
        MY_LOG_INFO("logon_config_.trade_server_addr: %s", logon_config_.trade_server_addr.c_str());
        MY_LOG_INFO("logon_config_.broker_id: %s", logon_config_.broker_id.c_str());
        MY_LOG_INFO("logon_config_.account: %s", logon_config_.account.c_str());
        MY_LOG_INFO("logon_config_.topic: %s", logon_config_.topic.c_str());
        for (const std::string &v : logon_config_.status_data_addrs)
        {
            MY_LOG_INFO("status_data_addrs: %s", v.c_str());
        }
        for (const std::string &v : logon_config_.mbl_data_addrs)
        {
            MY_LOG_INFO("mbl_data_addrs: %s", v.c_str());
        }
        for (const std::string &v : logon_config_.depth_market_data_addrs)
        {
            MY_LOG_INFO("depth_market_data_addrs: %s", v.c_str());
        }

        for (const TOEConfig &tcfg : logon_config_.toe_cfg)
        {
            MY_LOG_INFO("toe_config.connect_cpu_id: %s", tcfg.connect_cpu_id.c_str());
            MY_LOG_INFO("toe_config.data_recv_cpu_id: %s", tcfg.data_recv_cpu_id.c_str());
            MY_LOG_INFO("toe_config.toe_id: %s", tcfg.toe_id.c_str());
            MY_LOG_INFO("toe_config.pro_type: %s", tcfg.pro_type.c_str());
            MY_LOG_INFO("toe_config.local_ip: %s", tcfg.local_ip.c_str());
            MY_LOG_INFO("toe_config.local_port: %s", tcfg.local_port.c_str());
            MY_LOG_INFO("toe_config.remote_ip: %s", tcfg.remote_ip.c_str());
            MY_LOG_INFO("toe_config.remote_port: %s", tcfg.remote_port.c_str());
            MY_LOG_INFO("toe_config.multicast_ip: %s", tcfg.multicast_ip.c_str());
        }

        MY_LOG_INFO("logon_config_.query_server_addr: %s", logon_config_.query_server_addr.c_str());
        MY_LOG_INFO("logon_config_.query_brokerid: %s", logon_config_.query_brokerid.c_str());
        MY_LOG_INFO("logon_config_.query_account: %s", logon_config_.query_account.c_str());
        MY_LOG_INFO("logon_config_.user_productinfo: %s", logon_config_.user_productinfo.c_str());
        MY_LOG_INFO("logon_config_.auth_code: %s", logon_config_.auth_code.c_str());
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("ParseLogonConfig, info: %s", e.what());
    }
    catch (...)
    {
        MY_LOG_ERROR("unknown exception in ParseLogonConfig.");
    }
}

void ConfigData::ParseSubscribeConfig(ptree &pt)
{
    try
    {
        for (ptree::value_type &v : pt.get_child("matketdata_config.stock_list"))
        {
            if (v.first == "code")
            {
                // 重复检查
                if (subscribe_datas_.find(v.second.data()) != subscribe_datas_.end())
                {
                    MY_LOG_WARN("duplicate subscribe code: %s", v.second.data().c_str());
                    continue;
                }
                subscribe_datas_.insert(v.second.data());
            }
        }

        // 订阅全部的话，其它就作废了
        if (subscribe_datas_.find("All") != subscribe_datas_.end())
        {
            subscribe_datas_.clear();
            subscribe_datas_.insert("All");
        }

        // show config information
        for (const std::string &v : subscribe_datas_)
        {
            MY_LOG_INFO("subscribe: %s", v.c_str());
        }
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("ParseSubscribeConfig, info: %s", e.what());
    }
    catch (...)
    {
        MY_LOG_ERROR("unknown exception in ParseSubscribeConfig.");
    }
}

void ConfigData::ParseQuoteDataSaveParamConfig(boost::property_tree::ptree &pt)
{
    quote_save_param_.save_to_txt_file = false;
    try
    {
        quote_save_param_.save_to_txt_file = pt.get<bool>("matketdata_config.quote_data_save.save_txt_file");

        if (quote_save_param_.save_to_txt_file)
        {
            MY_LOG_INFO("save to file");
        }
        else
        {
            MY_LOG_INFO("not save to file");
        }
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_ERROR("parse <matketdata_config.quote_data_save.save_txt_file> failed, info: %s", e.what());
    }
    catch (...)
    {
        MY_LOG_ERROR("parse <matketdata_config.quote_data_save.save_txt_file> failed.");
    }

    try
    {
        quote_save_param_.file_name_id = pt.get<std::string>("matketdata_config.quote_data_save.file_name_id");
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("parse <matketdata_config.quote_data_save.file_name_id> failed, info: %s", e.what());
    }
    catch (...)
    {
        MY_LOG_INFO("parse <matketdata_config.quote_data_save.file_name_id> failed.");
    }
}

void ConfigData::ParseQuoteCfgFiles(ptree &pt)
{
    try
    {
        for (ptree::value_type &v : pt.get_child("matketdata_config.quote_cfg_file_list"))
        {
            if (v.first == "file_name")
            {
                // 重复检查
                if (quote_cfg_files_.find(v.second.data()) != quote_cfg_files_.end())
                {
                    MY_LOG_WARN("duplicate configure file: %s", v.second.data().c_str());
                    continue;
                }
                quote_cfg_files_.insert(v.second.data());
            }
        }

        // show config information
        for (const std::string &v : quote_cfg_files_)
        {
            MY_LOG_INFO("configure files: %s", v.c_str());
        }
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("ParseQuoteCfgFiles error, info: %s", e.what());
    }
    catch (...)
    {
        MY_LOG_ERROR("unknown exception in ParseQuoteCfgFiles.");
    }
}

void ConfigData::ParseTransferPolicy(ptree &pt)
{
    try
    {
        transfer_policy_.filter_duplicate_flag = true;
        // transfer policy
        transfer_policy_.filter_duplicate_flag = pt.get<bool>("matketdata_config.transfer_policy.filter");
    }
    catch (boost::property_tree::ptree_bad_path &e)
    {
        MY_LOG_INFO("ParseTransferPolicy, info: %s", e.what());
    }
    catch (...)
    {
        MY_LOG_ERROR("unknown exception in ParseTransferPolicy.");
    }

    // show configuration
    if (transfer_policy_.filter_duplicate_flag)
    {
        MY_LOG_INFO("filter duplicate quote datas.");
    }
    else
    {
        MY_LOG_INFO("doesn't filter duplicate quote datas");
    }
}
