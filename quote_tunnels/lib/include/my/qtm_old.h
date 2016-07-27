/**
 * 行情和通道的连接监测库
 * Revison:  2.0.6 20141020
 * 增加update_state接口,用于监测行情/通道 前置连接状态,和oss V2.0.6配合使用;
 *
 */
#pragma once
#include <stdio.h>
//#ifdef _cplusplus
#define EXPORT_INTERFACE extern "C"
////#else
//#define EXPORT_INTERFACE 	//intently left empty
//#endif

/**
 * 初始化，创建系统消息队列
 */
enum
{
	type_quote =0,
	type_tca
};
EXPORT_INTERFACE void init_qtm(int type);

struct criteria_t
{
	int interrupt;			//行情中断，默认为3秒
	int data_loss;		//行情tick丢失， 默认为3秒
	int tca_time_out;	//通道延迟，默认为5秒
};
/**
 * 设置行情延迟判断逻辑模块的告警阈值
 * NOTE: 若未通过该接口设置参数，则内部判断参数保持上述默认数值；
 */
EXPORT_INTERFACE void set_criteria(criteria_t *data);

/**
 * 提取行情中的时间戳，下发给行情延迟判断逻辑模块
 * 时间格式：09:15:00.000 即HH:MM:SS.mmm
 * H：Hour, M: Minute, S: Second, m: millisecond
 */
EXPORT_INTERFACE void acquire_quote_time(const char* time_str);

enum
{
	act_request=0,
	act_cancel_request,
	act_cancel_rtn,
	act_response,
	act_tradertn,
};
EXPORT_INTERFACE void acquire_tca_monitor_item(int action,		//通道动作定义，见上述枚举定义
		long time_stamp,				//时间戳  micro-seconds from epoch
		long seq_no);					// 流水号

/**
 * @param state_name 状态名称
 * @param state_code 状态码,>=0表示正常,<0表示异常
 * @param state_des  状态描述
 *
 */
EXPORT_INTERFACE void update_state(const char * state_name,
		int state_code,
		const char * state_des);


/**
 * 设置被监控的行情源/通道名称，字符串形式，用于oss client上标示行情；
 */
EXPORT_INTERFACE void set_identity(const char *name, size_t name_size);

/**
 * 清除动作，退出时删除系统消息队列
 * NOTE:消息队列为kernel-persist，退出时，可以选择不删除消息队列；
 */
EXPORT_INTERFACE void clear_qtm();

//old interface.obsoleted
#define init_qdm(x)  init_qtm(type_quote)
#define clear_qdm clear_qtm
#define set_quote_identity 	set_identity
#define set_judge_threshold(x,y)   set_criteria(x, sizeof(criteria_t))
