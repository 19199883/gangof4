#ifndef _C_KMDS_DEF_H_
#define _C_KMDS_DEF_H_

// Compile time controls for library generation.  Define with /D or #define
// To produce or use a static library: #define KMDSAPI_HAS_DLL = 0
//   Default is to produce/use a DLL
// While building the KMDSAPI DLL: #define KMDSAPI_BUILD_DLL
//   Default is to export symbols from a pre-built KMDSAPI DLL
//
// Within KMDSAPI use the KMDSAPI_Export macro where a __declspec is needed.

#if defined (_WIN32)|| defined(_WIN64)

#  if !defined (C_KMDSAPI_HAS_DLL)
#    define C_KMDSAPI_HAS_DLL 1
#  endif /* ! C_KMDSAPI_HAS_DLL */

#  if defined (C_KMDSAPI_HAS_DLL) && (C_KMDSAPI_HAS_DLL == 1)
#    if defined (C_KMDSAPI_BUILD_DLL)
#      define C_KMDSAPI_Export __declspec(dllexport)
#    else /* KMDSAPI_BUILD_DLL */
#      define C_KMDSAPI_Export __declspec(dllimport)
#    endif /* KMDSAPI_BUILD_DLL */
#      define KMDSSTDCALL __stdcall
#  else /* C_KMDSAPI_HAS_DLL == 0 */
#    define C_KMDSAPI_Export
#    define KMDSSTDCALL
#  endif /* C_KMDSAPI_HAS_DLL == 1 */

#  else /* !_WIN32 || !_WIN64 */
#    define C_KMDSAPI_Export
#    define KMDSSTDCALL
#  endif /* !_WIN32 || !_WIN64 */


typedef void* KMDSUSERAPIHANDLE;
typedef void* MSGDATAHANDLE;
typedef void* TABLEDATAHANDLE;

#if defined(__cplusplus)

#ifndef _BEGIN_KMDSAPI_DEF
#define	_BEGIN_KMDSAPI_DEF	extern "C" { \

#endif

#ifndef _END_KMDSAPI_DEF
#define	_END_KMDSAPI_DEF	\
                              }

#endif

#else

#ifndef _BEGIN_KMDSAPI_DEF
#define	_BEGIN_KMDSAPI_DEF
#endif

#ifndef _END_KMDSAPI_DEF
#define	_END_KMDSAPI_DEF
#endif

#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef DEF_CHECK_VOID_NULL
#define DEF_CHECK_VOID_NULL
#define CHECK_POINT_NULL(p,ret)  \
        if (p == NULL)\
        {\
		    return ret; \
        }
#endif
#ifndef DEF_CHECKPOINTVALID
#define DEF_CHECKPOINTVALID
#define CHECKPOINTVALID(datatype,hhandle,ret) \
	    CHECK_POINT_NULL(hhandle,ret) \
		int nret = -1;\
		datatype* pthis = static_cast<datatype*>(hhandle);\
        if (pthis == NULL) \
		{\
		   return ret;\
		}
#endif

#ifndef NET_TYPEDEFS
#define NET_TYPEDEFS
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#if defined(_WIN32) || defined(_WIN64) 
typedef unsigned __int64 uint64;
typedef signed __int64 int64;
#else
typedef long long __int64;
typedef unsigned long long uint64;
typedef signed long long int64;

#endif


typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

#endif

#ifndef DEF_ENUM_KMDS_DATA_TYPE
#define DEF_ENUM_KMDS_DATA_TYPE

#define	CKMDS_HQ_SYNC_REQ_DATA_MIN  1000
#define	CKMDS_HQ_CODETABLE          1001	// 证券代码表	请求
#define	CKMDS_HQ_K_LINE             1002	// K线数据	    请求
#define	CKMDS_HQ_SYNC_REQ_DATA_MAX  1100

#define	CKMDS_HQ_STK_PART_SNPST     6010	// 股票行情数据	推送
#define	CKMDS_HQ_FT_PART_SNPST      6011	// 期货行情数据	推送
#define	CKMDS_HQ_INDEX_PART_SNPST   6012	// 指数行情数据	推送
#define	CKMDS_HQ_ORDR_QU            6020	// 订单队列	    推送
#define	CKMDS_HQ_PER_STRK_ENTRT     6030	// 逐笔委托	    推送
#define	CKMDS_HQ_PER_STRK_BGN       6040	// 逐笔成交	    推送

#endif

// 订阅类型
#ifndef DEF_ENUM_KMDS_SUB_TYPE
#define DEF_ENUM_KMDS_SUB_TYPE

#define	CKMDS_SUB_REMOVE_ALL  0              //全部解订
#define	CKMDS_SUB_ADD         1              //增加
#define	CKMDS_SUB_REMOVE      2              //删除
#define	CKMDS_SUB_ADD_ALL     3              //全部订阅

#endif
typedef void (*ON_MSG_RECV_CALLBACK)(void* pmsgdesc, void* pmsgopts,int ncmd ,MSGDATAHANDLE pmsg);





#endif