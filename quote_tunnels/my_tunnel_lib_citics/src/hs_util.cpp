#include "hs_util.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

#ifdef MY_LOG_DEBUG
#undef MY_LOG_DEBUG
#endif
#define MY_LOG_DEBUG(format, args...) my_cmn::my_log::instance(NULL)->log(my_cmn::LOG_MOD_QUOTE, my_cmn::LOG_PRI_DEBUG, format, ##args)

using namespace std;
using namespace my_cmn;

namespace HSUtil
{

void ShowOneRowData(const std::string &desp, HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl,int iRow)
{
    int iCol;
    char szKey[64], szValue[512];
    iCol = CITICs_HsHlp_GetColCount(HlpHandle, lpMsg_Ctrl);

    MY_LOG_DEBUG("\n====================output of %s, %d rows", desp.c_str(), iRow);

	MY_LOG_DEBUG("nFuncID:%d,nReqNo:%d, ", lpMsg_Ctrl->nFuncID, lpMsg_Ctrl->nReqNo);
	for (int j = 0; j < iCol; j++){
		CITICs_HsHlp_GetColName(HlpHandle, j, szKey, lpMsg_Ctrl);
		CITICs_HsHlp_GetValueByIndex(HlpHandle, j, szValue, lpMsg_Ctrl);
		MY_LOG_DEBUG("%s:%s, ", szKey, szValue);
	}
}


//打印返回值。如果遇到和文档不一致的情况，以测试结果为准。
void ShowOneRowData(const std::string &desp, HSHLPHANDLE HlpHandle,int iRow)
{
    int iCol;
    char szKey[64], szValue[512];

    MY_LOG_DEBUG("%s %d row begin...",desp.c_str(),iRow);
    iCol = CITICs_HsHlp_GetColCount(HlpHandle);
	for (int j = 0; j < iCol; j++){
		CITICs_HsHlp_GetColName(HlpHandle, j, szKey);
		CITICs_HsHlp_GetValueByIndex(HlpHandle, j, szValue);
		MY_LOG_DEBUG("%s:%s, ", szKey, szValue);
	}

}

//打印返回值。如果遇到和文档不一致的情况，以测试结果为准。
void ShowAnsData(const std::string &desp, HSHLPHANDLE HlpHandle)
{
    int iRow, iCol;
    char szKey[64], szValue[512];

    iRow = CITICs_HsHlp_GetRowCount(HlpHandle);
    iCol = CITICs_HsHlp_GetColCount(HlpHandle);
    MY_LOG_DEBUG("\n====================output of %s, %d rows", desp.c_str(), iRow);
    for (int i = 0; i < iRow; i++){
		char szMsg[512] = { 0 };
		for (int j = 0; j < iCol; j++){
			CITICs_HsHlp_GetColName(HlpHandle, j, szKey);
			CITICs_HsHlp_GetValueByIndex(HlpHandle, j, szValue);
			MY_LOG_DEBUG("%s:%s, ", szKey, szValue);
		}

		if (i + 1 < iRow) MY_LOG_DEBUG("----------------------------------------------------------------");

        int rtn = CITICs_HsHlp_GetNextRow(HlpHandle);
        if (rtn == 0) break;
    }
    MY_LOG_DEBUG("\n==================finish output==================================");
}

void ShowAnsData(const std::string &desp, HSHLPHANDLE HlpHandle, LPMSG_CTRL lpMsg_Ctrl)
{
    int iRow, iCol;
    char szKey[64], szValue[512];
    iRow = CITICs_HsHlp_GetRowCount(HlpHandle, lpMsg_Ctrl);
    iCol = CITICs_HsHlp_GetColCount(HlpHandle, lpMsg_Ctrl);

    MY_LOG_DEBUG("\n====================output of %s, %d rows", desp.c_str(), iRow);
    for (int i = 0; i < iRow; i++){
    	MY_LOG_DEBUG("nFuncID:%d,nReqNo:%d, ", lpMsg_Ctrl->nFuncID, lpMsg_Ctrl->nReqNo);
		for (int j = 0; j < iCol; j++){
			CITICs_HsHlp_GetColName(HlpHandle, j, szKey, lpMsg_Ctrl);
			CITICs_HsHlp_GetValueByIndex(HlpHandle, j, szValue, lpMsg_Ctrl);
			MY_LOG_DEBUG("%s:%s, ", szKey, szValue);
		}
		if (i + 1 < iRow) MY_LOG_DEBUG("----------------------------------------------------------------");
		if (0 != CITICs_HsHlp_GetNextRow(HlpHandle, lpMsg_Ctrl)) break;

    }
    MY_LOG_DEBUG("\n==================finish output==================================");
}

void ShowErrMsg(HSHLPHANDLE HlpHandle, const int iFunc, int nErr)
{
    char szMsg[512] = { 0 };
    CITICs_HsHlp_GetErrorMsg(HlpHandle, &nErr, szMsg);
    MY_LOG_DEBUG("function execute failed[%d]: Error=(%d) %s", iFunc, nErr, szMsg);
}

}
