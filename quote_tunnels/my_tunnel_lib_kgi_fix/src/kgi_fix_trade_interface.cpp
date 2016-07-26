#include <iostream>
#include <time.h>
#include <map>
#include <string>
#include <mxml.h>
#include "my_cmn_log.h"
#include "qtm_with_code.h"

// f8 headers
#include <fix8/f8includes.hpp>

#include "KGIfix_types.hpp"
#include "KGIfix_router.hpp"
#include "KGIfix_classes.hpp"

#include "kgi_fix_trade_interface.h"
#include "trade_data_type.h"
#include "my_tunnel_lib.h"

typedef std::map<std::string, ContractInfo> ContractMap;

using namespace FIX8;

MYFixTrader::MYFixTrader(const std::string& cfg, const Tunnel_Info& tunnel_info)
    :
        mc_(new ReliableClientSession<MYFixSession>(KGI::ctx(), cfg, "DLD1")), order_id_(
            1), cfg_(cfg), tunnel_info_(tunnel_info), loggoned_(
        false)
{
    position_ = new position_log("./Position_datas.txt");
    mc_->session_ptr()->set_trader_ptr(this);
    try
    {
        mc_->start(false);
    }
    catch (f8Exception &e)
    {
        TNL_LOG_ERROR("FIX8 start error: %s", e.what());
        sleep(1);
    }
    catch (...)
    {
        TNL_LOG_ERROR("FIX8 start error: Unknown error.");
    }

    bool result1 = mc_->get_session_element()->GetAttr("sender_comp_id",
        account_);
    bool result2 = mc_->get_session_element()->GetAttr("target_comp_id",
        account_);
    bool result3 = mc_->get_session_element()->GetAttr("account", account_);

    if (!result1 || !result2 || !result3)
    {
        TNL_LOG_ERROR("Parse config in %s failed.", cfg_.c_str());
    }
    else
    {
        TNL_LOG_INFO("Parse config success.");
    }
}

int MYFixTrader::ReqOrderInsert(const T_PlaceOrder *po)
{
    if (!loggoned_)
    {
        TNL_LOG_ERROR("ReqOrderInsert when tunnel not ready.");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    if (mc_->session_ptr()->is_shutdown())
    {
        TNL_LOG_ERROR("Connection is shutdown");
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    //if (mc_->session_ptr()->get_session_state() <= States::st_logon_sent)
    //TNL_LOG_INFO("ReqOrderInsert.");
    FIX8::KGI::NewOrderSingle *nos(new FIX8::KGI::NewOrderSingle);
    char cl_orderid[20];
    char ordtype;
    char side;
    char time_in_force;
    std::string security_exchange;
    char transact_time[22];

    switch (po->order_kind)
    {
        case MY_TNL_OPT_LIMIT_PRICE:
            ordtype = FIX8::KGI::OrdType_LIMIT;
            *nos << new FIX8::KGI::Price(po->limit_price);
            break;
        case MY_TNL_OPT_ANY_PRICE:
            ordtype = FIX8::KGI::OrdType_MARKET;
            break;
        default:
            TNL_LOG_ERROR("order_kind error: %c", po->order_type);
            return TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION;
    }

    switch (po->direction)
    {
        case MY_TNL_D_BUY:
            side = FIX8::KGI::Side_BUY;
            break;
        case MY_TNL_D_SELL:
            side = FIX8::KGI::Side_SELL;
            break;
        default:
            TNL_LOG_ERROR("direction error: %c", po->direction);
            return TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION;
    }

    switch (po->order_type)
    {
        case MY_TNL_HF_NORMAL:
            time_in_force = FIX8::KGI::TimeInForce_DAY;
            break;
        case MY_TNL_HF_FAK:
            time_in_force = FIX8::KGI::TimeInForce_IMMEDIATE_OR_CANCEL;
            break;
        case MY_TNL_HF_FOK:
            time_in_force = FIX8::KGI::TimeInForce_FILL_OR_KILL;
            break;
        default:
            TNL_LOG_ERROR("order_type error: %c", po->order_type);
            return TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION;
    }

    switch (po->exchange_type)
    {
        case MY_TNL_EC_SZEX:
            security_exchange = KGI_FIX_SHENZHEN_B_STOCK;
            break;
        case MY_TNL_EC_SHEX:
            security_exchange = KGI_FIX_SHANGHAI_B_STOCK;
            break;
        case MY_TNL_EC_HKEX:
            security_exchange = KGI_FIX_HONGKONG_MARKET;
            break;
        default:
            TNL_LOG_ERROR("exchange_type error: %c", po->exchange_type);
            return TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION;
    }

    if (po->volume <= 0 || po->limit_price <= 0.0 || po->stock_code == '\0')
    {
        return TUNNEL_ERR_CODE::BAD_FIELD;
    }

    *nos << new FIX8::KGI::Account(account_)
        << new FIX8::KGI::HandlInst(
            FIX8::KGI::HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
        << new FIX8::KGI::OrderQty(po->volume)
        << new FIX8::KGI::OrdType(ordtype) << new FIX8::KGI::Side(side)
        << new FIX8::KGI::Symbol(po->stock_code)
        << new FIX8::KGI::TimeInForce(time_in_force)
        << new FIX8::KGI::SecurityExchange(security_exchange);

    {
        SetTransactTimeAndOrderId(transact_time, cl_orderid);
        *nos << new FIX8::KGI::TransactTime(transact_time)
            << new FIX8::KGI::ClOrdID(cl_orderid);
        std::lock_guard<std::mutex> lock(api_mutex_);
        try
        {
            if (mc_->session_ptr()->send(nos, false) == false)
            {
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("ReqOrderInsert failed: catch exception");
            return TUNNEL_ERR_CODE::RESULT_FAIL;
        }
        order_book_.insert(make_pair(po->serial_no, cl_orderid));
        serialno_book_.insert(make_pair(cl_orderid, po->serial_no));
        order_id_++;
    }
    TNL_LOG_INFO("Place order: stock_code: %s, price: %f, volume: %d",
        po->stock_code, po->limit_price, po->volume);
    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}

int MYFixTrader::ReqOrderAction(const T_CancelOrder *co)
{
    if (!loggoned_)
    {
        TNL_LOG_ERROR("ReqOrderAction when tunnel not ready.");
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }
    if (mc_->session_ptr()->is_shutdown())
    {
        TNL_LOG_ERROR("Connection is  shutdown");
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);
        return TUNNEL_ERR_CODE::NO_VALID_CONNECT_AVAILABLE;
    }

    FIX8::KGI::OrderCancelRequest *nos(new FIX8::KGI::OrderCancelRequest);

    SnToClOrdidMap::iterator it = order_book_.find(co->org_serial_no);
    if (it == order_book_.end())
    {
        TNL_LOG_ERROR("ReqOrderAction order not found.");
        return TUNNEL_ERR_CODE::ORDER_NOT_FOUND;
    }
    char cl_orderid[20];
    string orig_cl_ordid(it->second);
    char side;
    char transact_time[22];

    switch (co->direction)
    {
        case MY_TNL_D_BUY:
            side = FIX8::KGI::Side_BUY;
            break;
        case MY_TNL_D_SELL:
            side = FIX8::KGI::Side_SELL;
            break;
        default:
            return TUNNEL_ERR_CODE::UNSUPPORTED_FUNCTION;
    }

    *nos << new FIX8::KGI::OrigClOrdID(orig_cl_ordid)
        << new FIX8::KGI::Side(side)
        << new FIX8::KGI::Symbol(co->stock_code);

    {
        SetTransactTimeAndOrderId(transact_time, cl_orderid);
        *nos << new FIX8::KGI::ClOrdID(cl_orderid)
            << new FIX8::KGI::TransactTime(transact_time);
        std::lock_guard<std::mutex> lock(api_mutex_);
        try
        {
            if (mc_->session_ptr()->send(nos, false) == false)
            {
                TNL_LOG_ERROR("ReqOrderAction failed: fail to send");
                return TUNNEL_ERR_CODE::RESULT_FAIL;
            }
        }
        catch (...)
        {
            TNL_LOG_ERROR("ReqOrderAction failed: catch exception");
            return TUNNEL_ERR_CODE::RESULT_FAIL;
        }

        order_book_.insert(make_pair(co->serial_no, cl_orderid));
        serialno_book_.insert(make_pair(cl_orderid, co->serial_no));
        order_id_++;
    }
    TNL_LOG_INFO("Cancel order: stock_code: %s, price: %f, volume: %d",
        co->stock_code, co->limit_price, co->volume);
    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}

int MYFixTrader::QryPosition(T_PositionReturn &p)
{
    //TNL_LOG_WARN("QryPosition unsupported function");
    memset(&p, 0, sizeof(p));
    p.error_no = TUNNEL_ERR_CODE::RESULT_SUCCESS;
    position_->get_position(p.datas);

    if (QryPosReturnHandler_)
    {
        QryPosReturnHandler_(&p);
    }
    LogUtil::OnPositionRtn(&p, tunnel_info_);
    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}

int MYFixTrader::QryOrderDetail(const T_QryOrderDetail *p)
{
    TNL_LOG_WARN("QryOrderDetail unsupported function");
    T_OrderDetailReturn empty_element;
    memset(&empty_element, 0, sizeof(empty_element));
    if (QryOrderDetailReturnHandler_)
    {
        QryOrderDetailReturnHandler_(&empty_element);
    }
    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}

int MYFixTrader::QryTradeDetail(const T_QryTradeDetail *p)
{
    TNL_LOG_WARN("QryTradeDetail unsupported function");
    T_TradeDetailReturn empty_element;
    memset(&empty_element, 0, sizeof(empty_element));
    if (QryTradeDetailReturnHandler_)
    {
        QryTradeDetailReturnHandler_(&empty_element);
    }
    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}

int MYFixTrader::QryContractInfo(T_ContractInfoReturn &p)
{
    memset(&p, 0, sizeof(p));
    mxml_node_t *tree = LoadFileInTree("contract.xml");
    if (!tree)
    {
        TNL_LOG_ERROR("read contract data: contract_kgi.xml failed");
        return TUNNEL_ERR_CODE::RESULT_FAIL;
    }

    try
    {
        mxml_node_t *root = mxmlFindElement(tree, tree, "root", NULL, NULL,
        MXML_DESCEND);

        // get contract
        mxml_node_t *contract_node = mxmlFindElement(root, tree, "contracts",
        NULL, NULL, MXML_DESCEND);
        if (contract_node)
        {
            mxml_node_t *item_node = mxmlFindElement(contract_node, root,
                "item", NULL, NULL, MXML_DESCEND);
            while (item_node)
            {
                if (item_node->type == MXML_ELEMENT)
                {
                    ContractInfo param;
                    memset(&param, 0, sizeof(param));
                    if (mxmlElementGetAttr(item_node, "symbol"))
                    {
                        strcpy(param.stock_code, mxmlElementGetAttr(item_node, "symbol"));
                    }
                    if (mxmlElementGetAttr(item_node, "expiry"))
                    {
                        strcpy(param.ExpireDate, mxmlElementGetAttr(item_node, "expiry"));
                    }

                    strcpy(param.TradeDate, my_cmn::GetCurrentDateString().c_str());
                    p.datas.push_back(param);
                }
                item_node = mxmlGetNextSibling(item_node);
            }
        }
        TNL_LOG_INFO("contracts size:%d", p.datas.size());
    }
    catch (...)
    {
        TNL_LOG_ERROR("parse contract information failed.");
    }

    mxmlDelete(tree);
    if (QryContractInfoHandler_)
    {
        QryContractInfoHandler_(&p);
    }
    LogUtil::OnContractInfoRtn(&p, tunnel_info_);

    return TUNNEL_ERR_CODE::RESULT_SUCCESS;
}
