#pragma once

#include <string>
#include <sstream>
#include <list>
#include <unordered_map>
#include <map>
#include "SgitFtdcMdApi.h"
#include <boost/function.hpp>
#include "quote_interface_sgit.h"
#include "quote_sgit_data_type.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

class SgitDataHandler : public CSgitFtdcMdSpi
{
public:
    // 构造函数 
	SgitDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 构造函数，需要一个有效的指向CSgitFtdcMduserApi实例的指针
    SgitDataHandler(CSgitFtdcMdApi *pUserApi):api_(pUserApi) {}

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const SGIT_Depth_Market_Data_Field  *)> quote_data_handler);

    ~SgitDataHandler(void);

    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    void OnFrontConnected(void);
    
    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param pErrMsg 错误原因

    void OnFrontDisconnected(char *pErrMsg);

    ///登录请求响应
    void OnRspUserLogin(CSgitFtdcRspUserLoginField *pRspUserLogin, CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///登出请求响应
    void OnRspUserLogout(CSgitFtdcUserLogoutField *pUserLogout, CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///深度行情通知
    void OnRtnDepthMarketData(CSgitFtdcDepthMarketDataField *pDepthMarketData);

    ///// 针对用户请求的出错通知
    void OnRspError(CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	bool Logoned() const { return logoned_; }

private:
	CSgitFtdcMdApi  *api_;

	bool logoned_;

    // 数据处理函数对象
    boost::function<void(const SGIT_Depth_Market_Data_Field *)> quote_data_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;

    // save assistant object
    QuoteDataSave<SGIT_Depth_Market_Data_Field> *p_save_;
    struct SGIT_Depth_Market_Data_Field Convert(const CSgitFtdcDepthMarketDataField &sgit_data);

    std::string qtm_name;
};
