/**
 * 行情和通道的连接监测库
 * Revison:  20140610 V1.0.0
 *      1)实现行情程序延迟/中断监测;
 * Revison:  20141020 V1.1.0
 *      1)增加通道延迟的检测,增加行情前置连接状态检测，和Oss V2.0.6配合使用;
 * Revison:  20151008 V1.2.0
 *      1) 解决同一进程内行情/通道程序无法区分监测的问题;
 * Revison:  20151224 V1.3.0
 *      1) 简化接口，去掉使用index标识行情/通道接口的方式,
 *      修改为直接使用name(char*)标识的方式;
 */
#if !defined(_QTM_H_03D7806C_17E9_49CE_9821_12AA667C444A)
#define _QTM_H_03D7806C_17E9_49CE_9821_12AA667C444A

#include <stdio.h>

#define INTERFACE extern "C" __attribute__((visibility("default")))

/**
 * @brief 初始化，创建系统消息队列
 * @param [in] type:    TYPE_QUOTE 表明仅监测行情
 *                      TYPE_TCA 表明仅监测通道
 *                      TYPE_QUOTE|TYPE_TCA 表明同时检测行情和通道
 */
#if !defined(TYPE_QUOTE)
#define TYPE_QUOTE     (0x01<<0)
#endif

#if !defined(TYPE_TCA)
#define TYPE_TCA       (0x01<<1)
#endif

INTERFACE void
qtm_init(int type);

#pragma pack(push)
#pragma pack(8)
/* quote/tunnel judge criteria. */
typedef struct {
        int quote_delay; /* Quote delay, default to 3 seconds. */
        int quote_loss;  /* Quote data loss, default to 2 seconds. */
        int quote_interrupt;  /* Quote interrupted, default to 5 seconds, 
                                maximium :10 seconds. 
                               */
        int order_timeout;  /* Default to 5 seconds. */
        int trade_timeout;  /* Default to 30 mins. */
} criteria_t;
#pragma pack(pop)

/**
 * @brief 设置延迟判断逻辑模块的告警阈值
 * @param[in] data: 延迟判据数据
 *
 * NOTE: 若未通过该接口设置参数，则内部判断参数保持各参数默认值；
 * NOTE: 该接口并非线程安全（基于性能考虑），
 *    因此请在初始化qtm库后，开始推送行情/报单前 设定告警阈值。
 */
INTERFACE void
set_criteria(const criteria_t *data);

/**
 * @brief 监测行情数据延迟/中断
 *      通过行情中的时间戳和本地服务器时间戳比对进行检查。
 * @param [in] name 行情名称，最大长度为32 Bytes，超出部分将被截断;
 *             NOTE: 名称在同一个交易主机上必须唯一; (行情和通道的名称也必须相互不同）
 * @param [in] time_str提取行情中的时间戳，下发给行情延迟判断逻辑模块
 *              时间格式：09:15:00.000 即HH:MM:SS.mmm
 *              H：Hour, M: Minute, S: Second, m: millisecond
 */
INTERFACE void
acquire_quote_time_field(const char *name, const char *time_str);

/* @brief 监测通道报单延迟
 *      通过分析报单回报/成交回报等时间延迟进行判断
 * @param [in] name 行情名称，最大长度为32 Bytes，超出部分将被截断;
 *             NOTE: 名称在同一个交易主机上必须唯一; (行情和通道的名称也必须相互不同）
 * @param[in] action: 报单动作定义，@see tag_act_enum
 * @param[in] time_stamp: 报单动作时间戳，为从1970-1-1 0:0:0开始距现在的ns数;
 * @param[in] seq_no: 报单流水号
 *
 * NOTE: 行情和通道的索引进程内必须唯一(行情的索引也不允许和通道索引重复);
 */
enum tag_act_enum {
        act_request=0,
        act_cancel_request,
        act_cancel_rtn,
        act_response,
        act_tradertn,
};
INTERFACE void
acquire_tca_order(const char *name, int action, long time_stamp, long seq_no);

/**
 * @brief 更新市场（通道/行情）前置连接状态
 * @param[in] name 行情名称，最大长度为32 Bytes，超出部分将被截断;
 *            NOTE: 名称在同一个交易主机上必须唯一; (行情和通道的名称也必须相互不同）
 * @param[in] type: 表明为行情或通道 
 *            NOTE: TYPE_TCA, TYPE_QUOTE 分别表示通道和行情
 * @param[in] status 状态码,>=0表示正常,<0表示异常
 * @param[in] description  状态或步骤描述, 最大长度为128 Bytes，超出部分将被截断;
 *
 */
INTERFACE void
update_state(const char *name, int type, int status, const char *description);


/**
 * @param[in] name: 通道及账户标识
 * @param[in] type: 标识合规类型,@see tag_compl_type_enum
 * @param[in] status: 合规状态
 *            <0 为异常，可扩展定义具体的异常值，如-1,-2 ...
 *            >0 为正常
 *            =0 为未知
 * @param[in] description: 状态描述
 *            该字段可选，如果有具体的合规描述信息，可传递进来，没有可以传NULL，
 *            字符串长度最长128 bytes，超过则会被截断。
 */
enum tag_compl_type_enum {
        OPEN_ORDER_LIMIT = 1,
        CANCEL_ORDER_LIMIT,
};
INTERFACE void
update_compliance(const char *name, int type, int status, const char *description);


/**
 * 清除动作，退出时删除系统消息队列
 * NOTE:消息队列为kernel-persistent，退出时，可以选择不删除消息队列；
 */
INTERFACE void
qtm_finish();

#endif

