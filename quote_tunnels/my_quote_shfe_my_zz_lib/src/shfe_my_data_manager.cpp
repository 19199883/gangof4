#include "shfe_my_data_manager.h"

#include <algorithm>
#include "quote_cmn_utility.h"

#define SHFE_MD_CODE_COUNT_INIT 200
#define SHFE_PRICE_COUNT_INIT   600

static SHFEMDQuoteSnapshot ** AllocateSnapshotBuffer(int code_count, int price_pos_count)
{
    SHFEMDQuoteSnapshot ** pp = new SHFEMDQuoteSnapshot *[code_count];
    for (int i = 0; i < code_count; ++i)
    {
        pp[i] = new SHFEMDQuoteSnapshot();
        pp[i]->buy_count = 0;
        pp[i]->buy_price = new double[price_pos_count];
        pp[i]->buy_volume = new int[price_pos_count];
        pp[i]->sell_count = 0;
        pp[i]->sell_price = new double[price_pos_count];
        pp[i]->sell_volume = new int[price_pos_count];
    }

    return pp;
}

static void CopySnapshotBuffer(SHFEMDQuoteSnapshot **enlarged_buffer, SHFEMDQuoteSnapshot ** p_old,
    int code_count_old, int price_pos_count_old)
{
    for (int i = 0; i < code_count_old; ++i)
    {
        enlarged_buffer[i]->buy_count = p_old[i]->buy_count;
        memcpy(enlarged_buffer[i]->buy_price, p_old[i]->buy_price, price_pos_count_old * sizeof(double));
        memcpy(enlarged_buffer[i]->buy_volume, p_old[i]->buy_volume, price_pos_count_old * sizeof(int));
        enlarged_buffer[i]->sell_count = p_old[i]->sell_count;
        memcpy(enlarged_buffer[i]->sell_price, p_old[i]->sell_price, price_pos_count_old * sizeof(double));
        memcpy(enlarged_buffer[i]->sell_volume, p_old[i]->sell_volume, price_pos_count_old * sizeof(int));
    }
}

MYShfeMDManager::MYShfeMDManager(const ConfigData &cfg)
    : cfg_(cfg)
{
    buy_first_idx_ = 0;
    data_handler_ = NULL;

    // allocate buffer
    code_count_max = SHFE_MD_CODE_COUNT_INIT;
    price_position_count_max = SHFE_PRICE_COUNT_INIT;
    p_quote_buffer_ = AllocateSnapshotBuffer(code_count_max, price_position_count_max);
    last_code_index = 0;

    //data_in_.reserve(5000);
    data_handle_.reserve(5000);

    prev_dir_ = SHFE_FTDC_D_Buy;
    cur_dir_ = SHFE_FTDC_D_Buy;

}

MYShfeMDManager::~MYShfeMDManager()
{
}

void MYShfeMDManager::OnMBLData(const CShfeFtdcMBLMarketDataField* const pdata, bool last_flag)
{
    if (pdata)
    {
        data_handle_.push_back(SHFEQuote(*pdata, last_flag));
    }
    else if (last_flag)
    {
        data_handle_.push_back(SHFEQuote(true));
    }
}

inline void ResetSnapshot(SHFEMDQuoteSnapshot* pd)
{
    pd->buy_count = 0;
    pd->sell_count = 0;
}

void MYShfeMDManager::ProcessData(MYShfeMDManager* p_mngr)
{
    // 数据交换
    /*
    if (!p_mngr->data_in_.empty())
    {
        p_mngr->data_in_.swap(p_mngr->data_handle_);
    }
    */
    if (p_mngr->data_handle_.empty())
    {
        return;
    }

    for (MBLDataCollection::iterator p = p_mngr->data_handle_.begin(); p != p_mngr->data_handle_.end(); ++p)
    {
        if (p->isLast)
        {
            // snapshot is completed, should send to client
            if (p->field.Volume > 0)
            {
                // valid data
                cur_code_ = p->field.InstrumentID;
                (void) p_mngr->PushDataToBuffer(cur_code_, p);
            }

            // 帧结束了，将没有发送的合约数据全部发出
            bool send_cur_flag = false;
            while (p_mngr->GetLeftCode(prev_code_))
            {
                SHFEMDQuoteSnapshot * p_snapshot_prev = p_mngr->GetDataCache(prev_code_);
                p_mngr->SendToClient(prev_code_, p_snapshot_prev);
                if (!send_cur_flag && cur_code_ == prev_code_)
                {
                    send_cur_flag = true;
                }
            }
            if (!send_cur_flag)
            {
                SHFEMDQuoteSnapshot * p_snapshot = p_mngr->GetDataCache(cur_code_);
                p_mngr->SendToClient(cur_code_, p_snapshot);
            }

            // 重置数据
            p_mngr->ResetBuyCode();

            continue;
        }

        if (p->field.Volume > 0)
        {
            // 有效数据
            cur_code_ = p->field.InstrumentID;
            cur_dir_ = p->field.Direction;

            // 数据记录
            (void) p_mngr->PushDataToBuffer(cur_code_, p);

            // 买方向合约记录
            if (cur_dir_ == SHFE_FTDC_D_Buy)
            {
                p_mngr->PushNewBuyDirCode(cur_code_);
            }

            // 第一个卖方向数据，核对前方是否有涨停合约，该类合约无卖方向数据，应该发出
            else if (prev_dir_ == SHFE_FTDC_D_Buy && cur_dir_ == SHFE_FTDC_D_Sell)
            {
                while (p_mngr->GetPrevCode(cur_code_, prev_code_))
                {
                    SHFEMDQuoteSnapshot * p_snapshot_prev = p_mngr->GetDataCache(prev_code_);
                    p_mngr->SendToClient(prev_code_, p_snapshot_prev);
                }
            }

            // 卖方向中间的合约切换，将之前的合约数据发出
            else if (prev_dir_ == SHFE_FTDC_D_Sell && p->field.Direction == SHFE_FTDC_D_Sell && prev_code_ != cur_code_)
            {
                SHFEMDQuoteSnapshot * p_snapshot_prev = p_mngr->GetDataCache(prev_code_);
                p_mngr->SendToClient(prev_code_, p_snapshot_prev);

                while (p_mngr->GetPrevCode(cur_code_, prev_code_))
                {
                    SHFEMDQuoteSnapshot * p_snapshot_prev = p_mngr->GetDataCache(prev_code_);
                    p_mngr->SendToClient(prev_code_, p_snapshot_prev);
                }
            }

            prev_code_ = cur_code_;
            prev_dir_ = cur_dir_;
            continue;
        }
        else
        {
            // invalid data, no need to handle, shouldn't reach here
            if (!p->isLast) {
                MY_LOG_WARN("invalid data without last flag.");
            }
        }
    }
    p_mngr->data_handle_.clear();
}

SHFEMDQuoteSnapshot* MYShfeMDManager::GetDataCache(const std::string& code)
{
    CodeIndex::iterator it = code_index_.find(code);
    if (it == code_index_.end())
    {
        // assign new index
        int new_index = last_code_index++;

        // enlarge code count
        if (new_index >= code_count_max)
        {
            MY_LOG_WARN("code count beyond: %d", code_count_max);

            SHFEMDQuoteSnapshot ** p_old = p_quote_buffer_;
            int code_count_old = code_count_max;

            // allocate new space
            code_count_max *= 2;
            p_quote_buffer_ = AllocateSnapshotBuffer(code_count_max, price_position_count_max);

            // copy old data
            CopySnapshotBuffer(p_quote_buffer_, p_old, code_count_old, price_position_count_max);

            // clear old data
            for (int i = 0; i < code_count_old; ++i)
            {
                delete[] p_old[i]->buy_price;
                delete[] p_old[i]->buy_volume;
                delete[] p_old[i]->sell_price;
                delete[] p_old[i]->sell_volume;
            }
            delete[] p_old;
        }

        it = code_index_.insert(std::make_pair(code, new_index)).first;
    }

    if (p_quote_buffer_[it->second]->buy_count >= price_position_count_max
        || p_quote_buffer_[it->second]->sell_count >= price_position_count_max)
    {
        MY_LOG_WARN("price position count beyond: %d", price_position_count_max);

        // enlarge price position size
        SHFEMDQuoteSnapshot ** p_old = p_quote_buffer_;
        int price_pos_count_old = price_position_count_max;

        // allocate new space
        price_position_count_max *= 2;
        p_quote_buffer_ = AllocateSnapshotBuffer(code_count_max, price_position_count_max);

        // copy old data
        CopySnapshotBuffer(p_quote_buffer_, p_old, code_count_max, price_pos_count_old);

        // clear old data
        for (int i = 0; i < code_count_max; ++i)
        {
            delete[] p_old[i]->buy_price;
            delete[] p_old[i]->buy_volume;
            delete[] p_old[i]->sell_price;
            delete[] p_old[i]->sell_volume;
        }
        delete[] p_old;
    }

    return p_quote_buffer_[it->second];
}

SHFEMDQuoteSnapshot *MYShfeMDManager::PushDataToBuffer(const std::string &cur_code, MBLDataCollection::iterator &p)
{
    SHFEMDQuoteSnapshot * p_data = GetDataCache(cur_code);
    if (!p_data)
    {
        return p_data;
    }

    if (p->field.Direction == SHFE_FTDC_D_Buy)
    {
        p_data->buy_price[p_data->buy_count] = p->field.Price;
        p_data->buy_volume[p_data->buy_count] = p->field.Volume;
        ++p_data->buy_count;
    }
    else
    {
        p_data->sell_price[p_data->sell_count] = p->field.Price;
        p_data->sell_volume[p_data->sell_count] = p->field.Volume;
        ++p_data->sell_count;
    }

    return p_data;
}

static void FillStatisticFields(MYShfeMarketData &des_data, SHFEMDQuoteSnapshot * const src_data)
{
    des_data.buy_total_volume = 0;
    des_data.buy_weighted_avg_price = 0;
    double sum_pv = 0;
    for (int i = 0; i < src_data->buy_count; ++i)
    {
        des_data.buy_total_volume += src_data->buy_volume[i];
        sum_pv += (src_data->buy_volume[i] * src_data->buy_price[i]);
    }
    if (des_data.buy_total_volume > 0)
    {
        des_data.buy_weighted_avg_price = sum_pv / des_data.buy_total_volume;
    }

    des_data.sell_total_volume = 0;
    des_data.sell_weighted_avg_price = 0;
    sum_pv = 0;
    for (int i = 0; i < src_data->sell_count; ++i)
    {
        des_data.sell_total_volume += src_data->sell_volume[i];
        sum_pv += (src_data->sell_volume[i] * src_data->sell_price[i]);
    }
    if (des_data.sell_total_volume > 0)
    {
        des_data.sell_weighted_avg_price = sum_pv / des_data.sell_total_volume;
    }
}

void MYShfeMDManager::SendToClient(const std::string &code, SHFEMDQuoteSnapshot * const p_data)
{
    // prepare my data
    MYShfeMarketData my_data;
    memcpy(my_data.InstrumentID, code.c_str(), code.size() + 1);
    my_data.data_flag = 2;
    {
        DepthDataQueueOfCode::iterator it = data_depth_.find(code);
        if (it != data_depth_.end())
        {
            for (DepthDataQueue::const_iterator d_cit = it->second.begin(); d_cit != it->second.end(); ++d_cit)
            {
                memcpy(&my_data, &(*d_cit), sizeof(CDepthMarketDataField));

                if (d_cit + 1 != it->second.end())
                {
                    // not last element
                    my_data.data_flag = 1;
                    // 发给数据客户
                    if (data_handler_)
                    {
                        data_handler_->OnMYShfeMDData(&my_data);
                    }
                }
                else
                {
                    // last element, merged with mbl data
                    my_data.data_flag = 3;
                }
            }
            // clear all
            it->second.clear();
        }
    }
    memcpy(my_data.buy_price, p_data->buy_price, std::min(MY_SHFE_QUOTE_PRICE_POS_COUNT, p_data->buy_count) * sizeof(double));
    memcpy(my_data.buy_volume, p_data->buy_volume, std::min(MY_SHFE_QUOTE_PRICE_POS_COUNT, p_data->buy_count) * sizeof(int));
    memcpy(my_data.sell_price, p_data->sell_price, std::min(MY_SHFE_QUOTE_PRICE_POS_COUNT, p_data->sell_count) * sizeof(double));
    memcpy(my_data.sell_volume, p_data->sell_volume, std::min(MY_SHFE_QUOTE_PRICE_POS_COUNT, p_data->sell_count) * sizeof(int));

    FillStatisticFields(my_data, p_data);

    // 发给数据客户
    if (data_handler_)
    {
        data_handler_->OnMYShfeMDData(&my_data);
    }

    //MY_LOG_DEBUG("snapshot, code: %s, buy_count: %d, sell_count: %d", code.c_str(), p_data->buy_count, p_data->sell_count);

    // reset
    ResetSnapshot(p_data);
    PopFirstCode(code);
}

void MYShfeMDManager::OnDepthMarketData(const CDepthMarketDataField * const pdata)
{
    if (pdata)
    {
        DepthDataQueueOfCode::iterator it = data_depth_.find(pdata->InstrumentID);
        if (it == data_depth_.end())
        {
            it = data_depth_.insert(std::make_pair(pdata->InstrumentID, DepthDataQueue())).first;
        }

        it->second.push_back(*pdata);
    }
}
