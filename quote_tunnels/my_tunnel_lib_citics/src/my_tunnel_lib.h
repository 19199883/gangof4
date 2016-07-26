#if !defined(MY_TUNNEL_LIB_H_)
#define MY_TUNNEL_LIB_H_

#include <vector>
#include <functional>
#include "my_trade_tunnel_api.h"
#include "trade_log_util.h"
//#include "my_exchange_datatype.h"

#define DLL_PUBLIC __attribute__ ((visibility ("default")))

// 前向声明
class TunnelConfigData;

typedef char StockCodeType[64];
typedef long SerialNoType;
typedef long VolumeType;
typedef long EntrustNoType;

struct DLL_PUBLIC T_InternalOrder
{
	T_InternalOrder(){
		volume = 0;
		entrust_no = 0;
		last_px = 0.0;
		last_vol = 0;
		acc_vol = 0;
		terminated = false;
	}

    SerialNoType serial_no;   	// 报单序列号
    StockCodeType stock_code;  	// 合约代码
    double limit_price; 		// 价格
    char direction;   			// '0': buy; '1': sell
    char open_close;  			// '0': open; '1': close
    char speculator;  			// '0': speculation; '1': hedge; '2':arbitrage;
    VolumeType volume;      	// 手数
    char order_kind;  			// '0': limit price; '1': market price
    char order_type;  			// '0': normal; '1': fak;

    // Improvement
    char exchange_type;

    EntrustNoType entrust_no;

    double last_px;
    long last_vol;
    long acc_vol;
    char status;
    bool terminated;
};

class DLL_PUBLIC MYTunnel : public MYTunnelInterface
{
public:
    /**
     * @param provider_config_file 行情的配置文件名
     *                  配置文件为空时，默认使用 my_quote_lib.config。
     */
    MYTunnel(const std::string &provider_config_file);

    virtual void PlaceOrder(const T_PlaceOrder *pPlaceOrder);
    virtual void CancelOrder(const T_CancelOrder *pCancelOrder);
    virtual  void QueryPosition(const T_QryPosition *pQryPosition);

    virtual void QueryOrderDetail(const T_QryOrderDetail *pQryParam){}

    virtual void QueryTradeDetail(const T_QryTradeDetail *pQryParam);
	virtual void QueryContractInfo(const T_QryContractInfo *pQryParam);
    // 新增做市接口
	 ///询价录入请求
    virtual void ReqForQuoteInsert(const T_ReqForQuote *p); 
	///报价录入请求
    virtual  void ReqQuoteInsert(const T_InsertQuote *p);   
    ///报价操作请求
    virtual void ReqQuoteAction(const T_CancelQuote *p);

    /**
     * @param Callback_handler 函数对象。
     */
    virtual void SetCallbackHandler(std::function<void(const T_OrderRespond *)> callback_handler);
    virtual void SetCallbackHandler(std::function<void(const T_CancelRespond *)> callback_handler);
    virtual void SetCallbackHandler(std::function<void(const T_OrderReturn *)> callback_handler);
    virtual void SetCallbackHandler(std::function<void(const T_TradeReturn *)> callback_handler);

    virtual void SetCallbackHandler(std::function<void(const T_PositionReturn *)> handler);
    virtual void SetCallbackHandler(std::function<void(const T_OrderDetailReturn *)> handler);
    virtual void SetCallbackHandler(std::function<void(const T_TradeDetailReturn *)> handler);

    // 新增做市接口
    virtual void SetCallbackHandler(std::function<void(const T_RtnForQuote *)> handler);
    virtual void SetCallbackHandler(std::function<void(const T_RspOfReqForQuote *)> handler);
    virtual void SetCallbackHandler(std::function<void(const T_InsertQuoteRespond *)> handler);
    virtual void SetCallbackHandler(std::function<void(const T_CancelQuoteRespond *)> handler);
    virtual void SetCallbackHandler(std::function<void(const T_QuoteReturn *)> handler);
    virtual void SetCallbackHandler(std::function<void(const T_QuoteTrade *)> handler);

    virtual std::string GetClientID();


      
      virtual void SetCallbackHandler(std::function<void(const T_ContractInfoReturn *)> handler);

    virtual ~MYTunnel();

private:
    // 禁止拷贝和赋值
    //MYTunnel(const MYTunnel & other);
    //MYTunnel operator=(const MYTunnel & other);

    // 内部实现接口
    bool InitInf(const TunnelConfigData &cfg);

    void *trade_inf_;

    char exchange_code_;
    std::string user_id_;
    Tunnel_Info tunnel_info_;

    // 配置数据对象
    TunnelConfigData *lib_cfg_;

    // 数据处理函数对象
    std::function<void(const T_OrderRespond *)> OrderRespond_call_back_handler_;
    std::function<void(const T_CancelRespond *)> CancelRespond_call_back_handler_;
    std::function<void(const T_PositionReturn *)> QryPosReturnHandler_;
    std::function<void(const T_OrderDetailReturn *)> QryOrderDetailReturnHandler_;
    std::function<void(const T_TradeDetailReturn *)> QryTradeDetailReturnHandler_;

    std::function<void(const T_RspOfReqForQuote *)> RspOfReqForQuoteHandler_;
    std::function<void(const T_InsertQuoteRespond *)> InsertQuoteRespondHandler_;
    std::function<void(const T_CancelQuoteRespond *)> CancelQuoteRespondHandler_;

    // pending respond, should return in another thread
    std::vector<std::pair<int, void *> > pending_rsp_;
    void SendRespondAsync(int rsp_type, void *rsp);
    static void RespondHandleThread(MYTunnel *ptunnel);
};

 //the class factories
//extern "C" MYTunnelInterface* CreateTradeTunnel(const std::string &provider_config_file);
//
//extern "C" void DestroyTradeTunnel(MYTunnel* p) ;

#endif  //MY_TUNNEL_LIB_H_
