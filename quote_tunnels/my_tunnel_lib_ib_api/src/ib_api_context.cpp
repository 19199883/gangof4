#include "ib_api_context.h"

#include "tunnel_cmn_utility.h"
#include "my_cmn_util_funcs.h"

using namespace my_cmn;

TradeContext::TradeContext()
{
    req_id_ = 0;

    LoadContractFromFile();
    client_id_ = 1;
    perm_id_ = 33;
}

void TradeContext::LoadContractFromFile()
{
    mxml_node_t *tree = LoadFileInTree("contract.xml");

    if (!tree)
    {
        return;
    }

    try
    {
        mxml_node_t *root = mxmlFindElement(tree, tree, "root", NULL, NULL, MXML_DESCEND);

        // get contract
        mxml_node_t *contract_node = mxmlFindElement(root, tree, "contracts", NULL, NULL, MXML_DESCEND);
        if (contract_node)
        {
            mxml_node_t *item_node = mxmlFindElement(contract_node, root, "item", NULL, NULL, MXML_DESCEND);
            while (item_node)
            {
                if (item_node->type == MXML_ELEMENT)
                {
                    Contract param;
                    // <item symbol="2801" sec_type="STK" symbol_ib="2801" expiry="" exchange="SEHK" currency="HKD"/>
                    std::string symbol = mxmlElementGetAttr(item_node, "symbol");

                    param.secType = mxmlElementGetAttr(item_node, "sec_type");
                    param.symbol = mxmlElementGetAttr(item_node, "symbol_ib");
                    param.expiry = mxmlElementGetAttr(item_node, "expiry");
                    param.exchange = mxmlElementGetAttr(item_node, "exchange");
                    param.currency = mxmlElementGetAttr(item_node, "currency");

                    contracts_.insert(std::make_pair(symbol, param));
                }
                item_node = mxmlGetNextSibling(item_node);
            }
        }

        // show parse params
        MY_LOG_INFO("contracts size:%d", contracts_.size());
    }
    catch (...)
    {
        TNL_LOG_ERROR("parse contract information failed.");
    }

    mxmlDelete(tree);
}

int TradeContext::GetNewReqID()
{
    std::lock_guard<std::mutex> lock(context_mutex);
    return req_id_++;
}

const Contract* TradeContext::GetContractInfo(const char* po_stockcode)
{
    ContractMap::const_iterator contract_it = contracts_.find(po_stockcode);
    if (contract_it == contracts_.end())
    {
        TNL_LOG_ERROR("can't find contract information of %s.", po_stockcode);
        return NULL;
    }

    return &contract_it->second;
}

std::string TradeContext::GetSymbolOnMarket(const Contract &contract)
{
    for (ContractMap::const_iterator it = contracts_.begin(); it != contracts_.end(); ++it)
    {
        if (contract.symbol == it->second.symbol
            && (contract.expiry.empty() || strncmp(contract.expiry.c_str(), it->second.expiry.c_str(), 6) == 0))
        {
            return it->first;
        }
    }

    return "";
}

void TradeContext::GetContractInfo(T_ContractInfoReturn &c_ret)
{
    std::string cur_trade_day = my_cmn::GetCurrentDateString();
    for (ContractMap::const_iterator it = contracts_.begin(); it != contracts_.end(); ++it)
    {
        ContractInfo c;
        strncpy(c.stock_code, it->first.c_str(), sizeof(c.stock_code));
        strncpy(c.ExpireDate, it->second.expiry.c_str(), sizeof(c.ExpireDate));
        strncpy(c.TradeDate, cur_trade_day.c_str(), sizeof(c.TradeDate));

        c_ret.datas.push_back(c);
    }
}

const TnlOrderInfo* TradeContext::GetOrderByOrderID(long order_id)
{
    std::lock_guard<std::mutex> lock(order_mutex);

    IDToOrderMap::const_iterator cit = id_to_order_.find(order_id);
    if (cit != id_to_order_.end())
    {
        return cit->second;
    }
    return NULL;
}

const TnlOrderInfo* TradeContext::GetOrderBySn(SerialNoType order_sn)
{
    std::lock_guard<std::mutex> lock(order_mutex);

    SnToOrderMap::const_iterator cit = sn_to_order_.find(order_sn);
    if (cit != sn_to_order_.end())
    {
        return cit->second;
    }
    return NULL;
}

bool TradeContext::CreateNewOrder(const T_PlaceOrder *p, long order_id,
    Order &order, Contract &contract, T_OrderRespond &order_rsp)
{
    // fillup respond message
    bzero(&order_rsp, sizeof(order_rsp));
    order_rsp.entrust_no = 0;
    order_rsp.entrust_status = MY_TNL_OS_REPORDED;
    order_rsp.serial_no = p->serial_no;
    order_rsp.error_no = TUNNEL_ERR_CODE::RESULT_SUCCESS;

    // fillup contract fields
    ContractMap::const_iterator contract_it = contracts_.find(p->stock_code);
    if (contract_it == contracts_.end())
    {
        order_rsp.entrust_status = MY_TNL_OS_UNREPORT;
        order_rsp.error_no = TUNNEL_ERR_CODE::INSTRUMENT_NOT_FOUND;

        TNL_LOG_ERROR("can't find contract information of %s.", p->stock_code);
        return false;
    }

    contract = contract_it->second;
    TnlOrderInfo *po = new TnlOrderInfo(p, order_id);
    order_rsp.entrust_no = po->order_id;

    // fillup order fields
    order.orderId = po->order_id;
    order.clientId = client_id_;
    order.permId = perm_id_;

    if (p->direction == MY_TNL_D_BUY)
    {
        order.action = "BUY";
    }
    else if (p->direction == MY_TNL_D_SELL)
    {
        order.action = "SELL";
    }

    order.lmtPrice = p->limit_price;
    order.totalQuantity = p->volume;

    if (p->order_kind == MY_TNL_OPT_ANY_PRICE)
    {
        order.orderType = "MKT";
    }
    else if (p->order_kind == MY_TNL_OPT_LIMIT_PRICE)
    {
        order.orderType = "LMT";
    }

    order.transmit = true;
    order.account = "";

    // save to context map
    {
        std::lock_guard<std::mutex> lock(order_mutex);
        id_to_order_.insert(std::make_pair(po->order_id, po));
        sn_to_order_.insert(std::make_pair(p->serial_no, po));
    }

    return true;
}

void TradeContext::BuildOrderRtn(const TnlOrderInfo* p, T_OrderReturn& order_return)
{
    bzero(&order_return, sizeof(order_return));
    order_return.entrust_no = p->order_id;
    order_return.entrust_status = p->entrust_status;
    order_return.serial_no = p->po.serial_no;

    strncpy(order_return.stock_code, p->po.stock_code, sizeof(order_return.stock_code));
    order_return.direction = p->po.direction;
    order_return.open_close = p->po.open_close;
    order_return.speculator = p->po.speculator;
    order_return.volume = p->po.volume;
    order_return.limit_price = p->po.limit_price;

    order_return.volume_remain = p->volume_remain;

    order_return.exchange_type = p->po.exchange_type;
}
void TradeContext::BuildTradeRtn(const TnlOrderInfo* p, int filled, double fill_price, T_TradeReturn &trade_return)
{
    memset(&trade_return, 0, sizeof(trade_return));
    trade_return.business_no = 0;
    trade_return.business_price = fill_price;
    trade_return.business_volume = filled;
    trade_return.entrust_no = p->order_id;
    trade_return.serial_no = p->po.serial_no;

    strncpy(trade_return.stock_code, p->po.stock_code, sizeof(trade_return.stock_code));
    trade_return.direction = p->po.direction;
    trade_return.open_close = p->po.open_close;

    trade_return.exchange_type = p->po.exchange_type;
}

void TradeContext::BuildCancelRsp(const TnlOrderInfo* p, int error_no, T_CancelRespond& rsp)
{
    bzero(&rsp, sizeof(rsp));
    rsp.serial_no = p->last_cancel_sn;              // 撤单请求序列号
    rsp.error_no = error_no;                        // 错误号，0：正常；其它：异常
    rsp.entrust_no = p->order_id;                   // 交易所委托号
    rsp.entrust_status = MY_TNL_OS_REPORDED;        // 撤单操作状态（已报、拒绝，只能为这两种状态中的一个）
    if (error_no != TUNNEL_ERR_CODE::RESULT_SUCCESS) rsp.entrust_status = MY_TNL_OS_ERROR;
}

char TradeContext::GetExchangeID(const std::string &exchange_name)
{
    if (exchange_name == MY_TNL_EXID_SGX)
    {
        return MY_TNL_EC_SGX;
    }
    else if (exchange_name == MY_TNL_EXID_HKEX)
    {
        return MY_TNL_EC_HKEX;
    }
    else if (exchange_name == "SSE")
    {
        return MY_TNL_EC_SHEX;
    }
    else if (exchange_name == "SZE")
    {
        return MY_TNL_EC_SZEX;
    }

    return MY_TNL_EC_SGX;
}
