#include "config_data.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "trade_data_type.h"
#include "tunnel_cmn_utility.h"
#include "my_cmn_crypto.h"

using namespace my_cmn;

mxml_node_t *LoadFileInTree(const std::string &file_name)
{
    mxml_node_t *tree = NULL;
    try
    {
        // (cannot open file, parse error), an exception is thrown.
        TNL_LOG_INFO("read config file: %s", file_name.c_str());

        int fd;

        if (access(file_name.c_str(), F_OK) != 0)
        {
            TNL_LOG_INFO("file [%s] can't be access.", file_name.c_str());
            return NULL;
        }

        if ((fd = open(file_name.c_str(), O_RDONLY)) < 0)
        {
            TNL_LOG_INFO("open file [%s] failed.", file_name.c_str());
            return NULL;
        }

        tree = mxmlLoadFd(NULL, fd, MXML_OPAQUE_CALLBACK);
        close(fd);
    }
    catch (std::exception &ex)
    {
        TNL_LOG_ERROR("parse config file fail, file: %s; error: %s", file_name.c_str(), ex.what());
    }
    catch (...)
    {
        TNL_LOG_ERROR("parse config file fail, file: %s", file_name.c_str());
    }

    return tree;
}

std::string GetOneNodeStringValue(mxml_node_t *node, const std::string &sub_node_name)
{
    const char *v = NULL;
    mxml_node_t *sub_node = mxmlFindPath(node, sub_node_name.c_str());
    if (sub_node == NULL)
    {
        TNL_LOG_INFO("can't get node <%s>", sub_node_name.c_str());
        return "";
    }

    if ((v = mxmlGetOpaque(sub_node)) == NULL)
    {
        TNL_LOG_INFO("empty in node <%s>", sub_node_name.c_str());
        return "";
    }

    return v;
}

int TunnelConfigData::GetStandardErrorNo(int outer_error_no)
{
    OuterErrorNoToStandardErroNoCit cit = outer_error_to_standard_error_.find(outer_error_no);
    if (cit != outer_error_to_standard_error_.end())
    {
        return cit->second;
    }

    return TUNNEL_CONST_VAR::UNKNOWN_ERROR_NO;
}

void TunnelConfigData::Load(const std::string &cfg_file)
{
    LoadTunnelCfg(cfg_file);

    int interface_type = app_cfg_.provider_type;
    std::string error_cfg_file;
    if (TunnelProviderType::PROVIDERTYPE_CTP == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_CTP;
    }
    else if (TunnelProviderType::PROVIDERTYPE_DCE == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_DCE;
    }
    else if (TunnelProviderType::PROVIDERTYPE_FEMAS == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_FEMAS;
    }
    else if (TunnelProviderType::PROVIDERTYPE_ZEUSING == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_ZEUSING;
    }
    else if (TunnelProviderType::PROVIDERTYPE_KSG == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_KSG;
    }
    else if (TunnelProviderType::PROVIDERTYPE_CITICS_HS == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_CITICS_HS;
    }
    else if (TunnelProviderType::PROVIDERTYPE_ESUNNY == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_ESUNNY;
    }
    else if (TunnelProviderType::PROVIDERTYPE_XELE == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_XELE;
    }
    else if (TunnelProviderType::PROVIDERTYPE_REM == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_REM;
    }
    else if (TunnelProviderType::PROVIDERTYPE_IB_API == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_IB_API;
    }
    else if (TunnelProviderType::PROVIDERTYPE_LTS == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_LTS;
    }
    else if (TunnelProviderType::PROVIDERTYPE_SGIT == interface_type)
    {
        error_cfg_file = TUNNEL_CONST_VAR::ERR_CFG_FILE_SGIT;
    }
    else
    {
        TNL_LOG_ERROR("tunnel type(%d) can't recognized, no error code table for this tunnel!", interface_type);
    }

    if (!error_cfg_file.empty())
    {
        LoadErrCfg(error_cfg_file);
    }
}

void TunnelConfigData::LoadTunnelCfg(const std::string &cfg_file)
{
    mxml_node_t *tree = LoadFileInTree(cfg_file);

    if (tree)
    {
        try
        {
            // get root
            mxml_node_t *root = mxmlFindElement(tree, tree, "my_trade_tunnel", NULL, NULL, MXML_DESCEND);
            if (root == NULL)
            {
                throw std::bad_exception();
            }

            // app configuration
            app_cfg_.program_title = GetOneNodeStringValue(root, "program_title");
            app_cfg_.provider_type = atoi(GetOneNodeStringValue(root, "provider_type").c_str());
            app_cfg_.orderref_prefix_id = atoi(GetOneNodeStringValue(root, "orderref_prefix").c_str());
            app_cfg_.share_memory_key = atoi(GetOneNodeStringValue(root, "share_memory_key").c_str());

            // show configuration
            TNL_LOG_INFO("program_title: %s", app_cfg_.program_title.c_str());
            TNL_LOG_INFO("provider_type: %d", app_cfg_.provider_type);
            TNL_LOG_INFO("orderref_prefix_id: %d", app_cfg_.orderref_prefix_id);
            TNL_LOG_INFO("share_memory_key: %d", app_cfg_.share_memory_key);

            // parse login parameters
            TNL_LOG_INFO("parse trade login configuration.");
            mxml_node_t *trade_login_node = mxmlFindElement(root, tree, "login", NULL, NULL, MXML_DESCEND);
            if (trade_login_node == NULL)
            {
                TNL_LOG_ERROR("can't get login node.");
                throw std::bad_exception();
            }
            ParseLoginConfig(trade_login_node);

            // parse compliance check parameters
            TNL_LOG_INFO("parse compliance check configuration.");
            mxml_node_t *cc_node = mxmlFindElement(root, tree, "compliance_check_param", NULL, NULL, MXML_DESCEND);
            if (cc_node == NULL)
            {
                TNL_LOG_ERROR("can't get compliance check node.");
                throw std::bad_exception();
            }
            ParseComplianceCheckParam(cc_node, root);

            // parse initial policy parameters
            TNL_LOG_INFO("parse initial policy configuration.");
            mxml_node_t *init_policy = mxmlFindElement(root, tree, "initial_policy", NULL, NULL, MXML_DESCEND);
            if (init_policy == NULL)
            {
                TNL_LOG_ERROR("can't get initial policy node.");
                throw std::bad_exception();
            }
            ParseInitialPolicy(init_policy);
        }
        catch (std::exception &ex)
        {
            TNL_LOG_ERROR("parse config file(%s) failed.", cfg_file.c_str());
        }
        catch (...)
        {
            TNL_LOG_ERROR("parse config file(%s) failed.", cfg_file.c_str());
        }

        mxmlDelete(tree);
    }
}

void TunnelConfigData::ParseLoginConfig(mxml_node_t* login_node)
{
    try
    {
        const char *v = NULL;

        // front address
        mxml_node_t *front_addr = mxmlFindPath(login_node, "serverlist");
        mxml_node_t *addr_t = mxmlFindElement(front_addr, login_node, "server", NULL, NULL, MXML_DESCEND);
        while (addr_t)
        {
            if (addr_t->type == MXML_ELEMENT)
            {
                if ((v = mxmlGetOpaque(addr_t)) != NULL)
                {
                    logon_config_.front_addrs.push_back(v);
                }
                else
                {
                    TNL_LOG_INFO("empty in  <serverlist/addr>");
                }
            }
            addr_t = mxmlGetNextSibling(addr_t);
        }

        logon_config_.brokerid = GetOneNodeStringValue(login_node, "brokerid");
        logon_config_.investorid = GetOneNodeStringValue(login_node, "investorid");
        logon_config_.clientid = GetOneNodeStringValue(login_node, "userid");

        logon_config_.password = GetOneNodeStringValue(login_node, "password");
        logon_config_.password = MYDecrypt(logon_config_.password);

        logon_config_.exch_code = GetOneNodeStringValue(login_node, "exchangecode");
        logon_config_.UserProductInfo = GetOneNodeStringValue(login_node, "UserProductInfo");
        logon_config_.AuthCode = GetOneNodeStringValue(login_node, "AuthCode");
        logon_config_.ParticipantID = GetOneNodeStringValue(login_node, "ParticipantID");
        logon_config_.BusinessUnit = GetOneNodeStringValue(login_node, "BusinessUnit");
        logon_config_.InterfaceProductInfo = GetOneNodeStringValue(login_node, "InterfaceProductInfo");
        logon_config_.ProtocolInfo = GetOneNodeStringValue(login_node, "ProtocolInfo");
        logon_config_.DataCenterID = atoi(GetOneNodeStringValue(login_node, "DataCenterID").c_str());
      
        //ADD BY hwg 20160419 for citics tunnel
        logon_config_.client_name = GetOneNodeStringValue(login_node, "client_name").c_str();
        logon_config_.local_ip = GetOneNodeStringValue(login_node, "local_ip").c_str();
        logon_config_.mac_code = GetOneNodeStringValue(login_node, "mac_code").c_str();
        logon_config_.op_entrust_way = GetOneNodeStringValue(login_node, "op_entrust_way").c_str();
        logon_config_.password_type = GetOneNodeStringValue(login_node, "password_type").c_str();
       //ADD END
        std::string client_type = GetOneNodeStringValue(login_node, "ClientType");
        if (client_type.length() == 1)
        {
            logon_config_.ClientType = client_type[0];
        }
        else
        {
            logon_config_.ClientType = '-';
        }
        logon_config_.quote_front_addr = GetOneNodeStringValue(login_node, "quote_front_addr");

        logon_config_.mac_address = GetOneNodeStringValue(login_node, "mac_address");
        // show config information
        if (logon_config_.front_addrs.empty())
        {
            TNL_LOG_INFO("server address is empty");
        }
        for (const std::string &v : logon_config_.front_addrs)
        {
            TNL_LOG_INFO("server address: %s", v.c_str());
        }

        TNL_LOG_INFO("logon_config.brokerid: %s", logon_config_.brokerid.c_str());
        TNL_LOG_INFO("logon_config.investorid: %s", logon_config_.investorid.c_str());
        TNL_LOG_INFO("logon_config.clientid: %s", logon_config_.clientid.c_str());
        TNL_LOG_INFO("logon_config.exch_code: %s", logon_config_.exch_code.c_str());
        TNL_LOG_INFO("logon_config.UserProductInfo: %s", logon_config_.UserProductInfo.c_str());
        TNL_LOG_INFO("logon_config.AuthCode: %s", logon_config_.AuthCode.c_str());
        TNL_LOG_INFO("logon_config.ParticipantID: %s", logon_config_.ParticipantID.c_str());
        TNL_LOG_INFO("logon_config.BusinessUnit: %s", logon_config_.BusinessUnit.c_str());
        TNL_LOG_INFO("logon_config.InterfaceProductInfo: %s", logon_config_.InterfaceProductInfo.c_str());
        TNL_LOG_INFO("logon_config.ProtocolInfo: %s", logon_config_.ProtocolInfo.c_str());
        TNL_LOG_INFO("logon_config.DataCenterID: %d", logon_config_.DataCenterID);
        TNL_LOG_INFO("logon_config.ClientType: %c", logon_config_.ClientType);
        TNL_LOG_INFO("logon_config.quote_front_addr: %s", logon_config_.quote_front_addr.c_str());
        TNL_LOG_INFO("logon_config.mac_address: %s", logon_config_.mac_address.c_str());
        
        TNL_LOG_INFO("logon_config.client_name: %s", logon_config_.client_name.c_str());
        TNL_LOG_INFO("logon_config.local_ip: %s", logon_config_.local_ip.c_str());
        TNL_LOG_INFO("logon_config.mac_code: %s", logon_config_.mac_code.c_str());
        TNL_LOG_INFO("logon_config.op_entrust_way: %s", logon_config_.op_entrust_way.c_str());
        TNL_LOG_INFO("logon_config.password_type: %s", logon_config_.password_type.c_str());
        
    }
    catch (std::exception &ex)
    {
        TNL_LOG_ERROR("ParseLogonConfig error, info: %s", ex.what());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in ParseLogonConfig.");
    }
}

void TunnelConfigData::ParseComplianceCheckParam(mxml_node_t* cc_node, mxml_node_t *top)
{
    try
    {
        std::string selftrade_check_switch, cancel_time_check_switch, open_volume_check_switch;

        mxml_node_t *selftrade_check_node = mxmlFindElement(cc_node, top, "self_trade_check", NULL, NULL, MXML_DESCEND);
        if (selftrade_check_node != NULL)
        {
            selftrade_check_switch = GetOneNodeStringValue(selftrade_check_node, "switch");
        }
        else
        {
            TNL_LOG_ERROR("can't get self_trade_check node.");
        }

        mxml_node_t *max_cancel_times = mxmlFindElement(cc_node, top, "cancel_time_check", NULL, NULL, MXML_DESCEND);
        if (max_cancel_times != NULL)
        {
            compliance_check_param_.cancel_warn_threshold = atoi(GetOneNodeStringValue(max_cancel_times, "warn_threshold").c_str());
            compliance_check_param_.cancel_upper_limit = atoi(GetOneNodeStringValue(max_cancel_times, "upper_limit").c_str());
            cancel_time_check_switch = GetOneNodeStringValue(max_cancel_times, "switch");
            std::string init_cancel_time = GetOneNodeStringValue(max_cancel_times, "init_cancel_times_at_start");
            compliance_check_param_.init_cancel_times_at_start = 1;
            if (!init_cancel_time.empty())
            {
                compliance_check_param_.init_cancel_times_at_start = atoi(init_cancel_time.c_str());
            }
        }
        else
        {
            TNL_LOG_ERROR("can't get cancel_time_check node.");
        }

        mxml_node_t *max_open_orders = mxmlFindElement(cc_node, top, "open_volume_check", NULL, NULL, MXML_DESCEND);
        if (max_open_orders != NULL)
        {
            compliance_check_param_.max_open_of_speculate = atoi(GetOneNodeStringValue(max_open_orders, "speculate").c_str());
            compliance_check_param_.max_open_of_arbitrage = atoi(GetOneNodeStringValue(max_open_orders, "arbitrage").c_str());
            compliance_check_param_.max_open_of_total = atoi(GetOneNodeStringValue(max_open_orders, "total").c_str());
            open_volume_check_switch = GetOneNodeStringValue(max_open_orders, "switch");
        }
        else
        {
            TNL_LOG_ERROR("can't get open_volume_check node.");
        }

        // 从左开始，第1位:自成交；第2位:撤单次数；第3位:最大开仓数
        //compliance_check_param_.switch_mask = GetOneNodeStringValue(cc_node, "switch_mask");
        if (selftrade_check_switch.size() == 1)
        {
            compliance_check_param_.switch_mask.append(selftrade_check_switch);
        }
        else
        {
            compliance_check_param_.switch_mask.append("0");
        }
        if (cancel_time_check_switch.size() == 1)
        {
            compliance_check_param_.switch_mask.append(cancel_time_check_switch);
        }
        else
        {
            compliance_check_param_.switch_mask.append("0");
        }
        if (open_volume_check_switch.size() == 1)
        {
            compliance_check_param_.switch_mask.append(open_volume_check_switch);
        }
        else
        {
            compliance_check_param_.switch_mask.append("0");
        }

        if (cancel_time_check_switch == "0" && compliance_check_param_.init_cancel_times_at_start == 1)
        {
            compliance_check_param_.init_cancel_times_at_start = 0;
            TNL_LOG_WARN("don't need init cancel times if don't check cancel times.");
        }

        // show configuration
        TNL_LOG_INFO("switch_mask: %s", compliance_check_param_.switch_mask.c_str());
        TNL_LOG_INFO("init_cancel_times_at_start:%d ", compliance_check_param_.init_cancel_times_at_start);
        TNL_LOG_INFO("max_open_of_speculate: %d", compliance_check_param_.max_open_of_speculate);
        TNL_LOG_INFO("max_open_of_arbitrage: %d", compliance_check_param_.max_open_of_arbitrage);
        TNL_LOG_INFO("max_open_of_total: %d", compliance_check_param_.max_open_of_total);
        TNL_LOG_INFO("cancel_warn_threshold: %d", compliance_check_param_.cancel_warn_threshold);
        TNL_LOG_INFO("cancel_upper_limit: %d", compliance_check_param_.cancel_upper_limit);
    }
    catch (std::exception &ex)
    {
        TNL_LOG_ERROR("ParseComplianceCheckParam error, info: %s", ex.what());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in ParseComplianceCheckParam.");
    }
}

void TunnelConfigData::ParseInitialPolicy(mxml_node_t* policy_node)
{
    try
    {
        initial_policy_param_.cancel_orders_at_start =
            (atoi(GetOneNodeStringValue(policy_node, "cancel_orders_at_start").c_str()) != 0);

        // show configuration
        TNL_LOG_INFO("cancel_orders_at_start:%d ", initial_policy_param_.cancel_orders_at_start);

        // added on 20160318
        initial_policy_param_.time_to_query_pos = GetOneNodeStringValue(policy_node, "time_to_query_pos");
        // show configuration
        TNL_LOG_INFO("time_to_query_pos:%s ", initial_policy_param_.time_to_query_pos.c_str());
    }
    catch (std::exception &ex)
    {
        TNL_LOG_ERROR("ParseInitialPolicy error, info: %s", ex.what());
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in ParseInitialPolicy.");
    }
}

void TunnelConfigData::LoadErrCfg(const std::string &cfg_file)
{
    mxml_node_t *tree = LoadFileInTree(cfg_file);

    if (tree)
    {
        try
        {
            // get root
            mxml_node_t *root = mxmlFindElement(tree, tree, "ErrorDef", NULL, NULL, MXML_DESCEND);
            if (root == NULL)
            {
                throw std::bad_exception();
            }

            // ExternErrors
            mxml_node_t *err_map = mxmlFindElement(root, tree, "ExternErrors", NULL, NULL, MXML_DESCEND);
            mxml_node_t *err_item = mxmlFindElement(err_map, root, "ExternError", NULL, NULL, MXML_DESCEND);
            while (err_item)
            {
                if (err_item->type == MXML_ELEMENT)
                {
                    int error_no = atoi(mxmlElementGetAttr(err_item, "ErrorNo"));
                    int standard_error_no = atoi(mxmlElementGetAttr(err_item, "StdErrorNo"));
                    // 有重复，报告
                    if (outer_error_to_standard_error_.find(error_no)
                        != outer_error_to_standard_error_.end())
                    {
                        TNL_LOG_WARN("duplicate outer error number: %d", error_no);
                    }
                    else
                    {
                        TNL_LOG_INFO("tunnel_err_no: %d map to my_err_no %d", error_no, standard_error_no);
                        outer_error_to_standard_error_.insert(std::make_pair(error_no, standard_error_no));
                    }
                }
                err_item = mxmlGetNextSibling(err_item);
            }

            std::list<int> no_fatal_err_no_list;
            known_error_no_ = new char[max_known_error_no_ + 1];
            memset(known_error_no_, 0, sizeof(char) * (max_known_error_no_ + 1));

            mxml_node_t *no_fatal_err_map = mxmlFindElement(root, tree, "NonfatalAPIErrorNos", NULL, NULL, MXML_DESCEND);
            mxml_node_t *no_fatal_err_item = mxmlFindElement(no_fatal_err_map, root, "NonfatalError", NULL, NULL, MXML_DESCEND);
            while (no_fatal_err_item)
            {
            	if (no_fatal_err_item->type == MXML_ELEMENT)
            	{
            		int error_no = atoi(mxmlElementGetAttr(no_fatal_err_item, "ErrorNo"));
            		no_fatal_err_no_list.push_back(error_no);
            	}
            	no_fatal_err_item = mxmlGetNextSibling(no_fatal_err_item);
            }
            no_fatal_err_no_list.sort();
            if (no_fatal_err_no_list.front() <= 0 || no_fatal_err_no_list.back() > 100000)
            {
            	TNL_LOG_ERROR("nonfatal error code from %d to %d, exceed normal range, please check error config file.", no_fatal_err_no_list.front(), no_fatal_err_no_list.back());
            }
            else
            {
            	TNL_LOG_WARN("nonfatal error code from %d to %d", no_fatal_err_no_list.front(), no_fatal_err_no_list.back());
            	max_known_error_no_ = no_fatal_err_no_list.back();
                known_error_no_ = new char[max_known_error_no_ + 1];
                memset(known_error_no_, 0, sizeof(char) * (max_known_error_no_ + 1));
                for (std::list<int>::const_iterator cit = no_fatal_err_no_list.begin(); cit != no_fatal_err_no_list.end(); ++cit)
                {
                    known_error_no_[*cit] = (char) 1;
                }
            }
        }
        catch (std::exception &ex)
        {
            TNL_LOG_ERROR("parse config file(%s) failed.", cfg_file.c_str());
        }
        catch (...)
        {
            TNL_LOG_ERROR("parse config file(%s) failed.", cfg_file.c_str());
        }

        mxmlDelete(tree);
    }
}
