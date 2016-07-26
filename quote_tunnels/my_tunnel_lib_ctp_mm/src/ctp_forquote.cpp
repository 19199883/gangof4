#include "ctp_forquote.h"
#include <iomanip>
#include <vector>

#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"

using namespace std;

MYCTPDataHandler::MYCTPDataHandler(const Contracts &contracts, const std::string &quote_front_addr)
{
    // 初始化
    logoned_ = false;
    api_ = NULL;

    sub_count_ = contracts.size();
    pp_instruments_ = new char *[sub_count_];
    int i = 0;
    for (const std::string &value : contracts)
    {
        instruments_.append(value + "|");
        pp_instruments_[i] = new char[value.length() + 1];
        memcpy(pp_instruments_[i], value.c_str(), value.length() + 1);
        ++i;
    }

    // 初始化
    api_ = CThostFtdcMdApi::CreateFtdcMdApi();
    api_->RegisterSpi(this);

    // set front address
    char *addr_tmp = new char[quote_front_addr.size() + 1];
    memcpy(addr_tmp, quote_front_addr.c_str(), quote_front_addr.size() + 1);
    api_->RegisterFront(addr_tmp);
    TNL_LOG_INFO("CTP_ForQuote - RegisterFront, addr: %s", addr_tmp);
    delete[] addr_tmp;

    api_->Init();
}

MYCTPDataHandler::~MYCTPDataHandler(void)
{
    if (api_)
    {
        api_->RegisterSpi(NULL);
        api_->Release();
        api_ = NULL;
    }
}

void MYCTPDataHandler::OnFrontConnected()
{
    MY_LOG_INFO("CTP_ForQuote: OnFrontConnected");

    CThostFtdcReqUserLoginField req_login;
    memset(&req_login, 0, sizeof(CThostFtdcReqUserLoginField));
    api_->ReqUserLogin(&req_login, 0);

    MY_LOG_INFO("CTP_ForQuote - request logon");
}

void MYCTPDataHandler::OnFrontDisconnected(int nReason)
{
    logoned_ = false;
    MY_LOG_ERROR("CTP_ForQuote - OnFrontDisconnected, nReason: %d", nReason);
}

void MYCTPDataHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
{
    int error_code = pRspInfo ? pRspInfo->ErrorID : 0;
    MY_LOG_INFO("CTP_ForQuote - OnRspUserLogin, error code: %d", error_code);

    if (error_code == 0)
    {
        logoned_ = true;
        int ret = api_->SubscribeForQuoteRsp(pp_instruments_, sub_count_);
        MY_LOG_INFO("CTP_ForQuote - SubscribeForQuoteRsp, return: %d; codelist: %s", ret, instruments_.c_str());
    }
    else
    {
        std::string err_str("null");
        if (pRspInfo && pRspInfo->ErrorMsg[0] != '\0')
        {
            err_str = pRspInfo->ErrorMsg;
        }
        MY_LOG_WARN("CTP_ForQuote - Logon fail, error code: %d; error info: %s", error_code, err_str.c_str());
    }
}

///订阅询价应答
void MYCTPDataHandler::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    MY_LOG_INFO("CTP_ForQuote - OnRspSubForQuoteRsp, code: %s", pSpecificInstrument->InstrumentID);
}

///取消订阅询价应答
void MYCTPDataHandler::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    MY_LOG_INFO("CTP_ForQuote - OnRspUnSubForQuoteRsp, code: %s", pSpecificInstrument->InstrumentID);
}

void MYCTPDataHandler::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *data)
{
    try
    {
        if (quote_data_handler_)
        {
            strncpy(data_.stock_code, data->InstrumentID, sizeof(data_.stock_code));
            strncpy(data_.for_quote_id, data->ForQuoteSysID, sizeof(data_.for_quote_id));
            strncpy(data_.for_quote_time, data->ForQuoteTime, sizeof(data_.for_quote_time));

            quote_data_handler_(&data_);
        }

        MY_LOG_INFO("CTP_ForQuote - OnRtnForQuoteRsp, code: %s", data->InstrumentID);
    }
    catch (...)
    {
        MY_LOG_FATAL("CTP_ForQuote - Unknown exception in OnRtnForQuoteRsp.");
    }
}

void MYCTPDataHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int error_code = pRspInfo ? 0 : pRspInfo->ErrorID;
    if (error_code != 0)
    {
        MY_LOG_INFO("CTP_ForQuote - OnRspError, code: %d; info: %s", error_code, pRspInfo->ErrorMsg);
    }
}
