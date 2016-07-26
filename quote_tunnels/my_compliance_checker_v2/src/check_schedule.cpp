#include "check_schedule.h"

#include <string.h>
#include <unordered_map>
#include <mutex>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "const_define.h"
#include "my_cmn_util_funcs.h"

using namespace std;

enum CommodityIndex
{
    //具体品种
    IF_IDX = 0,
    IH_IDX,
    IC_IDX,
    //品种数目
    IDX_SIZE,
};

enum OrderStatus
{
    PENDING = 0,                //已下单
    PENDING_CANCEL,      //正在撤单
    CANCELED,                   //已撤单
    FILLED,                         //成交
};

struct AccountInfo
{
    //合规检测开关
    bool self_match_check;
    bool cancal_times_check;
    bool open_times_check;
    bool self_match_check_in_all;
    //最大数限制
    int warn_threthold_cancel_times;
    int max_cancel_times;
    int max_open_speculate_times;
    int max_open_arbitrage_times;
    int max_open_total_times;
    // 当前开仓统计数
    int cur_speculate_open_times[IDX_SIZE];
    int cur_arbitrage_open_times[IDX_SIZE];
    int cur_total_open_times[IDX_SIZE];

    AccountInfo()
    {
        self_match_check = true;
        cancal_times_check = true;
        open_times_check = true;
        self_match_check_in_all = false;
        warn_threthold_cancel_times = 0;
        max_cancel_times = 0;
        max_open_speculate_times = 0;
        max_open_arbitrage_times = 0;
        max_open_total_times = 0;

        for (int i = 0; i < IDX_SIZE; i++) {
            cur_speculate_open_times[i] = 0;
            cur_arbitrage_open_times[i] = 0;
            cur_total_open_times[i] = 0;
        }
    }
};

struct ContractInfo
{
    double cur_buy_price;
    double cur_sell_price;
    OrderRefDataType cur_buy_ref;
    OrderRefDataType cur_sell_ref;

    ContractInfo() {
        cur_buy_price = 0;
        cur_sell_price = 0;
        cur_buy_ref = 0;
        cur_sell_ref = 0;
    }
};

struct AccountContractInfo
{
    //当前的统计数
    int cur_cancel_times;

    //当前的最有效买卖价格及单号
    double cur_buy_price;
    double cur_sell_price;
    OrderRefDataType cur_buy_ref;
    OrderRefDataType cur_sell_ref;
    //最大数值限制
    int warn_threthold_cancel_times;
    int max_cancel_times;

    //合规检查开关
    bool self_match_check;
    bool cancal_times_check;
    bool open_times_check;
    bool self_match_check_in_all;

    AccountContractInfo(const AccountInfo &other)
    {
        memset(this, 0, sizeof(AccountContractInfo));
        warn_threthold_cancel_times = other.warn_threthold_cancel_times;
        max_cancel_times = other.max_cancel_times;
        self_match_check = other.self_match_check;
        cancal_times_check = other.cancal_times_check;
        open_times_check = other.open_times_check;
        self_match_check_in_all = other.self_match_check_in_all;
    }
};

struct OrderInfo
{
    //订单信息
    string account;
    string stock;
    OrderStatus status;
    char order_type;
    char buy_sell_flag;
    double price;
    int volume;

    OrderInfo(const string &ac, const string &code, const char ot, const char dir, const double p, const int vol)
    {
        account = ac;
        stock = code;
        order_type = ot;
        buy_sell_flag = dir;
        price = p;
        volume = vol;
        status = PENDING;
    }
};

struct hashing_string_func
{
    unsigned long operator()(string str) const
    {
        unsigned long hash = 5381;

        for (int i = 0; i < str.length(); i++)
        {
            hash += (hash << 5) + str[i];
        }

        return (hash & 0x7FFFFFFF);
    }
};

struct hashing_long_func
{
    unsigned long operator()(OrderRefDataType n) const
    {
        return /*( n / 100000000)*/n % 1000000;
    }
};

typedef unordered_map<string, AccountInfo> AccountInfoMap;
typedef unordered_map<string, ContractInfo> ContractInfoMap;
typedef unordered_map<string, AccountContractInfo> AccountContractInfoMap;
typedef unordered_map<OrderRefDataType, OrderInfo, hashing_long_func> OrderInfoMap;

typedef pthread_spinlock_t MyMutex;
typedef _TTSpinLockGuard MyLock;

static AccountInfoMap account_infomap;
static ContractInfoMap contract_infomap;
static AccountContractInfoMap account_contract_infomap;
static OrderInfoMap order_infomap;

static mutex check_mutex;

/* 获取品种的对应数组下标 */
inline int GetCommodityIndex(const string &code)
{
    if (code[1] == 'F')
    {
        return IF_IDX;
    }
    if (code[1] == 'C')
    {
        return IC_IDX;
    }
    if (code[1] == 'H')
    {
        return IH_IDX;
    }
    return -1;
}

static void ResetCurPriceAndOrder(const AccountContractInfoMap::iterator &it, const string &account, const string &code, const char buy_sell_flag)
{
    if (buy_sell_flag == CONST_ENTRUST_BUY) {
        it->second.cur_buy_price = 0;
    } else {
        it->second.cur_sell_price = 0;
    }
    for (auto oit = order_infomap.begin(); oit != order_infomap.end(); oit++)
    {
        if (oit->second.status != PENDING)
        {
            continue;
        }
        if (buy_sell_flag == CONST_ENTRUST_BUY)
        {
            if (oit->second.account == account
                && oit->second.stock == code
                && oit->second.buy_sell_flag == CONST_ENTRUST_BUY
                && oit->second.price >= it->second.cur_buy_price)
            {
                it->second.cur_buy_price = oit->second.price;
                it->second.cur_buy_ref = oit->first;
            }
        }
        else if (oit->second.account == account
            && oit->second.stock == code
            && oit->second.buy_sell_flag == CONST_ENTRUST_SELL
            && it->second.cur_sell_price != 0
            && oit->second.price <= it->second.cur_sell_price)
        {
            it->second.cur_sell_price = oit->second.price;
            it->second.cur_sell_ref = oit->first;
        }
    }
}

static void ResetCurPriceAndOrder(const ContractInfoMap::iterator &it, const string &code, const char buy_sell_flag)
{
    if (buy_sell_flag == CONST_ENTRUST_BUY) {
        it->second.cur_buy_price = 0;
    } else {
        it->second.cur_sell_price = 0;
    }
    for (auto oit = order_infomap.begin(); oit != order_infomap.end(); oit++)
    {
        if (oit->second.status != PENDING)
        {
            continue;
        }
        if (buy_sell_flag == CONST_ENTRUST_BUY)
        {
            if (oit->second.stock == code
                && oit->second.buy_sell_flag == CONST_ENTRUST_BUY
                && oit->second.price >= it->second.cur_buy_price)
            {
                it->second.cur_buy_price = oit->second.price;
                it->second.cur_buy_ref = oit->first;
            }
        }
        else if (oit->second.stock == code
            && oit->second.buy_sell_flag == CONST_ENTRUST_SELL
            && it->second.cur_sell_price != 0
            && oit->second.price <= it->second.cur_sell_price)
        {
            it->second.cur_sell_price = oit->second.price;
            it->second.cur_sell_ref = oit->first;
        }
    }
}

void COMPLIANCE_CHECK_API_CALL ComplianceCheck_Init(
    const char * account,       // account
    int warn_threthold,			// 撤单警告阈值
    int max_cancel,		// 最大撤单次数（不可达到）
    int max_open_speculate,		// 最大投机手数
    int max_open_arbitrage,		// 最大套利手数
    int max_open_total,			// 最大总手数
    const char *switch_mask
    )
{
    lock_guard<std::mutex> lock(check_mutex);
    //pthread_spin_init(&account_mutex , 0);
    //pthread_spin_init(&account_contract_mutex, 0);
    //pthread_spin_init(&order_mutex, 0);

    auto it = account_infomap.find(account);
// 如果不存在当前用户则添加到用户信息表里
    if (it == account_infomap.end())
    {
        AccountInfo new_item;
        new_item.max_cancel_times = max_cancel;
        new_item.max_open_arbitrage_times = max_open_arbitrage;
        new_item.max_open_speculate_times = max_open_speculate;
        new_item.max_open_total_times = max_open_total;
        new_item.warn_threthold_cancel_times = warn_threthold;

        if (switch_mask)
        {
            if (strlen(switch_mask) > 0 && switch_mask[0] == '0')
            {
                new_item.self_match_check = false;
            }
            if (strlen(switch_mask) > 0 && switch_mask[0] == '2')
            {
                new_item.self_match_check_in_all = true;
            }
            if (strlen(switch_mask) > 1 && switch_mask[1] == '0')
            {
                new_item.cancal_times_check = false;
            }
            if (strlen(switch_mask) > 2 && switch_mask[2] == '0')
            {
                new_item.open_times_check = false;
            }
        }
        account_infomap.insert(make_pair(account, new_item));
    }
}

/* 撤单数合规检查 */
static int CancelTimesInsertCheck(const AccountContractInfoMap::iterator &it, const char order_type, const char open_close_flag)
{
    if (it->second.cancal_times_check)
    {
        if (order_type != CONST_ENTRUSTKIND_FAK
            && order_type != CONST_ENTRUSTKIND_FOK
            && open_close_flag == CONST_ENTRUST_OPEN)
        {
            if (it->second.cur_cancel_times > it->second.warn_threthold_cancel_times)
            {
                return COMPLIANCE_CHECK_RESULT_CANCEL_TIMES_REACH_WARN_THRETHOLD;
            }
            else if (it->second.cur_cancel_times == it->second.warn_threthold_cancel_times)
            {
                return COMPLIANCE_CHECK_RESULT_CANCEL_TIMES_EQUAL_WARN_THRETHOLD;
            }
        }
    }

    return COMPLIANCE_CHECK_RESULT_SUCCESS;
}

/* 开仓数合规检查 */
static int OpenTimesInsertCheck(const AccountContractInfoMap::iterator &acit, const string &account, const char exchange_code,
    const char open_close_flag,
    const char hedge_flag, const string &code, const int volume)
{
    if (acit->second.open_times_check)
    {
        if (exchange_code == CONST_EXCHCODE_CFFEX
            && open_close_flag == CONST_ENTRUST_OPEN)
        {
            auto ait = account_infomap.find(account);
            if (ait != account_infomap.end())
            {
                int index = GetCommodityIndex(code);
                if (index != -1)
                {
                    if (hedge_flag == CONST_SHFLAG_TOU)
                    {
                        if (ait->second.cur_speculate_open_times[index] + volume > ait->second.max_open_speculate_times
                            || ait->second.cur_total_open_times[index] + volume > ait->second.max_open_total_times)
                        {
                            return COMPLIANCE_CHECK_RESULT_OPEN_REACH_EXCEED_LIMIT;
                        }
                        else if (ait->second.cur_speculate_open_times[index] + volume == ait->second.max_open_speculate_times
                            || ait->second.cur_total_open_times[index] + volume == ait->second.max_open_total_times)
                        {
                            return COMPLIANCE_CHECK_RESULT_OPEN_EQUAL_LIMIT;
                        }
                    }
                    else if (hedge_flag == CONST_SHFLAG_TAO)
                    {
                        if (ait->second.cur_arbitrage_open_times[index] + volume > ait->second.max_open_arbitrage_times
                            || ait->second.cur_total_open_times[index] + volume > ait->second.max_open_total_times)
                        {
                            return COMPLIANCE_CHECK_RESULT_OPEN_REACH_EXCEED_LIMIT;
                        }
                        else if (ait->second.cur_arbitrage_open_times[index] + volume == ait->second.max_open_arbitrage_times
                            || ait->second.cur_total_open_times[index] + volume == ait->second.max_open_total_times)
                        {
                            return COMPLIANCE_CHECK_RESULT_OPEN_EQUAL_LIMIT;
                        }
                    }
                }
            }
        }
    }

    return COMPLIANCE_CHECK_RESULT_SUCCESS;
}

/* 自成交合规检查 */
static int SelfTradeInsertCheck(const AccountContractInfoMap::iterator &it, const string &account, const string &code, const char price_flag,
    const char buy_sell_flag, const double price, OrderRefDataType * opposite_serial_no)
{
    int ret = COMPLIANCE_CHECK_RESULT_SUCCESS;

    //多账户内的自成交检查
    if (it->second.self_match_check_in_all)
    {
        //如果不是限价单，只要方向相反就拦截
        auto cit = contract_infomap.find(code);
        if (cit != contract_infomap.end())
        {
            if (price_flag != CONST_ENTRUSTKIND_XJ)
            {
                if (buy_sell_flag == CONST_ENTRUST_BUY && cit->second.cur_sell_price != 0)
                {
                    *opposite_serial_no = cit->second.cur_sell_ref;
                    return COMPLIANCE_CHECK_RESULT_SELFTRADE;   //返回自成交的错误码
                }
                else if (buy_sell_flag == CONST_ENTRUST_SELL && cit->second.cur_buy_price != 0)
                {
                    *opposite_serial_no = cit->second.cur_buy_ref;
                    return COMPLIANCE_CHECK_RESULT_SELFTRADE;
                }
            }
            else
            {
                //如果是限价单，判断买卖的价格是否有冲突，有冲突则返回错误
                if (buy_sell_flag == CONST_ENTRUST_BUY
                    && cit->second.cur_sell_price != 0
                    && price >= cit->second.cur_sell_price)
                {
                    *opposite_serial_no = cit->second.cur_sell_ref;
                    return COMPLIANCE_CHECK_RESULT_SELFTRADE;
                }
                else if (buy_sell_flag == CONST_ENTRUST_SELL
                    && cit->second.cur_buy_price != 0
                    && price <= cit->second.cur_buy_price)
                {
                    *opposite_serial_no = cit->second.cur_buy_ref;
                    return COMPLIANCE_CHECK_RESULT_SELFTRADE;
                }
            }
        }
    }
    //单账户内的自成交检查，在前面多加了账户判断，其余逻辑与多账户的一致
    else if (it->second.self_match_check)
    {
        if (price_flag != CONST_ENTRUSTKIND_XJ)
        {
            if (buy_sell_flag == CONST_ENTRUST_BUY
                && it->second.cur_sell_price != 0)
            {
                *opposite_serial_no = it->second.cur_sell_ref;
                return COMPLIANCE_CHECK_RESULT_SELFTRADE;
            }
            else if (buy_sell_flag == CONST_ENTRUST_SELL
                && it->second.cur_buy_price != 0)
            {
                *opposite_serial_no = it->second.cur_buy_ref;
                return COMPLIANCE_CHECK_RESULT_SELFTRADE;
            }
        }
        else
        {
            if (buy_sell_flag == CONST_ENTRUST_BUY
                && it->second.cur_sell_price != 0
                && price >= it->second.cur_sell_price)
            {
                *opposite_serial_no = it->second.cur_sell_ref;
                return COMPLIANCE_CHECK_RESULT_SELFTRADE;
            }
            else if (buy_sell_flag == CONST_ENTRUST_SELL
                && it->second.cur_buy_price != 0
                && price <= it->second.cur_buy_price)
            {
                *opposite_serial_no = it->second.cur_buy_ref;
                return COMPLIANCE_CHECK_RESULT_SELFTRADE;
            }
        }
    }

    return ret;
}

static int SelfTradeInsertCheck(const string &account, const string &code, const char price_flag, const char buy_sell_flag, const double price,
    OrderRefDataType * opposite_serial_no)
{
    int ret = COMPLIANCE_CHECK_RESULT_SUCCESS;

    //多账户内的自成交检查
    //如果不是限价单，只要方向相反就拦截
    auto cit = contract_infomap.find(code);
    if (cit != contract_infomap.end())
    {
        if (price_flag != CONST_ENTRUSTKIND_XJ)
        {
            if (buy_sell_flag == CONST_ENTRUST_BUY && cit->second.cur_sell_price != 0)
            {
                *opposite_serial_no = cit->second.cur_sell_ref;
                return COMPLIANCE_CHECK_RESULT_SELFTRADE;   //返回自成交的错误码
            }
            else if (buy_sell_flag == CONST_ENTRUST_SELL && cit->second.cur_buy_price != 0)
            {
                *opposite_serial_no = cit->second.cur_buy_ref;
                return COMPLIANCE_CHECK_RESULT_SELFTRADE;
            }
        }
        else
        {
            //如果是限价单，判断买卖的价格是否有冲突，有冲突则返回错误
            if (buy_sell_flag == CONST_ENTRUST_BUY
                && cit->second.cur_sell_price != 0
                && price >= cit->second.cur_sell_price)
            {
                *opposite_serial_no = cit->second.cur_sell_ref;
                return COMPLIANCE_CHECK_RESULT_SELFTRADE;
            }
            else if (buy_sell_flag == CONST_ENTRUST_SELL
                && cit->second.cur_buy_price != 0
                && price <= cit->second.cur_buy_price)
            {
                *opposite_serial_no = cit->second.cur_buy_ref;
                return COMPLIANCE_CHECK_RESULT_SELFTRADE;
            }
        }
    }

    return ret;
}

static int NewOrderInsertCheck(const AccountInfoMap::iterator &it, const char exchange_code, const char open_close_flag, const char hedge_flag,
    const int volume)
{
    int ret = COMPLIANCE_CHECK_RESULT_SUCCESS;
    if (it->second.open_times_check
        && open_close_flag == CONST_ENTRUST_OPEN
        && exchange_code == CONST_EXCHCODE_CFFEX)
    {
        if (hedge_flag == CONST_SHFLAG_TOU)
        {
            if (volume > it->second.max_open_speculate_times)
            {
                return COMPLIANCE_CHECK_RESULT_OPEN_REACH_EXCEED_LIMIT;
            }
            else if (volume == it->second.max_open_speculate_times)
            {
                return COMPLIANCE_CHECK_RESULT_OPEN_EQUAL_LIMIT;
            }
        }
        else if (hedge_flag == CONST_SHFLAG_TAO)
        {
            if (volume > it->second.max_open_arbitrage_times)
            {
                return COMPLIANCE_CHECK_RESULT_OPEN_REACH_EXCEED_LIMIT;
            }
            else if (volume == it->second.max_open_arbitrage_times)
            {
                return COMPLIANCE_CHECK_RESULT_OPEN_EQUAL_LIMIT;
            }
        }
    }
    return ret;
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
    char key[64];
    sprintf(key, "%s_%s", account, code);

    lock_guard<std::mutex> lock(check_mutex);

    RESULT_TYPE ret = COMPLIANCE_CHECK_RESULT_SUCCESS;
    auto acit = account_contract_infomap.find(key);

    if (acit != account_contract_infomap.end())
    {
        //撤单数检查
        ret = CancelTimesInsertCheck(acit, order_type, open_close_flag);

        if (ret != COMPLIANCE_CHECK_RESULT_SUCCESS &&
            ret != COMPLIANCE_CHECK_RESULT_CANCEL_TIMES_EQUAL_WARN_THRETHOLD)
        {
            return ret;
        }

        //开仓数检查
        int tmp_ret;
        tmp_ret = OpenTimesInsertCheck(acit, account, exchange_code, open_close_flag, hedge_flag, code, volumn);

        if (tmp_ret != COMPLIANCE_CHECK_RESULT_SUCCESS &&
            tmp_ret != COMPLIANCE_CHECK_RESULT_OPEN_EQUAL_LIMIT)
        {
            return tmp_ret;
        }
        else if (ret == COMPLIANCE_CHECK_RESULT_SUCCESS &&
            tmp_ret == COMPLIANCE_CHECK_RESULT_OPEN_EQUAL_LIMIT)
        {
            ret = tmp_ret;
        }

        //自成交检查

        tmp_ret = SelfTradeInsertCheck(acit, account, code, price_flag, buy_sell_flag, price, opposite_serial_no);

        if (tmp_ret != COMPLIANCE_CHECK_RESULT_SUCCESS)
        {
            return tmp_ret;
        }

        //成功下单，录入操作
        if (open_close_flag == CONST_ENTRUST_OPEN && exchange_code == CONST_EXCHCODE_CFFEX)
        {
            int index = GetCommodityIndex(code);
            if (index != -1)
            {
                auto ait = account_infomap.find(account);
                if (ait != account_infomap.end()) {
                    if (hedge_flag == CONST_SHFLAG_TOU)
                    {
                        ait->second.cur_speculate_open_times[index] += volumn;
                        ait->second.cur_total_open_times[index] += volumn;
                    }
                    else if (hedge_flag == CONST_SHFLAG_TAO)
                    {
                        ait->second.cur_arbitrage_open_times[index] += volumn;
                        ait->second.cur_total_open_times[index] += volumn;
                    }
                }
            }
        }

        if (order_type != CONST_ENTRUSTKIND_FAK
            && order_type != CONST_ENTRUSTKIND_FOK
            && order_infomap.find(order_ref) == order_infomap.end())
        {
            auto cit = contract_infomap.find(code);
            if (cit != contract_infomap.end()) {
                if (buy_sell_flag == CONST_ENTRUST_BUY && price >= cit->second.cur_buy_price)
                {
                    cit->second.cur_buy_price = price;
                    cit->second.cur_buy_ref = order_ref;
                }
                else if (buy_sell_flag == CONST_ENTRUST_SELL && price >= cit->second.cur_buy_price)
                {
                    cit->second.cur_sell_price = price;
                    cit->second.cur_sell_ref = order_ref;
                }
            }
            if (buy_sell_flag == CONST_ENTRUST_BUY && price >= acit->second.cur_buy_price)
            {
                acit->second.cur_buy_price = price;
                acit->second.cur_buy_ref = order_ref;
            }
            else if (buy_sell_flag == CONST_ENTRUST_SELL && price >= acit->second.cur_buy_price)
            {
                acit->second.cur_sell_price = price;
                acit->second.cur_sell_ref = order_ref;
            }
            OrderInfo item(account, code, order_type, buy_sell_flag, price, volumn);
            order_infomap.insert(make_pair(order_ref, item));
        }
    }
    else
    {
        auto ait = account_infomap.find(account);
        if (ait != account_infomap.end())
        {
            ret = NewOrderInsertCheck(ait, exchange_code, open_close_flag, hedge_flag, volumn);
            if (ret != COMPLIANCE_CHECK_RESULT_OPEN_REACH_EXCEED_LIMIT)
            {
                int tmp_ret = COMPLIANCE_CHECK_RESULT_SUCCESS;
                if (ait->second.self_match_check_in_all)
                {
                    tmp_ret = SelfTradeInsertCheck(account, code, price_flag, buy_sell_flag, price, opposite_serial_no);
                }
                if (tmp_ret != COMPLIANCE_CHECK_RESULT_SUCCESS)
                {
                    return tmp_ret;
                }

                AccountContractInfo item(ait->second);
                if (open_close_flag == CONST_ENTRUST_OPEN && exchange_code == CONST_EXCHCODE_CFFEX)
                {
                    int index = GetCommodityIndex(code);
                    if (index != -1)
                    {
                        if (hedge_flag == CONST_SHFLAG_TOU)
                        {
                            ait->second.cur_speculate_open_times[index] += volumn;
                            ait->second.cur_total_open_times[index] += volumn;
                        }
                        else if (hedge_flag == CONST_SHFLAG_TAO)
                        {
                            ait->second.cur_arbitrage_open_times[index] += volumn;
                            ait->second.cur_total_open_times[index] += volumn;
                        }
                    }
                }
                if (order_type != CONST_ENTRUSTKIND_FAK
                    && order_type != CONST_ENTRUSTKIND_FOK)
                {
                    auto cit = contract_infomap.find(code);
                    if (cit != contract_infomap.end()) {
                        if (buy_sell_flag == CONST_ENTRUST_BUY && price >= cit->second.cur_buy_price)
                        {
                            cit->second.cur_buy_price = price;
                            cit->second.cur_buy_ref = order_ref;
                        }
                        else if (buy_sell_flag == CONST_ENTRUST_SELL && price >= cit->second.cur_buy_price)
                        {
                            cit->second.cur_sell_price = price;
                            cit->second.cur_sell_ref = order_ref;
                        }
                    } else {
                        ContractInfo citem;
                        if (buy_sell_flag == CONST_ENTRUST_BUY) {
                            citem.cur_buy_price = price;
                            citem.cur_buy_ref = order_ref;
                        } else if (buy_sell_flag == CONST_ENTRUST_SELL){
                            citem.cur_sell_price = price;
                            citem.cur_sell_ref = order_ref;
                        }
                        contract_infomap.insert(make_pair(code, citem));
                    }
                    if (buy_sell_flag == CONST_ENTRUST_BUY)
                    {
                        item.cur_buy_price = price;
                        item.cur_buy_ref = order_ref;
                    }
                    else
                    {
                        item.cur_sell_price = price;
                        item.cur_sell_ref = order_ref;
                    }
                }
                account_contract_infomap.insert(make_pair(key, item));
            }

            if (order_type != CONST_ENTRUSTKIND_FAK
                && order_type != CONST_ENTRUSTKIND_FOK
                && order_infomap.find(order_ref) == order_infomap.end())
            {

                OrderInfo item(account, code, order_type, buy_sell_flag, price, volumn);
                order_infomap.insert(make_pair(order_ref, item));
            }

        }
        else
        {
            return COMPLIANCE_CHECK_RESULT_FAIL;
        }
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
    char key[64];
    sprintf(key, "%s_%s", account, code);

    lock_guard<std::mutex> lock(check_mutex);

    if (order_type != CONST_ENTRUSTKIND_FAK && order_type != CONST_ENTRUSTKIND_FOK)
    {
        order_infomap.erase(order_ref);
    }

    auto cit = contract_infomap.find(code);
    if (cit != contract_infomap.end())
    {
        if (cit->second.cur_buy_ref == order_ref)
        {
            ResetCurPriceAndOrder(cit, code, CONST_ENTRUST_BUY);
        }
        else if (cit->second.cur_sell_ref == order_ref)
        {
            ResetCurPriceAndOrder(cit, code, CONST_ENTRUST_SELL);
        }
    }

    auto it = account_contract_infomap.find(key);
    if (it != account_contract_infomap.end())
    {
        if (exchange_code == CONST_EXCHCODE_CFFEX)
        {

            int index = GetCommodityIndex(code);
            if (index != -1)
            {
                auto ait = account_infomap.find(account);
                if (ait != account_infomap.end()) {
                    if (hedge_flag == CONST_SHFLAG_TOU)
                    {
                        ait->second.cur_speculate_open_times[index] -= volumn;
                        ait->second.cur_total_open_times[index] -= volumn;
                    }
                    else if (hedge_flag == CONST_SHFLAG_TAO)
                    {
                        ait->second.cur_arbitrage_open_times[index] -= volumn;
                        ait->second.cur_total_open_times[index] -= volumn;
                    }
                }
            }
        }
        if (it->second.cur_buy_ref == order_ref)
        {
            ResetCurPriceAndOrder(it, account, code, CONST_ENTRUST_BUY);
        }
        else if (it->second.cur_sell_ref == order_ref)
        {
            ResetCurPriceAndOrder(it, account, code, CONST_ENTRUST_BUY);
        }
    }
}
/* 订单成交 */
void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderFilled(
    const char * account,       // account
    OrderRefDataType order_ref				// 报单引用
    )
{
    lock_guard<std::mutex> lock(check_mutex);

    auto it = order_infomap.find(order_ref);
    if (it != order_infomap.end())
    {
        string code = it->second.stock;

        char key[64];
        sprintf(key, "%s_%s", account, it->second.stock.c_str());

        auto acit = account_contract_infomap.find(key);

        if (acit != account_contract_infomap.end())
        {
            //如果订单处于撤单状态，撤单数减一
            if (it->second.status == PENDING_CANCEL)
            {
                acit->second.cur_cancel_times--;
            }
            it->second.status = FILLED;
            order_infomap.erase(order_ref);
            if (acit->second.cur_buy_ref == order_ref)
            {
                ResetCurPriceAndOrder(acit, account, it->second.stock, CONST_ENTRUST_BUY);
            }
            else if (acit->second.cur_sell_ref == order_ref)
            {
                ResetCurPriceAndOrder(acit, account, it->second.stock, CONST_ENTRUST_BUY);
            }
        }

        auto cit = contract_infomap.find(code);
        if (cit != contract_infomap.end())
        {
            if (cit->second.cur_buy_ref == order_ref)
            {
                ResetCurPriceAndOrder(cit, code, CONST_ENTRUST_BUY);
            }
            else if (cit->second.cur_sell_ref == order_ref)
            {
                ResetCurPriceAndOrder(cit, code, CONST_ENTRUST_SELL);
            }
        }
    }
}

/* 订单已撤销，修正完撤单数后删除订单 */
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
    lock_guard<std::mutex> lock(check_mutex);

    auto oit = order_infomap.find(order_ref);
    if (oit != order_infomap.end())
    {
        char buy_sell_flag = oit->second.buy_sell_flag;
        if (oit->second.status == PENDING_CANCEL)
        {
            auto ait = account_infomap.find(account);
            if (ait != account_infomap.end())
            {
                if (open_close_flag == CONST_ENTRUST_OPEN
                    && exchange_code == CONST_EXCHCODE_CFFEX)
                {
                    int index = GetCommodityIndex(code);
                    if (index != -1)
                    {
                        if (hedge_flag == CONST_SHFLAG_TOU)
                        {
                            ait->second.cur_speculate_open_times[index] -= canceled_volumn;
                            ait->second.cur_total_open_times[index] -= canceled_volumn;
                        }
                        else if (hedge_flag == CONST_SHFLAG_TAO)
                        {
                            ait->second.cur_arbitrage_open_times[index] -= canceled_volumn;
                            ait->second.cur_total_open_times[index] -= canceled_volumn;
                        }
                    }
                }
                oit->second.status = CANCELED;
                order_infomap.erase(order_ref);
                char key[64];
                sprintf(key, "%s_%s", account, code);
                auto acit = account_contract_infomap.find(key);
                if (acit != account_contract_infomap.end()) {
                    ResetCurPriceAndOrder(acit, account, code, buy_sell_flag);
                }
                auto cit = contract_infomap.find(key);
                if (cit != contract_infomap.end()) {
                    ResetCurPriceAndOrder(cit, code, buy_sell_flag);
                }
            }
        }
    }
}

/* 撤单检测，不能超过最大值 */
static int TryCancelCheck(const AccountContractInfoMap::iterator &it)
{
    int ret = COMPLIANCE_CHECK_RESULT_SUCCESS;
    if (it->second.cancal_times_check)
    {
        if (it->second.cur_cancel_times + 1 == it->second.warn_threthold_cancel_times)
        {
            ret = COMPLIANCE_CHECK_RESULT_CANCEL_TIMES_EQUAL_WARN_THRETHOLD;
        }
        else if (it->second.cur_cancel_times + 1 > it->second.warn_threthold_cancel_times
            && it->second.cur_cancel_times + 1 < it->second.max_cancel_times)
        {
            ret = COMPLIANCE_CHECK_RESULT_CANCEL_TIMES_REACH_WARN_THRETHOLD;
        }
        else if (it->second.cur_cancel_times + 1 >= it->second.max_cancel_times)
        {
            ret = COMPLIANCE_CHECK_RESULT_CANCEL_EXCEED_LIMIT;
        }
    }
    return ret;
}

/* 撤单合规检查，如果大于等于最大撤单限制数则禁止撤单 */
RESULT_TYPE COMPLIANCE_CHECK_API_CALL ComplianceCheck_TryReqOrderAction(
    const char * account,       // account
    const char * code, 			// 证券代码
    OrderRefDataType order_ref  // 报单引用
    )
{
    int ret = COMPLIANCE_CHECK_RESULT_SUCCESS;
    char key[64];
    sprintf(key, "%s_%s", account, code);

    lock_guard<std::mutex> lock(check_mutex);

    auto acit = account_contract_infomap.find(key);

    if (acit != account_contract_infomap.end())
    {
        auto oit = order_infomap.find(order_ref);
        if (oit != order_infomap.end())
        {
            if (oit->second.status == PENDING)
            {
                ret = TryCancelCheck(acit);

                if (ret == COMPLIANCE_CHECK_RESULT_CANCEL_EXCEED_LIMIT)
                {
                    return ret;
                }

                oit->second.status = PENDING_CANCEL;
                acit->second.cur_cancel_times++;

                if (acit->second.cur_buy_ref == order_ref)
                {
                    ResetCurPriceAndOrder(acit, account, code, CONST_ENTRUST_BUY);
                }
                else if (acit->second.cur_sell_ref == order_ref)
                {
                    ResetCurPriceAndOrder(acit, account, code, CONST_ENTRUST_BUY);
                }

                auto cit = contract_infomap.find(code);
                if (cit != contract_infomap.end())
                {
                    if (cit->second.cur_buy_ref == order_ref)
                    {
                        ResetCurPriceAndOrder(cit, code, CONST_ENTRUST_BUY);
                    }
                    else if (cit->second.cur_sell_ref == order_ref)
                    {
                        ResetCurPriceAndOrder(cit, code, CONST_ENTRUST_SELL);
                    }
                }
            }
        }
    }
    return ret;
}

/* 撤单失败，修正撤单数 */
void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderCancelFailed(
    const char * account,       // account
    const char * code, 			// 证券代码
    OrderRefDataType order_ref  // 报单引用
    )
{
    char key[64];
    sprintf(key, "%s_%s", account, code);

    lock_guard<std::mutex> lock(check_mutex);

    auto oit = order_infomap.find(order_ref);
    if (oit != order_infomap.end())
    {
        if (oit->second.status == PENDING_CANCEL)
        {
            oit->second.status = PENDING;
            auto acit = account_contract_infomap.find(key);
            if (acit != account_contract_infomap.end())
            {
                acit->second.cur_cancel_times--;
            }
            if (acit->second.cur_buy_ref == order_ref)
            {
                ResetCurPriceAndOrder(acit, account, code, CONST_ENTRUST_BUY);
            }
            else if (acit->second.cur_sell_ref == order_ref)
            {
                ResetCurPriceAndOrder(acit, account, code, CONST_ENTRUST_BUY);
            }

            auto cit = contract_infomap.find(code);
            if (cit != contract_infomap.end())
            {
                if (cit->second.cur_buy_ref == order_ref)
                {
                    ResetCurPriceAndOrder(cit, code, CONST_ENTRUST_BUY);
                }
                else if (cit->second.cur_sell_ref == order_ref)
                {
                    ResetCurPriceAndOrder(cit, code, CONST_ENTRUST_SELL);
                }
            }
        }
    }
}

void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderPendingCancel(
    const char * account,       // account
    OrderRefDataType order_ref	// 报单引用
    )
{

}

/* 设置撤单数 */
void COMPLIANCE_CHECK_API_CALL ComplianceCheck_SetCancelTimes(
    const char * account,       // account
    const char * code,          // 证券代码
    int cancel_times            // 撤单次数
    )
{
    char key[64];
    sprintf(key, "%s_%s", account, code);

    lock_guard<std::mutex> lock(check_mutex);

    auto acit = account_contract_infomap.find(key);
    if (acit != account_contract_infomap.end())
    {
        acit->second.cur_cancel_times = cancel_times;
    }
    else
    {
        auto ait = account_infomap.find(account);
        if (ait != account_infomap.end())
        {
            AccountContractInfo item(ait->second);
            item.cur_cancel_times = cancel_times;
            account_contract_infomap.insert(make_pair(key, item));
        }
    }
}

/* 设置开仓量 */
void COMPLIANCE_CHECK_API_CALL ComplianceCheck_SetOpenVolume(
    const char * account,       // account
    char exchange_code,         // 交易所编码
    const char * product,       // 品种
    char hedge_flag,            // 投机套保类型
    int open_volume             // 开仓数量
    )
{
    lock_guard<std::mutex> lock(check_mutex);

    auto ait = account_infomap.find(account);
    if (ait != account_infomap.end())
    {
        int index = GetCommodityIndex(product);
        if (index != -1)
        {
            if (hedge_flag == CONST_SHFLAG_TOU)
            {
                ait->second.cur_speculate_open_times[index] = open_volume;
                ait->second.cur_total_open_times[index] = open_volume;
            }
            else if (hedge_flag == CONST_SHFLAG_TAO)
            {
                ait->second.cur_arbitrage_open_times[index] = open_volume;
                ait->second.cur_total_open_times[index] = open_volume;
            }
        }
    }
}
