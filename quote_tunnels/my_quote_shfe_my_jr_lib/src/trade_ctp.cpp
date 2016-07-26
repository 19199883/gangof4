#include "trade_ctp.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <boost/thread.hpp>

#include "quote_cmn_utility.h"

MYCTPTradeInterface::~MYCTPTradeInterface()
{
    if (p_trader_api_)
    {
        p_trader_api_->RegisterSpi(NULL);
        p_trader_api_->Release();
        p_trader_api_ = NULL;
    }
}

void MYCTPTradeInterface::OnFrontConnected()
{
    MY_LOG_INFO("CTP-Trade - OnFrontConnected.");

    // 已经获取合约列表了，无需登录
    if (securities_get_finished)
    {
        return;
    }

    CThostFtdcReqUserLoginField user_logon;
    memset(&user_logon, 0, sizeof(user_logon));
    memcpy(user_logon.BrokerID, broker_id_, strlen(broker_id_));
    memcpy(user_logon.UserID, user_, strlen(user_));
    memcpy(user_logon.Password, password_, strlen(password_));

    // 发出登陆请求
    p_trader_api_->ReqUserLogin(&user_logon, 1);
    MY_LOG_INFO("CTP-Trade - try to login trade server.");
}

void MYCTPTradeInterface::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // 登陆了，则查查信息
    if (pRspInfo == NULL || pRspInfo->ErrorID == 0)
    {
        MY_LOG_INFO("CTP-Trade - OnRspUserLogin success.");
    }
    else
    {
        MY_LOG_ERROR("CTP-Trade - OnRspUserLogin, ErrorID: %d; ErrorMsg: %s",
            pRspInfo->ErrorID, pRspInfo->ErrorMsg);

        // 登录失败，不断重试
        boost::this_thread::sleep(boost::posix_time::millisec(1000));
        CThostFtdcReqUserLoginField user_logon;
        memset(&user_logon, 0, sizeof(user_logon));
        memcpy(user_logon.BrokerID, broker_id_, strlen(broker_id_));
        memcpy(user_logon.UserID, user_, strlen(user_));
        memcpy(user_logon.Password, password_, strlen(password_));

        // 发出登陆请求
        p_trader_api_->ReqUserLogin(&user_logon, 1);
    }

    if ((pRspInfo == NULL || pRspInfo->ErrorID == 0) && bIsLast)
    {
        // 查询 contract
        CThostFtdcQryInstrumentField instrument_struct;
        memset(&instrument_struct, 0, sizeof(instrument_struct));
        memcpy(instrument_struct.InstrumentID, "", strlen(""));
        memcpy(instrument_struct.ExchangeID, "SHFE", strlen("SHFE"));
        memcpy(instrument_struct.ExchangeInstID, "", strlen(""));
        memcpy(instrument_struct.ProductID, "", strlen(""));

        boost::this_thread::sleep(boost::posix_time::millisec(1000)); // 查询要间隔1s
        int ret = p_trader_api_->ReqQryInstrument(&instrument_struct, ++nRequestID);
        MY_LOG_INFO("ReqQryInstrument, return_code: %d", ret);
    }
}

void MYCTPTradeInterface::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo == NULL || pRspInfo->ErrorID == 0)
    {
        MY_LOG_INFO("CTP-Trade - OnRspUserLogout success.");
        securities_get_finished = true;
    }
    else
    {
        MY_LOG_ERROR("CTP-Trade - OnRspUserLogout, ErrorID: %d; ErrorMsg: %s",
            pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
}

void MYCTPTradeInterface::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // 成功了
    if (pRspInfo == NULL || pRspInfo->ErrorID == 0)
    {
        if (pInstrument)
        {
            MY_LOG_INFO("OnRspQryInstrument, InstrumentID: %s"
                "; ExchangeID: %s"
                "; InstrumentName: %s"
                "; ExchangeInstID: %s"
                "; ProductID: %s"
                "; ProductClass: %c"
                "; DeliveryYear: %04d"
                "; DeliveryMonth: %02d"
                "; MaxMarketOrderVolume: %d"
                "; MinMarketOrderVolume: %d"
                "; MaxLimitOrderVolume: %d"
                "; MinLimitOrderVolume: %d"
                "; VolumeMultiple: %d"
                "; PriceTick: %0.4f"
                "; CreateDate: %s"
                "; OpenDate: %s"
                "; ExpireDate: %s"
                "; StartDelivDate: %s"
                "; EndDelivDate: %s"
                "; InstLifePhase: %c"
                "; IsTrading: %d"
                "; PositionType: %c"
                "; PositionDateType: %c"
                "; LongMarginRatio: %0.4f"
                "; ShortMarginRatio: %.4f",
                pInstrument->InstrumentID,
                pInstrument->ExchangeID,
                pInstrument->InstrumentName,
                pInstrument->ExchangeInstID,
                pInstrument->ProductID,
                pInstrument->ProductClass,
                pInstrument->DeliveryYear,
                pInstrument->DeliveryMonth,
                pInstrument->MaxMarketOrderVolume,
                pInstrument->MinMarketOrderVolume,
                pInstrument->MaxLimitOrderVolume,
                pInstrument->MinLimitOrderVolume,
                pInstrument->VolumeMultiple,
                pInstrument->PriceTick,
                pInstrument->CreateDate,
                pInstrument->OpenDate,
                pInstrument->ExpireDate,
                pInstrument->StartDelivDate,
                pInstrument->EndDelivDate,
                pInstrument->InstLifePhase,
                pInstrument->IsTrading,
                pInstrument->PositionType,
                pInstrument->PositionDateType,
                InvalidToZeroD(pInstrument->LongMarginRatio),
                InvalidToZeroD(pInstrument->ShortMarginRatio)
                    );
            securities_exchcode.insert(std::make_pair(pInstrument->InstrumentID,
                ExchNameToExchCode(pInstrument->ExchangeID)));
        }
        else
        {
            MY_LOG_ERROR("OnRspQryInstrument, response info is null.");
        }
    }
    else
    {
        MY_LOG_ERROR("OnRspQryInstrument, ErrorID: %d; ErrorMsg: %s",
            pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }

    if (bIsLast)
    {
        MY_LOG_INFO("OnRspQryInstrument, task finished.");

        // 处理完毕，登出，节约交易连接数
        CThostFtdcUserLogoutField user_logout;
        memset(&user_logout, 0, sizeof(user_logout));
        memcpy(user_logout.BrokerID, broker_id_, strlen(broker_id_));
        memcpy(user_logout.UserID, user_, strlen(user_));

        // 发出登出请求
        boost::this_thread::sleep(boost::posix_time::millisec(1000)); // 间隔1s
        int ret = p_trader_api_->ReqUserLogout(&user_logout, ++nRequestID);
        MY_LOG_INFO("ReqUserLogout, return_code: %d", ret);
    }
}

MYCTPTradeInterface::MYCTPTradeInterface(const ConfigData &cfg)
{
    const LogonConfig &logon_info = cfg.Logon_config();

    front_addr_ = new char[logon_info.trade_server_addr.size() + 1];
    memcpy(front_addr_, logon_info.trade_server_addr.data(), logon_info.trade_server_addr.size() + 1);

    broker_id_ = new char[logon_info.broker_id.size() + 1];
    memcpy(broker_id_, logon_info.broker_id.data(), logon_info.broker_id.size() + 1);

    user_ = new char[logon_info.account.size() + 1];
    memcpy(user_, logon_info.account.data(), logon_info.account.size() + 1);

    password_ = new char[logon_info.password.size() + 1];
    memcpy(password_, logon_info.password.data(), logon_info.password.size() + 1);

    nRequestID = 1;

    p_trader_api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
    p_trader_api_->RegisterSpi(this);

    p_trader_api_->RegisterFront(front_addr_);
    p_trader_api_->SubscribePrivateTopic(THOST_TERT_QUICK);
    p_trader_api_->Init();
}

MYSecurityCollection MYCTPTradeInterface::securities_exchcode;

volatile bool MYCTPTradeInterface::securities_get_finished = false;

