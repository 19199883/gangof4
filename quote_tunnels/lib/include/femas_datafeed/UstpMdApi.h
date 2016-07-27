/*======================================================================================
系 统 名：行情API接口
作    者：hxs
功    能：定义了行情API接口
======================================================================================*/
#ifndef _USTP_MDAPI_H
#define _USTP_MDAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "USTPFtdcUserApiStruct.h"
#include <vector>

///错误代码
typedef int TUstpErrorID;
///错误信息
typedef char TUstpErrorMsg[80];
///主题号
typedef int  TUstpTopicID;
///用户接收多播网卡
typedef char TUstpIP[40];
///用户接收多播mac
typedef char TUstpMAC[40];

// 响应信息表
struct CUstpRspInfo
{
    TUstpErrorID    ErrorID;                       //错误代码
    TUstpErrorMsg   ErrorMsg;                      //错误信息
};

///用户登录请求
struct CUstpFtdcReqUserLoginField_MC
{
    ///用户代码
    TUstpFtdcUserIDType UserID;
    ///密码
    TUstpFtdcPasswordType   Password;
    ///主题号
    TUstpTopicID TopicID;
    ///用户接收多播IP  chenhui
    TUstpIP IP;
    ///用户接收多播MAC  chenhui
    TUstpMAC Mac;
};

using namespace std;

#ifdef WIN32
  #ifdef LIB_MD_API_EXPORT
	#define MD_API_EXPORT __declspec(dllexport)
  #else
	#define MD_API_EXPORT __declspec(dllimport)
  #endif
#else
    #define MD_API_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

///初始化API
///@param TimeOut						输入参数，所有请求超时时间,毫秒
///@param pRspInfo						返回信息
///@return true							初始化API成功
///@return false						初始化API失败
MD_API_EXPORT bool InitAPI(int TimeOut,CUstpRspInfo *pRspInfo);


///注册认证服务器地址
///@param IpPortGroup					认证服务器地址以及端口，格式为tcp://127.0.0.1:12001
MD_API_EXPORT void RegisterFront(char* ipPort);

///连接认证服务器地址
///@param pRspInfo						返回信息
///@return true							连接认证服务器成功
///@return false						连接认证服务器失败
MD_API_EXPORT bool ConnectFront(CUstpRspInfo *pRspInfo);

///用户登录请求
///@param pReqUserLogin					用户登陆请求域
///@param Mode					        获取行情方式:[true-始终获取递增的行情, false-始终获取最快的行情]
///@param pHandle						返回该用户连接句柄
///@return true							用户登陆成功
///@return false						用户登陆失败
MD_API_EXPORT bool ReqUserLogin(CUstpFtdcReqUserLoginField_MC *pReqUserLogin, bool Mode, void*& pHandle, CUstpRspInfo *pRspInfo);

///定时心跳检测,如果超时时间间隔10分钟的情况下不能再获取组播行情
///@param pHandle						登录时返回的连接句柄
///@param pRspInfo						返回信息
///@return true							心跳检测成功
///@return false						心跳检测失败
MD_API_EXPORT bool ReqHeartCheck(void*& pHandle, CUstpRspInfo *pRspInfo);

///检查连接句柄有效性
///@param pHandle						登录时返回的连接句柄
///@param pRspInfo					    返回信息
///@return true                         连接句柄有效
///@return false                        连接句柄无效
MD_API_EXPORT bool CheckValidateHandle(void*& pHandle, CUstpRspInfo *pRspInfo);

///行情快照查询
///@param pHandle						登录时返回的连接句柄
///@param ReqInstrumentID				查询的合约号，为空表示查询全部
///@param vMarket					    返回的行情快照数组
///@param pRspInfo					    返回信息
///@return true                         查询成功
///@return false                        查询失败
MD_API_EXPORT bool ReqQrySnapshotDepthMarket(void*& pHandle, TUstpFtdcInstrumentIDType ReqInstrumentID, vector<CUstpFtdcDepthMarketDataField>& vMarket, CUstpRspInfo *pRspInfo);

///获取组播行情消息
///@param iHandle                       登录时返回的连接句柄
///@param FtdcDepthMarketDataField      返回的行情消息
///@return true                         获取组播行情成功
///@return false                        获取组播行情失败
MD_API_EXPORT bool GetDepthMarketDataField(void*& pHandle, long& SequenceNumber, CUstpFtdcDepthMarketDataField& FtdcDepthMarketDataField, CUstpRspInfo *pRspInfo);

///用户退出请求
///@param pHandle						登录时返回的连接句柄
///@param pRspInfo						返回信息
///@return true							用户退出成功
///@return false						用户退出失败
MD_API_EXPORT bool ReqUserLogout(void*& pHandle, CUstpRspInfo *pRspInfo);

///删除用户连接句柄
///@param pHandle						用户连接句柄
MD_API_EXPORT void DestroyUserHandle(void*& pHandle);

///销毁API
MD_API_EXPORT void DestroyAPI();


#ifdef __cplusplus
}
#endif



#endif
