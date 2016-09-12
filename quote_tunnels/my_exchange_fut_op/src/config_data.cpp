#include "config_data.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "my_exchange_utility.h"

MYExConfigData::MYExConfigData(struct my_xchg_cfg& xchg_cfg)
{
    position_policy_.init_pos_at_start = xchg_cfg.init_pos_at_start;
    position_policy_.model_ctrl_oc = xchg_cfg.st_ctrl_oc;
    position_policy_.change_oc_flag = xchg_cfg.change_oc_flag;
    position_policy_.init_pos_from_ev = (xchg_cfg.init_pos_from_ev == 1);

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

            const char *mod_id_str = NULL;
            const char *contract_name = NULL;
            const char *max_pos_str = NULL;
            int model_id = 0;
            mxml_node_t *st_list = mxmlFindPath(root, "strategies");
            mxml_node_t *st_node = mxmlFindElement(st_list, root, "strategy", NULL, NULL, MXML_DESCEND);
            while (st_node)
            {
                if (st_node->type == MXML_ELEMENT)
                {
                    model_id = 0;
                    mod_id_str = mxmlElementGetAttr(st_node, "id");
                    if (mod_id_str)
                    {
                        model_id = atoi(mod_id_str);
                        if (model_id <= 0)
                        {
                            EX_LOG_INFO("read id error (%s) in <strategies/strategy>", mod_id_str);
                        }
                    }
                    else
                    {
                        EX_LOG_INFO("read id failed in  <strategies/strategy>");
                    }

                    if (model_id > 0)
                    {
                        // get ev file name  ev_name="ev/ev.txt"
                        const char *ev_file_name_str = mxmlElementGetAttr(st_node, "ev_name");
                        if (ev_file_name_str && strlen(ev_file_name_str) > 1)
                        {
                            position_policy_.ev_file_name_of_model.insert(std::make_pair(model_id, std::string(ev_file_name_str)));
                            EX_LOG_INFO("model(%d) config ev file name(%s)", model_id, ev_file_name_str);
                        }

                        mxml_node_t *st_symbol = mxmlFindElement(st_node, st_list, "symbol", NULL, NULL, MXML_DESCEND);
                        int symbol_count = 0;
                        int model_default_max_pos = 0;
                        std::string first_symbol;

                        while (st_symbol)
                        {
                            if (st_symbol->type == MXML_ELEMENT)
                            {
                                contract_name = mxmlElementGetAttr(st_symbol, "name");
                                max_pos_str = mxmlElementGetAttr(st_symbol, "max_pos");
                                if (contract_name && max_pos_str && strlen(contract_name) > 0)
                                {
                                    int max_pos = atoi(max_pos_str);
				    // added by wangying on 20160721,for testing strategy "bai dan"
				    max_pos *= 10; 
#ifdef ASYNC_CANCEL
                                    max_pos *= 3; // upper limit of position will rise to 3 times when cancel failed.
#endif
                                    UpperLimitOfModel::iterator it = position_policy_.upper_limit_of_model.find(model_id);
                                    if (it == position_policy_.upper_limit_of_model.end())
                                    {
                                        it = position_policy_.upper_limit_of_model.insert(std::make_pair(model_id, UpperLimitOfCode())).first;
                                    }

                                    it->second.insert(std::make_pair(std::string(contract_name), max_pos));
                                    ++symbol_count;
                                    if (symbol_count == 1)
                                    {
                                        model_default_max_pos = max_pos;
                                        first_symbol = contract_name;
                                    }
                                }
                                else
                                {
                                    EX_LOG_INFO("read name and max_pos failed in  <strategies/strategy/symbol>");
                                }
                            }
                            st_symbol = mxmlGetNextSibling(st_symbol);
                        }

                        // set max pos of first symbol as default max pos for the model
                        if (symbol_count > 0)
                        {
                            position_policy_.default_upper_limit_of_model.insert(std::make_pair(model_id, model_default_max_pos));
                            EX_LOG_INFO("config max_pos for (%d) symbols, set model(%d) default max pos to(%d) as first symbol(%s)",
                                symbol_count, model_id, model_default_max_pos, first_symbol.c_str());
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

