#ifndef _MY_EXCHANGE_POSITION_DATA_H_
#define  _MY_EXCHANGE_POSITION_DATA_H_

#include "my_exchange_datatype_inner.h"
#include <mutex>

class MYPositionDataManager
{
public:
    // 初始化每个合约的多空仓位，系统启动时调用，通过通道查询回来
    // 表示一个资金账户的初始可用仓位
    void InitPosition(const std::string &code, char dir, VolumeType volume);

    // 读取合约在市场的仓位（包括冻结量）。
    // 报单、报价，回报时，要同时回报合约的仓位信息
    // 单个模型时，为模型在市场的仓位
    T_PositionData GetPositionInMarketOfCode(const std::string &code);

    // 获取指定合约的仓位相关信息：包括仓位，冻结仓位，盘口开仓量，等；
    // 报单、或报价时，判断是否会达到仓位上限时使用
    PositionInfoInEX * GetPositionOfCode(const std::string &code);

    // 内部撮合，更新仓位
    void UpdateInnerPosition(const OrderInfoInEx *order, const OrderInfoInEx *other_order, VolumeType matched_volume);

    // 变换单内部撮合，更新内部仓位
    void InnerMatchTransferOrder(const std::string &code, char oc, VolumeType matched_volume);

    // 在市场发生撮合，更新仓位
    void UpdateOutterPosition(const std::string &code, char dir, char open_close, VolumeType volume, bool is_transform);

    // 根据报单的方向、开平、量，以及资金账户中，该合约在市场的仓位，计算需要做开平变换的量
    VolumeType GetTransformVolume(OrderInfoInEx *p);

    // 开平变换的单，终结时，需要将剩余的量回滚
    void RollBackTransformVolume(const std::string &code, char dir, char oc, VolumeType remain_volume);

    // 报单终结时，需要将剩余的量回滚（平仓要回滚冻结的量，开仓要回滚盘口的量）
    void RollBackPendingVolume(const std::string &code, char dir, char oc, VolumeType remain_volume);

    // 报价单，根据当前仓位进行开平变换（需要全部变换，或者不变换，不能做部分量的变换）
    void CheckAndTransformQuote(QuoteInfoInEx *p);

private:
    PositionDataOfCode pos_datas_;
    std::mutex mutex_pos_;
};

#endif
