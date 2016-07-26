#pragma once

#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <list>
#include "SecurityFtdcMdApi.h"

#include <boost/function.hpp>

#include "quote_interface_option_lts.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

typedef std::map<std::string, std::list<CSecurityFtdcInstrumentField> > OptionsOfEx;
typedef std::unordered_map<std::string, CSecurityFtdcInstrumentField > OptionOfContract;

class MYLTSDataHandler: public CSecurityFtdcMdSpi
{
public:
    // 构造函数
    MYLTSDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const T_OptionQuote *)> quote_data_handler);

    virtual ~MYLTSDataHandler(void);

    // 当客户端与交易托管系统建立起通信连接，客户端需要进行登录
    virtual void OnFrontConnected();

    // 当客户端与交易托管系统通信连接断开时，该方法被调用
    virtual void OnFrontDisconnected(int nReason);

    // 当客户端发出登录请求之后，该方法会被调用，通知客户端登录是否成功
    virtual void OnRspUserLogin(CSecurityFtdcRspUserLoginField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);

    ///订阅行情应答
    virtual void OnRspSubMarketData(CSecurityFtdcSpecificInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);

    ///取消订阅行情应答
    virtual void OnRspUnSubMarketData(CSecurityFtdcSpecificInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);

    // 行情应答
    virtual void OnRtnDepthMarketData(CSecurityFtdcDepthMarketDataField *pf);

    // 针对用户请求的出错通知
    virtual void OnRspError(CSecurityFtdcRspInfoField *rsp, int req_id, bool last);

    bool Logoned() const
    {
        return logoned_;
    }

private:
    CSecurityFtdcMdApi *api_;

    // options for subsribe from market
    OptionsOfEx options_;
    char ** pp_instruments_;
    char * p_ex_;
    std::string instruments_;
    int sub_count_;

    bool logoned_;
    void ReqLogin();

    // 数据处理函数对象
    boost::function<void(const T_OptionQuote *)> quote_data_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    char * broker_id_;
    char * user_;
    char * password_;

    // save assistant object
    QuoteDataSave<T_OptionQuote> *p_save_;
    OptionOfContract option_info_;
    void RalaceInvalidValue_LTS(CSecurityFtdcDepthMarketDataField &d);
    T_OptionQuote Convert(const CSecurityFtdcDepthMarketDataField &lts_data);

    std::string qtm_name;
};
