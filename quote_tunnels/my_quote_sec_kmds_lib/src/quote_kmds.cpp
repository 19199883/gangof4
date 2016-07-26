#include "quote_kmds.h"
#include <iomanip>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

KMDSUSERAPIHANDLE QuoteInterface_KMDS::pclient = NULL;
boost::function<void(const TDF_MARKET_DATA_MY *)> QuoteInterface_KMDS::md_data_handler_;
boost::function<void(const TDF_INDEX_DATA_MY *)> QuoteInterface_KMDS::id_data_handler_;
boost::function<void(const T_OptionQuote *)> QuoteInterface_KMDS::st_option_data_handler_;
boost::function<void(const T_OrderQueue *)> QuoteInterface_KMDS::st_order_queue_handler_;
boost::function<void(const T_PerEntrust *)> QuoteInterface_KMDS::st_per_entrust_handler_;
boost::function<void(const T_PerBargain *)> QuoteInterface_KMDS::st_per_bargain_handler_;

SubscribeContracts QuoteInterface_KMDS::subscribe_contracts_;
ConfigData QuoteInterface_KMDS::cfg_;
char QuoteInterface_KMDS::qtm_name_[32] = "kmds";

QuoteDataSave<TDF_MARKET_DATA_MY> *QuoteInterface_KMDS::s_p_stock_md_save;
QuoteDataSave<TDF_INDEX_DATA_MY> *QuoteInterface_KMDS::s_p_stock_idx_save;
QuoteDataSave<T_StockQuote> *QuoteInterface_KMDS::s_p_kmds_stock_save;
QuoteDataSave<T_FutureQuote> *QuoteInterface_KMDS::s_p_kmds_future_save;
QuoteDataSave<T_OptionQuote> *QuoteInterface_KMDS::s_p_kmds_option_save;
QuoteDataSave<T_IndexQuote> *QuoteInterface_KMDS::s_p_kmds_index_save;
QuoteDataSave<T_OrderQueue> *QuoteInterface_KMDS::s_p_kmds_ord_queue_save;
QuoteDataSave<T_PerEntrust> *QuoteInterface_KMDS::s_p_kmds_per_entrust_save;
QuoteDataSave<T_PerBargain> *QuoteInterface_KMDS::s_p_kmds_per_bgn_save;

boost::mutex QuoteInterface_KMDS::md_sync_mutex_;
StockCodeMap QuoteInterface_KMDS::stock_code_map_;
volatile int QuoteInterface_KMDS::ready_flag = 0;

static int s_stk_pv_fields_cnt = 10;
static char *s_stk_sell_prc_field[] =
    { "APL_SELL_PRC_1", "APL_SELL_PRC_2", "APL_SELL_PRC_3", "APL_SELL_PRC_4", "APL_SELL_PRC_5",
        "APL_SELL_PRC_6", "APL_SELL_PRC_7", "APL_SELL_PRC_8", "APL_SELL_PRC_9", "APL_SELL_PRC_10" };

static char *s_stk_sell_amt_field[] =
    { "APL_SELL_AMT_1", "APL_SELL_AMT_2", "APL_SELL_AMT_3", "APL_SELL_AMT_4", "APL_SELL_AMT_5",
        "APL_SELL_AMT_6", "APL_SELL_AMT_7", "APL_SELL_AMT_8", "APL_SELL_AMT_9", "APL_SELL_AMT_10" };

static char *s_stk_bid_prc_field[] =
    { "APL_BID_PRC_1", "APL_BID_PRC_2", "APL_BID_PRC_3", "APL_BID_PRC_4", "APL_BID_PRC_5",
        "APL_BID_PRC_6", "APL_BID_PRC_7", "APL_BID_PRC_8", "APL_BID_PRC_9", "APL_BID_PRC_10" };

static char *s_stk_bid_amt_field[] =
    { "APL_BID_AMT_1", "APL_BID_AMT_2", "APL_BID_AMT_3", "APL_BID_AMT_4", "APL_BID_AMT_5",
        "APL_BID_AMT_6", "APL_BID_AMT_7", "APL_BID_AMT_8", "APL_BID_AMT_9", "APL_BID_AMT_10" };

QuoteInterface_KMDS::QuoteInterface_KMDS(const SubscribeContracts* subscribe_contracts, const ConfigData& cfg)
{
    if (pclient) return;

    cfg_ = cfg;
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    sprintf(qtm_name_, "kmds_stock_%s_%u", cfg.Logon_config().account.c_str(), getpid());
    QuoteUpdateState(qtm_name_, QtmState::INIT);

    // create save object
    s_p_stock_md_save = new QuoteDataSave<TDF_MARKET_DATA_MY>(cfg_, qtm_name_, "tdf_market_data", TDF_STOCK_QUOTE_TYPE, false);
    s_p_stock_idx_save = new QuoteDataSave<TDF_INDEX_DATA_MY>(cfg_, qtm_name_, "tdf_index_data", TDF_INDEX_QUOTE_TYPE, false);
    s_p_kmds_stock_save = new QuoteDataSave<T_StockQuote>(cfg_, qtm_name_, "kmds_stock_data", KMDS_STOCK_SNAPSHOT_TYPE);
    s_p_kmds_future_save = new QuoteDataSave<T_FutureQuote>(cfg_, qtm_name_, "kmds_future_data", KMDS_FUTURE_QUOTE_TYPE);
    s_p_kmds_option_save = new QuoteDataSave<T_OptionQuote>(cfg_, qtm_name_, "kmds_option_data", KMDS_OPTION_QUOTE_TYPE);
    s_p_kmds_index_save = new QuoteDataSave<T_IndexQuote>(cfg_, qtm_name_, "kmds_index_data", KMDS_INDEX_TYPE, false);

    s_p_kmds_ord_queue_save = new QuoteDataSave<T_OrderQueue>(cfg_, qtm_name_, "kmds_order_queue_data", KMDS_ORDER_QUEUE_TYPE, false);
    s_p_kmds_per_entrust_save = new QuoteDataSave<T_PerEntrust>(cfg_, qtm_name_, "kmds_per_entrust_data", KMDS_PER_ENTRUST_TYPE, false);
    s_p_kmds_per_bgn_save = new QuoteDataSave<T_PerBargain>(cfg_, qtm_name_, "kmds_per_bargin_data", KMDS_PER_BARGAIN_TYPE, false);

    Init();
}

QuoteInterface_KMDS::~QuoteInterface_KMDS()
{
    if (pclient)
    {
        KmdsUserApi_Logout(pclient);
        KmdsUserApi_UnInit(pclient);
        KmdsUserApi_Destory(pclient);
    }

    if (s_p_stock_md_save) delete s_p_stock_md_save;
    if (s_p_stock_idx_save) delete s_p_stock_idx_save;
    if (s_p_kmds_stock_save) delete s_p_kmds_stock_save;
    if (s_p_kmds_future_save) delete s_p_kmds_future_save;
    if (s_p_kmds_option_save) delete s_p_kmds_option_save;
    if (s_p_kmds_index_save) delete s_p_kmds_index_save;

    if (s_p_kmds_ord_queue_save) delete s_p_kmds_ord_queue_save;
    if (s_p_kmds_per_entrust_save) delete s_p_kmds_per_entrust_save;
    if (s_p_kmds_per_bgn_save) delete s_p_kmds_per_bgn_save;
}

void QuoteInterface_KMDS::Init()
{
    pclient = KmdsUserApi_Create();
    if (pclient == NULL)
    {
        QuoteUpdateState(qtm_name_, QtmState::QUOTE_INIT_FAIL);
        MY_LOG_ERROR("KmdsUserApi_Create return null");
        return;
    }

    //初始化API
    int nret = KmdsUserApi_Init(pclient);
    if (nret != 0)
    {
        QuoteUpdateState(qtm_name_, QtmState::QUOTE_INIT_FAIL);
        MY_LOG_ERROR("KmdsUserApi_Init failed, error_code=%d,error_msg=%s", KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));
        KmdsUserApi_UnInit(pclient);
        KmdsUserApi_Destory(pclient);
        return;
    }

    //登录
    nret = KmdsUserApi_Login(pclient, cfg_.Logon_config().account.c_str(), cfg_.Logon_config().password.c_str());
    if (nret == 0)
    {
        QuoteUpdateState(qtm_name_, QtmState::LOG_ON_SUCCESS);
    }
    else
    {
        QuoteUpdateState(qtm_name_, QtmState::LOG_ON_FAIL);
        MY_LOG_ERROR("KmdsUserApi_Login failed, error_code=%d,error_msg=%s", KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));
        KmdsUserApi_UnInit(pclient);
        KmdsUserApi_Destory(pclient);
        return;
    }

    //设置回调
    KmdsUserApi_SetKmdsMsgCB(pclient, OnKmdsMsg);

    //订阅主题
    std::vector<std::string> topics;
    boost::split(topics, cfg_.Logon_config().topic, boost::is_any_of("|"));
    bool sub_all_failed = true;
    for (const std::string &v : topics)
    {
        if (v.empty())
        {
            continue;
        }

        MY_LOG_INFO("KmdsUserApi_Subscribe topic is:%s", v.c_str());
        nret = KmdsUserApi_Subscribe(pclient, v.c_str());
        if (nret == 0)
        {
            sub_all_failed = false;
            MY_LOG_INFO("KmdsUserApi_Subscribe topic(%s) successfully", v.c_str());
        }
        else
        {
            MY_LOG_ERROR("KmdsUserApi_Subscribe failed, error_code=%d,error_msg=%s", KmdsUserApi_GetErrCode(pclient),
                KmdsUserApi_GetErrMsg(pclient));
        }
    }
    if (sub_all_failed)
    {
        QuoteUpdateState(qtm_name_, QtmState::QUOTE_INIT_FAIL);
        KmdsUserApi_UnInit(pclient);
        KmdsUserApi_Destory(pclient);
        return;
    }

    // get code table
    boost::thread t(&QuoteInterface_KMDS::GetCodeTable);

    QuoteUpdateState(qtm_name_, QtmState::API_READY);
}

void QuoteInterface_KMDS::OnKmdsMsg(void* pmsgdesc, void* pmsgopts, int ncmd, MSGDATAHANDLE pmsg)
{
    if (pmsg == NULL)
    {
        MY_LOG_WARN("OnKmdsMsg receive null data object");
        return;
    }

//    if (ready_flag == 0)
//    {
//        return;
//    }

    switch (ncmd)
    {
        case 2061:  // CMD_HQSERVER_ANS_CODETABLE
            OnCodeTableMsg(pmsg);
            break;
        case CKMDS_HQ_STK_PART_SNPST:
            OnStockMD(pmsg);
            break;
        case CKMDS_HQ_FT_PART_SNPST:
            OnFutureMD(pmsg);
            break;
        case CKMDS_HQ_INDEX_PART_SNPST:
            OnIndex(pmsg);
            break;
        case 6014: // CMD_HQSERVER_HQ_STKOP_SNAPSHOT
            OnStockOptionMD(pmsg);
            break;
        case CKMDS_HQ_ORDR_QU:
            OnOrderQueue(pmsg);
            break;
        case CKMDS_HQ_PER_STRK_ENTRT:
            OnPerEntrust(pmsg);
            break;
        case CKMDS_HQ_PER_STRK_BGN:
            OnTransaction(pmsg);
            break;
        default:
            if (pmsgdesc)
            {
                MY_LOG_WARN("receive message:%s, ncmd:%d", (char * )pmsgdesc, ncmd);
            }
            break;
    }

    //CMsgData_Release(pmsg);
}

void QuoteInterface_KMDS::GetCodeTable()
{
    T_StockCode stkcode;
    MSGDATAHANDLE pdata = NULL;

    char *market[8] =
        { "1", "2", "4", "5", "6", "7", "15", "19" };

    for (int i = 0; i < 8; ++i)
    {
        MSGDATAHANDLE preqdata = CMsgData_Create();
        CMsgData_PutData(preqdata, "MKT_TP_CODE", market[i]);

        bool query_table_finished = false;

        while (!query_table_finished)
        {
            int ret = KmdsUserApi_GetHqData(pclient, CKMDS_HQ_CODETABLE, preqdata, &pdata);
            if (ret != 0)
            {
                MY_LOG_ERROR("GetCodeTable-KmdsUserApi_GetHqData failed, ret=%d,error_code=%d,error_msg=%s", ret,
                    KmdsUserApi_GetErrCode(pclient),
                    KmdsUserApi_GetErrMsg(pclient));
            }
            else
            {
                TABLEDATAHANDLE ptable = NULL;
                MSGDATAHANDLE prow = NULL;
                ret = CMsgData_GetData_TableData(pdata, "DATA0", &ptable);
                if (ret == 0 && ptable)
                {
                    int nrowcount = 0;
                    ret = CTableData_GetCount(ptable, (unsigned int *) &nrowcount);
                    if (ret != 0)
                    {
                        MY_LOG_INFO("GetCodeTable-CTableData_GetCount of codetable row_count:%d", nrowcount);
                        nrowcount = 0;
                    }
                    for (int i = 0; i < nrowcount; ++i)
                    {
                        ret = CTableData_GetRow(ptable, i, &prow);
                        if (ret == 0 && prow)
                        {
                            CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) stkcode.scr_code, sizeof(stkcode.scr_code));
                            CMsgData_GetData_String(prow, "SCR_NM", (unsigned char *) stkcode.scr_name, sizeof(stkcode.scr_name));
                            CMsgData_GetData_Int32(prow, "MKT_TP_CODE", &stkcode.market);
                            CMsgData_GetData_Int64(prow, "YSTD_CLS_QTN_PRICE", &stkcode.pre_close);
                            CMsgData_GetData_Int64(prow, "OPN_QTN_PRICE", &stkcode.open_price);
                            CMsgData_GetData_Int64(prow, "RISE_LMT_PRICE", &stkcode.high_price);
                            CMsgData_GetData_Int64(prow, "FALL_LMT_PRICE", &stkcode.low_price);
                            CMsgData_GetData_Int64(prow, "YSTD_DELTA", &stkcode.pre_delta);
                            CMsgData_GetData_Int64(prow, "YSTD_MKT_MAKE_POS_TOT_NUM", &stkcode.pre_open_interest);
                            CMsgData_GetData_Int64(prow, "TDY_SETL_PRICE", &stkcode.settle_price);
                            CMsgData_GetData_Int64(prow, "YSTD_SETL_PRICE", &stkcode.pre_settle_price);

                            MY_LOG_INFO("code_table(qry) - "
                                "scr_code=%s,"
                                "scr_name = %s,"
                                "market=%d,"
                                "pre_close=%lld,"
                                "open_price=%lld,"
                                "high_price=%lld,"
                                "low_price=%lld,"
                                "pre_delta=%lld,"
                                "pre_open_interest=%lld,"
                                "settle_price=%lld,"
                                "pre_settle_price=%lld",
                                stkcode.scr_code,
                                stkcode.scr_name,
                                stkcode.market,
                                stkcode.pre_close,
                                stkcode.open_price,
                                stkcode.high_price,
                                stkcode.low_price,
                                stkcode.pre_delta,
                                stkcode.pre_open_interest,
                                stkcode.settle_price,
                                stkcode.pre_settle_price);

                            // cache it
                            AddCodeInfo(stkcode.market, stkcode.scr_code, stkcode);
                        }
                        else
                        {
                            MY_LOG_WARN("GetCodeTable-CTableData_GetRow failed, return=%d,error_code=%d,error_msg=%s", ret,
                                KmdsUserApi_GetErrCode(pclient),
                                KmdsUserApi_GetErrMsg(pclient));
                        }
                    }
                    CTableData_Release(ptable);
                    query_table_finished = true;
                }
                else
                {
                    MY_LOG_WARN("GetCodeTable-CMsgData_GetData_TableData failed, return=%d,error_code=%d,error_msg=%s", ret,
                        KmdsUserApi_GetErrCode(pclient),
                        KmdsUserApi_GetErrMsg(pclient));
                    query_table_finished = false;
                }

                if (query_table_finished && preqdata)
                {
                    CMsgData_Release(preqdata);
                    preqdata = NULL;
                }
            }

            if (!query_table_finished)
            {
                // wait some time,then retry;
                sleep(20);
            }
        }
    }

    ready_flag = 1;
    MY_LOG_WARN("GetCodeTable-finished");
    return;
}
void QuoteInterface_KMDS::OnCodeTableMsg(MSGDATAHANDLE pmsg)
{
    TABLEDATAHANDLE ptable = NULL;
    MSGDATAHANDLE prow = NULL;
    T_StockCode data;

    int ret = CMsgData_GetData_TableData(pmsg, "SEQ_HQ_CODETABLE", &ptable);
    if (ret != 0)
    {
        MY_LOG_ERROR("OnCodeTableMsg-CMsgData_GetData_TableData failed, ret=%d,error_code=%d,error_msg=%s", ret,
            KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));

        ptable = NULL;
    }
    if (ptable)
    {
        timeval t;
        gettimeofday(&t, NULL);
        int nrowcount = 0;
        ret = CTableData_GetCount(ptable, (unsigned int *) &nrowcount); /*结果集记录数*/
        if (ret != 0)
        {
            MY_LOG_INFO("OnCodeTableMsg-CTableData_GetCount of codetable row_count:%d, return:%d", nrowcount, ret);
            nrowcount = 0;
        }
        for (int i = 0; i < nrowcount; ++i)
        {
            ret = CTableData_GetRow(ptable, i, &prow);
            if (ret == 0 && prow)
            {
                memset(&data, 0, sizeof(data));

                CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) data.scr_code, sizeof(data.scr_code));
                CMsgData_GetData_String(prow, "SCR_NM", (unsigned char *) data.scr_name, sizeof(data.scr_name));
                CMsgData_GetData_Int32(prow, "MKT_TP_CODE", &data.market);
                CMsgData_GetData_Int64(prow, "YSTD_CLS_QTN_PRICE", &data.pre_close);
                CMsgData_GetData_Int64(prow, "OPN_QTN_PRICE", &data.open_price);
                CMsgData_GetData_Int64(prow, "RISE_LMT_PRICE", &data.high_price);
                CMsgData_GetData_Int64(prow, "FALL_LMT_PRICE", &data.low_price);
                CMsgData_GetData_Int64(prow, "YSTD_DELTA", &data.pre_delta);
                CMsgData_GetData_Int64(prow, "YSTD_MKT_MAKE_POS_TOT_NUM", &data.pre_open_interest);
                CMsgData_GetData_Int64(prow, "TDY_SETL_PRICE", &data.settle_price);
                CMsgData_GetData_Int64(prow, "YSTD_SETL_PRICE", &data.pre_settle_price);

                MY_LOG_INFO("code_table(pub) - "
                    "scr_code=%s,"
                    "scr_name = %s,"
                    "market=%d,"
                    "pre_close=%lld,"
                    "open_price=%lld,"
                    "high_price=%lld,"
                    "low_price=%lld,"
                    "pre_delta=%lld,"
                    "pre_open_interest=%lld,"
                    "settle_price=%lld,"
                    "pre_settle_price=%lld",
                    data.scr_code,
                    data.scr_name,
                    data.market,
                    data.pre_close,
                    data.open_price,
                    data.high_price,
                    data.low_price,
                    data.pre_delta,
                    data.pre_open_interest,
                    data.settle_price,
                    data.pre_settle_price);

                // cache it
                AddCodeInfo(data.market, data.scr_code, data);
            }
            else
            {
                MY_LOG_WARN("OnCodeTableMsg-CTableData_GetRow failed, return=%d,error_code=%d,error_msg=%s", ret,
                    KmdsUserApi_GetErrCode(pclient),
                    KmdsUserApi_GetErrMsg(pclient));
            }
        }
        CTableData_Release(ptable);
    }
}
void QuoteInterface_KMDS::OnStockMD(MSGDATAHANDLE pmsg)
{
    TABLEDATAHANDLE ptable = NULL;
    MSGDATAHANDLE prow = NULL;
    T_StockQuote data;

    int ret = CMsgData_GetData_TableData(pmsg, "SEQ_HQ_SNAPSHOT", &ptable);
    if (ret != 0)
    {
        MY_LOG_ERROR("OnStockMD-CMsgData_GetData_TableData failed, ret=%d,error_code=%d,error_msg=%s", ret,
            KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));

        ptable = NULL;
    }
    if (ptable)
    {
        timeval t;
        gettimeofday(&t, NULL);
        int nrowcount = 0;
        ret = CTableData_GetCount(ptable, (unsigned int *) &nrowcount); /*结果集记录数*/
        if (ret != 0)
        {
            MY_LOG_INFO("OnStockMD-CTableData_GetCount of codetable row_count:%d, return:%d", nrowcount, ret);
            nrowcount = 0;
        }
        for (int i = 0; i < nrowcount; ++i)
        {
            ret = CTableData_GetRow(ptable, i, &prow);
            if (ret == 0 && prow)
            {
                memset(&data, 0, sizeof(data));

                CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) &data.scr_code, sizeof(data.scr_code));

                CMsgData_GetData_Int32(prow, "TM", &data.time);
                CMsgData_GetData_Int32(prow, "MKT_TP_CODE", &data.market);
                CMsgData_GetData_Int64(prow, "SEQ_ID", &data.seq_id);
                CMsgData_GetData_Int32(prow, "HIGH_PRICE", &data.high_price);
                CMsgData_GetData_Int32(prow, "LWS_PRICE", &data.low_price);
                CMsgData_GetData_Int32(prow, "LTST_PRICE", &data.last_price);
                CMsgData_GetData_Int32(prow, "BGN_CNT", &data.bgn_count);
                CMsgData_GetData_Int64(prow, "BGN_TOT_NUM", &data.bgn_total_vol);
                CMsgData_GetData_Int64(prow, "BGN_TOT_AMT", &data.bgn_total_amt);
                CMsgData_GetData_Int64(prow, "ENTRT_BUY_TOT_NUM", &data.total_bid_lot);
                CMsgData_GetData_Int64(prow, "ENTRT_SELL_TOT_NUM", &data.total_sell_lot);
                CMsgData_GetData_Int32(prow, "WGHT_AVG_ENTRT_BUY_PRC", &data.wght_avl_bid_price);
                CMsgData_GetData_Int32(prow, "WGHT_AVG_ENTRT_SELL_PRC", &data.wght_avl_sell_price);

                CMsgData_GetData_Int32(prow, "IOPV_NET_VAL_VALT", &data.iopv_net_val_valt);
                CMsgData_GetData_Int32(prow, "MTUR_YLD", &data.mtur_yld);
                CMsgData_GetData_Int32(prow, "PART_SNPST_ST_TP_CODE", &data.status);
                CMsgData_GetData_Int32(prow, "PE_RATIO_1", &data.pe_ratio_1);
                CMsgData_GetData_Int32(prow, "PE_RATIO_2", &data.pe_ratio_2);
                CMsgData_GetData_Int64(prow, "ERROR_MARK", &data.error_mark);
                CMsgData_GetData_Int64(prow, "PUBLISH_TM1", &data.publish_tm1);
                CMsgData_GetData_Int64(prow, "PUBLISH_TM2", &data.publish_tm2);
                CMsgData_GetData_String(prow, "PRE_FIX", (unsigned char *) &data.pre_fix, sizeof(data.pre_fix));

                for (int j = 0; j < s_stk_pv_fields_cnt; j++)
                {
                    CMsgData_GetData_Int32(prow, s_stk_sell_prc_field[j], &data.sell_price[j]);
                    CMsgData_GetData_Int32(prow, s_stk_bid_prc_field[j], &data.bid_price[j]);
                    CMsgData_GetData_UInt32(prow, s_stk_sell_amt_field[j], &data.sell_volume[j]);
                    CMsgData_GetData_UInt32(prow, s_stk_bid_amt_field[j], &data.bid_volume[j]);
                }

                // convert to tdf type
                TDF_MARKET_DATA_MY tdf_data = Convert(data);

                // send to client
                if (md_data_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(data.scr_code) != subscribe_contracts_.end()))
                {
                    md_data_handler_(&tdf_data);
                }

                // save it
                s_p_stock_md_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &tdf_data);
                s_p_kmds_stock_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
            }
            else
            {
                MY_LOG_WARN("OnStockMD-CTableData_GetRow failed, return=%d,error_code=%d,error_msg=%s", ret,
                    KmdsUserApi_GetErrCode(pclient),
                    KmdsUserApi_GetErrMsg(pclient));
            }
        }
        CTableData_Release(ptable);
    }
}

void QuoteInterface_KMDS::OnFutureMD(MSGDATAHANDLE pmsg)
{
    TABLEDATAHANDLE ptable = NULL;
    MSGDATAHANDLE prow = NULL;
    T_FutureQuote data;

    int ret = CMsgData_GetData_TableData(pmsg, "SEQ_HQ_SNAPSHOT", &ptable);
    if (ret != 0)
    {
        MY_LOG_ERROR("OnFutureMD-CMsgData_GetData_TableData failed, ret=%d,error_code=%d,error_msg=%s", ret,
            KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));

        ptable = NULL;
    }
    if (ptable)
    {
        int nrowcount = 0;
        timeval t;
        gettimeofday(&t, NULL);
        ret = CTableData_GetCount(ptable, (unsigned int *) &nrowcount); /*结果集记录数*/
        if (ret != 0)
        {
            MY_LOG_INFO("OnFutureMD-CTableData_GetCount of codetable row_count:%d, return:%d", nrowcount, ret);
            nrowcount = 0;
        }
        for (int i = 0; i < nrowcount; ++i)
        {
            ret = CTableData_GetRow(ptable, i, &prow);
            if (ret == 0 && prow)
            {
                memset(&data, 0, sizeof(data));

                CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) &data.scr_code, sizeof(data.scr_code));

                CMsgData_GetData_Int32(prow, "TM", &data.time);
                CMsgData_GetData_Int32(prow, "MKT_TP_CODE", &data.market);
                CMsgData_GetData_Int32(prow, "HIGH_PRICE", &data.high_price);
                CMsgData_GetData_Int32(prow, "LWS_PRICE", &data.low_price);
                CMsgData_GetData_Int32(prow, "LTST_PRICE", &data.last_price);
                CMsgData_GetData_Int32(prow, "TDY_CLS_QTN_PRICE", &data.today_close);
                CMsgData_GetData_Int32(prow, "TDY_DELTA", &data.today_delta);
                CMsgData_GetData_Int64(prow, "BGN_TOT_NUM", &data.bgn_total_vol);
                CMsgData_GetData_Int64(prow, "BGN_TOT_AMT", &data.bgn_total_amt);
                CMsgData_GetData_Int64(prow, "TDY_MKT_MAKE_POS_TOT_NUM", &data.today_pos);
                CMsgData_GetData_Int64(prow, "ERROR_MARK", &data.error_mark);
                CMsgData_GetData_Int64(prow, "PUBLISH_TM1", &data.publish_tm1);
                CMsgData_GetData_Int64(prow, "PUBLISH_TM2", &data.publish_tm2);

                for (int j = 0; j < 5; j++)
                {
                    CMsgData_GetData_Int32(prow, s_stk_sell_prc_field[j], &data.sell_price[j]);
                    CMsgData_GetData_UInt32(prow, s_stk_sell_amt_field[j], &data.sell_volume[j]);
                    CMsgData_GetData_Int32(prow, s_stk_bid_prc_field[j], &data.bid_price[j]);
                    CMsgData_GetData_UInt32(prow, s_stk_bid_amt_field[j], &data.bid_volume[j]);
                }

                // send to client

                // save it
                s_p_kmds_future_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
            }
            else
            {
                MY_LOG_WARN("OnFutureMD-CTableData_GetRow failed, return=%d,error_code=%d,error_msg=%s", ret,
                    KmdsUserApi_GetErrCode(pclient),
                    KmdsUserApi_GetErrMsg(pclient));
            }
        }

        CTableData_Release(ptable);
    }
}

void QuoteInterface_KMDS::OnIndex(MSGDATAHANDLE pmsg)
{
    TABLEDATAHANDLE ptable = NULL;
    MSGDATAHANDLE prow = NULL;
    T_IndexQuote data;

    int ret = CMsgData_GetData_TableData(pmsg, "SEQ_HQ_SNAPSHOT", &ptable);
    if (ret != 0)
    {
        MY_LOG_ERROR("OnIndex-CMsgData_GetData_TableData failed, ret=%d,error_code=%d,error_msg=%s", ret,
            KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));

        ptable = NULL;
    }
    if (ptable)
    {
        timeval t;
        gettimeofday(&t, NULL);
        int nrowcount = 0;
        ret = CTableData_GetCount(ptable, (unsigned int *) &nrowcount); /*结果集记录数*/
        if (ret != 0)
        {
            MY_LOG_INFO("OnIndex-CTableData_GetCount of codetable row_count:%d, return:%d", nrowcount, ret);
            nrowcount = 0;
        }
        for (int i = 0; i < nrowcount; ++i)
        {
            int ret = CTableData_GetRow(ptable, i, &prow);
            if (ret == 0 && prow)
            {
                memset(&data, 0, sizeof(data));

                CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) &data.scr_code, sizeof(data.scr_code));

                CMsgData_GetData_Int32(prow, "TM", &data.time);
                CMsgData_GetData_Int32(prow, "MKT_TP_CODE", &data.market);
                CMsgData_GetData_Int64(prow, "SEQ_ID", &data.seq_id);
                CMsgData_GetData_Int32(prow, "HIGH_PRICE", &data.high_price);
                CMsgData_GetData_Int32(prow, "LWS_PRICE", &data.low_price);
                CMsgData_GetData_Int32(prow, "LTST_PRICE", &data.last_price);
                CMsgData_GetData_Int64(prow, "BGN_TOT_NUM", &data.bgn_total_vol);
                CMsgData_GetData_Int64(prow, "BGN_TOT_AMT", &data.bgn_total_amt);
                CMsgData_GetData_Int64(prow, "ERROR_MARK", &data.error_mark);
                CMsgData_GetData_Int64(prow, "PUBLISH_TM1", &data.publish_tm1);
                CMsgData_GetData_Int64(prow, "PUBLISH_TM2", &data.publish_tm2);

                // convert to tdf type
                TDF_INDEX_DATA_MY tdf_data = Convert(data);

                // send to client
                if (id_data_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(data.scr_code) != subscribe_contracts_.end()))
                {
                    id_data_handler_(&tdf_data);
                }

                // save it
                s_p_kmds_index_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
                s_p_stock_idx_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &tdf_data);
            }
            else
            {
                MY_LOG_WARN("OnIndex-CTableData_GetRow failed, return=%d,error_code=%d,error_msg=%s", ret,
                    KmdsUserApi_GetErrCode(pclient),
                    KmdsUserApi_GetErrMsg(pclient));
            }
        }

        CTableData_Release(ptable);
    }
}

void QuoteInterface_KMDS::OnOrderQueue(MSGDATAHANDLE pmsg)
{
    TABLEDATAHANDLE ptable = NULL;
    MSGDATAHANDLE prow = NULL;
    T_OrderQueue data;
    char buffer[4096];

    int ret = CMsgData_GetData_TableData(pmsg, "SEQ_HQ_ORDR_QU", &ptable);
    if (ret != 0)
    {
        MY_LOG_ERROR("OnOrderQueue-CMsgData_GetData_TableData failed, ret=%d,error_code=%d,error_msg=%s", ret,
            KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));

        ptable = NULL;
    }
    if (ptable)
    {
        timeval t;
        gettimeofday(&t, NULL);
        int nrowcount = 0;
        ret = CTableData_GetCount(ptable, (unsigned int *) &nrowcount); /*结果集记录数*/
        if (ret != 0)
        {
            MY_LOG_INFO("OnOrderQueue-CTableData_GetCount of codetable row_count:%d, return:%d", nrowcount, ret);
            nrowcount = 0;
        }
        for (int i = 0; i < nrowcount; ++i)
        {
            int ret = CTableData_GetRow(ptable, i, &prow);
            if (ret == 0 && prow)
            {
                memset(&data, 0, sizeof(data));

                CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) &data.scr_code, sizeof(data.scr_code));

                CMsgData_GetData_Int32(prow, "TM", &data.time);
                CMsgData_GetData_Int32(prow, "MKT_TP_CODE", &data.market);
                CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) &data.scr_code, sizeof(data.scr_code));
                CMsgData_GetData_String(prow, "INSR_TXN_TP_CODE", (unsigned char *) &data.insr_txn_tp_code,
                    sizeof(data.insr_txn_tp_code));

                CMsgData_GetData_Int32(prow, "PRC", &data.ord_price);
                CMsgData_GetData_Int32(prow, "ORDR_TOT_RND_LOT_NBR", &data.ord_qty);
                CMsgData_GetData_Int32(prow, "ORDR_NBR", &data.ord_nbr);

                // ORDR_DTL_RND_LOT_NBR  订单明细数量  String  以'|'分割
                ret = CMsgData_GetData_String(prow, "ORDR_DTL_RND_LOT_NBR", (unsigned char *) buffer, sizeof(buffer));
                if (ret == 0 && strlen(buffer) > 0)
                {
                    std::vector<std::string> fields;
                    boost::split(fields, buffer, boost::is_any_of("|"));
                    int offset = 0;
                    for (const std::string & v : fields)
                    {
                        int volume = atoi(v.c_str());
                        if (volume > 0)
                        {
                            data.ord_detail_vol[offset] = volume;
                            ++offset;
                        }
                        if (offset >= 200)
                        {
                            MY_LOG_ERROR("order volume detail exceed 200");
                            break;
                        }
                    }
                }

                // send to client
                if (st_order_queue_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(data.scr_code) != subscribe_contracts_.end()))
                {
                    st_order_queue_handler_(&data);
                }
                // save it
                s_p_kmds_ord_queue_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
            }
            else
            {
                MY_LOG_WARN("OnOrderQueue-CTableData_GetRow failed, return=%d,error_code=%d,error_msg=%s", ret,
                    KmdsUserApi_GetErrCode(pclient),
                    KmdsUserApi_GetErrMsg(pclient));
            }
        }

        CTableData_Release(ptable);
    }
}

void QuoteInterface_KMDS::OnPerEntrust(MSGDATAHANDLE pmsg)
{
    TABLEDATAHANDLE ptable = NULL;
    MSGDATAHANDLE prow = NULL;
    T_PerEntrust data;

    int ret = CMsgData_GetData_TableData(pmsg, "SEQ_HQ_PER_STRK_ENTRT", &ptable);
    if (ret != 0)
    {
        MY_LOG_ERROR("OnPerEntrust-CMsgData_GetData_TableData failed, ret=%d,error_code=%d,error_msg=%s", ret,
            KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));

        ptable = NULL;
    }
    if (ptable)
    {
        timeval t;
        gettimeofday(&t, NULL);
        int nrowcount = 0;
        ret = CTableData_GetCount(ptable, (unsigned int *) &nrowcount); /*结果集记录数*/
        if (ret != 0)
        {
            MY_LOG_INFO("OnPerEntrust-CTableData_GetCount of codetable row_count:%d, return:%d", nrowcount, ret);
            nrowcount = 0;
        }
        for (int i = 0; i < nrowcount; ++i)
        {
            int ret = CTableData_GetRow(ptable, i, &prow);
            if (ret == 0 && prow)
            {
                memset(&data, 0, sizeof(data));

                CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) &data.scr_code, sizeof(data.scr_code));

                CMsgData_GetData_Int32(prow, "MKT_TP_CODE", &data.market);
                CMsgData_GetData_Int32(prow, "ENTRT_TM", &data.entrt_time);
                CMsgData_GetData_Int32(prow, "ENTRT_PRC", &data.entrt_price);
                CMsgData_GetData_Int64(prow, "ENTRT_QTY", &data.entrt_vol);
                CMsgData_GetData_Int64(prow, "ENTRT_ID", &data.entrt_id);

                CMsgData_GetData_String(prow, "INSR_TXN_TP_CODE", (unsigned char *) &data.insr_txn_tp_code,
                    sizeof(data.insr_txn_tp_code));
                CMsgData_GetData_String(prow, "ENTRT_TP", (unsigned char *) &data.entrt_tp, sizeof(data.entrt_tp));

                // send to client
                if (st_per_entrust_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(data.scr_code) != subscribe_contracts_.end()))
                {
                    st_per_entrust_handler_(&data);
                }
                // save it
                s_p_kmds_per_entrust_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
            }
            else
            {
                MY_LOG_WARN("OnPerEntrust-CTableData_GetRow failed, return=%d,error_code=%d,error_msg=%s", ret,
                    KmdsUserApi_GetErrCode(pclient),
                    KmdsUserApi_GetErrMsg(pclient));
            }
        }

        CTableData_Release(ptable);
    }
}

void QuoteInterface_KMDS::OnTransaction(MSGDATAHANDLE pmsg)
{
    TABLEDATAHANDLE ptable = NULL;
    MSGDATAHANDLE prow = NULL;
    T_PerBargain data;

    int ret = CMsgData_GetData_TableData(pmsg, "SEQ_HQ_STRK_BGN", &ptable);
    if (ret != 0)
    {
        MY_LOG_ERROR("OnTransaction-CMsgData_GetData_TableData failed, ret=%d,error_code=%d,error_msg=%s", ret,
            KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));

        ptable = NULL;
    }
    if (ptable)
    {
        timeval t;
        gettimeofday(&t, NULL);
        int nrowcount = 0;
        ret = CTableData_GetCount(ptable, (unsigned int *) &nrowcount); /*结果集记录数*/
        if (ret != 0)
        {
            MY_LOG_INFO("OnTransaction-CTableData_GetCount of codetable row_count:%d, return:%d", nrowcount, ret);
            nrowcount = 0;
        }
        for (int i = 0; i < nrowcount; ++i)
        {
            int ret = CTableData_GetRow(ptable, i, &prow);
            if (ret == 0 && prow)
            {
                memset(&data, 0, sizeof(data));

                CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) &data.scr_code, sizeof(data.scr_code));

                CMsgData_GetData_Int32(prow, "TM", &data.time);
                CMsgData_GetData_Int32(prow, "MKT_TP_CODE", &data.market);
                CMsgData_GetData_Int64(prow, "BGN_ID", &data.bgn_id);
                CMsgData_GetData_Int32(prow, "TRDD_PRC", &data.bgn_price);
                CMsgData_GetData_Int64(prow, "BGN_QTY", &data.bgn_qty);
                CMsgData_GetData_Int64(prow, "BGN_AMT", &data.bgn_amt);

                CMsgData_GetData_String(prow, "NSR_TXN_TP_CODE", (unsigned char *) &data.nsr_txn_tp_code,
                    sizeof(data.nsr_txn_tp_code));
                CMsgData_GetData_String(prow, "BGN_FLG", (unsigned char *) &data.bgn_flg, sizeof(data.bgn_flg));

                // send to client
                if (st_per_bargain_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(data.scr_code) != subscribe_contracts_.end()))
                {
                    st_per_bargain_handler_(&data);
                }
                // save it
                s_p_kmds_per_bgn_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
            }
            else
            {
                MY_LOG_WARN("OnTransaction-CTableData_GetRow failed, return=%d,error_code=%d,error_msg=%s", ret,
                    KmdsUserApi_GetErrCode(pclient),
                    KmdsUserApi_GetErrMsg(pclient));
            }
        }

        CTableData_Release(ptable);
    }
}

void QuoteInterface_KMDS::OnStockOptionMD(MSGDATAHANDLE pmsg)
{
    TABLEDATAHANDLE ptable = NULL;
    MSGDATAHANDLE prow = NULL;
    T_OptionQuote data;

    int ret = CMsgData_GetData_TableData(pmsg, "SEQ_HQ_SNAPSHOT", &ptable);
    if (ret != 0)
    {
        MY_LOG_ERROR("OnStockOptionMD-CMsgData_GetData_TableData failed, ret=%d,error_code=%d,error_msg=%s", ret,
            KmdsUserApi_GetErrCode(pclient),
            KmdsUserApi_GetErrMsg(pclient));

        ptable = NULL;
    }
    if (ptable)
    {
        int nrowcount = 0;
        timeval t;
        gettimeofday(&t, NULL);
        ret = CTableData_GetCount(ptable, (unsigned int *) &nrowcount); /*结果集记录数*/
        if (ret != 0)
        {
            MY_LOG_INFO("OnStockOptionMD-CTableData_GetCount of codetable row_count:%d, return:%d", nrowcount, ret);
            nrowcount = 0;
        }
        for (int i = 0; i < nrowcount; ++i)
        {
            ret = CTableData_GetRow(ptable, i, &prow);
            if (prow)
            {
                memset(&data, 0, sizeof(data));

                CMsgData_GetData_String(prow, "SCR_CODE", (unsigned char *) &data.scr_code, sizeof(data.scr_code));

                CMsgData_GetData_Int64(prow, "TM", &data.time);
                CMsgData_GetData_Int32(prow, "MKT_TP_CODE", &data.market);
                CMsgData_GetData_Int64(prow, "HIGH_PRICE", &data.high_price);
                CMsgData_GetData_Int64(prow, "LWS_PRICE", &data.low_price);
                CMsgData_GetData_Int64(prow, "LTST_PRICE", &data.last_price);
                CMsgData_GetData_Int64(prow, "BGN_TOT_NUM", &data.bgn_total_vol);
                CMsgData_GetData_Int64(prow, "BGN_TOT_AMT", &data.bgn_total_amt);
                CMsgData_GetData_Int64(prow, "AUCTION_PRICE", &data.auction_price);
                CMsgData_GetData_Int64(prow, "AUCTION_QTY", &data.auction_qty);

                CMsgData_GetData_Int64(prow, "TOT_LONG_POSITION", &data.long_position);
                CMsgData_GetData_Int64(prow, "ERROR_MARK", &data.error_mark);
                CMsgData_GetData_Int64(prow, "PUBLISH_TM1", &data.publish_tm1);
                CMsgData_GetData_Int64(prow, "PUBLISH_TM2", &data.publish_tm2);
                CMsgData_GetData_Int64(prow, "PUB_NUM", &data.pub_num);

                CMsgData_GetData_String(prow, "TRAD_PHASE_CODE", (unsigned char *) &data.trad_phase, sizeof(data.trad_phase));
                CMsgData_GetData_String(prow, "MD_REPORT_ID", (unsigned char *) &data.md_report_id, sizeof(data.md_report_id));

                for (int j = 0; j < 5; j++)
                {
                    CMsgData_GetData_Int64(prow, s_stk_sell_prc_field[j], &data.sell_price[j]);
                    CMsgData_GetData_Int64(prow, s_stk_sell_amt_field[j], &data.sell_volume[j]);
                    CMsgData_GetData_Int64(prow, s_stk_bid_prc_field[j], &data.bid_price[j]);
                    CMsgData_GetData_Int64(prow, s_stk_bid_amt_field[j], &data.bid_volume[j]);
                }

                // fillup static fields from code table (20150709)
                T_StockCode sc;
                if (GetCodeInfo(data.market, data.scr_code, &sc))
                {
                    memcpy(data.contract_code, sc.scr_name, sizeof(data.contract_code));
                    data.high_limit_price = sc.high_price;
                    data.low_limit_price = sc.low_price;
                    data.pre_close_price = sc.pre_close;
                    data.open_price = sc.open_price;
                }

                // send to client
                if (st_option_data_handler_
                    && (subscribe_contracts_.empty() || subscribe_contracts_.find(data.scr_code) != subscribe_contracts_.end()))
                {
                    st_option_data_handler_(&data);
                }
                // save it
                s_p_kmds_option_save->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
            }
            else
            {
                MY_LOG_WARN("OnStockOptionMD-CTableData_GetRow failed, return=%d,error_code=%d,error_msg=%s", ret,
                    KmdsUserApi_GetErrCode(pclient),
                    KmdsUserApi_GetErrMsg(pclient));
            }
        }

        CTableData_Release(ptable);
    }
}

static const char * market_code[] =
    { "0", "SH", "SZ", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "SQ", "16", "17", "18", "19", NULL };

TDF_MARKET_DATA_MY QuoteInterface_KMDS::Convert(const T_StockQuote& data)
{
    TDF_MARKET_DATA_MY d;
    memset(&d, 0, sizeof(d));

    std::string sz_wcode(data.scr_code);
    sz_wcode.append(".");
    int market_idx = data.market;
    if (market_idx < 0 || market_idx > 19) market_idx = 0;
    sz_wcode.append(market_code[market_idx]);

    strncpy(d.szWindCode, sz_wcode.c_str(), sizeof(d.szWindCode)); // 32 32
    strncpy(d.szCode, data.scr_code, sizeof(d.szCode)); // 32 32
    //d.nActionDay = data.nActionDay;
    //d.nTradingDay = data.nTradingDay;
    d.nTime = data.time;
    d.nStatus = data.status;
    d.nHigh = data.high_price;
    d.nLow = data.low_price;
    d.nMatch = data.last_price;

    //memcpy(d.nAskPrice, data.sell_price, sizeof(d.nAskPrice)); // 10 10
    for (int i = 0; i < 10; ++i)
        d.nAskPrice[i] = data.sell_price[i];

    //memcpy(d.nAskVol, data.sell_volume, sizeof(d.nAskVol)); // 10 10
    for (int i = 0; i < 10; ++i)
        d.nAskVol[i] = data.sell_volume[i];

    //memcpy(d.nBidPrice, data.bid_price, sizeof(d.nBidPrice)); // 10 10
    for (int i = 0; i < 10; ++i)
        d.nBidPrice[i] = data.bid_price[i];

    //memcpy(d.nBidVol, data.bid_volume, sizeof(d.nBidVol)); // 10 10
    for (int i = 0; i < 10; ++i)
        d.nBidVol[i] = data.bid_volume[i];

    d.nNumTrades = data.bgn_count;
    d.iVolume = data.bgn_total_vol;
    d.iTurnover = data.bgn_total_amt / 10000;
    d.nTotalBidVol = data.total_bid_lot;
    d.nTotalAskVol = data.total_sell_lot;
    d.nWeightedAvgBidPrice = data.wght_avl_bid_price;
    d.nWeightedAvgAskPrice = data.wght_avl_sell_price;
    //d.nIOPV = data.nIOPV;
    //d.nYieldToMaturity = data.nYieldToMaturity;
    //memcpy(d.chPrefix, data.chPrefix, sizeof(d.chPrefix)); // 4 4
    d.nSyl1 = data.pe_ratio_1;
    d.nSyl2 = data.pe_ratio_2;
    d.nSD2 = 0;

    T_StockCode sc;
    if (GetCodeInfo(data.market, data.scr_code, &sc))
    {
        d.nHighLimited = sc.high_price;
        d.nLowLimited = sc.low_price;
        d.nPreClose = sc.pre_close;
        d.nOpen = sc.open_price;
    }

    return d;
}

TDF_INDEX_DATA_MY QuoteInterface_KMDS::Convert(const T_IndexQuote& data)
{
    TDF_INDEX_DATA_MY d;
    memset(&d, 0, sizeof(d));

    std::string sz_wcode(data.scr_code);
    sz_wcode.append(".");
    int market_idx = data.market;
    if (market_idx < 0 || market_idx > 1) market_idx = 0;
    sz_wcode.append(market_code[market_idx]);

    strncpy(d.szWindCode, sz_wcode.c_str(), sizeof(d.szWindCode)); // 32 32
    strncpy(d.szCode, data.scr_code, sizeof(d.szCode)); // 32 32
    //    d.nActionDay = data.nActionDay;
    //    d.nTradingDay = data.nTradingDay;
    d.nTime = data.time;
    d.nHighIndex = data.high_price;
    d.nLowIndex = data.low_price;
    d.nLastIndex = data.last_price;
    d.iTotalVolume = data.bgn_total_vol;
    d.iTurnover = data.bgn_total_amt;

    T_StockCode sc;
    if (GetCodeInfo(data.market, data.scr_code, &sc))
    {
        d.nOpenIndex = sc.open_price;
        d.nPreCloseIndex = sc.pre_close;
    }

    return d;
}

void QuoteInterface_KMDS::AddCodeInfo(int market, const std::string& scr_code, const T_StockCode& stock_code)
{
    std::string ctmp = BuildScrCode(market, scr_code);

    boost::mutex::scoped_lock lock(md_sync_mutex_);
    StockCodeMap::iterator it = stock_code_map_.find(ctmp);
    if (it == stock_code_map_.end())
    {
        stock_code_map_.insert(std::make_pair(ctmp, stock_code));
    }
    else
    {
        if (stock_code.pre_close > 0) it->second.pre_close = stock_code.pre_close;
        if (stock_code.open_price > 0) it->second.open_price = stock_code.open_price;
        if (stock_code.high_price > 0) it->second.high_price = stock_code.high_price;
        if (stock_code.low_price > 0) it->second.low_price = stock_code.low_price;
    }
}

bool QuoteInterface_KMDS::GetCodeInfo(int market, const std::string &scr_code, T_StockCode *stock_code_info)
{
    std::string ctmp = BuildScrCode(market, scr_code);

    boost::mutex::scoped_lock lock(md_sync_mutex_);
    StockCodeMap::const_iterator cit = stock_code_map_.find(ctmp);
    if (cit != stock_code_map_.end())
    {
        memcpy(stock_code_info, &cit->second, sizeof(T_StockCode));
        return true;
    }

    return false;
}

std::string QuoteInterface_KMDS::BuildScrCode(int market, const std::string& scr_code)
{
    char buff[128];
    sprintf(buff, "%d.%s", market, scr_code.c_str());
    return buff;
}
