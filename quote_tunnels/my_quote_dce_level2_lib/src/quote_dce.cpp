#include "quote_dce.h"
#include "qtm.h"

using namespace my_cmn;

// 行情回放有同步问题，自行处理
static boost::mutex quote_mutex;

using namespace std;
using namespace DFITC_L2;

MYDCEDataHandler::MYDCEDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg),
        p_save_best_and_deep_(NULL),
        p_save_ten_entrust_(NULL),
        p_save_arbi_(NULL),
        p_save_order_statistic_(NULL),
        p_save_realtime_price_(NULL),
        p_save_march_price_qty_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }
    qtm_name = "dce_level2_" + boost::lexical_cast<std::string>(getpid());
    QuoteUpdateState(qtm_name.c_str(), QtmState::INIT);

    // 初始化
    logoned_ = false;
    api_ = NULL;

    // 从配置解析参数
    if (!ParseConfig())
    {
        return;
    }

    p_save_best_and_deep_ = new QuoteDataSave<MDBestAndDeep_MY>(cfg_, qtm_name.c_str(), "bestanddeepquote", DCE_MDBESTANDDEEP_QUOTE_TYPE);
    p_save_ten_entrust_ = new QuoteDataSave<MDTenEntrust_MY>(cfg_, qtm_name.c_str(), "tenentrust", DCE_MDTENENTRUST_QUOTE_TYPE, false);
    p_save_arbi_ = new QuoteDataSave<MDArbi_MY>(cfg_, qtm_name.c_str(), "arbi", DCE_ARBI_QUOTE_TYPE, false);
    p_save_order_statistic_ = new QuoteDataSave<MDOrderStatistic_MY>(cfg_, qtm_name.c_str(), "orderstatistic", DCE_MDORDERSTATISTIC_QUOTE_TYPE, false);
    p_save_realtime_price_ = new QuoteDataSave<MDRealTimePrice_MY>(cfg_, qtm_name.c_str(), "realtimeprice", DCE_MDREALTIMEPRICE_QUOTE_TYPE, false);
    p_save_march_price_qty_ = new QuoteDataSave<MDMarchPriceQty_MY>(cfg_, qtm_name.c_str(), "marchpriceqty", DCE_MDMARCHPRICEQTY_QUOTE_TYPE, false);

    //创建Level-2行情API实例
    api_ = NEW_CONNECTOR();
    if (api_ == NULL)
    {
        MY_LOG_ERROR("DCE - IDCEMarketServiceApi::CreateInstance failed");
        return;
    }
    else
    {
        char *addr_tmp = new char[quote_addr_.size() + 1];
        memcpy(addr_tmp, quote_addr_.c_str(), quote_addr_.size() + 1);
        MY_LOG_INFO("prepare to connect quote provider: %s", quote_addr_.c_str());
        if (0 == api_->Connect(addr_tmp, this, 1))
        {
            MY_LOG_INFO("DCE - connect quote provider success");
            QuoteUpdateState(qtm_name.c_str(), QtmState::CONNECT_SUCCESS);
        }
        else
        {
            MY_LOG_ERROR("DCE - connect quote provider failed.");
            QuoteUpdateState(qtm_name.c_str(), QtmState::CONNECT_FAIL);
            return;
        }
        delete[] addr_tmp;
        boost::this_thread::sleep(boost::posix_time::millisec(100)); //稍等一会

        //登录服务器
        struct DFITCUserLoginField UserLogin;
        strncpy(UserLogin.accountID, user_.data(), sizeof(UserLogin.accountID) - 1);
        strncpy(UserLogin.passwd, pswd_.data(), sizeof(UserLogin.passwd) - 1);
        MY_LOG_INFO("DCE - prepare to login, account: %s; password: %s",
            UserLogin.accountID, UserLogin.passwd);

        if (0 != api_->ReqUserLogin(&UserLogin))
        {
            MY_LOG_ERROR("DCE - login failed.");
            QuoteUpdateState(qtm_name.c_str(), QtmState::LOG_ON_FAIL);
            return;
        }
    }
}

MYDCEDataHandler::~MYDCEDataHandler(void)
{
    if (api_)
    {
        //退出行情登录
        MY_LOG_INFO("DCE - cancel subscribe...");
        api_->UnSubscribeAll();

        MY_LOG_INFO("DCE - prepare logout...");
        DFITCUserLogoutField pReqUserLogoutField;
        pReqUserLogoutField.RequestID = 0;
        strncpy(pReqUserLogoutField.AccountID, user_.data(), sizeof(pReqUserLogoutField.AccountID));
        if (logoned_)
        {
            MY_LOG_INFO("DCE - logout...");
            api_->ReqUserLogout(&pReqUserLogoutField);
        }
        MY_LOG_INFO("DCE - finish logout...");

        boost::this_thread::sleep(boost::posix_time::millisec(100)); //稍等一会

        DELETE_CONNECTOR(api_);
    }

    if (p_save_best_and_deep_) delete p_save_best_and_deep_;
    if (p_save_ten_entrust_) delete p_save_ten_entrust_;
    if (p_save_arbi_) delete p_save_arbi_;
    if (p_save_order_statistic_) delete p_save_order_statistic_;
    if (p_save_realtime_price_) delete p_save_realtime_price_;
    if (p_save_march_price_qty_) delete p_save_march_price_qty_;
}

bool MYDCEDataHandler::ParseConfig()
{
    // 行情服务地址配置
    if (cfg_.Logon_config().quote_provider_addrs.empty())
    {
        MY_LOG_ERROR("DCE - no configure quote provider address");
        return false;
    }
    // tcp://192.168.60.23:7120
    quote_addr_ = cfg_.Logon_config().quote_provider_addrs.front();

    // 用户密码
    user_ = cfg_.Logon_config().account;
    pswd_ = cfg_.Logon_config().password;

    // 检查
    if (quote_addr_.empty())
    {
        MY_LOG_ERROR("DCE - no configure quote provider address");
        return false;
    }

    return true;
}

void MYDCEDataHandler::OnRspUserLogin(struct DFITC_L2::ErrorRtnField * pErrorFiled)
{
    logoned_ = true;
    QuoteUpdateState(qtm_name.c_str(), QtmState::LOG_ON_SUCCESS);
    if (cfg_.Subscribe_datas().empty()) {
        if (0 == api_->SubscribeAll())
        {
            QuoteUpdateState(qtm_name.c_str(), QtmState::API_READY);
            MY_LOG_INFO("DCE - subscribe quote success.");
        }
        else
        {
            MY_LOG_ERROR("DCE - subscribe quote failed.");
        }
    } else {
        int count = cfg_.Subscribe_datas().size();
        char ** contract_ary = new char *[count];
        int i = 0;
        for (auto it = cfg_.Subscribe_datas().begin(); it != cfg_.Subscribe_datas().end(); ++it, ++i) {
            contract_ary[i] = new char[it->length() + 1];
            strcpy(contract_ary[i], it->c_str());
        }
        if (api_->SubscribeMarketData(contract_ary, count) == 0)
        {
            QuoteUpdateState(qtm_name.c_str(), QtmState::API_READY);
            MY_LOG_INFO("DCE - subscribe quote success.");
        }
        else
        {
            MY_LOG_ERROR("DCE - subscribe quote failed.");
        }
    }
}

void MYDCEDataHandler::OnDisconnected(int)
{
    QuoteUpdateState(qtm_name.c_str(), QtmState::DISCONNECT);
}

void MYDCEDataHandler::OnBestAndDeep(DFITC_L2::MDBestAndDeep * const p)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);
        MDBestAndDeep_MY data_my = Convert(*p);

        // 发出去
        if (best_and_deep_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->Contract) != subscribe_contracts_.end()))
        {
            best_and_deep_handler_(&data_my);
        }

        // 存起来
        p_save_best_and_deep_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
    }
    catch (...)
    {
        MY_LOG_FATAL("DCE - Unknown exception in OnBestAndDeep.");
    }
}

void MYDCEDataHandler::OnTenEntrust(DFITC_L2::MDTenEntrust * const p)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);
        MDTenEntrust_MY data_my = Convert(*p);

        // 发出去
        if (ten_entrust_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->Contract) != subscribe_contracts_.end()))
        {
            ten_entrust_handler_(&data_my);
        }

        // 存起来
        p_save_ten_entrust_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
    }
    catch (...)
    {
        MY_LOG_FATAL("DCE - Unknown exception in OnTenEntrust.");
    }
}

void MYDCEDataHandler::OnArbi(DFITC_L2::MDBestAndDeep * const p)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);
        MDArbi_MY data_my = Convert2Arbi(*p);

        // 发出去
        if (arbi_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->Contract) != subscribe_contracts_.end()))
        {
            arbi_handler_(&data_my);
        }

        // 存起来
        p_save_arbi_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
    }
    catch (...)
    {
        MY_LOG_FATAL("DCE - Unknown exception in OnArbi.");
    }
}

void MYDCEDataHandler::OnRealtime(DFITC_L2::MDRealTimePrice * const p)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);
        MDRealTimePrice_MY data_my = Convert(*p);

        // 发出去
        if (realtime_price_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->ContractID) != subscribe_contracts_.end()))
        {
            realtime_price_handler_(&data_my);
        }

        // 存起来
        p_save_realtime_price_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
    }
    catch (...)
    {
        MY_LOG_FATAL("DCE - Unknown exception in OnRealtime.");
    }
}

void MYDCEDataHandler::OnOrderStatistic(DFITC_L2::MDOrderStatistic * const p)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);
        MDOrderStatistic_MY data_my = Convert(*p);

        // 发出去
        if (order_statistic_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->ContractID) != subscribe_contracts_.end()))
        {
            order_statistic_handler_(&data_my);
        }

        // 存起来
        p_save_order_statistic_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
    }
    catch (...)
    {
        MY_LOG_FATAL("DCE - Unknown exception in OnOrderStatistic.");
    }
}

void MYDCEDataHandler::OnMarchPrice(DFITC_L2::MDMarchPriceQty * const p)
{
    try
    {
        timeval t;
        gettimeofday(&t, NULL);
        MDMarchPriceQty_MY data_my = Convert(*p);

        // 发出去
        if (march_price_handler_
            && (subscribe_contracts_.empty() || subscribe_contracts_.find(p->ContractID) != subscribe_contracts_.end()))
        {
            march_price_handler_(&data_my);
        }

        // 存起来
        p_save_march_price_qty_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data_my);
    }
    catch (...)
    {
        MY_LOG_FATAL("DCE - Unknown exception in OnMarchPrice.");
    }
}

void MYDCEDataHandler::SetQuoteDataHandler(boost::function<void(const MDBestAndDeep_MY *)> quote_data_handler)
{
    best_and_deep_handler_ = quote_data_handler;
}

void MYDCEDataHandler::SetQuoteDataHandler(boost::function<void(const MDTenEntrust_MY *)> quote_data_handler)
{
    ten_entrust_handler_ = quote_data_handler;
}

void MYDCEDataHandler::SetQuoteDataHandler(boost::function<void(const MDArbi_MY *)> quote_data_handler)
{
    arbi_handler_ = quote_data_handler;
}

void MYDCEDataHandler::SetQuoteDataHandler(boost::function<void(const MDOrderStatistic_MY *)> quote_data_handler)
{
    order_statistic_handler_ = quote_data_handler;
}

void MYDCEDataHandler::SetQuoteDataHandler(boost::function<void(const MDRealTimePrice_MY *)> quote_data_handler)
{
    realtime_price_handler_ = quote_data_handler;
}

void MYDCEDataHandler::SetQuoteDataHandler(boost::function<void(const MDMarchPriceQty_MY *)> quote_data_handler)
{
    march_price_handler_ = quote_data_handler;
}

MDBestAndDeep_MY MYDCEDataHandler::Convert(const DFITC_L2::MDBestAndDeep &other)
{
    MDBestAndDeep_MY data;

    data.Type = other.Type;             // Type;
    data.Length = other.Length;           // Length;                         //报文长度
    data.Version = other.Version;          // Version;                        //版本从1开始
    data.Time = other.Time;             // Time;                           //预留字段
    memcpy(data.Exchange, other.Exchange, 3);      // Exchange[3];                    //交易所
    memcpy(data.Contract, other.Contract, 80);     // Contract[80];                   //合约代码
    data.SuspensionSign = other.SuspensionSign;   // SuspensionSign;                 //停牌标志
    data.LastClearPrice = InvalidToZeroF(other.LastClearPrice);   // LastClearPrice;                 //昨结算价
    data.ClearPrice = InvalidToZeroF(other.ClearPrice);       // ClearPrice;                     //今结算价
    data.AvgPrice = InvalidToZeroF(other.AvgPrice);         // AvgPrice;                       //成交均价
    data.LastClose = InvalidToZeroF(other.LastClose);        // LastClose;                      //昨收盘
    data.Close = InvalidToZeroF(other.Close);            // Close;                          //今收盘
    data.OpenPrice = InvalidToZeroF(other.OpenPrice);        // OpenPrice;                      //今开盘
    data.LastOpenInterest = other.LastOpenInterest; // LastOpenInterest;               //昨持仓量
    data.OpenInterest = other.OpenInterest;     // OpenInterest;                   //持仓量
    data.LastPrice = InvalidToZeroF(other.LastPrice);        // LastPrice;                      //最新价
    data.MatchTotQty = other.MatchTotQty;      // MatchTotQty;                    //成交数量
    data.Turnover = InvalidToZeroD(other.Turnover);         // Turnover;                       //成交金额
    data.RiseLimit = InvalidToZeroF(other.RiseLimit);        // RiseLimit;                      //最高报价
    data.FallLimit = InvalidToZeroF(other.FallLimit);        // FallLimit;                      //最低报价
    data.HighPrice = InvalidToZeroF(other.HighPrice);        // HighPrice;                      //最高价
    data.LowPrice = InvalidToZeroF(other.LowPrice);         // LowPrice;                       //最低价
    data.PreDelta = InvalidToZeroF(other.PreDelta);         // PreDelta;                       //昨虚实度
    data.CurrDelta = InvalidToZeroF(other.CurrDelta);        // CurrDelta;                      //今虚实度

    data.BuyPriceOne = InvalidToZeroF(other.BuyPriceOne);      // BuyPriceOne;                    //买入价格1
    data.BuyQtyOne = other.BuyQtyOne;        // BuyQtyOne;                      //买入数量1
    data.BuyImplyQtyOne = other.BuyImplyQtyOne;
    data.BuyPriceTwo = InvalidToZeroF(other.BuyPriceTwo);      // BuyPriceTwo;
    data.BuyQtyTwo = other.BuyQtyTwo;        // BuyQtyTwo;
    data.BuyImplyQtyTwo = other.BuyImplyQtyTwo;
    data.BuyPriceThree = InvalidToZeroF(other.BuyPriceThree);    // BuyPriceThree;
    data.BuyQtyThree = other.BuyQtyThree;      // BuyQtyThree;
    data.BuyImplyQtyThree = other.BuyImplyQtyThree;
    data.BuyPriceFour = InvalidToZeroF(other.BuyPriceFour);     // BuyPriceFour;
    data.BuyQtyFour = other.BuyQtyFour;       // BuyQtyFour;
    data.BuyImplyQtyFour = other.BuyImplyQtyFour;
    data.BuyPriceFive = InvalidToZeroF(other.BuyPriceFive);     // BuyPriceFive;
    data.BuyQtyFive = other.BuyQtyFive;       // BuyQtyFive;
    data.BuyImplyQtyFive = other.BuyImplyQtyFive;
    data.SellPriceOne = InvalidToZeroF(other.SellPriceOne);     // SellPriceOne;                   //卖出价格1
    data.SellQtyOne = other.SellQtyOne;       // SellQtyOne;                     //买出数量1
    data.SellImplyQtyOne = other.SellImplyQtyOne;
    data.SellPriceTwo = InvalidToZeroF(other.SellPriceTwo);     // SellPriceTwo;
    data.SellQtyTwo = other.SellQtyTwo;       // SellQtyTwo;
    data.SellImplyQtyTwo = other.SellImplyQtyTwo;
    data.SellPriceThree = InvalidToZeroF(other.SellPriceThree);   // SellPriceThree;
    data.SellQtyThree = other.SellQtyThree;     // SellQtyThree;
    data.SellImplyQtyThree = other.SellImplyQtyThree;
    data.SellPriceFour = InvalidToZeroF(other.SellPriceFour);    // SellPriceFour;
    data.SellQtyFour = other.SellQtyFour;      // SellQtyFour;
    data.SellImplyQtyFour = other.SellImplyQtyFour;
    data.SellPriceFive = InvalidToZeroF(other.SellPriceFive);    // SellPriceFive;
    data.SellQtyFive = other.SellQtyFive;      // SellQtyFive;
    data.SellImplyQtyFive = other.SellImplyQtyFive;

    memcpy(data.GenTime, other.GenTime, 13);      // GenTime[13];                    //行情产生时间
    data.LastMatchQty = other.LastMatchQty;     // LastMatchQty;                   //最新成交量
    data.InterestChg = other.InterestChg;      // InterestChg;                    //持仓量变化
    data.LifeLow = InvalidToZeroF(other.LifeLow);          // LifeLow;                        //历史最低价
    data.LifeHigh = InvalidToZeroF(other.LifeHigh);         // LifeHigh;                       //历史最高价
    data.Delta = InvalidToZeroD(other.Delta);            // Delta;                          //delta
    data.Gamma = InvalidToZeroD(other.Gamma);            // Gamma;                          //gama
    data.Rho = InvalidToZeroD(other.Rho);              // Rho;                            //rho
    data.Theta = InvalidToZeroD(other.Theta);            // Theta;                          //theta
    data.Vega = InvalidToZeroD(other.Vega);             // Vega;                           //vega
    memcpy(data.TradeDate, other.TradeDate, 9);     // TradeDate[9];                   //行情日期
    memcpy(data.LocalDate, other.LocalDate, 9);     // LocalDate[9];                   //本地日期

    return data;
}

MDTenEntrust_MY MYDCEDataHandler::Convert(const DFITC_L2::MDTenEntrust &other)
{
    MDTenEntrust_MY data;

    data.Type = other.Type;                  // Type;                           //行情域标识
    data.Len = other.Len;                   // Len;
    memcpy(data.Contract, other.Contract, 80);          // Contract[80];                   //合约代码
    data.BestBuyOrderPrice = InvalidToZeroD(other.BestBuyOrderPrice);     // BestBuyOrderPrice;              //价格
    data.BestBuyOrderQtyOne = other.BestBuyOrderQtyOne;    // BestBuyOrderQtyOne;             //委托量1
    data.BestBuyOrderQtyTwo = other.BestBuyOrderQtyTwo;    // BestBuyOrderQtyTwo;             //委托量2
    data.BestBuyOrderQtyThree = other.BestBuyOrderQtyThree;  // BestBuyOrderQtyThree;           //委托量3
    data.BestBuyOrderQtyFour = other.BestBuyOrderQtyFour;   // BestBuyOrderQtyFour;            //委托量4
    data.BestBuyOrderQtyFive = other.BestBuyOrderQtyFive;   // BestBuyOrderQtyFive;            //委托量5
    data.BestBuyOrderQtySix = other.BestBuyOrderQtySix;    // BestBuyOrderQtySix;             //委托量6
    data.BestBuyOrderQtySeven = other.BestBuyOrderQtySeven;  // BestBuyOrderQtySeven;           //委托量7
    data.BestBuyOrderQtyEight = other.BestBuyOrderQtyEight;  // BestBuyOrderQtyEight;           //委托量8
    data.BestBuyOrderQtyNine = other.BestBuyOrderQtyNine;   // BestBuyOrderQtyNine;            //委托量9
    data.BestBuyOrderQtyTen = other.BestBuyOrderQtyTen;    // BestBuyOrderQtyTen;             //委托量10
    data.BestSellOrderPrice = InvalidToZeroD(other.BestSellOrderPrice);    // BestSellOrderPrice;             //价格
    data.BestSellOrderQtyOne = other.BestSellOrderQtyOne;   // BestSellOrderQtyOne;            //委托量1
    data.BestSellOrderQtyTwo = other.BestSellOrderQtyTwo;   // BestSellOrderQtyTwo;            //委托量2
    data.BestSellOrderQtyThree = other.BestSellOrderQtyThree; // BestSellOrderQtyThree;          //委托量3
    data.BestSellOrderQtyFour = other.BestSellOrderQtyFour;  // BestSellOrderQtyFour;           //委托量4
    data.BestSellOrderQtyFive = other.BestSellOrderQtyFive;  // BestSellOrderQtyFive;           //委托量5
    data.BestSellOrderQtySix = other.BestSellOrderQtySix;   // BestSellOrderQtySix;            //委托量6
    data.BestSellOrderQtySeven = other.BestSellOrderQtySeven; // BestSellOrderQtySeven;          //委托量7
    data.BestSellOrderQtyEight = other.BestSellOrderQtyEight; // BestSellOrderQtyEight;          //委托量8
    data.BestSellOrderQtyNine = other.BestSellOrderQtyNine;  // BestSellOrderQtyNine;           //委托量9
    data.BestSellOrderQtyTen = other.BestSellOrderQtyTen;   // BestSellOrderQtyTen;            //委托量10
    memcpy(data.GenTime, other.GenTime, 13);           // GenTime[13];                    //生成时间

    return data;
}

MDArbi_MY MYDCEDataHandler::Convert2Arbi(const DFITC_L2::MDBestAndDeep &other)
{
    MDArbi_MY data;

    data.Type = other.Type;             // Type;
    data.Length = other.Length;           // Length;                         //报文长度
    data.Version = other.Version;          // Version;                        //版本从1开始
    data.Time = other.Time;             // Time;                           //预留字段
    memcpy(data.Exchange, other.Exchange, 3);      // Exchange[3];                    //交易所
    memcpy(data.Contract, other.Contract, 80);     // Contract[80];                   //合约代码
    data.SuspensionSign = other.SuspensionSign;   // SuspensionSign;                 //停牌标志
    data.LastClearPrice = InvalidToZeroF(other.LastClearPrice);   // LastClearPrice;                 //昨结算价
    data.ClearPrice = InvalidToZeroF(other.ClearPrice);       // ClearPrice;                     //今结算价
    data.AvgPrice = InvalidToZeroF(other.AvgPrice);         // AvgPrice;                       //成交均价
    data.LastClose = InvalidToZeroF(other.LastClose);        // LastClose;                      //昨收盘
    data.Close = InvalidToZeroF(other.Close);            // Close;                          //今收盘
    data.OpenPrice = InvalidToZeroF(other.OpenPrice);        // OpenPrice;                      //今开盘
    data.LastOpenInterest = other.LastOpenInterest; // LastOpenInterest;               //昨持仓量
    data.OpenInterest = other.OpenInterest;     // OpenInterest;                   //持仓量
    data.LastPrice = InvalidToZeroF(other.LastPrice);        // LastPrice;                      //最新价
    data.MatchTotQty = other.MatchTotQty;      // MatchTotQty;                    //成交数量
    data.Turnover = InvalidToZeroD(other.Turnover);         // Turnover;                       //成交金额
    data.RiseLimit = InvalidToZeroF(other.RiseLimit);        // RiseLimit;                      //最高报价
    data.FallLimit = InvalidToZeroF(other.FallLimit);        // FallLimit;                      //最低报价
    data.HighPrice = InvalidToZeroF(other.HighPrice);        // HighPrice;                      //最高价
    data.LowPrice = InvalidToZeroF(other.LowPrice);         // LowPrice;                       //最低价
    data.PreDelta = InvalidToZeroF(other.PreDelta);         // PreDelta;                       //昨虚实度
    data.CurrDelta = InvalidToZeroF(other.CurrDelta);        // CurrDelta;                      //今虚实度

    data.BuyPriceOne = InvalidToZeroF(other.BuyPriceOne);      // BuyPriceOne;                    //买入价格1
    data.BuyQtyOne = other.BuyQtyOne;        // BuyQtyOne;                      //买入数量1
    data.BuyImplyQtyOne = other.BuyImplyQtyOne;
    data.BuyPriceTwo = InvalidToZeroF(other.BuyPriceTwo);      // BuyPriceTwo;
    data.BuyQtyTwo = other.BuyQtyTwo;        // BuyQtyTwo;
    data.BuyImplyQtyTwo = other.BuyImplyQtyTwo;
    data.BuyPriceThree = InvalidToZeroF(other.BuyPriceThree);    // BuyPriceThree;
    data.BuyQtyThree = other.BuyQtyThree;      // BuyQtyThree;
    data.BuyImplyQtyThree = other.BuyImplyQtyThree;
    data.BuyPriceFour = InvalidToZeroF(other.BuyPriceFour);     // BuyPriceFour;
    data.BuyQtyFour = other.BuyQtyFour;       // BuyQtyFour;
    data.BuyImplyQtyFour = other.BuyImplyQtyFour;
    data.BuyPriceFive = InvalidToZeroF(other.BuyPriceFive);     // BuyPriceFive;
    data.BuyQtyFive = other.BuyQtyFive;       // BuyQtyFive;
    data.BuyImplyQtyFive = other.BuyImplyQtyFive;
    data.SellPriceOne = InvalidToZeroF(other.SellPriceOne);     // SellPriceOne;                   //卖出价格1
    data.SellQtyOne = other.SellQtyOne;       // SellQtyOne;                     //买出数量1
    data.SellImplyQtyOne = other.SellImplyQtyOne;
    data.SellPriceTwo = InvalidToZeroF(other.SellPriceTwo);     // SellPriceTwo;
    data.SellQtyTwo = other.SellQtyTwo;       // SellQtyTwo;
    data.SellImplyQtyTwo = other.SellImplyQtyTwo;
    data.SellPriceThree = InvalidToZeroF(other.SellPriceThree);   // SellPriceThree;
    data.SellQtyThree = other.SellQtyThree;     // SellQtyThree;
    data.SellImplyQtyThree = other.SellImplyQtyThree;
    data.SellPriceFour = InvalidToZeroF(other.SellPriceFour);    // SellPriceFour;
    data.SellQtyFour = other.SellQtyFour;      // SellQtyFour;
    data.SellImplyQtyFour = other.SellImplyQtyFour;
    data.SellPriceFive = InvalidToZeroF(other.SellPriceFive);    // SellPriceFive;
    data.SellQtyFive = other.SellQtyFive;      // SellQtyFive;
    data.SellImplyQtyFive = other.SellImplyQtyFive;

    memcpy(data.GenTime, other.GenTime, 13);      // GenTime[13];                    //行情产生时间
    data.LastMatchQty = other.LastMatchQty;     // LastMatchQty;                   //最新成交量
    data.InterestChg = other.InterestChg;      // InterestChg;                    //持仓量变化
    data.LifeLow = InvalidToZeroF(other.LifeLow);          // LifeLow;                        //历史最低价
    data.LifeHigh = InvalidToZeroF(other.LifeHigh);         // LifeHigh;                       //历史最高价
    data.Delta = InvalidToZeroD(other.Delta);            // Delta;                          //delta
    data.Gamma = InvalidToZeroD(other.Gamma);            // Gamma;                          //gama
    data.Rho = InvalidToZeroD(other.Rho);              // Rho;                            //rho
    data.Theta = InvalidToZeroD(other.Theta);            // Theta;                          //theta
    data.Vega = InvalidToZeroD(other.Vega);             // Vega;                           //vega
    memcpy(data.TradeDate, other.TradeDate, 9);     // TradeDate[9];                   //行情日期
    memcpy(data.LocalDate, other.LocalDate, 9);     // LocalDate[9];                   //本地日期

    return data;
}
MDOrderStatistic_MY MYDCEDataHandler::Convert(const DFITC_L2::MDOrderStatistic &other)
{
    MDOrderStatistic_MY data;

    data.Type = other.Type;                         // Type;                           //行情域标识
    data.Len = other.Len;                          // Len;
    memcpy(data.ContractID, other.ContractID, 80);               // ContractID[80];                 //合约号
    data.TotalBuyOrderNum = other.TotalBuyOrderNum;             // TotalBuyOrderNum;               //买委托总量
    data.TotalSellOrderNum = other.TotalSellOrderNum;            // TotalSellOrderNum;              //卖委托总量
    data.WeightedAverageBuyOrderPrice = InvalidToZeroD(other.WeightedAverageBuyOrderPrice); // WeightedAverageBuyOrderPrice;   //加权平均委买价格
    data.WeightedAverageSellOrderPrice = InvalidToZeroD(other.WeightedAverageSellOrderPrice); // WeightedAverageSellOrderPrice;  //加权平均委卖价格

    return data;
}
MDRealTimePrice_MY MYDCEDataHandler::Convert(const DFITC_L2::MDRealTimePrice &other)
{
    MDRealTimePrice_MY data;

    data.Type = other.Type;           // Type;                           //行情域标识
    data.Len = other.Len;            // Len;
    memcpy(data.ContractID, other.ContractID, 80); // ContractID[80];                 //合约号
    data.RealTimePrice = InvalidToZeroD(other.RealTimePrice);  // RealTimePrice;                  //实时结算价

    return data;
}
MDMarchPriceQty_MY MYDCEDataHandler::Convert(const DFITC_L2::MDMarchPriceQty &other)
{
    MDMarchPriceQty_MY data;

    data.Type = other.Type;           // Type;                           //行情域标识
    data.Len = other.Len;            // Len;
    memcpy(data.ContractID, other.ContractID, 80); // ContractID[80];                 //合约号
    data.PriceOne = InvalidToZeroD(other.PriceOne);       // PriceOne;                       //价格
    data.PriceOneBOQty = other.PriceOneBOQty;  // PriceOneBOQty;                  //买开数量
    data.PriceOneBEQty = other.PriceOneBEQty;  // PriceOneBEQty;                  //买平数量
    data.PriceOneSOQty = other.PriceOneSOQty;  // PriceOneSOQty;                  //卖开数量
    data.PriceOneSEQty = other.PriceOneSEQty;  // PriceOneSEQty;                  //卖平数量
    data.PriceTwo = InvalidToZeroD(other.PriceTwo);       // PriceTwo;                       //价格
    data.PriceTwoBOQty = other.PriceTwoBOQty;  // PriceTwoBOQty;                  //买开数量
    data.PriceTwoBEQty = other.PriceTwoBEQty;  // PriceTwoBEQty;                  //买平数量
    data.PriceTwoSOQty = other.PriceTwoSOQty;  // PriceTwoSOQty;                  //卖开数量
    data.PriceTwoSEQty = other.PriceTwoSEQty;  // PriceTwoSEQty;                  //卖平数量
    data.PriceThree = InvalidToZeroD(other.PriceThree);     // PriceThree;                     //价格
    data.PriceThreeBOQty = other.PriceThreeBOQty;     // PriceThreeBOQty;                //买开数量
    data.PriceThreeBEQty = other.PriceThreeBEQty;     // PriceThreeBEQty;                //买平数量
    data.PriceThreeSOQty = other.PriceThreeSOQty;     // PriceThreeSOQty;                //卖开数量
    data.PriceThreeSEQty = other.PriceThreeSEQty;     // PriceThreeSEQty;                //卖平数量
    data.PriceFour = InvalidToZeroD(other.PriceFour);      // PriceFour;                      //价格
    data.PriceFourBOQty = other.PriceFourBOQty; // PriceFourBOQty;                 //买开数量
    data.PriceFourBEQty = other.PriceFourBEQty; // PriceFourBEQty;                 //买平数量
    data.PriceFourSOQty = other.PriceFourSOQty; // PriceFourSOQty;                 //卖开数量
    data.PriceFourSEQty = other.PriceFourSEQty; // PriceFourSEQty;                 //卖平数量
    data.PriceFive = InvalidToZeroD(other.PriceFive);      // PriceFive;                      //价格
    data.PriceFiveBOQty = other.PriceFiveBOQty; // PriceFiveBOQty;                 //买开数量
    data.PriceFiveBEQty = other.PriceFiveBEQty; // PriceFiveBEQty;                 //买平数量
    data.PriceFiveSOQty = other.PriceFiveSOQty; // PriceFiveSOQty;                 //卖开数量
    data.PriceFiveSEQty = other.PriceFiveSEQty; // PriceFiveSEQty;                 //卖平数量

    return data;
}

