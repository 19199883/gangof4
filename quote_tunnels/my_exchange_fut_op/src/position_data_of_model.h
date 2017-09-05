#ifndef _MY_EXCHANGE_POSITION_DATA_OF_MODEL_H_
#define  _MY_EXCHANGE_POSITION_DATA_OF_MODEL_H_

#include "my_exchange_datatype_inner.h"
#include <mutex>
#include <unordered_map>
#include <map>
#include "config_data.h"

#include "my_order_hash.h"

struct ModelPositionData
{
    int long_pos;            // 多仓, available position
    int short_pos;           // 空仓, available position
    int freeze_long_pos;     // 冻结多仓
    int freeze_short_pos;    // 冻结空仓
    int pending_long_pos;    // 未成多仓
    int pending_short_pos;   // 未成空仓

    const int max_pos;

    inline int max_possible_long()
    {
        return long_pos + freeze_long_pos + pending_long_pos;
    }
    inline int max_possible_short()
    {
        return short_pos + freeze_short_pos + pending_short_pos;
    }

    ModelPositionData()
        : max_pos(0)
    {
        long_pos = short_pos = freeze_long_pos = freeze_short_pos = pending_long_pos = pending_short_pos = 0;
    }
    ModelPositionData(int pos_upper_limit)
        : max_pos(pos_upper_limit)
    {
        long_pos = short_pos = freeze_long_pos = freeze_short_pos = pending_long_pos = pending_short_pos = 0;
    }
};

typedef std::unordered_map<SerialNoType, int> OrderCloseVolumeMap;
typedef std::unordered_map<SerialNoType, std::pair<int, int> > QuoteCloseVolumeMap;
typedef std::unordered_map<std::string, ModelPositionData> ContractPositionDataMap;
typedef std::unordered_map<int, ContractPositionDataMap> ModelPositionDataMap;

class ModelPositionManager
{
public:
    ModelPositionManager(const MYExConfigData &cfg)
        : cfg_(cfg)
            , m_order_close_vol_hash(1000000)
    {
        model_ctrl_oc = cfg_.Position_policy().model_ctrl_oc;
        change_oc_flag = cfg_.Position_policy().change_oc_flag;
        const MYExPositionPolicy &pp = cfg_.Position_policy();

        // init position of each contract of each model
        for (const UpperLimitOfModel::value_type &vm : pp.upper_limit_of_model)
        {
            int mod_id = vm.first;
            ModelPositionDataMap::iterator it = model_pos_datas.find(mod_id);
            if (it == model_pos_datas.end())
            {
                it = model_pos_datas.insert(std::make_pair(mod_id, ContractPositionDataMap())).first;
            }

            for (const UpperLimitOfCode::value_type &vc : vm.second)
            {
                ContractPositionDataMap::iterator ccit = it->second.find(vc.first);
                if (ccit == it->second.end())
                {
                    ccit = it->second.insert(std::make_pair(vc.first, ModelPositionData(vc.second))).first;
                }
            }
        }
    }

    void InitPosition(const std::string &code, char dir, VolumeType volume);
    void InitPosition(int mod_id, const std::string &code, char dir, VolumeType volume);

    // 获取模型的仓位，用于回报
    // 在给交易程序推送回报前调用
    T_PositionData GetPosition(SerialNoType sn, const std::string &contract);

    // 获取模型的可用仓位，报单/报价时，计算可平的量
    // 处理流程：获取仓位，冻结仓位，记录该报单/报价的平仓量
    // 接收交易程序的报单/报价请求时，处理
    int CheckAvailablePosition(SerialNoType sn, const std::string &contract, char dir, char oc, VolumeType place_volume)
    {
        if (!change_oc_flag)
        {
            return CheckAvailablePosNoOCChange(sn, contract, dir, oc, place_volume);
        }
        else if (model_ctrl_oc)
        {
            return CheckAvailablePosWithOC(sn, contract, dir, oc, place_volume);
        }
        else
        {
            return CheckAvailablePosWithoutOC(sn, contract, dir, place_volume);
        }
    }
    bool CheckAvailablePosition(SerialNoType sn, const std::string &contract, VolumeType buy_volume, VolumeType sell_volume);

    // 有成交时更新（需要报单序号，获取报单平仓部分的量）
    void UpdatePositionForOrder(SerialNoType sn, const std::string &contract, char dir, VolumeType matched_volume);
    void UpdatePositionForQuote(SerialNoType sn, const std::string &contract, char dir, VolumeType matched_volume);

    // 报单终结时，需要将剩余的量回滚（平仓要回滚冻结的量，开仓要回滚盘口的量）
    // 删除报单序号记录的平仓部分的量的数据
    void RollBackPendingVolumeForOrder(SerialNoType sn, const std::string &contract, char dir, VolumeType remain_volume);
    void RollBackPendingVolumeForQuote(SerialNoType sn, const std::string &contract, char dir, VolumeType remain_volume);

private:
    MYExConfigData cfg_;
    bool model_ctrl_oc;
    bool change_oc_flag;
    inline int GetPosLimit(int model_id, const std::string &contract);

    std::mutex mutex_pos_;

    MyOrderHash<int> m_order_close_vol_hash;

    //OrderCloseVolumeMap order_close_volume;
    QuoteCloseVolumeMap quote_close_volume;
    ModelPositionDataMap model_pos_datas;

    inline int GetModelIdBySn(SerialNoType sn);
    inline ModelPositionData *GetPosImp(int model_id, const std::string &contract);
    inline ModelPositionData *GetOrInsertPosImp(int model_id, const std::string &contract);

    int CheckAvailablePosWithoutOC(SerialNoType sn, const std::string &contract, char dir, VolumeType place_volume);
    int CheckAvailablePosWithOC(SerialNoType sn, const std::string &contract, char dir, char oc, VolumeType place_volume);
    int CheckAvailablePosNoOCChange(SerialNoType sn, const std::string &contract, char dir, char oc, VolumeType place_volume);

    // for debug
    void OutputCurPos();
};

inline int ModelPositionManager::GetPosLimit(int model_id, const std::string &contract)
{
    const MYExPositionPolicy &pp = cfg_.Position_policy();

    // find configure value of this contract of this model
    UpperLimitOfModel::const_iterator cit_mod = pp.upper_limit_of_model.find(model_id);
    if (cit_mod != pp.upper_limit_of_model.end())
    {
        UpperLimitOfCode::const_iterator cit_cp = cit_mod->second.find(contract);
        if (cit_cp != cit_mod->second.end())
        {
            return cit_cp->second;
        }
    }

    // find default configure value of this model
    DefaultUpperLimitOfModel::const_iterator cit_dm = pp.default_upper_limit_of_model.find(model_id);
    if (cit_dm != pp.default_upper_limit_of_model.end())
    {
        return cit_dm->second;
    }

    // use default value
    return pp.default_upper_limit;
}

inline int ModelPositionManager::GetModelIdBySn(SerialNoType sn)
{
    //return (int) (sn % 1000);
    // modified on 20160321, use 8 digits for model id (ABCDxxxx)
	// support c-trader by wangying on 20170905
    return (int) (sn % 1000);
}

inline ModelPositionData *ModelPositionManager::GetPosImp(int model_id, const std::string &contract)
{
    ModelPositionData * pr = NULL;
    ModelPositionDataMap::iterator it;
    ContractPositionDataMap::iterator ccit;

    it = model_pos_datas.find(model_id);
    if (it == model_pos_datas.end())
    {
        it = model_pos_datas.insert(std::make_pair(model_id, ContractPositionDataMap())).first;
    }

    ccit = it->second.find(contract);

    if (ccit != it->second.end())
    {
        pr = &ccit->second;
    }

    if (pr
        && (pr->freeze_long_pos < 0 || pr->freeze_short_pos < 0
            || pr->pending_long_pos < 0 || pr->pending_short_pos < 0))
    {
        EX_LOG_ERROR("[GetPosImp] position of model error");
        OutputCurPos();
    }

    return pr;
}

inline ModelPositionData *ModelPositionManager::GetOrInsertPosImp(int model_id, const std::string &contract)
{
    ModelPositionDataMap::iterator it = model_pos_datas.find(model_id);
    if (it == model_pos_datas.end())
    {
        it = model_pos_datas.insert(std::make_pair(model_id, ContractPositionDataMap())).first;
    }
    ContractPositionDataMap::iterator ccit = it->second.find(contract);
    if (ccit == it->second.end())
    {
        ccit = it->second.insert(std::make_pair(contract, ModelPositionData(GetPosLimit(model_id, contract)))).first;
    }

    return &ccit->second;
}

#endif
