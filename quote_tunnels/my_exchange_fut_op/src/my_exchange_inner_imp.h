/*
 * my_exchange 的内部实现，以便对接口隐藏实现细节
 */

#ifndef _MY_EXCHANGE_INNER_IMP_H_
#define  _MY_EXCHANGE_INNER_IMP_H_

#include <pthread.h>
#include <atomic>
#include <map>
#include <unordered_map>
#include <mutex>
#include <string>

#include "my_trade_tunnel_api.h"
#include "config_data.h"

#include "position_data_of_model.h"

typedef std::unordered_map<SerialNoType, int> TunnelIdxOfSerialNo;

typedef std::unordered_map<std::string, int> TunnelIdxOfContract;

class MYExchangeInnerImp
{
public:
    MYExchangeInnerImp(MYTunnelInterface *p, const MYExConfigData &cfg, int tunnel_count)
        : p_tunnel(p), cfg_(cfg), model_pos(cfg)
    {
        cur_idx_ = 0;
        cur_idx_of_contract_ = 0;
        max_tunnel_index_ = tunnel_count;

        finish_init_pos_flag = false;
        have_send_qry_pos_flag = false;

        qry_pos_flag = false;
        qry_order_flag = false;
        qry_trade_flag = false;
        qry_contract_flag = false;

        init_pos_from_ev_flag = cfg_.Position_policy().InitPosByEvfile();

        // don't need init pos, set finish flag
        if (cfg_.Position_policy().init_pos_at_start == false)
        {
            finish_init_pos_flag = true;
        }

        //pthread_spin_init(&op_spin_lock, 0);
    }

    void* QryPosForInit();
    void InitModelPosByEvFile();

    inline int GetTunnelIdxOfContract(const std::string &constract);

    inline int GetNextTunnelIdx()
    {
        if (max_tunnel_index_ < 1) return -1;
        if (max_tunnel_index_ == 1) return 0;

        std::lock_guard<std::mutex> lock(mutex_tunnel_idx_);
        if (++cur_idx_ >= max_tunnel_index_)
        {
            cur_idx_ = 0;
        }
        return cur_idx_;
    }

    std::atomic_bool finish_init_pos_flag;
    std::atomic_bool have_send_qry_pos_flag;

    // interface for manager position of model
    // 为避免改变MYExchange对象结构，模型的仓位管理用MYExchange内部支持对象转发
    inline void InitPosition(const std::string &code, char dir, VolumeType volume)
    {
        return model_pos.InitPosition(code, dir, volume);
    }
    inline T_PositionData GetPosition(SerialNoType sn, const std::string &contract)
    {
        return model_pos.GetPosition(sn, contract);
    }
    inline int CheckAvailablePosition(SerialNoType sn, const std::string &contract, char dir, char oc, VolumeType place_volume)
    {
        return model_pos.CheckAvailablePosition(sn, contract, dir, oc, place_volume);
    }
    inline bool CheckAvailablePosition(SerialNoType sn, const std::string &contract, VolumeType buy_volume, VolumeType sell_volume)
    {
        return model_pos.CheckAvailablePosition(sn, contract, buy_volume, sell_volume);
    }
    inline void UpdatePositionForOrder(SerialNoType sn, const std::string &contract, char dir, VolumeType matched_volume)
    {
        return model_pos.UpdatePositionForOrder(sn, contract, dir, matched_volume);
    }
    inline void UpdatePositionForQuote(SerialNoType sn, const std::string &contract, char dir, VolumeType matched_volume)
    {
        return model_pos.UpdatePositionForQuote(sn, contract, dir, matched_volume);
    }
    inline void RollBackPendingVolumeForOrder(SerialNoType sn, const std::string &contract, char dir, VolumeType remain_volume)
    {
        return model_pos.RollBackPendingVolumeForOrder(sn, contract, dir, remain_volume);
    }
    inline void RollBackPendingVolumeForQuote(SerialNoType sn, const std::string &contract, char dir, VolumeType remain_volume)
    {
        return model_pos.RollBackPendingVolumeForQuote(sn, contract, dir, remain_volume);
    }

    // cache query result and set flag. of position, order detail, and trade detail
	bool qry_pos_flag;
    T_PositionReturn qry_pos_result;
	bool qry_order_flag;
    T_OrderDetailReturn qry_order_result;
    bool qry_trade_flag;
    T_TradeDetailReturn qry_trade_result;

    bool qry_contract_flag;
    T_ContractInfoReturn qry_contract_result;

    //pthread_spinlock_t op_spin_lock;
    std::mutex op_mutex_lock;

    bool init_pos_from_ev_flag;

private:
    MYTunnelInterface *p_tunnel;
    MYExConfigData cfg_;

    std::mutex mutex_tunnel_idx_;
    TunnelIdxOfSerialNo tunnel_idx_of_serial;
    TunnelIdxOfContract tunnel_idx_of_contract;
    int cur_idx_;
    int cur_idx_of_contract_;
    int max_tunnel_index_;

    ModelPositionManager model_pos;
};

inline int MYExchangeInnerImp::GetTunnelIdxOfContract(const std::string &constract)
{
    if (max_tunnel_index_ < 1) return -1;
    if (max_tunnel_index_ == 1) return 0;

    std::lock_guard<std::mutex> lock(mutex_tunnel_idx_);
    TunnelIdxOfContract::iterator it = tunnel_idx_of_contract.find(constract);
    if (it == tunnel_idx_of_contract.end())
    {
        it = tunnel_idx_of_contract.insert(std::make_pair(constract, cur_idx_of_contract_)).first;
        if (++cur_idx_of_contract_ >= max_tunnel_index_)
        {
            cur_idx_of_contract_ = 0;
        }
    }

    return it->second;
}

#endif
