#ifndef CTP_QUOTE_INTERFACE_H_
#define CTP_QUOTE_INTERFACE_H_

#include <string>
#include <sstream>
#include <list>
#include <functional>

#include "ThostFtdcMdApi.h"
#include "my_trade_tunnel_struct.h"

// contract collection
typedef std::list<std::string> Contracts;

// 询价通知
typedef std::function<void(const T_RtnForQuote *)> RtnForQuoteHandler;

class MYCTPDataHandler: public CThostFtdcMdSpi
{
public:
    // 构造函数
    MYCTPDataHandler(const Contracts &contracts, const std::string &quote_front_addr);

    /**
     * @param quote_handler 询价通知处理的函数对象。
     */
    void SetQuoteDataHandler(RtnForQuoteHandler quote_handler)
    {
        quote_data_handler_ = quote_handler;
    }

    virtual ~MYCTPDataHandler(void);

    // 当客户端与交易托管系统建立起通信连接，客户端需要进行登录
    virtual void OnFrontConnected();

    // 当客户端与交易托管系统通信连接断开时，该方法被调用
    virtual void OnFrontDisconnected(int nReason);

    // 当客户端发出登录请求之后，该方法会被调用，通知客户端登录是否成功
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
        bool bIsLast);

    ///订阅询价应答
    virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    ///取消订阅询价应答
    virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);

    ///询价通知
    virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);

    // 针对用户请求的出错通知
    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    bool Logoned() const
    {
        return logoned_;
    }

private:
    CThostFtdcMdApi *api_;
    char ** pp_instruments_;
    std::string instruments_;
    int sub_count_;

    bool logoned_;

    // 数据处理函数对象
    RtnForQuoteHandler quote_data_handler_;
    T_RtnForQuote data_;
};

#endif // GTA_QUOTE_INTERFACE_H_
