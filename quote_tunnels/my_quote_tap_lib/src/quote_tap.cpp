/*
 * 		quote_tap.cpp
 *
 * 		the implement of quote_tap.h
 *
 * 		Copyright(c) 2007-2015, by MY Capital Inc.
 */

#include "quote_tap.h"

using namespace my_cmn;

static std::mutex quote_mutex;
static std::mutex exchange_list_mutex;
static std::mutex commodity_list_mutex;
static std::mutex contract_list_mutex;

using namespace std;

MYTAPDataHandler::MYTAPDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    :
        tap_handler_(NULL),
        cfg_(cfg)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    /* 初始化 */
    logoned_ = false;
    in_login_flag_ = false;
    api_ = NULL;
    sID = new unsigned int;
    check_login_ = false;

    /* 从配置解析参数 */
    if (!ParseConfig())
    {
        return;
    }

    sprintf(qtm_name_, "TAP - tap_%s_%u", cfg_.Logon_config().account.c_str(), getpid());

    p_save_tap_ = new QuoteDataSave<TapAPIQuoteWhole_MY>(cfg_, qtm_name_, "tapl2quote", TAP_QUOTE_TYPE);

    /* 设置接口所需信息 */
    struct TapAPIApplicationInfo p_info_;

    /* 设置授权码 第一个为内盘授权码 第二个为外盘授权码 */
	
	// test debug
/*	
	strcpy(p_info_.AuthCode,
				"B112F916FE7D27BCE7B97EB620206457946CED32E26C1EAC946CED32E26C1EAC946CED32E26C1EAC946CED32E26C1EAC5211AF9FEE541DDE9D6F622F72E25D5EF7F47AA93A738EF5A51B81D8526AB6A9D19E65B41F59D6A946CED32E26C1EACCAF8D4C61E28E2B1ABD9B8F170E14F8847D3EA0BF4E191F5DCB1B791E63DC19D1576DEAF5EC563CA3E560313C0C3411B45076795F550EB050A62C4F74D5892D2D14892E812723FAC858DEBD8D4AF9410729FB849D5D8D6EA48A1B8DC67E03781A279CE9426070929D5DA085659772E24A6F5EA52CF92A4D403F9E46083F27B19A88AD99812DADA44100324759F9FD1964EBD4F2F0FB50B51CD31C0B02BB43");
*/	

	// first
	/*
	strcpy(p_info_.AuthCode,
     "67EA896065459BECDFDB924B29CB7DF1946CED32E26C1EAC946CED32E26C1EAC946CED32E26C1EAC946CED32E26C1EAC5211AF9FEE541DDE41BCBAB68D525B0D111A0884D847D57163FF7F329FA574E7946CED32E26C1EAC946CED32E26C1EAC733827B0CE853869ABD9B8F170E14F8847D3EA0BF4E191F5D97B3DFE4CCB1F01842DD2B3EA2F4B20CAD19B8347719B7E20EA1FA7A3D1BFEFF22290F4B5C43E6C520ED5A40EC1D50ACDF342F46A92CCF87AEE6D73542C42EC17818349C7DEDAB0E4DB16977714F873D505029E27B3D57EB92D5BEDA0A710197EB67F94BB1892B30F58A3F211D9C3B3839BE2D73FD08DD776B9188654853DDA57675EBB7D6FBBFC");
*/
	 // second

	strcpy(p_info_.AuthCode,
        "B112F916FE7D27BCE7B97EB620206457946CED32E26C1EAC946CED32E26C1EAC946CED32E26C1EAC946CED32E26C1EAC5211AF9FEE541DDE123D2F2F8E7F3E4B946CED32E26C1EAC5A51B81D8526AB6A67D1B6302B4DDA7D946CED32E26C1EACD33D6030790F8965ABD9B8F170E14F8847D3EA0BF4E191F50905910EA362CB063C704B1E62DE54B938D80BD82C58B3980985E67B9910AF76A06C27260450E7F792D349532A6533D9952A55F6D7C8C437456145239FEDE5078EA7CBC5AB74E107BA8DC0B7CE56681E22C185C880AC2723510A31A504180EE423496CBBE968917E1A292DAECE9F5F491626856EE3C81F0C3F2F4454DC7EB391DA8AF4EC06A48782");
	
		getcwd(p_info_.KeyOperationLogPath, 301);

    int iResult = 0;

    api_ = CreateTapQuoteAPI(&p_info_, iResult);

    if ( NULL == api_)
    {
        MY_LOG_ERROR("TAP - CreateTapQuoteAPI failed, the error code is %d", iResult);
        return;
    }
    else
    {
        api_->SetAPINotify(this);
        char *addr_tmp = new char[quote_addr_.size() + 1];
        char *addr_tmp2, *port_tmp;

        memcpy(addr_tmp, quote_addr_.c_str(), quote_addr_.size() + 1);
        MY_LOG_INFO("TAP - prepare to connect quote provider: %s", quote_addr_.c_str());
        addr_tmp2 = strtok(addr_tmp, ":");
        port_tmp = strtok(NULL, ":");

        /* 设置服务器地址 */
        if (0 == api_->SetHostAddress(addr_tmp2, atoi(port_tmp)))
        {
        }
        else
        {
            return;
        }
        delete[] addr_tmp;
        sleep(1);

        std::thread login(std::bind(&MYTAPDataHandler::req_login, this, 0));
        login.detach();
    }
}

void
MYTAPDataHandler::req_login(int wait_seconds)
{
    if (in_login_flag_)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(quote_mutex);

    if (in_login_flag_)
    {
        return;
    }

    in_login_flag_ = true;
    int ret = 0;

    try
    {
        if (wait_seconds > 0)
        {
            sleep(wait_seconds);
        }

        if (logoned_)
        {
            in_login_flag_ = false;
            return;
        }

        /* 登录服务器 */
        struct TapAPIQuoteLoginAuth login_ ;
        strncpy(login_.UserNo, cfg_.Logon_config().account.c_str(), sizeof(login_.UserNo) - 1);
        strncpy(login_.Password, cfg_.Logon_config().password.c_str(), sizeof(login_.Password) - 1);
        login_.ISModifyPassword = 'N';     /* 不修改密码 */
        login_.ISDDA = 'N';                             /* 不需要动态验证 */

 again:
        while (0 != (ret = api_->Login(&login_)))
        {
            MY_LOG_ERROR("TAP - login failed, the error code is %d", ret);
            sleep(10);
        }

        sleep(10);

        if (false == check_login_) {
            MY_LOG_ERROR("TAP - Login timeout, reconnecting.");
            goto again;
        }

        MY_LOG_INFO("TAP - call Login() successful.");
    }
    catch (...)
    {

    }

    in_login_flag_ = false;
}

MYTAPDataHandler::~MYTAPDataHandler(void)
{
    if (api_)
    {
        /* 退出行情登录 */
        MY_LOG_INFO("TAP - cancel subscribe...");
        api_->Disconnect();
    }

    if (sID)
    {
        delete sID;
    }

    if (p_save_tap_)
    {
        delete p_save_tap_;
    }
}

bool
MYTAPDataHandler::ParseConfig()
{
    if (cfg_.Logon_config().quote_provider_addrs.empty())
    {
        MY_LOG_ERROR("TAP - no configure quote provider address");
        return false;
    }
    /* 服务器地址 */
    quote_addr_ = cfg_.Logon_config().quote_provider_addrs.front();

    /* 用户名 密码 */
    user_ = cfg_.Logon_config().account;
    pswd_ = cfg_.Logon_config().password;

    /* 订阅集合 */
    subscribe_contracts_ = cfg_.Subscribe_datas();

    /* 检查 */
    if (quote_addr_.empty())
    {
        MY_LOG_ERROR("TAP - no configure quote provider address");
        return false;
    }

    return true;
}

void
MYTAPDataHandler::OnRspLogin(TAPIINT32 errorCode, const TapAPIQuotLoginRspInfo *info)
{
    MY_LOG_INFO("TAP - OnRspLogin");

    check_login_ = true;

    if (0 == errorCode)
    {
        logoned_ = true;
    }
    else
    {
        MY_LOG_WARN("TAP - login failed, ErrorCode = %d", errorCode);
    }
}

void
MYTAPDataHandler::OnAPIReady()
{
    MY_LOG_INFO("TAP - OnAPIReady");

    for (SubscribeContracts::iterator it = subscribe_contracts_.begin(); it != subscribe_contracts_.end(); it++)
    {
        int split_pos1 = it->find('.');
        int split_pos2 = it->substr(split_pos1 + 1).find('.');
        int split_pos3 = it->substr(split_pos1 + 1).substr(split_pos2 + 1).find('.');

        exchange_no_ = it->substr(0, split_pos1);
        commodity_type_ = (*it)[split_pos1 + 1];
        commodity_no_ = it->substr(split_pos1 + 1).substr(split_pos2 + 1).substr(0, split_pos3);
        contract_no_ = it->substr(split_pos1 + 1).substr(split_pos2 + 1).substr(split_pos3 + 1);

        /* 根据每个变量的值订阅相应的合约 */
		if ("ALL" == commodity_no_ ||
            "All" == commodity_no_ ||
            "all" == commodity_no_)
        {
            TapAPICommodity tmp;

            strcpy(tmp.ExchangeNo, exchange_no_.c_str());
            tmp.CommodityType = commodity_type_;
            strcpy(tmp.CommodityNo, "");

            std::thread thread_commodity_no(std::bind(&MYTAPDataHandler::qry_contract, this, tmp));
            thread_commodity_no.detach();
        }
        else if ("ALL" == contract_no_ ||
            "All" == contract_no_ ||
            "all" == contract_no_)
        {
            TapAPICommodity tmp;

            strcpy(tmp.ExchangeNo, exchange_no_.c_str());
            tmp.CommodityType = commodity_type_;
            strcpy(tmp.CommodityNo, commodity_no_.c_str());

            std::thread thread_contract_no(std::bind(&MYTAPDataHandler::qry_contract, this, tmp));
            thread_contract_no.detach();
        }
        else
        {
            TapAPIContract tmp;

            strcpy(tmp.Commodity.ExchangeNo, exchange_no_.c_str());
            tmp.Commodity.CommodityType = commodity_type_;
            strcpy(tmp.Commodity.CommodityNo, commodity_no_.c_str());
            strcpy(tmp.ContractNo1, contract_no_.c_str());
            tmp.CallOrPutFlag1 = TAPI_CALLPUT_FLAG_NONE;
            tmp.CallOrPutFlag2 = TAPI_CALLPUT_FLAG_NONE;

            std::thread thread_subscribe_quote(std::bind(&MYTAPDataHandler::subscribe_quote, this, tmp));
            thread_subscribe_quote.detach();
        }
    }
}

void
MYTAPDataHandler::qry_contract(TapAPICommodity &qryReq)
{
    int ret = 0;

    while ((ret = api_->QryContract(sID, &qryReq)) != 0)
    {
        MY_LOG_ERROR("TAP - QryContract failed, ExchangeNo is %s, CommodityNo is %s, errorCode is %d.", qryReq.ExchangeNo, qryReq.CommodityNo, ret);
        sleep(5);
    }

    MY_LOG_INFO("TAP - QryContract successful, ExchangeNo is %s, CommodityNo is %s.", qryReq.ExchangeNo, qryReq.CommodityNo);
}

void
MYTAPDataHandler::subscribe_quote(TapAPIContract &contract)
{
    int ret = 0;

    while ((ret = api_->SubscribeQuote(sID, &contract)) != 0)
    {
        MY_LOG_ERROR("TAP - SubscribeQuote failed, ExchangeNo is %s, CommodityNo is %s, ContractNo is %s, errorCode is %d.",
            contract.Commodity.ExchangeNo, contract.Commodity.CommodityNo, contract.ContractNo1, ret);
        sleep(5);
    }

    MY_LOG_INFO("TAP - SubscribeQuote successful, ExchangeNo is %s, CommodityNo is %s, ContractNo is %s.", contract.Commodity.ExchangeNo,
        contract.Commodity.CommodityNo, contract.ContractNo1);
}

void
MYTAPDataHandler::OnDisconnect(TAPIINT32 reasonCode)
{
    MY_LOG_INFO("TAP - OnDisconnect, reasonCode is %d, reconnecting.", reasonCode);
    check_login_ = false;
    int ret;

    struct TapAPIQuoteLoginAuth login_ ;
    strncpy(login_.UserNo, cfg_.Logon_config().account.c_str(), sizeof(login_.UserNo) - 1);
    strncpy(login_.Password, cfg_.Logon_config().password.c_str(), sizeof(login_.Password) - 1);
    login_.ISModifyPassword = 'N';     /* 不修改密码 */
    login_.ISDDA = 'N';                             /* 不需要动态验证 */

reconnect:
    while (0 != (ret = api_->Login(&login_)))
    {
        MY_LOG_ERROR("TAP - login failed, the error code is %d", ret);
        sleep(10);
    }

    sleep(10);

    if (false == check_login_) {
        MY_LOG_ERROR("TAP - Login timeout, reconnecting.");
        goto reconnect;
    }
    MY_LOG_INFO("TAP - Reconnect successful.");
}

void
MYTAPDataHandler::OnRspQryExchange(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIExchangeInfo *info)
{
    MY_LOG_INFO("TAP - OnRspQryExchange");

    if (0 == errorCode)
    {

        std::lock_guard<std::mutex> lock(exchange_list_mutex);

        if ( NULL != info)
        {
            exchange_list_.push_front(info->ExchangeNo);
        }

        if (APIYNFLAG_YES == isLast)
        {
            std::thread qryEx(std::bind(&MYTAPDataHandler::qry_contract_by_exchange, this));
            qryEx.detach();
        }
        MY_LOG_INFO("TAP - QryExchange successful, ExchangeNo is %s.", info->ExchangeNo);
    }
    else
    {
        MY_LOG_WARN("TAP - QryExchange failed, the error code is %d.", errorCode);
    }
}

/* 通过交易所列表查找合约 */
void
MYTAPDataHandler::qry_contract_by_exchange()
{
    MY_LOG_INFO("TAP - qry_contract_by_exchange");

    std::lock_guard<std::mutex> lock(exchange_list_mutex);

    int ret = 0;

    for (auto it = exchange_list_.begin(); it != exchange_list_.end(); it++)
    {
        TapAPICommodity tmp;
        strcpy(tmp.ExchangeNo, it->c_str());
        tmp.CommodityType = TAPI_COMMODITY_TYPE_NONE;
        strcpy(tmp.CommodityNo, "");

        while (0 != (ret = api_->QryContract(sID, &tmp)))
        {
            MY_LOG_ERROR("TAP - QryContract failed, ExchangeNo is %s,  errorCode  is %d.", tmp.ExchangeNo, ret);
            sleep(5);
        }

        MY_LOG_INFO("TAP - QryContract successful, ExchangeNo is %s.", tmp.ExchangeNo);
    }

    exchange_list_.clear();

    MY_LOG_INFO("TAP - qry_contract_by_exchange successful.");
}

void
MYTAPDataHandler::OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteCommodityInfo *info)
{
    MY_LOG_INFO("TAP - OnRspQryCommodity");

    if (0 == errorCode)
    {
        std::lock_guard<std::mutex> lock(commodity_list_mutex);

        if ( NULL != info)
        {
            commodity_list_.push_front(info->Commodity);
        }

        if (APIYNFLAG_YES == isLast)
        {
            std::thread qryCom(std::bind(&MYTAPDataHandler::qry_contract_by_commodity, this));
            qryCom.detach();
        }

        MY_LOG_INFO("TAP - QryContract successful, CommodityNo is %s.", info->Commodity.CommodityNo);
    }
    else
    {
        MY_LOG_WARN("TAP - QryCommodity failed, errorCode is %d.", errorCode);
    }
}

/* 通过品种列表查找合约 */
void
MYTAPDataHandler::qry_contract_by_commodity()
{
    MY_LOG_INFO("TAP - qry_contract_by_commodity");

    std::lock_guard<std::mutex> lock(commodity_list_mutex);

    int ret = 0;

    for (auto it = commodity_list_.begin(); it != commodity_list_.end(); it++)
    {
        while (0 != (ret = api_->QryContract(sID, &(*it))))
        {
            MY_LOG_ERROR("TAP - QryContract failed, ExchangeNo is %s, CommodityNo is %s, errorCode is %d.", it->ExchangeNo, it->CommodityNo, ret);
            sleep(5);
        }
        MY_LOG_INFO("TAP - QryContract successful, ExchangeNo is %s, CommodityNo is %s.", it->ExchangeNo, it->CommodityNo);
    }
    commodity_list_.clear();

    MY_LOG_INFO("TAP - qry_contract_by_commodity successful.");
}

void
MYTAPDataHandler::OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteContractInfo *info)
{
    MY_LOG_INFO("TAP - OnRspQryContract");

    if (0 == errorCode)
    {
        std::lock_guard<std::mutex> lock(contract_list_mutex);

        if ( NULL != info)
        {
            /* 把得到的合约发送至合约列表 */
            TapAPIContract *tmp = new TapAPIContract;
            memcpy(tmp, &(info->Contract), sizeof(TapAPIContract));

            contract_list_.push_front(tmp);
        }
        else
        {
            MY_LOG_WARN("TAP - OnRspQryContract the info is NULL.");
        }

        /* 最后一个时执行订阅线程 */
        if (APIYNFLAG_YES == isLast)
        {
            std::thread qrtCon(std::bind(&MYTAPDataHandler::subscribe_by_contract, this));
            qrtCon.detach();
        }
		MY_LOG_INFO("TAP - OnRspQryContract successful, ExchangeNo is %s, CommodityNo is %s, ContractNo is %s.", info->Contract.Commodity.ExchangeNo,
			info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1);

    }
    else
    {
        MY_LOG_ERROR("TAP - QryContract failed, the error code is %d.", errorCode);
    }
}

void
MYTAPDataHandler::OnRtnContract(const TapAPIQuoteContractInfo *info)
{
    MY_LOG_INFO("TAP - OnRtnContract");

    if ( NULL != info)
    {
        TapAPIContract tmp = info->Contract;
        std::thread rtn_contract(std::bind(&MYTAPDataHandler::subscribe_rtn_contract, this, tmp));
        rtn_contract.detach();

        MY_LOG_INFO("TAP - OnRtnContract successful, ExchangeNo is %s, CommodityNo is %s, ContractNo is %s.", info->Contract.Commodity.ExchangeNo,
            info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1);
    }
    else
    {
        MY_LOG_ERROR("TAP - OnRtnContract failed, the pointer is null.");
    }
}

void
MYTAPDataHandler::subscribe_rtn_contract(const TapAPIContract &info)
{
    MY_LOG_INFO("TAP - subscribe_rtn_contract");

    int ret = 0;

    while (0 != (ret = api_->SubscribeQuote(sID, &info)))
    {
        MY_LOG_ERROR("TAP - subscribe_rtn_contract failed, the error code is %d", ret);
        sleep(5);
    }

    MY_LOG_INFO("TAP - subscribe_rtn_contract successful");
}

/* 根据合约列表订阅行情 */
void
MYTAPDataHandler::subscribe_by_contract()
{
    MY_LOG_INFO("TAP - subscribe_by_contract");

    std::lock_guard<std::mutex> lock(contract_list_mutex);

    int ret = 0;

    for (auto it = contract_list_.begin(); it != contract_list_.end(); it++)
    {
        while (0 != (ret = api_->SubscribeQuote(sID, *it)))
        {
            MY_LOG_ERROR("TAP - subscribe_by_contract failed, ExchangeNo is %s, CommodityNo is %s, ContractNo is %s, errorCode is %d.",
                (*it)->Commodity.ExchangeNo, (*it)->Commodity.CommodityNo, (*it)->ContractNo1, ret);
            sleep(5);
        }
        /*                  查看订阅合约的内容，用于调试
         printf("ExchangeNo %s, CommodityNo %s, ContractNo1%s, ContractNo2 %s, CallOrPutFlag1 %c, CallOrPutFlag2 %c.\n", (*it)->Commodity.ExchangeNo,
         (*it)->Commodity.CommodityNo, (*it)->ContractNo1, (*it)->ContractNo2, (*it)->CallOrPutFlag1, (*it)->CallOrPutFlag2);
         */
        delete *it;
    }
    contract_list_.clear();
    MY_LOG_INFO("TAP - subscribe_by_contract successful.");
}

void
MYTAPDataHandler::OnRspSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteWhole *info)
{
    MY_LOG_INFO("TAP - OnRspSubscribeQuote");

    if (errorCode == 0 && NULL != info)
    {
        try
        {
            timeval t;
            gettimeofday(&t, NULL);
            TapAPIQuoteWhole_MY data_my = Convert(*info);

            /* 发出去 */
            if (tap_handler_)
            {
                tap_handler_(&data_my);
            }

            /* 存起来 */
            p_save_tap_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);

        }
        catch (...)
        {
            MY_LOG_FATAL("TAP - Unknown exception in ConsumeThread.");
        }

        MY_LOG_INFO("TAP - OnRspSubscribeQuote Successful, ExchangNo is %s, CommodityNo is %s, ContractNo is %s.",
            info->Contract.Commodity.ExchangeNo, info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1);
    }
    else
    {
        MY_LOG_WARN("TAP - SubscribeQuote failed, the error code is %d.", errorCode);
    }
}

void
MYTAPDataHandler::OnRtnQuote(const TapAPIQuoteWhole *info)
{
    //MY_LOG_INFO("TAP - OnRtnQuote");
    if ( NULL != info)
    {
        try
        {
            timeval t;
            gettimeofday(&t, NULL);
            TapAPIQuoteWhole_MY data_my = Convert(*info);

            /* 发出去 */
            if (tap_handler_)
            {
                tap_handler_(&data_my);
            }

            /* 存起来 */
            p_save_tap_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);

        }
        catch (...)
        {
            MY_LOG_FATAL("TAP - Unknown exception in ConsumeThread.");
        }

      //  MY_LOG_INFO("TAP - OnRtnQuote Successful, ExchangNo is %s, CommodityNo is %s, ContractNo is %s.",
       //     info->Contract.Commodity.ExchangeNo, info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1);
    }
    else
    {
        MY_LOG_ERROR("TAP - RtnQuote failed, the pointer is null.");
    }

}

TapAPIQuoteWhole_MY
MYTAPDataHandler::Convert(const TapAPIQuoteWhole &other)
{
    TapAPIQuoteWhole_MY data;

    strcpy(data.ExchangeNo, other.Contract.Commodity.ExchangeNo);
    data.CommodityType = other.Contract.Commodity.CommodityType;
    strcpy(data.CommodityNo, other.Contract.Commodity.CommodityNo);
    strcpy(data.ContractNo1, other.Contract.ContractNo1);
    strcpy(data.StrikePrice1, other.Contract.StrikePrice1);
    data.CallOrPutFlag1 = other.Contract.CallOrPutFlag1;
    strcpy(data.ContractNo2, other.Contract.ContractNo2);
    strcpy(data.StrikePrice2, other.Contract.StrikePrice2);
    data.CallOrPutFlag2 = other.Contract.CallOrPutFlag2;

    strcpy(data.CurrencyNo, other.CurrencyNo);
    data.TradingState = other.TradingState;
    strcpy(data.DateTimeStamp, other.DateTimeStamp);

    data.QPreClosingPrice = other.QPreClosingPrice;
    data.QPreSettlePrice = other.QPreSettlePrice;
    data.QPrePositionQty = other.QPrePositionQty;
    data.QOpeningPrice = other.QOpeningPrice;
    data.QLastPrice = other.QLastPrice;
    data.QHighPrice = other.QHighPrice;
    data.QLowPrice = other.QLowPrice;
    data.QHisHighPrice = other.QHisHighPrice;
    data.QHisLowPrice = other.QHisLowPrice;
    data.QLimitUpPrice = other.QLimitUpPrice;
    data.QLimitDownPrice = other.QLimitDownPrice;
    data.QTotalQty = other.QTotalQty;
    data.QTotalTurnover = other.QTotalTurnover;
    data.QPositionQty = other.QPositionQty;
    data.QAveragePrice = other.QAveragePrice;
    data.QClosingPrice = other.QClosingPrice;
    data.QSettlePrice = other.QSettlePrice;
    data.QLastQty = other.QLastQty;
    for(int i = 0; i < 20; i++) {
        data.QBidPrice[i] = other.QBidPrice[i];
        data.QBidQty[i] = other.QBidQty[i];
        data.QAskPrice[i] = other.QAskPrice[i];
        data.QAskQty[i] = other.QAskQty[i];
    }
    data.QImpliedBidPrice = other.QImpliedBidPrice;
    data.QImpliedBidQty = other.QImpliedBidQty;
    data.QImpliedAskPrice = other.QImpliedAskPrice;
    data.QImpliedAskQty = other.QImpliedAskQty;
    data.QPreDelta = other.QPreDelta;
    data.QCurrDelta = other.QCurrDelta;
    data.QInsideQty = other.QInsideQty;
    data.QOutsideQty = other.QOutsideQty;
    data.QTurnoverRate = other.QTurnoverRate;
    data.Q5DAvgQty = other.Q5DAvgQty;
    data.QPERatio = other.QPERatio;
    data.QTotalValue = other.QTotalValue;
    data.QNegotiableValue = other.QNegotiableValue;
    data.QPositionTrend = other.QPositionTrend;
    data.QChangeSpeed = other.QChangeSpeed;
    data.QChangeRate = other.QChangeRate;
    data.QChangeValue = other.QChangeValue;
    data.QSwing = other.QSwing;
    data.QTotalBidQty = other.QTotalBidQty;
    data.QTotalAskQty = other.QTotalAskQty;

    return data;
}

/* 下列的回调函数暂时还没实现（没需求） */

void
MYTAPDataHandler::OnRspUnSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIContract *info)
{
    MY_LOG_INFO("TAP - OnRspUnSubscribeQuote");
    if (0 == errorCode)
    {

        if ( NULL != info)
        {

        }

        if (APIYNFLAG_YES == isLast)
        {

        }

        MY_LOG_INFO("TAP - OnRspUnSubscribeQuote Successful.");
    }
    else
    {
        MY_LOG_WARN("OnRspUnSubscribeQuote failed, the error code is %d.", errorCode);
    }
}

void
MYTAPDataHandler::OnRspChangePassword(TAPIUINT32 sessionID, TAPIINT32 errorCode)
{
    MY_LOG_INFO("TAP - OnRspChangePassword");
    if (0 == errorCode)
    {
        MY_LOG_INFO("TAP - OnRspChangePassword Successful");
    }
    else
    {
        MY_LOG_WARN("TAP - OnRspChangePassword failed, the error code is %d.", errorCode);
    }
}

