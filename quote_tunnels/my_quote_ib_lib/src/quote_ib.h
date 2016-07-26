/* Copyright (C) 2013 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#pragma once 

#include "EWrapper.h"
#include "quote_datatype_ib.h"
#include "quote_interface_ib.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include <memory>

#include <string>
#include <map>
#include <mutex>
#include <thread>

using namespace std;

class EPosixClientSocket;

enum IBDepthOperation
{
    IB_ADD, //0
    IB_UPDATE, //1
    IB_DELETE, //2
};

enum IBDirection
{
    IB_ASK, //0
    IB_BID, //1
};

struct PriceLevel
{
    double Price;
    int Vol;
    PriceLevel(double px = 0, int v = 0)
        :
            Price(px), Vol(v)
    {
    }
    PriceLevel& operator =(const PriceLevel& o)
    {
        if (this != &o)
        {
            Price = o.Price;
            Vol = o.Vol;
        }
        return *this;
    }
};

struct IBMktDataDepth
{
    std::string Symbol;
    std::string Exchange;
    std::string Currency;
    std::string Timestamp;
    double LastPrice;
    int LastSize;
    double OpenPrice;
    double HighPrice;
    double LowPrice;
    double LastClosePrice;
    int TotalVolume;
    vector<PriceLevel> AskLevel;
    vector<PriceLevel> BidLevel;

    IBMktDataDepth()
        :
            Symbol(""), Exchange(""), Currency(""), Timestamp(""), LastPrice(0), LastSize(0), OpenPrice(0), HighPrice(0), LowPrice(0), LastClosePrice(
                0),
            TotalVolume(0)
    {
    }
    void Add(int side, int position, double price, int size);

    void Update(int side, int position, double price, int size);

    void Delete(int side, int position, double price, int size);
    /*
     static void MoveForward(vector<PriceLevel>& pl, int start, int end)
     {
     while (start < (int) (pl.size() - 1) && start < end)
     {
     pl[start] = pl[start + 1];
     ++start;
     }
     pl[start] = PriceLevel();
     }

     static void MoveBackward(vector<PriceLevel>& pl, int start, int end)
     {
     while (start > 0 && start > end)
     {
     pl[start] = pl[start - 1];
     --start;
     }
     }

     static int CmpPrice(double x, double y)
     {
     long long xx = x * 10000;
     long long yy = y * 10000;
     return (xx == yy ? 0 : (xx < yy ? -1 : 1));
     }

     static void RemoveEmptyPriceLevel(vector<PriceLevel>& pl)
     {
     size_t s = pl.size();
     size_t j = 1;
     for (size_t i = 0; i < s - 1;)
     {
     if (!CmpPrice(pl[i].Price, 0))
     {
     for (; j < s; ++j)
     {
     if (CmpPrice(pl[j].Price, 0))
     {
     pl[i++] = pl[j];
     pl[j] = PriceLevel();
     }
     }
     }
     else
     {
     ++i;
     }
     }
     }
     */
};

class MYIBDataHandler: public EWrapper
{
public:

    MYIBDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData& cfg, const std::string id);
    ~MYIBDataHandler();
    void SetQuoteDataHandler(boost::function<void(const IBDepth*)> depth_handler);
    void SetQuoteDataHandler(boost::function<void(const IBTick*)> tick_handler);

private:

    void reqCurrentTime();
    void placeOrder();
    void cancelOrder();

public:
    // events
    void tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute);
    void tickSize(TickerId tickerId, TickType field, int size);
    void tickOptionComputation(TickerId tickerId, TickType tickType, double impliedVol, double delta, double optPrice, double pvDividend,
        double gamma, double vega, double theta, double undPrice);
    void tickGeneric(TickerId tickerId, TickType tickType, double value);
    void tickString(TickerId tickerId, TickType tickType, const IBString& value);
    void tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints, double totalDividends, int holdDays,
        const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry);
    void orderStatus(OrderId orderId, const IBString &status, int filled, int remaining, double avgFillPrice, int permId, int parentId,
        double lastFillPrice, int clientId, const IBString& whyHeld);
    void openOrder(OrderId orderId, const Contract&, const Order&, const OrderState&);
    void openOrderEnd();
    void winError(const IBString &str, int lastError);
    void connectionClosed();
    void updateAccountValue(const IBString& key, const IBString& val, const IBString& currency, const IBString& accountName);
    void updatePortfolio(const Contract& contract, int position, double marketPrice, double marketValue, double averageCost, double unrealizedPNL,
        double realizedPNL, const IBString& accountName);
    void updateAccountTime(const IBString& timeStamp);
    void accountDownloadEnd(const IBString& accountName);
    void nextValidId(OrderId orderId);
    void contractDetails(int reqId, const ContractDetails& contractDetails);
    void bondContractDetails(int reqId, const ContractDetails& contractDetails);
    void contractDetailsEnd(int reqId);
    void execDetails(int reqId, const Contract& contract, const Execution& execution);
    void execDetailsEnd(int reqId);
    void error(const int id, const int errorCode, const IBString errorString);
    void updateMktDepth(TickerId id, int position, int operation, int side, double price, int size);
    void updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation, int side, double price, int size);
    void updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch);
    void managedAccounts(const IBString& accountsList);
    void receiveFA(faDataType pFaDataType, const IBString& cxml);
    void historicalData(TickerId reqId, const IBString& date, double open, double high, double low, double close, int volume, int barCount,
        double WAP, int hasGaps);
    void scannerParameters(const IBString &xml);
    void scannerData(int reqId, int rank, const ContractDetails &contractDetails, const IBString &distance, const IBString &benchmark,
        const IBString &projection, const IBString &legsStr);
    void scannerDataEnd(int reqId);
    void realtimeBar(TickerId reqId, long time, double open, double high, double low, double close, long volume, double wap, int count);
    void currentTime(long time);
    void fundamentalData(TickerId reqId, const IBString& data);
    void deltaNeutralValidation(int reqId, const UnderComp& underComp);
    void tickSnapshotEnd(int reqId);
    void marketDataType(TickerId reqId, int marketDataType);
    void commissionReport(const CommissionReport& commissionReport);
    void position(const IBString& account, const Contract& contract, int position, double avgCost);
    void positionEnd();
    void
    accountSummary(int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency);
    void accountSummaryEnd(int reqId);
    void verifyMessageAPI(const IBString& apiData);
    void verifyCompleted(bool isSuccessful, const IBString& errorText);
    void displayGroupList(int reqId, const IBString& groups);
    void displayGroupUpdated(int reqId, const IBString& contractInfo);

private:

    std::auto_ptr<EPosixClientSocket> api_;

    bool ParseConfig();
    void connect(int wait_seconds);
    void reqMktDepth();
    void Convert(IBDepth &result, const IBMktDataDepth &d);
    void PlayBackFile();
    void test();
    void test_init();
    void print(int id);

    bool reconnected_;
    int client_id_;
    char* quote_addr_;
    std::string host_;
    unsigned int port_;

    long count_total_;
    long count_of_l2_;
    std::string ip_;
    std::string id_;
    char qtm_name_[32];

    IBMktDataDepth *contract_;
    SubscribeContracts subscribe_contracts_;
    SubscribeContracts subscribe_contracts_user_;
    std::mutex ids_sync_;
    std::mutex quote_mutex_;

    ConfigData cfg_;
    QuoteDataSave<IBDepth>* p_save_depth_;
    QuoteDataSave<IBTick>* p_save_tick_;
    boost::function<void(const IBDepth*)> depth_handler_;
    boost::function<void(const IBTick*)> tick_handler_;

};

