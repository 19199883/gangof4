#ifndef MY_EX_CONFIG_DATA_H_
#define MY_EX_CONFIG_DATA_H_

#include <unordered_map>
#include <string>
#include <vector>

#include <mxml.h>

#include "my_exchange.h"

// 通道的配置（通道id对应通道配置文件）

// MYExchange内部的配置

// relation of Strategy id and tunnel id
// 模型在多个账号间均衡下单，每个账号有最大下单数，可能需要拆单
// 一个账号，可能对应多个物理通道（多个账号间进行均衡，可能不需要多个物理通道了；或者一个账号对应多个物理通道的处理，放在通道内）

typedef std::vector<std::string> TunnelCfgFiles;

struct MYExMatchConfigure
{
    double inner_match_radio;
};

typedef std::unordered_map<std::string, int> UpperLimitOfCode;
typedef std::unordered_map<int, UpperLimitOfCode> UpperLimitOfModel;
typedef std::unordered_map<int, int> DefaultUpperLimitOfModel;
typedef std::unordered_map<std::string, int> MultiplyCoefficientOfCommodity;
typedef std::unordered_map<std::string, double> TickPriceOfCommodity;
typedef std::unordered_map<int, std::string> EvFileNameOfModel;

struct MYExPositionPolicy
{
    bool init_pos_at_start;
    bool model_ctrl_oc;  // model control open/close by itself
    bool change_oc_flag;
    int default_upper_limit;
    DefaultUpperLimitOfModel default_upper_limit_of_model;
    UpperLimitOfModel upper_limit_of_model;
    EvFileNameOfModel ev_file_name_of_model;

    bool init_pos_from_ev;
    bool InitPosByEvfile() const
    {
        return init_pos_from_ev;
    }

    MYExPositionPolicy()
    {
        init_pos_at_start = false;
        model_ctrl_oc = false;
        change_oc_flag = false;
        default_upper_limit = 0; // no limit

        init_pos_from_ev = false;
    }
};

// 配置数据管理
class MYExConfigData
{
public:
    MYExConfigData(struct my_xchg_cfg& xchg_cfg);

    const MYExPositionPolicy & Position_policy() const
    {
        return position_policy_;
    }

private:
    void LoadModelCfg();

    mxml_node_t *LoadFileInTree(const std::string &file_name);
    std::string GetOneNodeStringValue(mxml_node_t *node, const std::string &sub_node_name);

    std::string cfg_file_;

    // config datas
    TunnelCfgFiles tunnel_cfg_files_;

    MYExPositionPolicy position_policy_;
};

#endif
