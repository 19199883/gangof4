/*! \file  EesTraderErr.h
 *  \brief EES交易客户端API使用错误类型定义。
*/

#pragma  once

#ifndef _EES_TRADE_API_ERROR_H_
#define _EES_TRADE_API_ERROR_H_

/// 没有错误
#ifndef NO_ERROR
#define NO_ERROR                    0	
#endif

/// 连接服务失败
#define CONN_SERVER_FAILED          1

/// 连接服务失败，一般会在服务器断开时候这个错误
#define REMOTE_DISCONN_SERVER       2	

/// 本地网络断开，会在本地主动断开的时候，产生这个错误
#define LOCAL_DISCONN_SERVER        3

/// 网络出错，网络异常，会产生这个错误
#define NEWWORK_ERROR               4

/// 登录服务失败，会在登录的时候产生
#define LOGON_FAILED                5	

/// 用户进行操作，是需要提前登录的，如果没有登录会产生这个错误
#define NOT_LOGON                   6

/// 操作之前，需要连接服务器
#define NO_CONN_SERVER              7	

/// 错误的交易对象句柄
#define HANDLE_ERRNOR               8	 
/// 设置订单 token 错误
#define ORDER_TOKEN_ERROR			9

//非法的密码，目前只支持全空密码检测
#define INVALID_PASSWORD_ERROR		10

#endif
