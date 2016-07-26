/*
 * interface of my_exchange
 * trade interface:
 *   place order
 *   cancel order
 * position interface:
 *   get position
 */

#ifndef _MY_EXCHANGE_H_
#define  _MY_EXCHANGE_H_

#include "my_exchange_datatype.h"

#include <mutex>
#include <functional>
#include "my_trade_tunnel_api.h"

typedef std::function<void(const T_OrderRespond *)> OrderRespondHandler;
typedef std::function<void(const T_CancelRespond *)> CancelRespondHandler;
typedef std::function<void(const T_OrderReturn *)> OrderReturnHandler;
typedef std::function<void(const T_TradeReturn *)> TradeReturnHandler;
typedef std::function<void(const T_PositionReturn *)> PositionReturnHandler;

typedef std::function<void(const T_OrderDetailReturn *)> OrderDetailReturnHandler;
typedef std::function<void(const T_TradeDetailReturn *)> TradeDetailReturnHandler;

// modify on 20141202, for support push position data
typedef std::function<void(const T_OrderRespond *, const T_PositionData *)> OrderRespondWithPosHandler;
typedef std::function<void(const T_OrderReturn *, const T_PositionData *)> OrderReturnWithPosHandler;
typedef std::function<void(const T_TradeReturn *, const T_PositionData *)> TradeReturnWithPosHandler;

// added on 20141216, for support MM(market making)
// 询价响应
typedef std::function<void(const T_RspOfReqForQuote *)> RspOfReqForQuoteHandler;
// 询价通知
typedef std::function<void(const T_RtnForQuote *)> RtnForQuoteHandler;
//  撤销报价应答
typedef std::function<void(const T_CancelQuoteRespond *)> CancelQuoteRspHandler;
//  报价应答
typedef std::function<void(const T_InsertQuoteRespond *)> InsertQuoteRspHandler;
typedef std::function<void(const T_InsertQuoteRespond *, const T_PositionData *)> InsertQuoteRspWithPosHandler;
//  报价回报
typedef std::function<void(const T_QuoteReturn *)> QuoteReturnHandler;
typedef std::function<void(const T_QuoteReturn *, const T_PositionData *)> QuoteReturnWithPosHandler;
//  报价成交回报
typedef std::function<void(const T_QuoteTrade *)> QuoteTradeReturnHandler;
typedef std::function<void(const T_QuoteTrade *, const T_PositionData *)> QuoteTradeReturnWithPosHandler;
typedef std::function<void(const T_ContractInfoReturn *)> ContractInfoReturnHandler;

// forward declare
class MYOrderDataManager;
class MYOrderReq;
class MYOrderRsp;
class MYPositionDataManager;

//class MYTunnel;
class MYExConfigData;
class OrderInfo;

// MYExchange inner implement class, for hide detail
class MYExchangeInnerImp;

class DLL_PUBLIC MYExchange : public MYExchangeInterface
{
public:
    MYExchange(struct my_xchg_cfg& cfg);
    virtual ~MYExchange();

    // interface for trade
    virtual void PlaceOrder(const T_PlaceOrder *pPlaceOrder);
    virtual void CancelOrder(const T_CancelOrder *pCancelOrder);
    virtual void QueryPosition(const T_QryPosition *pQryPosition);
    virtual  void QueryOrderDetail(const T_QryOrderDetail *pQryParam);
    virtual  void QueryTradeDetail(const T_QryTradeDetail *pQryParam);

    // added on 20141216, for support MM(market making)
    // 询价请求
    virtual void ReqForQuote(const T_ReqForQuote *p);
    // 报价请求
    virtual void InsertQuote(const T_InsertQuote *p);
    // 撤销报价
    virtual void CancelQuote(const T_CancelQuote *p);

    virtual void ReqForQuoteInsert(const T_ReqForQuote *pReq);

    /// 交易响应数据处理函数对象的设置接口
    virtual  void SetCallbackHandler(OrderRespondWithPosHandler handler)
   {
	   order_pos_respond_handler_ = handler;
   }
    virtual  void SetCallbackHandler(CancelRespondHandler handler)
   {
	   cancel_respond_handler_ = handler;
   }
    virtual  void SetCallbackHandler(OrderReturnWithPosHandler handler)
   {
	   order_pos_return_handler_ = handler;
   }
    virtual  void SetCallbackHandler(TradeReturnWithPosHandler handler)
	{
		trade_pos_return_handler_ = handler;
	}
    virtual  void SetCallbackHandler(PositionReturnHandler handler)
   {
	   position_return_handler_ = handler;
   }
   virtual  void SetCallbackHandler(OrderDetailReturnHandler handler)
   {
	   order_detail_return_handler_ = handler;
   }
   virtual  void SetCallbackHandler(TradeDetailReturnHandler handler)
  {
	  trade_detail_return_handler_ = handler;
  }

  virtual  void SetCallbackHandler(RtnForQuoteHandler handler)//  询价通知 通道 -> 模型
  {
	 forquote_rtn_handler_ = handler;
  }
  virtual  void SetCallbackHandler(RspOfReqForQuoteHandler handler)//  询价响应 通道 -> 模型
	{
		req_forquote_rsp_handler_ = handler;
	}
	  virtual  void SetCallbackHandler(InsertQuoteRspWithPosHandler handler)//  报价应答 通道 -> 模型
	{
		insert_quote_rsp_pos_h_ = handler;
	}
	  virtual  void SetCallbackHandler(CancelQuoteRspHandler  handler)//  撤销报价应答 通道 -> 模型
	 {
		 cancel_quote_rsp_h_ = handler;
	 }
	 virtual  void SetCallbackHandler(QuoteReturnWithPosHandler handler)//  报价回报 通道 -> 模型
	 {
		 quote_rtn_pos_h_ = handler;
	 }
	 virtual  void SetCallbackHandler(QuoteTradeReturnWithPosHandler handler)//  报价成交回报  通道 -> 模型
	 {
		 quote_trade_pos_h_ = handler;
	 }

	 virtual void SetCallbackHandler(ContractInfoReturnHandler handler)
	{
		contract_info_return_handler_ = handler;
	}

    virtual std::string GetClientID();

    virtual void ReqQuoteAction(const T_CancelQuote *pReq);
    virtual void QueryContractInfo(const T_QryContractInfo *pQryParam);

    virtual void ReqQuoteInsert(const T_InsertQuote *pReq);
    // interface for get position. not implemented yet
    // PositionData GetPositionOfStrategy(int strategy_id);

    // TODO 转发请求到通道，供MYExchange内部使用，后续多通道时，需要独立出来
    void PlaceOrderToTunnel(const T_PlaceOrder *pPlaceOrder);
    void CancelOrderToTunnel(const T_CancelOrder *pCancelOrder);
    void InsertQuoteToTunnel(const T_InsertQuote *p);
    void CancelQuoteToTunnel(const T_CancelQuote *p);

    // TODO 转发响应给客户，供MYExchange内部使用
    void SendCancelRespond(const T_CancelRespond * cancel_rsp);
    void SendOrderRespond(const T_OrderRespond * order_rsp, const T_PositionData * pos);
    void SendOrderReturn(const T_OrderReturn * order_rtn, const T_PositionData *pos);
    void SendTradeReturn(const T_TradeReturn * trade_rtn, const T_PositionData *pos);

    // added for market making. on 20141216
    void SendReqForQuoteRespond(const T_RspOfReqForQuote * p);
    void SendForQuoteRtn(const T_RtnForQuote * p);
    void SendCancelQuoteRespond(const T_CancelQuoteRespond * p);
    void SendInsertQuoteRespond(const T_InsertQuoteRespond * p, const T_PositionData * pos);
    void SendQuoteReturn(const T_QuoteReturn * p, const T_PositionData *pos);
    void SendQuoteTradeReturn(const T_QuoteTrade * p, const T_PositionData *pos);

    void SendContractReturn(const T_ContractInfoReturn * rtn);

    // 增加仓位查询接口 20160701
    virtual T_PositionData GetPosition(SerialNoType sn, const std::string &contract)
    {
        T_PositionData empty;
        return empty;
    }

private:
    // forbid copy
    MYExchange(const MYExchange &);
    MYExchange & operator=(const MYExchange &);

    // inner interface
    void OrderRespond(const T_OrderRespond * order_rsp);
    void CancelRespond(const T_CancelRespond * cancel_rsp);
    void OrderReturn(const T_OrderReturn * order_rtn);
    void TradeReturn(const T_TradeReturn * trade_rtn);

    void ReqForQuoteRespond(const T_RspOfReqForQuote * p);//  询价响应 通道 -> 模型
    void ForQuoteRtn(const T_RtnForQuote * p); //  询价通知 通道 -> 模型
    void InsertQuoteRespond(const T_InsertQuoteRespond * p);//  报价应答 通道 -> 模型
    void CancelQuoteRespond(const T_CancelQuoteRespond * p);//  撤销报价应答 通道 -> 模型
    void QuoteReturn(const T_QuoteReturn * p);//  报价回报 通道 -> 模型
    void QuoteTradeReturn(const T_QuoteTrade * p);//  报价成交回报  通道 -> 模型

    void SendPositionReturn(const T_PositionReturn * rtn);
    void SendOrderDetailReturn(const T_OrderDetailReturn * rtn);
    void SendTradeDetailReturn(const T_TradeDetailReturn * rtn);

    // handlers
    CancelRespondHandler cancel_respond_handler_;
    OrderRespondWithPosHandler order_pos_respond_handler_;
    OrderReturnWithPosHandler order_pos_return_handler_;
    TradeReturnWithPosHandler trade_pos_return_handler_;

    PositionReturnHandler position_return_handler_;
    OrderDetailReturnHandler order_detail_return_handler_;
    TradeDetailReturnHandler trade_detail_return_handler_;

    // added on 20141218, for market making
    // 询价响应
    RspOfReqForQuoteHandler req_forquote_rsp_handler_;
    // 询价通知
    RtnForQuoteHandler forquote_rtn_handler_;
    //  撤销报价应答
    CancelQuoteRspHandler cancel_quote_rsp_h_;
    //  报价应答
    InsertQuoteRspWithPosHandler insert_quote_rsp_pos_h_;
    //  报价回报
    QuoteReturnWithPosHandler quote_rtn_pos_h_;
    //  报价成交回报
    QuoteTradeReturnWithPosHandler quote_trade_pos_h_;
    ContractInfoReturnHandler contract_info_return_handler_;

    // data manager
    MYOrderDataManager * p_order_manager_;
    MYPositionDataManager *p_position_manager_;
    MYOrderReq *p_order_req_;
    MYOrderRsp *p_order_rsp_;

    // tunnels
    void* m_tnl_hdl;
    MYTunnelInterface **pp_tunnel_;
    int cur_tunnel_index_;
    int max_tunnel_index_;

    // can't operate order concurrently
    std::mutex mutex_operator_;

    MYExchangeInnerImp *my_exchange_inner_imp_;

    T_ContractInfoReturn contract_rst_;
};
// the class factories
//extern "C" MYExchangeInterface* CreateExchange(/*const std::string &future_exchange_config_file*/);
//
//extern "C" void create(/*const std::string &future_exchange_config_file*/);
//
//extern "C" void DestroyExchange(MYExchange* p) ;
#endif
