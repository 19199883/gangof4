#ifndef _MY_EXCHANGE_ORDER_DATA_H_
#define  _MY_EXCHANGE_ORDER_DATA_H_

#include "my_trade_tunnel_struct.h"
#include "my_exchange_datatype_inner.h"
#include "my_exchange_utility.h"

#include "my_order_hash.h"

#include <vector>
#include <mutex>

class OrderContext
{
public:
    OrderContext()
        : st_id_mask(100000000), inner_serial_start(9000000000000000)
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
        : m_order_hash(1000000)
    {
    }

    // 报单时，记录挂单信息
    bool AddOrder(OrderInfoInEx *place_order)
    {
		int ret;
        EX_LOG_DEBUG("MYOrderDataManager::AddOrder - %ld", place_order->serial_no);
        bool insert_success = false;
        {
            std::lock_guard < std::mutex > lock(mutex_order_);
			ret = m_order_hash.AddData(place_order->serial_no, place_order);
			insert_success = (ret == 0);
            //insert_success = order_datas_.insert(std::make_pair(place_order->serial_no, place_order)).second;
            unterminated_orders_.insert(place_order);
        }
        if (!insert_success)
        {
            // 插入失败了，记录日志
            EX_LOG_ERROR("AddOrder failed, order serial no: %ld", place_order->serial_no);
        }

        return insert_success;
    }

    // 报单更新,并返回产生连锁更新的最上层报单
    OrderInfoInEx * UpdateOrder(SerialNoType serial_no, char new_status)
    {
		OrderInfoInEx ** the_ord;
		OrderInfoInEx *updated_order;

        EX_LOG_DEBUG("UpdateOrder serial=%ld,status=%c", serial_no, new_status);
        std::lock_guard < std::mutex > lock(mutex_order_);

		the_ord = m_order_hash.GetData(serial_no);
		if (NULL == the_ord)
		{
			EX_LOG_ERROR("try to update an unknown order serial no: %ld", serial_no);
			return NULL;
		}

		updated_order = *the_ord;
		
        // 终结状态，不允许更新
        if (OrderUtil::IsTerminated(updated_order->entrust_status))
        {
            return NULL;
        }

        updated_order->entrust_status = new_status;
        // 终结状态要上传上层
        if (OrderUtil::IsTerminated(new_status))
        {
            unterminated_orders_.erase(updated_order);
            SerialNoType super_serial_no = updated_order->super_serial_no;
            OrderInfoInEx *upper_order = updated_order;
            while (super_serial_no != 0)
            {
                the_ord = m_order_hash.GetData(serial_no);
                if (the_ord != NULL)
                {
                    upper_order = *the_ord;
                }
                else
                {
                    EX_LOG_ERROR("AddOrder failed, order serial no: %ld", super_serial_no);
                    upper_order = NULL;
                    break;
                }

                // 如果所有变换单都终结了，将自身改成终结
                bool all_terminate = true;
                for (MYOrderSet::value_type v : upper_order->transform_orders)
                {
                    if (!OrderUtil::IsTerminated(v->entrust_status))
                    {
                        all_terminate = false;
                        break;
                    }
                }

                if (all_terminate)
                {
                    upper_order->entrust_status = new_status;
                    updated_order = upper_order;
                    unterminated_orders_.erase(updated_order);
                }
                else
                {
                    break;
                }

                super_serial_no = upper_order->super_serial_no;
            }
        }

        return updated_order;  
    }

    OrderInfoInEx * UpdateOrder(SerialNoType serial_no, char new_status, EntrustNoType entrust_no, VolumeType volume_remain, VolumeType &volume_changed)
    {
        OrderInfoInEx** the_ord;
        EX_LOG_DEBUG("UpdateOrder serial=%ld,status=%c,volume_remain=%ld", serial_no, new_status, volume_remain);
        std::lock_guard < std::mutex > lock(mutex_order_);
        volume_changed = 0;

        the_ord = m_order_hash.GetData(serial_no);
        if (NULL == the_ord)
        {
            EX_LOG_ERROR("try to update an unknown order serial no: %ld", serial_no);
            return NULL;
        }

        OrderInfoInEx *order = *the_ord;

        volume_changed = order->volume_remain - volume_remain;
        if (volume_changed < 0)
        {
            volume_changed = 0;
            EX_LOG_ERROR("order: %ld, original volume_remain=%ld, current volume_remain=%ld",
                serial_no, order->volume_remain, volume_remain);
        }
        order->volume_remain = volume_remain;

        // 终结状态，不允许更新
        if (!OrderUtil::IsTerminated(order->entrust_status))
        {
            order->entrust_status = new_status;
            if (OrderUtil::IsTerminated(order->entrust_status))
            {
                unterminated_orders_.erase(order);
            }
        }

        /* update entrust_no */
        if ((order->entrust_no == 0) && (entrust_no != 0))
        {
            order->entrust_no = entrust_no;
        }

        return UpdateUpperOrdersImp(order, volume_changed);
    }

    OrderInfoInEx * UpdateUpperOrders(OrderInfoInEx * order, VolumeType volume_changed)
    {
        EX_LOG_DEBUG("UpdateUpperOrders serial=%ld,status=%c,volume_changed=%ld", order->serial_no, order->entrust_status,
            volume_changed);
        std::lock_guard < std::mutex > lock(mutex_order_);
        return UpdateUpperOrdersImp(order, volume_changed);
    }

    // 根据序列号获取报单
    OrderInfoInEx *GetOrder(SerialNoType serial_no)
    {
        OrderInfoInEx** the_ord;

        std::lock_guard < std::mutex > lock(mutex_order_);
        
        the_ord = m_order_hash.GetData(serial_no);
        if (NULL == the_ord)
        {
            EX_LOG_ERROR("GetOrder failed, serial_no=%ld", serial_no);
            return NULL;
        }

        return *the_ord;
    }

    OrderInfoInEx *GetNewOrderOfInnerMatch(SerialNoType cancel_serial_no)
    {
        OrderInfoInEx** the_ord;
        std::lock_guard < std::mutex > lock(mutex_order_);
        MYSerialNoMap::const_iterator cit = inner_match_order_map_.find(cancel_serial_no);
        if (cit != inner_match_order_map_.end())
        {
            SerialNoType new_order_serial_no = cit->second;

            the_ord = m_order_hash.GetData(new_order_serial_no);
            if (the_ord != NULL)
            {
                return *the_ord;
            }
        }

        EX_LOG_ERROR("GetNewOrderOfInnerMatch failed, cancel order's serial_no=%ld", cancel_serial_no);
        return NULL;
    }

    // 待撤报单管理（当报单还没有收到委托号时，要做撤单操作，需要先记录，等收到委托号后，再撤单）
    bool ShouldAddToPendingCancel(OrderInfoInEx * po, T_CancelOrder &co)
    {
        bool should = false;
        std::lock_guard < std::mutex > lock(mutex_cancel_);

        should = (po->entrust_no == 0);
        co.entrust_no = po->entrust_no;

        if (should)
        {
            pending_cancel_orders_.insert(std::make_pair(po->serial_no, co));
        }

        return should;
    }

    void RemoveFromPendingCancel(SerialNoType po_serial_no)
    {
        std::lock_guard < std::mutex > lock(mutex_cancel_);
        PendingCancelOrders::iterator it = pending_cancel_orders_.find(po_serial_no);
        if (it != pending_cancel_orders_.end())
        {
            pending_cancel_orders_.erase(it);
        }
    }

    bool GetAndRemovePendingCancel(OrderInfoInEx * po, T_CancelOrder &co, EntrustNoType entrust_no)
    {
        std::lock_guard < std::mutex > lock(mutex_cancel_);
        po->entrust_no = entrust_no;
        PendingCancelOrders::iterator it = pending_cancel_orders_.find(po->serial_no);
        if (it != pending_cancel_orders_.end())
        {
            memcpy(&co, &(it->second), sizeof(T_CancelOrder));
            co.entrust_no = entrust_no;
            pending_cancel_orders_.erase(it);
            return true;
        }
        EX_LOG_DEBUG("order with entrust_no not in pending cancel collection, serial_no=%ld", po->serial_no);
        return false;
    }

    bool InPendingCancel(SerialNoType po_serial_no)
    {
        std::lock_guard < std::mutex > lock(mutex_cancel_);
        return pending_cancel_orders_.find(po_serial_no) != pending_cancel_orders_.end();
    }

    inline bool NoPendingCancel()
    {
        std::lock_guard < std::mutex > lock(mutex_cancel_);
        return pending_cancel_orders_.empty();
    }

    void CancelInnerMatching(OrderInfoInEx *po)
    {
        {
            //对还没有报出撤单的对手单，从PendingCancel集合清除
            std::lock_guard < std::mutex > lock(mutex_cancel_);
            for (SerialNoType matched_serial_no : po->inner_matched_orders)
            {
                pending_cancel_orders_.erase(matched_serial_no);
                inner_match_order_map_.erase(matched_serial_no);
            }
        }
        {
            std::lock_guard < std::mutex > lock(mutex_order_);
            po->inner_matched_orders.clear();
        }
    }

    // 是否原始报单的判断
    inline bool IsOriginalSerialNo(SerialNoType serial_no) const
    {
        return !order_context_.IsInnerSerialNo(serial_no);
    }
    inline SerialNoType GetNewSerialNo(SerialNoType super_sn)
    {
        return order_context_.GetNewSerialNo(super_sn);
    }

    // 内部撮合的报单关系数据管理
    // 插入需要内部撮合的挂单和新报单的关系
    inline bool InsertOrderSerialNoForInnerMatch(SerialNoType order_pending, SerialNoType new_order)
    {
        bool insert_success = false;
        {
            std::lock_guard < std::mutex > lock(mutex_order_);
            insert_success = inner_match_order_map_.insert(std::make_pair(order_pending, new_order)).second;
        }
        if (!insert_success)
        {
            // 插入失败了，记录日志
            EX_LOG_ERROR("insert match orders serial no failed, pending order: %ld, new order: %ld", order_pending, new_order);
        }

        return insert_success;
    }
    // 是否内部撮合的报单
    inline bool IsOrderSerialNoForInnerMatch(SerialNoType serial_no, SerialNoType &new_order_serial_no)
    {
        std::lock_guard < std::mutex > lock(mutex_order_);
        MYSerialNoMap::const_iterator cit = inner_match_order_map_.find(serial_no);
        if (cit != inner_match_order_map_.end())
        {
            new_order_serial_no = cit->second;
            return true;
        }

        return false;
    }
    // 删除报单的内部撮合关联关系
    inline bool RemoveOrderInnerMatchOrderRelation(SerialNoType cancel_serial_no, OrderInfoInEx * new_match_order)
    {
        std::lock_guard < std::mutex > lock(mutex_order_);
        new_match_order->inner_matched_orders.erase(cancel_serial_no);
        return inner_match_order_map_.erase(cancel_serial_no) > 0;
    }

    // 获取一个报单的变换单集合
    void GetTransformedOrders(OrderInfoInEx * order, OrderCollection &transformed_orders);

    // 相同合约，方向相反，价格相等或更好
    // 返回报单，价格好(low price of sell, high price of buy)的排前面，相同价格则报单号大(newer order)的排前面
    OrderCollection GetSelfTradeOrders(const OrderInfoInEx *place_order);

    VolumeType GetTransformRemainVolume(const std::string &code, char dir);
    VolumeType GetOpenRemainVolume(const std::string &code, char dir);

    bool HaveSelfTradeOrderInPendingCancel(const OrderInfoInEx *place_order);

    // added for support mm
    // insert quote，record new quote information
    bool AddQuote(QuoteInfoInEx *p)
    {
        EX_LOG_DEBUG("MYOrderDataManager::AddQuote - %d", p->data.serial_no);
        bool insert_success = false;
        {
            std::lock_guard < std::mutex > lock(mutex_quote_);
            insert_success = quote_datas_.insert(std::make_pair(p->data.serial_no, p)).second;
        }
        if (!insert_success)
        {
            // 插入失败了，记录日志
            EX_LOG_ERROR("AddQuote failed, order serial no: %d", p->data.serial_no);
        }

        return insert_success;
    }
    QuoteInfoInEx *GetQuote(SerialNoType serial_no)
    {
        std::lock_guard < std::mutex > lock(mutex_quote_);
        QuoteDatas::iterator it = quote_datas_.find(serial_no);
        if (it != quote_datas_.end())
        {
            return (it->second);
        }
        else
        {
            EX_LOG_ERROR("GetQuote failed, serial_no=%d", serial_no);
            return NULL;
        }
    }
    QuoteInfoInEx *UpdateAndGetQuote(SerialNoType serial_no, char status, EntrustNoType quote_entrust_no)
    {
        std::lock_guard < std::mutex > lock(mutex_quote_);
        QuoteDatas::iterator it = quote_datas_.find(serial_no);
        if (it != quote_datas_.end())
        {
            it->second->quote_status = status;
            it->second->quote_entrust_no = quote_entrust_no;
            return (it->second);
        }
        else
        {
            EX_LOG_ERROR("UpdateAndGetQuote - GetQuote failed, serial_no=%d", serial_no);
            return NULL;
        }
    }

    void UpdateQuote(SerialNoType serial_no, char dir, char status, EntrustNoType entrust_no, VolumeType vol_remain)
    {
        std::lock_guard < std::mutex > lock(mutex_quote_);
        QuoteDatas::iterator it = quote_datas_.find(serial_no);
        if (it != quote_datas_.end())
        {
            if (dir == MY_TNL_D_BUY)
            {
                it->second->buy_remain = vol_remain;
                it->second->buy_status = status;
                it->second->buy_entrust_no = entrust_no;
            }
            else
            {
                it->second->sell_remain = vol_remain;
                it->second->sell_status = status;
                it->second->sell_entrust_no = entrust_no;
            }
        }
        else
        {
            EX_LOG_ERROR("UpdateQuote - GetQuote failed, serial_no=%d", serial_no);
        }
    }

private:
    // forbid copy
    MYOrderDataManager(const MYOrderDataManager &);
    MYOrderDataManager & operator=(const MYOrderDataManager &);

    // 报单信息，及报单数据的管理同步变量
    std::mutex mutex_order_;
    
	MyOrderHash <OrderInfoInEx*> m_order_hash;
	//OrderDatas order_datas_;

    MYOrderSet unterminated_orders_;
    MYSerialNoMap inner_match_order_map_; // cancel order -> new place order

    // 撤单信息
    std::mutex mutex_cancel_;
    PendingCancelOrders pending_cancel_orders_;

    std::mutex mutex_quote_;
    QuoteDatas quote_datas_;

    // 报单上下文，辅助上下文信息维护
    OrderContext order_context_;

    void GetTransformedOrdersImp(OrderInfoInEx * order, OrderCollection &transformed_orders);

    OrderInfoInEx * UpdateUpperOrdersImp(OrderInfoInEx * order, VolumeType volume_changed);
};

#endif
