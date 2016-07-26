#include "order_data.h"
#include "my_exchange_utility.h"


VolumeType MYOrderDataManager::GetOpenRemainVolume(const std::string& code, char dir)
{
    VolumeType total = 0;
    std::lock_guard < std::mutex > lock(mutex_order_);

    for (OrderDatas::value_type &v : order_datas_)
    {
        if (v.second->volume_remain > 0
            && v.second->direction == dir
            && v.second->open_close == BUSSINESS_DEF::ENTRUST_OPEN
            && !OrderUtil::IsTerminated(v.second->entrust_status)
            && strcmp(code.c_str(), v.second->stock_code) == 0)
        {
            total += v.second->volume_remain;
        }
    }
    return total;
}



