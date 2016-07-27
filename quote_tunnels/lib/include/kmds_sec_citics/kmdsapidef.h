#ifndef _KMDS_API_DEF_HH_
#define _KMDS_API_DEF_HH_
/************************************************************************/
/*  windows 生成dll，并导出类,则编译时需要加 KMDSAPI_EXPORT_DEF KMDSAPI_BUILD_DLL两个宏才能正确生成。
            使用dll时，则可执行程序编译时需要增加KMDSAPI_EXPORT_DEF宏定义。
	默认情况是使用静态库Lib来进行链接的。
            */
/************************************************************************/

#if defined (_WIN32)|| defined(_WIN64)
#  if defined(KMDSAPI_EXPORT_DEF)
#    if defined (KMDSAPI_BUILD_DLL)
#      define KMDSAPI_Export __declspec(dllexport)
#    else /* KMDSAPI_BUILD_DLL */
#      define KMDSAPI_Export __declspec(dllimport)
#    endif /* KMDSAPI_BUILD_DLL */
#  else
#    define KMDSAPI_Export
#  endif

#  else /* !_WIN32 || !_WIN64 */
#    define KMDSAPI_Export
#  endif /* !_WIN32 || !_WIN64 */

#include "msgdata.h"
#include "kdhqplayback_define.h"


// kmds用户API-程序版本号
#define KMDS_USER_API_VERSION           "1.3.0"

// kmds用户API-数据类型
enum ENUM_KMDS_DATA_TYPE
{
	KMDS_HQ_SYNC_REQ_DATA_MIN = 1000,
	KMDS_HQ_CODETABLE         = 1001,	// 证券代码表	请求
	KMDS_HQ_LAST_PART_SNPST    = 1002, // 最新一笔行情快照
	KMDS_HQ_MIN_K_LINE         = 1003,	// 分钟线数据	    请求
	KMDS_HQ_SYNC_REQ_DATA_MAX = 1100,

    KMDS_HQ_CODETABLE_SNPST   = 2061,   // 实时代码表变动数据  推送


	KMDS_HQ_SPQA_SNAPSHOT = 6005,                   // 黄金雁联现货行情回报  推送
	KMDS_HQ_FWQA_SNAPSHOT = 6006,                   // 黄金雁联远期行情回报  推送
	KMDS_HQ_DFQA_SNAPSHOT = KMDS_HQ_FWQA_SNAPSHOT,  // 黄金雁联延期行情回报  推送
	KMDS_HQ_SPIS_SNAPSHOT = 6007,                   // 黄金雁联现货合约状态改变事件  推送
	KMDS_HQ_FWIS_SNAPSHOT = KMDS_HQ_SPIS_SNAPSHOT,  // 黄金雁联远期合约状态改变事件  推送
	KMDS_HQ_DFIS_SNAPSHOT = KMDS_HQ_SPIS_SNAPSHOT,  // 黄金雁联延期合约状态改变事件  推送

	KMDS_HQ_STK_PART_SNPST    = 6010,	// 股票行情数据	推送
	KMDS_HQ_FT_PART_SNPST     = 6011,	// 期货行情数据	推送
	KMDS_HQ_INDEX_PART_SNPST  = 6012,	// 指数行情数据	推送
	KMDS_HQ_STKOP_PART_SNPST  = 6014,   // 个股期权行情数据  推送
	KMDS_HQ_RDM_PART_SNPST    = 6015,   // 路透行情数据 推送
	KMDS_HQ_CMDS_PART_SNPST   = 6016,   // 恒泰银行间数据  推送
	KMDS_HQ_ORDR_QU           = 6020,	// 订单队列	    推送
	KMDS_HQ_PER_STRK_ENTRT    = 6030,	// 逐笔委托	    推送
	KMDS_HQ_PER_STRK_BGN      = 6040,	// 逐笔成交	    推送

	// add by hanpc
	KMDS_HQ_HISTORY_CODETABLE    = 7010,	// 历史代码表
	KMDS_HQ_HISTORY_PART_SNPST   = 7020,   // 历史分笔快照
	KMDS_HQ_HISTORY_ORDR_QU      = 7030,	// 历史订单队列
	KMDS_HQ_HISTORY_PER_STRK_ENTRT = 7040,  // 历史逐笔委托
	KMDS_HQ_HISTORY_PER_STRK_BGN	= 7050,   // 历史逐笔成交
	KMDS_HQ_HISTORY_DAY_K_LINE     = 7060,     // 历史日K线
	KMDS_HQ_HISTORY_MIN_K_LINE     = 7070,		// 历史分钟线

	// add by hanpc
	KMDS_HQ_INFO = 8010,   // 查询咨询接口

    // add by lind
    KMDS_HQ_CHECK_PART_SNPST = 9010,  // 行情快照查询(历史和实时快照)

    KMDS_HQ_HGT_REFF = 9021,
    KMDS_HQ_HGT_EXRA = 9022,
};

// 订阅类型
enum ENUM_KMDS_SUB_TYPE
{
	KMDS_SUB_REMOVE_ALL = 0,            //全部解订
	KMDS_SUB_ADD,                       //增加
	KMDS_SUB_REMOVE,                    //删除
	KMDS_SUB_ADD_ALL                    //全部订阅
};

//---------------------------------------------------------------------
//事件回调接口类

// 消息回调接口
class CKmdsMsgCB
{
public:
	CKmdsMsgCB(){}
	virtual ~CKmdsMsgCB(){}
public:
	// 订阅消息回调函数
	virtual void OnKmdsMsg(void* pmsgdesc, void* pmsgopts,int ncmd ,CMsgData* pmsg){}

	// FT集群响应事件回调函数(服务器端API接口调用)
	// enum MOM_FTAction enaction  momdef.h中
	virtual void OnRvFtAction(const char *pgroupname, int enaction){}
};

// 登录结果回调接口
class CKmdsLoginCB
{
public:
	CKmdsLoginCB(){}
	virtual ~CKmdsLoginCB(){}
public:
	virtual int OnKmdsLoginCB(int nid,char* pszmsg) = 0;
};

//请求消息回调接口(服务器端API接口调用)
class CKmdsMsgReqCB
{
public:
	CKmdsMsgReqCB(){}
	virtual ~CKmdsMsgReqCB(){}
public:
	virtual void OnKmdsReqMsg(void* pmsgdesc, void* pmsgopts,int ncmd ,CMsgData* pmsg) = 0;
};
//------------------------------------------------------------------------


#endif
