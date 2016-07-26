/* Copyright (C) 2013 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#include "EPosixClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "Contract.h"
#include "Order.h"

#include <stdio.h>
#include <mutex>
#include <thread>
#include <boost/algorithm/string.hpp>
#include "quote_ib.h"

///////////////////////////////////////////////////////////
// member funcs
MYIBDataHandler::MYIBDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData& cfg, const std::string id)
    : api_(new EPosixClientSocket(this)), client_id_(0), quote_addr_(0), ip_(id), contract_(NULL), cfg_(cfg), depth_handler_(NULL), tick_handler_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_user_ = *subscribe_contracts;
    }

    /* 初始化 */
    reconnected_ = false;
    count_total_ = 0;
    count_of_l2_ = 0;

    sprintf(qtm_name_, "ib_%s_%s_%u", cfg.Logon_config().account.c_str(), cfg.Logon_config().broker_id.c_str(), getpid());

    if (!ParseConfig())
    {
        MY_LOG_ERROR("IB - Parse config failed.");
        return;
    }

    p_save_depth_ = new QuoteDataSave<IBDepth>(cfg, qtm_name_, "ib_depth_quote", IB_QUOTE_TYPE, true);
    p_save_tick_ = new QuoteDataSave<IBTick>(cfg, qtm_name_, "ib_tick_quote", IB_QUOTE_TICK_TYPE, false);

    QuoteUpdateState(qtm_name_, QtmState::INIT);
    //PlayBackFile();
    //test();
    std::thread listen(std::bind(&MYIBDataHandler::connect, this, 0));
    listen.detach();
}

MYIBDataHandler::~MYIBDataHandler()
{
    api_->eDisconnect();
    MY_LOG_INFO("IB - disconnected");
    QuoteUpdateState(qtm_name_, QtmState::DISCONNECT);

    std::lock_guard<std::mutex> lock(ids_sync_);

    if (quote_addr_)
    {
        delete[] quote_addr_;
    }
    if (contract_)
    {
        delete[] contract_;
    }

    if (p_save_depth_)
    {
        delete p_save_depth_;
    }
    if (p_save_tick_)
    {
        delete p_save_tick_;
    }
}

bool MYIBDataHandler::ParseConfig()
{
    if (cfg_.Logon_config().quote_provider_addrs.empty())
    {
        MY_LOG_ERROR("%s: IB - no configure quote provider address", quote_addr_);
        return false;
    }
    std::vector<std::string> fields;
    boost::split(fields, ip_, boost::is_any_of(","));
    quote_addr_ = new char[fields[0].length() + 1];
    strcpy(quote_addr_, fields[0].c_str());
    id_ = fields[1];
    host_ = strtok(quote_addr_, ":");
    port_ = atoi(strtok(NULL, ":"));
    client_id_ = atoi(cfg_.Logon_config().broker_id.c_str());
    subscribe_contracts_ = cfg_.Subscribe_datas();

    if (host_.empty())
    {
        MY_LOG_ERROR("%s: IB - no configure quote provider address", quote_addr_);
        return false;
    }

    return true;
}

void MYIBDataHandler::reqMktDepth()
{
    QuoteUpdateState(qtm_name_, QtmState::API_READY);
    std::vector<std::string> fields;
    std::vector<Contract> depth;
    std::vector<Contract> level1;

    Contract c;
    IBMktDataDepth item;

    for (auto it = subscribe_contracts_.begin(); it != subscribe_contracts_.end(); it++)
    {
        boost::split(fields, *it, boost::is_any_of(","));
        if (fields.size() < 8)
        {
            std::string err_msg;
            err_msg = "Can not read the config item: " + *it;
            update_state(qtm_name_, TYPE_QUOTE, QtmState::QUOTE_INIT_FAIL, err_msg.c_str());
            MY_LOG_ERROR(err_msg.c_str());
            continue;
        }
        if (fields[7] == id_)
        {
            c.localSymbol = fields[0];
            c.symbol = fields[1];
            c.secType = fields[2];
            c.exchange = fields[3];
            c.expiry = fields[4];
            c.currency = fields[5];

            if ("0" == fields[6])
            {
                depth.push_back(c);
            }
            else if ("1" == fields[6])
            {
                level1.push_back(c);
            }
        }
    }

    count_of_l2_ = depth.size();
    count_total_ = count_of_l2_ + level1.size();
    contract_ = new IBMktDataDepth[count_total_];

    for (int i = 0; i < count_of_l2_; i++)
    {
        item.Symbol = depth[i].localSymbol;
        item.Exchange = depth[i].exchange;
        item.Currency = depth[i].currency;
        contract_[i] = item;
        TagValueListSPtr opt;
        IBString genericTick = "100,101,221,233";
        depth[i].localSymbol = "";
        std::lock_guard<std::mutex> lock(ids_sync_);

        api_->reqMktDepth(i, depth[i], 20, opt);
        api_->reqMktData(i, depth[i], genericTick, 0, opt);
    }
    for (int i = count_of_l2_; i < count_total_; i++)
    {
        int j = i - count_of_l2_;
        item.Symbol = level1[j].localSymbol;
        item.Exchange = level1[j].exchange;
        item.Currency = level1[j].currency;
        contract_[i] = item;
        TagValueListSPtr opt;
        IBString genericTick = "100,101,221,233";
        level1[j].localSymbol = "";
        std::lock_guard<std::mutex> lock(ids_sync_);

        api_->reqMktData(i, level1[j], genericTick, 0, opt);
    }

    MY_LOG_INFO("%s: IB - reqMktDepth success", quote_addr_);
}

void MYIBDataHandler::connect(int wait_seconds)
{
    while (true)
    {
        if (wait_seconds > 0)
        {
            sleep(wait_seconds);
        }

        while (api_->isConnected() == false)
        {
            api_->eConnect(host_.c_str(), port_, client_id_);
            MY_LOG_ERROR("%s: IB - connect failed", quote_addr_);
            sleep(1);
        }

        MY_LOG_INFO("%s: IB - connect success", quote_addr_);
        QuoteUpdateState(qtm_name_, QtmState::CONNECT_SUCCESS);

        if (reconnected_ == false)
        {
            reqMktDepth();
        }

        while (api_->isConnected())
        {
            api_->checkMessages();
        }
        MY_LOG_INFO("%s: IB - connect interrupt", quote_addr_);
        QuoteUpdateState(qtm_name_, QtmState::DISCONNECT);

        reconnected_ = true;
    }
}

void MYIBDataHandler::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute)
{
    //MY_LOG_INFO("%s: tickPrice: tickerId: %d, field: %d, price: %f, canAutoExecute: %d", quote_addr_, tickerId, field, price, canAutoExecute);
    std::lock_guard<std::mutex> lock(ids_sync_);
    if (tickerId < count_total_ && tickerId >= 0)
    {
        switch (field)
        {
            case BID:
                if (tickerId >= count_of_l2_)
                {
                    if (contract_[tickerId].BidLevel.empty())
                    {
                        contract_[tickerId].BidLevel.push_back(PriceLevel(price, 0));
                    }
                    else
                    {
                        contract_[tickerId].BidLevel[0].Price = price;
                    }
                }
                else
                {
                    return;
                }
                break;
            case ASK:
                if (tickerId >= count_of_l2_)
                {
                    if (contract_[tickerId].AskLevel.empty())
                    {
                        contract_[tickerId].AskLevel.push_back(PriceLevel(price, 0));
                    }
                    else
                    {
                        contract_[tickerId].AskLevel[0].Price = price;
                    }
                }
                else
                {
                    return;
                }
                break;
            case LAST:
                contract_[tickerId].LastPrice = price;
                break;
            case HIGH:
                contract_[tickerId].HighPrice = price;
                break;
            case LOW:
                contract_[tickerId].LowPrice = price;
                break;
            case CLOSE:
                contract_[tickerId].LastClosePrice = price;
                break;
            case OPEN:
                contract_[tickerId].OpenPrice = price;
                break;
            default:
                return;
        }

        timeval t;
        gettimeofday(&t, NULL);

        IBDepth d;
        Convert(d, contract_[tickerId]);

        if (subscribe_contracts_user_.empty() || subscribe_contracts_user_.count(d.Symbol))
        {
            if (depth_handler_)
            {
                depth_handler_(&d);
            }
        }

        p_save_depth_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &d);

    }
}

void MYIBDataHandler::tickSize(TickerId tickerId, TickType field, int size)
{
    //MY_LOG_INFO("%s: tickSize: id: %d, field: %d, size: %d", quote_addr_, tickerId, field, size);
    std::lock_guard<std::mutex> lock(ids_sync_);
    if (tickerId < count_total_ && tickerId >= 0)
    {
        switch (field)
        {
            case BID_SIZE:
                if (tickerId >= count_of_l2_)
                {
                    if (contract_[tickerId].BidLevel.empty())
                    {
                        contract_[tickerId].BidLevel.push_back(PriceLevel(0, size));
                    }
                    else
                    {
                        contract_[tickerId].BidLevel[0].Vol = size;
                    }
                }
                else
                {
                    return;
                }
                break;
            case ASK_SIZE:
                if (tickerId >= count_of_l2_)
                {
                    if (contract_[tickerId].AskLevel.empty())
                    {
                        contract_[tickerId].AskLevel.push_back(PriceLevel(0, size));
                    }
                    else
                    {
                        contract_[tickerId].AskLevel[0].Vol = size;
                    }
                }
                else
                {
                    return;
                }
                break;
            case LAST_SIZE:
                contract_[tickerId].LastSize = size;
                break;
            case VOLUME:
                contract_[tickerId].TotalVolume = size;
                break;
            default:
                return;
        }

        timeval t;
        gettimeofday(&t, NULL);

        IBDepth d;
        Convert(d, contract_[tickerId]);

        if (subscribe_contracts_user_.empty() || subscribe_contracts_user_.count(d.Symbol))
        {
            if (depth_handler_)
            {
                depth_handler_(&d);
            }
        }

        p_save_depth_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &d);

        if (field == LAST_SIZE)
        {
            contract_[tickerId].LastSize = 0;
        }
    }
}

void MYIBDataHandler::updateMktDepth(TickerId id, int position, int operation, int side, double price, int size)
{
    //MY_LOG_INFO("%s: updateMktDepth: id: %d, position: %d, operation: %d, side: %d, price: %f, size: %d.", quote_addr_, id, position, operation, side,
    //price, size);
    std::lock_guard<std::mutex> lock(ids_sync_);
    if (id <= count_total_ && id >= 0)
    {
        int step = 0;
        switch (operation)
        {
            case IB_ADD:
                case IB_UPDATE:
                if (side == (int) IB_BID)
                {
                    if (((int) contract_[id].BidLevel.size() > position) && (contract_[id].BidLevel[position].Price == price)
                        && (contract_[id].BidLevel[position].Vol == size))
                    {
                        //MY_LOG_DEBUG("call updateMktDepth() with update the same value.");
                        return;
                    }
                    while (contract_[id].AskLevel.empty() == false && (price >= contract_[id].AskLevel[0].Price))
                    {
                        contract_[id].AskLevel.erase(contract_[id].AskLevel.begin());
                        MY_LOG_INFO("AskLevel erase position 0.");
                    }
                    while (((int) contract_[id].BidLevel.size() > (position + 1)) && (contract_[id].BidLevel[position + 1].Price >= price))
                    {
                        contract_[id].BidLevel.erase(contract_[id].BidLevel.begin() + position + 1);
                        MY_LOG_INFO("BidLevel erase position %d.", position + 1);
                    }
                    if (position >= (int) contract_[id].BidLevel.size())
                    {
                        step = position - contract_[id].BidLevel.size();
                    }
                    while ((position - step > 0) && (contract_[id].BidLevel[position - step - 1].Price <= price))
                    {
                        contract_[id].BidLevel.erase(contract_[id].BidLevel.begin() + position - step - 1);
                        MY_LOG_INFO("BidLevel erase position %d.", position - step - 1);
                        step++;
                    }
                }
                if (side == (int) IB_ASK)
                {
                    if (((int) contract_[id].AskLevel.size() > position) && (contract_[id].AskLevel[position].Price == price)
                        && (contract_[id].AskLevel[position].Vol == size))
                    {
                        //MY_LOG_DEBUG("call updateMktDepth() with update the same value.");
                        return;
                    }
                    while (contract_[id].BidLevel.empty() == false && (price <= contract_[id].BidLevel[0].Price))
                    {
                        contract_[id].BidLevel.erase(contract_[id].BidLevel.begin());
                        MY_LOG_INFO("BidLevel erase position 0.");
                    }
                    while (((int) contract_[id].AskLevel.size() > (position + 1)) && (contract_[id].AskLevel[position + 1].Price <= price))
                    {
                        contract_[id].AskLevel.erase(contract_[id].AskLevel.begin() + position + 1);
                        MY_LOG_INFO("AskLevel erase position %d.", position + 1);
                    }
                    if (position >= (int) contract_[id].AskLevel.size())
                    {
                        step = position - contract_[id].AskLevel.size();
                    }
                    while ((position - step > 0) && (contract_[id].AskLevel[position - step - 1].Price >= price))
                    {
                        contract_[id].AskLevel.erase(contract_[id].AskLevel.begin() + position - step - 1);
                        MY_LOG_INFO("AskLevel erase position %d.", position - step - 1);
                        step++;
                    }
                }
                contract_[id].Update(side, position - step, price, size);
                break;
            case IB_DELETE:
                contract_[id].Delete(side, position, price, size);
                break;
            default:
                MY_LOG_WARN("call updateMktDepth() with wrong side: %d, id: %d, position: %d, operation: %d, price: %f, size: %d", size, id, position,
                    operation, price, size);
                return;
                break;
        }

        timeval t;
        gettimeofday(&t, NULL);

        IBDepth d;
        IBTick tick(contract_[id].Symbol, position, operation, side, price, size);
        Convert(d, contract_[id]);
        if (subscribe_contracts_user_.empty() || subscribe_contracts_user_.count(d.Symbol))
        {
            if (depth_handler_)
            {
                depth_handler_(&d);
            }

            if (tick_handler_)
            {
                tick_handler_(&tick);
            }
        }

        p_save_depth_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &d);
        p_save_tick_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &tick);

#if 0
        for(int j = (int)contract_[id].AskLevel.size(); j >= 0; j--)
        {
            MY_LOG_INFO("Ask %d: Price: %f, Vol: %d", j, contract_[id].AskLevel[j].Price, depth_[id].AskLevel[j].Vol);
        }
        for(int j = 0; j < (int)contract_[id].BidLevel.size(); j++)
        {
            MY_LOG_INFO("Bid %d: Price: %f, Vol: %d", j, contract_[id].BidLevel[j].Price, depth_[id].BidLevel[j].Vol);
        }
#endif
    }
}

void MYIBDataHandler::error(const int id, const int errorCode, const IBString errorString)
{
    MY_LOG_ERROR("%s: error: id: %d, errorCode:%d, errorString: %s", quote_addr_, id, errorCode, errorString.c_str());
    if (317 == errorCode)
    {
        if (0 <= id && id <= count_total_)
        {
            contract_[id].AskLevel.clear();
            contract_[id].BidLevel.clear();
        }
    }
}

void MYIBDataHandler::SetQuoteDataHandler(boost::function<void(const IBDepth*)> depth_handler)
{
    depth_handler_ = depth_handler;
}

void MYIBDataHandler::SetQuoteDataHandler(boost::function<void(const IBTick*)> tick_handler)
{
    tick_handler_ = tick_handler;
}

void MYIBDataHandler::Convert(IBDepth &result, const IBMktDataDepth &d)
{
    strncpy(&result.Symbol[0], d.Symbol.c_str(), 8);
    strncpy(&result.Exchange[0], d.Exchange.c_str(), 8);
    strncpy(&result.Currency[0], d.Currency.c_str(), 8);
    result.Timestamp = ::atoll(d.Timestamp.c_str());
    result.OpenPrice = d.OpenPrice;
    result.HighPrice = d.HighPrice;
    result.LowPrice = d.LowPrice;
    result.LastClosePrice = d.LastClosePrice;
    result.LastPrice = d.LastPrice;
    result.LastSize = d.LastSize;
    result.TotalVolume = d.TotalVolume;
    for (int i = 0; i < 10 && i < (int) d.AskLevel.size(); i++)
    {
        result.AskPrice[i] = d.AskLevel[i].Price;
        result.AskVol[i] = d.AskLevel[i].Vol;
    }
    for (int i = 0; i < 10 && i < (int) d.BidLevel.size(); i++)
    {
        result.BidPrice[i] = d.BidLevel[i].Price;
        result.BidVol[i] = d.BidLevel[i].Vol;
    }
}

void IBMktDataDepth::Update(int side, int position, double price, int size)
{
    if (position < 0)
    {
        MY_LOG_ERROR("Update with position less than 0.");
        return;
    }
    if (side == (int) IB_BID)
    {
        if ((int) BidLevel.size() <= position)
        {
            BidLevel.push_back(PriceLevel(price, size));
        }
        else
        {
            BidLevel[position] = PriceLevel(price, size);
        }
    }
    else if (side == (int) IB_ASK)
    {
        if ((int) AskLevel.size() <= position)
        {
            AskLevel.push_back(PriceLevel(price, size));
        }
        else
        {
            AskLevel[position] = PriceLevel(price, size);
        }
    }
    else
    {
        MY_LOG_ERROR("Update with error side.");
    }
}

void IBMktDataDepth::Delete(int side, int position, double price, int size)
{
    if (position < 0)
    {
        MY_LOG_ERROR("Delete with position less than 0.");
        return;
    }
    if (side == (int) IB_BID && position < (int) BidLevel.size())
    {
        BidLevel.erase(BidLevel.begin() + position);
    }
    else if (side == (int) IB_ASK && position < (int) AskLevel.size())
    {
        AskLevel.erase(AskLevel.begin() + position);
    }
    else
    {
        MY_LOG_ERROR("Delete with error position.");
    }
}

/* 下列的回调函数暂时还没实现（没需求） */
void MYIBDataHandler::reqCurrentTime()
{
    MY_LOG_DEBUG(__FUNCTION__);
}

void MYIBDataHandler::placeOrder()
{
    MY_LOG_DEBUG(__FUNCTION__);
}

void MYIBDataHandler::cancelOrder()
{
    MY_LOG_DEBUG(__FUNCTION__);
}

void MYIBDataHandler::orderStatus(OrderId orderId, const IBString &status, int filled, int remaining, double avgFillPrice, int permId, int parentId,
    double lastFillPrice, int clientId, const IBString& whyHeld)
{
    MY_LOG_DEBUG(__FUNCTION__);
}

void MYIBDataHandler::nextValidId(OrderId orderId)
{
    MY_LOG_DEBUG(__FUNCTION__);
}

void MYIBDataHandler::currentTime(long time)
{
    MY_LOG_DEBUG(__FUNCTION__);
}

void MYIBDataHandler::tickOptionComputation(TickerId tickerId, TickType tickType, double impliedVol, double delta, double optPrice, double pvDividend,
    double gamma, double vega, double theta, double undPrice)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::tickGeneric(TickerId tickerId, TickType tickType, double value)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::tickString(TickerId tickerId, TickType tickType, const IBString& value)
{
    //MY_LOG_INFO("%s: tickString: tickerId: %d, tickType: %d, value: %s.", quote_addr_, tickerId, tickType, value.c_str());
}

void MYIBDataHandler::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints, double totalDividends,
    int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::openOrder(OrderId orderId, const Contract&, const Order&, const OrderState& ostate)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::openOrderEnd()
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::winError(const IBString &str, int lastError)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::connectionClosed()
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::updateAccountValue(const IBString& key, const IBString& val, const IBString& currency, const IBString& accountName)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::updatePortfolio(const Contract& contract, int position, double marketPrice, double marketValue, double averageCost,
    double unrealizedPNL, double realizedPNL, const IBString& accountName)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::updateAccountTime(const IBString& timeStamp)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::accountDownloadEnd(const IBString& accountName)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::contractDetails(int reqId, const ContractDetails& contractDetails)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::bondContractDetails(int reqId, const ContractDetails& contractDetails)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::contractDetailsEnd(int reqId)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::execDetails(int reqId, const Contract& contract, const Execution& execution)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::execDetailsEnd(int reqId)
{
    MY_LOG_DEBUG(__FUNCTION__);
}

void MYIBDataHandler::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation, int side, double price, int size)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::managedAccounts(const IBString& accountsList)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::receiveFA(faDataType pFaDataType, const IBString& cxml)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::historicalData(TickerId reqId, const IBString& date, double open, double high, double low, double close, int volume,
    int barCount, double WAP, int hasGaps)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::scannerParameters(const IBString &xml)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::scannerData(int reqId, int rank, const ContractDetails &contractDetails, const IBString &distance, const IBString &benchmark,
    const IBString &projection, const IBString &legsStr)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::scannerDataEnd(int reqId)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close, long volume, double wap, int count)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::fundamentalData(TickerId reqId, const IBString& data)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::deltaNeutralValidation(int reqId, const UnderComp& underComp)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::tickSnapshotEnd(int reqId)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::marketDataType(TickerId reqId, int marketDataType)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::commissionReport(const CommissionReport& commissionReport)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::position(const IBString& account, const Contract& contract, int position, double avgCost)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::positionEnd()
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::accountSummary(int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::accountSummaryEnd(int reqId)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::verifyMessageAPI(const IBString& apiData)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::verifyCompleted(bool isSuccessful, const IBString& errorText)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::displayGroupList(int reqId, const IBString& groups)
{
    MY_LOG_DEBUG(__FUNCTION__);
}
void MYIBDataHandler::displayGroupUpdated(int reqId, const IBString& contractInfo)
{
    MY_LOG_DEBUG(__FUNCTION__);
}

void MYIBDataHandler::PlayBackFile()
{
    std::map<std::string, long int> symbol_to_id;
    symbol_to_id.insert(make_pair("CNX15", 0));

    std::string md_file = "/home/hh/workspace/ib/quote_ib_tick_ib_quote_20151014.dat";
    std::ifstream f_in(md_file.c_str(), std::ios_base::binary);
    SaveFileHeaderStruct f_header;
    f_header.data_count = 0;
    f_header.data_length = 0;
    f_header.data_type = 0;

// 错误处理
    if (!f_in) return;

    f_in.read((char *) &f_header, sizeof(f_header));

// 错误处理
    if (f_header.data_length == 0) return;

    SaveDataStruct<IBTick> t;
    std::cout << "size = " << f_header.data_length << "; sizeof = " << sizeof(t) << std::endl;

    std::cout << "start play back" << std::endl;
    for (int i = 0; i < f_header.data_count; ++i)
    {
        f_in.read((char *) &t, sizeof(t));
        {
            //update (TickerId id, int position, int operation, int side, double price, int size)
            auto it = symbol_to_id.find(t.data_.Symbol);
            if (it != symbol_to_id.end())
            {
                updateMktDepth(it->second, t.data_.Position, t.data_.Operation, t.data_.Side, t.data_.Price, t.data_.Size);
            }
            else
            {
                MY_LOG_INFO("No such symbol: %s.", t.data_.Symbol);
            }

            usleep(1000);
        }
    }
    f_in.close();
    std::cout << "finish all, exit soon" << std::endl;
}

void MYIBDataHandler::print(int id)
{
    for (int j = (int) contract_[id].AskLevel.size() - 1; j >= 0; j--)
    {
        printf("Ask %d: Price: %f, Vol: %d\n", j, contract_[id].AskLevel[j].Price, contract_[id].AskLevel[j].Vol);
    }
    for (int j = 0; j < (int) contract_[id].BidLevel.size(); j++)
    {
        printf("Bid %d: Price: %f, Vol: %d\n", j, contract_[id].BidLevel[j].Price, contract_[id].BidLevel[j].Vol);
    }
    printf("\n\n");
}

void MYIBDataHandler::test_init()
{
    updateMktDepth(0, 0, 1, 0, 101, 10);
    updateMktDepth(0, 1, 1, 0, 102, 10);
    updateMktDepth(0, 2, 1, 0, 103, 10);
    updateMktDepth(0, 3, 1, 0, 104, 10);
    updateMktDepth(0, 4, 1, 0, 105, 10);

    updateMktDepth(0, 0, 1, 1, 99, 10);
    updateMktDepth(0, 1, 1, 1, 98, 10);
    updateMktDepth(0, 2, 1, 1, 97, 10);
    updateMktDepth(0, 3, 1, 1, 96, 10);
    updateMktDepth(0, 4, 1, 1, 95, 10);
    printf("init.\n");
    print(0);

}

void MYIBDataHandler::test()
{
    while (api_->eConnect(host_.c_str(), port_, client_id_) == false)
    {
        sleep(1);
    }
    int i = 0;
    reqMktDepth();

//卖价超过买价
    printf("test%d\n", ++i);
    test_init();
    updateMktDepth(0, 0, 1, 0, 98, 10);
    print(0);

//买价超过卖价
    printf("test%d\n", ++i);
    test_init();
    updateMktDepth(0, 0, 1, 1, 102, 10);
    print(0);

//卖价多档同价
    printf("test%d\n", ++i);
    test_init();
    updateMktDepth(0, 0, 1, 0, 103, 10);
    print(0);

//卖价多档同价
    printf("test%d\n", ++i);
    test_init();
    updateMktDepth(0, 2, 1, 0, 102, 10);
    print(0);

//买价多档同价
    printf("test%d\n", ++i);
    test_init();
    updateMktDepth(0, 0, 1, 1, 96.5, 10);
    print(0);

//买价多档同价
    printf("test%d\n", ++i);
    test_init();
    updateMktDepth(0, 3, 1, 1, 98, 10);
    print(0);

//复杂
    printf("test%d\n", ++i);
    test_init();
    updateMktDepth(0, 2, 1, 0, 95, 10);
    print(0);
    updateMktDepth(0, 4, 1, 1, 95.5, 10);
    print(0);

//复杂
    printf("test%d\n", ++i);
    test_init();
    updateMktDepth(0, 2, 1, 1, 105, 10);
    print(0);
    updateMktDepth(0, 4, 1, 0, 104.5, 10);
    print(0);

}
