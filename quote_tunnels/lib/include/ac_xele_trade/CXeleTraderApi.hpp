/////////////////////////////////////////////////////////////////////////
///@system Xele-Trade
///@company AcceleCom
///@file CXeleFtdcTraderApi.hpp
///@brief 定义了客户端接口
///@history 
/////////////////////////////////////////////////////////////////////////

#if !defined(_XELE_FTDCTRADERAPI_H)

#define _XELE_FTDCTRADERAPI_H

#include "CXeleFtdcUserApiStruct.h"

class CXeleTraderSpi
{
public:
    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected()
    {
    }
    ;

    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param nReason 错误代码
    virtual void OnFrontDisconnected(int nReason)
    {
    }
    ;

    ///报文回调开始通知。当API收到一个报文后，首先调用本方法，然后是各数据域的回调，最后是报文回调结束通知。
    ///@param nTopicID 主题代码（如私有流、公共流、行情流等）
    ///@param nSequenceNo 报文序号
    virtual void OnPackageStart(int nTopicID, int nSequenceNo)
    {
    }
    ;

    ///报文回调结束通知。当API收到一个报文后，首先调用报文回调开始通知，然后是各数据域的回调，最后调用本方法。
    ///@param nTopicID 主题代码（如私有流、公共流、行情流等）
    ///@param nSequenceNo 报文序号
    virtual void OnPackageEnd(int nTopicID, int nSequenceNo)
    {
    }
    ;

    ///错误应答
    virtual void OnRspError(CXeleFtdcRspInfoField *pRspInfo,
                            int nRequestID,
                            bool bIsLast)
    {
    }
    ;

    ///用户登录应答
    virtual void OnRspUserLogin(CXeleFtdcRspUserLoginField *pRspUserLogin,
                                CXeleFtdcRspInfoField *pRspInfo,
                                int nRequestID,
                                bool bIsLast)
    {
    }
    ;

    ///用户退出应答
    virtual void OnRspUserLogout(CXeleFtdcRspUserLogoutField *pRspUserLogout,
                                 CXeleFtdcRspInfoField *pRspInfo,
                                 int nRequestID,
                                 bool bIsLast)
    {
    }
    ;

    ///报单录入应答
    virtual void OnRspOrderInsert(CXeleFtdcInputOrderField *pInputOrder,
                                  CXeleFtdcRspInfoField *pRspInfo,
                                  int nRequestID,
                                  bool bIsLast)
    {
    }
    ;

    ///报单操作应答
    virtual void OnRspOrderAction(CXeleFtdcOrderActionField *pOrderAction,
                                  CXeleFtdcRspInfoField *pRspInfo,
                                  int nRequestID,
                                  bool bIsLast)
    {
    }
    ;

    ///客户持仓查询应答
    virtual void OnRspQryClientPosition(CXeleFtdcRspClientPositionField *pRspClientPosition,
                                        CXeleFtdcRspInfoField *pRspInfo,
                                        int nRequestID,
                                        bool bIsLast)
    {
    }
    ;

    ///合约查询应答
    virtual void OnRspQryInstrument(CXeleFtdcRspInstrumentField *pRspInstrument,
                                    CXeleFtdcRspInfoField *pRspInfo,
                                    int nRequestID,
                                    bool bIsLast)
    {
    }
    ;

    ///客户资金查询应答
    virtual void OnRspQryClientAccount(CXeleFtdcRspClientAccountField *pClientAccount,
                                       CXeleFtdcRspInfoField *pRspInfo,
                                       int nRequestID,
                                       bool bIsLast)
    {
    }
    ;
    ///合约保证金率查询应答
    virtual void OnRspQryInstrumentMarginRate(CXeleFtdcRspInstrumentMarginRateField *pRspInstrumentMarginRate,
                                              CXeleFtdcRspInfoField *pRspInfo,
                                              int nRequestID,
                                              bool bIsLast)
    {
    }
    ;
    ///合约手续费率查询应答
    virtual void OnRspQryInstrumentCommissionRate(CXeleFtdcRspInstrumentCommissionRateField *pRspInstrumentCommissionRate,
                                                  CXeleFtdcRspInfoField *pRspInfo,
                                                  int nRequestID,
                                                  bool bIsLast)
    {
    }
    ;

    virtual void OnRspUserPasswordUpdate(CXeleFtdcUserPasswordUpdateField *pUserPasswordUpdate,
                                         CXeleFtdcRspInfoField *pRspInfo,
                                         int nRequestID,
                                         bool bIsLast)
    {
    }
    ;

    ///成交回报
    virtual void OnRtnTrade(CXeleFtdcTradeField *pTrade)
    {
    }
    ;

    ///报单回报
    virtual void OnRtnOrder(CXeleFtdcOrderField *pOrder)
    {
    }
    ;

    ///合约交易状态通知
    virtual void OnRtnInstrumentStatus(CXeleFtdcInstrumentStatusField *pInstrumentStatus)
    {
    }
    ;

    ///增加合约通知
    virtual void OnRtnInsInstrument(CXeleFtdcInstrumentField *pInstrument)
    {
    }
    ;

    ///报单录入错误回报
    virtual void OnErrRtnOrderInsert(CXeleFtdcInputOrderField *pInputOrder,
                                     CXeleFtdcRspInfoField *pRspInfo)
    {
    }
    ;

    ///报单操作错误回报
    virtual void OnErrRtnOrderAction(CXeleFtdcOrderActionField *pOrderAction,
                                     CXeleFtdcRspInfoField *pRspInfo)
    {
    }
    ;
    ///报单查询应答
    virtual void OnRspQryOrder(CXeleFtdcOrderField* pOrderField, CXeleFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast){};
    ///成交单查询应答
    virtual void OnRspQryTrade(CXeleFtdcTradeField* pTradeField, CXeleFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast){};

    virtual void OnRspInstrumentPrice(CXeleFtdcRspInstrumentPriceField* pTradeField, CXeleFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast){};
};

class CXeleTraderApi
{
public:
    ///创建TraderApi
    ///@return 创建出的UserApi
    ///exchange=1(上期所），0或其他（中金所）
    static CXeleTraderApi *CreateTraderApi(int exchangeID=0);

    ///获取系统版本号
    ///@return 系统标识字符串
    static const char *GetVersion();

    ///删除接口对象本身
    ///@remark 不再使用本接口对象时,调用该函数删除接口对象
    virtual void Release() = 0;

    ///初始化
    ///@remark 初始化运行环境,只有调用后,接口才开始工作
    virtual void Init() = 0;

    ///等待接口线程结束运行
    ///@return 线程退出代码
    virtual int Join() = 0;

    ///注册前置机网络地址
    ///@param pszFrontAddress：前置机网络地址。
    ///@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:17001”。
    ///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”17001”代表服务器端口号。
    virtual void RegisterFront(char *pszFrontAddress) = 0;

    ///注册回调接口
    ///@param pSpi 派生自回调接口类的实例
    virtual void RegisterSpi(CXeleTraderSpi *pSpi) = 0;

    ///订阅私有流。
    ///@param nResumeType 私有流重传方式
    ///        TERT_RESTART:从本交易日开始重传
    ///        TERT_RESUME:从上次收到的续传
    ///        TERT_QUICK:只传送登录后私有流的内容
    ///@remark 该方法要在Init方法前调用。若不调用则不会收到私有流的数据。
    virtual void SubscribePrivateTopic(XELE_TE_RESUME_TYPE nResumeType) = 0;

    ///订阅公共流。
    ///@param nResumeType 公共流重传方式
    ///        TERT_RESTART:从本交易日开始重传
    ///        TERT_RESUME:从上次收到的续传
    ///        TERT_QUICK:只传送登录后公共流的内容
    ///@remark 该方法要在Init方法前调用。若不调用则不会收到公共流的数据。
    virtual void SubscribePublicTopic(XELE_TE_RESUME_TYPE nResumeType) = 0;

    ///订阅交易员流。
    ///@param nResumeType 交易员流重传方式
    ///        TERT_RESTART:从本交易日开始重传
    ///        TERT_RESUME:从上次收到的续传
    ///        TERT_QUICK:只传送登录后交易员流的内容
    ///@remark 该方法要在Init方法前调用。若不调用则不会收到交易员流的数据。
    virtual void SubscribeUserTopic(XELE_TE_RESUME_TYPE nResumeType) = 0;

    ///用户登录请求
    virtual int ReqUserLogin(CXeleFtdcReqUserLoginField *pReqUserLogin,
                             int nRequestID) = 0;

    ///用户退出请求
    virtual int ReqUserLogout(CXeleFtdcReqUserLogoutField *pReqUserLogout,
                              int nRequestID) = 0;

    ///报单录入请求
    virtual int ReqOrderInsert(CXeleFtdcInputOrderField *pInputOrder,
                               int nRequestID) = 0;

    ///报单操作请求
    virtual int ReqOrderAction(CXeleFtdcOrderActionField *pOrderAction,
                               int nRequestID) = 0;

    ///客户资金查询
    virtual int ReqQryClientAccount(CXeleFtdcQryClientAccountField *pQryClientAccount,
                                    int nRequestID) = 0;
    ///客户持仓查询请求
    virtual int ReqQryClientPosition(CXeleFtdcQryClientPositionField *pQryClientPosition,
                                     int nRequestID) = 0;

    ///合约查询请求
    virtual int ReqQryInstrument(CXeleFtdcQryInstrumentField *pQryInstrument,
                                 int nRequestID) = 0;

    ///合约保证金率查询
    virtual int ReqQryInstrumentMarginRate(CXeleFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate,
                                           int nRequestID) = 0;
    ///合约手续费率查询
    virtual int ReqQryInstrumentCommissionRate(CXeleFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate,
                                               int nRequestID) = 0;
    virtual int ReqUserPasswordUpdate(CXeleFtdcUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID) = 0;
    ///报单查询
    virtual int ReqQryOrder(CXeleFtdcQryOrderField *pQryOrder, int nRequestID) = 0;
    ///成交查询
    virtual int ReqQryTrade(CXeleFtdcQryTradeField *pQryTrade, int nRequestID) = 0;

    virtual int ReqQryInstrumentPrice(CXeleFtdcQryInstrumentField *pQryTrade, int nRequestID) = 0;
protected:
    ~CXeleTraderApi();
};

#endif
