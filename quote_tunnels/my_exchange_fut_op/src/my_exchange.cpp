#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "my_exchange.h"
#include "my_exchange_datatype_inner.h"
#include "my_exchange_inner_imp.h"
#include "order_data.h"
#include "order_req.h"
#include "order_rsp.h"
#include "position_data.h"
#include "my_exchange_utility.h"
#include "config_data.h"
#include "my_trade_tunnel_api.h"
#include "my_cmn_util_funcs.h"

#include <atomic>

#include "log_util_myex.h"

// MYExchange::MYExchange(const std::string &config_file)
// {
//     MYExConfigData cfg(config_file);
//     MYExchangeLogUtil::Start("my_exchange", 0);
// 
//     // 构建通道
//     cur_tunnel_index_ = 0;
//     max_tunnel_index_ = 0;
//     pp_tunnel_ = new MYTunnelInterface*[cfg.Tunnel_cfg_files().size() + 1];
//     pp_tunnel_[cfg.Tunnel_cfg_files().size()] = NULL;
//     for (const std::string &tunnel_file : cfg.Tunnel_cfg_files())
//     {
//         pp_tunnel_[max_tunnel_index_] = CreateTradeTunnel(tunnel_file);
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((OrderRespondHandler) std::bind(&MYExchange::OrderRespond, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((CancelRespondHandler) std::bind(&MYExchange::CancelRespond, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((OrderReturnHandler) std::bind(&MYExchange::OrderReturn, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((TradeReturnHandler) std::bind(&MYExchange::TradeReturn, this, std::placeholders::_1));
// 
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
//             (PositionReturnHandler) std::bind(&MYExchange::SendPositionReturn, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
//             (OrderDetailReturnHandler) std::bind(&MYExchange::SendOrderDetailReturn, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
//             (TradeDetailReturnHandler) std::bind(&MYExchange::SendTradeDetailReturn, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
//             (ContractInfoReturnHandler) std::bind(&MYExchange::SendContractReturn, this, std::placeholders::_1));
// 
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
//             (RspOfReqForQuoteHandler) std::bind(&MYExchange::ReqForQuoteRespond, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((RtnForQuoteHandler) std::bind(&MYExchange::ForQuoteRtn, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
//             (CancelQuoteRspHandler) std::bind(&MYExchange::CancelQuoteRespond, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
//             (InsertQuoteRspHandler) std::bind(&MYExchange::InsertQuoteRespond, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler((QuoteReturnHandler) std::bind(&MYExchange::QuoteReturn, this, std::placeholders::_1));
//         pp_tunnel_[max_tunnel_index_]->SetCallbackHandler(
//             (QuoteTradeReturnHandler) std::bind(&MYExchange::QuoteTradeReturn, this, std::placeholders::_1));
//         ++max_tunnel_index_;
//     }
// 
//     // 构造数据管理对象
//     p_order_manager_ = new MYOrderDataManager();
//     p_position_manager_ = new MYPositionDataManager();
//     p_order_req_ = new MYOrderReq(this, p_order_manager_, p_position_manager_, cfg);
//     p_order_rsp_ = new MYOrderRsp(this, p_order_manager_, p_position_manager_, p_order_req_);
// 
//     // 接口无需暴露的内部实现，隐藏在实现类中，避免接口的修改
//     my_exchange_inner_imp_ = new MYExchangeInnerImp(pp_tunnel_[0], cfg, max_tunnel_index_);
//     my_exchange_inner_imp_->qry_pos_flag = false;
//     my_exchange_inner_imp_->qry_order_flag = false;
//     my_exchange_inner_imp_->qry_trade_flag = false;
//     my_exchange_inner_imp_->qry_contract_flag = false;
// 
//     if (max_tunnel_index_ > 0)
//     {
//         investorid = pp_tunnel_[0]->GetClientID();
//     }
// 
//     // 读取历史仓位数据，init仓位数据
//     if (max_tunnel_index_ > 0 && cfg.Position_policy().init_pos_at_start)
//     {
//         std::thread t_qry_pos(&MYExchangeInnerImp::QryPosForInit, my_exchange_inner_imp_);
//         t_qry_pos.detach();
//     }
// }

typedef MYTunnelInterface* (*CreateTradeTunnelFuncType)(const std::string &tunnel_config_file);

MYExchange::MYExchange(struct my_xchg_cfg& _cfg)
{
    MYExConfigData cfg(_cfg);
    void* fn;
    CreateTradeTunnelFuncType create_trd_tunnel;
    MYExchangeLogUtil::Start("my_exchange_fut_op", 0);
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
            fprintf(stderr, "Failed to dlopen %s, %s \n", _cfg.tunnel_so_path, dlerror());
            break;
        }

        m_tnl_hdl = hdl;

        fn = dlsym(hdl, "CreateTradeTunnel");
        if (NULL == fn)
        {
            fprintf(stderr, "Failed to dlsym CreateTradeTunnel: %s \n", dlerror());
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
    p_order_req_ = new MYOrderReq(this, p_order_manager_, p_position_manager_, cfg);
    p_order_rsp_ = new MYOrderRsp(this, p_order_manager_, p_position_manager_, p_order_req_);

    // 接口无需暴露的内部实现，隐藏在实现类中，避免接口的修改
    my_exchange_inner_imp_ = new MYExchangeInnerImp(pp_tunnel_[0], cfg, max_tunnel_index_);
    my_exchange_inner_imp_->qry_pos_flag = false;
    my_exchange_inner_imp_->qry_order_flag = false;
    my_exchange_inner_imp_->qry_trade_flag = false;
    my_exchange_inner_imp_->qry_contract_flag = false;

    if (max_tunnel_index_ > 0)
    {
        investorid = pp_tunnel_[0]->GetClientID();
    }

	// TODO: wangying modify here to support saving each strategy's position respectively
    // init model position by ev file of model
    if (cfg.Position_policy().InitPosByEvfile())
    {
        my_exchange_inner_imp_->InitModelPosByEvFile();
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
    MYExchangeLogUtil::OnPlaceOrder(p, investorid);
    EX_LOG_INFO("MYExchange::PlaceOrder serial_no=%ld;code=%s;dir=%c;oc=%c;volume=%ld;price=%02f",
        p->serial_no, p->stock_code, p->direction, p->open_close, p->volume, p->limit_price);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
    {
        EX_LOG_ERROR("init position not finished, can't place order now.");

        T_OrderRespond order_rsp;
        OrderUtil::CreateOrderRespond(order_rsp, TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE, p->serial_no, 0,
        MY_TNL_OS_ERROR);

        SendOrderRespond(&order_rsp, p->stock_code, p->direction, p->volume);

        return;
    }

    int left_volume = my_exchange_inner_imp_->CheckAvailablePosition(p->serial_no, p->stock_code, p->direction, p->open_close, p->volume);

    {
        //_TTSpinLockGuard lock(my_exchange_inner_imp_->op_spin_lock);
        std::lock_guard<std::mutex> lock(my_exchange_inner_imp_->op_mutex_lock);

        if (p->volume <= 0)
        {
            T_OrderRespond order_rsp;
            OrderUtil::CreateOrderRespond(order_rsp, TUNNEL_ERR_CODE::BAD_FIELD,
                p->serial_no, 0, MY_TNL_OS_ERROR);

            SendOrderRespond(&order_rsp, p->stock_code, p->direction, p->volume);
            return;
        }

        // 转发报单实现
        if (left_volume < p->volume)
        {
            T_OrderRespond order_rsp;
            OrderUtil::CreateOrderRespond(order_rsp, TUNNEL_ERR_CODE::REACH_UPPER_LIMIT_POSITION,
                p->serial_no, 0, MY_TNL_OS_ERROR);

            SendOrderRespond(&order_rsp, p->stock_code, p->direction, p->volume);

            // don't place partial volume order. modified on 20151026 by cys
//            if (left_volume <= 0 || p->order_type == MY_TNL_HF_FOK)
//            {
//                T_OrderRespond order_rsp = OrderUtil::CreateOrderRespond(TUNNEL_ERR_CODE::REACH_UPPER_LIMIT_POSITION,
//                    p->serial_no, 0, MY_TNL_OS_ERROR);
//
//                SendOrderRespond(&order_rsp, p->stock_code, p->direction, p->volume);
//            }
//            else
//            {
//                OrderInfoInEx *new_po = new OrderInfoInEx(p);
//                p_order_manager_->AddOrder(new_po);
//                OrderInfoInEx *left_order = new OrderInfoInEx(new_po, p_order_manager_->GetNewSerialNo(new_po->serial_no),
//                    left_volume, false);
//                p_order_manager_->AddOrder(left_order);
//                p_order_req_->PlaceOrderImp(left_order);
//            }
        }
        else
        {
            OrderInfoInEx *new_po = new OrderInfoInEx(p);
            p_order_manager_->AddOrder(new_po);
            p_order_req_->PlaceOrderImp(new_po);
        }
    }
}

void MYExchange::CancelOrder(const T_CancelOrder *p)
{
    MYExchangeLogUtil::OnCancelOrder(p, investorid);
    EX_LOG_INFO("MYExchange::CancelOrder serial_no=%ld;code=%s;entrust_no=%ld;org_serial_no=%ld",
        p->serial_no, p->stock_code, p->entrust_no, p->org_serial_no);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
    {
        EX_LOG_ERROR("init position not finished, can't cancel order now.");

        T_CancelRespond cancel_rsp;
        OrderUtil::CreateCancelRespond(cancel_rsp, TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE, p->serial_no, 0);
        SendCancelRespond(&cancel_rsp);

        return;
    }

    {
        //_TTSpinLockGuard lock(my_exchange_inner_imp_->op_spin_lock);
        std::lock_guard<std::mutex> lock(my_exchange_inner_imp_->op_mutex_lock);

        // no entrust no, refuse for simplify handle
        if (p->entrust_no == 0)
        {
            T_CancelRespond cancel_rsp;
            OrderUtil::CreateCancelRespond(cancel_rsp, TUNNEL_ERR_CODE::UNSUITABLE_ORDER_STATUS, p->serial_no, 0);
            SendCancelRespond(&cancel_rsp);
            return;
        }

        // 转发撤单实现
        p_order_req_->CancelOrderImp(p);
    }
}
void MYExchange::ReqForQuoteInsert(const T_ReqForQuote *p)
{
    MYExchangeLogUtil::OnReqForQuote(p, investorid);
    int idx = my_exchange_inner_imp_->GetNextTunnelIdx();
    EX_LOG_INFO("MYExchange::ReqForQuote, tunnel_index=%d", idx);

    pp_tunnel_[idx]->ReqForQuoteInsert(p);
}

void MYExchange::ReqQuoteInsert(const T_InsertQuote *p)
{
    MYExchangeLogUtil::OnInsertQuote(p, investorid);

    bool have_available_pos = my_exchange_inner_imp_->CheckAvailablePosition(p->serial_no, p->stock_code, p->buy_volume,
        p->sell_volume);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
    {
        EX_LOG_ERROR("init position not finished, can't insert quote now.");

        T_InsertQuoteRespond rsp;
        OrderUtil::CreateInsertQuoteRsp(rsp, TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE, p->serial_no, 0,
        MY_TNL_OS_ERROR);

        SendInsertQuoteRespond(&rsp, p->stock_code, p->buy_volume, p->sell_volume);

        return;
    }

    {
        //_TTSpinLockGuard lock(my_exchange_inner_imp_->op_spin_lock);
        std::lock_guard<std::mutex> lock(my_exchange_inner_imp_->op_mutex_lock);

        // 转发insert quote实现
        if (!have_available_pos)
        {
            T_InsertQuoteRespond rsp;
            OrderUtil::CreateInsertQuoteRsp(rsp, TUNNEL_ERR_CODE::REACH_UPPER_LIMIT_POSITION,
                p->serial_no, 0, MY_TNL_OS_ERROR);

            SendInsertQuoteRespond(&rsp, p->stock_code, p->buy_volume, p->sell_volume);
        }
        else
        {
            QuoteInfoInEx *p_quote_inex = new QuoteInfoInEx(p);
            p_order_manager_->AddQuote(p_quote_inex);
            p_order_req_->InsertQuoteImp(p_quote_inex);
        }
    }
}

void MYExchange::ReqQuoteAction(const T_CancelQuote *p)
{
    MYExchangeLogUtil::OnCancelQuote(p, investorid);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
    {
        EX_LOG_ERROR("init position not finished, can't cancel quote now.");

        T_CancelQuoteRespond rsp;
        OrderUtil::CreateCancelQuoteRespond(rsp, TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE, p->serial_no,
            p->entrust_no, MY_TNL_OS_ERROR);

        SendCancelQuoteRespond(&rsp);

        return;
    }
    QuoteInfoInEx *dest_order = p_order_manager_->GetQuote(p->org_serial_no);
    if (!dest_order)
    {
        T_CancelQuoteRespond rsp;
        OrderUtil::CreateCancelQuoteRespond(rsp, TUNNEL_ERR_CODE::ORDER_NOT_FOUND, p->serial_no,
            p->entrust_no, MY_TNL_OS_ERROR);
        SendCancelQuoteRespond(&rsp);

        return;
    }
    if (OrderUtil::IsTerminated(dest_order->quote_status))
    {
        T_CancelQuoteRespond rsp;
        OrderUtil::CreateCancelQuoteRespond(rsp, TUNNEL_ERR_CODE::DUPLICATE_ORDER_ACTION_REF, p->serial_no,
            p->entrust_no, MY_TNL_OS_ERROR);
        SendCancelQuoteRespond(&rsp);

        return;
    }

    //_TTSpinLockGuard lock(my_exchange_inner_imp_->op_spin_lock);
    std::lock_guard<std::mutex> lock(my_exchange_inner_imp_->op_mutex_lock);
    p_order_req_->CancelQuoteImp(p);
}

void MYExchange::QueryPosition(const T_QryPosition *p)
{
    MYExchangeLogUtil::OnQryPosition(p, investorid);
    int idx = my_exchange_inner_imp_->GetNextTunnelIdx();
    EX_LOG_INFO("MYExchange::QueryPosition, tunnel_index=%d", idx);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
    {
        EX_LOG_ERROR("init position not finished, can't query position now.");

        T_PositionReturn ret;
        ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;

        MYExchangeLogUtil::OnPositionRtn(&ret, investorid);
        if (position_return_handler_) position_return_handler_(&ret);

        return;
    }

    if (my_exchange_inner_imp_->qry_pos_flag)
    {
        MYExchangeLogUtil::OnPositionRtn(&my_exchange_inner_imp_->qry_pos_result, investorid);
        if (position_return_handler_) position_return_handler_(&my_exchange_inner_imp_->qry_pos_result);
        return;
    }

    if (idx >= 0) pp_tunnel_[idx]->QueryPosition(p);
}

void MYExchange::QueryOrderDetail(const T_QryOrderDetail *p)
{
    MYExchangeLogUtil::OnQryOrderDetail(p, investorid);
    int idx = my_exchange_inner_imp_->GetNextTunnelIdx();
    EX_LOG_INFO("MYExchange::QueryOrderDetail, tunnel_index=%d", idx);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
    {
        EX_LOG_ERROR("init position not finished, can't query order now.");

        T_OrderDetailReturn ret;
        ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;

        MYExchangeLogUtil::OnOrderDetailRtn(&ret, investorid);
        if (order_detail_return_handler_) order_detail_return_handler_(&ret);

        return;
    }

    if (my_exchange_inner_imp_->qry_order_flag)
    {
        MYExchangeLogUtil::OnOrderDetailRtn(&my_exchange_inner_imp_->qry_order_result, investorid);
        if (order_detail_return_handler_) order_detail_return_handler_(&my_exchange_inner_imp_->qry_order_result);
        return;
    }

    if (idx >= 0) pp_tunnel_[idx]->QueryOrderDetail(p);

}
void MYExchange::QueryTradeDetail(const T_QryTradeDetail *p)
{
    MYExchangeLogUtil::OnQryTradeDetail(p, investorid);
    int idx = my_exchange_inner_imp_->GetNextTunnelIdx();
    EX_LOG_INFO("MYExchange::QueryTradeDetail, tunnel_index=%d", idx);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
    {
        EX_LOG_ERROR("init position not finished, can't query trade now.");

        T_TradeDetailReturn ret;
        ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;

        MYExchangeLogUtil::OnTradeDetailRtn(&ret, investorid);
        if (trade_detail_return_handler_) trade_detail_return_handler_(&ret);

        return;
    }

    if (my_exchange_inner_imp_->qry_trade_flag)
    {
        MYExchangeLogUtil::OnTradeDetailRtn(&my_exchange_inner_imp_->qry_trade_result, investorid);
        if (trade_detail_return_handler_) trade_detail_return_handler_(&my_exchange_inner_imp_->qry_trade_result);
        return;
    }

    if (idx >= 0) pp_tunnel_[idx]->QueryTradeDetail(p);
}
void MYExchange::QueryContractInfo(const T_QryContractInfo *p)
{
    MYExchangeLogUtil::OnQryContractInfo(p, investorid);
    int idx = my_exchange_inner_imp_->GetNextTunnelIdx();
    EX_LOG_DEBUG("MYExchange::QueryContractInfo, tunnel_index=%d", idx);

    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
    {
        EX_LOG_ERROR("init position not finished, can't query contract now.");

        T_ContractInfoReturn ret;
        ret.error_no = TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;

        MYExchangeLogUtil::OnContractInfoRtn(&ret, investorid);
        if (contract_info_return_handler_) contract_info_return_handler_(&ret);

        return;
    }

    if (my_exchange_inner_imp_->qry_contract_flag)
    {
        MYExchangeLogUtil::OnContractInfoRtn(&my_exchange_inner_imp_->qry_contract_result, investorid);
        if (contract_info_return_handler_) contract_info_return_handler_(&my_exchange_inner_imp_->qry_contract_result);
        return;
    }

    if (idx >= 0) pp_tunnel_[idx]->QueryContractInfo(p);
}

void MYExchange::OrderRespond(const T_OrderRespond * p)
{
    EX_LOG_DEBUG("MYExchange::OrderRespond serial_no=%ld;error_no=%d;entrust_no=%ld;entrust_status=%c",
        p->serial_no, p->error_no, p->entrust_no, p->entrust_status);

    return p_order_rsp_->OrderRespondImp(false, p);
}

void MYExchange::CancelRespond(const T_CancelRespond * p)
{
    EX_LOG_DEBUG("MYExchange::CancelRespond serial_no=%ld;error_no=%d;entrust_no=%ld;entrust_status=%c",
        p->serial_no, p->error_no, p->entrust_no, p->entrust_status);
    if (p_order_manager_->IsOriginalSerialNo(p->serial_no))
    {
        // 交易程序发出的原始撤单，直接回报
        SendCancelRespond(p);
    }
}

void MYExchange::OrderReturn(const T_OrderReturn * p)
{
    EX_LOG_DEBUG("MYExchange::OrderReturn serial_no=%ld;code=%s;entrust_no=%d;entrust_status=%c",
        p->serial_no, p->stock_code, p->entrust_no, p->entrust_status);

    return p_order_rsp_->OrderReturnImp(false, p);
}

void MYExchange::TradeReturn(const T_TradeReturn * p)
{
    EX_LOG_DEBUG("MYExchange::TradeReturn serial_no=%ld;code=%s;entrust_no=%ld;business_volume=%d;direction=%c;open_close=%c",
        p->serial_no, p->stock_code, p->entrust_no, p->business_volume, p->direction, p->open_close);

    return p_order_rsp_->TradeReturnImp(false, p);
}

void MYExchange::ReqForQuoteRespond(const T_RspOfReqForQuote * p)
{
    EX_LOG_DEBUG("MYExchange::ReqForQuoteRespond serial_no=%ld;error_no=%d", p->serial_no, p->error_no);

    SendReqForQuoteRespond(p);
}

void MYExchange::ForQuoteRtn(const T_RtnForQuote * p)
{
    EX_LOG_DEBUG("MYExchange::ForQuoteRtn stock_code=%s, for_quote_id=%s, for_quote_time=%s",
        p->stock_code, p->for_quote_id, p->for_quote_time);

    SendForQuoteRtn(p);
}

void MYExchange::CancelQuoteRespond(const T_CancelQuoteRespond * p)
{
    EX_LOG_DEBUG("MYExchange::CancelQuoteRespond serial_no=%ld;error_no=%d;entrust_no=%d;entrust_status=%c",
        p->serial_no, p->error_no, p->entrust_no, p->entrust_status);

    SendCancelQuoteRespond(p);
}

void MYExchange::InsertQuoteRespond(const T_InsertQuoteRespond * p)
{
    EX_LOG_DEBUG("MYExchange::InsertQuoteRespond serial_no=%ld;error_no=%d;entrust_no=%d;entrust_status=%c",
        p->serial_no, p->error_no, p->entrust_no, p->entrust_status);

    return p_order_rsp_->QuoteRespondImp(p);
}

void MYExchange::QuoteReturn(const T_QuoteReturn * p)
{
    EX_LOG_DEBUG("MYExchange::QuoteReturn serial_no=%ld;code=%s;entrust_no=%d;entrust_status=%c",
        p->serial_no, p->stock_code, p->entrust_no, p->entrust_status);

    return p_order_rsp_->QuoteReturnImp(p);
}

void MYExchange::QuoteTradeReturn(const T_QuoteTrade * p)
{
    EX_LOG_DEBUG("MYExchange::QuoteTradeReturn serial_no=%ld;code=%s;entrust_no=%d;business_volume=%d;direction=%c;open_close=%c",
        p->serial_no, p->stock_code, p->entrust_no, p->business_volume, p->direction, p->open_close);

    return p_order_rsp_->QuoteTradeImp(p);
}

void MYExchange::PlaceOrderToTunnel(const T_PlaceOrder *pPlaceOrder)
{
    int idx = my_exchange_inner_imp_->GetTunnelIdxOfContract(pPlaceOrder->stock_code);
    EX_LOG_DEBUG("MYExchange::PlaceOrderToTunnel, tunnel_index=%d", idx);

    // TODO should close from account which hold the position
    pp_tunnel_[idx]->PlaceOrder(pPlaceOrder);
}
void MYExchange::CancelOrderToTunnel(const T_CancelOrder *pCancelOrder)
{
    int idx = my_exchange_inner_imp_->GetTunnelIdxOfContract(pCancelOrder->stock_code);
    EX_LOG_DEBUG("MYExchange::CancelOrderToTunnel, tunnel_index=%d", idx);

    pp_tunnel_[idx]->CancelOrder(pCancelOrder);
}
void MYExchange::CancelQuoteToTunnel(const T_CancelQuote *p)
{
    int idx = my_exchange_inner_imp_->GetTunnelIdxOfContract(p->stock_code);
    EX_LOG_DEBUG("MYExchange::CancelQuoteToTunnel, tunnel_index=%d", idx);

    pp_tunnel_[idx]->ReqQuoteAction(p);
}
void MYExchange::InsertQuoteToTunnel(const T_InsertQuote *p)
{
    int idx = my_exchange_inner_imp_->GetTunnelIdxOfContract(p->stock_code);
    EX_LOG_DEBUG("MYExchange::InsertQuoteToTunnel, tunnel_index=%d", idx);

    pp_tunnel_[idx]->ReqQuoteInsert(p);
}
void MYExchange::SendOrderRespond(const T_OrderRespond * order_rsp, const char *contract, char dir, VolumeType volume_remain)
{
    MYExchangeLogUtil::OnOrderRespond(order_rsp, investorid);

    if (OrderUtil::IsTerminated(order_rsp->entrust_status))
    {
        my_exchange_inner_imp_->RollBackPendingVolumeForOrder(order_rsp->serial_no, contract, dir, volume_remain);
    }

    T_PositionData pos = my_exchange_inner_imp_->GetPosition(order_rsp->serial_no, contract);

    if (order_pos_respond_handler_)
    {
        order_pos_respond_handler_(order_rsp, &pos);
    }
    MY_LOG_DEBUG("MYExchange::SendOrderRespond, serial_no=%ld, long=%d, short=%d, uflag=%d",
        order_rsp->serial_no, pos.long_position, pos.short_position, pos.update_flag);
}
void MYExchange::SendCancelRespond(const T_CancelRespond * cancel_rsp)
{
    MYExchangeLogUtil::OnCancelRespond(cancel_rsp, investorid);
    if (cancel_respond_handler_) cancel_respond_handler_(cancel_rsp);
}

void MYExchange::SendOrderReturn(const T_OrderReturn * order_rtn, VolumeType matched_volume)
{
    MYExchangeLogUtil::OnOrderReturn(order_rtn, investorid);

    // update position of model
    if (matched_volume > 0)
    {
        my_exchange_inner_imp_->UpdatePositionForOrder(order_rtn->serial_no, order_rtn->stock_code,
            order_rtn->direction, matched_volume);
    }

    // rollback remain volume
    if (OrderUtil::IsTerminated(order_rtn->entrust_status))
    {
        my_exchange_inner_imp_->RollBackPendingVolumeForOrder(order_rtn->serial_no,
            order_rtn->stock_code, order_rtn->direction, order_rtn->volume_remain);
    }

    // get position of model
    T_PositionData pos = my_exchange_inner_imp_->GetPosition(order_rtn->serial_no, order_rtn->stock_code);
    pos.update_flag = (matched_volume > 0);

    // send to client
    if (order_pos_return_handler_)
    {
        order_pos_return_handler_(order_rtn, &pos);
    }

    MY_LOG_DEBUG("MYExchange::SendOrderReturn, serial_no=%ld, long=%d, short=%d, uflag=%d",
        order_rtn->serial_no, pos.long_position, pos.short_position, pos.update_flag);
}
void MYExchange::SendTradeReturn(const T_TradeReturn * trade_rtn)
{
    MYExchangeLogUtil::OnTradeReturn(trade_rtn, investorid);

    // get position of model
    T_PositionData pos = my_exchange_inner_imp_->GetPosition(trade_rtn->serial_no, trade_rtn->stock_code);

    if (trade_pos_return_handler_) trade_pos_return_handler_(trade_rtn, &pos);

    MY_LOG_DEBUG("MYExchange::SendTradeReturn, serial_no=%ld, long=%d, short=%d, uflag=%d",
        trade_rtn->serial_no, pos.long_position, pos.short_position, pos.update_flag);
}
void MYExchange::SendReqForQuoteRespond(const T_RspOfReqForQuote * p)
{
    MYExchangeLogUtil::OnRspOfReqForQuote(p, investorid);
    if (req_forquote_rsp_handler_) req_forquote_rsp_handler_(p);
}
void MYExchange::SendForQuoteRtn(const T_RtnForQuote * p)
{
    MYExchangeLogUtil::OnRtnForQuote(p, investorid);
    if (forquote_rtn_handler_) forquote_rtn_handler_(p);
}
void MYExchange::SendCancelQuoteRespond(const T_CancelQuoteRespond * p)
{
    MYExchangeLogUtil::OnCancelQuoteRespond(p, investorid);
    if (cancel_quote_rsp_h_) cancel_quote_rsp_h_(p);
}
void MYExchange::SendInsertQuoteRespond(const T_InsertQuoteRespond * p, const char *contract, VolumeType buy_remain,
    VolumeType sell_remain)
{
    MYExchangeLogUtil::OnInsertQuoteRespond(p, investorid);

    if (OrderUtil::IsTerminated(p->entrust_status))
    {
        my_exchange_inner_imp_->RollBackPendingVolumeForQuote(p->serial_no, contract, MY_TNL_D_BUY, buy_remain);
        my_exchange_inner_imp_->RollBackPendingVolumeForQuote(p->serial_no, contract, MY_TNL_D_SELL, sell_remain);
    }

    T_PositionData pos = my_exchange_inner_imp_->GetPosition(p->serial_no, contract);

    if (insert_quote_rsp_pos_h_)
    {
        insert_quote_rsp_pos_h_(p, &pos);
    }
}
void MYExchange::SendQuoteReturn(const T_QuoteReturn * p, VolumeType matched_volume)
{
    MYExchangeLogUtil::OnQuoteReturn(p, investorid);

    // update position of model
    if (matched_volume > 0)
    {
        my_exchange_inner_imp_->UpdatePositionForOrder(p->serial_no, p->stock_code,
            p->direction, matched_volume);
    }

    // rollback remain volume
    if (OrderUtil::IsTerminated(p->entrust_status))
    {
        my_exchange_inner_imp_->RollBackPendingVolumeForOrder(p->serial_no,
            p->stock_code, p->direction, p->volume_remain);
    }

    // get position of model
    T_PositionData pos = my_exchange_inner_imp_->GetPosition(p->serial_no, p->stock_code);
    pos.update_flag = (matched_volume > 0);

    if (quote_rtn_pos_h_)
    {
        quote_rtn_pos_h_(p, &pos);
    }
}
void MYExchange::SendQuoteTradeReturn(const T_QuoteTrade * p)
{
    MYExchangeLogUtil::OnQuoteTrade(p, investorid);

    // get position of model
    T_PositionData pos = my_exchange_inner_imp_->GetPosition(p->serial_no, p->stock_code);

    if (quote_trade_pos_h_) quote_trade_pos_h_(p, &pos);
}

void MYExchange::SendPositionReturn(const T_PositionReturn * rtn)
{
    if (my_exchange_inner_imp_ && !my_exchange_inner_imp_->finish_init_pos_flag)
    {
        if (rtn->error_no == 0)
        {
            my_exchange_inner_imp_->have_send_qry_pos_flag = true;
            for (const PositionDetail &pos : rtn->datas)
            {
                p_position_manager_->InitPosition(pos.stock_code, pos.direction, pos.position);

                // if init model position from ev file, don't init when query from market
				// TODO: pos_calc,wangying, modify this function to support for saving each strategy's position respectively
                //if (!my_exchange_inner_imp_->init_pos_from_ev_flag)
                //{
                //    my_exchange_inner_imp_->InitPosition(pos.stock_code, pos.direction, pos.position);
                //}
            }
            my_exchange_inner_imp_->finish_init_pos_flag = true;
        }

        if (!my_exchange_inner_imp_->qry_pos_flag && rtn->error_no == 0)
        {
            my_exchange_inner_imp_->qry_pos_result = *rtn;
            my_exchange_inner_imp_->qry_pos_flag = true;
        }
        return;
    }

    MYExchangeLogUtil::OnPositionRtn(rtn, investorid);
    if (position_return_handler_) position_return_handler_(rtn);
}

void MYExchange::SendOrderDetailReturn(const T_OrderDetailReturn * rtn)
{
    MYExchangeLogUtil::OnOrderDetailRtn(rtn, investorid);
    if (order_detail_return_handler_) order_detail_return_handler_(rtn);
    if (!my_exchange_inner_imp_->qry_order_flag && rtn->error_no == 0)
    {
        my_exchange_inner_imp_->qry_order_result = *rtn;
        my_exchange_inner_imp_->qry_order_flag = true;
    }
}

void MYExchange::SendTradeDetailReturn(const T_TradeDetailReturn * rtn)
{
    MYExchangeLogUtil::OnTradeDetailRtn(rtn, investorid);
    if (trade_detail_return_handler_) trade_detail_return_handler_(rtn);

    if (!my_exchange_inner_imp_->qry_trade_flag && rtn->error_no == 0)
    {
        my_exchange_inner_imp_->qry_trade_result = *rtn;
        my_exchange_inner_imp_->qry_trade_flag = true;
    }
}

void MYExchange::SendContractReturn(const T_ContractInfoReturn * rtn)
{
    MYExchangeLogUtil::OnContractInfoRtn(rtn, investorid);
    if (contract_info_return_handler_) contract_info_return_handler_(rtn);

    if (!my_exchange_inner_imp_->qry_contract_flag && rtn->error_no == 0)
    {
        my_exchange_inner_imp_->qry_contract_result = *rtn;
        my_exchange_inner_imp_->qry_contract_flag = true;
    }
}

T_PositionData MYExchange::GetPosition(SerialNoType sn, const std::string &contract)
{
    return my_exchange_inner_imp_->GetPosition(sn, contract);
}

extern "C" MYExchangeInterface *CreateExchange(struct my_xchg_cfg& cfg/*const std::string &future_exchange_config_file*/)
{
    return new MYExchange(cfg/*future_exchange_config_file*/);
}

void DestroyExchange(MYExchangeInterface * p)
{
    if (p) delete p;
}
