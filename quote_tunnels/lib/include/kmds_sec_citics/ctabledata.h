#ifndef _C_TABLE_DATA_H_
#define _C_TABLE_DATA_H_
#include "ckmdsdef.h"

_BEGIN_KMDSAPI_DEF

/*************************************************
	Function:       CTableData_Create
	Description:    创建实例
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:
	Output:
	Return:         返回CTableData句柄 失败返回NULL
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export TABLEDATAHANDLE KMDSSTDCALL CTableData_Create(void);

/*************************************************
	Function:       CTableData_Release
	Description:    释放
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE hhandle		Y		CTableData句柄
	Output:
	Return:         返回引用计数
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_Release(TABLEDATAHANDLE hhandle);

/*************************************************
	Function:       CTableData_AddRef
	Description:    增加引用
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE hhandle		Y		CTableData句柄
	Output:
	Return:         返回引用计数
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_AddRef(TABLEDATAHANDLE hhandle);

/*************************************************
	Function:       CTableData_Reset
	Description:    清空CTableData数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE hhandle		Y		CTableData句柄
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_Reset(TABLEDATAHANDLE hhandle);

/*************************************************
	Function:       CTableData_AddRow
	Description:    增加行数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE    hhandle		  Y		  CTableData句柄
	                MSGDATAHANDLE*     pmsghandle     Y       CMsgData句柄,需要增加的行数据
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_AddRow(TABLEDATAHANDLE hhandle,MSGDATAHANDLE* pmsghandle);

/*************************************************
	Function:       CTableData_SaveRow
	Description:    保存行数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE    hhandle		  Y		  CTableData句柄
	               
	Output:
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_SaveRow(TABLEDATAHANDLE hhandle);

/*************************************************
	Function:       CTableData_GetCount
	Description:    获取CTableData的行数
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE hhandle		        Y		CTableData句柄
	                unsigned int    *pnCount			Y		行数
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_GetCount(TABLEDATAHANDLE hhandle,unsigned int *pnCount);

/*************************************************
	Function:       CTableData_GetRow
	Description:    获取CTableData指定行的数据
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE  hhandle		   Y		CTableData句柄
					int              nRow			   Y		CTableData指定的行
	                MSGDATAHANDLE*   pHandle           Y	    获取行数据的CMsgData
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_GetRow(TABLEDATAHANDLE hhandle,int nrow, MSGDATAHANDLE* prowmsgdata);

/*************************************************
	Function:       CTableData_GetFieldCount
	Description:    获取二维表的字段数
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE hhandle		Y		CTableData句柄
                    unsigned int *pnFieldCount		Y		字段数
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_GetFieldCount(TABLEDATAHANDLE hhandle,unsigned int *pnfieldcount);

/*************************************************
	Function:       CTableData_GetFieldName
	Description:    取字段名称
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE hhandle		Y		CTableData句柄
					int nFieldno					Y		字段序号
					int nMaxLen						Y		字段名的最大长度
	Output:			unsigned char *pszFieldName				字段名
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_GetFieldName(TABLEDATAHANDLE hhandle,int nfieldno,unsigned char *szfieldname,int nmaxlen);

/*************************************************
	Function:       CTableData_GetFieldType
	Description:    取字段类型
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE hhandle		Y		CTableData句柄
					int nFieldno					Y		字段序号
	     			unsigned int *pnFieldType		Y		字段类型
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_GetFieldType(TABLEDATAHANDLE hhandle,int nfieldno,unsigned int *pnfieldtype);

/*************************************************
	Function:       CTableData_GetFieldNo
	Description:    取字段序号
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE hhandle		Y		CTableData句柄
					unsigned char *pszFieldName				字段名
	    			unsigned int *pnFieldno					字段序号
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_GetFieldNo(TABLEDATAHANDLE hhandle,char *szfieldname,int *pnfieldno);

/*************************************************
	Function:       CTableData_AddFieldNo
	Description:    增加字段
	Calls:          
	Called By:      
	Table Accessed: 
	Table Updated:  
	Input:			TABLEDATAHANDLE hhandle		Y		CTableData句柄
					unsigned char *pszFieldName				字段名
	    			unsigned int *pnFieldno					字段序号
	Return:         0 - 成功 否则 - 失败
	Others:         // 其它说明
	ex:             
*************************************************/
C_KMDSAPI_Export int KMDSSTDCALL CTableData_AddFieldNo(TABLEDATAHANDLE hhandle,char *szfieldname,int *pnfieldno);

_END_KMDSAPI_DEF

#endif