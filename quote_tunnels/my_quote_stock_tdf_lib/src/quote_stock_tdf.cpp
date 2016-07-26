#include "quote_stock_tdf.h"

#include <sstream>

#include "TDFAPI.h"

boost::function<void(const TDF_MARKET_DATA_MY *)> QuoteInterface_TDF::md_data_handler_;
boost::function<void(const TDF_INDEX_DATA_MY *)> QuoteInterface_TDF::id_data_handler_;
boost::function<void(const T_OptionQuote*)> QuoteInterface_TDF::option_quote_handler_;
boost::function<void(const T_OrderQueue*)> QuoteInterface_TDF::order_queue_handler_;
boost::function<void(const T_PerEntrust*)> QuoteInterface_TDF::perentrust_handler_;
boost::function<void(const T_PerBargain*)> QuoteInterface_TDF::perbargain_handler_;
SubscribeContracts QuoteInterface_TDF::subscribe_contracts_;

static QuoteDataSave<TDF_MARKET_DATA_MY> *s_p_stock_md_save = NULL;
static QuoteDataSave<TDF_INDEX_DATA_MY> *s_p_stock_idx_save = NULL;
static QuoteDataSave<T_OrderQueue> *s_p_stock_oq_save = NULL;
static QuoteDataSave<T_PerEntrust> *s_p_stock_pe_save = NULL;
static QuoteDataSave<T_PerBargain> *s_p_stock_pb_save = NULL;

char QuoteInterface_TDF::qtm_name_[32] = "tdf";

static char* DeepCopy(const char*szIn)
{
    if (szIn)
    {
        int nLen = strlen(szIn);
        char* pRet = new char[nLen + 1];
        pRet[nLen] = 0;
        strncpy(pRet, szIn, nLen + 1);
        return pRet;
    }
    else
    {
        return NULL;
    }
}

static std::string FormatSubscribString(const SubsribeDatas &subs)
{
    std::stringstream ss;
    // 例如"600000.sh;ag.shf;000001.sz"，需要订阅的股票，以“;”分割，为空则订阅全市场
    for (SubsribeDatas::const_iterator cit = subs.begin(); cit != subs.end(); ++cit)
    {
        ss << *cit << ";";
    }

    return ss.str();
}

static TDF_MARKET_DATA_MY Convert(const TDF_MARKET_DATA &data)
{
    TDF_MARKET_DATA_MY d;

    memcpy(d.szWindCode, data.szWindCode, sizeof(d.szWindCode)); // 32 32
    memcpy(d.szCode, data.szCode, sizeof(d.szCode)); // 32 32
    d.nActionDay = data.nActionDay;
    d.nTradingDay = data.nTradingDay;
    d.nTime = data.nTime;
    d.nStatus = data.nStatus;
    d.nPreClose = data.nPreClose;
    d.nOpen = data.nOpen;
    d.nHigh = data.nHigh;
    d.nLow = data.nLow;
    d.nMatch = data.nMatch;
    memcpy(d.nAskPrice, data.nAskPrice, sizeof(d.nAskPrice)); // 10 10
    memcpy(d.nAskVol, data.nAskVol, sizeof(d.nAskVol)); // 10 10
    memcpy(d.nBidPrice, data.nBidPrice, sizeof(d.nBidPrice)); // 10 10
    memcpy(d.nBidVol, data.nBidVol, sizeof(d.nBidVol)); // 10 10
    d.nNumTrades = data.nNumTrades;
    d.iVolume = data.iVolume;
    d.iTurnover = data.iTurnover;
    d.nTotalBidVol = data.nTotalBidVol;
    d.nTotalAskVol = data.nTotalAskVol;
    d.nWeightedAvgBidPrice = data.nWeightedAvgBidPrice;
    d.nWeightedAvgAskPrice = data.nWeightedAvgAskPrice;
    d.nIOPV = data.nIOPV;
    d.nYieldToMaturity = data.nYieldToMaturity;
    d.nHighLimited = data.nHighLimited;
    d.nLowLimited = data.nLowLimited;
    memcpy(d.chPrefix, data.chPrefix, sizeof(d.chPrefix)); // 4 4
    d.nSyl1 = data.nSyl1;
    d.nSyl2 = data.nSyl2;
    d.nSD2 = data.nSD2;

    return d;
}

static TDF_INDEX_DATA_MY Convert(const TDF_INDEX_DATA &data)
{
    TDF_INDEX_DATA_MY d;

    memcpy(d.szWindCode, data.szWindCode, sizeof(d.szWindCode)); // 32 32
    memcpy(d.szCode, data.szCode, sizeof(d.szCode)); // 32 32
    d.nActionDay = data.nActionDay;
    d.nTradingDay = data.nTradingDay;
    d.nTime = data.nTime;
    d.nOpenIndex = data.nOpenIndex;
    d.nHighIndex = data.nHighIndex;
    d.nLowIndex = data.nLowIndex;
    d.nLastIndex = data.nLastIndex;
    d.iTotalVolume = data.iTotalVolume;
    d.iTurnover = data.iTurnover;
    d.nPreCloseIndex = data.nPreCloseIndex;

    return d;
}

static T_OrderQueue Convert(const TDF_ORDER_QUEUE &data)
{
    T_OrderQueue d;
    memset(&d, 0, sizeof(d));
    d.time = data.nTime;
    if (strstr(data.szWindCode, "SH"))
    {
        d.market = MY_QUOTE_CONST::EXCHCODE_SHEX[0];
    }
    else
    {
        d.market = MY_QUOTE_CONST::EXCHCODE_SZEX[0];
    }
    strcpy(d.scr_code, data.szCode);
    if (data.nSide == 'B')
    {
        strcpy(d.insr_txn_tp_code, "B");
    }
    else
    {
        strcpy(d.insr_txn_tp_code, "S");
    }
    d.ord_price = data.nPrice;
    d.ord_qty = data.nOrders;
    d.ord_nbr = data.nABItems;
    memcpy(d.ord_detail_vol, data.nABVolume, sizeof(int) * 200);

    return d;
}

static T_PerEntrust Convert(const TDF_ORDER &data)
{
    T_PerEntrust d;
    memset(&d, 0, sizeof(d));
    strcpy(d.scr_code, data.szCode);
    d.entrt_time = data.nTime;
    if (strstr(data.szWindCode, "SH"))
    {
        d.market = MY_QUOTE_CONST::EXCHCODE_SHEX[0];
    }
    else
    {
        d.market = MY_QUOTE_CONST::EXCHCODE_SZEX[0];
    }
    d.entrt_price = data.nPrice;
    d.entrt_id = data.nOrder;
    d.entrt_vol = data.nVolume;
    if (data.chFunctionCode == 'B')
    {
        strcpy(d.insr_txn_tp_code, "B");
    }
    else
    {
        strcpy(d.insr_txn_tp_code, "S");
    }
    d.entrt_tp[0] = data.chOrderKind;

    return d;
}

static T_PerBargain Convert(const TDF_TRANSACTION &data)
{
    T_PerBargain d;
    memset(&d, 0, sizeof(d));
    d.time = data.nTime;
    if (strstr(data.szWindCode, "SH"))
    {
        d.market = MY_QUOTE_CONST::EXCHCODE_SHEX[0];
    }
    else
    {
        d.market = MY_QUOTE_CONST::EXCHCODE_SZEX[0];
    }
    strcpy(d.scr_code, data.szCode);
    d.bgn_id = data.nIndex;
    d.bgn_price = data.nPrice;
    d.bgn_qty = data.nVolume;
    d.bgn_amt = data.nTurnover;
    d.bgn_flg[0] = data.chOrderKind;

    return d;
}

void QuoteInterface_TDF::InitOpenSetting(TDF_OPEN_SETTING *settings, const ConfigData &cfg)
{
    if (cfg.Logon_config().quote_provider_addrs.empty())
    {
        MY_LOG_ERROR("TDF - quote provider's address is empty");
        return;
    }
    // tcp://192.168.60.23:7120
    const std::string &addr_cfg = cfg.Logon_config().quote_provider_addrs.front();
    std::string ip_port = addr_cfg.substr(6);
    std::size_t split_pos = ip_port.find(":");
    if ((split_pos == std::string::npos) || (split_pos + 1 >= ip_port.length()))
    {
        MY_LOG_ERROR("TDF - parse quote provider's address error: %s", addr_cfg.c_str());
    }

    MY_LOG_DEBUG("ip_port: %s", ip_port.c_str());

    std::string addr_ip = ip_port.substr(0, split_pos);
    std::string addr_port = ip_port.substr(split_pos + 1);

    //MY_LOG_DEBUG("ip: %s, port: %s, user: %s, pw: %s", addr_ip.c_str(),  addr_port.c_str(), cfg.Logon_config().account.c_str(), cfg.Logon_config().password.c_str());
    //MY_LOG_DEBUG("subscriptions: %s", DeepCopy(FormatSubscribString(cfg.Subscribe_datas()).c_str()));

    memset(settings, 0, sizeof(TDF_OPEN_SETTING));

    strncpy(settings->szIp, addr_ip.c_str(), sizeof(settings->szIp) - 1);
    strncpy(settings->szPort, addr_port.c_str(), sizeof(settings->szPort) - 1);
    strncpy(settings->szUser, cfg.Logon_config().account.c_str(), sizeof(settings->szUser) - 1);
    strncpy(settings->szPwd, cfg.Logon_config().password.c_str(), sizeof(settings->szPwd) - 1);

    settings->nReconnectCount = 99999999;
    settings->nReconnectGap = 5;

    settings->pfnMsgHandler = &QuoteInterface_TDF::QuoteDataHandler;
    settings->pfnSysMsgNotify = &QuoteInterface_TDF::SystemMsgHandler;

    settings->nProtocol = 0;
    settings->szMarkets = DeepCopy("SZ;SH;"); // subscibe two market
    settings->szSubScriptions = DeepCopy(FormatSubscribString(cfg.Subscribe_datas()).c_str());

    settings->nDate = 0;
    settings->nTime = 0;

    settings->nTypeFlags = DATA_TYPE_ALL;//DATA_TYPE_INDEX;
}

static std::map<TDF_ERR, const char*> mapErrStr;
static void InitErrorMap()
{
    mapErrStr.insert(std::make_pair(TDF_ERR_UNKOWN, "TDF_ERR_UNKOWN"));
    mapErrStr.insert(std::make_pair(TDF_ERR_INITIALIZE_FAILURE, "TDF_ERR_INITIALIZE_FAILURE"));
    mapErrStr.insert(std::make_pair(TDF_ERR_NETWORK_ERROR, "TDF_ERR_NETWORK_ERROR"));
    mapErrStr.insert(std::make_pair(TDF_ERR_INVALID_PARAMS, "TDF_ERR_INVALID_PARAMS"));
    mapErrStr.insert(std::make_pair(TDF_ERR_VERIFY_FAILURE, "TDF_ERR_VERIFY_FAILURE"));
    mapErrStr.insert(std::make_pair(TDF_ERR_NO_AUTHORIZED_MARKET, "TDF_ERR_NO_AUTHORIZED_MARKET"));
    mapErrStr.insert(std::make_pair(TDF_ERR_NO_CODE_TABLE, "TDF_ERR_NO_CODE_TABLE"));
    mapErrStr.insert(std::make_pair(TDF_ERR_SUCCESS, "TDF_ERR_SUCCESS"));
}
static const char* GetErrStr(TDF_ERR nErr)
{
    if (mapErrStr.find(nErr) == mapErrStr.end())
    {
        return "TDF_ERR_UNKOWN";
    }
    else
    {
        return mapErrStr[nErr];
    }
}

//数据回调，用于通知用户收到了行情、逐笔成交，逐笔委托，委托队列等
//pMsgHead->pAppHead->ItemCount字段可以获知得到了多少条记录
//pMsgHead->pAppHead->pData指向第一条数据记录
void QuoteInterface_TDF::QuoteDataHandler(THANDLE hTdf, TDF_MSG* pMsgHead)
{
    if (!pMsgHead || !pMsgHead->pData)
    {
        MY_LOG_ERROR("QuoteDataHandler receive null data.");
        return;
    }

    unsigned int nItemCount = pMsgHead->pAppHead->nItemCount;

    if (!nItemCount)
    {
        MY_LOG_ERROR("QuoteDataHandler receive data, count is 0.");
        return;
    }
    //MY_LOG_DEBUG("QuoteDataHandler receive data, DataType: %d", pMsgHead->nDataType);
    switch (pMsgHead->nDataType)
    {
        case MSG_DATA_MARKET:
            {
            TDF_MARKET_DATA *p = (TDF_MARKET_DATA*) pMsgHead->pData;

            timeval t;
            gettimeofday(&t, NULL);
            for (unsigned int i = 0; i < nItemCount; ++i)
            {
                TDF_MARKET_DATA_MY tdf_my = Convert(*p);
                if (md_data_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->szCode) != subscribe_contracts_.end()))
                {
                    md_data_handler_(&tdf_my);
                }

                // 存起来
                s_p_stock_md_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &tdf_my);
                ++p;
            }
        }
            break;
        case MSG_DATA_INDEX:
            {
            TDF_INDEX_DATA *p = (TDF_INDEX_DATA*) pMsgHead->pData;

            timeval t;
            gettimeofday(&t, NULL);
            for (unsigned int i = 0; i < nItemCount; ++i)
            {
                TDF_INDEX_DATA_MY tdf_my = Convert(*p);
                if (id_data_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->szCode) != subscribe_contracts_.end()))
                {
                    id_data_handler_(&tdf_my);
                }

                // 存起来
                s_p_stock_idx_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &tdf_my);
                ++p;
            }
        }
            break;
        case MSG_DATA_ORDERQUEUE:
        {
            TDF_ORDER_QUEUE *p = (TDF_ORDER_QUEUE*)pMsgHead->pData;

            timeval t;
            gettimeofday(&t, NULL);
            for (unsigned int i = 0; i < nItemCount; ++i)
            {
                T_OrderQueue tdf_my = Convert(*p);
                if (order_queue_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->szCode) != subscribe_contracts_.end()))
                {
                    order_queue_handler_(&tdf_my);
                }

                // 存起来
                s_p_stock_oq_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &tdf_my);
                ++p;
            }
        }
        break;
        case MSG_DATA_ORDER:
        {
            TDF_ORDER *p = (TDF_ORDER*)pMsgHead->pData;

            timeval t;
            gettimeofday(&t, NULL);
            for (unsigned int i = 0; i < nItemCount; ++i)
            {
                T_PerEntrust tdf_my = Convert(*p);
                if (perentrust_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->szCode) != subscribe_contracts_.end()))
                {
                    perentrust_handler_(&tdf_my);
                }

                // 存起来
                s_p_stock_pe_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &tdf_my);
                ++p;
            }
        }
        break;
        case MSG_DATA_TRANSACTION:
        {
            TDF_TRANSACTION *p = (TDF_TRANSACTION*)pMsgHead->pData;

            timeval t;
            gettimeofday(&t, NULL);
            for (unsigned int i = 0; i < nItemCount; ++i)
            {
                T_PerBargain tdf_my = Convert(*p);
                if (perbargain_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->szCode) != subscribe_contracts_.end()))
                {
                    perbargain_handler_(&tdf_my);
                }

                // 存起来
                s_p_stock_pb_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &tdf_my);
                ++p;
            }
        }
        break;
        default:
            break;
    }
}

QuoteInterface_TDF::QuoteInterface_TDF(const SubscribeContracts* subscribe_contracts, const ConfigData& cfg)
    : cfg_(cfg), p_stock_md_save_(NULL), p_stock_idx_save_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    InitErrorMap();

    sprintf(qtm_name_, "tdf_%s_%u", cfg.Logon_config().account.c_str(), getpid());
    QuoteUpdateState(qtm_name_, QtmState::INIT);

    // create save object
    p_stock_md_save_ = new QuoteDataSave<TDF_MARKET_DATA_MY>(cfg_, qtm_name_, "tdf_market_data", TDF_STOCK_QUOTE_TYPE);
    p_stock_idx_save_ = new QuoteDataSave<TDF_INDEX_DATA_MY>(cfg_, qtm_name_, "tdf_index_data", TDF_INDEX_QUOTE_TYPE);
    p_stock_oq_save_ = new QuoteDataSave<T_OrderQueue>(cfg_, qtm_name_, "order_queue_data", KMDS_ORDER_QUEUE_TYPE);
    p_stock_pe_save_ = new QuoteDataSave<T_PerEntrust>(cfg_, qtm_name_, "per_entrust_data", KMDS_PER_ENTRUST_TYPE);
    p_stock_pb_save_ = new QuoteDataSave<T_PerBargain>(cfg_, qtm_name_, "per_bargain_data", KMDS_PER_BARGAIN_TYPE);
    s_p_stock_md_save = p_stock_md_save_;
    s_p_stock_idx_save = p_stock_idx_save_;
    s_p_stock_oq_save = p_stock_oq_save_;
    s_p_stock_pe_save = p_stock_pe_save_;
    s_p_stock_pb_save = p_stock_pb_save_;

    // environment settings
    //TDF_SetEnv(TDF_ENVIRON_HEART_BEAT_INTERVAL, 10);
    //TDF_SetEnv(TDF_ENVIRON_MISSED_BEART_COUNT, 2);
    //TDF_SetEnv(TDF_ENVIRON_OPEN_TIME_OUT, 30);
    TDF_SetEnv(TDF_ENVIRON_OUT_LOG, 1);

    // quote connection settings
    open_settings = new TDF_OPEN_SETTING();
    InitOpenSetting(open_settings, cfg);

    TDF_ERR nErr = TDF_ERR_SUCCESS;
    g_hTDF = TDF_Open(open_settings, &nErr);

    // try till open success
    while (nErr == TDF_ERR_NETWORK_ERROR || g_hTDF == NULL)
    {
        MY_LOG_ERROR("TDF_Open returned: %s; try again.", GetErrStr(nErr));
        QuoteUpdateState(qtm_name_, QtmState::CONNECT_FAIL);
        sleep(3);

        g_hTDF = TDF_Open(open_settings, &nErr);
    }
    MY_LOG_INFO("TDF_Open success.");
    QuoteUpdateState(qtm_name_, QtmState::CONNECT_SUCCESS);
    QuoteUpdateState(qtm_name_, QtmState::API_READY);
}

QuoteInterface_TDF::~QuoteInterface_TDF()
{
    MY_LOG_DEBUG("TDF_Close");
    TDF_Close(g_hTDF);
}

//系统消息回调，用于通知用户收到了网络断开事件、连接（重连）结果、代码表结果等。
//当获取系统消息时，pMsgHead->pAppHead指针为空,
//pMsgHead->pData指向相应的结构体
void QuoteInterface_TDF::SystemMsgHandler(THANDLE hTdf, TDF_MSG* pMsgHead)
{
    if (!pMsgHead || !hTdf)
    {
        return;
    }


    switch (pMsgHead->nDataType)
    {
        case MSG_SYS_DISCONNECT_NETWORK:
            {
            MY_LOG_INFO("network disconnected.");
            QuoteUpdateState(qtm_name_, QtmState::DISCONNECT);
        }
            break;
        case MSG_SYS_CONNECT_RESULT:
            {
            TDF_CONNECT_RESULT* pConnResult = (TDF_CONNECT_RESULT*) pMsgHead->pData;
            if (pConnResult && pConnResult->nConnResult)
            {
                MY_LOG_INFO("TDF - connect success.");
                QuoteUpdateState(qtm_name_, QtmState::CONNECT_SUCCESS);
            }
            else
            {
                MY_LOG_ERROR("TDF - connect failed.");
                QuoteUpdateState(qtm_name_, QtmState::CONNECT_FAIL);
            }
        }
            break;
        case MSG_SYS_LOGIN_RESULT:
            {
            TDF_LOGIN_RESULT* pLoginResult = (TDF_LOGIN_RESULT*) pMsgHead->pData;
            if (pLoginResult && pLoginResult->nLoginResult)
            {
                MY_LOG_INFO("TDF - login success.");
                QuoteUpdateState(qtm_name_, QtmState::LOG_ON_SUCCESS);
                for (int i = 0; i < pLoginResult->nMarkets; i++)
                {
                    MY_LOG_INFO("market: %s, dyn_date: %d", pLoginResult->szMarket[i], pLoginResult->nDynDate[i]);
                }
            }
            else
            {
                MY_LOG_ERROR("TDF - login failed.");
                QuoteUpdateState(qtm_name_, QtmState::LOG_ON_FAIL);
            }
        }
            break;
        case MSG_SYS_CODETABLE_RESULT:
            {
            TDF_CODE_RESULT* pCodeResult = (TDF_CODE_RESULT*) pMsgHead->pData;
            MY_LOG_INFO("code table: %s; market count: %d", pCodeResult->szInfo, pCodeResult->nMarkets);
            for (int i = 0; i < pCodeResult->nMarkets; i++)
            {
                MY_LOG_INFO("market: %s; items: %d; date: %d", pCodeResult->szMarket[i], pCodeResult->nCodeCount[i],
                    pCodeResult->nCodeDate[i]);
                //获取代码表
                TDF_CODE* pCodeTable;
                unsigned int nItems;
                TDF_GetCodeTable(hTdf, pCodeResult->szMarket[i], &pCodeTable, &nItems);
                for (unsigned int i = 0; i < nItems; i++)
                {
                    TDF_CODE& code = pCodeTable[i];
                    MY_LOG_INFO("code: %s; market: %s; type: %d", code.szCode, code.szMarket, code.nType);
                }
                TDF_FreeArr(pCodeTable);
            }
        }
            break;
        case MSG_SYS_QUOTATIONDATE_CHANGE:
            {
            TDF_QUOTATIONDATE_CHANGE* pChange = (TDF_QUOTATIONDATE_CHANGE*) pMsgHead->pData;
            if (pChange)
            {
                MY_LOG_INFO("date changed, prepare reconnect.");
            }
        }
            break;
        case MSG_SYS_MARKET_CLOSE:
            {
            TDF_MARKET_CLOSE* pCloseInfo = (TDF_MARKET_CLOSE*) pMsgHead->pData;
            if (pCloseInfo)
            {
                MY_LOG_INFO("close market: %s; time: %d", pCloseInfo->szMarket, pCloseInfo->nTime);
            }
        }
            break;
        case MSG_SYS_HEART_BEAT:
            {
            MY_LOG_INFO("TDF - heart beat.");
        }
            break;
        default:
            break;
    }
}
