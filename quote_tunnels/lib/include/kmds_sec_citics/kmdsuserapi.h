/******************************************************************************
*
*									KMDS V1.0
*
*           Copyright (C) 2013-2014, SHENZHEN KINGDOM Co., Ltd.
*
*                             All Rights Reserved.
*
* ===========================================================================
*
* 文 件 名：kmdsuserapi.h
* 模块名称：用户接口API
* 功能说明：提供同步请求，连接服务器，用户订阅主题等功能
* 作    者：malin
* 创建日期：2013-08-22
* 版 本 号：1.0
* 修改历史：
*
* 修改日期       姓名            内容
* ---------------------------------------------------------------------------
* 2013-08-22     malin           初始版本
* 2013-12-02     fanw	         初始发布版本
******************************************************************************/

#ifndef __KMDS_USER_API_HH_
#define __KMDS_USER_API_HH_

#include "msgdata.h"
#include "kmdsapidef.h"
#define ERR_BUFSIZE   1024

class KMDSAPI_Export CKmdsUserApi
{
public:
	CKmdsUserApi();

	~CKmdsUserApi();
public:
	/*************************************************
	Function:       Init
	Description:    初始化
	Calls:
	Called By:
	Table Accessed:
	Table Updated:
	Input:
	                const char* szcfgpath	 本地配置文件名

	Output:
	Return:         0表示成功， 其他失败
	Others:         完成对本地配置信息的装载、进行通讯引擎的初始化，
	                用户接口库加载成功后调用
	ex:
	*************************************************/
	int Init(const char* szcfgpath = NULL);

	/*************************************************
	Function:       UnInit
	Description:    退出订阅主题的监听，退出各工作线程，释放相应资源
	Calls:
	Called By:
	Table Accessed:
	Table Updated:
	Input:

	Output:
	Return:         0表示成功， 其他失败
	Others:         客户端退出时调用
	ex:
	*************************************************/
	int UnInit();

	/*************************************************
	Function:       Login
	Description:    用户登录认证
	                向总线管理服务器进行用户权限认证，
	                如果认证通过则从服务器获取对应的用户行情订阅主题表、
					通讯协议解析文件（Fast协议模版文件）、通讯中间件配置
					参数等信息。 然后再根据这些参数来建立各个通讯通道，
					准备开始接收行情数据
	Calls:
	Called By:
	Table Accessed:
	Table Updated:
	Input:
	                const char* szname  授权的用户名
				    const char* szpass  用户密码
	Output:
	Return:         0表示成功， 其他失败
	Others:
			        初始化之后调用，调用返回成功后才能接收行情服务。
	ex:
	*************************************************/
	int Login(const char* szname,const char* szpass);

    /*************************************************
	Function:       Logout
	Description:    用户登出
	Calls:
	Called By:
	Table Accessed:
	Table Updated:
	Input:
	Output:
	Return:         0表示成功， 其他失败
	Others:
	ex:
	*************************************************/
    int Logout();

	/*************************************************
	Function:       SyncCallLBM
	Description:    同步KCBP请求。
	                与远端KCBP服务器进行点对点的通讯，同步获取返回的请求应答数据
	Calls:
	Called By:
	Table Accessed:
	Table Updated:
	Input:
					const char*  szlbmno      请求的行情类型
					CMsgData*    preqdata     请求参数集
					CMsgData**   ppansdata    应答数据集
					int		     ntimeout	  请求超时值
	Output:
	Return:         0表示成功， 其他失败
	Others:
			        用户登录认证接口调用成功后使用
	ex:
	*************************************************/
	int SyncCallLBM(const char* szlbmno,CMsgData* preqdata, CMsgData **ppansdata,int ntimeout = -1);


	/*************************************************
	Function:       Subscribe
	Description:    设置订阅主题和需要执行的消息回调函数(主题串)

	Calls:
	Called By:
	Table Accessed:
	Table Updated:
	Input:
	                const char* sztopic   订阅主题串
	Output:
	Return:         0表示成功， 其他失败
	Others:
			        用户登录认证接口调用成功后使用。用户模块将校验订阅的主题串，
			        若无此订阅权限则返回错误。
	ex:
	*************************************************/
	int Subscribe(const char* sztopic);

	/*************************************************
	Function:       UnSubscribe
	Description:    取消订阅主题(主题串)

	Calls:
	Called By:
	Table Accessed:
	Table Updated:
	Input:
	                const char* sztopic  订阅主题参数串,为空时取消全部订阅
	Output:
	Return:         0表示成功， 其他失败
	Others:

	ex:
	*************************************************/
	int UnSubscribe(const char* sztopic);

	/*************************************************
	Function:       Subscribe
	Description:    设置订阅主题和需要执行的消息回调函数
	Input:
					int    nSubType       订阅方式: 0：全部解订 1：增加  2:删除  3: 全部订阅
					char*  szMarketCode   订阅市场  CHAR2  见 “字典-市场”
					char*  szMqType		  行情类别  CHAR4  见 “字典-行情类型”
					char*  szWkType       工作类型  CHAR2  工作类型标识生产与模拟，0为生产，其他数字为不同的模拟者
					char*  szDsLevel      发布级别  CHAR   字符'1': 一级发布；'2':二级发布
					char*  szDsCode       数据来源  CHAR4  数据来源标识行情来源编号，取值为1..n，0表示比较处理后发布的数据
					char*  szPsCode       发布节点  CHAR2	发布节点标识行情发布节点编号，如01,02等, 主要为了RVRD不同节点传输
	Output:
	Return:			0表示成功， 其他失败
	Others:
					用户登录认证接口调用成功后使用。用户模块将校验订阅的主题，若无此订阅权限则返回错误。
					nSubType = 3时，将订阅用户权限的全部主题
	ex:
	*************************************************/
	int Subscribe(int nSubType,
				  char* szMarketCode = NULL,
				  char* szMqType = NULL,
				  char* szWkType = NULL,
				  char* szDsLevel = NULL,
				  char* szDsCode = NULL,
				  char* szPsCode = NULL
				 );

	/*************************************************
	Function:       SetKmdsMsgCB
	Description:    设置订阅消息回调类指针
	Input:
	                CKmdsMsgCB* pMsgCB  回调类接口
					如果为NULL则取消订阅回调
	Output:
	Return:         0表示成功， 其他失败
	Others:
			        用户登录认证接口调用成功后使用。
	ex:
	*************************************************/
	int SetKmdsMsgCB(CKmdsMsgCB* pMsgCB);

	/*************************************************
	Function:       SubscribeByStkCode
	Description:    按证券代码订阅行情
	Input:
					int nType  1:添加  2：删除  3：还原（全部删除）
	                int nMarketCode            市场
	                const char *lpszStkCode    证券代码

	Output:
	Return:         0表示无错误，其他表示错误号
	Others:         订阅证券代码来过滤收到的市场行情数据，
					可以加快API接收速度。若不设置证券代码，则接收
					全市场数据
	ex:
	*************************************************/
	int SubscribeStkCode(int nType,int nMarketCode, const char *lpszStkCode);

	/*************************************************
	Function:       GetHqData
	Description:    获取数据
	Input:          int ncmd               命令号 参见 ENUM_KMDS_DATA_TYPE
	                CMsgData* preqdata     请求参数,参见LBM接口文档
	                CMsgData** ppansdata   应答数据
	Output:
	Return:         0表示无错误，其他表示错误号
	Others:
	ex:
	*************************************************/
	int GetHqData(int ncmd,CMsgData* preqdata,CMsgData** ppansdata);

	/*************************************************
	Function:       GetErrCode
	Description:    获取最后一次调用的错误号
	Input:			无
	Output:
	Return:         0表示无错误，其他表示错误号
	Others:
	ex:
	*************************************************/
	int GetErrCode();

	/*************************************************
	Function:       GetErrMsg
	Description:    获取最后一次调用的错误信息
	Input:			无
	Output:
	Return:         返回为空是无错误
	Others:      	在调用其他接口函数返回值不为0时调用
	ex:
	*************************************************/
	const char* GetErrMsg();

	/*************************************************
	Function:       Version
	Description:    获取程序版本信息
	Input:			无
	Output:
	Return:         程序版本信息字符串
	Others:
	ex:
	*************************************************/
	const char* Version();

    /*************************************************
	Function:       GetTopicJurisdiction
	Description:    获取用户权限
	Input:
	Output:         CMsgData * * ppansdata  主题列表
	Return:         0表示无错误，其他表示错误号
	Others:
	ex:
	*************************************************/
    int GetTopicJurisdiction(CMsgData * * ppansdata);

    /*************************************************
	Function:       StartPlayBackData
	Description:    启动回放
	Input:          CMsgData *pclReq  请求参数
	Output:
	Return:         0表示无错误，其他表示错误号
	Others:
	ex:
	*************************************************/
    int StartPlayBackData(CMsgData *pclReq);

    /*************************************************
	Function:       StopPlayBackData
	Description:    停止回放
	Input:
	Output:
	Return:         0表示无错误，其他表示错误号
	Others:
	ex:
	*************************************************/
    int StopPlayBackData();

private:
	void*             m_pkmdsuserapiimp;

};
#endif
