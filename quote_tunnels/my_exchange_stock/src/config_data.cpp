#include "config_data.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "my_exchange_utility.h"

MYExConfigData::MYExConfigData(struct my_xchg_cfg& xchg_cfg/*, const std::string &config_file*/)
/*    : cfg_file_(config_file)*/
{
    position_policy_.init_pos_at_start = xchg_cfg.init_pos_at_start;
    position_policy_.model_ctrl_oc = xchg_cfg.st_ctrl_oc;
    position_policy_.change_oc_flag = xchg_cfg.change_oc_flag;
    //LoadCfg();
    LoadModelCfg();
}

mxml_node_t *MYExConfigData::LoadFileInTree(const std::string &file_name)
{
    mxml_node_t *tree = NULL;
    try
    {
        // (cannot open file, parse error), an exception is thrown.
        EX_LOG_INFO("read config file: %s", file_name.c_str());

        int fd;

        if (access(file_name.c_str(), F_OK) != 0)
        {
            EX_LOG_INFO("file [%s] can't be access.", file_name.c_str());
            return NULL;
        }

        if ((fd = open(file_name.c_str(), O_RDONLY)) < 0)
        {
            EX_LOG_INFO("open file [%s] failed.", file_name.c_str());
            return NULL;
        }

        tree = mxmlLoadFd(NULL, fd, MXML_OPAQUE_CALLBACK);
        close(fd);
    }
    catch (std::exception &ex)
    {
        EX_LOG_ERROR("parse config file fail, file: %s; error: %s", file_name.c_str(), ex.what());
    }
    catch (...)
    {
        EX_LOG_ERROR("parse config file fail, file: %s", file_name.c_str());
    }

    return tree;
}

std::string MYExConfigData::GetOneNodeStringValue(mxml_node_t *node, const std::string &sub_node_name)
{
    const char *v = NULL;
    mxml_node_t *sub_node = mxmlFindPath(node, sub_node_name.c_str());
    if (sub_node == NULL)
    {
        EX_LOG_INFO("can't get node <%s>", sub_node_name.c_str());
        return "";
    }

    if ((v = mxmlGetOpaque(sub_node)) == NULL)
    {
        EX_LOG_INFO("empty in node <%s>", sub_node_name.c_str());
        return "";
    }

    return v;
}

void MYExConfigData::LoadCfg()
{
    mxml_node_t *tree = LoadFileInTree(cfg_file_);

    if (tree)
    {
        try
        {
            // get root
            mxml_node_t *root = mxmlFindElement(tree, tree, "root", NULL, NULL, MXML_DESCEND);
            if (root == NULL)
            {
                throw std::bad_exception();
            }

            // tunnels configuration
            EX_LOG_INFO("parse tunnel configuration.");
            ParseTunnelCfg(root);

            // match settings
            EX_LOG_INFO("parse match settings.");
            ParseMatchConfigure(root);

            // position policy
            EX_LOG_INFO("parse position policy.");
            ParsePositionPolicy(root);

            // multiply coefficient
            EX_LOG_INFO("parse multiply coefficient of commodity.");
            ParseMultiplyCoefficient(root);

            // tick price
            EX_LOG_INFO("parse tick price of commodity.");
            ParseTickPrice(root);
        }
        catch (std::exception &ex)
        {
            EX_LOG_ERROR("parse config file fail.");
        }
        catch (...)
        {
            EX_LOG_ERROR("parse config file fail.");
        }

        mxmlDelete(tree);
    }
}

void MYExConfigData::LoadModelCfg()
{
    mxml_node_t *tree = LoadFileInTree("trasev.config");

    if (tree)
    {
        try
        {
            // get root
            mxml_node_t *root = mxmlFindElement(tree, tree, "MyExchange", NULL, NULL, MXML_DESCEND);
            if (root == NULL)
            {
                throw std::bad_exception();
            }

            const char *id = NULL;
            const char *n = NULL;
            const char *mp = NULL;
            int model_id = 0;
            mxml_node_t *st_list = mxmlFindPath(root, "strategies");
            mxml_node_t *st_node = mxmlFindElement(st_list, root, "strategy", NULL, NULL, MXML_DESCEND);
            while (st_node)
            {
                if (st_node->type == MXML_ELEMENT)
                {
                    model_id = 0;
                    id = mxmlElementGetAttr(st_node, "id");
                    if (id)
                    {
                        model_id = atoi(id);
                        if (model_id <= 0)
                        {
                            EX_LOG_INFO("read id error (%s) in <strategies/strategy>", id);
                        }
                    }
                    else
                    {
                        EX_LOG_INFO("read id failed in  <strategies/strategy>");
                    }

                    if (model_id > 0)
                    {
                        mxml_node_t *st_symbol = mxmlFindElement(st_node, st_list, "symbol", NULL, NULL, MXML_DESCEND);

                        while (st_symbol)
                        {
                            if (st_symbol->type == MXML_ELEMENT)
                            {
                                n = mxmlElementGetAttr(st_symbol, "name");
                                mp = mxmlElementGetAttr(st_symbol, "max_pos");
                                if (n && mp && strlen(n) > 0)
                                {
                                    int max_pos = atoi(mp);

                                    if (position_policy_.upper_limit_of_model.find(model_id)
                                        == position_policy_.upper_limit_of_model.end())
                                    {
                                        UpperLimitOfCode lc;
                                        pair<std::string, int> apair;
                                        pair<int, UpperLimitOfCode> bpair;
                                        apair.first = n;
                                        apair.second = max_pos;
                                        lc.insert(apair);
                                        //lc.insert(std::make_pair(n, max_pos));

                                        bpair.first = model_id;
                                        bpair.second = lc;
                                        position_policy_.upper_limit_of_model.insert(bpair);
                                    }
                                    else
                                    {
                                        position_policy_.upper_limit_of_model.find(model_id)->second.insert(
                                            std::make_pair(n, max_pos));
                                    }
                                }
                                else
                                {
                                    EX_LOG_INFO("read name and max_pos failed in  <strategies/strategy/symbol>");
                                }
                            }
                            st_symbol = mxmlGetNextSibling(st_symbol);
                        }
                    }
                }
                st_node = mxmlGetNextSibling(st_node);
            }

            // show max pos configure
            for (UpperLimitOfModel::value_type &vm : position_policy_.upper_limit_of_model)
            {
                for (UpperLimitOfCode::value_type &vc : vm.second)
                {
                    EX_LOG_INFO("upper limit of model %d (%s): %d", vm.first, vc.first.c_str(), vc.second);
                }
            }

        }
        catch (std::exception &ex)
        {
            EX_LOG_ERROR("parse my_capital.config failed.");
        }
        catch (...)
        {
            EX_LOG_ERROR("parse my_capital.config failed.");
        }

        mxmlDelete(tree);
    }
}

void MYExConfigData::ParseTunnelCfg(mxml_node_t* root)
{
    try
    {
        const char *v = NULL;

        // subscribe
        mxml_node_t *tunnel_cfg_list = mxmlFindPath(root, "tunnel_cfg_list");
        mxml_node_t *cfg_file_name = mxmlFindElement(tunnel_cfg_list, root, "file_name", NULL, NULL, MXML_DESCEND);
        while (cfg_file_name)
        {
            if (cfg_file_name->type == MXML_ELEMENT)
            {
                v = mxmlGetOpaque(cfg_file_name);
                if (v)
                {
                    tunnel_cfg_files_.push_back(v);
                }
                else
                {
                    EX_LOG_INFO("empty in  <tunnel_cfg_list/file_name>");
                }
            }
            cfg_file_name = mxmlGetNextSibling(cfg_file_name);

        }
        if (tunnel_cfg_files_.empty())
        {
            EX_LOG_INFO("empty in node <tunnel_cfg_list/file_name>");
        }

        // show config information
        for (const std::string &v : tunnel_cfg_files_)
        {
            EX_LOG_INFO("tunnel configure file: %s", v.c_str());
        }
    }
    catch (std::exception &ex)
    {
        EX_LOG_INFO("ParseTunnelCfg error, info: %s", ex.what());
    }
    catch (...)
    {
        EX_LOG_ERROR("unknown exception in ParseTunnelCfg.");
    }
}

void MYExConfigData::ParseMatchConfigure(mxml_node_t* root)
{
    try
    {
        match_configure_.inner_match_radio = atof(GetOneNodeStringValue(root, "match_config/inner_match_radio").c_str());

        // show config information
        EX_LOG_INFO("match_configure_.inner_match_radio: %06f", match_configure_.inner_match_radio);
    }
    catch (std::exception &ex)
    {
        EX_LOG_INFO("ParseMatchConfigure error, info: %s", ex.what());
    }
    catch (...)
    {
        EX_LOG_ERROR("unknown exception in ParseMatchConfigure.");
    }
}

void MYExConfigData::ParsePositionPolicy(mxml_node_t* root)
{
    position_policy_.default_upper_limit = 0;
    position_policy_.init_pos_at_start = false;
    position_policy_.model_ctrl_oc = false;
    position_policy_.change_oc_flag = false;
    try
    {
        position_policy_.init_pos_at_start = atoi(GetOneNodeStringValue(root, "position_policy/init_pos_at_start").c_str()) != 0;
        position_policy_.model_ctrl_oc = atoi(GetOneNodeStringValue(root, "position_policy/model_ctrl_oc").c_str()) != 0;
        position_policy_.change_oc_flag = atoi(GetOneNodeStringValue(root, "position_policy/change_oc_flag").c_str()) != 0;

        // parse max position from my_capital.config
//        const char *v = NULL;
//
//        // subscribe
//        mxml_node_t *pos_policy = mxmlFindPath(root, "position_policy");
//        mxml_node_t *pos_upper_limit = mxmlFindElement(pos_policy, root, "pos_upper_limit", NULL, NULL, MXML_DESCEND);
//        while (pos_upper_limit)
//        {
//            if (pos_upper_limit->type == MXML_ELEMENT)
//            {
//                v = mxmlGetOpaque(pos_upper_limit);
//                if (v && strlen(v) > 3)
//                {
//                    std::string tmp(v);
//                    std::vector<std::string> fields;
//                    MYStringSplit(tmp, fields, ':');
//
//                    if (fields.size() < 2)
//                    {
//                        EX_LOG_ERROR("pos_upper_limit: %s", v);
//                    }
//                    else if (fields.size() == 2)
//                    {
//                        int limit = atoi(fields[1].c_str());
//                        if (limit >= 0 && fields[0] == std::string("default"))
//                        {
//                            position_policy_.default_upper_limit = limit;
//                        }
//                        else
//                        {
//                            EX_LOG_ERROR("pos_upper_limit, item parse error, cfg_item: %s", v);
//                        }
//                    }
//                    else if (fields.size() == 3)
//                    {
//                        int model_id = atoi(fields[0].c_str());
//                        int limit = atoi(fields[2].c_str());
//                        if (model_id > 0 && limit >= 0 && fields[1] == std::string("default"))
//                        {
//                            position_policy_.default_upper_limit_of_model.insert(std::make_pair(model_id, limit));
//                        }
//                        else if (model_id > 0 && limit >= 0 && fields[1].length() > 0)
//                        {
//                            UpperLimitOfCode lc;
//                            lc.insert(std::make_pair(fields[1], limit));
//                            position_policy_.upper_limit_of_model.insert(std::make_pair(model_id, lc));
//                        }
//                        else
//                        {
//                            EX_LOG_ERROR("pos_upper_limit, item parse error, cfg_item: %s", v);
//                        }
//                    }
//                    else
//                    {
//                        EX_LOG_ERROR("pos_upper_limit, item parse error, cfg_item: %s", v);
//                    }
//                }
//                else
//                {
//                    EX_LOG_INFO("empty in  <position_policy/pos_upper_limit>");
//                }
//            }
//            pos_upper_limit = mxmlGetNextSibling(pos_upper_limit);
//
//        }
//        if (position_policy_.upper_limit_of_model.empty())
//        {
//            EX_LOG_INFO("empty in node <position_policy/pos_upper_limit>");
//        }


        // show config information
        EX_LOG_INFO("init_pos_at_start: %d", position_policy_.init_pos_at_start);
        EX_LOG_INFO("model_ctrl_oc: %d", position_policy_.model_ctrl_oc);
        EX_LOG_INFO("change_oc_flag: %d", position_policy_.change_oc_flag);
    }
    catch (std::exception &ex)
    {
        EX_LOG_INFO("ParsePositionPolicy error, info: %s", ex.what());
    }
    catch (...)
    {
        EX_LOG_ERROR("unknown exception in ParsePositionPolicy.");
    }
}

void MYExConfigData::ParseMultiplyCoefficient(mxml_node_t* root)
{
    try
    {
        const char *v = NULL;

        mxml_node_t *cfg_node = mxmlFindPath(root, "multiply_coefficent_of_commodity");
        mxml_node_t *item_node = mxmlFindElement(cfg_node, root, "item", NULL, NULL, MXML_DESCEND);
        while (item_node)
        {
            if (item_node->type == MXML_ELEMENT)
            {
                v = mxmlGetOpaque(item_node);
                if (v && strlen(v) > 3)
                {
                    std::string tmp(v);
                    std::size_t split_pos = tmp.find(":");
                    if ((split_pos == std::string::npos) || (split_pos + 1 >= tmp.length()))
                    {
                        EX_LOG_ERROR("multiply_coefficent_of_commodity-item: %s", v);
                    }
                    else
                    {
                        std::string code_t = tmp.substr(0, split_pos);
                        std::string limit_t = tmp.substr(split_pos + 1);
                        int coe = atoi(limit_t.c_str());
                        if (coe < 0)
                        {
                            EX_LOG_ERROR("multiply_coefficent_of_commodity, coefficent is less than 0, cfg_item: %s", v);
                        }
                        else
                        {
                            multiply_coefficent_of_commodity_.insert(std::make_pair(code_t, coe));
                        }
                    }
                }
                else
                {
                    EX_LOG_INFO("empty in <multiply_coefficent_of_commodity/item>");
                }
            }
            item_node = mxmlGetNextSibling(item_node);

        }
        if (multiply_coefficent_of_commodity_.empty())
        {
            EX_LOG_INFO("empty in node <multiply_coefficent_of_commodity/item>");
        }

        // show config information
        for (MultiplyCoefficientOfCommodity::value_type &v : multiply_coefficent_of_commodity_)
        {
            EX_LOG_INFO("multiply coefficent of %s = %d", v.first.c_str(), v.second);
        }
    }
    catch (std::exception &ex)
    {
        EX_LOG_INFO("ParseMultiplyCoefficient error, info: %s", ex.what());
    }
    catch (...)
    {
        EX_LOG_ERROR("unknown exception in ParseMultiplyCoefficient.");
    }
}

void MYExConfigData::ParseTickPrice(mxml_node_t* root)
{
    try
    {
        const char *v = NULL;

        mxml_node_t *cfg_node = mxmlFindPath(root, "tick_price_of_commodity");
        mxml_node_t *item_node = mxmlFindElement(cfg_node, root, "item", NULL, NULL, MXML_DESCEND);
        while (item_node)
        {
            if (item_node->type == MXML_ELEMENT)
            {
                v = mxmlGetOpaque(item_node);
                if (v && strlen(v) > 3)
                {
                    std::string tmp(v);
                    std::size_t split_pos = tmp.find(":");
                    if ((split_pos == std::string::npos) || (split_pos + 1 >= tmp.length()))
                    {
                        EX_LOG_ERROR("tick_price_of_commodity-item: %s", v);
                    }
                    else
                    {
                        std::string code_t = tmp.substr(0, split_pos);
                        std::string limit_t = tmp.substr(split_pos + 1);
                        double tp = atof(limit_t.c_str());
                        if (tp < 0)
                        {
                            EX_LOG_ERROR("tick_price_of_commodity, tick price is less than 0, cfg_item: %s", v);
                        }
                        else
                        {
                            tick_price_of_commodity_.insert(std::make_pair(code_t, tp));
                        }
                    }
                }
                else
                {
                    EX_LOG_INFO("empty in <tick_price_of_commodity/item>");
                }
            }
            item_node = mxmlGetNextSibling(item_node);

        }
        if (tick_price_of_commodity_.empty())
        {
            EX_LOG_INFO("empty in node <tick_price_of_commodity/item>");
        }

        // show config information
        for (TickPriceOfCommodity::value_type &v : tick_price_of_commodity_)
        {
            EX_LOG_INFO("tick price of %s = %.4f", v.first.c_str(), v.second);
        }
    }
    catch (std::exception &ex)
    {
        EX_LOG_INFO("ParseTickPrice error, info: %s", ex.what());
    }
    catch (...)
    {
        EX_LOG_ERROR("unknown exception in ParseTickPrice.");
    }
}

double MYExConfigData::GetTickPriceOfContract(const std::string &c)
{
    std::size_t commodity_len = 0;
    while (!isdigit(c[commodity_len]) && commodity_len < c.size())
    {
        ++commodity_len;
    }
    std::string commodity = c.substr(0, commodity_len);
    TickPriceOfCommodity::const_iterator cit = tick_price_of_commodity_.find(commodity);
    if (cit != tick_price_of_commodity_.end())
    {
        return cit->second;
    }

    EX_LOG_WARN("can't find tick price of contract: %s, use 1.0 for default", c.c_str());
    return 1.0;
}
