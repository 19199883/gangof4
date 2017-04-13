#ifndef XSPEED_QUOTE_INTERFACE_H_
#define XSPEED_QUOTE_INTERFACE_H_

#include <string>
#include <sstream>
#include <list>
#include "DFITCMdApi.h"

#include <boost/function.hpp>

//#include "quote_interface_level1.h"
#include "quote_datatype_level1.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

using namespace DFITCXSPEEDMDAPI;

// 订阅的合约，集合类型
 typedef std::set<std::string> SubscribeContracts;

//
/// 继承实现Spi函数：当 pRspInfo/pErrorInfo 不为NULL时，表示有错误信息
/// 否则其值将为NULL，Spi中的其它参数均不会为NULL
class MyMdSpi : public DFITCXSPEEDMDAPI::DFITCMdSpi
{
public:
    // 构造函数
    MyMdSpi(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const CDepthMarketDataField *)> quote_data_handler);

    virtual ~MyMdSpi(void);

    // 当客户端与交易托管系统建立起通信连接，客户端需要进行登录
    virtual void OnFrontConnected();

    // 当客户端与交易托管系统通信连接断开时，该方法被调用
    virtual void OnFrontDisconnected(int nReason);

    // 当客户端发出登录请求之后，该方法会被调用，通知客户端登录是否成功
    virtual void OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pRspUserLogin, struct DFITCErrorRtnField * pRspInfo);

	virtual void OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pRspUsrLogout, struct DFITCErrorRtnField * pRspInfo);

    ///订阅行情应答
    virtual void OnRspSubMarketData(struct DFITCSpecificInstrumentField *pSpecificInstrument, struct DFITCErrorRtnField *pErrorInfo);

    ///取消订阅行情应答
    virtual void OnRspUnSubMarketData(struct DFITCSpecificInstrumentField * pSpecificInstrument, struct DFITCErrorRtnField * pRspInfo);

    // 行情应答
    virtual void OnMarketData(struct DFITCDepthMarketDataField * pMarketDataField);

    // 针对用户请求的出错通知
    virtual void OnRspError(struct DFITCErrorRtnField *pRspInfo);

    bool Logoned() const
    {
        return logoned_;
    }

private:
	DFITCXSPEEDMDAPI::DFITCMdApi *api_;
    char ** pp_instruments_;
    std::string instruments_;
    int sub_count_;

    bool logoned_;

    // 数据处理函数对象
    boost::function<void(const CDepthMarketDataField *)> quote_data_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;

    // save assistant object
    QuoteDataSave<CDepthMarketDataField> *p_save_;

	// TODO: test, commented by wangying
//void RalaceInvalidValue_CTP(CThostFtdcDepthMarketDataField &d);
//		CDepthMarketDataField Convert(const CThostFtdcDepthMarketDataField &ctp_data);

    std::string qtm_name;
};

#endif // XSPEED_QUOTE_INTERFACE_H_
