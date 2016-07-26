#pragma once

#include "pthread.h"
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <thread>
#include <map>
#include <mutex>

#include "SgitFtdcTraderApi.h"

#include "my_trade_tunnel_struct.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "config_data.h"
#include "check_schedule.h"
#include "sgit_field_convert.h"
#include "sgit_data_formater.h"
#include "trade_log_util.h"
#include "tunnel_cmn_utility.h"

class TunnelInitializer
{
public:
    TunnelInitializer(const TunnelConfigData &cfg, CSgitFtdcTraderApi *api);
    ~TunnelInitializer(void);

    void StartInit();

    bool Finished()
    {
        return finished_;
    }

    double GetAvailableFund()
    {
        return available_fund_;
    }
    const std::vector<PositionDetail> &GetPosition()
    {
        return pos_buffer_;
    }
    const std::vector<OrderDetail> &GetOrderInfo()
    {
        return od_buffer_;
    }
    const std::vector<TradeDetail> &GetTradeInfo()
    {
        return td_buffer_;
    }
    const std::vector<ContractInfo> &GetContractInfo()
    {
        return ci_buffer_;
    }

    void RecvRtnOrder(CSgitFtdcOrderField *pf, CSgitFtdcRspInfoField *rsp);
    void RecvRtnTrade(CSgitFtdcTradeField *pf);
    void RecvRspOrderAction(CSgitFtdcInputOrderActionField *pf, CSgitFtdcRspInfoField *rsp);
    void RecvRspQryTradingAccount(CSgitFtdcTradingAccountField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last);

    void RecvRspQryInstrument(CSgitFtdcInstrumentField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool last_flag);
    void RecvRspQryInvestorPosition(CSgitFtdcInvestorPositionField *pf, CSgitFtdcRspInfoField *rsp, int req_id, bool is_last);

private:
    CSgitFtdcTraderApi *api_;
    std::mutex sync_lock_;
    void TaskSchedule();
    int elapase_seconds_;
    void IncTimer()
    {
        std::lock_guard<std::mutex> lock(sync_lock_);
        ++elapase_seconds_;
    }
    bool IsTimeout(int s)
    {
        std::lock_guard<std::mutex> lock(sync_lock_);
        return elapase_seconds_ > s;
    }
    void ResetTimer()
    {
        std::lock_guard<std::mutex> lock(sync_lock_);
        elapase_seconds_ = 0;
    }

    std::string user_;
    std::size_t user_id_len_;
    std::string broker_id_;
    Tunnel_Info tunnel_info_;
    char exchange_code_;
    TSgitFtdcDateType trade_day;

    // set when all sub tasks finished
    bool finished_;

    bool finish_query_account_;
    bool finish_query_contract_;
    bool finish_query_position_;
    bool finish_cancel_active_orders_;
    bool finish_stats_cancel_times_;

    inline bool IsMineReturn(const char *user_id);
    inline bool IsTodayOperation(const char *op_date);
    inline bool IsErrRespond(CSgitFtdcRspInfoField *rsp);

    void QueryContractInfo();
    void QueryPosition();
    PositionDetail PackageAvailableFunc();

    // buffers
    double available_fund_;
    std::map<std::string, int> cancel_times_of_contract;
    bool NoActiveOrder();

    // cancel all unterminated orders
    void CancelAllActiveOrders();
    CSgitFtdcInputOrderActionField CreatCancelParam(OrderRefDataType order_ref, const OrderDetail & order_field);
    bool IsTerminateStatus(char entrust_status);

    // buffers of query results
    std::vector<PositionDetail> pos_buffer_;
    std::vector<OrderDetail> od_buffer_;
    std::vector<TradeDetail> td_buffer_;
    std::vector<ContractInfo> ci_buffer_;

    std::map<OrderRefDataType, OrderDetail> orders_;
    std::map<OrderRefDataType, CSgitFtdcOrderField> original_orders_;
    OrderDetail BuildOrderDetail(const CSgitFtdcOrderField& o);
    TradeDetail BuildTradeDetail(const CSgitFtdcTradeField& o);
    void AddOrder(const CSgitFtdcOrderField& o);
    void AddTrade(const CSgitFtdcTradeField& o);
    bool UpdateOrderStatus(OrderRefDataType order_ref, TMyTnlOrderStatusType s);
    bool UpdateOrderMatchedVolume(OrderRefDataType order_ref, int matched_vol);
};

inline bool TunnelInitializer::IsMineReturn(const char* user_id)
{
    return memcmp(user_id, user_.c_str(), user_id_len_) == 0;
}

inline bool TunnelInitializer::IsTodayOperation(const char* op_date)
{
    return memcmp(op_date, trade_day, sizeof(trade_day)) == 0;
}

inline bool TunnelInitializer::IsErrRespond(CSgitFtdcRspInfoField* rsp)
{
    return rsp && rsp->ErrorID != 0;
}

inline bool TunnelInitializer::NoActiveOrder()
{
    std::lock_guard<std::mutex> lock(sync_lock_);
    for (std::map<OrderRefDataType, OrderDetail>::value_type& v : orders_)
    {
        if (!IsTerminateStatus(v.second.entrust_status))
        {
            return false;
        }
    }
    return true;
}

inline bool TunnelInitializer::IsTerminateStatus(char entrust_status)
{
    return entrust_status == MY_TNL_OS_COMPLETED
        || entrust_status == MY_TNL_OS_WITHDRAWED
        || entrust_status == MY_TNL_OS_ERROR
    ;
}

inline OrderDetail TunnelInitializer::BuildOrderDetail(const CSgitFtdcOrderField& o)
{
    OrderDetail od;

    strncpy(od.stock_code, o.InstrumentID, sizeof(TSgitFtdcInstrumentIDType));
    od.entrust_no = atol(o.OrderSysID);
    od.order_kind = FieldConvert::GetMYPriceType(o.OrderPriceType);
    od.direction = o.Direction;
    od.open_close = o.CombOffsetFlag[0];
    od.speculator = FieldConvert::GetMYHedgeFlag(o.CombHedgeFlag[0]);
    od.entrust_status = MY_TNL_OS_REPORDED; // TODO value is 0 can't use it. FieldConvert::GetMYEntrustStatus(o.OrderSubmitStatus, o.OrderStatus);
    od.limit_price = o.LimitPrice;
    od.volume = o.VolumeTotalOriginal;
    od.volume_traded = 0;
    od.volume_remain = od.volume;

    return od;
}

inline TradeDetail TunnelInitializer::BuildTradeDetail(const CSgitFtdcTradeField& o)
{
    TradeDetail td;

    strncpy(td.stock_code, o.InstrumentID, sizeof(TSgitFtdcInstrumentIDType));
    td.entrust_no = atol(o.OrderSysID);
    td.direction = o.Direction;
    td.open_close = o.OffsetFlag;
    td.speculator = FieldConvert::GetMYHedgeFlag(o.HedgeFlag);
    td.trade_price = o.Price;
    td.trade_volume = o.Volume;
    strncpy(td.trade_time, o.TradeTime, sizeof(TSgitFtdcTimeType));

    return td;
}

inline void TunnelInitializer::AddOrder(const CSgitFtdcOrderField& o)
{
    std::lock_guard<std::mutex> lock(sync_lock_);
    OrderRefDataType order_ref = atoll(o.OrderRef);
    if (orders_.find(order_ref) != orders_.end())
    {
        TNL_LOG_WARN("duplicate order when resume, OrderRef:%s", o.OrderRef);
        return;
    }

    orders_.insert(std::make_pair(order_ref, BuildOrderDetail(o)));
    original_orders_.insert(std::make_pair(order_ref, o));
}

inline void TunnelInitializer::AddTrade(const CSgitFtdcTradeField& o)
{
    std::lock_guard<std::mutex> lock(sync_lock_);
    td_buffer_.push_back(BuildTradeDetail(o));
}

inline bool TunnelInitializer::UpdateOrderStatus(OrderRefDataType order_ref, TMyTnlOrderStatusType s)
{
    std::lock_guard<std::mutex> lock(sync_lock_);
    std::map<OrderRefDataType, OrderDetail>::iterator it = orders_.find(order_ref);
    if (it != orders_.end())
    {
        it->second.entrust_status = s;
        return true;
    }
    TNL_LOG_WARN("can't find order when update its status, OrderRef:%012lld", order_ref);
    return false;
}

inline bool TunnelInitializer::UpdateOrderMatchedVolume(OrderRefDataType order_ref, int matched_vol)
{
    std::lock_guard<std::mutex> lock(sync_lock_);
    std::map<OrderRefDataType, OrderDetail>::iterator it = orders_.find(order_ref);
    if (it != orders_.end())
    {
        it->second.volume_traded += matched_vol;
        it->second.volume_remain = it->second.volume - it->second.volume_traded;

        it->second.entrust_status = MY_TNL_OS_PARTIALCOM;
        if (it->second.volume_remain == 0) it->second.entrust_status = MY_TNL_OS_COMPLETED;

        return true;
    }
    TNL_LOG_WARN("can't find order when update its matched volume, OrderRef:%012lld", order_ref);
    return false;
}
