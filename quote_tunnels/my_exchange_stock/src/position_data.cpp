#include "position_data.h"
//﻿#include "position_data.h"
#include "my_exchange_utility.h"
#include <algorithm>

void MYPositionDataManager::InitPosition(
		const std::string &code, char dir, VolumeType volume,int today_buy_volume,
		int today_sell_volume,int yestoday_long_pos,char ex)
{
    MY_LOG_DEBUG("MYPositionDataManager::InitPosition: %s, dir=%c, volume=%ld", code.c_str(), dir, volume);

    if (volume != 0){
        std::lock_guard < std::mutex > lock(mutex_pos_);

        PositionDataOfCode::iterator it = pos_datas_.find(code);
        if (it == pos_datas_.end()){
            it = pos_datas_.insert(std::make_pair(code, PositionInfoInEX())).first;
        }

        MY_LOG_DEBUG("before InitPosition (%s): long=%d, short=%d",
            code.c_str(), it->second.long_position, it->second.short_position);
        it->second.exchange = ex;
        if (dir == BUSSINESS_DEF::ENTRUST_BUY){
            it->second.long_position += volume;
            it->second.today_buy_volume += today_buy_volume;
            it->second.today_sell_volume += today_sell_volume;
            it->second.long_pos_yd += yestoday_long_pos;
        }else if (dir == BUSSINESS_DEF::ENTRUST_SELL){
            it->second.short_position += volume;
        }else{
            MY_LOG_ERROR("MYPositionDataManager::InitPosition goto wrong branch");
        }

        MY_LOG_DEBUG("after InitPosition, pos(%s): l=%d,s=%d",
            code.c_str(),it->second.long_position, it->second.short_position);
    }
}

T_PositionData MYPositionDataManager::GetPositionInMarketOfCode(const std::string &code)
{
    T_PositionData pos;

    std::lock_guard < std::mutex > lock(mutex_pos_);

    PositionDataOfCode::iterator it = pos_datas_.find(code);
    if (it != pos_datas_.end()){
        pos.long_position = it->second.long_position;
        pos.short_position = it->second.short_position;
        pos.today_buy_volume = it->second.today_buy_volume;
        pos.today_sell_volume = it->second.today_sell_volume;
        pos.exchange_type = it->second.exchange;
    }
    return pos;
}
//PositionInfoInEX * MYPositionDataManager::GetPositionOfCode(const std::string &code)
//{
//    std::lock_guard < std::mutex > lock(mutex_pos_);
//
//    PositionDataOfCode::iterator it = pos_datas_.find(code);
//    if (it == pos_datas_.end()){
//        it = pos_datas_.insert(std::make_pair(code, PositionInfoInEX())).first;
//    }
//    return &(it->second);
//
//}

void MYPositionDataManager::UpdateOutterPosition(const std::string &code, char dir,
		char open_close, VolumeType volume,char ex)
{
    MY_LOG_DEBUG("UpdateOutterPosition: %s, dir=%c, oc=%c, volume=%ld", code.c_str(), dir, open_close,volume);

    if (volume > 0){
        std::lock_guard < std::mutex > lock(mutex_pos_);
        PositionDataOfCode::iterator it = pos_datas_.find(code);
        if (it == pos_datas_.end()){
		   it = pos_datas_.insert(std::make_pair(code, PositionInfoInEX())).first;
	    }

        it->second.exchange = ex;
        if (dir == BUSSINESS_DEF::ENTRUST_BUY){
            it->second.long_position += volume;
            it->second.today_buy_volume += volume;
        }else if (dir == BUSSINESS_DEF::ENTRUST_SELL){
        	it->second.today_sell_volume += volume;
        	it->second.long_position -= volume;
        }

        MY_LOG_DEBUG("after UpdateOutterPosition, pos(%s): l=%d,s=%d",
            code.c_str(),it->second.long_position, it->second.short_position);
    }
}
