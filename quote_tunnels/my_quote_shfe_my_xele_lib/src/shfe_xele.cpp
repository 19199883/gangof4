#include "shfe_xele.h"
#include "quote_cmn_utility.h"
#include "ac_xele_md.h"
#include "qtm_with_code.h"

using namespace std;
using namespace my_cmn;

#define ST_NORMAL 0
#define ST_NET_DISCONNECTED -1
#define ST_LOGIN_FAILED -2
#define ST_LOGOUT -3

inline long long GetMicrosFromEpoch()
{
    // get ns(nano seconds) from Unix Epoch
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

QuoteInterface_MY_SHFE_MD::QuoteInterface_MY_SHFE_MD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_shfe_deep_save_(NULL), p_shfe_ex_save_(NULL), p_my_shfe_md_save_(NULL),
        p_xele_handler_(NULL), my_shfe_md_inf_(cfg)
{
	qtm_name_ = "xele_shfe_" + boost::lexical_cast<std::string>(getpid());

    //update_state(qtm_name_.c_str(), TYPE_QUOTE, QtmState::INIT, GetDescriptionWithState(QtmState::INIT).c_str());
    //MY_LOG_INFO("update_state: name: %s, State: %d, Description: %s.", qtm_name_.c_str(), QtmState::INIT, GetDescriptionWithState(QtmState::INIT).c_str());

    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    if (subscribe_contracts_.empty())
    {
        MD_LOG_INFO("MY_SHFE_MD - subscribe all contract");
    }
    else
    {
        for (const std::string &v : subscribe_contracts_)
        {
            MD_LOG_INFO("MY_SHFE_MD - subscribe: %s", v.c_str());
        }
    }

    // initilize xele interface
    if (!AcXeleMDInit())
    {
    	QuoteUpdateState(qtm_name_.c_str(), QtmState::QUOTE_INIT_FAIL);
    	MD_LOG_ERROR("MY_SHFE_MD - Init Xele MD Interface failed!");
    	return;
    }

    // print api version
    MD_LOG_INFO("AcXeleMDVersion: %s", AcXeleMDVersion());

    running_flag_ = true;

    p_shfe_deep_save_ = new QuoteDataSave<SHFEQuote>(cfg_, qtm_name_, "shfe_deep", SHFE_DEEP_QUOTE_TYPE, false);
    p_shfe_ex_save_ = new QuoteDataSave<CDepthMarketDataField>(cfg_, qtm_name_, "quote_level1", SHFE_EX_QUOTE_TYPE, false);
    p_my_shfe_md_save_ = new QuoteDataSave<MYShfeMarketData>(cfg_, qtm_name_, "my_shfe_md", MY_SHFE_MD_QUOTE_TYPE);

    // data manager instance
    my_shfe_md_inf_.SetDataHandler(this);

    // start receive threads
    p_xele_handler_ = new boost::thread(boost::bind(&QuoteInterface_MY_SHFE_MD::XeleDataHandler, this));
}

QuoteInterface_MY_SHFE_MD::~QuoteInterface_MY_SHFE_MD()
{
    // terminate all threads
    running_flag_ = false;
    if (p_xele_handler_)
    {
        p_xele_handler_->interrupt();
    }

    // destroy all save object
    if (p_shfe_deep_save_)
        delete p_shfe_deep_save_;

    if (p_shfe_ex_save_)
        delete p_shfe_ex_save_;

    if (p_my_shfe_md_save_)
        delete p_my_shfe_md_save_;
}

ShfeMDAndState& QuoteInterface_MY_SHFE_MD::GetL1MD(const char* contract)
{
    ContractToMDMap::iterator it = md_buffer_.find(contract);
    if (it == md_buffer_.end())
    {
        it = md_buffer_.insert(std::make_pair(contract, ShfeMDAndState(contract))).first;
    }

    return it->second;
}

void QuoteInterface_MY_SHFE_MD::SendAllValidL1()
{
    int send_cnt = 0;
    for (ContractToMDMap::iterator it = md_buffer_.begin(); it != md_buffer_.end(); ++it)
    {
        if (it->second.is_valid())
        {
            my_shfe_md_inf_.OnDepthMarketData(&it->second.md);
            it->second.reset();
            p_shfe_ex_save_->OnQuoteData(GetMicrosFromEpoch(), &it->second.md);
            ++send_cnt;
        }
    }

    if (send_cnt > 0)
    {
        MD_LOG_DEBUG("send %d level1 data before mbl data", send_cnt);
    }
}

#define US_TIME_OUT_FROM_LAST_MBL (220 * 1000)  // 220 ms (for nagle)
void QuoteInterface_MY_SHFE_MD::XeleDataHandler()
{
    try
    {
        // 获取行情数据共享内存首地址
        const MarketData *md = AcXeleMDData();
        QuoteUpdateState(qtm_name_.c_str(),QtmState::API_READY);

        int last_count = 0;
        int current_count = AcXeleMDCount(); // 当前共享内存中行情数量
        if (current_count > 0)
        {
            SnapshotDataHandler(md, current_count);
            last_count = current_count;
//            // TODO for test 20151218
//            last_count = 176;
        }

        long long prev_data_tp = 0;
        SHFEQuote last_mbl;
        bzero(&last_mbl, sizeof(last_mbl));
        last_mbl.isLast = true;

        bool new_snapshot_flag = false;

        while (running_flag_)
        {
            // 获取当前共享内存中行情的总数，如若比前一次取出的行情总数大，说明有新的行情到来，需要进行相关的业务处理
            current_count = AcXeleMDCount();

            if (current_count > last_count)
            {
                //MD_LOG_DEBUG("recv data - last_count:%d, current_count:%d", last_count, current_count);
                // 行情位置为行情的数量-1，即last_count为第last_count + 1个行情的位置
                for (int i = last_count; i < current_count; ++i)
                {
                    const MarketData* market = md + i;

                    //先判断该行情类型
                    if (strncmp(market->Md, "MD", 2) == 0)
                    {
                        new_snapshot_flag = true;

                        // 一档行情高频
                        LevelOneMarketDataField *ld = (LevelOneMarketDataField*) market->dataItem;
                        ShfeMDAndState& l1md = GetL1MD(ld->InstrumentID);
                        l1md.set_l1(*ld);
                        if (l1md.is_finish())
                        {
                            my_shfe_md_inf_.OnDepthMarketData(&l1md.md);
                            l1md.reset();
                            p_shfe_ex_save_->OnQuoteData(GetMicrosFromEpoch(), &l1md.md);
                        }
                        MD_LOG_DEBUG("recv MD - %s_%s_%03d", ld->InstrumentID, ld->UpdateTime, ld->UpdateMillisec);
                    }
                    else if (strncmp(market->Md, "SM", 2) == 0)
                    {
                        // 一档行情低频
                        LowLevelOneMarketDataField *ld = (LowLevelOneMarketDataField*) market->dataItem;
                        ShfeMDAndState& l1md = GetL1MD(ld->InstrumentID);
                        l1md.set_lowl1(*ld);
                        if (l1md.is_finish())
                        {
                            my_shfe_md_inf_.OnDepthMarketData(&l1md.md);
                            l1md.reset();
                            p_shfe_ex_save_->OnQuoteData(GetMicrosFromEpoch(), &l1md.md);
                        }
                        MD_LOG_DEBUG("recv SM - %s_%s", ld->InstrumentID, ld->UpdateTime);
                    }
                    else if (strncmp(market->Md, "QM", 2) == 0)
                    {
                        // market data order: MD, SM, QM
                        if (new_snapshot_flag)
                        {
                            SendAllValidL1();
                            new_snapshot_flag = false;
                        }

                        // 查询行情
                        QueryMarketDataField *qd = (QueryMarketDataField*) market->dataItem;
                        SHFEQuote mbl;
                        strncpy(mbl.field.InstrumentID, qd->InstrumentID, sizeof(mbl.field.InstrumentID));
                        mbl.field.Direction = qd->Direction;

                        // record time to check timeout
                        prev_data_tp = GetMicrosFromEpoch();

                        if (qd->Volume1 > 0)
                        {
                            mbl.field.Price = qd->Price1;
                            mbl.field.Volume = qd->Volume1;
                            my_shfe_md_inf_.OnMBLData(&mbl.field, false);
                            p_shfe_deep_save_->OnQuoteData(prev_data_tp, &mbl);
                        }
                        if (qd->Volume2 > 0)
                        {
                            mbl.field.Price = qd->Price2;
                            mbl.field.Volume = qd->Volume2;
                            my_shfe_md_inf_.OnMBLData(&mbl.field, false);
                            p_shfe_deep_save_->OnQuoteData(prev_data_tp, &mbl);
                        }
                        if (qd->Volume3 > 0)
                        {
                            mbl.field.Price = qd->Price3;
                            mbl.field.Volume = qd->Volume3;
                            my_shfe_md_inf_.OnMBLData(&mbl.field, false);
                            p_shfe_deep_save_->OnQuoteData(prev_data_tp, &mbl);
                        }
                        if (qd->Volume4 > 0)
                        {
                            mbl.field.Price = qd->Price4;
                            mbl.field.Volume = qd->Volume4;
                            my_shfe_md_inf_.OnMBLData(&mbl.field, false);
                            p_shfe_deep_save_->OnQuoteData(prev_data_tp, &mbl);
                        }
                        if (qd->Volume5 > 0)
                        {
                            mbl.field.Price = qd->Price5;
                            mbl.field.Volume = qd->Volume5;
                            my_shfe_md_inf_.OnMBLData(&mbl.field, false);
                            p_shfe_deep_save_->OnQuoteData(prev_data_tp, &mbl);
                        }
                    }
                    else
                    {
                        MD_LOG_WARN("Xele receive unknown type md: %c%c.", market->Md[0], market->Md[1]);
                    }
                }

                last_count = current_count;
            }
            else if (current_count < last_count)
            {
                // 保证在ftd_md重启后仍能够正常接受最新的数据
                last_count = current_count;
                MD_LOG_WARN("recv data(ftd_md restart) - last_count:%d, current_count:%d", last_count, current_count);
            }
            else
            {
                // check timeout
                if (prev_data_tp > 0)
                {
                    long long cur_tp = GetMicrosFromEpoch();
                    if ((cur_tp - prev_data_tp) > US_TIME_OUT_FROM_LAST_MBL)
                    {
                        prev_data_tp = 0;
                        my_shfe_md_inf_.OnMBLData(&last_mbl.field, true);
                        p_shfe_deep_save_->OnQuoteData(cur_tp, &last_mbl);
                        MD_LOG_WARN("time out to send last flag");
                    }
                }
            }
        }
    }
    catch (...)
    {
        MD_LOG_FATAL("Unknown exception in XeleDataHandler.");
    }

    MD_LOG_INFO("thread of XeleDataHandler is going to exit.");
    QuoteUpdateState(qtm_name_.c_str(), QtmState::DISCONNECT);
}

void QuoteInterface_MY_SHFE_MD::OnMYShfeMDData(MYShfeMarketData *data)
{
    long long cur_tp = GetMicrosFromEpoch();
    if (my_shfe_md_handler_
        && (subscribe_contracts_.empty()
            || subscribe_contracts_.find(data->InstrumentID) != subscribe_contracts_.end()))
    {
        my_shfe_md_handler_(data);
    }
    p_my_shfe_md_save_->OnQuoteData(cur_tp, data);
}

void QuoteInterface_MY_SHFE_MD::SnapshotDataHandler(const MarketData * md, int current_count)
{
    MD_LOG_INFO("handle snapshot start - current_count:%d", current_count);
    int i = 0;
    for (; i < current_count; ++i)
    {
        const MarketData* market = md + i;
        if (market->Md[0] == 'K')
        {
            // 一档行情低频
            QuickStartMarketDataField *ld = (QuickStartMarketDataField*) market->dataItem;
            ShfeMDAndState& l1md = GetL1MD(ld->InstrumentID);
            l1md.set_snapshot_l1(*ld);

            MD_LOG_DEBUG("count:%d - %s - %s_%s,op:%.4f,hp:%.4f,lp:%.4f,cp:%.4f,ulp:%.4f,llp:%.4f,sp:%.4f,delta:%.4f",
                i, market->Md,
                ld->InstrumentID,
                ld->UpdateTime,
                InvalidToZeroD(ld->OpenPrice),
                InvalidToZeroD(ld->HighestPrice),
                InvalidToZeroD(ld->LowestPrice),
                InvalidToZeroD(ld->ClosePrice),
                InvalidToZeroD(ld->UpperLimitPrice),
                InvalidToZeroD(ld->LowerLimitPrice),
                InvalidToZeroD(ld->SettlementPrice),
                InvalidToZeroD(ld->CurrDelta));
        }

        // end of snapshot
        if (market->Md[0] == 'E')
        {
            break;
        }
    }
    MD_LOG_INFO("handle snapshot finished - snapshot_count:%d", i);

    // output another 2000000 data items
    int count = 0;
    int max_count = 2000000;
    for (; i < current_count; ++i)
    {
        const MarketData* market = md + i;
        if (market->Md[0] == 'K' || market->Md[0] == 'E')
        {
            QuickStartMarketDataField *ld = (QuickStartMarketDataField*) market->dataItem;
            MD_LOG_DEBUG("count:%d - %s - %s_%s,op:%.4f,hp:%.4f,lp:%.4f,cp:%.4f,ulp:%.4f,llp:%.4f,sp:%.4f,delta:%.4f",
                i, std::string(market->Md, 2).c_str(),
                ld->InstrumentID,
                ld->UpdateTime,
                InvalidToZeroD(ld->OpenPrice),
                InvalidToZeroD(ld->HighestPrice),
                InvalidToZeroD(ld->LowestPrice),
                InvalidToZeroD(ld->ClosePrice),
                InvalidToZeroD(ld->UpperLimitPrice),
                InvalidToZeroD(ld->LowerLimitPrice),
                InvalidToZeroD(ld->SettlementPrice),
                InvalidToZeroD(ld->CurrDelta));
        }
        else if (strncmp(market->Md, "MD", 2) == 0)
        {
            // 一档行情高频
            LevelOneMarketDataField *ld = (LevelOneMarketDataField*) market->dataItem;
            MD_LOG_DEBUG("count:%d - %s - %s_%s_%03d,vol:%d,lp:%.4f,Turnover:%.4f,OpenInterest:%d,bp:%.2f,ap:%.2f,bv:%d,av:%d",
                i, std::string(market->Md, 2).c_str(),
                ld->InstrumentID,
                ld->UpdateTime,
                ld->UpdateMillisec,
                ld->Volume,
                InvalidToZeroD(ld->LastPrice),
                ld->Turnover,
                ld->OpenInterest,
                InvalidToZeroD(ld->BidPrice),
                InvalidToZeroD(ld->AskPrice),
                ld->BidVolume,
                ld->AskVolume);
        }
        else if (strncmp(market->Md, "SM", 2) == 0)
        {
            // 一档行情低频
            LowLevelOneMarketDataField *ld = (LowLevelOneMarketDataField*) market->dataItem;
            MD_LOG_DEBUG("count:%d - %s - %s_%s,op:%.4f,hp:%.4f,lp:%.4f,cp:%.4f,ulp:%.4f,llp:%.4f,sp:%.4f,delta:%.4f",
                i, std::string(market->Md, 2).c_str(),
                ld->InstrumentID,
                ld->UpdateTime,
                ld->OpenPrice,
                ld->HighestPrice,
                ld->LowestPrice,
                ld->ClosePrice,
                ld->UpperLimitPrice,
                ld->LowerLimitPrice,
                ld->SettlementPrice,
                ld->CurrDelta);
        }
        else if (strncmp(market->Md, "QM", 2) == 0)
        {
            // 查询行情
            QueryMarketDataField *qd = (QueryMarketDataField*) market->dataItem;
            MD_LOG_DEBUG("count:%d - %s - %s_%s_%03d,dir:%c,v1:%d,p1:%.2f,v2:%d,p2:%.2f,v3:%d,p3:%.2f,v4:%d,p4:%.2f,v5:%d,p5:%.2f",
                i, std::string(market->Md, 2).c_str(),
                qd->InstrumentID,
                qd->UpdateTime,
                qd->UpdateMillisec,
                qd->Direction,
                qd->Volume1,
                qd->Price1,
                qd->Volume2,
                qd->Price2,
                qd->Volume3,
                qd->Price3,
                qd->Volume4,
                qd->Price4,
                qd->Volume5,
                qd->Price5);
        }

        ++count;
        if (count > max_count)
        {
            break;
        }
    }
}
