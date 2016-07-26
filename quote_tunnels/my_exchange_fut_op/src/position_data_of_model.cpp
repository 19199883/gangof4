#include "position_data_of_model.h"
#include "my_exchange_utility.h"
#include "my_cmn_log.h"
#include <algorithm>

#define GET_MID_BY_SN(sn)
//#define OUTPUT_DEBUG_INFO()  OutputCurPos()
#define OUTPUT_DEBUG_INFO()

void ModelPositionManager::InitPosition(const std::string &code, char dir, VolumeType volume)
{
    EX_LOG_INFO("ModelPositionManager::InitPosition- code:%s, dir=%c, volume=%ld", code.c_str(), dir, volume);

    if (model_pos_datas.empty())
    {
        EX_LOG_ERROR("ModelPositionManager::InitPosition, no initial data of model, can't init position");
        return;
    }

    // all position set to first model witch trade on this contract
    bool set_ret = false;
    for (ModelPositionDataMap::iterator it = model_pos_datas.begin(); it != model_pos_datas.end(); ++it)
    {
        ContractPositionDataMap::iterator ccit = it->second.find(code);

        if (ccit != it->second.end())
        {
            if (dir == MY_TNL_D_BUY)
            {
                ccit->second.long_pos += volume;
            }
            else if (dir == MY_TNL_D_SELL)
            {
                ccit->second.short_pos += volume;
            }

            set_ret = true;
            EX_LOG_INFO("ModelPositionManager::InitPosition set success- mod:%d,contract:%s,long:%d,short:%d",
                it->first, code.c_str(), ccit->second.long_pos, ccit->second.short_pos);
            break;
        }
    }

    // all position set to first model witch trade something
    if (!set_ret)
    {
        for (ModelPositionDataMap::iterator it = model_pos_datas.begin(); it != model_pos_datas.end(); ++it)
        {
            if (!it->second.empty())
            {
                ContractPositionDataMap::iterator ccit = it->second.find(code);

                if (ccit == it->second.end())
                {
                    ccit = it->second.insert(std::make_pair(code, ModelPositionData())).first;
                }

                if (dir == MY_TNL_D_BUY)
                {
                    ccit->second.long_pos += volume;
                }
                else if (dir == MY_TNL_D_SELL)
                {
                    ccit->second.short_pos += volume;
                }

                set_ret = true;
                EX_LOG_INFO("ModelPositionManager::InitPosition set success- mod:%d,contract:%s,long:%d,short:%d",
                    it->first, code.c_str(), ccit->second.long_pos, ccit->second.short_pos);
                break;
            }
        }
    }

    if (!set_ret)
    {
        EX_LOG_ERROR("ModelPositionManager::InitPosition failed, no model trade on this contract");
    }
}
void ModelPositionManager::InitPosition(int mod_id, const std::string &code, char dir, VolumeType volume)
{
    EX_LOG_INFO("ModelPositionManager::InitPosition- mod:%d,code:%s, dir=%c, volume=%ld", mod_id, code.c_str(), dir, volume);

    if (model_pos_datas.empty())
    {
        EX_LOG_ERROR("ModelPositionManager::InitPosition, no inital data of model, can't init position");
        return;
    }

    ModelPositionDataMap::iterator it = model_pos_datas.find(mod_id);
    if (it == model_pos_datas.end())
    {
        it = model_pos_datas.insert(std::make_pair(mod_id, ContractPositionDataMap())).first;
    }
    ContractPositionDataMap::iterator ccit = it->second.find(code);

    if (ccit == it->second.end())
    {
        ccit = it->second.insert(std::make_pair(code, ModelPositionData())).first;
    }

    if (dir == MY_TNL_D_BUY)
    {
        ccit->second.long_pos += volume;
    }
    else if (dir == MY_TNL_D_SELL)
    {
        ccit->second.short_pos += volume;
    }

    EX_LOG_INFO("ModelPositionManager::InitPosition set success- mod:%d,contract:%s,long:%d,short:%d",
        mod_id, code.c_str(), ccit->second.long_pos, ccit->second.short_pos);
}

T_PositionData ModelPositionManager::GetPosition(SerialNoType sn, const std::string& contract)
{
    T_PositionData pos;
    std::lock_guard<std::mutex> lock(mutex_pos_);

    ModelPositionData *model_pos = GetPosImp(GetModelIdBySn(sn), contract);

    if (model_pos)
    {
        pos.long_position = model_pos->long_pos + model_pos->freeze_long_pos;
        pos.short_position = model_pos->short_pos + model_pos->freeze_short_pos;
    }

    //OUTPUT_DEBUG_INFO();
    return pos;
}

int ModelPositionManager::CheckAvailablePosWithoutOC(SerialNoType sn, const std::string &contract, char dir,
    VolumeType place_volume)
{
    int ret;
    int left_vol = (int) place_volume;

    std::lock_guard<std::mutex> lock(mutex_pos_);

    ModelPositionData *model_pos = GetOrInsertPosImp(GetModelIdBySn(sn), contract);

    int close_volume = 0;
    if (model_pos)
    {
        if (model_pos->max_pos > 0)
        {
            if (dir == MY_TNL_D_BUY)
            {
                left_vol = model_pos->max_pos + model_pos->short_pos - model_pos->max_possible_long();
            }
            else if (dir == MY_TNL_D_SELL)
            {
                left_vol = model_pos->max_pos + model_pos->long_pos - model_pos->max_possible_short();
            }

            left_vol = std::min<int>((int) place_volume, left_vol);
        }

        if (dir == MY_TNL_D_BUY)
        {
            close_volume = std::min<int>(model_pos->short_pos, (int) place_volume);
            model_pos->short_pos -= close_volume;
            model_pos->freeze_short_pos += close_volume;
            model_pos->pending_long_pos += ((int) place_volume - close_volume);
        }
        else if (dir == MY_TNL_D_SELL)
        {
            close_volume = std::min<int>(model_pos->long_pos, (int) place_volume);
            model_pos->long_pos -= close_volume;
            model_pos->freeze_long_pos += close_volume;
            model_pos->pending_short_pos += ((int) place_volume - close_volume);
        }

        if (close_volume > 0)
        {
            ret = m_order_close_vol_hash.AddData(sn, close_volume);
            if (ret != 0)
            {
                EX_LOG_ERROR("[CheckAvailablePosWithoutOC] AddData Failed : sn = %ld\n", sn);
            }
            //order_close_volume.insert(std::make_pair(sn, close_volume));
        }
    }
    else
    {
        left_vol = 0;
        EX_LOG_ERROR("[CheckAvailablePosWithoutOC] get position of model failed, sn:%lld, contract:%s", sn, contract.c_str());
    }

    OUTPUT_DEBUG_INFO();
    return left_vol;
}

int ModelPositionManager::CheckAvailablePosNoOCChange(SerialNoType sn, const std::string &contract,
    char dir, char oc, VolumeType place_volume)
{
    int ret;
    int left_vol = (int) place_volume;

    std::lock_guard<std::mutex> lock(mutex_pos_);

    ModelPositionData *model_pos = GetOrInsertPosImp(GetModelIdBySn(sn), contract);

    int close_volume = 0;
    if (model_pos)
    {
        if (oc == MY_TNL_D_OPEN)
        {
            close_volume = 0;
            if (model_pos->max_pos > 0)
            {
                if (dir == MY_TNL_D_BUY)
                {
                    left_vol = model_pos->max_pos + model_pos->short_pos - model_pos->max_possible_long();
                }
                else if (dir == MY_TNL_D_SELL)
                {
                    left_vol = model_pos->max_pos + model_pos->long_pos - model_pos->max_possible_short();
                }

                left_vol = std::min<int>((int) place_volume, left_vol);
            }

            if (dir == MY_TNL_D_BUY)
            {
                model_pos->pending_long_pos += ((int) place_volume);
            }
            else if (dir == MY_TNL_D_SELL)
            {
                model_pos->pending_short_pos += ((int) place_volume);
            }
        }
        else
        {
            close_volume = place_volume;
            if (dir == MY_TNL_D_BUY)
            {
                left_vol = std::min<int>(model_pos->short_pos, (int) place_volume);
                model_pos->short_pos -= close_volume;
                model_pos->freeze_short_pos += close_volume;
            }
            else if (dir == MY_TNL_D_SELL)
            {
                left_vol = std::min<int>(model_pos->long_pos, (int) place_volume);
                model_pos->long_pos -= close_volume;
                model_pos->freeze_long_pos += close_volume;
            }
        }

        if (close_volume > 0)
        {
            ret = m_order_close_vol_hash.AddData(sn, close_volume);
            if (ret != 0)
            {
                EX_LOG_ERROR("[CheckAvailablePosWithoutOC] AddData Failed : sn = %ld\n", sn);
            }
            //order_close_volume.insert(std::make_pair(sn, close_volume));
        }
    }
    else
    {
        left_vol = 0;
        EX_LOG_ERROR("[CheckAvailablePosNoOCChange] get position of model failed, sn:%lld, contract:%s", sn, contract.c_str());
    }

    OUTPUT_DEBUG_INFO();
    return left_vol;
}

int ModelPositionManager::CheckAvailablePosWithOC(SerialNoType sn, const std::string &contract,
    char dir, char oc, VolumeType place_volume)
{
    int ret;
    int left_vol = (int) place_volume;

    std::lock_guard<std::mutex> lock(mutex_pos_);

    ModelPositionData *model_pos = GetOrInsertPosImp(GetModelIdBySn(sn), contract);

    int close_volume = 0;
    if (model_pos)
    {
        if (oc == MY_TNL_D_OPEN)
        {
            close_volume = 0;
            if (model_pos->max_pos > 0)
            {
                if (dir == MY_TNL_D_BUY)
                {
                    left_vol = model_pos->max_pos - model_pos->max_possible_long();
                }
                else if (dir == MY_TNL_D_SELL)
                {
                    left_vol = model_pos->max_pos - model_pos->max_possible_short();
                }

                left_vol = std::min<int>((int) place_volume, left_vol);
            }

            if (dir == MY_TNL_D_BUY)
            {
                model_pos->pending_long_pos += ((int) place_volume);
            }
            else if (dir == MY_TNL_D_SELL)
            {
                model_pos->pending_short_pos += ((int) place_volume);
            }
        }
        else
        {
            close_volume = place_volume;
            if (dir == MY_TNL_D_BUY)
            {
                left_vol = std::min<int>(model_pos->short_pos, (int) place_volume);
                model_pos->short_pos -= close_volume;
                model_pos->freeze_short_pos += close_volume;
            }
            else if (dir == MY_TNL_D_SELL)
            {
                left_vol = std::min<int>(model_pos->long_pos, (int) place_volume);
                model_pos->long_pos -= close_volume;
                model_pos->freeze_long_pos += close_volume;
            }
        }

        if (close_volume > 0)
        {
            ret = m_order_close_vol_hash.AddData(sn, close_volume);
            if (ret != 0)
            {
                EX_LOG_ERROR("[CheckAvailablePosWithoutOC] AddData Failed : sn = %ld\n", sn);
            }
            //order_close_volume.insert(std::make_pair(sn, close_volume));
        }
    }
    else
    {
        left_vol = 0;
        EX_LOG_ERROR("[CheckAvailablePosWithOC] get position of model failed, sn:%lld, contract:%s", sn, contract.c_str());
    }

    OUTPUT_DEBUG_INFO();
    return left_vol;
}

bool ModelPositionManager::CheckAvailablePosition(SerialNoType sn, const std::string &contract, VolumeType buy_volume,
    VolumeType sell_volume)
{
    std::lock_guard<std::mutex> lock(mutex_pos_);
    bool check_res = true;

    ModelPositionData *model_pos = GetOrInsertPosImp(GetModelIdBySn(sn), contract);

    int buy_close_volume = 0;
    int sell_close_volume = 0;
    if (model_pos)
    {
        if (model_pos->max_pos > 0)
        {
            if ((model_pos->max_pos + model_pos->short_pos - model_pos->max_possible_long() < buy_volume)
                || (model_pos->max_pos + model_pos->long_pos - model_pos->max_possible_short() < sell_volume))
            {
                check_res = false;
            }
        }

        if (model_pos->short_pos >= buy_volume)
        {
            buy_close_volume = buy_volume;
            model_pos->short_pos -= buy_close_volume;
            model_pos->freeze_short_pos += buy_close_volume;
        }
        else
        {
            model_pos->pending_long_pos += buy_volume;
        }

        if (model_pos->long_pos > sell_volume)
        {
            sell_close_volume = sell_volume;
            model_pos->long_pos -= sell_close_volume;
            model_pos->freeze_long_pos += sell_close_volume;
        }
        else
        {
            model_pos->pending_short_pos += sell_volume;
        }

        if (buy_close_volume > 0 || sell_close_volume > 0)
        {
            quote_close_volume.insert(std::make_pair(sn, std::make_pair(buy_close_volume, sell_close_volume)));
        }
    }

    OUTPUT_DEBUG_INFO();
    return check_res;
}

void ModelPositionManager::UpdatePositionForOrder(SerialNoType sn, const std::string& contract, char dir, VolumeType matched_volume)
{
    int* the_close_vol;
    int close_vol = 0;
    if (matched_volume <= 0) return;

    std::lock_guard<std::mutex> lock(mutex_pos_);
    ModelPositionData *model_pos = GetOrInsertPosImp(GetModelIdBySn(sn), contract);
    if (!model_pos)
    {
        EX_LOG_ERROR("[UpdatePositionForOrder] get position of model failed, sn:%lld, contract:%s", sn, contract.c_str());
        return;
    }

    // get and update close volume of this order
    the_close_vol = m_order_close_vol_hash.GetData(sn);
    if (the_close_vol != NULL)
    {
        close_vol = std::min<int>(*the_close_vol, (int) matched_volume);

        *the_close_vol -= close_vol;

        if (0 == *the_close_vol)
        {
            m_order_close_vol_hash.DelData(sn);
        }
    }
// 
//     OrderCloseVolumeMap::iterator it_close_v = order_close_volume.find(sn);
//     if (it_close_v != order_close_volume.end() && it_close_v->second > 0)
//     {
//         close_vol = std::min<int>(it_close_v->second, (int) matched_volume);
//         it_close_v->second -= close_vol;
// 
//         if (it_close_v->second == 0)
//         {
//             order_close_volume.erase(it_close_v);
//         }
//     }

    int open_vol = matched_volume - close_vol;

    if (dir == MY_TNL_D_BUY)
    {
        model_pos->freeze_short_pos -= close_vol;
        model_pos->long_pos += open_vol;
        model_pos->pending_long_pos -= open_vol;
    }
    else
    {
        model_pos->freeze_long_pos -= close_vol;
        model_pos->short_pos += open_vol;
        model_pos->pending_short_pos -= open_vol;
    }

    OUTPUT_DEBUG_INFO();
}

void ModelPositionManager::UpdatePositionForQuote(SerialNoType sn, const std::string& contract, char dir, VolumeType matched_volume)
{
    if (matched_volume <= 0) return;

    std::lock_guard<std::mutex> lock(mutex_pos_);
    ModelPositionData *model_pos = GetOrInsertPosImp(GetModelIdBySn(sn), contract);
    if (!model_pos)
    {
        EX_LOG_ERROR("[UpdatePositionForQuote] get position of model failed, sn:%lld, contract:%s", sn, contract.c_str());
        return;
    }

    int close_vol = 0;

    // get and update close volume of this quote
    QuoteCloseVolumeMap::iterator it_close_v = quote_close_volume.find(sn);
    if (it_close_v != quote_close_volume.end())
    {
        if (dir == MY_TNL_D_BUY)
        {
            close_vol = std::min<int>(it_close_v->second.first, (int) matched_volume);
            it_close_v->second.first -= close_vol;
        }
        else
        {
            close_vol = std::min<int>(it_close_v->second.second, (int) matched_volume);
            it_close_v->second.second -= close_vol;
        }

        if (it_close_v->second.first == 0 && it_close_v->second.second == 0)
        {
            quote_close_volume.erase(it_close_v);
        }
    }

    int open_vol = matched_volume - close_vol;

    if (dir == MY_TNL_D_BUY)
    {
        model_pos->freeze_short_pos -= close_vol;
        model_pos->long_pos += open_vol;
        model_pos->pending_long_pos -= open_vol;
    }
    else
    {
        model_pos->freeze_long_pos -= close_vol;
        model_pos->short_pos += open_vol;
        model_pos->pending_short_pos -= open_vol;
    }

    OUTPUT_DEBUG_INFO();
}

void ModelPositionManager::RollBackPendingVolumeForOrder(SerialNoType sn, const std::string& contract, char dir,
    VolumeType remain_volume)
{
    int* the_val;
    if (remain_volume <= 0) return;

    std::lock_guard<std::mutex> lock(mutex_pos_);
    ModelPositionData *model_pos = GetPosImp(GetModelIdBySn(sn), contract);
    if (!model_pos)
    {
        EX_LOG_ERROR("[RollBackPendingVolumeForOrder] get position of model failed, sn:%lld, contract:%s", sn, contract.c_str());
        return;
    }

    int close_vol = 0;

    // get and erase close volume of this order
    the_val = m_order_close_vol_hash.GetData(sn);
    if (the_val != NULL)
    {
        close_vol = std::min<int>(*the_val, (int) remain_volume);
        m_order_close_vol_hash.DelData(sn);
    }
//     OrderCloseVolumeMap::iterator it_close_v = order_close_volume.find(sn);
//     if (it_close_v != order_close_volume.end() && it_close_v->second > 0)
//     {
//         close_vol = std::min<int>(it_close_v->second, (int) remain_volume);
// 
//         order_close_volume.erase(it_close_v);
//     }

    int open_vol = remain_volume - close_vol;

    // rollback
    if (dir == MY_TNL_D_BUY)
    {
        model_pos->freeze_short_pos -= close_vol;
        model_pos->short_pos += close_vol;
        model_pos->pending_long_pos -= open_vol;
    }
    else
    {
        model_pos->freeze_long_pos -= close_vol;
        model_pos->long_pos += close_vol;
        model_pos->pending_short_pos -= open_vol;
    }

    OUTPUT_DEBUG_INFO();
}

void ModelPositionManager::RollBackPendingVolumeForQuote(SerialNoType sn, const std::string& contract, char dir,
    VolumeType remain_volume)
{
    if (remain_volume <= 0) return;

    std::lock_guard<std::mutex> lock(mutex_pos_);
    ModelPositionData *model_pos = GetPosImp(GetModelIdBySn(sn), contract);

    int close_vol = 0;

    // get and update close volume of this quote
    QuoteCloseVolumeMap::iterator it_close_v = quote_close_volume.find(sn);
    if (it_close_v != quote_close_volume.end())
    {
        if (dir == MY_TNL_D_BUY)
        {
            close_vol = std::min<int>(it_close_v->second.first, (int) remain_volume);
            it_close_v->second.first -= close_vol;
        }
        else
        {
            close_vol = std::min<int>(it_close_v->second.second, (int) remain_volume);
            it_close_v->second.second -= close_vol;
        }

        if (it_close_v->second.first == 0 && it_close_v->second.second == 0)
        {
            quote_close_volume.erase(it_close_v);
        }
    }

    int open_vol = remain_volume - close_vol;

    // rollback
    if (dir == MY_TNL_D_BUY)
    {
        model_pos->freeze_short_pos -= close_vol;
        model_pos->short_pos += close_vol;
        model_pos->pending_long_pos -= open_vol;
    }
    else
    {
        model_pos->freeze_long_pos -= close_vol;
        model_pos->long_pos += close_vol;
        model_pos->pending_short_pos -= open_vol;
    }

    OUTPUT_DEBUG_INFO();
}

void ModelPositionManager::OutputCurPos()
{
    ModelPositionDataMap::iterator it = model_pos_datas.begin();
    for (; it != model_pos_datas.end(); ++it)
    {
        ContractPositionDataMap::iterator ccit = it->second.begin();
        for (; ccit != it->second.end(); ++ccit)
        {
            EX_LOG_DEBUG("current position of model:"
                " modid:%d,contract:%s,long:%d,short:%d,freeze_long:%d,freeze_short:%d,pending_long:%d,pending_short:%d,max:%d",
                it->first, ccit->first.c_str(),
                ccit->second.long_pos,
                ccit->second.short_pos,
                ccit->second.freeze_long_pos,
                ccit->second.freeze_short_pos,
                ccit->second.pending_long_pos,
                ccit->second.pending_short_pos,
                ccit->second.max_pos);
        }
    }

//     for (OrderCloseVolumeMap::iterator oc_it = order_close_volume.begin();
//         oc_it != order_close_volume.end(); ++oc_it)
//     {
//         EX_LOG_DEBUG("order close volume,%ld: %d", oc_it->first, oc_it->second);
//     }

    for (QuoteCloseVolumeMap::iterator qc_it = quote_close_volume.begin();
        qc_it != quote_close_volume.end(); ++qc_it)
    {
        EX_LOG_DEBUG("quote close volume,%ld: (%d,%d)", qc_it->first, qc_it->second.first, qc_it->second.second);
    }
}
