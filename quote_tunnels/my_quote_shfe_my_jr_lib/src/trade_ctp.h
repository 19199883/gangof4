#if !defined(CTP_TRADE_INTERFACE_H_)
#define CTP_TRADE_INTERFACE_H_

#include "ThostFtdcTraderApi.h"

#include <unordered_map>
#include <string>

#include "quote_cmn_config.h"

typedef std::unordered_map<std::string, std::string> MYSecurityCollection;
typedef std::unordered_map<std::string, int> MYSecurityVolumnMultiple;
class MYCTPTradeInterface: public CThostFtdcTraderSpi
{
public:
    static volatile bool securities_get_finished;
    static MYSecurityCollection securities_exchcode;

    MYCTPTradeInterface(const ConfigData &cfg);
    virtual ~MYCTPTradeInterface();

    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected();

    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param nReason 错误原因
    ///        0x1001 网络读失败
    ///        0x1002 网络写失败
    ///        0x2001 接收心跳超时
    ///        0x2002 发送心跳失败
    ///        0x2003 收到错误报文
    virtual void OnFrontDisconnected(int nReason)
    {
    }

    ///登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
        bool bIsLast);

    ///登出请求响应
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
        bool bIsLast);

    ///请求查询合约响应
    virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
        bool bIsLast);

private:
    CThostFtdcTraderApi *p_trader_api_;

    // 连接登陆参数
    char * front_addr_;
    char * broker_id_;
    char * user_;
    char * password_;

    int nRequestID;

};

#endif // CTP_TRADE_INTERFACE_H_
