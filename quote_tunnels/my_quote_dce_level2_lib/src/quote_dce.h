#ifndef DCE_QUOTE_INTERFACE_H_
#define DCE_QUOTE_INTERFACE_H_

#include <string>
#include <sstream>
#include <list>
#include <iomanip>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "DFITCL2Api.h"

#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

#include "quote_interface_dce_level2.h"

class MYDCEDataHandler: public DFITC_L2::DFITCL2Spi
{
public:
    // 构造函数
    MYDCEDataHandler(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const MDBestAndDeep_MY *)> quote_data_handler);
    void SetQuoteDataHandler(boost::function<void(const MDTenEntrust_MY *)> quote_data_handler);
    void SetQuoteDataHandler(boost::function<void(const MDArbi_MY *)> quote_data_handler);
    void SetQuoteDataHandler(boost::function<void(const MDOrderStatistic_MY *)> quote_data_handler);
    void SetQuoteDataHandler(boost::function<void(const MDRealTimePrice_MY *)> quote_data_handler);
    void SetQuoteDataHandler(boost::function<void(const MDMarchPriceQty_MY *)> quote_data_handler);

    ~MYDCEDataHandler(void);

    virtual void OnRspUserLogin(struct DFITC_L2::ErrorRtnField * pErrorFiled);

    /**
     * 最优与五档深度行情消息应答:如果订阅行情成功且有行情返回时，该方法会被调用。
     * @param pQuote:指向最优与五档深度行情信息结构的指针，结构体中包含具体的行情信息。
     */
    virtual void OnBestAndDeep(DFITC_L2::MDBestAndDeep * const pQuote);

    /**
     * 套利行情消息应答:如果订阅行情成功且有行情返回时，该方法会被调用。
     * @param pQuote:套利行情信息结构的指针，结构体中包含具体的行情信息。
     */
    virtual void OnArbi(DFITC_L2::MDBestAndDeep * const pQuote);

    /**
     * 最优价位上十笔委托行情消息应答:如果订阅行情成功且有行情返回时，该方法会被调用。
     * @param pQuote:最优价位上十笔委托行情信息结构的指针，结构体中包含具体的行情信息。
     */
    virtual void OnTenEntrust(DFITC_L2::MDTenEntrust * const pQuote);

    /**
     * 实时结算价行情消息应答:如果订阅行情成功且有行情返回时，该方法会被调用。
     * @param pQuote:实时结算价行情信息结构的指针，结构体中包含具体的行情信息。
     */
    virtual void OnRealtime(DFITC_L2::MDRealTimePrice * const pQuote);

    /**
     * 加权平均及委托行情消息应答:如果订阅行情成功且有行情返回时，该方法会被调用。
     * @param pQuote:加权平均及委托行情信息结构的指针，结构体中包含具体的行情信息。
     */
    virtual void OnOrderStatistic(DFITC_L2::MDOrderStatistic * const pQuote);

    /**
     * 分价位成交行情消息应答:如果订阅行情成功且有行情返回时，该方法会被调用。
     * @param pQuote:分价位成交行情信息结构的指针，结构体中包含具体的行情信息。
     */
    virtual void OnMarchPrice(DFITC_L2::MDMarchPriceQty * const pQuote);

    /**
     * 连接断开响应:当api与level 2 server失去连接时此方法会被调用。api会自动重新连接，客户端可不做处理
     * /@param nReason 错误原因
     *        1 心跳超时
     *        2 网络断开
     *        3 收到错误报文
     */
    virtual void OnDisconnected(int pReason);

    // property
    bool Logoned() const
    {
        return logoned_;
    }

private:
    DFITC_L2::DFITCL2Api *api_;
    std::string quote_addr_;
    std::string user_;
    std::string pswd_;

    bool logoned_;

    // 数据处理函数对象
    boost::function<void(const MDBestAndDeep_MY *)> best_and_deep_handler_;
    boost::function<void(const MDTenEntrust_MY *)> ten_entrust_handler_;
    boost::function<void(const MDArbi_MY *)> arbi_handler_;
    boost::function<void(const MDOrderStatistic_MY *)> order_statistic_handler_;
    boost::function<void(const MDRealTimePrice_MY *)> realtime_price_handler_;
    boost::function<void(const MDMarchPriceQty_MY *)> march_price_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;
    bool ParseConfig();

    // save assistant object
    QuoteDataSave<MDBestAndDeep_MY> *p_save_best_and_deep_;
    QuoteDataSave<MDTenEntrust_MY> *p_save_ten_entrust_;
    QuoteDataSave<MDArbi_MY> *p_save_arbi_;
    QuoteDataSave<MDOrderStatistic_MY> *p_save_order_statistic_;
    QuoteDataSave<MDRealTimePrice_MY> *p_save_realtime_price_;
    QuoteDataSave<MDMarchPriceQty_MY> *p_save_march_price_qty_;

    // data convert functions
    MDBestAndDeep_MY Convert(const DFITC_L2::MDBestAndDeep &other);
    MDTenEntrust_MY Convert(const DFITC_L2::MDTenEntrust &other);
    MDArbi_MY Convert2Arbi(const DFITC_L2::MDBestAndDeep &other);
    MDOrderStatistic_MY Convert(const DFITC_L2::MDOrderStatistic &other);
    MDRealTimePrice_MY Convert(const DFITC_L2::MDRealTimePrice &other);
    MDMarchPriceQty_MY Convert(const DFITC_L2::MDMarchPriceQty &other);

    std::string qtm_name;
};

#endif // DCE_QUOTE_INTERFACE_H_
