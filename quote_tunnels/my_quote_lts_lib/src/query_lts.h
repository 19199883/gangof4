#pragma once

#include "SecurityFtdcQueryApi.h"

#include <unordered_map>
#include <string>

#include "quote_cmn_config.h"

#ifdef USE_LTS_API_2
#include "SecurityFtdcTraderApi2.h"
#include "SecurityFtdcQueryApi2.h"

#define CSecurityFtdcTraderApi CSecurityFtdcTraderApi2
#define CSecurityFtdcQueryApi CSecurityFtdcQueryApi2
#endif

typedef std::unordered_map<std::string, CSecurityFtdcInstrumentField> MYSecurityCollection;
class MYLTSQueryInterface: public CSecurityFtdcQuerySpi
{
public:
    static MYSecurityCollection securities_exchcode;

    MYLTSQueryInterface(const ConfigData &cfg);
    virtual ~MYLTSQueryInterface();

    bool TaskFinished()
    {
        return securities_get_finished_ && (login_flag_ == false);
    }

    // overriding functions of CSecurityFtdcQuerySpi
    virtual void OnFrontConnected();

    virtual void OnFrontDisconnected(int reason);

    virtual void OnHeartBeatWarning(int time_lapse);

    virtual void OnRspError(CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    virtual void OnRspUserLogin(CSecurityFtdcRspUserLoginField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    virtual void OnRspUserLogout(CSecurityFtdcUserLogoutField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    virtual void OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);

    virtual void OnRspQryExchange(CSecurityFtdcExchangeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryInstrument(CSecurityFtdcInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last);
    virtual void OnRspQryInvestor(CSecurityFtdcInvestorField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryTradingCode(CSecurityFtdcTradingCodeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryTradingAccount(CSecurityFtdcTradingAccountField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryMarketRationInfo(CSecurityFtdcMarketRationInfoField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryInstrumentCommissionRate(CSecurityFtdcInstrumentCommissionRateField *pf, CSecurityFtdcRspInfoField *rsp, int req_id,
        bool last)
    {
    }
    virtual void OnRspQryETFInstrument(CSecurityFtdcETFInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryETFBasket(CSecurityFtdcETFBasketField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryOFInstrument(CSecurityFtdcOFInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQrySFInstrument(CSecurityFtdcSFInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryInstrumentUnitMargin(CSecurityFtdcInstrumentUnitMarginField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryMarketDataStaticInfo(CSecurityFtdcMarketDataStaticInfoField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryOrder(CSecurityFtdcOrderField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryTrade(CSecurityFtdcTradeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }
    virtual void OnRspQryInvestorPosition(CSecurityFtdcInvestorPositionField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
    }

private:
    CSecurityFtdcQueryApi *p_query_api_;

    // 连接登陆参数
    char * front_addr_;
    char * broker_id_;
    char * user_;
    char * password_;
    char * user_productinfo_;
    char * auth_code_;
    SubsribeDatas sub_codes_;

    TSecurityFtdcAuthCodeType rand_code_;

    int req_id_;
    bool login_flag_;
    bool securities_get_finished_;

    void QueryReqLogin(int wait_seconds);
    void QueryMDStaticInfo();
    void QueryReqLogout();
};
