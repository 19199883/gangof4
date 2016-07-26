#ifndef _MY_EXCHANGE_POSITION_DATA_H_
#define  _MY_EXCHANGE_POSITION_DATA_H_

#include "my_exchange_datatype_inner.h"
#include <mutex>

class MYPositionDataManager
{
public:
    // Init仓位, at beginning
    void InitPosition(const std::string &code, char dir, VolumeType volume,int today_buy_volume,
    		int today_sell_volume,int yestoday_long_pos,char ex);

    T_PositionData GetPositionInMarketOfCode(const std::string &code);
//    PositionInfoInEX * GetPositionOfCode(const std::string &code);

    // 外部撮合，更新仓位
    void UpdateOutterPosition(const std::string &code, char dir, char open_close, VolumeType volume,char ex);

private:
    PositionDataOfCode pos_datas_;
    std::mutex mutex_pos_;
};

#endif
