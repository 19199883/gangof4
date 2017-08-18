#include "my_exchange.h"
#include "my_exchange_datatype_inner.h"
#include "my_exchange_inner_imp.h"
#include "order_data.h"
#include "order_req.h"
#include "order_rsp.h"
#include "position_data.h"
#include "my_exchange_utility.h"
#include "config_data.h"
//#include "my_tunnel_lib.h"
#include <thread>
#include <dlfcn.h>
#include "log_util_myex.h"

typedef MYTunnelInterface* (*CreateTradeTunnelFuncType)(const std::string &tunnel_config_file);


MYExchange::MYExchange(struct my_xchg_cfg& _cfg)
{
	MYExConfigData cfg(_cfg);
	void* fn;
	CreateTradeTunnelFuncType create_trd_tunnel;
	MYExchangeLogUtil::Start("my_exchange", 0);
	m_tnl_hdl = NULL;

	// 构建通道
	cur_tunnel_index_ = 0;
	max_tunnel_index_ = 0;
	pp_tunnel_ = new MYTunnelInterface*[2];
	pp_tunnel_[1] = NULL;
	while (max_tunnel_index_ < 1)
	{
		void* hdl;
		/* dlopen */
		hdl = dlopen(_cfg.tunnel_so_path, RTLD_NOW);
		if (NULL == hdl)
		{
			fprintf(stderr, "Failed to dlopen %s\n", _cfg.tunnel_so_path);
			break;
		}

		m_tnl_hdl = hdl;

		fn = dlsym(hdl, "CreateTradeTunnel");
		if (NULL == fn)
		{
			fprintf(stderr, "Failed to dlsym CreateTradeTunnel\n");
			break;
		}

		create_trd_tunnel = CreateTradeTunnelFuncType(fn);
		pp_tunnel_[max_tunnel_index_] = create_trd_tunnel(_cfg.tunnel_cfg_path);
		if (NULL == pp_tunnel_[max_tunnel_index_])
		{
			fprintf(stderr, "Failed to create_trd_tunnel by config : %s\n", _cfg.tunnel_cfg_path);
			break;
		}

		/* dlsym, get address of CreateTradeTunnel */
		//pp_tunnel_[max_tunnel_index_] = CreateTradeTunnel(cfg.tunnel_cfg_path);
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((OrderRespondHandler) std::bind(&MYExchange::OrderRespond, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((CancelRespondHandler) std::bind(&MYExchange::CancelRespond, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((OrderReturnHandler) std::bind(&MYExchange::OrderReturn, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((TradeReturnHandler) std::bind(&MYExchange::TradeReturn, this, std::placeholders::_1));

		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
			(PositionReturnHandler) std::bind(&MYExchange::SendPositionReturn, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
			(OrderDetailReturnHandler) std::bind(&MYExchange::SendOrderDetailReturn, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
			(TradeDetailReturnHandler) std::bind(&MYExchange::SendTradeDetailReturn, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
			(ContractInfoReturnHandler) std::bind(&MYExchange::SendContractReturn, this, std::placeholders::_1));

		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
			(RspOfReqForQuoteHandler) std::bind(&MYExchange::ReqForQuoteRespond, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((RtnForQuoteHandler) std::bind(&MYExchange::ForQuoteRtn, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
			(CancelQuoteRspHandler) std::bind(&MYExchange::CancelQuoteRespond, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
			(InsertQuoteRspHandler) std::bind(&MYExchange::InsertQuoteRespond, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((QuoteReturnHandler) std::bind(&MYExchange::QuoteReturn, this, std::placeholders::_1));
		pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
			(QuoteTradeReturnHandler) std::bind(&MYExchange::QuoteTradeReturn, this, std::placeholders::_1));
		++max_tunnel_index_;
	}

	// 构造数据管理对象
	p_order_manager_ = new MYOrderDataManager();
	p_position_manager_ = new MYPositionDataManager();
	p_order_req_ = new MYOrderReq(this, p_order_manager_, p_position_manager_);
	p_order_rsp_ = new MYOrderRsp(this, p_order_manager_, p_position_manager_, p_order_req_);

	// 接口无需暴露的内部实现，隐藏在实现类中，避免接口的修改
	my_exchange_inner_imp_ = new MYExchangeInnerImp(pp_tunnel_[0], cfg, max_tunnel_index_);
//	my_exchange_inner_imp_->qry_pos_flag = false;
//	my_exchange_inner_imp_->qry_order_flag = false;
//	my_exchange_inner_imp_->qry_trade_flag = false;
//	my_exchange_inner_imp_->qry_contract_flag = false;

	if (max_tunnel_index_ > 0)
	{
//		investorid = pp_tunnel_[0]->GetClientID();
	}

	// 读取历史仓位数据，init仓位数据
	if (max_tunnel_index_ > 0 && cfg.Position_policy().init_pos_at_start)
	{
		std::thread t_qry_pos(&MYExchangeInnerImp::QryPosForInit, my_exchange_inner_imp_);
		t_qry_pos.detach();
	}
}

MYExchange::~MYExchange()
{
	if (m_tnl_hdl != NULL)
	{
		dlclose(m_tnl_hdl);
	}
}

void MYExchange::PlaceOrder(const T_PlaceOrder *p)
{
    MYExchangeLogUtil::OnPlaceOrder(p,"");
    MY_LOG_DEBUG("MYExchange::PlaceOrder serial_no=%ld;code=%s;dir=%c;oc=%c;volume=%ld;price=%02f",
        p->serial_no, p->stock_code, p->direction, p->open_close, p->volume, p->limit_price);

    if (my_exchange_inner_imp_!=NULL && !my_exchange_inner_imp_->finish_init_pos_flag){
        MY_LOG_ERROR("init position not finished, can't place order now.");
        T_OrderRespond order_rsp = OrderUtil::CreateOrderRespond(
        		TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE, p->serial_no, 0,BUSSINESS_DEF::ENTRUST_STATUS_ERROR);
        T_PositionData pos = p_position_manager_->GetPositionInMarketOfCode(p->stock_code);
        SendOrderRespond(&order_rsp, &pos);

    }else{
        std::lock_guard < std::mutex > lock(mutex_operator_);
		OrderInfoInEx *new_po = new OrderInfoInEx(p);
		p_order_manager_->AddOrder(new_po);
		p_order_req_->PlaceOrderImp(new_po);
    }
}

void MYExchange::CancelOrder(const T_CancelOrder *p)
{
    MYExchangeLogUtil::OnCancelOrder(p,"");
    MY_LOG_DEBUG("MYExchange::CancelOrder serial_no=%ld;code=%s;entrust_no=%ld;org_serial_no=%ld;exchange_type=%c",
        p->serial_no, p->stock_code, p->entrust_no, p->org_serial_no,p->exchange_type);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag){
        MY_LOG_ERROR("init position not finished, can't cancel order now.");
        T_CancelRespond cancel_rsp = OrderUtil::CreateCancelRespond(TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE, p->serial_no, 0);
        SendCancelRespond(&cancel_rsp);
    }else{
        std::lock_guard < std::mutex > lock(mutex_operator_);
        p_order_req_->CancelOrderImp(p);
    }
}

void MYExchange::QueryPosition(const T_QryPosition *p)
{
    MYExchangeLogUtil::OnQryPosition(p,"");
    int idx = my_exchange_inner_imp_->GetNextTunnelIdx();
    MY_LOG_DEBUG("MYExchange::QueryPosition, tunnel_index=%d", idx);

    if (my_exchange_inner_imp_!=NULL &&
    		!my_exchange_inner_imp_->finish_init_pos_flag){
        MY_LOG_ERROR("init position not finished, can't query position now.");
        T_PositionReturn ret;
        ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        MYExchangeLogUtil::OnPositionRtn(&ret,"");
        if (position_return_handler_!=NULL) position_return_handler_(&ret);
    }else{
    	if (idx >= 0) pp_tunnel_[idx]->QueryPosition(p);
    }
}

void MYExchange::QueryOrderDetail(const T_QryOrderDetail *p)
{
}

void MYExchange::QueryTradeDetail(const T_QryTradeDetail *p)
{
    MYExchangeLogUtil::OnQryTradeDetail(p,"");
    int idx = my_exchange_inner_imp_->GetNextTunnelIdx();
    MY_LOG_DEBUG("MYExchange::QueryTradeDetail, tunnel_index=%d", idx);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag){
        MY_LOG_ERROR("init position not finished, can't query trade now.");
        T_TradeDetailReturn ret;
        ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
        MYExchangeLogUtil::OnTradeDetailRtn(&ret,"");
        if (trade_detail_return_handler_!=NULL) trade_detail_return_handler_(&ret);
    }else{
    	if (idx >= 0) pp_tunnel_[idx]->QueryTradeDetail(p);
    }
}

void MYExchange::OrderRespond(const T_OrderRespond * p)
{
    MY_LOG_DEBUG("MYExchange::OrderRespond serial_no=%ld;error_no=%d;entrust_no=%ld;entrust_status=%c",
        p->serial_no, p->error_no, p->entrust_no, p->entrust_status);

    return p_order_rsp_->OrderRespondImp(p);
}

void MYExchange::CancelRespond(const T_CancelRespond * p)
{
    MY_LOG_DEBUG("MYExchange::CancelRespond serial_no=%ld;error_no=%d;entrust_no=%ld;entrust_status=%c",
        p->serial_no, p->error_no, p->entrust_no, p->entrust_status);
     SendCancelRespond(p);
}

void MYExchange::OrderReturn(const T_OrderReturn * p)
{
    MY_LOG_DEBUG("MYExchange::OrderReturn serial_no=%ld;code=%s;entrust_no=%d;entrust_status=%c",
        p->serial_no, p->stock_code, p->entrust_no, p->entrust_status);
    return p_order_rsp_->OrderReturnImp(false, p);
}

void MYExchange::TradeReturn(const T_TradeReturn * p)
{
    MY_LOG_DEBUG("MYExchange::TradeReturn serial_no=%ld;code=%s;entrust_no=%ld;business_volume=%d;direction=%c;open_close=%c",
        p->serial_no, p->stock_code, p->entrust_no, p->business_volume, p->direction, p->open_close);
    return p_order_rsp_->TradeReturnImp(false, p);
}

void MYExchange::PlaceOrderToTunnel(const T_PlaceOrder *pPlaceOrder)
{
    int idx = my_exchange_inner_imp_->GetTunnelIdxOfContract(pPlaceOrder->stock_code);
    MY_LOG_DEBUG("MYExchange::PlaceOrderToTunnel, tunnel_index=%d", idx);
    pp_tunnel_[idx]->PlaceOrder(pPlaceOrder);
}
void MYExchange::CancelOrderToTunnel(const T_CancelOrder *pCancelOrder)
{
    int idx = my_exchange_inner_imp_->GetTunnelIdxOfContract(pCancelOrder->stock_code);
    MY_LOG_DEBUG("MYExchange::CancelOrderToTunnel, tunnel_index=%d", idx);

    pp_tunnel_[idx]->CancelOrder(pCancelOrder);
}
void MYExchange::SendOrderRespond(const T_OrderRespond * order_rsp, const T_PositionData * pos)
{
    MYExchangeLogUtil::OnOrderRespond(order_rsp,"");
    if (order_pos_respond_handler_) order_pos_respond_handler_(order_rsp, pos);

    MY_LOG_DEBUG("MYExchange::SendOrderRespond, serial_no=%ld, long=%d, short=%d, uflag=%d",
        order_rsp->serial_no, pos->long_position, pos->short_position, pos->update_flag);
}
void MYExchange::SendCancelRespond(const T_CancelRespond * cancel_rsp)
{
    MYExchangeLogUtil::OnCancelRespond(cancel_rsp,"");
    if (cancel_respond_handler_) cancel_respond_handler_(cancel_rsp);
}
void MYExchange::SendOrderReturn(const T_OrderReturn * order_rtn, const T_PositionData * pos)
{
    MYExchangeLogUtil::OnOrderReturn(order_rtn,"");
    if (order_pos_return_handler_) order_pos_return_handler_(order_rtn, pos);

    MY_LOG_DEBUG("MYExchange::SendOrderReturn, serial_no=%ld, code=%s, v=%ld, vol_remain=%ld, long=%d, short=%d, uflag=%d",
        order_rtn->serial_no, order_rtn->stock_code, order_rtn->volume, order_rtn->volume_remain,
        pos->long_position, pos->short_position, pos->update_flag);
    pos->update_flag = 0; // only the first have update flag
}
void MYExchange::SendTradeReturn(const T_TradeReturn * trade_rtn, const T_PositionData * pos)
{
    MYExchangeLogUtil::OnTradeReturn(trade_rtn,"");
    if (trade_pos_return_handler_) trade_pos_return_handler_(trade_rtn, pos);

    MY_LOG_DEBUG("MYExchange::SendTradeReturn, serial_no=%ld, code=%s, vol_matched=%d, long=%d, short=%d, uflag=%d",
        trade_rtn->serial_no, trade_rtn->stock_code, trade_rtn->business_volume,
        pos->long_position, pos->short_position, pos->update_flag);
}

void MYExchange::SendPositionReturn(const T_PositionReturn * rtn)
{
    if (my_exchange_inner_imp_!=NULL && !my_exchange_inner_imp_->finish_init_pos_flag){
        if (rtn->error_no == 0){
            my_exchange_inner_imp_->have_send_qry_pos_flag = true;
            for (const PositionDetail &pos : rtn->datas){
                p_position_manager_->InitPosition(
                		pos.stock_code, pos.direction, pos.position,pos.today_buy_volume,
                		pos.today_sell_volume,pos.yestoday_position,pos.exchange_type);
            }
            my_exchange_inner_imp_->finish_init_pos_flag = true;
        }
    }else{
    	MYExchangeLogUtil::OnPositionRtn(rtn,"");
    	if (position_return_handler_!=NULL) position_return_handler_(rtn);
    }
}

void MYExchange::SendOrderDetailReturn(const T_OrderDetailReturn * rtn)
{
    MYExchangeLogUtil::OnOrderDetailRtn(rtn,"");
    if (order_detail_return_handler_) order_detail_return_handler_(rtn);
}

void MYExchange::SendTradeDetailReturn(const T_TradeDetailReturn * rtn)
{
    MYExchangeLogUtil::OnTradeDetailRtn(rtn,"");
    if (trade_detail_return_handler_) trade_detail_return_handler_(rtn);
}

void MYExchange::ReqForQuoteRespond(const T_RspOfReqForQuote * p)
{
    // EX_LOG_DEBUG("MYExchange::ReqForQuoteRespond serial_no=%ld;error_no=%d", p->serial_no, p->error_no);

    //SendReqForQuoteRespond(p);
}

void MYExchange::ForQuoteRtn(const T_RtnForQuote * p)
{
//    EX_LOG_DEBUG("MYExchange::ForQuoteRtn stock_code=%s, for_quote_id=%s, for_quote_time=%s",
//        p->stock_code, p->for_quote_id, p->for_quote_time);

  //  SendForQuoteRtn(p);
}

void MYExchange::CancelQuoteRespond(const T_CancelQuoteRespond * p)
{
//    EX_LOG_DEBUG("MYExchange::CancelQuoteRespond serial_no=%ld;error_no=%d;entrust_no=%d;entrust_status=%c",
//        p->serial_no, p->error_no, p->entrust_no, p->entrust_status);

    //SendCancelQuoteRespond(p);
}

void MYExchange::InsertQuoteRespond(const T_InsertQuoteRespond * p)
{
//    EX_LOG_DEBUG("MYExchange::InsertQuoteRespond serial_no=%ld;error_no=%d;entrust_no=%d;entrust_status=%c",
//        p->serial_no, p->error_no, p->entrust_no, p->entrust_status);

    //return p_order_rsp_->QuoteRespondImp(p);
}

void MYExchange::QuoteReturn(const T_QuoteReturn * p)
{
//    EX_LOG_DEBUG("MYExchange::QuoteReturn serial_no=%ld;code=%s;entrust_no=%d;entrust_status=%c",
//        p->serial_no, p->stock_code, p->entrust_no, p->entrust_status);

    //return p_order_rsp_->QuoteReturnImp(p);
}

void MYExchange::QuoteTradeReturn(const T_QuoteTrade * p)
{
//    EX_LOG_DEBUG("MYExchange::QuoteTradeReturn serial_no=%ld;code=%s;entrust_no=%d;business_volume=%d;direction=%c;open_close=%c",
//        p->serial_no, p->stock_code, p->entrust_no, p->business_volume, p->direction, p->open_close);

    //return p_order_rsp_->QuoteTradeImp(p);
}

void MYExchange::ReqForQuote(const T_ReqForQuote *p)
{

}

void MYExchange::InsertQuote(const T_InsertQuote *p)
{

}

void MYExchange::CancelQuote(const T_CancelQuote *p)
{

}


std::string MYExchange::GetClientID()
{
	return "";
}

void MYExchange::ReqQuoteAction(const T_CancelQuote *pReq)
{

}

void MYExchange::ReqForQuoteInsert(const T_ReqForQuote *pReq)
{

}

void MYExchange::QueryContractInfo(const T_QryContractInfo *p)
{
	// TODO: test
	MYExchangeLogUtil::OnQryContractInfo(p, "");
	int idx = my_exchange_inner_imp_->GetNextTunnelIdx();
	EX_LOG_DEBUG("MYExchange::QueryContractInfo, tunnel_index=%d", idx);

	if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
	{
		EX_LOG_ERROR("init position not finished, can't query contract now.");

		T_ContractInfoReturn ret;
		ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;

		MYExchangeLogUtil::OnContractInfoRtn(&ret, "");
		if (contract_info_return_handler_) contract_info_return_handler_(&ret);

		return;
	}

//	if (my_exchange_inner_imp_->qry_contract_flag)
//	{
		//MYExchangeLogUtil::OnContractInfoRtn(&my_exchange_inner_imp_->qry_contract_result, 0);
		contract_rst_.error_no = 0;
		contract_rst_.datas.clear();
		if (contract_info_return_handler_) contract_info_return_handler_(&contract_rst_);
		return;
//	}

	if (idx >= 0) pp_tunnel_[idx]->QueryContractInfo(p);
}

void MYExchange::SendContractReturn(const T_ContractInfoReturn * rtn)
{
//    MYExchangeLogUtil::OnContractInfoRtn(rtn, investorid);
//    if (contract_info_return_handler_) contract_info_return_handler_(rtn);
//
//    if (!my_exchange_inner_imp_->qry_contract_flag && rtn->error_no == 0)
//    {
//        my_exchange_inner_imp_->qry_contract_result = *rtn;
//        my_exchange_inner_imp_->qry_contract_flag = true;
//    }
}


void MYExchange::ReqQuoteInsert(const T_InsertQuote *pReq)
{

}

extern "C" MYExchangeInterface *CreateExchange(struct my_xchg_cfg& cfg/*const std::string &future_exchange_config_file*/)
{
    return new MYExchange(cfg/*future_exchange_config_file*/);
}


 void DestroyExchange(MYExchangeInterface* p) {
   delete p;
 }
