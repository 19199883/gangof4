#if !defined(QUOTE_DATATYPE_DCE_H_)
#define QUOTE_DATATYPE_DCE_H_

#include "DFITCL2ApiDataType.h"
#include "quote_datatype_common.h"

#include <string.h>

#pragma pack(push)
#pragma pack(8)

////////////////////////////////////////////////
///MDBestAndDeep：最优与五档深度行情
////////////////////////////////////////////////
struct MDBestAndDeep_MY
{
    INT1 Type;
    UINT4 Length;                         //报文长度
    UINT4 Version;                        //版本从1开始
    UINT4 Time;                           //预留字段
    INT1 Exchange[3];                    //交易所
    INT1 Contract[80];                   //合约代码
    BOOL SuspensionSign;                 //停牌标志
    REAL4 LastClearPrice;                 //昨结算价
    REAL4 ClearPrice;                     //今结算价
    REAL4 AvgPrice;                       //成交均价
    REAL4 LastClose;                      //昨收盘
    REAL4 Close;                          //今收盘
    REAL4 OpenPrice;                      //今开盘
    UINT4 LastOpenInterest;               //昨持仓量
    UINT4 OpenInterest;                   //持仓量
    REAL4 LastPrice;                      //最新价
    UINT4 MatchTotQty;                    //成交数量
    REAL8 Turnover;                       //成交金额
    REAL4 RiseLimit;                      //最高报价
    REAL4 FallLimit;                      //最低报价
    REAL4 HighPrice;                      //最高价
    REAL4 LowPrice;                       //最低价
    REAL4 PreDelta;                       //昨虚实度
    REAL4 CurrDelta;                      //今虚实度

    REAL4 BuyPriceOne;              //买入价格1
    UINT4 BuyQtyOne;                //买入数量1
    UINT4 BuyImplyQtyOne;           //买1推导量
    REAL4 BuyPriceTwo;
    UINT4 BuyQtyTwo;
    UINT4 BuyImplyQtyTwo;
    REAL4 BuyPriceThree;
    UINT4 BuyQtyThree;
    UINT4 BuyImplyQtyThree;
    REAL4 BuyPriceFour;
    UINT4 BuyQtyFour;
    UINT4 BuyImplyQtyFour;
    REAL4 BuyPriceFive;
    UINT4 BuyQtyFive;
    UINT4 BuyImplyQtyFive;
    REAL4 SellPriceOne;             //卖出价格1
    UINT4 SellQtyOne;               //买出数量1
    UINT4 SellImplyQtyOne;          //卖1推导量
    REAL4 SellPriceTwo;
    UINT4 SellQtyTwo;
    UINT4 SellImplyQtyTwo;
    REAL4 SellPriceThree;
    UINT4 SellQtyThree;
    UINT4 SellImplyQtyThree;
    REAL4 SellPriceFour;
    UINT4 SellQtyFour;
    UINT4 SellImplyQtyFour;
    REAL4 SellPriceFive;
    UINT4 SellQtyFive;
    UINT4 SellImplyQtyFive;

    INT1 GenTime[13];                    //行情产生时间
    UINT4 LastMatchQty;                   //最新成交量
    INT4 InterestChg;                    //持仓量变化
    REAL4 LifeLow;                        //历史最低价
    REAL4 LifeHigh;                       //历史最高价
    // 20140327 update by dce
//    UINT4 BidImplyQty;                    //申买推导量
//    UINT4 AskImplyQty;                    //申卖推导量
    REAL8 Delta;                          //delta
    REAL8 Gamma;                          //gama
    REAL8 Rho;                            //rho
    REAL8 Theta;                          //theta
    REAL8 Vega;                           //vega
    INT1 TradeDate[9];                   //行情日期
    INT1 LocalDate[9];                   //本地日期

    MDBestAndDeep_MY()
    {
    }
    MDBestAndDeep_MY(const DFITC_L2::MDBestAndDeep &other)
    {
        Type = other.Type;             // Type;
        Length = other.Length;           // Length;                         //报文长度
        Version = other.Version;          // Version;                        //版本从1开始
        Time = other.Time;             // Time;                           //预留字段
        memcpy(Exchange, other.Exchange, 3);      // Exchange[3];                    //交易所
        memcpy(Contract, other.Contract, 80);     // Contract[80];                   //合约代码
        SuspensionSign = other.SuspensionSign;   // SuspensionSign;                 //停牌标志
        LastClearPrice = InvalidToZeroF(other.LastClearPrice);   // LastClearPrice;                 //昨结算价
        ClearPrice = InvalidToZeroF(other.ClearPrice);       // ClearPrice;                     //今结算价
        AvgPrice = InvalidToZeroF(other.AvgPrice);         // AvgPrice;                       //成交均价
        LastClose = InvalidToZeroF(other.LastClose);        // LastClose;                      //昨收盘
        Close = InvalidToZeroF(other.Close);            // Close;                          //今收盘
        OpenPrice = InvalidToZeroF(other.OpenPrice);        // OpenPrice;                      //今开盘
        LastOpenInterest = other.LastOpenInterest; // LastOpenInterest;               //昨持仓量
        OpenInterest = other.OpenInterest;     // OpenInterest;                   //持仓量
        LastPrice = InvalidToZeroF(other.LastPrice);        // LastPrice;                      //最新价
        MatchTotQty = other.MatchTotQty;      // MatchTotQty;                    //成交数量
        Turnover = InvalidToZeroD(other.Turnover);         // Turnover;                       //成交金额
        RiseLimit = InvalidToZeroF(other.RiseLimit);        // RiseLimit;                      //最高报价
        FallLimit = InvalidToZeroF(other.FallLimit);        // FallLimit;                      //最低报价
        HighPrice = InvalidToZeroF(other.HighPrice);        // HighPrice;                      //最高价
        LowPrice = InvalidToZeroF(other.LowPrice);         // LowPrice;                       //最低价
        PreDelta = InvalidToZeroF(other.PreDelta);         // PreDelta;                       //昨虚实度
        CurrDelta = InvalidToZeroF(other.CurrDelta);        // CurrDelta;                      //今虚实度

        BuyPriceOne = InvalidToZeroF(other.BuyPriceOne);      // BuyPriceOne;                    //买入价格1
        BuyQtyOne = other.BuyQtyOne;        // BuyQtyOne;                      //买入数量1
        BuyImplyQtyOne = other.BuyImplyQtyOne;
        BuyPriceTwo = InvalidToZeroF(other.BuyPriceTwo);      // BuyPriceTwo;
        BuyQtyTwo = other.BuyQtyTwo;        // BuyQtyTwo;
        BuyImplyQtyTwo = other.BuyImplyQtyTwo;
        BuyPriceThree = InvalidToZeroF(other.BuyPriceThree);    // BuyPriceThree;
        BuyQtyThree = other.BuyQtyThree;      // BuyQtyThree;
        BuyImplyQtyThree = other.BuyImplyQtyThree;
        BuyPriceFour = InvalidToZeroF(other.BuyPriceFour);     // BuyPriceFour;
        BuyQtyFour = other.BuyQtyFour;       // BuyQtyFour;
        BuyImplyQtyFour = other.BuyImplyQtyFour;
        BuyPriceFive = InvalidToZeroF(other.BuyPriceFive);     // BuyPriceFive;
        BuyQtyFive = other.BuyQtyFive;       // BuyQtyFive;
        BuyImplyQtyFive = other.BuyImplyQtyFive;
        SellPriceOne = InvalidToZeroF(other.SellPriceOne);     // SellPriceOne;                   //卖出价格1
        SellQtyOne = other.SellQtyOne;       // SellQtyOne;                     //买出数量1
        SellImplyQtyOne = other.SellImplyQtyOne;
        SellPriceTwo = InvalidToZeroF(other.SellPriceTwo);     // SellPriceTwo;
        SellQtyTwo = other.SellQtyTwo;       // SellQtyTwo;
        SellImplyQtyTwo = other.SellImplyQtyTwo;
        SellPriceThree = InvalidToZeroF(other.SellPriceThree);   // SellPriceThree;
        SellQtyThree = other.SellQtyThree;     // SellQtyThree;
        SellImplyQtyThree = other.SellImplyQtyThree;
        SellPriceFour = InvalidToZeroF(other.SellPriceFour);    // SellPriceFour;
        SellQtyFour = other.SellQtyFour;      // SellQtyFour;
        SellImplyQtyFour = other.SellImplyQtyFour;
        SellPriceFive = InvalidToZeroF(other.SellPriceFive);    // SellPriceFive;
        SellQtyFive = other.SellQtyFive;      // SellQtyFive;
        SellImplyQtyFive = other.SellImplyQtyFive;

        memcpy(GenTime, other.GenTime, 13);      // GenTime[13];                    //行情产生时间
        LastMatchQty = other.LastMatchQty;     // LastMatchQty;                   //最新成交量
        InterestChg = other.InterestChg;      // InterestChg;                    //持仓量变化
        LifeLow = InvalidToZeroF(other.LifeLow);          // LifeLow;                        //历史最低价
        LifeHigh = InvalidToZeroF(other.LifeHigh);         // LifeHigh;                       //历史最高价
        //BidImplyQty = other.BidImplyQty;      // BidImplyQty;                    //申买推导量
        //AskImplyQty = other.AskImplyQty;      // AskImplyQty;                    //申卖推导量
        Delta = InvalidToZeroD(other.Delta);            // Delta;                          //delta
        Gamma = InvalidToZeroD(other.Gamma);            // Gamma;                          //gama
        Rho = InvalidToZeroD(other.Rho);              // Rho;                            //rho
        Theta = InvalidToZeroD(other.Theta);            // Theta;                          //theta
        Vega = InvalidToZeroD(other.Vega);             // Vega;                           //vega
        memcpy(TradeDate, other.TradeDate, 9);     // TradeDate[9];                   //行情日期
        memcpy(LocalDate, other.LocalDate, 9);     // LocalDate[9];                   //本地日期
    }
};

////////////////////////////////////////////////
///MDBestAndDeep：arbi深度行情
////////////////////////////////////////////////
struct MDArbi_MY
{
    INT1 Type;
    UINT4 Length;                         //报文长度
    UINT4 Version;                        //版本从1开始
    UINT4 Time;                           //预留字段
    INT1 Exchange[3];                    //交易所
    INT1 Contract[80];                   //合约代码
    BOOL SuspensionSign;                 //停牌标志
    REAL4 LastClearPrice;                 //昨结算价
    REAL4 ClearPrice;                     //今结算价
    REAL4 AvgPrice;                       //成交均价
    REAL4 LastClose;                      //昨收盘
    REAL4 Close;                          //今收盘
    REAL4 OpenPrice;                      //今开盘
    UINT4 LastOpenInterest;               //昨持仓量
    UINT4 OpenInterest;                   //持仓量
    REAL4 LastPrice;                      //最新价
    UINT4 MatchTotQty;                    //成交数量
    REAL8 Turnover;                       //成交金额
    REAL4 RiseLimit;                      //最高报价
    REAL4 FallLimit;                      //最低报价
    REAL4 HighPrice;                      //最高价
    REAL4 LowPrice;                       //最低价
    REAL4 PreDelta;                       //昨虚实度
    REAL4 CurrDelta;                      //今虚实度

    REAL4 BuyPriceOne;              //买入价格1
    UINT4 BuyQtyOne;                //买入数量1
    UINT4 BuyImplyQtyOne;           //买1推导量
    REAL4 BuyPriceTwo;
    UINT4 BuyQtyTwo;
    UINT4 BuyImplyQtyTwo;
    REAL4 BuyPriceThree;
    UINT4 BuyQtyThree;
    UINT4 BuyImplyQtyThree;
    REAL4 BuyPriceFour;
    UINT4 BuyQtyFour;
    UINT4 BuyImplyQtyFour;
    REAL4 BuyPriceFive;
    UINT4 BuyQtyFive;
    UINT4 BuyImplyQtyFive;
    REAL4 SellPriceOne;             //卖出价格1
    UINT4 SellQtyOne;               //买出数量1
    UINT4 SellImplyQtyOne;          //卖1推导量
    REAL4 SellPriceTwo;
    UINT4 SellQtyTwo;
    UINT4 SellImplyQtyTwo;
    REAL4 SellPriceThree;
    UINT4 SellQtyThree;
    UINT4 SellImplyQtyThree;
    REAL4 SellPriceFour;
    UINT4 SellQtyFour;
    UINT4 SellImplyQtyFour;
    REAL4 SellPriceFive;
    UINT4 SellQtyFive;
    UINT4 SellImplyQtyFive;

    INT1 GenTime[13];                    //行情产生时间
    UINT4 LastMatchQty;                   //最新成交量
    INT4 InterestChg;                    //持仓量变化
    REAL4 LifeLow;                        //历史最低价
    REAL4 LifeHigh;                       //历史最高价
    // 20140327 update by dce
    //    UINT4 BidImplyQty;                    //申买推导量
    //    UINT4 AskImplyQty;                    //申卖推导量
    REAL8 Delta;                          //delta
    REAL8 Gamma;                          //gama
    REAL8 Rho;                            //rho
    REAL8 Theta;                          //theta
    REAL8 Vega;                           //vega
    INT1 TradeDate[9];                   //行情日期
    INT1 LocalDate[9];                   //本地日期

    MDArbi_MY()
    {
    }
    MDArbi_MY(const DFITC_L2::MDBestAndDeep &other)
    {
        Type = other.Type;             // Type;
        Length = other.Length;           // Length;                         //报文长度
        Version = other.Version;          // Version;                        //版本从1开始
        Time = other.Time;             // Time;                           //预留字段
        memcpy(Exchange, other.Exchange, 3);      // Exchange[3];                    //交易所
        memcpy(Contract, other.Contract, 80);     // Contract[80];                   //合约代码
        SuspensionSign = other.SuspensionSign;   // SuspensionSign;                 //停牌标志
        LastClearPrice = InvalidToZeroF(other.LastClearPrice);   // LastClearPrice;                 //昨结算价
        ClearPrice = InvalidToZeroF(other.ClearPrice);       // ClearPrice;                     //今结算价
        AvgPrice = InvalidToZeroF(other.AvgPrice);         // AvgPrice;                       //成交均价
        LastClose = InvalidToZeroF(other.LastClose);        // LastClose;                      //昨收盘
        Close = InvalidToZeroF(other.Close);            // Close;                          //今收盘
        OpenPrice = InvalidToZeroF(other.OpenPrice);        // OpenPrice;                      //今开盘
        LastOpenInterest = other.LastOpenInterest; // LastOpenInterest;               //昨持仓量
        OpenInterest = other.OpenInterest;     // OpenInterest;                   //持仓量
        LastPrice = InvalidToZeroF(other.LastPrice);        // LastPrice;                      //最新价
        MatchTotQty = other.MatchTotQty;      // MatchTotQty;                    //成交数量
        Turnover = InvalidToZeroD(other.Turnover);         // Turnover;                       //成交金额
        RiseLimit = InvalidToZeroF(other.RiseLimit);        // RiseLimit;                      //最高报价
        FallLimit = InvalidToZeroF(other.FallLimit);        // FallLimit;                      //最低报价
        HighPrice = InvalidToZeroF(other.HighPrice);        // HighPrice;                      //最高价
        LowPrice = InvalidToZeroF(other.LowPrice);         // LowPrice;                       //最低价
        PreDelta = InvalidToZeroF(other.PreDelta);         // PreDelta;                       //昨虚实度
        CurrDelta = InvalidToZeroF(other.CurrDelta);        // CurrDelta;                      //今虚实度

        BuyPriceOne = InvalidToZeroF(other.BuyPriceOne);      // BuyPriceOne;                    //买入价格1
        BuyQtyOne = other.BuyQtyOne;        // BuyQtyOne;                      //买入数量1
        BuyImplyQtyOne = other.BuyImplyQtyOne;
        BuyPriceTwo = InvalidToZeroF(other.BuyPriceTwo);      // BuyPriceTwo;
        BuyQtyTwo = other.BuyQtyTwo;        // BuyQtyTwo;
        BuyImplyQtyTwo = other.BuyImplyQtyTwo;
        BuyPriceThree = InvalidToZeroF(other.BuyPriceThree);    // BuyPriceThree;
        BuyQtyThree = other.BuyQtyThree;      // BuyQtyThree;
        BuyImplyQtyThree = other.BuyImplyQtyThree;
        BuyPriceFour = InvalidToZeroF(other.BuyPriceFour);     // BuyPriceFour;
        BuyQtyFour = other.BuyQtyFour;       // BuyQtyFour;
        BuyImplyQtyFour = other.BuyImplyQtyFour;
        BuyPriceFive = InvalidToZeroF(other.BuyPriceFive);     // BuyPriceFive;
        BuyQtyFive = other.BuyQtyFive;       // BuyQtyFive;
        BuyImplyQtyFive = other.BuyImplyQtyFive;
        SellPriceOne = InvalidToZeroF(other.SellPriceOne);     // SellPriceOne;                   //卖出价格1
        SellQtyOne = other.SellQtyOne;       // SellQtyOne;                     //买出数量1
        SellImplyQtyOne = other.SellImplyQtyOne;
        SellPriceTwo = InvalidToZeroF(other.SellPriceTwo);     // SellPriceTwo;
        SellQtyTwo = other.SellQtyTwo;       // SellQtyTwo;
        SellImplyQtyTwo = other.SellImplyQtyTwo;
        SellPriceThree = InvalidToZeroF(other.SellPriceThree);   // SellPriceThree;
        SellQtyThree = other.SellQtyThree;     // SellQtyThree;
        SellImplyQtyThree = other.SellImplyQtyThree;
        SellPriceFour = InvalidToZeroF(other.SellPriceFour);    // SellPriceFour;
        SellQtyFour = other.SellQtyFour;      // SellQtyFour;
        SellImplyQtyFour = other.SellImplyQtyFour;
        SellPriceFive = InvalidToZeroF(other.SellPriceFive);    // SellPriceFive;
        SellQtyFive = other.SellQtyFive;      // SellQtyFive;
        SellImplyQtyFive = other.SellImplyQtyFive;

        memcpy(GenTime, other.GenTime, 13);      // GenTime[13];                    //行情产生时间
        LastMatchQty = other.LastMatchQty;     // LastMatchQty;                   //最新成交量
        InterestChg = other.InterestChg;      // InterestChg;                    //持仓量变化
        LifeLow = InvalidToZeroF(other.LifeLow);          // LifeLow;                        //历史最低价
        LifeHigh = InvalidToZeroF(other.LifeHigh);         // LifeHigh;                       //历史最高价
        //BidImplyQty = other.BidImplyQty;      // BidImplyQty;                    //申买推导量
        //AskImplyQty = other.AskImplyQty;      // AskImplyQty;                    //申卖推导量
        Delta = InvalidToZeroD(other.Delta);            // Delta;                          //delta
        Gamma = InvalidToZeroD(other.Gamma);            // Gamma;                          //gama
        Rho = InvalidToZeroD(other.Rho);              // Rho;                            //rho
        Theta = InvalidToZeroD(other.Theta);            // Theta;                          //theta
        Vega = InvalidToZeroD(other.Vega);             // Vega;                           //vega
        memcpy(TradeDate, other.TradeDate, 9);     // TradeDate[9];                   //行情日期
        memcpy(LocalDate, other.LocalDate, 9);     // LocalDate[9];                   //本地日期
    }
};

////////////////////////////////////////////////
///MDTenEntrust：最优价位上十笔委托
////////////////////////////////////////////////
struct MDTenEntrust_MY
{
    INT1 Type;                           //行情域标识
    UINT4 Len;
    INT1 Contract[80];                   //合约代码
    REAL8 BestBuyOrderPrice;              //价格
    UINT4 BestBuyOrderQtyOne;             //委托量1
    UINT4 BestBuyOrderQtyTwo;             //委托量2
    UINT4 BestBuyOrderQtyThree;           //委托量3
    UINT4 BestBuyOrderQtyFour;            //委托量4
    UINT4 BestBuyOrderQtyFive;            //委托量5
    UINT4 BestBuyOrderQtySix;             //委托量6
    UINT4 BestBuyOrderQtySeven;           //委托量7
    UINT4 BestBuyOrderQtyEight;           //委托量8
    UINT4 BestBuyOrderQtyNine;            //委托量9
    UINT4 BestBuyOrderQtyTen;             //委托量10
    REAL8 BestSellOrderPrice;             //价格
    UINT4 BestSellOrderQtyOne;            //委托量1
    UINT4 BestSellOrderQtyTwo;            //委托量2
    UINT4 BestSellOrderQtyThree;          //委托量3
    UINT4 BestSellOrderQtyFour;           //委托量4
    UINT4 BestSellOrderQtyFive;           //委托量5
    UINT4 BestSellOrderQtySix;            //委托量6
    UINT4 BestSellOrderQtySeven;          //委托量7
    UINT4 BestSellOrderQtyEight;          //委托量8
    UINT4 BestSellOrderQtyNine;           //委托量9
    UINT4 BestSellOrderQtyTen;            //委托量10
    INT1 GenTime[13];                    //生成时间

    MDTenEntrust_MY()
    {
    }
    MDTenEntrust_MY(const DFITC_L2::MDTenEntrust &other)
    {
        Type = other.Type;                  // Type;                           //行情域标识
        Len = other.Len;                   // Len;
        memcpy(Contract, other.Contract, 80);          // Contract[80];                   //合约代码
        BestBuyOrderPrice = InvalidToZeroD(other.BestBuyOrderPrice);     // BestBuyOrderPrice;              //价格
        BestBuyOrderQtyOne = other.BestBuyOrderQtyOne;    // BestBuyOrderQtyOne;             //委托量1
        BestBuyOrderQtyTwo = other.BestBuyOrderQtyTwo;    // BestBuyOrderQtyTwo;             //委托量2
        BestBuyOrderQtyThree = other.BestBuyOrderQtyThree;  // BestBuyOrderQtyThree;           //委托量3
        BestBuyOrderQtyFour = other.BestBuyOrderQtyFour;   // BestBuyOrderQtyFour;            //委托量4
        BestBuyOrderQtyFive = other.BestBuyOrderQtyFive;   // BestBuyOrderQtyFive;            //委托量5
        BestBuyOrderQtySix = other.BestBuyOrderQtySix;    // BestBuyOrderQtySix;             //委托量6
        BestBuyOrderQtySeven = other.BestBuyOrderQtySeven;  // BestBuyOrderQtySeven;           //委托量7
        BestBuyOrderQtyEight = other.BestBuyOrderQtyEight;  // BestBuyOrderQtyEight;           //委托量8
        BestBuyOrderQtyNine = other.BestBuyOrderQtyNine;   // BestBuyOrderQtyNine;            //委托量9
        BestBuyOrderQtyTen = other.BestBuyOrderQtyTen;    // BestBuyOrderQtyTen;             //委托量10
        BestSellOrderPrice = InvalidToZeroD(other.BestSellOrderPrice);    // BestSellOrderPrice;             //价格
        BestSellOrderQtyOne = other.BestSellOrderQtyOne;   // BestSellOrderQtyOne;            //委托量1
        BestSellOrderQtyTwo = other.BestSellOrderQtyTwo;   // BestSellOrderQtyTwo;            //委托量2
        BestSellOrderQtyThree = other.BestSellOrderQtyThree; // BestSellOrderQtyThree;          //委托量3
        BestSellOrderQtyFour = other.BestSellOrderQtyFour;  // BestSellOrderQtyFour;           //委托量4
        BestSellOrderQtyFive = other.BestSellOrderQtyFive;  // BestSellOrderQtyFive;           //委托量5
        BestSellOrderQtySix = other.BestSellOrderQtySix;   // BestSellOrderQtySix;            //委托量6
        BestSellOrderQtySeven = other.BestSellOrderQtySeven; // BestSellOrderQtySeven;          //委托量7
        BestSellOrderQtyEight = other.BestSellOrderQtyEight; // BestSellOrderQtyEight;          //委托量8
        BestSellOrderQtyNine = other.BestSellOrderQtyNine;  // BestSellOrderQtyNine;           //委托量9
        BestSellOrderQtyTen = other.BestSellOrderQtyTen;   // BestSellOrderQtyTen;            //委托量10
        memcpy(GenTime, other.GenTime, 13);           // GenTime[13];                    //生成时间
    }
};

////////////////////////////////////////////////
///MDOrderStatistic：加权平均以及委托总量行情
////////////////////////////////////////////////
struct MDOrderStatistic_MY
{
    INT1 Type;                           //行情域标识
    UINT4 Len;
    INT1 ContractID[80];                 //合约号
    UINT4 TotalBuyOrderNum;               //买委托总量
    UINT4 TotalSellOrderNum;              //卖委托总量
    REAL8 WeightedAverageBuyOrderPrice;   //加权平均委买价格
    REAL8 WeightedAverageSellOrderPrice;  //加权平均委卖价格

    MDOrderStatistic_MY()
    {
    }
    MDOrderStatistic_MY(const DFITC_L2::MDOrderStatistic &other)
    {
        Type = other.Type;                         // Type;                           //行情域标识
        Len = other.Len;                          // Len;
        memcpy(ContractID, other.ContractID, 80);               // ContractID[80];                 //合约号
        TotalBuyOrderNum = other.TotalBuyOrderNum;             // TotalBuyOrderNum;               //买委托总量
        TotalSellOrderNum = other.TotalSellOrderNum;            // TotalSellOrderNum;              //卖委托总量
        WeightedAverageBuyOrderPrice = InvalidToZeroD(other.WeightedAverageBuyOrderPrice); // WeightedAverageBuyOrderPrice;   //加权平均委买价格
        WeightedAverageSellOrderPrice = InvalidToZeroD(other.WeightedAverageSellOrderPrice); // WeightedAverageSellOrderPrice;  //加权平均委卖价格
    }
};

////////////////////////////////////////////////
///MDRealTimePrice：实时结算价
////////////////////////////////////////////////
struct MDRealTimePrice_MY
{
    INT1 Type;                           //行情域标识
    UINT4 Len;
    INT1 ContractID[80];                 //合约号
    REAL8 RealTimePrice;                  //实时结算价

    MDRealTimePrice_MY()
    {
    }
    MDRealTimePrice_MY(const DFITC_L2::MDRealTimePrice &other)
    {
        Type = other.Type;           // Type;                           //行情域标识
        Len = other.Len;            // Len;
        memcpy(ContractID, other.ContractID, 80); // ContractID[80];                 //合约号
        RealTimePrice = InvalidToZeroD(other.RealTimePrice);  // RealTimePrice;                  //实时结算价
    }
};

////////////////////////////////////////////////
///分价位成交：分价位成交
////////////////////////////////////////////////
struct MDMarchPriceQty_MY
{
    INT1 Type;                           //行情域标识
    UINT4 Len;
    INT1 ContractID[80];                 //合约号
    REAL8 PriceOne;                       //价格
    UINT4 PriceOneBOQty;                  //买开数量
    UINT4 PriceOneBEQty;                  //买平数量
    UINT4 PriceOneSOQty;                  //卖开数量
    UINT4 PriceOneSEQty;                  //卖平数量
    REAL8 PriceTwo;                       //价格
    UINT4 PriceTwoBOQty;                  //买开数量
    UINT4 PriceTwoBEQty;                  //买平数量
    UINT4 PriceTwoSOQty;                  //卖开数量
    UINT4 PriceTwoSEQty;                  //卖平数量
    REAL8 PriceThree;                     //价格
    UINT4 PriceThreeBOQty;                //买开数量
    UINT4 PriceThreeBEQty;                //买平数量
    UINT4 PriceThreeSOQty;                //卖开数量
    UINT4 PriceThreeSEQty;                //卖平数量
    REAL8 PriceFour;                      //价格
    UINT4 PriceFourBOQty;                 //买开数量
    UINT4 PriceFourBEQty;                 //买平数量
    UINT4 PriceFourSOQty;                 //卖开数量
    UINT4 PriceFourSEQty;                 //卖平数量
    REAL8 PriceFive;                      //价格
    UINT4 PriceFiveBOQty;                 //买开数量
    UINT4 PriceFiveBEQty;                 //买平数量
    UINT4 PriceFiveSOQty;                 //卖开数量
    UINT4 PriceFiveSEQty;                 //卖平数量

    MDMarchPriceQty_MY()
    {
    }
    MDMarchPriceQty_MY(const DFITC_L2::MDMarchPriceQty &other)
    {
        Type = other.Type;           // Type;                           //行情域标识
        Len = other.Len;            // Len;
        memcpy(ContractID, other.ContractID, 80); // ContractID[80];                 //合约号
        PriceOne = InvalidToZeroD(other.PriceOne);       // PriceOne;                       //价格
        PriceOneBOQty = other.PriceOneBOQty;  // PriceOneBOQty;                  //买开数量
        PriceOneBEQty = other.PriceOneBEQty;  // PriceOneBEQty;                  //买平数量
        PriceOneSOQty = other.PriceOneSOQty;  // PriceOneSOQty;                  //卖开数量
        PriceOneSEQty = other.PriceOneSEQty;  // PriceOneSEQty;                  //卖平数量
        PriceTwo = InvalidToZeroD(other.PriceTwo);       // PriceTwo;                       //价格
        PriceTwoBOQty = other.PriceTwoBOQty;  // PriceTwoBOQty;                  //买开数量
        PriceTwoBEQty = other.PriceTwoBEQty;  // PriceTwoBEQty;                  //买平数量
        PriceTwoSOQty = other.PriceTwoSOQty;  // PriceTwoSOQty;                  //卖开数量
        PriceTwoSEQty = other.PriceTwoSEQty;  // PriceTwoSEQty;                  //卖平数量
        PriceThree = InvalidToZeroD(other.PriceThree);     // PriceThree;                     //价格
        PriceThreeBOQty = other.PriceThreeBOQty;     // PriceThreeBOQty;                //买开数量
        PriceThreeBEQty = other.PriceThreeBEQty;     // PriceThreeBEQty;                //买平数量
        PriceThreeSOQty = other.PriceThreeSOQty;     // PriceThreeSOQty;                //卖开数量
        PriceThreeSEQty = other.PriceThreeSEQty;     // PriceThreeSEQty;                //卖平数量
        PriceFour = InvalidToZeroD(other.PriceFour);      // PriceFour;                      //价格
        PriceFourBOQty = other.PriceFourBOQty; // PriceFourBOQty;                 //买开数量
        PriceFourBEQty = other.PriceFourBEQty; // PriceFourBEQty;                 //买平数量
        PriceFourSOQty = other.PriceFourSOQty; // PriceFourSOQty;                 //卖开数量
        PriceFourSEQty = other.PriceFourSEQty; // PriceFourSEQty;                 //卖平数量
        PriceFive = InvalidToZeroD(other.PriceFive);      // PriceFive;                      //价格
        PriceFiveBOQty = other.PriceFiveBOQty; // PriceFiveBOQty;                 //买开数量
        PriceFiveBEQty = other.PriceFiveBEQty; // PriceFiveBEQty;                 //买平数量
        PriceFiveSOQty = other.PriceFiveSOQty; // PriceFiveSOQty;                 //卖开数量
        PriceFiveSEQty = other.PriceFiveSEQty; // PriceFiveSEQty;                 //卖平数量
    }
};

#pragma pack(pop)

#endif  //QUOTE_DATATYPE_DCE_H_
