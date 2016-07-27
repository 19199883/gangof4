/******************************************************************************
*
*									KMDS V1.0 
*
*           Copyright (C) 2012-2013, SHENZHEN KINGDOM Co., Ltd.
* 
*                             All Rights Reserved.
*
* ===========================================================================
*
* 文 件 名：kdhqplayback_define.h 
* 模块名称：模拟回放模块
* 功能说明：公共基础宏定义
* 作    者：liulj
* 创建日期：2014-08-11
* 版 本 号：1.0
* 修改历史：
*
* 修改日期       姓名            内容
* ---------------------------------------------------------------------------
* 2014-08-11     liulj         初始版本
*
******************************************************************************/
#ifndef _KDHQPLAYBACK_DEFINE_H_
#define _KDHQPLAYBACK_DEFINE_H_

#pragma pack(push,1)

enum ENUM_PLAYBACK_SPEED
{
	ENUM_SPEED_FULL = 0,			// 全速
	ENUM_SPEED_NORMAL = 1,			// 正常速度
	ENUM_SPEED_DOUBLE = 2,			// 两倍速度
	ENUM_SPEED_FOUR = 4				// 四倍
};
enum ENUM_PLAYBACK_HQTYPE
{
	ENUM_HQTYPE_SNAPSHOT_1 = 0x01,		// 行情快照Level1
	ENUM_HQTYPE_SNAPSHOT_2 = 0x02,		// 行情快照Level2
	ENUM_HQTYPE_PER_STRK_BGN = 0x04,	// 逐笔成交
	ENUM_HQTYPE_PER_STRK_ENTRT = 0x08,	// 逐笔委托
	ENUM_HQTYPE_ORDR_QU = 0x10,			// 订单队列
	ENUM_HQTYPE_ALL = 0xFFFF,
};

// 状态
enum ENUM_STATUS
{
	ENUM_STATUS_NORMAL = 0,		 // 正常
	ENUM_STATUS_ABNORMAL,        // 异常	
};
// 模拟回放请求参数结构
struct ST_HQPLAYBACK
{
	char szUSER_NM[64];				// 用户名
	char szStkcode[1024];			// 市场证券代码 如1:600446;2:000001,证券代码-1表示全市场
	char szIP[32];					// IP地址
	int nHQTYPE;					// 行情类型 支持 |
	int nSPEED;						// 回放速度
	int nSRT_DT;
	int nEND_DT;
	int nSRT_TM;					// 精确到秒
	int nEND_TM;
	char szRESERVE[2048];			// 保留数据
};
#pragma pack(pop)

#endif