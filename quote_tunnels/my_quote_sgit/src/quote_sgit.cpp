#include "quote_sgit.h"
#include <iomanip>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

#include "quote_cmn_config.h"
#include "quote_cmn_utility.h"

using namespace my_cmn;
using namespace std;

SgitDataHandler::SgitDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_save_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }
    // 初始化
    logoned_ = false;
    api_ = NULL;
    char qtm_name_buf[64];
    sprintf(qtm_name_buf, "sgit_%lu", getpid());
    qtm_name = qtm_name_buf;
    QuoteUpdateState(qtm_name.c_str(), QtmState::INIT);

    p_save_ = new QuoteDataSave<struct SGIT_Depth_Market_Data_Field>(cfg_, qtm_name, "sgit_data", SGIT_QUOTE_TYPE);

    SubsribeDatas code_list = cfg_.Subscribe_datas();
    const LogonConfig &logon_cfg = cfg_.Logon_config();

    // 初始化
    const char *store_dir = "../store/";
    api_ = CSgitFtdcMdApi::CreateFtdcMdApi(store_dir);
    MY_LOG_INFO("call  CSgitFtdcMdApi::CreateFtdcMdApi(store_dir);");

    api_->RegisterSpi(this);
    MY_LOG_INFO("call  api_->RegisterSpi(this);");

    // set front address
    char *addr_tmp = NULL;
    for (const std::string &v : logon_cfg.quote_provider_addrs)
    {
        addr_tmp = new char[v.size() + 1];
        memcpy(addr_tmp, v.c_str(), v.size() + 1);
        MY_LOG_INFO("sgit - RegisterFront, addr: %s", addr_tmp);
        break;
    }

    ///订阅市场流。
    ///@param nResumeType 公共流重传方式
    ///        Sgit_TERT_RESTART:从本交易日开始重传
    ///        Sgit_TERT_RESUME:从上次收到的续传
    ///        Sgit_TERT_QUICK:只传送登录后公共流的内容
    ///@remark 该方法要在Init方法前调用。若不调用则不会收到公共流的数据。
    api_->SubscribeMarketTopic(Sgit_TERT_QUICK);

    // 设置交易系统行情前置NameServer的地址
    api_->RegisterFront(addr_tmp);
    QuoteUpdateState(qtm_name.c_str(), QtmState::SET_ADDRADRESS_SUCCESS);

    ///初始化
    ///@remark 初始化运行环境,只有调用后,接口才开始工作
    ///isLogged 开发调试时使用true，可以打印出收到的消息包内容
    api_->Init(false);
    MY_LOG_INFO("call   api_->Init(false);");
}

SgitDataHandler::~SgitDataHandler(void)
{
    if (api_)
    {
        api_->Release();
        api_ = NULL;
    }
}

void SgitDataHandler::OnFrontConnected()
{
    MY_LOG_INFO("sgit - OnFrontConnected");

    QuoteUpdateState(qtm_name.c_str(), QtmState::CONNECT_SUCCESS);

    CSgitFtdcReqUserLoginField reqUserLogin;
    memset(&reqUserLogin, 0, sizeof(reqUserLogin));
//    strcpy(reqUserLogin.TradingDay, api_->GetTradingDay());
//    strncpy(reqUserLogin.BrokerID, cfg_.Logon_config().broker_id.c_str(), sizeof(reqUserLogin.BrokerID));
    strncpy(reqUserLogin.UserID, cfg_.Logon_config().account.c_str(), sizeof(reqUserLogin.UserID));
    strncpy(reqUserLogin.Password, cfg_.Logon_config().password.c_str(), sizeof(reqUserLogin.Password));
    int ret = api_->ReqUserLogin(&reqUserLogin, 0);

    MY_LOG_INFO("sgit - ReqUserLogin, return:%d, UserID:%s", ret, reqUserLogin.UserID);
}

void SgitDataHandler::OnFrontDisconnected(char *pErrMsg)
{
    QuoteUpdateState(qtm_name.c_str(), QtmState::DISCONNECT);
    logoned_ = false;
    MY_LOG_ERROR("sgit - OnFrontDisconnected, pErrMsg: %s", pErrMsg);
}

void SgitDataHandler::OnRspUserLogin(CSgitFtdcRspUserLoginField *pRspUserLogin, CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo)
    {
        MY_LOG_INFO("sgit - OnRspUserLogin, ErrorCode:%d, ErrorMsg:%s, RequestID:%d, IsLast:%d",
            pRspInfo->ErrorID, pRspInfo->ErrorMsg, nRequestID, bIsLast);
    }
    else
    {
        MY_LOG_INFO("sgit - OnRspUserLogin, pRspInfo is null");
    }

    if (!pRspInfo || pRspInfo->ErrorID == 0)
    {
        logoned_ = true;

        CSgitSubQuotField sub_param;
        memset(&sub_param, 0, sizeof(sub_param));
        strcpy(sub_param.ContractID, "all");
        int ret = api_->SubQuot(&sub_param);
        MY_LOG_INFO("sgit - SubQuot return:%d", ret);

        api_->Ready();
        QuoteUpdateState(qtm_name.c_str(), QtmState::API_READY);
    }
}

void SgitDataHandler::OnRspUserLogout(CSgitFtdcUserLogoutField *pUserLogout, CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    MY_LOG_INFO("sgit - OnRspUserLogout");
}

// 针对用户请求的出错通知
void SgitDataHandler::OnRspError(CSgitFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo)
    {
        MY_LOG_INFO("OnRspError, ErrorID:%d, ErrorMsg:%s", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
    else
    {
        MY_LOG_INFO("OnRspError, pRspInfo is null");
    }
}

void SgitDataHandler::OnRtnDepthMarketData(CSgitFtdcDepthMarketDataField * p)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);
        struct SGIT_Depth_Market_Data_Field md = Convert(*p);

        if (quote_data_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->InstrumentID) != subscribe_contracts_.end()))
        {
            quote_data_handler_(&md);
        }

        // 存起来
        p_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &md);
    }
    catch (...)
    {
        MY_LOG_FATAL("sgit - Unknown exception in OnRtnDepthMarketData.");
    }
}

void SgitDataHandler::SetQuoteDataHandler(boost::function<void(const SGIT_Depth_Market_Data_Field *)> quote_data_handler)
{
    quote_data_handler_ = quote_data_handler;
}

struct SGIT_Depth_Market_Data_Field SgitDataHandler::Convert(const CSgitFtdcDepthMarketDataField &sgit_data)
{
    SGIT_Depth_Market_Data_Field md;
    memset(&md, 0, sizeof(SGIT_Depth_Market_Data_Field));
    memcpy(&md, &sgit_data, sizeof(SGIT_Depth_Market_Data_Field));
    return md;
}
