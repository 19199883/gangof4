#include <string.h>
#include "check_schedule.h"
#include "self_buy_sell_check.h"
#include "cancel_times_check.h"
#include "open_times_check.h"
#include "const_define.h"

static bool s_check_self_match = true;
static bool s_check_cancel_times = true;
static bool s_check_open_pos = true;
static bool s_check_self_match_in_all_acc = false;

void COMPLIANCE_CHECK_API_CALL ComplianceCheck_Init(
    const char * account,       // account
    int warn_threthold,			// 撤单警告阈值
    int max_cancel_times,		// 最大撤单次数（不可达到）
    int max_open_speculate,		// 最大投机手数
    int max_open_arbitrage,		// 最大套利手数
    int max_open_total,			// 最大总手数
    const char *switch_mask
    )
{
    s_check_self_match = true;
    s_check_cancel_times = true;
    s_check_open_pos = true;
    s_check_self_match_in_all_acc = false;

    // 合规检查是否打开的位掩码.从左开始，第1位:自成交；第2位:撤单次数；第3位:最大开仓数；无配置表示打开
    if (switch_mask)
    {
        if (strlen(switch_mask) > 0 && switch_mask[0] == '0')
        {
            s_check_self_match = false;
        }
        if (strlen(switch_mask) > 0 && switch_mask[0] == '2')
        {
            s_check_self_match_in_all_acc = true;
        }
        if (strlen(switch_mask) > 1 && switch_mask[1] == '0')
        {
            s_check_cancel_times = false;
        }
        if (strlen(switch_mask) > 2 && switch_mask[2] == '0')
        {
            s_check_open_pos = false;
        }
    }

    // 调用所有合规检查项的初始化接口（如果有的话）
    //if (s_check_self_match)
    {
        SelfBuySellCheck::Init();
    }

    //if (s_check_cancel_times)
    {
        CancelTimesCheck::Init(account, warn_threthold, max_cancel_times);
    }

    //if (s_check_open_pos)
    {
        OpenTimesCheck::Init(account, max_open_speculate, max_open_arbitrage, max_open_total);
    }
}

RESULT_TYPE COMPLIANCE_CHECK_API_CALL ComplianceCheck_TryReqOrderInsert(
    const char * account,       // account
    OrderRefDataType order_ref,	// 报单引用
    char exchange_code,			// 交易所编码
    const char * code, 			// 证券代码
    int volumn, 				// 数量
    double price,				// 价格
    char price_flag,			// 价格类型
    char hedge_flag,			// 投机套保类型
    char buy_sell_flag,			// 买卖方向
    char open_close_flag,		// 开平方向
    char order_type,			// 报单类型
    OrderRefDataType * opposite_serial_no	// 【输出】导致自成交对手单的报单引用
    )
{
    // 达到撤单告警阈值后，不允许开仓
    if (s_check_cancel_times)
    {
        if (order_type != CONST_ENTRUSTKIND_FAK
            && order_type != CONST_ENTRUSTKIND_FOK
            && open_close_flag == CONST_ENTRUST_OPEN
            && CancelTimesCheck::ReachWarnThreshold(account, code))
        {
            return COMPLIANCE_CHECK_RESULT_CANCEL_TIMES_REACH_WARN_THRETHOLD;
        }
    }

    RESULT_TYPE ret = COMPLIANCE_CHECK_RESULT_SUCCESS;

    // 开仓数检查、累加
    if (s_check_open_pos)
    {
        ret = OpenTimesCheck::TryReqOrderInsert(
            account,
            exchange_code,
            code,
            volumn,
            hedge_flag,
            open_close_flag,
            opposite_serial_no);
    }
    if (ret != COMPLIANCE_CHECK_RESULT_SUCCESS)
    {
        return ret;
    }

    // 自成交检查(需要先做其它验证，以免累加委托数据后，因其它原因不能成功时需要回滚)
    if (s_check_self_match_in_all_acc)
    {
        ret = SelfBuySellCheck::TryReqOrderInsertMerge(
            account,
            order_ref,
            code,
            price,
            price_flag,
            buy_sell_flag,
            order_type,
            opposite_serial_no);
    }
    else if (s_check_self_match)
    {
        ret = SelfBuySellCheck::TryReqOrderInsert(
            account,
            order_ref,
            code,
            price,
            price_flag,
            buy_sell_flag,
            order_type,
            opposite_serial_no);
    }

    if (ret != COMPLIANCE_CHECK_RESULT_SUCCESS)
    {
        // 开仓数回滚
        if (s_check_open_pos)
        {
            OpenTimesCheck::OnOrderInsertFailed(
                account,
                exchange_code,
                code,
                volumn,
                hedge_flag,
                open_close_flag);
        }

        return ret;
    }

    return ret;
}

void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderInsertFailed(
    const char * account,       // account
    OrderRefDataType order_ref,	// 报单引用
    char exchange_code,			// 交易所编码
    const char * code,          // 证券代码
    int volumn, 				// 数量
    char hedge_flag,			// 投机套保类型
    char open_close_flag,		// 开平方向
    char order_type 			// 报单类型
    )
{
    // 自成交检查
    if (s_check_self_match)
    {
        if (order_type != CONST_ENTRUSTKIND_FAK && order_type != CONST_ENTRUSTKIND_FOK)
        {
            SelfBuySellCheck::OnOrderInsertFailed(account, order_ref);
        }
    }

    // 开仓数检查
    if (s_check_open_pos)
    {
        OpenTimesCheck::OnOrderInsertFailed(
            account,
            exchange_code,
            code,
            volumn,
            hedge_flag,
            open_close_flag);
    }
}

void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderFilled(
    const char * account,       // account
    OrderRefDataType order_ref				// 报单引用
    )
{
    // 自成交检查
    if (s_check_self_match)
    {
        SelfBuySellCheck::OnOrderFilled(account, order_ref);
    }

    // 其它检查项
}

void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderCanceled(
    const char * account,       // account
    const char * code,          // 证券代码
    OrderRefDataType order_ref,	// 报单引用
    char exchange_code,			// 交易所编码
    int canceled_volumn,		// 撤单数量
    char hedge_flag,			// 投机套保类型
    char open_close_flag		// 开平方向
    )
{
    // 自成交检查
    // 在上报撤单请求时，告诉自成交模块，避免不必要的拦截。 modified by chenyongshun 20131114
    // 我们没有发撤单的时候，柜台可能会自行撤单，需要从委托列表中清除
    if (s_check_self_match)
    {
        SelfBuySellCheck::OnOrderCanceled(account, order_ref);
    }

    // 开仓数检查
    if (s_check_open_pos)
    {
        OpenTimesCheck::OnOrderCanceled(
            account,
            exchange_code,
            code,
            canceled_volumn,
            hedge_flag,
            open_close_flag);
    }

    if (s_check_cancel_times)
    {
        CancelTimesCheck::OnOrderCanceled(account, code, order_ref);
    }
}

RESULT_TYPE COMPLIANCE_CHECK_API_CALL ComplianceCheck_TryReqOrderAction(
    const char * account,       // account
    const char * code, 			// 证券代码
    OrderRefDataType order_ref  // 报单引用
    )
{
    RESULT_TYPE ret = COMPLIANCE_CHECK_RESULT_SUCCESS;

    // 撤单次数检查
    if (s_check_cancel_times)
    {
        ret = CancelTimesCheck::TryReqOrderAction(account, code, order_ref);
    }

    // 其它检查项

    return ret;
}

void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderCancelFailed(
    const char * account,       // account
    const char * code, 			// 证券代码
    OrderRefDataType order_ref  // 报单引用
    )
{
    // 撤单次数检查
    if (s_check_cancel_times)
    {
        CancelTimesCheck::OnOrderCancelFailed(account, code, order_ref);
    }

    // 其它检查项
}

// 在上报撤单请求时，告诉自成交模块，避免不必要的拦截。 modified by chenyongshun 20131114
void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderPendingCancel(
    const char * account,       // account
    OrderRefDataType order_ref	// 报单引用
    )
{
    // 自成交检查
    if (s_check_self_match)
    {
        SelfBuySellCheck::OnOrderCanceled(account, order_ref);
    }

    // 开仓数检查
}
