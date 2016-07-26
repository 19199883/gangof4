#ifndef REM_FIELD_CONVERT_H_
#define REM_FIELD_CONVERT_H_

#include "my_trade_tunnel_data_type.h"

#include <stdexcept>
#include "my_cmn_log.h"
#include "EesTraderDefine.h"

static EES_SideType s_side_table[2][4] =
    {
        { EES_SideType_open_long, EES_SideType_close_today_short, EES_SideType_close_today_short, EES_SideType_close_today_short },
        { EES_SideType_open_short, EES_SideType_close_today_long, EES_SideType_close_today_long, EES_SideType_close_today_long }
    };

//#define EES_HedgeFlag_Arbitrage             1           ///< 套利
//#define EES_HedgeFlag_Speculation           2           ///< 投机
//#define EES_HedgeFlag_Hedge                 3           ///< 套保
static EES_HedgeFlag s_hedge_table[3] =
    {
    EES_HedgeFlag_Speculation, EES_HedgeFlag_Hedge, EES_HedgeFlag_Arbitrage
    };

static char s_my_hedge_table[4] =
    {
        0, MY_TNL_HF_ARBITRAGE, MY_TNL_HF_SPECULATION, MY_TNL_HF_HEDGE
    };

//#define EES_SideType_open_long                  1       ///< =买单（开今）
//#define EES_SideType_close_today_long           2       ///< =卖单（平今）
//#define EES_SideType_close_today_short          3       ///< =买单（平今）
//#define EES_SideType_open_short                 4       ///< =卖单（开今）
//#define EES_SideType_close_ovn_short            5       ///< =买单（平昨）
//#define EES_SideType_close_ovn_long             6       ///< =卖单（平昨）
//#define EES_SideType_force_close_ovn_short      7       ///< =买单 （强平昨）
//#define EES_SideType_force_close_ovn_long       8       ///< =卖单 （强平昨）
//#define EES_SideType_force_close_today_short    9       ///< =买单 （强平今）
//#define EES_SideType_force_close_today_long     10      ///< =卖单 （强平今）
static char s_my_direction_table[11] =
    {
        0,
        MY_TNL_D_BUY, MY_TNL_D_SELL, MY_TNL_D_BUY, MY_TNL_D_SELL, MY_TNL_D_BUY,
        MY_TNL_D_SELL, MY_TNL_D_BUY, MY_TNL_D_SELL, MY_TNL_D_BUY, MY_TNL_D_SELL
    };

static char s_my_ocflag_table[11] =
    {
        0,
        MY_TNL_D_OPEN, MY_TNL_D_CLOSE, MY_TNL_D_CLOSE, MY_TNL_D_OPEN, MY_TNL_D_CLOSEYESTERDAY,
        MY_TNL_D_CLOSEYESTERDAY, MY_TNL_D_CLOSE, MY_TNL_D_CLOSE, MY_TNL_D_CLOSE, MY_TNL_D_CLOSE
    };

static long long zero_risk_res[12] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
namespace RemFieldConvert
{
    // 转换成REM字段取值
    inline EES_SideType GetRemSide(char dir, char oc)
    {
        if (dir - '0' < 0 || dir - '0' >= 2 || oc - '0' < 0 || oc - '0' >= 4)
        {
            TNL_LOG_ERROR("GetRemSide, parameter exceed range - dir:%c, oc:%c", dir, oc);
            throw std::invalid_argument("parameter exceed range");
        }
        return s_side_table[dir - '0'][oc - '0'];
    }

    inline EES_OrderTif GetRemCondition(char order_type)
    {
        if (order_type == MY_TNL_HF_NORMAL)
        {
            return EES_OrderTif_Day;
        }
        return EES_OrderTif_IOC;
    }

    inline EES_HedgeFlag GetRemHedgeFlag(char hedge_type)
    {
        if (hedge_type - '0' < 0 || hedge_type - '0' >= 3)
        {
            TNL_LOG_ERROR("GetRemHedgeFlag, parameter exceed range - hedge_type:%c", hedge_type);
            throw std::invalid_argument("parameter exceed range");
        }
        return s_hedge_table[hedge_type - '0'];
    }

    inline char GetMYStatus(EES_OrderStatus api_res)
    {
        //#define EES_OrderStatus_shengli_accept          0x80    ///< bit7=1：EES系统已接受
        //#define EES_OrderStatus_mkt_accept              0x40    ///< bit6=1：市场已接受或者手工干预订单
        //#define EES_OrderStatus_executed                0x20    ///< bit5=1：已成交或部分成交
        //#define EES_OrderStatus_cancelled               0x10    ///< bit4=1：已撤销, 可以是部分成交后撤销
        //#define EES_OrderStatus_cxl_requested           0x08    ///< bit3=1：发过客户撤单请求
        //#define EES_OrderStatus_reserved1               0x04    ///< bit2：保留, 目前无用
        //#define EES_OrderStatus_reserved2               0x02    ///< bit1：保留, 目前无用
        //#define EES_OrderStatus_closed                  0x01    ///< bit0=1：已关闭, (拒绝/全部成交/已撤销)
        if ((api_res & EES_OrderStatus_cancelled) == EES_OrderStatus_cancelled)
        {
            return MY_TNL_OS_WITHDRAWED;
        }
        if ((api_res & 0x21) == 0x21)
        {
            return MY_TNL_OS_COMPLETED;
        }
        if ((api_res & 0x21) == 0x20)
        {
            return MY_TNL_OS_PARTIALCOM;
        }
        if ((api_res & 0xc0) != 0)
        {
            return MY_TNL_OS_REPORDED;
        }
        if ((api_res & EES_OrderStatus_closed) == EES_OrderStatus_closed)
        {
            return MY_TNL_OS_ERROR;
        }

        return MY_TNL_OS_UNREPORT;
    }

    inline int GetMYErrorNo(EES_GrammerResult api_res, EES_RiskResult risk_res)
    {
        //  0   整体结果，下列任何原因造成拒绝，该位都会为1
        //  1   强平原因非法，目前只支持“0-非强平”
        //  2   交易所代码非法，目前只支持“102-中金所”
        //  3   不使用
        //  4   TIF不在合法值范围，目前只支持：EES_OrderTif_IOC(0) 和 EES_OrderTif_Day(99998)
        //  5   不使用
        //  6   委托价格必须>0
        //  7   不使用
        //  8   不使用
        //  9   交易品种不合法，目前只支持“3-期货”
        //  10  委托数量不合法，必须>0
        //  11  不使用
        //  12  买卖方向不合法，目前只支持1-6
        //  13  不使用
        //  14  对没有权限的账户进行操作
        //  15  委托编号重复
        //  16  不存在的账户
        //  17  不合法的合约代码
        //  18  委托总数超限，目前系统容量是每日最多850万个委托
        //  19  保留，不用关注
        //  20  资金账号未正确配置交易所编码
        //  21  m_MinQty的值超过了m_Qty
        //  22  当所有交易所连接都处于断开状态时，拒绝报单
        int ret = TUNNEL_ERR_CODE::RESULT_SUCCESS;

        if (api_res[0] != 0)
        {
            ret = TUNNEL_ERR_CODE::RESULT_FAIL;

            if (api_res[1] == 1 || api_res[2] == 1 || api_res[4] == 1 || api_res[6] == 1 || api_res[9] == 1 || api_res[10] == 1 || api_res[12] == 1)
                ret = TUNNEL_ERR_CODE::BAD_FIELD;

            else if (api_res[14] == 1 || api_res[16] == 1)
                ret = TUNNEL_ERR_CODE::NO_TRADING_RIGHT;

            else if (api_res[15] == 1)
                ret = TUNNEL_ERR_CODE::DUPLICATE_ORDER_REF;

            else if (api_res[17] == 1)
                ret = TUNNEL_ERR_CODE::INSTRUMENT_NOT_FOUND;

            else if (api_res[20] == 1 || api_res[22] == 1) ret = TUNNEL_ERR_CODE::NO_VALID_TRADER_AVAILABLE;
        }
        else if (memcmp(risk_res, zero_risk_res, sizeof(EES_RiskResult)) != 0)
        {
            // 委托指令的拒绝原因，风控检查结果对照表：
            //  0   订单手数限制
            //  1   订单占经纪商保证金额限制
            //  2   订单报价增额超限：与盘口价相比
            //  3   订单报价增额超限: 与最近成交价相比
            //  4   订单报价百分比超限:与盘口价相比
            //  5   订单报价百分比超限:与最近成交价相比
            //  6   订单报价增额超限：与昨结算价相比
            //  7   订单报价百分比超限：与昨结算价相比
            //  8   限价委托订单手数限制
            //  9   市价委托订单手数限制
            //  10  累计下订单发出次数限制
            //  11  累计下订单发出手数限制
            //  12  累计下订单发出金额限制
            //  13  在指定时间1内收到订单次数上限
            //  14  在指定时间2内收到订单次数上限
            //  15  禁止交易
            //  16  累计开仓订单发出次数限制
            //  17  累计平仓订单发出次数限制
            //  18  风控校验不通过次数限制
            //  19  客户权益核查
            //  20  总挂单金额校验
            //  21  最大撤单次数限制
            //  22  某合约最大撤单发出次数限制
            //  23  在指定时间1内撤单次数上限
            //  24  在指定时间2内撤单次数上限
            //  25  大额订单撤单次数限制
            //  26  累计持仓手数限制
            //  27  累计持仓占用保证金额总和限制
            //  28  累计成交手数限制
            //  29  成交金额总和限制
            //  30  下订单被市场拒绝次数的限制
            //  31  下单被柜台系统拒绝次数限制
            //  32  撤单被市场拒绝次数限制
            //  33  在指定时间1内下订单被市场拒绝次数上限
            //  34  在指定时间2内下订单被市场拒绝次数上限
            //  35  在指定时间1内撤单被市场拒绝次数上限
            //  36  在指定时间2内撤单被市场拒绝次数上限
            //  37  净盈亏限制
            //  38  浮动盈亏限制
            //  39  总盈亏限制
            //  40  持多仓手数限制
            //  41  持空仓手数限制
            //  42  持多仓占用保证金额限制
            //  43  持空仓占用保证金额限制
            //  44  某合约持多仓手数限制
            //  45  某合约持空仓手数限制
            //  46  某合约持多仓占用保证金额限制
            //  47  某合约持空仓占用保证金额限制
            //  48  某合约持仓总手数限制
            //  49  某合约持仓占保证金额总额限制
            //  50  某合约净收益限制
            //  51  某合约浮动盈亏限制
            //  52  某合约总收益限制
            //  53  累计开仓成交手数限制
            //  54  累计开仓成交金额总和限制
            //  55  累计开多仓成交手数限制
            //  56  累计开空仓成交手数限制
            //  57  累计开多仓成交金额总和限制
            //  58  累计开空仓成交金额总和限制
            //  59  经纪商风险度限制
            //  60  交易所风险度限制
            //  61  在指定时间1内下单被柜台系统拒绝次数上限
            //  62  在指定时间2内下单被柜台系统拒绝次数上限
            //  63  不使用
            //  64  可用资金不足
            //  65  可平仓位不足
            //  66  委托价格超过涨跌停范围

            ret = TUNNEL_ERR_CODE::RESULT_FAIL;

            if (risk_res[64] == 1)
                ret = TUNNEL_ERR_CODE::INSUFFICIENT_MONEY;
            else if (risk_res[65] == 1)
                ret = TUNNEL_ERR_CODE::OVER_CLOSE_POSITION;
            else if (risk_res[66] == 1)
                ret = TUNNEL_ERR_CODE::PRICE_OVER_LIMIT;
            else if (risk_res[8] == 1 || risk_res[9] == 1)
                ret = TUNNEL_ERR_CODE::REACH_UPPER_LIMIT_POSITION;
            else if (risk_res[21] == 1 || risk_res[22] == 1 || risk_res[25] == 1)
                ret = TUNNEL_ERR_CODE::CANCEL_REACH_LIMIT;
        }

        return ret;
    }

    inline int GetMYErrorNoOfRejectReason(unsigned int reason_code)
    {
        //  0   整体校验结果
        //  1   委托尚未被交易所接受
        //  2   要撤销的委托找不到
        //  3   撤销的用户名和委托的用户名不一致
        //  4   撤销的账户和委托的账户不一致
        //  5   委托已经关闭，如已经撤销/成交等
        //  6   重复撤单
        //  7   被动单不能被撤单
        int ret = TUNNEL_ERR_CODE::RESULT_SUCCESS;

        if ((reason_code & 0x01) == 0x01)
            ret = TUNNEL_ERR_CODE::RESULT_FAIL;

        if ((reason_code & 0x02) == 0x02 || (reason_code & 0x04) == 0x04)
            ret = TUNNEL_ERR_CODE::ORDER_NOT_FOUND;

        if ((reason_code & 0x20) == 0x20)
            ret = TUNNEL_ERR_CODE::UNSUITABLE_ORDER_STATUS;

        if ((reason_code & 0x40) == 0x40)
            ret = TUNNEL_ERR_CODE::DUPLICATE_ORDER_ACTION_REF;

        return ret;
    }

    inline char GetMYDirection(EES_SideType side)
    {
        if (side >= 11)
        {
            TNL_LOG_ERROR("GetMYDirection, parameter exceed range - side:%d", (int ) side);
            throw std::invalid_argument("parameter exceed range");
        }
        return s_my_direction_table[side];
    }

    inline char GetMYOCflag(EES_SideType side)
    {
        if (side >= 11)
        {
            TNL_LOG_ERROR("GetMYOCflag, parameter exceed range - side:%d", (int ) side);
            throw std::invalid_argument("parameter exceed range");
        }
        return s_my_ocflag_table[side];
    }

    inline char GetMYHedgeFlag(EES_HedgeFlag hedge_type)
    {
        if (hedge_type > 3)
        {
            TNL_LOG_ERROR("GetMYHedgeFlag, parameter exceed range - hedge_type:%d", (int ) hedge_type);
            throw std::invalid_argument("parameter exceed range");
        }
        return s_my_hedge_table[hedge_type];
    }
}

#endif
