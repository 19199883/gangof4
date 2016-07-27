#ifndef XELE_FTDCMDUSERAPI_H
#define XELE_FTDCMDUSERAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XeleMdFtdcUserApiStruct.h"



#if defined(ISLIB) && defined(WIN32)
#ifdef LIB_MDUSER_API_EXPORT
#define MDUSER_API_EXPORT __declspec(dllexport)
#else
#define MDUSER_API_EXPORT __declspec(dllimport)
#endif
#else
#define MDUSER_API_EXPORT
#endif
/*
 *
 * LoginInit return value
 */
enum
{
    XELEAPI_SUCCESS = 0,
    XELEAPI_TCP_CLOSED,
    XELEAPI_SOCKET_ERROR,
    XELEAPI_USER_LOGIN_RESPONSE_TIMEOUT,
    XELEAPI_BAD_USERID_OR_PASSWORD,

    XELEAPI_BAD_FRONTADDRESS,
    XELEAPI_DEBUG,
};


struct MarketDataTick
{
    char tag[6];
    CXeleMdFtdcDepthMarketDataField data;
}__attribute__((packed));

class CXeleMdSpi
{
public:
    virtual void OnFrontUserLoginSuccess()
    {
    }
    ;

    virtual void OnFrontDisconnected(int nReason)
    {
    }
    ;
};

class MDUSER_API_EXPORT CXeleMdApi
{
public:
    static CXeleMdApi *CreateMdApi(CXeleMdSpi *spi);

    virtual const char *GetVersion() = 0;

    virtual int LoginInit(const char *frontAddress,
                             const char *multicastAddress,
                             const char* nic,
                              CXeleMdFtdcReqUserLoginField *pReqUserLogin) = 0;
    virtual bool RecvMarketDataTick(MarketDataTick* mdtick) = 0;

    virtual void Release() = 0;

};

#endif
