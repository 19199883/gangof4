#ifndef _C_MSGDATA_H_
#define _C_MSGDATA_H_

/************************************************************************/
/* @author 马林
/* @date 2013.12.16
/* @descripe CMsgData转换成C接口函数
/* 返回0则成功
/************************************************************************/
#include "ckmdsdef.h"


_BEGIN_KMDSAPI_DEF

/*************************************************
	Function:       CMsgData_Create
	Description:    使用CMsgData前初始化,相当于获取实例
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:
	Output:
	Return:         返回CMsgData句柄 失败返回NULL
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export MSGDATAHANDLE KMDSSTDCALL CMsgData_Create(void);

/*************************************************
	Function:       CMsgData_Release
	Description:    CMsgData 释放，引用计数器减一，知道等于零则销毁实例
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:
	Output:
	Return:         返回当前引用计数
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_Release(MSGDATAHANDLE hhandle);

/*************************************************
	Function:       CMsgData_AddRef
	Description:    CMsgData 增加引用计数，引用计数器加一
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:
	Output:
	Return:         返回当前引用计数
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_AddRef(MSGDATAHANDLE hhandle);


/*************************************************
	Function:       CMsgData_ClearAll
	Description:    清理CMsgData，清除CMsgData数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int   KMDSSTDCALL CMsgData_ClearAll(MSGDATAHANDLE hhandle);

/*************************************************
	Function:       CMsgData_DeleteItem
	Description:    删除CMsgData内指定键名的数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
					char* pszkey					Y		要删除的CMsgData数据的键名
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int   KMDSSTDCALL CMsgData_DeleteItem(MSGDATAHANDLE hhandle,char* pszkey);

// ------------------------------------------------------------------------------------------------
// 设置数据
/*************************************************
	Function:       CMsgData_PutData_Int8
	Description:    设置CMsgData数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
					char* pszkey					Y		键名
					signed char cValue				Y		键值
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_Int8(MSGDATAHANDLE hhandle,char* szkey, int8 cValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_Int16(MSGDATAHANDLE hhandle,char* szkey,int16 shValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_Int32(MSGDATAHANDLE hhandle,char* szkey,int32 iValue);
// linux __int64 long long
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_Int64(MSGDATAHANDLE hhandle,char* szkey, int64 n64Value);

C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_UInt8(MSGDATAHANDLE hhandle,char* szkey, uint8 cValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_UInt16(MSGDATAHANDLE hhandle,char* szkey, uint16 shValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_UInt32(MSGDATAHANDLE hhandle,char* szkey, uint32 iValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_UInt64(MSGDATAHANDLE hhandle,char* szkey, uint64 n64Value);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_Float(MSGDATAHANDLE hhandle,char* szkey, float fValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_Double(MSGDATAHANDLE hhandle,char* szkey, double dValue);
/*************************************************
	Function:       CMsgData_PutData_Buf
	Description:    设置CMsgData数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
					char* pszkey					Y		键名
					unsigned char *pbuf				Y		键值 一般用于二进制数据或者大字段数据保存
					unsigned int dwlen              Y       数据长度
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_Buf(MSGDATAHANDLE hhandle,char* szkey, unsigned char *pbuf,unsigned int dwlen);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData(MSGDATAHANDLE hhandle,char* szkey, char* pszValue);
	
// 获取数据
/*************************************************
	Function:       CMsgData_GetData_Int8
	Description:    读取CMsgData数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
					char* pszkey					Y		键名
					signed char* cValue				Y		键值
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_Int8(MSGDATAHANDLE hhandle,char* szkey, int8 *pcValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_Int16(MSGDATAHANDLE hhandle,char* szkey,int16 *pshValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_Int32(MSGDATAHANDLE hhandle,char* szkey,int32 *piValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_Int64(MSGDATAHANDLE hhandle,char* szkey,int64 *pn64Value);

C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_UInt8(MSGDATAHANDLE hhandle,char* szkey, uint8 *pcValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_UInt16(MSGDATAHANDLE hhandle,char* szkey,uint16 *pshValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_UInt32(MSGDATAHANDLE hhandle,char* szkey,uint32 *piValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_UInt64(MSGDATAHANDLE hhandle,char* szkey,uint64 *pn64Value);

C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_Float(MSGDATAHANDLE hhandle,char* szkey, float *pfValue);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_Double(MSGDATAHANDLE hhandle,char* szkey, double *pdValue);
/*************************************************
	Function:       CMsgData_GetData_Buf
	Description:    获取CMsgData数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
					char* pszkey					Y		键名
					unsigned char *pbuf				Y		键值 一般用于二进制数据或者大字段数据
					unsigned int dwlen              Y       获取到的数据长度
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_Buf(MSGDATAHANDLE hhandle,char* szkey, unsigned char **pbuf,unsigned int *pdwlen);
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_String(MSGDATAHANDLE hhandle,char* szkey,unsigned char *pValue,int nmaxlen);
// 递归消息实体存储
/*************************************************
	Function:       CMsgData_PutData_MsgData
	Description:    设置递归消息实体存储(即另外一个CMsgData作为键值放到CMsgData中）
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
					char* pszkey					Y		键名
					MSGDATAHANDLE pHandle		Y		CMsgData消息实体句柄
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_MsgData(MSGDATAHANDLE hhandle,char* szkey, MSGDATAHANDLE pHandle);
/*************************************************
	Function:       CMsgData_GetData_MsgData
	Description:    获得递归消息实体存储（从一个CMsgData获取CMsgData类型的键值）
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
					char* pszkey					Y		键名
					MSGDATAHANDLE* pHandle		Y		CMsgData消息实体句柄，不能为NULL
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/	
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_MsgData(MSGDATAHANDLE hhandle,char* szkey, MSGDATAHANDLE* pHandle);
// 二维数据存储
/*************************************************
	Function:       CMsgData_PutData_TableData
	Description:    设置二维数据存储到CMsgData中
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
					char* pszkey					Y		键名
					TABLEDATAHANDLE pHandle		Y		CommTableData消息实体句柄
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/	
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_PutData_TableData(MSGDATAHANDLE hhandle,char* szkey,TABLEDATAHANDLE pHandle);
// 二维数据存储
/*************************************************
	Function:       CMsgData_GetData_TableData
	Description:    获取二维数据存储
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		    Y		CMsgData句柄
					char* pszkey					Y		键名
					TABLEDATAHANDLE pHandle		Y		CommTableData消息实体句柄，需要调用前创建TABLEDATAHANDLE句柄，不能为NULL
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CMsgData_GetData_TableData(MSGDATAHANDLE hhandle,char* szkey,TABLEDATAHANDLE* pHandle);

// ------------------------------------------------------------------------------------------------
/*************************************************
	Function:       CMsgData_HaveData
	Description:    判断数据是否存在
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			MSGDATAHANDLE hhandle		Y		CMsgData句柄
					char* pszkey					Y		键名
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
 C_KMDSAPI_Export int  KMDSSTDCALL CMsgData_HaveData(MSGDATAHANDLE hhandle,char* szkey);


_END_KMDSAPI_DEF

#endif