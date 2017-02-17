#include "xele_trade_interface.h"
#include <iomanip>

#include "my_protocol_packager.h"
#include "check_schedule.h"

#include "qtm_with_code.h"

using namespace std;

void MyXeleTradeSpi::ReportErrorState(int api_error_no, const std::string &error_msg)
{
    if (api_error_no == 0)
    {
        return;
    }
    if (!cfg_.IsKnownErrorNo(api_error_no))
    {
        char err_msg[127];
        sprintf(err_msg, "api error no: %d, error msg: %s", api_error_no, error_msg.c_str());
        update_state(tunnel_info_.qtm_name.c_str(), TYPE_TCA, QtmState::UNDEFINED_API_ERROR, err_msg);
        TNL_LOG_INFO("update_state: name: %s, State: %d, Description: %s.", tunnel_info_.qtm_name.c_str(), QtmState::UNDEFINED_API_ERROR, err_msg);
    }
}

MyXeleTradeSpi::MyXeleTradeSpi(const TunnelConfigData &cfg)
    : cfg_(cfg)
{
    connected_ = false;
    logoned_ = false;
    in_init_state_ = true;

    // whether it is need to support cancel all order when init
    have_handled_unterminated_orders_ = true;
    if (cfg_.Initial_policy_param().cancel_orders_at_start)
    {
        have_handled_unterminated_orders_ = false;
    }
    finish_query_canceltimes_ = false;
    if (cfg_.Compliance_check_param().init_cancel_times_at_start == 0)
    {
        finish_query_canceltimes_ = true;
    }
    qry_order_t_ = NULL;

    // TODO xele not support query order, shutdown two features which depend on it
    have_handled_unterminated_orders_ = true;
    finish_query_canceltimes_ = true;

    xele_trade_context_.InitOrderRef(cfg_.App_cfg().orderref_prefix_id);

    // 从配置解析参数
    if (!ParseConfig())
    {
        return;
    }

    //qtm init
    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "xele_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

    // create
    api_ = NULL;
    TNL_LOG_INFO("CXeleTraderApi::GetVersion() return:%s", CXeleTraderApi::GetVersion());
    api_ = CXeleTraderApi::CreateTraderApi();
    if (api_ == NULL)
    {
        TNL_LOG_ERROR("CXeleTraderApi::CreateTraderApi() failed.");
        return;
    }
    api_->RegisterSpi(this);

    // set front address list
    for (const std::string &v : cfg_.Logon_config().front_addrs)
    {
        char *addr_tmp = new char[v.size() + 1];
        memcpy(addr_tmp, v.c_str(), v.size() + 1);
        api_->RegisterFront(addr_tmp);
        TNL_LOG_WARN("RegisterFront, addr: %s", addr_tmp);
        delete[] addr_tmp;
    }

    // subscribe (xele only support TERT_QUICK option)
    api_->SubscribePrivateTopic(XELE_TERT_QUICK);
    api_->SubscribePublicTopic(XELE_TERT_QUICK);

    // init
    api_->Init();
}

void MyXeleTradeSpi::ReqLogin()
{
    // 登录
    CXeleFtdcReqUserLoginField login_data;
    memset(&login_data, 0, sizeof(CXeleFtdcReqUserLoginField));

    strncpy(login_data.UserID, user_.c_str(), sizeof(login_data.UserID));
    strncpy(login_data.ParticipantID, cfg_.Logon_config().ParticipantID.c_str(), sizeof(login_data.ParticipantID));
    strncpy(login_data.Password, pswd_.c_str(), sizeof(login_data.Password));
    strncpy(login_data.UserProductInfo, cfg_.Logon_config().UserProductInfo.c_str(), sizeof(login_data.UserProductInfo));
    strncpy(login_data.InterfaceProductInfo, cfg_.Logon_config().InterfaceProductInfo.c_str(),
        sizeof(login_data.InterfaceProductInfo));
    strncpy(login_data.ProtocolInfo, cfg_.Logon_config().ProtocolInfo.c_str(), sizeof(login_data.ProtocolInfo));
    login_data.DataCenterID = cfg_.Logon_config().DataCenterID;

    // 成功登录后，断开不再重新登录
    if (in_init_state_)
    {
        int ret = api_->ReqUserLogin(&login_data, 0);
        TNL_LOG_INFO("ReqUserLogin - return: %d \n%s", ret, XeleDatatypeFormater::ToString(&login_data).c_str());
    }
}

MyXeleTradeSpi::~MyXeleTradeSpi(void)
{
    if (api_ != NULL)
    {
        api_->Release();
    }
}

bool MyXeleTradeSpi::ParseConfig()
{
    // 用户密码
    tunnel_info_.account = cfg_.Logon_config().investorid;
    user_ = cfg_.Logon_config().clientid;
    pswd_ = cfg_.Logon_config().password;
    return true;
}

void MyXeleTradeSpi::SetCallbackHandler(std::function<void(const T_OrderRespond *)> handler)
{
    OrderRespond_call_back_handler_ = handler;
}

void MyXeleTradeSpi::SetCallbackHandler(std::function<void(const T_CancelRespond *)> handler)
{
    CancelRespond_call_back_handler_ = handler;
}

void MyXeleTradeSpi::SetCallbackHandler(std::function<void(const T_OrderReturn *)> handler)
{
    OrderReturn_call_back_handler_ = handler;
}

void MyXeleTradeSpi::SetCallbackHandler(std::function<void(const T_TradeReturn *)> handler)
{
    TradeReturn_call_back_handler_ = handler;
}

void MyXeleTradeSpi::OnFrontConnected()
{
    TNL_LOG_WARN("OnFrontConnected.");
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::CONNECT_SUCCESS);
    connected_ = true;
    ReqLogin();
}

void MyXeleTradeSpi::OnFrontDisconnected(int nReason)
{
    connected_ = false;
    logoned_ = false;
    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);

    TNL_LOG_WARN("OnFrontDisconnected, nReason: %d", nReason);
}

void MyXeleTradeSpi::OnPackageStart(int nTopicID, int nSequenceNo)
{
    // 不能用于识别初始化时的恢复数据，每单个Rtn消息都有开始结束
    //TNL_LOG_INFO("OnPackageStart, nTopicID:%d, nSequenceNo:%d", nTopicID, nSequenceNo);
}

void MyXeleTradeSpi::OnPackageEnd(int nTopicID, int nSequenceNo)
{
    // 不能用于识别初始化时的恢复数据，每单个Rtn消息都有开始结束
    //TNL_LOG_INFO("OnPackageEnd, nTopicID:%d, nSequenceNo:%d", nTopicID, nSequenceNo);
}

void MyXeleTradeSpi::OnRspError(CXeleFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    try
    {
        if (pRspInfo)
        {
            TNL_LOG_WARN("OnRspError - req_id:%d, is_last:%d \n%s",
                nRequestID, bIsLast, XeleDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnRspError ");
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRspError - unknown exception.");
    }
}

void MyXeleTradeSpi::OnRspUserLogin(CXeleFtdcRspUserLoginField *pRspUserLogin, CXeleFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    TNL_LOG_INFO("OnRspUserLogin: req_id:%d, is_last:%d \n%s %s",
        nRequestID, bIsLast,
        XeleDatatypeFormater::ToString(pRspUserLogin).c_str(),
        XeleDatatypeFormater::ToString(pRspInfo).c_str());

    // logon success
    if (pRspInfo == NULL || pRspInfo->ErrorID == 0)
    {
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_SUCCESS);
        // get max local id, for avoiding duplicate id
        max_order_ref_ = atoll(pRspUserLogin->MaxOrderLocalID);

        // 设置到交易环境变量中
        xele_trade_context_.SetOrderRef(max_order_ref_);

        logoned_ = true;
        in_init_state_ = false;

        // start thread for cancel unterminated orders
        if (!HaveFinishQueryOrders())
        {
            qry_order_t_ = new std::thread(&MyXeleTradeSpi::QueryAndHandleOrders, this);
        }
        else
        {
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
        }
    }
    else
    {
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        int standard_error_no = cfg_.GetStandardErrorNo(pRspInfo->ErrorID);

        TNL_LOG_WARN("OnRspUserLogin, external errorid: %d; Internal errorid: %d",
            pRspInfo->ErrorID, standard_error_no);

        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_FAIL);
    }
}

void MyXeleTradeSpi::OnRspUserLogout(CXeleFtdcRspUserLogoutField *pRspUserLogout, CXeleFtdcRspInfoField *pRspInfo, int nRequestID,
bool bIsLast)
{
    try
    {
        logoned_ = false;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_OUT);
        TNL_LOG_WARN("OnRspUserLogout - req_id:%d, is_last:%d \n%s %s",
            nRequestID, bIsLast,
            XeleDatatypeFormater::ToString(pRspUserLogout).c_str(),
            XeleDatatypeFormater::ToString(pRspInfo).c_str());
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRspUserLogout - unknown exception.");
    }
}

void MyXeleTradeSpi::HandleFillupRsp(long entrust_no, SerialNoType serial_no)
{
    T_OrderRespond order_respond;
    memset(&order_respond, 0, sizeof(order_respond));
    order_respond.entrust_no = entrust_no;
    order_respond.entrust_status = MY_TNL_OS_REPORDED;
    order_respond.serial_no = serial_no;
    order_respond.error_no = 0;
    OrderRespond_call_back_handler_(&order_respond);
    LogUtil::OnOrderRespond(&order_respond, tunnel_info_);
    TNL_LOG_WARN("fill up respond for trade_return: %ld", serial_no);
}

void MyXeleTradeSpi::HandleFillupRtn(long entrust_no, char order_status, const XeleOriginalReqInfo* p)
{
    T_OrderReturn order_return;
    memset(&order_return, 0, sizeof(order_return));
    order_return.entrust_no = entrust_no;
    order_return.entrust_status = order_status;
    order_return.serial_no = p->serial_no;

    strncpy(order_return.stock_code, p->stock_code.c_str(), sizeof(order_return.stock_code));
    order_return.direction = p->buy_sell_flag;
    order_return.open_close = p->open_close_flag;
    order_return.speculator = p->hedge_type;
    order_return.volume = p->order_volum;
    order_return.limit_price = 0;
    OrderReturn_call_back_handler_(&order_return);
    LogUtil::OnOrderReturn(&order_return, tunnel_info_);
    TNL_LOG_WARN("fill up return for trade_return: %ld", p->serial_no);
}

///报单查询应答
void MyXeleTradeSpi::OnRspQryOrder(CXeleFtdcOrderField *pOrder, CXeleFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!HaveFinishQueryOrders())
    {
        if (!finish_query_canceltimes_)
        {
            std::unique_lock<std::mutex> lock(stats_canceltimes_sync_);
            if (pOrder && pOrder->OrderStatus == XELE_FTDC_OST_Canceled)
            {
                std::unordered_map<std::string, int>::iterator it = cancel_times_of_contract.find(pOrder->InstrumentID);
                if (it == cancel_times_of_contract.end())
                {
                    cancel_times_of_contract.insert(std::make_pair(pOrder->InstrumentID, 1));
                }
                else
                {
                    ++it->second;
                }
            }

            if (bIsLast)
            {
                for (std::unordered_map<std::string, int>::iterator it = cancel_times_of_contract.begin();
                    it != cancel_times_of_contract.end(); ++it)
                {
                    ComplianceCheck_SetCancelTimes(tunnel_info_.account.c_str(), it->first.c_str(), it->second);
                }
                cancel_times_of_contract.clear();
                TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_CANCEL_TIMES_SUCCESS);
                finish_query_canceltimes_ = true;
            }
        }

        if (!have_handled_unterminated_orders_)
        {
            std::unique_lock<std::mutex> lock(cancel_sync_);
            if (pOrder && !IsOrderTerminate(*pOrder))
            {
                unterminated_orders_.push_back(*pOrder);
            }

            if (bIsLast && unterminated_orders_.empty())
            {
                have_handled_unterminated_orders_ = true;
            }

            if (bIsLast)
            {
                qry_order_finish_cond_.notify_one();
            }
        }

        if (TunnelIsReady())
        {
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
        }

        TNL_LOG_INFO("OnRspQryOrder when cancel unterminated orders or stats cancel times, order: 0X%X, last: %d", pOrder, bIsLast);
        return;
    }

    T_OrderDetailReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        ret.error_no = -1;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        TNL_LOG_DEBUG("OnRspQryOrder return error - %s", XeleDatatypeFormater::ToString(pRspInfo).c_str());
        return;
    }
    TNL_LOG_DEBUG("OnRspQryOrder - req_id:%d; is_last:%d; \n%s",
        nRequestID, bIsLast, XeleDatatypeFormater::ToString(pOrder).c_str());

    // empty Order detail
    if (!pOrder)
    {
        ret.error_no = 0;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        return;
    }

    OrderDetail od;
    strncpy(od.stock_code, pOrder->InstrumentID, sizeof(TXeleFtdcInstrumentIDType));
    od.entrust_no = atol(pOrder->OrderSysID);
    od.order_kind = XeleFieldConvert::GetMYPriceType(pOrder->OrderPriceType);
    od.direction = pOrder->Direction;
    od.open_close = pOrder->CombOffsetFlag[0];
    od.speculator = XeleFieldConvert::GetMYHedgeType(pOrder->CombHedgeFlag[0]);
    od.entrust_status = XeleFieldConvert::GetMYEntrustStatus(pOrder->OrderStatus);
    od.limit_price = pOrder->LimitPrice;
    od.volume = pOrder->VolumeTotalOriginal;
    od.volume_traded = pOrder->VolumeTraded;
    od.volume_remain = pOrder->VolumeTotal;
    od_buffer_.push_back(od);

    if (bIsLast)
    {
        ret.datas.swap(od_buffer_);
        ret.error_no = 0;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}

///成交单查询应答
void MyXeleTradeSpi::OnRspQryTrade(CXeleFtdcTradeField *pTrade, CXeleFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    T_TradeDetailReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        ret.error_no = -1;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        TNL_LOG_DEBUG("OnRspQryTrade return error - %s", XeleDatatypeFormater::ToString(pRspInfo).c_str());
        return;
    }

    TNL_LOG_DEBUG("OnRspQryTrade - req_id:%d; is_last:%d; \n%s",
        nRequestID, bIsLast, XeleDatatypeFormater::ToString(pTrade).c_str());

    // empty trade detail
    if (!pTrade)
    {
        ret.error_no = 0;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        return;
    }

    TradeDetail td;
    strncpy(td.stock_code, pTrade->InstrumentID, sizeof(td.stock_code));
    td.entrust_no = atol(pTrade->OrderSysID);
    td.direction = pTrade->Direction;
    td.open_close = pTrade->OffsetFlag;
    td.speculator = XeleFieldConvert::GetMYHedgeType(pTrade->HedgeFlag);
    td.trade_price = pTrade->Price;
    td.trade_volume = pTrade->Volume;
    strncpy(td.trade_time, pTrade->TradeTime, sizeof(td.trade_time));
    td_buffer_.push_back(td);

    if (bIsLast)
    {
        ret.datas.swap(td_buffer_);
        ret.error_no = 0;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

///客户持仓查询应答
void MyXeleTradeSpi::OnRspQryClientPosition(CXeleFtdcRspClientPositionField *pf,
    CXeleFtdcRspInfoField *pRspInfo,
    int nRequestID,
    bool bIsLast)
{
    T_PositionReturn ret;

    // respond error
    if (pRspInfo && pRspInfo->ErrorID != 0)
    {
        ret.error_no = -1;
        QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    TNL_LOG_DEBUG("OnRspQryClientPosition - req_id:%d; is_last:%d; \n%s",
        nRequestID, bIsLast, XeleDatatypeFormater::ToString(pf).c_str());

    // empty positioon
    if (!pf)
    {
        ret.error_no = 0;
        QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
        return;
    }

    PositionDetail pos;

    // set volume multiple parameter
    int volume_multiple = 1;
    if (pf->InstrumentID[0] == 'I' && pf->InstrumentID[1] == 'F')
    {
        volume_multiple = 300;
    }
    else if (pf->InstrumentID[0] == 'I' && pf->InstrumentID[1] == 'H')
    {
        volume_multiple = 300;
    }
    else if (pf->InstrumentID[0] == 'I' && pf->InstrumentID[1] == 'C')
    {
        volume_multiple = 200;
    }
    else if (pf->InstrumentID[0] == 'T')
    {
        volume_multiple = 10000;
    }

    strncpy(pos.stock_code, pf->InstrumentID, sizeof(pos.stock_code));
    pos.direction = MY_TNL_D_BUY;
    if (pf->PosiDirection == XELE_FTDC_PD_Short)
    {
        pos.direction = MY_TNL_D_SELL;
    }

    pos.position = pf->Position;
    pos.position_avg_price = 0;
    if (pos.position > 0)
    {
        pos.position_avg_price = pf->PositionCost / pos.position / volume_multiple;
    }

    pos.yestoday_position = pf->YdPosition;
    pos.yd_position_avg_price = 0;
    if (pos.yestoday_position > 0)
    {
        pos.yd_position_avg_price = pf->YdPositionCost / pos.yestoday_position / volume_multiple;
    }

    if (pos.position > 0 || pos.yestoday_position > 0)
    {
        pos_buffer_.push_back(pos);
    }

    if (bIsLast)
    {
        ret.datas.swap(pos_buffer_);
        ret.error_no = 0;
        QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}

CXeleFtdcOrderActionField MyXeleTradeSpi::CreatCancelParam(const CXeleFtdcOrderField& order_field)
{
    CXeleFtdcOrderActionField req_fld;
    OrderRefDataType order_ref = xele_trade_context_.GetNewOrderRef();
    memset(&req_fld, 0, sizeof(req_fld));

    // 原报单交易所标识
    memcpy(req_fld.OrderSysID, order_field.OrderSysID, sizeof(req_fld.OrderSysID));
    memcpy(req_fld.OrderLocalID, order_field.OrderLocalID, sizeof(req_fld.OrderLocalID));
    req_fld.ActionFlag = XELE_FTDC_AF_Delete;

    strncpy(req_fld.ParticipantID, cfg_.Logon_config().ParticipantID.c_str(), sizeof(req_fld.ParticipantID));
    strncpy(req_fld.ClientID, cfg_.Logon_config().investorid.c_str(), sizeof(req_fld.ClientID));
    strncpy(req_fld.UserID, cfg_.Logon_config().clientid.c_str(), sizeof(req_fld.UserID));

    req_fld.LimitPrice = 0;
    req_fld.VolumeChange = 0;

    snprintf(req_fld.ActionLocalID, sizeof(req_fld.ActionLocalID), "%012lld", order_ref);

    return req_fld;
}

bool MyXeleTradeSpi::IsOrderTerminate(const CXeleFtdcOrderField& order_field)
{
    return order_field.OrderStatus == XELE_FTDC_OST_AllTraded
        || order_field.OrderStatus == XELE_FTDC_OST_Canceled;
}

void MyXeleTradeSpi::QueryAndHandleOrders()
{
    // query order detail parameter
    CXeleFtdcQryOrderField qry_param;
    memset(&qry_param, 0, sizeof(CXeleFtdcQryOrderField));

    const LogonConfig & log_cfg = cfg_.Logon_config();
    strncpy(qry_param.ClientID, log_cfg.clientid.c_str(), sizeof(qry_param.ClientID));

    //超时后没有完成查询，重试。为防止委托单太多，10s都回报不了，每次超时加5s
    int wait_seconds = 10;

    std::vector<CXeleFtdcOrderField> unterminated_orders_t;
    while (!HaveFinishQueryOrders())
    {
        {
            std::unique_lock<std::mutex> lock(stats_canceltimes_sync_);
            cancel_times_of_contract.clear();
        }
        //主动查询所有报单
        while (true)
        {
            // TODO the interface not implemented by acxele
            //int qry_result = api_->ReqQryOrder(&qry_param, 0);
            int qry_result = 0;
            have_handled_unterminated_orders_ = true;
            finish_query_canceltimes_ = true;

            if (qry_result != 0)
            {
                // retry if failed, wait some seconds
                sleep(2);
                continue;
            }
            TNL_LOG_INFO("ReqQryOrder for cancel unterminated orders or stats cancel times, return %d", qry_result);

            break;
        }

        //处理未终结报单（撤未终结报单）
        if (!have_handled_unterminated_orders_)
        {
            {
                std::unique_lock<std::mutex> lock(cancel_sync_);
                qry_order_finish_cond_.wait_for(lock, std::chrono::seconds(10));
                unterminated_orders_t.swap(unterminated_orders_);
            }
            //遍历 unterminated_orders 间隔20ms（流控要求），发送撤单请求
            for (const CXeleFtdcOrderField &order_field : unterminated_orders_t)
            {
                CXeleFtdcOrderActionField action_field = CreatCancelParam(order_field);
                int ret = api_->ReqOrderAction(&action_field, 0);
                if (ret != 0)
                {
                    TNL_LOG_WARN("in CancelUnterminatedOrders, ReqOrderAction return %d", ret);
                }
                usleep(20 * 1000);
            }
            unterminated_orders_t.clear();

            //全部发送完毕后，等待 1s ， 判断 handle_flag , 如没有完成，则retry
            sleep(1);
        }
        else if (!finish_query_canceltimes_)
        {
            // wait order query result return back
            sleep(wait_seconds);
            wait_seconds += 5;
        }
    }
}

//void MyXeleTradeSpi::OnRspQryPartAccount(CXeleFtdcRspPartAccountField* pf, CXeleFtdcRspInfoField* pRspInfo,
//    int nRequestID, bool bIsLast)
//{
//    TNL_LOG_INFO("OnRspQryPartAccount - req_id:%d, is_last:%d \n%s %s",
//        nRequestID, bIsLast,
//        XeleDatatypeFormater::ToString(pf).c_str(),
//        XeleDatatypeFormater::ToString(pRspInfo).c_str());
//}
//
//void MyXeleTradeSpi::OnRspQryClient(CXeleFtdcRspClientField* pf, CXeleFtdcRspInfoField* pRspInfo, int nRequestID,
//bool bIsLast)
//{
//    TNL_LOG_DEBUG("OnRspQryClient - req_id:%d; is_last:%d; "
//        "ClientID:%s; "
//        "ClientName:%s; "
//        "ClientType:0X%02X; "
//        "IdentifiedCardNo:%s; "
//        "IdentifiedCardType:%s; "
//        "IsActive:%d; "
//        "ParticipantID:%s; "
//        "TradingRole:0X%02X; "
//        "UseLess:%s; "
//        ,
//        nRequestID, bIsLast,
//        pf->ClientID,
//        pf->ClientName,
//        pf->ClientType,
//        pf->IdentifiedCardNo,
//        pf->IdentifiedCardType,
//        pf->IsActive,
//        pf->ParticipantID,
//        pf->TradingRole,
//        pf->UseLess);
//}
//
//void MyXeleTradeSpi::OnRspQryPartPosition(CXeleFtdcRspPartPositionField* pf, CXeleFtdcRspInfoField* pRspInfo,
//    int nRequestID, bool bIsLast)
//{
//    T_PositionReturn ret;
//
//    // respond error
//    if (pRspInfo && pRspInfo->ErrorID != 0)
//    {
//        ret.error_no = -1;
//        QryPosReturnHandler_(&ret);
//        LogUtil::OnPositionRtn(&ret, investorid_);
//        return;
//    }
//
//    TNL_LOG_DEBUG("OnRspQryPartPosition - req_id:%d; is_last:%d; \n%s",
//        nRequestID, bIsLast, XeleDatatypeFormater::ToString(pf).c_str());
//
//    // empty positioon
//    if (!pf)
//    {
//        ret.error_no = 0;
//        QryPosReturnHandler_(&ret);
//        LogUtil::OnPositionRtn(&ret, investorid_);
//        return;
//    }
//
//    PositionDetail pos;
//    strncpy(pos.stock_code, pf->InstrumentID, sizeof(pos.stock_code));
//    pos.direction = FUNC_CONST_DEF::CONST_ENTRUST_BUY;
//    if (pf->PosiDirection == XELE_FTDC_PD_Short)
//    {
//        pos.direction = FUNC_CONST_DEF::CONST_ENTRUST_SELL;
//    }
//
//    pos.position = pf->Position;
//    pos.position_avg_price = 0;
//    if (pos.position > 0)
//    {
//        pos.position_avg_price = 0; //pf->PositionCost / pos.position / 300;
//    }
//
//    pos.yestoday_position = pf->YdPosition;
//    pos.yd_position_avg_price = 0;
//    if (pos.yestoday_position > 0)
//    {
//        pos.yd_position_avg_price = 0; //pf->YdPositionCost / pos.yestoday_position / 300;
//    }
//
//    if (pos.position > 0 || pos.yestoday_position > 0)
//    {
//        pos_buffer_.push_back(pos);
//    }
//
//    if (bIsLast)
//    {
//        ret.datas.swap(pos_buffer_);
//        ret.error_no = 0;
//        QryPosReturnHandler_(&ret);
//        LogUtil::OnPositionRtn(&ret, investorid_);
//    }
//}

void MyXeleTradeSpi::OnRspQryInstrument(CXeleFtdcRspInstrumentField* pf, CXeleFtdcRspInfoField* pRspInfo,
    int nRequestID, bool bIsLast)
{
    TNL_LOG_INFO("OnRspQryInstrument - req_id:%d, is_last:%d \n%s %s",
        nRequestID, bIsLast,
        XeleDatatypeFormater::ToString(pf).c_str(),
        XeleDatatypeFormater::ToString(pRspInfo).c_str());
}

void MyXeleTradeSpi::OnRspQryInstrumentStatus(CXeleFtdcInstrumentStatusField* pf, CXeleFtdcRspInfoField* pRspInfo,
    int nRequestID, bool bIsLast)
{
    TNL_LOG_INFO("OnRspQryInstrumentStatus - req_id:%d, is_last:%d \n%s %s",
        nRequestID, bIsLast,
        XeleDatatypeFormater::ToString(pf).c_str(),
        XeleDatatypeFormater::ToString(pRspInfo).c_str());
}

void MyXeleTradeSpi::OnRspQryTopic(CXeleFtdcDisseminationField* pDissemination, CXeleFtdcRspInfoField* pRspInfo, int nRequestID,
bool bIsLast)
{
    TNL_LOG_DEBUG("OnRspQryBulletin - req_id:%d; is_last:%d; ", nRequestID, bIsLast);
}

void MyXeleTradeSpi::OnRtnInstrumentStatus(CXeleFtdcInstrumentStatusField* pf)
{
    TNL_LOG_INFO("OnRtnInstrumentStatus - \n%s", XeleDatatypeFormater::ToString(pf).c_str());
}

void MyXeleTradeSpi::OnRtnInsInstrument(CXeleFtdcInstrumentField* pf)
{
    TNL_LOG_DEBUG("OnRtnInsInstrument - InstrumentID:%s; VolumeMultiple:%d",
        pf->InstrumentID,
        pf->VolumeMultiple);
}

void MyXeleTradeSpi::OnRtnDelInstrument(CXeleFtdcInstrumentField* pf)
{
    TNL_LOG_DEBUG("OnRtnDelInstrument - InstrumentID:%s; VolumeMultiple:%d",
        pf->InstrumentID,
        pf->VolumeMultiple);
}

void MyXeleTradeSpi::OnRspOrderInsert(CXeleFtdcInputOrderField* pInputOrder, CXeleFtdcRspInfoField* pRspInfo, int nRequestID,
bool bIsLast)
{
    int errorid = 0;
    int standard_error_no = 0;
    try
    {
        if (!pInputOrder)
        {
            TNL_LOG_WARN("OnRspOrderInsert, pInputOrder is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            errorid = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(errorid);
        }

        TNL_LOG_WARN("OnRspOrderInsert, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            errorid, standard_error_no, pInputOrder->OrderLocalID);

        OrderRefDataType OrderRef = atoll(pInputOrder->OrderLocalID);
        const XeleOriginalReqInfo *p = xele_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            T_OrderRespond pOrderRespond;

            if (0 != errorid)
            {
                if (xele_trade_context_.CheckAndSetHaveHandleInsertErr(p->serial_no))
                {
                    // 报单失败，报告合规检查
                    ComplianceCheck_OnOrderInsertFailed(
                        tunnel_info_.account.c_str(),
                        atoll(pInputOrder->OrderLocalID),
                        p->exchange_code,
						p->stock_code.c_str(),
                        pInputOrder->VolumeTotalOriginal,
                        p->hedge_type,
                        p->open_close_flag,
                        p->order_type);

                    XELEPacker::OrderRespond(standard_error_no, p->serial_no, "0", 'e', pOrderRespond);
                    StatusCheckResult check_result = xele_trade_context_.CheckAndUpdateStatus(
                        atoll(pInputOrder->OrderLocalID),
                        FemasRespondType::order_respond,
                        pOrderRespond.entrust_status,
                        0);

                    if (check_result == StatusCheckResult::none)
                    {
                        // 应答
                        OrderRespond_call_back_handler_(&pOrderRespond);
                        LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
                    }
                    else
                    {
                        TNL_LOG_WARN("abandon a respond: %ld", p->serial_no);
                    }
                }
                else
                {
                    TNL_LOG_WARN("rsp err insert have handled, order ref: %s", pInputOrder->OrderLocalID);
                }
            }
            else
            {
                xele_trade_context_.AddNewOrderSysID(atoll(pInputOrder->OrderSysID));
                XELEPacker::OrderRespond(standard_error_no, p->serial_no, pInputOrder->OrderSysID, '3', pOrderRespond);

                StatusCheckResult check_result = xele_trade_context_.CheckAndUpdateStatus(
                    atoll(pInputOrder->OrderLocalID),
                    FemasRespondType::order_respond,
                    pOrderRespond.entrust_status,
                    0);

                if (check_result == StatusCheckResult::none)
                {
                    // 应答
                    OrderRespond_call_back_handler_(&pOrderRespond);
                    LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
                }
                else
                {
                    TNL_LOG_WARN("OnRspOrderInsert - abandon a respond: %ld", p->serial_no);
                }
            }

            TNL_LOG_DEBUG("OnRspOrderInsert - req_id:%d, is_last:%d \n%s %s",
                nRequestID, bIsLast,
                XeleDatatypeFormater::ToString(pInputOrder).c_str(),
                XeleDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnRspOrderInsert - %s - can't get original order info.", pInputOrder->OrderLocalID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRspOrderInsert - unknown exception.");
    }
}

void MyXeleTradeSpi::OnRspOrderAction(CXeleFtdcOrderActionField* pOrderAction, CXeleFtdcRspInfoField* pRspInfo, int nRequestID,
bool bIsLast)
{
    int errorid = 0;
    int standard_error_no = 0;
    try
    {
        if (!HaveFinishQueryOrders())
        {
            TNL_LOG_WARN("OnRspOrderAction when cancel unterminated orders.");
            return;
        }
        if (!pOrderAction)
        {
            TNL_LOG_WARN("OnRspOrderAction, pInputOrderAction is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            errorid = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(errorid);
        }

        TNL_LOG_WARN("OnRspOrderAction, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            errorid, standard_error_no, pOrderAction->ActionLocalID);

        OrderRefDataType OrderRef = atoll(pOrderAction->ActionLocalID);
        const XeleOriginalReqInfo *p = xele_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            T_CancelRespond cancel_respond;

            if (0 != errorid)
            {
                if (xele_trade_context_.CheckAndSetHaveHandleActionErr(p->serial_no))
                {
                    XELEPacker::CancelRespond(standard_error_no, p->serial_no, "0", cancel_respond);
                    // 撤单拒绝，报告合规检查
                    ComplianceCheck_OnOrderCancelFailed(
                        tunnel_info_.account.c_str(),
                        p->stock_code.c_str(),
                        p->original_order_ref);

                    // 应答
                    CancelRespond_call_back_handler_(&cancel_respond);
                    LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
                }
                else
                {
                    TNL_LOG_WARN("OnRspOrderAction - rsp err action have handled, order sn: %ld", p->serial_no);
                }
            }
            else
            {
                XELEPacker::CancelRespond(standard_error_no, p->serial_no, "0", cancel_respond);

                // 应答
                CancelRespond_call_back_handler_(&cancel_respond);
                LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
            }

            TNL_LOG_DEBUG("OnRspOrderAction - req_id:%d, is_last:%d \n%s %s",
                nRequestID, bIsLast,
                XeleDatatypeFormater::ToString(pOrderAction).c_str(),
                XeleDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnRspOrderAction - %s - can't get original action info.", pOrderAction->ActionLocalID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRspOrderAction - unknown exception.");
    }
}

void MyXeleTradeSpi::OnErrRtnOrderInsert(CXeleFtdcInputOrderField* pInputOrder, CXeleFtdcRspInfoField* pRspInfo)
{
    int errorid = 0;
    int standard_error_no = 0;
    try
    {
        if (!pInputOrder)
        {
            TNL_LOG_WARN("OnErrRtnOrderInsert, pInputOrder is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            errorid = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(errorid);
        }

        TNL_LOG_WARN("OnErrRtnOrderInsert, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            errorid, standard_error_no, pInputOrder->OrderLocalID);

        OrderRefDataType OrderRef = atoll(pInputOrder->OrderLocalID);
        const XeleOriginalReqInfo *p = xele_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            T_OrderRespond pOrderRespond;

            if (xele_trade_context_.CheckAndSetHaveHandleInsertErr(p->serial_no))
            {
                // 报单失败，报告合规检查
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    atoll(pInputOrder->OrderLocalID),
                    p->exchange_code,
					p->stock_code.c_str(),
                    pInputOrder->VolumeTotalOriginal,
                    p->hedge_type,
                    p->open_close_flag,
                    p->order_type);

                XELEPacker::OrderRespond(standard_error_no, p->serial_no, "0", 'e', pOrderRespond);

                StatusCheckResult check_result = xele_trade_context_.CheckAndUpdateStatus(
                    atoll(pInputOrder->OrderLocalID),
                    FemasRespondType::order_respond,
                    pOrderRespond.entrust_status,
                    0);
                if (check_result == StatusCheckResult::none)
                {
                    // 应答
                    OrderRespond_call_back_handler_(&pOrderRespond);
                    LogUtil::OnOrderRespond(&pOrderRespond, tunnel_info_);
                }
                else
                {
                    TNL_LOG_WARN("OnErrRtnOrderInsert - abandon a err insert respond: %ld", p->serial_no);
                }
            }
            else
            {
                TNL_LOG_WARN("OnErrRtnOrderInsert - rsp err insert have handled, order ref: %s", pInputOrder->OrderLocalID);
            }

            TNL_LOG_DEBUG("OnErrRtnOrderInsert -  \n%s %s",
                XeleDatatypeFormater::ToString(pInputOrder).c_str(),
                XeleDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnErrRtnOrderInsert - %s - not mine(%s) in OnErrRtnOrderInsert", pInputOrder->OrderLocalID,
                pInputOrder->UserID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnErrRtnOrderInsert - unknown exception.");
    }
}

void MyXeleTradeSpi::OnErrRtnOrderAction(CXeleFtdcOrderActionField* pOrderAction, CXeleFtdcRspInfoField* pRspInfo)
{
    int errorid = 0;
    int standard_error_no = 0;
    try
    {
        if (!pOrderAction)
        {
            TNL_LOG_WARN("OnErrRtnOrderAction, pOrderAction is null.");
            return;
        }
        if (pRspInfo)
        {
            ReportErrorState(pRspInfo->ErrorID, pRspInfo->ErrorMsg);
            errorid = pRspInfo->ErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(errorid);
        }

        TNL_LOG_WARN("OnErrRtnOrderAction, api_err_code: %d; my_err_code: %d; OrderRef: %s",
            errorid, standard_error_no, pOrderAction->ActionLocalID);

        OrderRefDataType OrderRef = atoll(pOrderAction->ActionLocalID);
        const XeleOriginalReqInfo *p = xele_trade_context_.GetOrderInfoByOrderRef(OrderRef);
        if (p)
        {
            T_CancelRespond cancel_respond;

            if (xele_trade_context_.CheckAndSetHaveHandleActionErr(p->serial_no))
            {
                XELEPacker::CancelRespond(standard_error_no, p->serial_no, "0", cancel_respond);

                // 撤单拒绝，报告合规检查
                ComplianceCheck_OnOrderCancelFailed(
                    tunnel_info_.account.c_str(),
                    p->stock_code.c_str(),
                    p->original_order_ref);

                // 应答
                CancelRespond_call_back_handler_(&cancel_respond);
                LogUtil::OnCancelRespond(&cancel_respond, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("OnErrRtnOrderAction - rsp err action have handled, order sn: %ld", p->serial_no);
            }

            TNL_LOG_DEBUG("OnErrRtnOrderAction -  \n%s %s",
                XeleDatatypeFormater::ToString(pOrderAction).c_str(),
                XeleDatatypeFormater::ToString(pRspInfo).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnErrRtnOrderAction - %s - can't get original action info.", pOrderAction->ActionLocalID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnErrRtnOrderAction - unknown exception.");
    }
}

void MyXeleTradeSpi::OnRtnTrade(CXeleFtdcTradeField* pTrade)
{
    try
    {
        if (!HaveFinishQueryOrders())
        {
            TNL_LOG_WARN("OnRtnTrade when tunnel not ready.");
            return;
        }
        if (!pTrade)
        {
            TNL_LOG_WARN("OnRtnTrade, pTrade is null.");
            return;
        }

        EntrustNoType sysid = atoll(pTrade->OrderSysID);
        OrderRefDataType order_ref = atoll(pTrade->OrderLocalID);
        const XeleOriginalReqInfo *p = NULL;
        if (xele_trade_context_.IsSysIDOfThisTunnel(sysid))
        {
            p = xele_trade_context_.GetOrderInfoByOrderRef(order_ref);
        }
        if (p)
        {
            T_TradeReturn trade_return;
            XELEPacker::TradeReturn(p, pTrade, trade_return);

            // 成交
            StatusCheckResult check_result = xele_trade_context_.CheckAndUpdateStatus(
                order_ref,
                FemasRespondType::trade_return,
                MY_TNL_OS_PARTIALCOM,
                0);

            if (check_result == StatusCheckResult::fillup_rsp_rtn_c
                || check_result == StatusCheckResult::fillup_rsp_rtn_p)
            {
                HandleFillupRsp(trade_return.entrust_no, p->serial_no);
            }

            if (check_result == StatusCheckResult::fillup_rsp_rtn_c
                || check_result == StatusCheckResult::fillup_rtn_c)
            {
                HandleFillupRtn(trade_return.entrust_no, MY_TNL_OS_COMPLETED, p);
            }
            else if (check_result == StatusCheckResult::fillup_rsp_rtn_p
                || check_result == StatusCheckResult::fillup_rtn_p)
            {
                HandleFillupRtn(trade_return.entrust_no, MY_TNL_OS_PARTIALCOM, p);
            }

            TradeReturn_call_back_handler_(&trade_return);
            LogUtil::OnTradeReturn(&trade_return, tunnel_info_);

            TNL_LOG_DEBUG("OnRtnTrade - \n%s", XeleDatatypeFormater::ToString(pTrade).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnRtnTrade - %s - can't get original order info(sysid:%s)",
                pTrade->OrderLocalID, pTrade->OrderSysID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRtnTrade - unknown exception.");
    }
}

void MyXeleTradeSpi::OnRtnOrder(CXeleFtdcOrderField* pOrder)
{
    try
    {
        if (!HaveFinishQueryOrders())
        {
            TNL_LOG_WARN("OnRtnOrder when tunnel not ready.");
            return;
        }
        if (!pOrder)
        {
            TNL_LOG_WARN("OnRtnOrder, pOrder is null.");
            return;
        }

        OrderRefDataType order_ref = atoll(pOrder->OrderLocalID);

        const XeleOriginalReqInfo *p = xele_trade_context_.GetOrderInfoByOrderRef(order_ref);

        if (p)
        {
            xele_trade_context_.AddNewOrderSysID(atoll(pOrder->OrderSysID));
            SerialNoType serial_no = p->serial_no;

            // 全成，报告合规检查
            if (pOrder->OrderStatus == XELE_FTDC_OST_AllTraded)
            {
                ComplianceCheck_OnOrderFilled(
                    tunnel_info_.account.c_str(),
                    order_ref);
            }
            else if (pOrder->OrderStatus == XELE_FTDC_OST_Canceled)
            {
                ComplianceCheck_OnOrderCanceled(
                    tunnel_info_.account.c_str(),
                    p->stock_code.c_str(),
                    order_ref,
                    p->exchange_code,
                    pOrder->VolumeTotal,
                    p->hedge_type,
                    p->open_close_flag);
            }

            T_OrderReturn order_return;
            XELEPacker::OrderReturn(serial_no, pOrder, order_return);

            StatusCheckResult check_result = xele_trade_context_.CheckAndUpdateStatus(
                order_ref,
                FemasRespondType::order_return,
                order_return.entrust_status,
                0);

            if (check_result == StatusCheckResult::fillup_rsp)
            {
                HandleFillupRsp(order_return.entrust_no, serial_no);
            }
            if (check_result != StatusCheckResult::abandon)
            {
                OrderReturn_call_back_handler_(&order_return);
                LogUtil::OnOrderReturn(&order_return, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("OnRtnOrder - abandon a return: %ld", p->serial_no);
            }

            TNL_LOG_DEBUG("OnRtnOrder - \n%s", XeleDatatypeFormater::ToString(pOrder).c_str());
        }
        else
        {
            TNL_LOG_INFO("OnRtnOrder - %s - not mine(%s) in OnRtnOrder", pOrder->OrderLocalID, pOrder->UserID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("OnRtnOrder - unknown exception.");
    }
}
