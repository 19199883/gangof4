#ifndef CHECKSCHEDULE_H_
#define CHECKSCHEDULE_H_

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 CHECK_SCHEDULE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// CHECK_SCHEDULE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef WIN32
#ifdef CHECK_SCHEDULE_EXPORTS
#define CHECK_SCHEDULE_API __declspec(dllexport)
#else
#define CHECK_SCHEDULE_API __declspec(dllimport)
#endif

#define COMPLIANCE_CHECK_API_CALL _cdecl

#else
#define CHECK_SCHEDULE_API __attribute__ ((visibility ("default")))
#define COMPLIANCE_CHECK_API_CALL
#endif

typedef int RESULT_TYPE;
typedef long long OrderRefDataType;

// 返回码定义，和TradeTunnel中定义的错误码一致，避免不必要的转换
#define COMPLIANCE_CHECK_RESULT_SUCCESS								0
#define COMPLIANCE_CHECK_RESULT_FAIL								-1
#define COMPLIANCE_CHECK_RESULT_OPEN_REACH_EXCEED_LIMIT				4
#define COMPLIANCE_CHECK_RESULT_SELFTRADE							5
#define COMPLIANCE_CHECK_RESULT_CANCEL_TIMES_REACH_WARN_THRETHOLD	6
#define COMPLIANCE_CHECK_RESULT_CANCEL_EXCEED_LIMIT					7
#define COMPLIANCE_CHECK_RESULT_OPEN_EQUAL_LIMIT	8
#define COMPLIANCE_CHECK_RESULT_CANCEL_TIMES_EQUAL_WARN_THRETHOLD 9

extern "C"
{
// 初始化
CHECK_SCHEDULE_API void COMPLIANCE_CHECK_API_CALL ComplianceCheck_Init(
    const char * account,       // account
    int warn_threthold,			// 撤单警告阈值
    int max_cancel_times,		// 最大撤单次数（不可达到）
    int max_open_speculate,		// 最大投机手数
    int max_open_arbitrage,		// 最大套利手数
    int max_open_total,			// 最大总手数
    const char *switch_mask = 0 // 合规检查是否打开的位掩码.从左开始，第1位: 自成交；第2位: 撤单次数；第3位: 最大开仓数；无配置表示打开
    );

// 撤单次数初始化（合约当前的撤单次数）
CHECK_SCHEDULE_API void COMPLIANCE_CHECK_API_CALL ComplianceCheck_SetCancelTimes(
    const char * account,       // account
    const char * code,          // 证券代码
    int cancel_times            // 撤单次数
    );

// 开仓量初始化（品种当前的开仓量）
CHECK_SCHEDULE_API void COMPLIANCE_CHECK_API_CALL ComplianceCheck_SetOpenVolume(
    const char * account,       // account
    char exchange_code,         // 交易所编码
    const char * product,       // 品种
    char hedge_flag,            // 投机套保类型
    int open_volume             // 开仓数量
    );

// 下单尝试，增加报单委托/增加 开仓计数
CHECK_SCHEDULE_API RESULT_TYPE COMPLIANCE_CHECK_API_CALL ComplianceCheck_TryReqOrderInsert(
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
    OrderRefDataType * opposite_serial_no	// 【输出】导致自成交对手单的报单引用/当前开仓数
    );

// 下单失败回报，删除该报单/回滚开仓计数
CHECK_SCHEDULE_API void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderInsertFailed(
    const char * account,       // account
    OrderRefDataType order_ref,	// 报单引用
    char exchange_code,			// 交易所编码
    const char * code,          // 证券代码
    int volumn, 				// 数量
    char hedge_flag,			// 投机套保类型
    char open_close_flag,		// 开平方向
    char order_type 			// 报单类型
    );

// 报单全成时，删除该报单
CHECK_SCHEDULE_API void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderFilled(
    const char * account,       // account
    OrderRefDataType order_ref	// 报单引用
    );

// 报单撤单时，删除该报单/更新开仓计数
// 自成交模块的处理，移到撤单请求时，避免不必要的拦截。 modified by chenyongshun 20131114）
CHECK_SCHEDULE_API void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderCanceled(
    const char * account,       // account
    const char * code,          // 证券代码
    OrderRefDataType order_ref,	// 报单引用
    char exchange_code,			// 交易所编码
    int canceled_volumn,		// 撤单数量
    char hedge_flag,			// 投机套保类型
    char open_close_flag		// 开平方向
    );

// 撤单尝试，增加该品种撤单计数
CHECK_SCHEDULE_API RESULT_TYPE COMPLIANCE_CHECK_API_CALL ComplianceCheck_TryReqOrderAction(
    const char * account,       // account
    const char * code, 			// 证券代码
    OrderRefDataType order_ref  // 报单引用
    );

// 撤单失败，减少该品种撤单次数
CHECK_SCHEDULE_API void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderCancelFailed(
    const char * account,       // account
    const char * code, 			// 证券代码
    OrderRefDataType order_ref  // 报单引用
    );

// 上报撤单时，删除该报单 （在上报撤单请求时，告诉自成交模块，避免不必要的拦截。 modified by chenyongshun 20131114）
CHECK_SCHEDULE_API void COMPLIANCE_CHECK_API_CALL ComplianceCheck_OnOrderPendingCancel(
    const char * account,       // account
    OrderRefDataType order_ref	// 报单引用
    );
}
#endif // CHECKSCHEDULE_H_
