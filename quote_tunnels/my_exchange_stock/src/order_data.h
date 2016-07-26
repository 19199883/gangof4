#ifndef _MY_EXCHANGE_ORDER_DATA_H_
#define  _MY_EXCHANGE_ORDER_DATA_H_

#include "my_trade_tunnel_api.h"
#include "my_exchange_datatype_inner.h"
#include "my_exchange_utility.h"

#include <vector>
#include <mutex>

class OrderContext
{
public:
    OrderContext()
        : st_id_mask(1000), inner_serial_start(9000000000 * st_id_mask)
    {
        // 序列号，要和MYTrader提供的序列号在同一个空间
        // 最好是可以分段区分，简化识别
        serial_no_myexchange_ = inner_serial_start / st_id_mask;
    }

    SerialNoType GetNewSerialNo(SerialNoType super_sn)
    {
        ++serial_no_myexchange_;
        return serial_no_myexchange_ * st_id_mask + super_sn % st_id_mask;
    }

    inline bool IsInnerSerialNo(SerialNoType serial_no) const
        {
        return serial_no >= inner_serial_start;
    }

private:
    const SerialNoType st_id_mask;
    const SerialNoType inner_serial_start;
    SerialNoType serial_no_myexchange_;
};

// 报单数据的管理类
// 所有外部接口，都需要进行同步锁定；内部函数，认为是在外部接口的锁定范围内调用，不再锁定
class MYOrderDataManager
{
public:
    MYOrderDataManager()
    {
    }

    // 报单时，记录挂单信息
    bool AddOrder(OrderInfoInEx *place_order)
    {
        MY_LOG_DEBUG("MYOrderDataManager::AddOrder - %ld", place_order->serial_no);
        bool insert_success = false;
        {
            std::lock_guard < std::mutex > lock(mutex_order_);
            insert_success = order_datas_.insert(std::make_pair(place_order->serial_no, place_order)).second;
        }
        if (!insert_success){
            MY_LOG_ERROR("AddOrder failed, order serial no: %ld", place_order->serial_no);
        }

        return insert_success;
    }

    // 报单更新,并返回产生连锁更新的最上层报单
    OrderInfoInEx * UpdateOrder(SerialNoType serial_no, char new_status)
    {
    	OrderInfoInEx *result = NULL;

        MY_LOG_DEBUG("UpdateOrder serial=%ld,status=%c", serial_no, new_status);

        std::lock_guard < std::mutex > lock(mutex_order_);

        OrderDatas::iterator it = order_datas_.find(serial_no);
        if (it != order_datas_.end()){
            OrderInfoInEx *updated_order = it->second;
            if (OrderUtil::IsTerminated(updated_order->entrust_status)){
            	result = NULL;
            }else{
            	updated_order->entrust_status = new_status;
            }
            result = updated_order;
        }else{
        	result = NULL;
            MY_LOG_ERROR("try to update an unknown order serial no: %ld", serial_no);
        }

        return result;
    }

    OrderInfoInEx * UpdateOrder(SerialNoType serial_no, char new_status,
    		EntrustNoType entrust_no, VolumeType volume_remain)
    {
    	OrderInfoInEx *result = NULL;

        MY_LOG_DEBUG("UpdateOrder serial=%ld,status=%c,volume_remain=%ld", serial_no, new_status, volume_remain);

        std::lock_guard < std::mutex > lock(mutex_order_);

        OrderDatas::iterator it = order_datas_.find(serial_no);
        if (it != order_datas_.end()){
        	result = it->second;
            VolumeType volume_changed = result->volume_remain - volume_remain;
            if (volume_changed < 0){
                MY_LOG_ERROR("order: %ld, original volume_remain=%ld, current volume_remain=%ld",
                    serial_no, result->volume_remain, volume_remain);
            }
            result->volume_remain = volume_remain;
            if (!OrderUtil::IsTerminated(result->entrust_status)){
            	result->entrust_status = new_status;
            }

            if (result->entrust_no == 0 && entrust_no != 0){
            	result->entrust_no = entrust_no;
            }
        }else{
        	result = NULL;
            MY_LOG_ERROR("try to update an unknown order serial no: %ld", serial_no);
        }

        return result;
    }

    // 根据序列号获取报单
    OrderInfoInEx *GetOrder(SerialNoType serial_no)
    {
        std::lock_guard < std::mutex > lock(mutex_order_);
        OrderDatas::iterator it = order_datas_.find(serial_no);
        if (it != order_datas_.end())
        {
            return (it->second);
        }

        MY_LOG_ERROR("GetOrder failed, serial_no=%ld", serial_no);
        return NULL;
    }

    inline SerialNoType GetNewSerialNo(SerialNoType super_sn)
    {
        return order_context_.GetNewSerialNo(super_sn);
    }

    VolumeType GetOpenRemainVolume(const std::string &code, char dir);

private:
    // forbid copy
    MYOrderDataManager(const MYOrderDataManager &);
    MYOrderDataManager & operator=(const MYOrderDataManager &);

    // 报单信息，及报单数据的管理同步变量
    std::mutex mutex_order_;
    OrderDatas order_datas_;

    // 报单上下文，辅助上下文信息维护
    OrderContext order_context_;
};

#endif
