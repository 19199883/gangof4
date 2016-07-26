/* Copyright (C) 2013 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#ifndef ewrapper_def
#define ewrapper_def

#include "CommonDefs.h"
#include "IBString.h"

enum TickType { 		BID_SIZE, 			//0, bid size
				BID, 				//1, bid px
				ASK, 				//2, ask size
				ASK_SIZE, 			//3, ask px
				LAST, 				//4, last px
				LAST_SIZE,			//5, last size
				HIGH, 				//6, high px
				LOW, 				//7, low px
				VOLUME, 			//8, traded volume
				CLOSE,				//9, last close
				BID_OPTION_COMPUTATION, 	//10
				ASK_OPTION_COMPUTATION, 	//11
				LAST_OPTION_COMPUTATION,	//12
				MODEL_OPTION,			//13
				OPEN,				//14, open px
				LOW_13_WEEK,			//15
				HIGH_13_WEEK,			//16
				LOW_26_WEEK,			//17
				HIGH_26_WEEK,			//18
				LOW_52_WEEK,			//19
				HIGH_52_WEEK,			//20
				AVG_VOLUME,			//21
				OPEN_INTEREST,			//22
				OPTION_HISTORICAL_VOL,		//23
				OPTION_IMPLIED_VOL,		//24
				OPTION_BID_EXCH,		//25
				OPTION_ASK_EXCH,		//26
				OPTION_CALL_OPEN_INTEREST,	//27
				OPTION_PUT_OPEN_INTEREST,	//28
				OPTION_CALL_VOLUME,		//29
				OPTION_PUT_VOLUME,		//30
				INDEX_FUTURE_PREMIUM,		//31
				BID_EXCH,			//32
				ASK_EXCH,			//33
				AUCTION_VOLUME,			//34
				AUCTION_PRICE,			//35
				AUCTION_IMBALANCE,		//36
				MARK_PRICE,			//37
				BID_EFP_COMPUTATION,		//38
				ASK_EFP_COMPUTATION,		//39
				LAST_EFP_COMPUTATION,		//40
				OPEN_EFP_COMPUTATION,		//41
				HIGH_EFP_COMPUTATION,		//42
				LOW_EFP_COMPUTATION,		//43
				CLOSE_EFP_COMPUTATION,		//44
				LAST_TIMESTAMP,			//45, timestamp
				SHORTABLE,			//46
				FUNDAMENTAL_RATIOS,		//47
				RT_VOLUME,			//48
				HALTED,				//49
				BID_YIELD,			//50
				ASK_YIELD,			//51
				LAST_YIELD,			//52
				CUST_OPTION_COMPUTATION,	//53
				TRADE_COUNT,			//54
				TRADE_RATE,			//55
				VOLUME_RATE,			//56
				LAST_RTH_TRADE,			//57
				NOT_SET };	

inline bool isPrice( TickType tickType) {
	return tickType == BID || tickType == ASK || tickType == LAST;
}

struct Contract;
struct ContractDetails;
struct Order;
struct OrderState;
struct Execution;
struct UnderComp;
struct CommissionReport;

class EWrapper
{
public:
   virtual ~EWrapper() {};

   virtual void tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute) = 0;
   virtual void tickSize( TickerId tickerId, TickType field, int size) = 0;
   virtual void tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
	   double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice) = 0;
   virtual void tickGeneric(TickerId tickerId, TickType tickType, double value) = 0;
   virtual void tickString(TickerId tickerId, TickType tickType, const IBString& value) = 0;
   virtual void tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
	   double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry) = 0;
   virtual void orderStatus( OrderId orderId, const IBString &status, int filled,
	   int remaining, double avgFillPrice, int permId, int parentId,
	   double lastFillPrice, int clientId, const IBString& whyHeld) = 0;
   virtual void openOrder( OrderId orderId, const Contract&, const Order&, const OrderState&) = 0;
   virtual void openOrderEnd() = 0;
   virtual void winError( const IBString &str, int lastError) = 0;
   virtual void connectionClosed() = 0;
   virtual void updateAccountValue(const IBString& key, const IBString& val,
   const IBString& currency, const IBString& accountName) = 0;
   virtual void updatePortfolio( const Contract& contract, int position,
      double marketPrice, double marketValue, double averageCost,
      double unrealizedPNL, double realizedPNL, const IBString& accountName) = 0;
   virtual void updateAccountTime(const IBString& timeStamp) = 0;
   virtual void accountDownloadEnd(const IBString& accountName) = 0;
   virtual void nextValidId( OrderId orderId) = 0;
   virtual void contractDetails( int reqId, const ContractDetails& contractDetails) = 0;
   virtual void bondContractDetails( int reqId, const ContractDetails& contractDetails) = 0;
   virtual void contractDetailsEnd( int reqId) = 0;
   virtual void execDetails( int reqId, const Contract& contract, const Execution& execution) =0;
   virtual void execDetailsEnd( int reqId) =0;
   virtual void error(const int id, const int errorCode, const IBString errorString) = 0;
   virtual void updateMktDepth(TickerId id, int position, int operation, int side,
      double price, int size) = 0;
   virtual void updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
      int side, double price, int size) = 0;
   virtual void updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch) = 0;
   virtual void managedAccounts( const IBString& accountsList) = 0;
   virtual void receiveFA(faDataType pFaDataType, const IBString& cxml) = 0;
   virtual void historicalData(TickerId reqId, const IBString& date, double open, double high, 
	   double low, double close, int volume, int barCount, double WAP, int hasGaps) = 0;
   virtual void scannerParameters(const IBString &xml) = 0;
   virtual void scannerData(int reqId, int rank, const ContractDetails &contractDetails,
	   const IBString &distance, const IBString &benchmark, const IBString &projection,
	   const IBString &legsStr) = 0;
   virtual void scannerDataEnd(int reqId) = 0;
   virtual void realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
	   long volume, double wap, int count) = 0;
   virtual void currentTime(long time) = 0;
   virtual void fundamentalData(TickerId reqId, const IBString& data) = 0;
   virtual void deltaNeutralValidation(int reqId, const UnderComp& underComp) = 0;
   virtual void tickSnapshotEnd( int reqId) = 0;
   virtual void marketDataType( TickerId reqId, int marketDataType) = 0;
   virtual void commissionReport( const CommissionReport &commissionReport) = 0;
   virtual void position( const IBString& account, const Contract& contract, int position, double avgCost) = 0;
   virtual void positionEnd() = 0;
   virtual void accountSummary( int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency) = 0;
   virtual void accountSummaryEnd( int reqId) = 0;
   virtual void verifyMessageAPI( const IBString& apiData) = 0;
   virtual void verifyCompleted( bool isSuccessful, const IBString& errorText) = 0;
   virtual void displayGroupList( int reqId, const IBString& groups) = 0;
   virtual void displayGroupUpdated( int reqId, const IBString& contractInfo) = 0;
};


#endif
