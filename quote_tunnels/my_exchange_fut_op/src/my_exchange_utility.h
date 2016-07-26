#if !defined(MY_EXCHANGE_UTILITY_H_)
#define MY_EXCHANGE_UTILITY_H_

#include <pthread.h>
#include <sys/timeb.h>
#include <time.h>
#include <string>
#include <vector>
#include "my_trade_tunnel_struct.h"
#include "my_cmn_log.h"

namespace OrderUtil
{
	// 从报单状态判断是否可以撤单
	static inline bool CanBeCancel(char order_status)
	{
		return order_status != MY_TNL_OS_COMPLETED
			&& order_status != MY_TNL_OS_WITHDRAWING
			&& order_status != MY_TNL_OS_ERROR
			&& order_status != MY_TNL_OS_WITHDRAWED;
	}

	// 从报单状态判断是否已经终结
	static inline bool IsTerminated(char order_status)
	{
		return order_status == MY_TNL_OS_COMPLETED
			|| order_status == MY_TNL_OS_ERROR
			|| order_status == MY_TNL_OS_WITHDRAWED;
	}

	// 20140101 YYYYMMDD
	std::string MYGetCurrentDateString();

	// 20140101_123456 YYYYMMDD_HHMMSS
	std::string GetCurrentDateTimeString();
};


#endif  //MY_EXCHANGE_UTILITY_H_
