#pragma once

#include "SecurityFtdcQueryApi.h"
#include "lts_trade_interface.h"

class LTSQueryInf: public CSecurityFtdcQuerySpi
{
public:
    LTSQueryInf(LTSTradeInf *tr_)
        : tr(tr_)
    {
    }

    virtual ~LTSQueryInf(void)
    {
    }

    // overriding functions of CSecurityFtdcQuerySpi
    virtual void OnFrontConnected()
    {
        return tr->OnQueryFrontConnected();
    }

    virtual void OnFrontDisconnected(int reason)
    {
        return tr->OnQueryFrontDisconnected(reason);
    }

    virtual void OnHeartBeatWarning(int time_lapse)
    {
        return tr->OnQueryHeartBeatWarning(time_lapse);
    }

    virtual void OnRspError(CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnQueryRspError(rsp, req_id, last);
    }
    virtual void OnRspUserLogin(CSecurityFtdcRspUserLoginField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnQueryRspUserLogin(pf, rsp, req_id, last);
    }
    virtual void OnRspUserLogout(CSecurityFtdcUserLogoutField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnQueryRspUserLogout(pf, rsp, req_id, last);
    }
    virtual void OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnQueryRspFetchAuthRandCode(pf, rsp, req_id, last);
    }

    virtual void OnRspQryExchange(CSecurityFtdcExchangeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryExchange(pf, rsp, req_id, last);
    }
    virtual void OnRspQryInstrument(CSecurityFtdcInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryInstrument(pf, rsp, req_id, last);
    }
    virtual void OnRspQryInvestor(CSecurityFtdcInvestorField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryInvestor(pf, rsp, req_id, last);
    }
    virtual void OnRspQryTradingCode(CSecurityFtdcTradingCodeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryTradingCode(pf, rsp, req_id, last);
    }
    virtual void OnRspQryTradingAccount(CSecurityFtdcTradingAccountField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryTradingAccount(pf, rsp, req_id, last);
    }
    virtual void OnRspQryMarketRationInfo(CSecurityFtdcMarketRationInfoField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryMarketRationInfo(pf, rsp, req_id, last);
    }
    virtual void OnRspQryInstrumentCommissionRate(CSecurityFtdcInstrumentCommissionRateField *pf, CSecurityFtdcRspInfoField *rsp, int req_id,
        bool last)
    {
        return tr->OnRspQryInstrumentCommissionRate(pf, rsp, req_id, last);
    }
    virtual void OnRspQryETFInstrument(CSecurityFtdcETFInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryETFInstrument(pf, rsp, req_id, last);
    }
    virtual void OnRspQryETFBasket(CSecurityFtdcETFBasketField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryETFBasket(pf, rsp, req_id, last);
    }
    virtual void OnRspQryOFInstrument(CSecurityFtdcOFInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryOFInstrument(pf, rsp, req_id, last);
    }
    virtual void OnRspQrySFInstrument(CSecurityFtdcSFInstrumentField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQrySFInstrument(pf, rsp, req_id, last);
    }
    virtual void OnRspQryInstrumentUnitMargin(CSecurityFtdcInstrumentUnitMarginField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryInstrumentUnitMargin(pf, rsp, req_id, last);
    }
    virtual void OnRspQryMarketDataStaticInfo(CSecurityFtdcMarketDataStaticInfoField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryMarketDataStaticInfo(pf, rsp, req_id, last);
    }
    virtual void OnRspQryOrder(CSecurityFtdcOrderField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryOrder(pf, rsp, req_id, last);
    }
    virtual void OnRspQryTrade(CSecurityFtdcTradeField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryTrade(pf, rsp, req_id, last);
    }
    virtual void OnRspQryInvestorPosition(CSecurityFtdcInvestorPositionField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
    {
        return tr->OnRspQryInvestorPosition(pf, rsp, req_id, last);
    }

private:
    LTSTradeInf *tr;
};

