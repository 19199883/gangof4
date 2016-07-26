#include "query_lts.h"
#include <dirent.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <boost/thread.hpp>

#include "quote_cmn_utility.h"

// initilize static variables
MYSecurityCollection MYLTSQueryInterface::securities_exchcode;
MYMDStaticInfo MYLTSQueryInterface::md_staticinfo;

MYLTSQueryInterface::MYLTSQueryInterface(const ConfigData& cfg)
{
    const LogonConfig &logon_info = cfg.Logon_config();

    front_addr_ = new char[logon_info.query_server_addr.size() + 1];
    memcpy(front_addr_, logon_info.query_server_addr.data(), logon_info.query_server_addr.size() + 1);

    broker_id_ = new char[logon_info.query_brokerid.size() + 1];
    memcpy(broker_id_, logon_info.query_brokerid.data(), logon_info.query_brokerid.size() + 1);

    user_ = new char[logon_info.query_account.size() + 1];
    memcpy(user_, logon_info.query_account.data(), logon_info.query_account.size() + 1);

    password_ = new char[logon_info.query_password.size() + 1];
    memcpy(password_, logon_info.query_password.data(), logon_info.query_password.size() + 1);

    user_productinfo_ = new char[logon_info.user_productinfo.size() + 1];
    memcpy(user_productinfo_, logon_info.user_productinfo.data(), logon_info.user_productinfo.size() + 1);
    auth_code_ = new char[logon_info.auth_code.size() + 1];
    memcpy(auth_code_, logon_info.auth_code.data(), logon_info.auth_code.size() + 1);

    sub_codes_ = cfg.Subscribe_datas();

    req_id_ = 1;
    login_flag_ = false;
    securities_get_finished_ = false;
    memset(rand_code_, 0, sizeof(rand_code_));

    //Create Directory
    char cur_path[256];
    char full_path[256];
    getcwd(cur_path, sizeof(cur_path));
    sprintf(full_path, "%s/flow_query_%s/", cur_path, user_);

    // check whether dir exist
    if (opendir(full_path) == NULL)
    {
        mkdir(full_path, 0755);
    }

    p_query_api_ = CSecurityFtdcQueryApi::CreateFtdcQueryApi(full_path);
    if (p_query_api_ == NULL)
    {
        MY_LOG_ERROR("LTS-Query - CreateFtdcQueryApi failed, return NULL.");
        return;
    }
    p_query_api_->RegisterSpi(this);

    p_query_api_->RegisterFront(front_addr_);
    MY_LOG_INFO("LTS-Query - RegisterFront:%s.", front_addr_);
    p_query_api_->Init();
}

MYLTSQueryInterface::~MYLTSQueryInterface()
{
    if (p_query_api_)
    {
        p_query_api_->RegisterSpi(NULL);
        p_query_api_->Release();
        p_query_api_ = NULL;
    }
}

void MYLTSQueryInterface::OnFrontConnected()
{
    MY_LOG_INFO("LTS-Query - OnFrontConnected.");

    if (securities_get_finished_)
    {
        MY_LOG_INFO("LTS-Query - return directly when have got all contracts.");
        return;
    }

    CSecurityFtdcAuthRandCodeField pAuthRandCode;
    memset(&pAuthRandCode, 0, sizeof(pAuthRandCode));

    int ret = p_query_api_->ReqFetchAuthRandCode(&pAuthRandCode, 1);
    TNL_LOG_INFO("try to Query ReqFetchAuthRandCode, return:%d", ret);
}

void MYLTSQueryInterface::OnFrontDisconnected(int reason)
{
    login_flag_ = false;
    MY_LOG_INFO("LTS-Query - OnFrontDisconnected.");
}

void MYLTSQueryInterface::OnHeartBeatWarning(int time_lapse)
{
    MY_LOG_INFO("LTS-Query - OnHeartBeatWarning, time_lapse:%d.", time_lapse);
}

void MYLTSQueryInterface::OnRspError(CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
    int error_id = 0;
    if (rsp) error_id = rsp->ErrorID;
    MY_LOG_INFO("LTS-Query - OnRspError, ErrorID:%d.", error_id);
}

void MYLTSQueryInterface::QueryReqLogin(int wait_seconds)
{
    // wait when need
    if (wait_seconds > 0)
    {
        sleep(wait_seconds);
    }

    CSecurityFtdcReqUserLoginField login_data;
    memset(&login_data, 0, sizeof(CSecurityFtdcReqUserLoginField));
    strncpy(login_data.BrokerID, broker_id_, sizeof(login_data.BrokerID));
    strncpy(login_data.UserID, user_, sizeof(login_data.UserID));
    strncpy(login_data.Password, password_, sizeof(login_data.Password));
    strncpy(login_data.RandCode, rand_code_, sizeof(login_data.RandCode));
    strncpy(login_data.UserProductInfo, user_productinfo_, sizeof(login_data.UserProductInfo));
    strncpy(login_data.AuthCode, auth_code_, sizeof(login_data.AuthCode));

    int ret = p_query_api_->ReqUserLogin(&login_data, ++req_id_);
    MY_LOG_INFO("LTS-Query - try to login query server, return:%d.", ret);
}

void MYLTSQueryInterface::OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField* pf, CSecurityFtdcRspInfoField* rsp, int req_id, bool last)
{
    TNL_LOG_INFO("OnRspFetchAuthRandCode");

    if (rsp && rsp->ErrorID != 0)
    {
        MY_LOG_ERROR("LTS-Query - OnRspFetchAuthRandCode failed, ErrorID: %d; ErrorMsg: %s", rsp->ErrorID, rsp->ErrorMsg);
        return;
    }
    memcpy(rand_code_, pf->RandCode, sizeof(rand_code_));
    MY_LOG_INFO("RandCode: %s", pf->RandCode);
    QueryReqLogin(0);
}

void MYLTSQueryInterface::OnRspUserLogin(CSecurityFtdcRspUserLoginField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
{
    if (rsp == NULL || rsp->ErrorID == 0)
    {
        login_flag_ = true;
        MY_LOG_INFO("LTS-Query - OnRspUserLogin success.");
    }
    else
    {
        login_flag_ = false;
        MY_LOG_ERROR("LTS-Query - OnRspUserLogin, ErrorID: %d; ErrorMsg: %s", rsp->ErrorID, rsp->ErrorMsg);

        // login failed, retry
        //boost::thread t(&MYLTSQueryInterface::QueryReqLogin, this, 30);
    }

    if (login_flag_)
    {
        // query exchange
        boost::thread t(&MYLTSQueryInterface::QueryMDStaticInfo, this);
    }
}

void MYLTSQueryInterface::QueryMDStaticInfo()
{
    // wait 1 second, 1 query request / second
    sleep(1);

    CSecurityFtdcQryMarketDataStaticInfoField qry_param;
    memset(&qry_param, 0, sizeof(qry_param));

    int ret = p_query_api_->ReqQryMarketDataStaticInfo(&qry_param, ++req_id_);
    MY_LOG_INFO("LTS-Query - try to ReqQryMarketDataStaticInfo, return:%d.", ret);
}

void MYLTSQueryInterface::OnRspQryMarketDataStaticInfo(CSecurityFtdcMarketDataStaticInfoField *pf,
    CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
{
    // 成功了
    if (rsp == NULL || rsp->ErrorID == 0)
    {
        if (pf != NULL)
        {
            // save to map
            std::string ex_code(pf->ExchangeID);
            ex_code.append(".");
            ex_code.append(pf->InstrumentID);
            md_staticinfo.insert(std::make_pair(ex_code, *pf));
        }
        else
        {
            MY_LOG_ERROR("OnRspQryMarketDataStaticInfo, response info is null.");
        }
    }
    else
    {
        MY_LOG_ERROR("OnRspQryMarketDataStaticInfo, ErrorID: %d; ErrorMsg: %s", rsp->ErrorID, rsp->ErrorMsg);
    }

    if (last)
    {
        securities_get_finished_ = true;
        MY_LOG_INFO("OnRspQryMarketDataStaticInfo, task finished, try logout.");

        // output all securites
        std::map<std::string, CSecurityFtdcMarketDataStaticInfoField> tmp_all(md_staticinfo.begin(), md_staticinfo.end());
        for (auto &v : tmp_all)
        {
            CSecurityFtdcMarketDataStaticInfoField & d = v.second;
            MY_LOG_INFO("OnRspQryMarketDataStaticInfo, EX.Code: %s"
                "; PreClosePrice: %.4f"
                "; PreSettlementPrice: %.4f"
                "; UpperLimitPrice: %.4f"
                "; LowerLimitPrice: %.4f",
                v.first.c_str(),
                d.PreClosePrice,
                d.PreSettlementPrice,
                d.UpperLimitPrice,
                d.LowerLimitPrice
                );
        }
        boost::thread t(&MYLTSQueryInterface::QueryReqLogout, this);
    }
}

void MYLTSQueryInterface::QueryReqLogout()
{
    // wait 1 second, 1 query request / second
    sleep(1);

    CSecurityFtdcUserLogoutField qry_param;
    memset(&qry_param, 0, sizeof(qry_param));
    ///经纪公司代码
    strncpy(qry_param.BrokerID, broker_id_, sizeof(qry_param.BrokerID));
    ///用户代码
    strncpy(qry_param.UserID, user_, sizeof(qry_param.UserID));

    int ret = p_query_api_->ReqUserLogout(&qry_param, ++req_id_);
    MY_LOG_INFO("LTS-Query - try to ReqUserLogout, return: %d.", ret);
}

void MYLTSQueryInterface::OnRspUserLogout(CSecurityFtdcUserLogoutField *pf, CSecurityFtdcRspInfoField *rsp, int req_id, bool last)
{
    if (rsp == NULL || rsp->ErrorID == 0)
    {
        login_flag_ = false;
        MY_LOG_INFO("LTS-Query - OnRspUserLogout success.");
    }
    else
    {
        MY_LOG_ERROR("LTS-Query - OnRspUserLogout, ErrorID: %d; ErrorMsg: %s", rsp->ErrorID, rsp->ErrorMsg);
    }
}
