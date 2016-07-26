/*
 * my_exchange 的内部实现，以便对接口隐藏实现细节
 */

#ifndef _MY_EXCHANGE_INNER_IMP_H_
#define  _MY_EXCHANGE_INNER_IMP_H_

#include <atomic>
#include <map>
#include <unordered_map>
#include <mutex>
#include <string>

//#include "my_tunnel_lib.h"
#include "my_trade_tunnel_api.h"
#include "config_data.h"

typedef std::unordered_map<SerialNoType, int> TunnelIdxOfSerialNo;
//typedef std::map<SerialNoType, int> TunnelIdxOfSerialNo;

typedef std::unordered_map<std::string, int> TunnelIdxOfContract;
//typedef std::map<std::string, int> TunnelIdxOfContract;

class MYExchangeInnerImp
{
public:
    MYExchangeInnerImp(MYTunnelInterface *p, const MYExConfigData &cfg, int tunnel_count)
        : p_tunnel(p), cfg_(cfg)
    {
        cur_idx_ = 0;
        cur_idx_of_contract_ = 0;
        max_tunnel_index_ = tunnel_count;

        finish_init_pos_flag = false;
        have_send_qry_pos_flag = false;

        // don't need init pos, set finish flag
        if (cfg_.Position_policy().init_pos_at_start == false)
        {
            finish_init_pos_flag = true;
        }
    }

    void* QryPosForInit();

    inline int GetTunnelIdxOfContract(const std::string &constract);

    inline int GetNextTunnelIdx()
    {
        if (max_tunnel_index_ < 1) return -1;
        if (max_tunnel_index_ == 1) return 0;

        std::lock_guard < std::mutex > lock(mutex_tunnel_idx_);

        if (++cur_idx_ >= max_tunnel_index_)
        {
            cur_idx_ = 0;
        }
        return cur_idx_;
    }

    inline int GetUpperLimitOfCode(const std::string &code)
    {
//        UpperLimitOfCode::const_iterator cit = cfg_.Position_policy().upper_limit_of_code.find(code);
//        if (cit != cfg_.Position_policy().upper_limit_of_code.end())
//        {
//            return cit->second;
//        }

        return cfg_.Position_policy().default_upper_limit;
    }

    std::atomic_bool finish_init_pos_flag;
    std::atomic_bool have_send_qry_pos_flag;

private:
    MYTunnelInterface *p_tunnel;
    MYExConfigData cfg_;

    std::mutex mutex_tunnel_idx_;
    TunnelIdxOfSerialNo tunnel_idx_of_serial;
    TunnelIdxOfContract tunnel_idx_of_contract;
    int cur_idx_;
    int cur_idx_of_contract_;
    int max_tunnel_index_;
};

inline int MYExchangeInnerImp::GetTunnelIdxOfContract(const std::string &constract)
{
    if (max_tunnel_index_ < 1) return -1;
    if (max_tunnel_index_ == 1) return 0;

    std::lock_guard < std::mutex > lock(mutex_tunnel_idx_);
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
