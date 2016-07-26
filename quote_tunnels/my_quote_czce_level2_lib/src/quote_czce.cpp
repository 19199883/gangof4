#include "quote_czce.h"
#include "qtm.h"

using namespace my_cmn;

using namespace std;

static int s_quote_idx = 0;

MYCZCEDataHandler::MYCZCEDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg),
        p_save_l2_quote_(NULL),
        p_save_cmb_quote_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    // 初始化
    char qtm_name_buf[64];
    sprintf(qtm_name_buf, "zce_level2_%lu_%d", getpid(), s_quote_idx++);
    qtm_name = qtm_name_buf;
    QuoteUpdateState(qtm_name.c_str(), QtmState::INIT);

    logoned_ = false;
    in_login_flag = false;
    api_ = NULL;
    publish_data_flag_ = false;
    recv_data_flag_ = false;

    p_save_l2_quote_ = new QuoteDataSave<ZCEL2QuotSnapshotField_MY>(cfg_, qtm_name.c_str(), "czce_level2", CZCE_LEVEL2_QUOTE_TYPE);
    p_save_cmb_quote_ = new QuoteDataSave<ZCEQuotCMBQuotField_MY>(cfg_, qtm_name.c_str(), "czce_cmb", CZCE_CMB_QUOTE_TYPE);

    //创建Level-2行情API实例
    api_ = CreateZCEL2QuotClient(this);
    if (api_ == NULL)
    {
        MY_LOG_ERROR("CreateZCEL2QuotClient failed");
        return;
    }
    else
    {
        SetServerAddr();

        // maybe connect failed, should retry in another thread
        // ReqLogin(0);
        boost::thread(boost::bind(&MYCZCEDataHandler::ReqLogin, this, 0));
    }
}

void MYCZCEDataHandler::SetServerAddr()
{
    int max_service_addr = 2;
    int set_count = 0;
    int ret = 0;
    for (const std::string &quote_addr : cfg_.Logon_config().quote_provider_addrs)
    {
        ++set_count;
        if (set_count > max_service_addr)
        {
            MY_LOG_WARN("count of service address can't exceed %d", max_service_addr);
            MY_LOG_WARN("address %s can't be effective", quote_addr.c_str());
            break;
        }

        IPAndPortNum ip_and_port = ParseIPAndPortNum(quote_addr);

        // set server address
        ret = api_->SetService(ip_and_port.first.c_str(), ip_and_port.second);
        MY_LOG_INFO("SetService, ip: %s, port: %d, return code: %d", ip_and_port.first.c_str(), ip_and_port.second, ret);
    }
}

void MYCZCEDataHandler::ReqLogin(int wait_seconds)
{
    if (in_login_flag) return;

    boost::mutex::scoped_lock lock(quote_mutex);
    if (in_login_flag) return;

    in_login_flag = true;

    int ret = 0;
    try
    {
        if (wait_seconds > 0)
        {
            sleep(wait_seconds);
        }

        if (logoned_)
        {
            in_login_flag = false;
            return;
        }

        // connect
        while ((ret = api_->ConnectService()) != 0)
        {
            QuoteUpdateState(qtm_name.c_str(), QtmState::CONNECT_FAIL);
            MY_LOG_ERROR("connect service failed, return code: %d.", ret);
            sleep(10);
        }

        MY_LOG_INFO("connect service successful.");
        QuoteUpdateState(qtm_name.c_str(), QtmState::CONNECT_SUCCESS);

        struct ZCEL2QuotReqLoginField login_param;
        strcpy(login_param.UserNo, cfg_.Logon_config().account.c_str());
        strcpy(login_param.Password, cfg_.Logon_config().password.c_str());

        // login after connect
        while ((ret = api_->Login(&login_param)) != 0)
        {
            QuoteUpdateState(qtm_name.c_str(), QtmState::LOG_ON_FAIL);
            MY_LOG_ERROR("call Login() failed, return code: %d.", ret);
            sleep(10);
        }

        MY_LOG_INFO("call Login() successful.");
    }
    catch (...)
    {
    }

    in_login_flag = false;
}

MYCZCEDataHandler::~MYCZCEDataHandler(void)
{
    if (api_)
    {
        //退出行情登录
        QuoteUpdateState(qtm_name.c_str(), QtmState::LOG_OUT);
        MY_LOG_INFO("CZCE - prepare logout...");
        api_->Logout();

        DelZCEL2QuotClient(api_);
    }

    if (p_save_l2_quote_) delete p_save_l2_quote_;
    if (p_save_cmb_quote_) delete p_save_cmb_quote_;
}

void MYCZCEDataHandler::OnRspLogin(const ZCEL2QuotRspLoginField& loginMsg)
{
    MY_LOG_INFO("OnRspLogin");

    if (loginMsg.ErrorCode == 0)
    {
        logoned_ = true;
        QuoteUpdateState(qtm_name.c_str(), QtmState::API_READY);
        MY_LOG_INFO("login successful.");
    }
    else
    {
        QuoteUpdateState(qtm_name.c_str(), QtmState::LOG_ON_FAIL);
        MY_LOG_WARN("login failed, ErrorCode = %d", loginMsg.ErrorCode);
    }
}

void MYCZCEDataHandler::OnRecvQuote(const ZCEL2QuotSnapshotField& snapshot)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);
        ZCEL2QuotSnapshotField_MY data_my = Convert(snapshot);

        // 发出去
        if (publish_data_flag_
            && l2_quote_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(data_my.ContractID) != subscribe_contracts_.end()))
        {
            l2_quote_handler_(&data_my);
        }

        // 存起来
        p_save_l2_quote_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);

        recv_data_flag_ = true;
    }
    catch (...)
    {
        MY_LOG_FATAL("CZCE - Unknown exception in OnRecvQuote.");
    }
}

void MYCZCEDataHandler::OnRecvCmbQuote(const ZCEL2QuotCMBQuotField& snapshot)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);
        ZCEQuotCMBQuotField_MY data_my = Convert(snapshot);

        // 发出去
        if (publish_data_flag_
            && cmb_quote_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(data_my.ContractID) != subscribe_contracts_.end()))
        {
            cmb_quote_handler_(&data_my);
        }

        // 存起来
        p_save_cmb_quote_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
    }
    catch (...)
    {
        MY_LOG_FATAL("CZCE - Unknown exception in OnRecvCmbQuote.");
    }
}

void MYCZCEDataHandler::OnConnectClose()
{
    QuoteUpdateState(qtm_name.c_str(), QtmState::DISCONNECT);
    logoned_ = false;
    MY_LOG_ERROR("CZCE - OnConnectClose, try reconnect");

    // 登录失败，不断重试
    boost::thread(boost::bind(&MYCZCEDataHandler::ReqLogin, this, 6));
}

ZCEL2QuotSnapshotField_MY MYCZCEDataHandler::Convert(const ZCEL2QuotSnapshotField &o)
{
    ZCEL2QuotSnapshotField_MY data;
    memcpy(data.ContractID, o.ContractID, sizeof(data.ContractID)); /*合约编码*/
    data.ContractIDType = o.ContractIDType; /*合约类型 0->目前应该为0， 扩充：0:期货,1:期权,2:组合*/
    data.PreSettle = o.PreSettle; /*前结算价格*/
    data.PreClose = o.PreClose; /*前收盘价格*/
    data.PreOpenInterest = o.PreOpenInterest; /*昨日空盘量*/
    data.OpenPrice = o.OpenPrice; /*开盘价*/
    data.HighPrice = o.HighPrice; /*最高价*/
    data.LowPrice = o.LowPrice; /*最低价*/
    data.LastPrice = o.LastPrice; /*最新价*/
    memcpy(data.BidPrice, o.BidPrice, sizeof(data.BidPrice)); /*买入价格 下标从0开始*/
    memcpy(data.AskPrice, o.AskPrice, sizeof(data.AskPrice)); /*卖出价 下标从0开始*/
    memcpy(data.BidLot, o.BidLot, sizeof(data.BidLot)); /*买入数量 下标从0开始*/
    memcpy(data.AskLot, o.AskLot, sizeof(data.AskLot)); /*卖出数量 下标从0开始*/
    data.TotalVolume = o.TotalVolume; /*总成交量*/
    data.OpenInterest = o.OpenInterest; /*持仓量*/
    data.ClosePrice = o.ClosePrice; /*收盘价*/
    data.SettlePrice = o.SettlePrice; /*结算价*/
    data.AveragePrice = o.AveragePrice; /*均价*/
    data.LifeHigh = o.LifeHigh; /*历史最高成交价格*/
    data.LifeLow = o.LifeLow; /*历史最低成交价格*/
    data.HighLimit = o.HighLimit; /*涨停板*/
    data.LowLimit = o.LowLimit; /*跌停板*/
    data.TotalBidLot = o.TotalBidLot; /*委买总量*/
    data.TotalAskLot = o.TotalAskLot; /*委卖总量*/
    memcpy(data.TimeStamp, o.TimeStamp, sizeof(data.TimeStamp));      //时间：如2014-02-03 13:23:45

    return data;
}

ZCEQuotCMBQuotField_MY MYCZCEDataHandler::Convert(const ZCEL2QuotCMBQuotField &o)
{
    ZCEQuotCMBQuotField_MY data;
    data.CmbType = o.CmbType; /*组合类型 1 -- SPD, 2 -- IPS*/
    memcpy(data.ContractID, o.ContractID, sizeof(data.ContractID)); /*合约编码*/
    data.BidPrice = o.BidPrice; /*买入价格*/
    data.AskPrice = o.AskPrice; /*卖出价*/
    data.BidLot = o.BidLot; /*买入数量*/
    data.AskLot = o.AskLot; /*卖出数量*/
    data.VolBidLot = o.VolBidLot; /*总买入数量*/
    data.VolAskLot = o.VolAskLot; /*总卖出数量*/
    memcpy(data.TimeStamp, o.TimeStamp, sizeof(data.TimeStamp));      //时间：如2008-02-03 13:23:45

    return data;
}

