#include "position_data.h"
#include "my_exchange_utility.h"
#include <algorithm>

void MYPositionDataManager::InitPosition(const std::string &code, char dir, VolumeType volume)
{
    EX_LOG_INFO("MYPositionDataManager::InitPosition: %s, dir=%c, volume=%ld", code.c_str(), dir, volume);

    if (volume != 0)
    {
        std::lock_guard < std::mutex > lock(mutex_pos_);
        PositionDataOfCode::iterator it = pos_datas_.find(code);
        if (it == pos_datas_.end())
        {
            it = pos_datas_.insert(std::make_pair(code, PositionInfoInEX())).first;
        }

        EX_LOG_DEBUG("before InitPosition (%s): long=%d, short=%d",
            code.c_str(), it->second.long_position, it->second.short_position);
        if (dir == MY_TNL_D_BUY)
        {
            it->second.long_position += volume;
        }
        else if (dir == MY_TNL_D_SELL)
        {
            it->second.short_position += volume;
        }
        else
        {
            EX_LOG_ERROR("MYPositionDataManager::InitPosition goto wrong branch");
        }

        EX_LOG_INFO("after InitPosition, pos(%s): p_ex=%d,l=%d,s=%d,l_f=%d,s_f=%d,b_o=%d,s_o=%d,b_to=%d,s_to=%d",
            code.c_str(),
            it->second.position_in_myex,
            it->second.long_position, it->second.short_position,
            it->second.long_freeze, it->second.short_freeze,
            it->second.long_pending_volume_of_open, it->second.short_pending_volume_of_open,
            it->second.long_pending_volume_trans_to_open, it->second.short_pending_volume_trans_to_open);
    }
}

T_PositionData MYPositionDataManager::GetPositionInMarketOfCode(const std::string &code)
{
    T_PositionData pos;

    std::lock_guard < std::mutex > lock(mutex_pos_);
    PositionDataOfCode::iterator it = pos_datas_.find(code);
    if (it != pos_datas_.end())
    {
        pos.long_position = it->second.long_position + it->second.long_freeze;
        pos.short_position = it->second.short_position + it->second.short_freeze;
    }
    return pos;
}
PositionInfoInEX * MYPositionDataManager::GetPositionOfCode(const std::string &code)
{
    std::lock_guard < std::mutex > lock(mutex_pos_);
    PositionDataOfCode::iterator it = pos_datas_.find(code);
    if (it == pos_datas_.end())
    {
        it = pos_datas_.insert(std::make_pair(code, PositionInfoInEX())).first;
    }
    return &(it->second);
}

void MYPositionDataManager::UpdateInnerPosition(const OrderInfoInEx *order, const OrderInfoInEx *other_order,
    VolumeType matched_volume)
{
    EX_LOG_DEBUG("UpdateInnerPosition: %ld(dir=%c;oc=%c;v=%ld) with other order %ld(dir=%c;oc=%c;v=%ld), match=%ld",
        order->serial_no,
        order->direction, order->open_close, order->volume,
        other_order->serial_no,
        other_order->direction, other_order->open_close, other_order->volume,
        matched_volume);

    // 只有反向单才可能内部撮合，非法调用核对
    if (strcmp(order->stock_code, other_order->stock_code) != 0
        || order->direction == other_order->direction)
    {
        EX_LOG_ERROR("inner match error, order : %ld with other order %ld", order->serial_no, other_order->serial_no);
    }

    int vol = 0;
    // open - open，负仓位（开仓内部撮合了，内部欠着客户的仓位）
    if (order->open_close == MY_TNL_D_OPEN && order->open_close == other_order->open_close)
    {
        vol = -matched_volume;
    }

    // close - close，正仓位（平仓内部撮合了，内部有多余的仓位）
    if (order->open_close == MY_TNL_D_CLOSE && order->open_close == other_order->open_close)
    {
        vol = matched_volume;
    }

    // open - close，发生策略/账户间的仓位转换，多账号处理时再考虑

    if (vol != 0)
    {
        std::lock_guard < std::mutex > lock(mutex_pos_);
        PositionDataOfCode::iterator it = pos_datas_.find(order->stock_code);
        if (it == pos_datas_.end())
        {
            it = pos_datas_.insert(std::make_pair(order->stock_code, PositionInfoInEX())).first;
        }
        it->second.position_in_myex += vol;

        EX_LOG_DEBUG("after UpdateInnerPosition, pos(%s): p_ex=%d,l=%d,s=%d,l_f=%d,s_f=%d,b_o=%d,s_o=%d,b_to=%d,s_to=%d",
            order->stock_code,
            it->second.position_in_myex,
            it->second.long_position, it->second.short_position,
            it->second.long_freeze, it->second.short_freeze,
            it->second.long_pending_volume_of_open, it->second.short_pending_volume_of_open,
            it->second.long_pending_volume_trans_to_open, it->second.short_pending_volume_trans_to_open);
    }
}

void MYPositionDataManager::InnerMatchTransferOrder(const std::string &code, char oc, VolumeType matched_volume)
{
    EX_LOG_DEBUG("InnerMatchTransferOrder: %s,oc=%c,inner_match=%ld",
        code.c_str(), oc, matched_volume);

    std::lock_guard < std::mutex > lock(mutex_pos_);
    PositionDataOfCode::iterator it = pos_datas_.find(code);
    if (it != pos_datas_.end())
    {
        // 变换成开仓的撮合，增加内部仓位
        if (oc == MY_TNL_D_OPEN)
        {
            it->second.position_in_myex += matched_volume;
        }

        // 变换成平仓的撮合，减少内部仓位
        if (oc == MY_TNL_D_CLOSE)
        {
            it->second.position_in_myex -= matched_volume;
        }

        EX_LOG_DEBUG("after InnerMatchTransferOrder, pos(%s): p_ex=%d,l=%d,s=%d,l_f=%d,s_f=%d,b_o=%d,s_o=%d,b_to=%d,s_to=%d",
            code.c_str(),
            it->second.position_in_myex,
            it->second.long_position, it->second.short_position,
            it->second.long_freeze, it->second.short_freeze,
            it->second.long_pending_volume_of_open, it->second.short_pending_volume_of_open,
            it->second.long_pending_volume_trans_to_open, it->second.short_pending_volume_trans_to_open);
    }
}

void MYPositionDataManager::UpdateOutterPosition(const std::string &code, char dir, char open_close, VolumeType volume,
    bool is_transform)
{
    EX_LOG_DEBUG("UpdateOutterPosition: %s, dir=%c, oc=%c, volume=%ld", code.c_str(), dir, open_close,
        volume);

    // 平仓已经预扣，only update freeze position
    if (open_close == MY_TNL_D_CLOSE && volume > 0)
    {
        std::lock_guard < std::mutex > lock(mutex_pos_);
        PositionDataOfCode::iterator it = pos_datas_.find(code);
        // 买平filled
        if (dir == MY_TNL_D_BUY)
        {
            it->second.short_freeze -= volume;
        }

        // 卖平filled
        if (dir == MY_TNL_D_SELL)
        {
            it->second.long_freeze -= volume;
        }

        EX_LOG_DEBUG("after UpdateOutterPosition, pos(%s): p_ex=%d,l=%d,s=%d,l_f=%d,s_f=%d,b_o=%d,s_o=%d,b_to=%d,s_to=%d",
            code.c_str(),
            it->second.position_in_myex,
            it->second.long_position, it->second.short_position,
            it->second.long_freeze, it->second.short_freeze,
            it->second.long_pending_volume_of_open, it->second.short_pending_volume_of_open,
            it->second.long_pending_volume_trans_to_open, it->second.short_pending_volume_trans_to_open);

        return;
    }

    if (open_close == MY_TNL_D_OPEN && volume > 0)
    {
        std::lock_guard < std::mutex > lock(mutex_pos_);
        PositionDataOfCode::iterator it = pos_datas_.find(code);
        if (it == pos_datas_.end())
        {
            it = pos_datas_.insert(std::make_pair(code, PositionInfoInEX())).first;
        }

        EX_LOG_DEBUG("current position (%s): long=%d, short=%d",
            code.c_str(), it->second.long_position, it->second.short_position);
        if (dir == MY_TNL_D_BUY && open_close == MY_TNL_D_OPEN)
        {
            it->second.long_position += volume;

            it->second.long_pending_volume_of_open -= volume;
            if (is_transform) it->second.long_pending_volume_trans_to_open -= volume;
        }
        else if (dir == MY_TNL_D_SELL && open_close == MY_TNL_D_OPEN)
        {
            it->second.short_position += volume;

            it->second.short_pending_volume_of_open -= volume;
            if (is_transform) it->second.short_pending_volume_trans_to_open -= volume;
        }
        else
        {
            EX_LOG_ERROR("MYPositionDataManager::UpdateOutterPosition goto wrong branch");
        }

        EX_LOG_DEBUG("after UpdateOutterPosition, pos(%s): p_ex=%d,l=%d,s=%d,l_f=%d,s_f=%d,b_o=%d,s_o=%d,b_to=%d,s_to=%d",
            code.c_str(),
            it->second.position_in_myex,
            it->second.long_position, it->second.short_position,
            it->second.long_freeze, it->second.short_freeze,
            it->second.long_pending_volume_of_open, it->second.short_pending_volume_of_open,
            it->second.long_pending_volume_trans_to_open, it->second.short_pending_volume_trans_to_open);
    }
}

VolumeType MYPositionDataManager::GetTransformVolume(OrderInfoInEx *p)
{
    int trans_vol = 0;

    std::lock_guard < std::mutex > lock(mutex_pos_);
    PositionDataOfCode::iterator it = pos_datas_.find(p->stock_code);
    if (it == pos_datas_.end())
    {
        it = pos_datas_.insert(std::make_pair(p->stock_code, PositionInfoInEX())).first;
    }

    if (it != pos_datas_.end())
    {
        // 对交易系统中，可平仓的量 = short_position(long_position) - position_in_myex
        // 在实际交易所中，最大可平仓位
        int max_close_vol = 0;
        if (p->direction == MY_TNL_D_BUY && it->second.short_position > 0)
        {
            max_close_vol = it->second.short_position;
        }
        else if (p->direction == MY_TNL_D_SELL && it->second.long_position > 0)
        {
            max_close_vol = it->second.long_position;
        }

        // 尽量平，剩余的转换（维持小的资金占用）
        int close_vol = std::min<int>(max_close_vol, p->volume_remain);

        // fak/fok can't split
        if ((p->order_type == MY_TNL_HF_FAK || p->order_type == MY_TNL_HF_FOK)
            && close_vol < p->volume_remain)
        {
            close_vol = 0;
        }
        int open_vol = p->volume_remain - close_vol;

        // 平仓实际仓位的预扣, pending open increment
        if (p->direction == MY_TNL_D_BUY)
        {
            it->second.short_position -= close_vol;
            it->second.short_freeze += close_vol;
            it->second.long_pending_volume_of_open += open_vol;
        }
        else if (p->direction == MY_TNL_D_SELL)
        {
            it->second.long_position -= close_vol;
            it->second.long_freeze += close_vol;
            it->second.short_pending_volume_of_open += open_vol;
        }

        // 内部仓位的预扣
        // 不够平，剩余的是转换
        if (p->open_close == MY_TNL_D_CLOSE && open_vol > 0)
        {
            // 平转开，将增加内部仓位
            EX_LOG_DEBUG("Transform(%s) position_in_myex = %d(original) + %d(close to open)",
                p->stock_code, it->second.position_in_myex, open_vol);

            it->second.position_in_myex += open_vol;

            // pending transform to open
            if (p->direction == MY_TNL_D_BUY)
            {
                it->second.long_pending_volume_trans_to_open += open_vol;
            }
            else if (p->direction == MY_TNL_D_SELL)
            {
                it->second.short_pending_volume_trans_to_open += open_vol;
            }

            trans_vol = open_vol;
        }

        // 开仓，尽量转换成平仓，平仓的部分是转换量
        if (p->open_close == MY_TNL_D_OPEN && close_vol > 0)
        {
            // 开转平，减少内部仓位
            EX_LOG_DEBUG("Transform(%s) position_in_myex = %d(original) - %d(open to close)",
                p->stock_code, it->second.position_in_myex, close_vol);

            it->second.position_in_myex -= close_vol;

            trans_vol = close_vol;
        }

        EX_LOG_DEBUG("after Transform, pos(%s): p_ex=%d,l=%d,s=%d,l_f=%d,s_f=%d,b_o=%d,s_o=%d,b_to=%d,s_to=%d",
            p->stock_code,
            it->second.position_in_myex,
            it->second.long_position, it->second.short_position,
            it->second.long_freeze, it->second.short_freeze,
            it->second.long_pending_volume_of_open, it->second.short_pending_volume_of_open,
            it->second.long_pending_volume_trans_to_open, it->second.short_pending_volume_trans_to_open);
    }

    return trans_vol;
}

void MYPositionDataManager::CheckAndTransformQuote(QuoteInfoInEx *p)
{
    std::lock_guard < std::mutex > lock(mutex_pos_);
    PositionDataOfCode::iterator it = pos_datas_.find(p->data.stock_code);
    if (it == pos_datas_.end())
    {
        it = pos_datas_.insert(std::make_pair(p->data.stock_code, PositionInfoInEX())).first;
    }

    if (it != pos_datas_.end())
    {
        if (it->second.long_position >= p->data.sell_volume)
        {
            p->data.sell_open_close = MY_TNL_D_CLOSE;
            it->second.long_position -= p->data.sell_volume;
            it->second.long_freeze += p->data.sell_volume;
            //it->second.position_in_myex -= p->data.sell_volume; // 开转平，减少内部仓位,don't need maintain it 20150105
        }
        else
        {
            p->data.sell_open_close = MY_TNL_D_OPEN;
            it->second.short_pending_volume_of_open += p->data.sell_volume;
        }

        if (it->second.short_position >= p->data.buy_volume)
        {
            p->data.buy_open_close = MY_TNL_D_CLOSE;
            it->second.short_position -= p->data.buy_volume;
            it->second.short_freeze += p->data.buy_volume;
            //it->second.position_in_myex -= p->data.buy_volume; // 开转平，减少内部仓位,don't need maintain it 20150105
        }
        else
        {
            p->data.buy_open_close = MY_TNL_D_OPEN;
            it->second.long_pending_volume_of_open += p->data.buy_volume;
        }
    }

    EX_LOG_DEBUG("after CheckAndTransformQuote, pos(%s): p_ex=%d,l=%d,s=%d,l_f=%d,s_f=%d,b_o=%d,s_o=%d,b_to=%d,s_to=%d",
        p->data.stock_code,
        it->second.position_in_myex,
        it->second.long_position, it->second.short_position,
        it->second.long_freeze, it->second.short_freeze,
        it->second.long_pending_volume_of_open, it->second.short_pending_volume_of_open,
        it->second.long_pending_volume_trans_to_open, it->second.short_pending_volume_trans_to_open);
}

void MYPositionDataManager::RollBackTransformVolume(const std::string &code, char dir, char oc, VolumeType remain_volume)
{
    EX_LOG_DEBUG("MYPositionDataManager::RollBackTransformVolume(%s) oc=%c;remain_volume=%ld",
        code.c_str(), oc, remain_volume);
    std::lock_guard < std::mutex > lock(mutex_pos_);
    PositionDataOfCode::iterator it = pos_datas_.find(code);
    if (it != pos_datas_.end())
    {
        // 开仓回滚，恢复负仓位
        if (oc == MY_TNL_D_OPEN)
        {
            EX_LOG_DEBUG("Transform(%s) position_in_myex = %d - %ld",
                code.c_str(), it->second.position_in_myex, remain_volume);

            it->second.position_in_myex -= remain_volume;

            // rollback pending transfrom to open
            if (dir == MY_TNL_D_BUY)
            {
                it->second.long_pending_volume_trans_to_open -= remain_volume;
            }
            else if (dir == MY_TNL_D_SELL)
            {
                it->second.short_pending_volume_trans_to_open -= remain_volume;
            }
        }

        // 平仓回滚，恢复正仓位
        if (oc == MY_TNL_D_CLOSE)
        {
            EX_LOG_DEBUG("Transform(%s) position_in_myex = %d + %ld",
                code.c_str(), it->second.position_in_myex, remain_volume);
            it->second.position_in_myex += remain_volume;
        }

        EX_LOG_DEBUG("after RollBackTransformVolume, pos(%s): p_ex=%d,l=%d,s=%d,l_f=%d,s_f=%d,b_o=%d,s_o=%d,b_to=%d,s_to=%d",
            code.c_str(),
            it->second.position_in_myex,
            it->second.long_position, it->second.short_position,
            it->second.long_freeze, it->second.short_freeze,
            it->second.long_pending_volume_of_open, it->second.short_pending_volume_of_open,
            it->second.long_pending_volume_trans_to_open, it->second.short_pending_volume_trans_to_open);
    }
}

void MYPositionDataManager::RollBackPendingVolume(const std::string &code, char dir, char oc, VolumeType remain_volume)
{
    EX_LOG_DEBUG("MYPositionDataManager::RollBackPendingVolume(%s) dir=%c;remain_volume=%ld",
        code.c_str(), dir, remain_volume);

    std::lock_guard < std::mutex > lock(mutex_pos_);
    PositionDataOfCode::iterator it = pos_datas_.find(code);
    if (it != pos_datas_.end())
    {
        if (oc == MY_TNL_D_OPEN)
        {
            if (dir == MY_TNL_D_BUY)
            {
                it->second.long_pending_volume_of_open -= remain_volume;
            }
            else
            {
                it->second.short_pending_volume_of_open -= remain_volume;
            }
        }
        else
        {
            if (dir == MY_TNL_D_BUY)
            {
                // 买平回滚
                it->second.short_position += remain_volume;
                it->second.short_freeze -= remain_volume;
            }
            else
            {
                // 卖平回滚
                it->second.long_position += remain_volume;
                it->second.long_freeze -= remain_volume;
            }
        }

        EX_LOG_DEBUG("after RollBackPendingVolume, pos(%s): p_ex=%d,l=%d,s=%d,l_f=%d,s_f=%d,b_o=%d,s_o=%d,b_to=%d,s_to=%d",
            code.c_str(),
            it->second.position_in_myex,
            it->second.long_position, it->second.short_position,
            it->second.long_freeze, it->second.short_freeze,
            it->second.long_pending_volume_of_open, it->second.short_pending_volume_of_open,
            it->second.long_pending_volume_trans_to_open, it->second.short_pending_volume_trans_to_open);
    }
}
