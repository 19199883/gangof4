#include <fix8/f8includes.hpp>
#include <fix8/field.hpp>

#include "kgi_fix_struct.h"
#include "KGIfix_types.hpp"
#include "KGIfix_classes.hpp"

#include "my_trade_tunnel_struct.h"
#include "kgi_fix_trade_interface.h"
#include "check_schedule.h"
#include "qtm_with_code.h"

using namespace FIX8;
using namespace std;

bool MYFixRouter::operator()(const FIX8::KGI::ExecutionReport *msg) const
{
    long serial_no;
    FIX8::KGI::ClOrdID cl_ordid;
    if (msg->get(cl_ordid))
    {
        if (!cl_ordid.is_valid())
        {
            TNL_LOG_ERROR("ExecutionReport ClOrdID is not valid.");
            return false;
        }
        ClOrdidToSnMap::iterator it = session_.GetClOrdIdPos(cl_ordid.get());
        if (session_.ItIsEnd(it))
        {
            TNL_LOG_ERROR("ExecutionReport can not find serial num.");
            return false;
        }
        else
        {
            serial_no = it->second;
        }

        FIX8::KGI::OrdStatus status;
        if (msg->get(status))
        {
            //TNL_LOG_INFO("ExecutionReport order status: %c", status.get());
            switch (status.get())
            {
                case '0':
                {
                    T_OrderReturn rtn;
                    memset(&rtn, 0, sizeof(rtn));
                    FIX8::KGI::Text text;
                    FIX8::KGI::Symbol symbol;
                    FIX8::KGI::Side side;
                    FIX8::KGI::Price price;
                    FIX8::KGI::OrderQty volume;
                    FIX8::KGI::LeavesQty remain_vol;
                    FIX8::KGI::SecurityExchange exchange;
                    FIX8::KGI::ClOrdID ordid;

                    rtn.serial_no = serial_no;

                    if (msg->get(text))
                    {
                        if (text.is_valid())
                        {
                            if (text.get() == "Pend")
                            {
                                TNL_LOG_INFO("Status: New Pending.");
                                rtn.entrust_status = MY_TNL_OS_UNREPORT;
                            }
                            else if (text.get() == "Queu")
                            {
                                TNL_LOG_INFO("Status: New Queuing.");
                                rtn.entrust_status = MY_TNL_OS_REPORDED;
                            }
                        }
                    }

                    if (msg->get(symbol))
                    {
                        if (symbol.is_valid())
                        {
                            strcpy(rtn.stock_code, symbol.get().c_str());
                        }
                    }

                    if (msg->get(side))
                    {
                        if (side.is_valid())
                        {
                            if (side.get() == FIX8::KGI::Side_BUY)
                            {
                                rtn.direction = MY_TNL_D_BUY;
                            }
                            else if (side.get() == FIX8::KGI::Side_SELL)
                            {
                                rtn.direction = MY_TNL_D_SELL;
                            }
                        }
                    }

                    if (msg->get(volume))
                    {
                        if (volume.is_valid())
                        {
                            rtn.volume = volume.get();
                        }
                    }

                    if (msg->get(price))
                    {
                        if (price.is_valid())
                        {
                            rtn.limit_price = price.get();
                        }
                    }

                    if (msg->get(remain_vol))
                    {
                        if (remain_vol.is_valid())
                        {
                            rtn.volume_remain = remain_vol.get();
                        }
                    }

                    if (msg->get(exchange))
                    {
                        if (exchange.is_valid())
                        {
                            if (exchange.get() == KGI_FIX_SHENZHEN_B_STOCK)
                            {
                                rtn.exchange_type = MY_TNL_EC_SZEX;
                            }
                            else if (exchange.get() == KGI_FIX_SHANGHAI_B_STOCK)
                            {
                                rtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                            else if (exchange.get() == KGI_FIX_HONGKONG_MARKET)
                            {
                                rtn.exchange_type = MY_TNL_EC_HKEX;
                            }
                            else
                            {
                                rtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                        }
                    }

                    if (msg->get(ordid))
                    {
                        if (ordid.is_valid())
                        {
                            rtn.entrust_no = session_.GetEntrustNo(ordid.get());
                        }
                    }

                    TNL_LOG_INFO(
                        "Order return: stock_code: %s, exchange_type: %c, direction: %c, entrust_no: %d, serial_no: %d, volume: %d, limit_price: %f.",
                        rtn.stock_code, rtn.exchange_type, rtn.direction,
                        rtn.entrust_no, rtn.serial_no, rtn.volume,
                        rtn.limit_price);
                    session_.SentToOrderReturnCallBackHandler(&rtn);
                    LogUtil::OnOrderReturn(&rtn, session_.GetTunnelInfo());
                }
                    break;
                case '1':
                {
                    TNL_LOG_INFO("Status: Partially Filled.");
                    T_OrderReturn ortn;
                    memset(&ortn, 0, sizeof(ortn));
                    T_TradeReturn trtn;
                    memset(&trtn, 0, sizeof(trtn));
                    FIX8::KGI::Symbol symbol;
                    FIX8::KGI::Side side;
                    FIX8::KGI::Price price;
                    FIX8::KGI::OrderQty volume;
                    FIX8::KGI::LeavesQty remain_vol;
                    FIX8::KGI::SecurityExchange exchange;
                    FIX8::KGI::ClOrdID ordid;
                    FIX8::KGI::ExecID execid;
                    FIX8::KGI::LastPx last_price;
                    FIX8::KGI::LastShares last_vol;

                    ortn.serial_no = serial_no;
                    trtn.serial_no = serial_no;
                    ortn.entrust_status = MY_TNL_OS_PARTIALCOM;

                    if (msg->get(symbol))
                    {
                        if (symbol.is_valid())
                        {
                            strcpy(ortn.stock_code, symbol.get().c_str());
                            strcpy(trtn.stock_code, symbol.get().c_str());
                        }
                    }

                    if (msg->get(side))
                    {
                        if (side.is_valid())
                        {
                            if (side.get() == FIX8::KGI::Side_BUY)
                            {
                                ortn.direction = MY_TNL_D_BUY;
                                trtn.direction = MY_TNL_D_BUY;
                            }
                            else if (side.get() == FIX8::KGI::Side_SELL)
                            {
                                ortn.direction = MY_TNL_D_SELL;
                                trtn.direction = MY_TNL_D_SELL;
                            }
                        }
                    }

                    if (msg->get(price))
                    {
                        if (price.is_valid())
                        {
                            ortn.limit_price = price.get();
                        }
                    }

                    if (msg->get(volume))
                    {
                        if (volume.is_valid())
                        {
                            ortn.volume = volume.get();
                        }
                    }

                    if (msg->get(remain_vol))
                    {
                        if (remain_vol.is_valid())
                        {
                            ortn.volume_remain = remain_vol.get();
                        }
                    }

                    if (msg->get(exchange))
                    {
                        if (exchange.is_valid())
                        {
                            if (exchange.get() == KGI_FIX_SHENZHEN_B_STOCK)
                            {
                                ortn.exchange_type = MY_TNL_EC_SZEX;
                                trtn.exchange_type = MY_TNL_EC_SZEX;
                            }
                            else if (exchange.get() == KGI_FIX_SHANGHAI_B_STOCK)
                            {
                                ortn.exchange_type = MY_TNL_EC_SHEX;
                                trtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                            else if (exchange.get() == KGI_FIX_HONGKONG_MARKET)
                            {
                                ortn.exchange_type = MY_TNL_EC_HKEX;
                                trtn.exchange_type = MY_TNL_EC_HKEX;
                            }
                            else
                            {
                                ortn.exchange_type = MY_TNL_EC_SHEX;
                                trtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                        }
                    }

                    if (msg->get(ordid))
                    {
                        if (ordid.is_valid())
                        {
                            ortn.entrust_no = session_.GetEntrustNo(ordid.get());
                            trtn.entrust_no = ortn.entrust_no;
                        }
                    }

                    if (msg->get(execid))
                    {
                        if (execid.is_valid())
                        {
                            trtn.business_no = atoi(execid.get().c_str());
                        }
                    }

                    if (msg->get(last_vol))
                    {
                        if (last_vol.is_valid())
                        {
                            trtn.business_volume = last_vol.get();
                        }
                    }

                    if (msg->get(last_price))
                    {
                        if (last_price.is_valid())
                        {
                            trtn.business_price = last_price.get();
                        }
                    }

                    TNL_LOG_INFO(
                        "Order return: stock_code: %s, exchange_type: %c, direction: %c, entrust_no: %d, serial_no: %d, volume: %d, limit_price: %f.",
                        ortn.stock_code, ortn.exchange_type, ortn.direction,
                        ortn.entrust_no, ortn.serial_no, ortn.volume,
                        ortn.limit_price);
                    session_.SentToOrderReturnCallBackHandler(&ortn);
                    LogUtil::OnOrderReturn(&ortn, session_.GetTunnelInfo());

                    session_.SentToTradeReturnCallBackHandler(&trtn);
                    LogUtil::OnTradeReturn(&trtn, session_.GetTunnelInfo());
                    session_.UpdatePositionData(trtn);
                }
                    break;
                case '2':
                {
                    TNL_LOG_INFO("Status: Filled.");
                    T_OrderReturn ortn;
                    memset(&ortn, 0, sizeof(ortn));
                    T_TradeReturn trtn;
                    memset(&trtn, 0, sizeof(trtn));
                    FIX8::KGI::Symbol symbol;
                    FIX8::KGI::Side side;
                    FIX8::KGI::Price price;
                    FIX8::KGI::OrderQty volume;
                    FIX8::KGI::LeavesQty remain_vol;
                    FIX8::KGI::SecurityExchange exchange;
                    FIX8::KGI::ClOrdID ordid;
                    FIX8::KGI::ExecID execid;
                    FIX8::KGI::LastPx last_price;
                    FIX8::KGI::LastShares last_vol;

                    ortn.serial_no = serial_no;
                    trtn.serial_no = serial_no;
                    ortn.entrust_status = MY_TNL_OS_COMPLETED;

                    if (msg->get(symbol))
                    {
                        if (symbol.is_valid())
                        {
                            strcpy(ortn.stock_code, symbol.get().c_str());
                            strcpy(trtn.stock_code, symbol.get().c_str());
                        }
                    }

                    if (msg->get(side))
                    {
                        if (side.is_valid())
                        {
                            if (side.get() == FIX8::KGI::Side_BUY)
                            {
                                ortn.direction = MY_TNL_D_BUY;
                                trtn.direction = MY_TNL_D_BUY;
                            }
                            else if (side.get() == FIX8::KGI::Side_SELL)
                            {
                                ortn.direction = MY_TNL_D_SELL;
                                trtn.direction = MY_TNL_D_SELL;
                            }
                        }
                    }

                    if (msg->get(price))
                    {
                        if (price.is_valid())
                        {
                            ortn.limit_price = price.get();
                        }
                    }

                    if (msg->get(volume))
                    {
                        if (volume.is_valid())
                        {
                            ortn.volume = volume.get();
                        }
                    }

                    if (msg->get(remain_vol))
                    {
                        if (remain_vol.is_valid())
                        {
                            ortn.volume_remain = remain_vol.get();
                        }
                    }

                    if (msg->get(exchange))
                    {
                        if (exchange.is_valid())
                        {
                            if (exchange.get() == KGI_FIX_SHENZHEN_B_STOCK)
                            {
                                ortn.exchange_type = MY_TNL_EC_SZEX;
                                trtn.exchange_type = MY_TNL_EC_SZEX;
                            }
                            else if (exchange.get() == KGI_FIX_SHANGHAI_B_STOCK)
                            {
                                ortn.exchange_type = MY_TNL_EC_SHEX;
                                trtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                            else if (exchange.get() == KGI_FIX_HONGKONG_MARKET)
                            {
                                ortn.exchange_type = MY_TNL_EC_HKEX;
                                trtn.exchange_type = MY_TNL_EC_HKEX;
                            }
                            else
                            {
                                ortn.exchange_type = MY_TNL_EC_SHEX;
                                trtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                        }
                    }

                    if (msg->get(ordid))
                    {
                        if (ordid.is_valid())
                        {
                            ortn.entrust_no = session_.GetEntrustNo(ordid.get());
                            trtn.entrust_no = ortn.entrust_no;
                        }
                    }

                    if (msg->get(execid))
                    {
                        if (execid.is_valid())
                        {
                            trtn.business_no = atoi(execid.get().c_str());
                        }
                    }

                    if (msg->get(last_vol))
                    {
                        if (last_vol.is_valid())
                        {
                            trtn.business_volume = last_vol.get();
                        }
                    }

                    if (msg->get(last_price))
                    {
                        if (last_price.is_valid())
                        {
                            trtn.business_price = last_price.get();
                        }
                    }
                    //cout << "ComplianceCheck_OnOrderFilled SerialNo: " << serial_no << " User: " << session_.GetUser().c_str() << endl;
                    ComplianceCheck_OnOrderFilled(session_.GetTunnelInfo().account.c_str(), serial_no);
                    TNL_LOG_INFO(
                        "Order return: stock_code: %s, exchange_type: %c, direction: %c, entrust_no: %d, serial_no: %d, volume: %d, limit_price: %f.",
                        ortn.stock_code, ortn.exchange_type, ortn.direction,
                        ortn.entrust_no, ortn.serial_no, ortn.volume,
                        ortn.limit_price);

                    session_.SentToOrderReturnCallBackHandler(&ortn);
                    LogUtil::OnOrderReturn(&ortn, session_.GetTunnelInfo());

                    session_.SentToTradeReturnCallBackHandler(&trtn);
                    LogUtil::OnTradeReturn(&trtn, session_.GetTunnelInfo());
                    session_.UpdatePositionData(trtn);
                }
                    break;
                case '4':
                {
                    TNL_LOG_INFO("Status: Canceled.");
                    T_OrderReturn rtn;
                    memset(&rtn, 0, sizeof(rtn));
                    FIX8::KGI::Text text;
                    FIX8::KGI::Symbol symbol;
                    FIX8::KGI::Side side;
                    FIX8::KGI::Price price;
                    FIX8::KGI::OrderQty volume;
                    FIX8::KGI::LeavesQty remain_vol;
                    FIX8::KGI::SecurityExchange exchange;
                    FIX8::KGI::ClOrdID ordid;

                    rtn.serial_no = serial_no;
                    rtn.entrust_status = MY_TNL_OS_WITHDRAWED;

                    if (msg->get(symbol))
                    {
                        if (symbol.is_valid())
                        {
                            strcpy(rtn.stock_code, symbol.get().c_str());
                        }
                    }

                    if (msg->get(side))
                    {
                        if (side.is_valid())
                        {
                            if (side.get() == FIX8::KGI::Side_BUY)
                            {
                                rtn.direction = MY_TNL_D_BUY;
                            }
                            else if (side.get() == FIX8::KGI::Side_SELL)
                            {
                                rtn.direction = MY_TNL_D_SELL;
                            }
                        }
                    }

                    if (msg->get(price))
                    {
                        if (price.is_valid())
                        {
                            rtn.limit_price = price.get();
                        }
                    }

                    if (msg->get(volume))
                    {
                        if (volume.is_valid())
                        {
                            rtn.volume = volume.get();
                        }
                    }

                    if (msg->get(remain_vol))
                    {
                        if (remain_vol.is_valid())
                        {
                            rtn.volume_remain = remain_vol.get();
                        }
                    }

                    if (msg->get(exchange))
                    {
                        if (exchange.is_valid())
                        {
                            if (exchange.get() == KGI_FIX_SHENZHEN_B_STOCK)
                            {
                                rtn.exchange_type = MY_TNL_EC_SZEX;
                            }
                            else if (exchange.get() == KGI_FIX_SHANGHAI_B_STOCK)
                            {
                                rtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                            else if (exchange.get() == KGI_FIX_HONGKONG_MARKET)
                            {
                                rtn.exchange_type = MY_TNL_EC_HKEX;
                            }
                            else
                            {
                                rtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                        }
                    }

                    if (msg->get(ordid))
                    {
                        if (ordid.is_valid())
                        {
                            rtn.entrust_no = session_.GetEntrustNo(ordid.get());
                        }
                    }
                    //cout << "ComplianceCheck_OnOrderCanceled SerialNo: " << serial_no << " User: "<<  session_.GetUser().c_str() <<  endl;
                    ComplianceCheck_OnOrderCanceled(
                        session_.GetTunnelInfo().account.c_str(),
                        rtn.stock_code,
                        serial_no,
                        rtn.exchange_type,
                        rtn.volume,
                        rtn.speculator,
                        rtn.open_close);

                    TNL_LOG_INFO(
                        "Order return: stock_code: %s, exchange_type: %c, direction: %c, entrust_no: %d, serial_no: %d, volume: %d, limit_price: %f.",
                        rtn.stock_code, rtn.exchange_type, rtn.direction,
                        rtn.entrust_no, rtn.serial_no, rtn.volume,
                        rtn.limit_price);
                    session_.SentToOrderReturnCallBackHandler(&rtn);
                    LogUtil::OnOrderReturn(&rtn, session_.GetTunnelInfo());
                }
                    break;
                case '6':
                {
                    TNL_LOG_INFO("Status: Pending Canceled.");
                    T_OrderReturn rtn;
                    memset(&rtn, 0, sizeof(rtn));
                    FIX8::KGI::Text text;
                    FIX8::KGI::Symbol symbol;
                    FIX8::KGI::Side side;
                    FIX8::KGI::Price price;
                    FIX8::KGI::OrderQty volume;
                    FIX8::KGI::LeavesQty remain_vol;
                    FIX8::KGI::SecurityExchange exchange;
                    FIX8::KGI::ClOrdID ordid;

                    rtn.serial_no = serial_no;
                    rtn.entrust_status = MY_TNL_OS_WITHDRAWING;

                    if (msg->get(symbol))
                    {
                        if (symbol.is_valid())
                        {
                            strcpy(rtn.stock_code, symbol.get().c_str());
                        }
                    }

                    if (msg->get(side))
                    {
                        if (side.is_valid())
                        {
                            if (side.get() == FIX8::KGI::Side_BUY)
                            {
                                rtn.direction = MY_TNL_D_BUY;
                            }
                            else if (side.get() == FIX8::KGI::Side_SELL)
                            {
                                rtn.direction = MY_TNL_D_SELL;
                            }
                        }
                    }

                    if (msg->get(price))
                    {
                        if (price.is_valid())
                        {
                            rtn.limit_price = price.get();
                        }
                    }

                    if (msg->get(volume))
                    {
                        if (volume.is_valid())
                        {
                            rtn.volume = volume.get();
                        }
                    }

                    if (msg->get(remain_vol))
                    {
                        if (remain_vol.is_valid())
                        {
                            rtn.volume_remain = remain_vol.get();
                        }
                    }

                    if (msg->get(exchange))
                    {
                        if (exchange.is_valid())
                        {
                            if (exchange.get() == KGI_FIX_SHENZHEN_B_STOCK)
                            {
                                rtn.exchange_type = MY_TNL_EC_SZEX;
                            }
                            else if (exchange.get() == KGI_FIX_SHANGHAI_B_STOCK)
                            {
                                rtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                            else if (exchange.get() == KGI_FIX_HONGKONG_MARKET)
                            {
                                rtn.exchange_type = MY_TNL_EC_HKEX;
                            }
                            else
                            {
                                rtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                        }
                    }

                    if (msg->get(ordid))
                    {
                        if (ordid.is_valid())
                        {
                            rtn.entrust_no = session_.GetEntrustNo(ordid.get());
                        }
                    }
                    TNL_LOG_INFO(
                        "Order return: stock_code: %s, exchange_type: %c, direction: %c, entrust_no: %d, serial_no: %d, volume: %d, limit_price: %f.",
                        rtn.stock_code, rtn.exchange_type, rtn.direction,
                        rtn.entrust_no, rtn.serial_no, rtn.volume,
                        rtn.limit_price);
                    session_.SentToOrderReturnCallBackHandler(&rtn);
                    LogUtil::OnOrderReturn(&rtn, session_.GetTunnelInfo());
                }
                    break;
                case '8':
                {
                    FIX8::KGI::Text text;
                    if (msg->get(text))
                    {
                        TNL_LOG_INFO("Status: Rejected, Information: %s",
                            text.get().c_str());
                    }
                    T_OrderReturn rtn;
                    memset(&rtn, 0, sizeof(rtn));

                    FIX8::KGI::Symbol symbol;
                    FIX8::KGI::Side side;
                    FIX8::KGI::Price price;
                    FIX8::KGI::OrderQty volume;
                    FIX8::KGI::LeavesQty remain_vol;
                    FIX8::KGI::SecurityExchange exchange;
                    FIX8::KGI::ClOrdID ordid;

                    rtn.serial_no = serial_no;
                    rtn.entrust_status = MY_TNL_OS_ERROR;

                    if (msg->get(symbol))
                    {
                        if (symbol.is_valid())
                        {
                            strcpy(rtn.stock_code, symbol.get().c_str());
                        }
                    }

                    if (msg->get(side))
                    {
                        if (side.is_valid())
                        {
                            if (side.get() == FIX8::KGI::Side_BUY)
                            {
                                rtn.direction = MY_TNL_D_BUY;
                            }
                            else if (side.get() == FIX8::KGI::Side_SELL)
                            {
                                rtn.direction = MY_TNL_D_SELL;
                            }
                        }
                    }

                    if (msg->get(price))
                    {
                        if (price.is_valid())
                        {
                            rtn.limit_price = price.get();
                        }
                    }

                    if (msg->get(volume))
                    {
                        if (volume.is_valid())
                        {
                            rtn.volume = volume.get();
                        }
                    }

                    if (msg->get(remain_vol))
                    {
                        if (remain_vol.is_valid())
                        {
                            rtn.volume_remain = remain_vol.get();
                        }
                    }

                    if (msg->get(exchange))
                    {
                        if (exchange.is_valid())
                        {
                            if (exchange.get() == KGI_FIX_SHENZHEN_B_STOCK)
                            {
                                rtn.exchange_type = MY_TNL_EC_SZEX;
                            }
                            else if (exchange.get() == KGI_FIX_SHANGHAI_B_STOCK)
                            {
                                rtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                            else if (exchange.get() == KGI_FIX_HONGKONG_MARKET)
                            {
                                rtn.exchange_type = MY_TNL_EC_HKEX;
                            }
                            else
                            {
                                rtn.exchange_type = MY_TNL_EC_SHEX;
                            }
                        }
                    }

                    if (msg->get(ordid))
                    {
                        if (ordid.is_valid())
                        {
                            rtn.entrust_no = 0;
                        }
                    }
                    //cout << "ComplianceCheck_OnOrderInsertFailed SerialNo: " << serial_no << " User: " << session_.GetUser().c_str() << endl;
                    ComplianceCheck_OnOrderInsertFailed(
                        session_.GetTunnelInfo().account.c_str(),
                        serial_no,
                        rtn.exchange_type,
                        rtn.stock_code,
                        rtn.volume,
                        rtn.speculator,
                        rtn.open_close,
                        MY_TNL_HF_NORMAL);

                    TNL_LOG_INFO(
                        "Order return: stock_code: %s, exchange_type: %c, direction: %c, entrust_no: %d, serial_no: %d, volume: %d, limit_price: %f.",
                        rtn.stock_code, rtn.exchange_type, rtn.direction,
                        rtn.entrust_no, rtn.serial_no, rtn.volume,
                        rtn.limit_price);
                    session_.SentToOrderReturnCallBackHandler(&rtn);
                    LogUtil::OnOrderReturn(&rtn, session_.GetTunnelInfo());
                }
                    break;
                default:
                    TNL_LOG_ERROR("ExecutionReport unkown ordstatus.");
                    break;
            }
            return true;
        }
    }
    return false;
}

bool MYFixRouter::operator()(
    const class FIX8::KGI::OrderCancelReject *msg) const
{
    TNL_LOG_INFO("OrderCancelReject Called.");
    long serial_no;
    FIX8::KGI::Text text;
    if (msg->get(text))
    {
        if (text.is_valid())
        {
            TNL_LOG_INFO("Cancel Reject: %s", text.get().c_str());
        }
    }

    FIX8::KGI::ClOrdID cl_ordid;
    if (msg->get(cl_ordid))
    {
        if (!cl_ordid.is_valid())
        {
            TNL_LOG_ERROR("ExecutionReport ClOrdID is not valid.");
            return false;
        }
        ClOrdidToSnMap::iterator it = session_.GetClOrdIdPos(cl_ordid.get());
        if (session_.ItIsEnd(it))
        {
            FIX8::KGI::OrigClOrdID oricl_ordid;
            if (msg->get(oricl_ordid))
            {
                if (!oricl_ordid.is_valid())
                {
                    TNL_LOG_ERROR("ExecutionReport OrigClOrdID is not valid.");
                    return false;
                }
                it = session_.GetClOrdIdPos(oricl_ordid.get());
                if (session_.ItIsEnd(it))
                {
                    TNL_LOG_ERROR("ExecutionReport can not find serial num.");
                    return false;
                }
                else
                {
                    serial_no = it->second;
                }
            }
        }
        else
        {
            serial_no = it->second;
        }

        T_OrderReturn rtn;
        memset(&rtn, 0, sizeof(rtn));
        rtn.serial_no = serial_no;
        rtn.entrust_status = MY_TNL_OS_ERROR;
        rtn.entrust_no = 0;

        //cout << "ComplianceCheck_OnOrderCancelFailed SerialNo: " << serial_no << " User: " << session_.GetUser().c_str() << endl;
        ComplianceCheck_OnOrderCancelFailed(
            session_.GetTunnelInfo().account.c_str(),
            rtn.stock_code,
            serial_no);

        session_.SentToOrderReturnCallBackHandler(&rtn);
        LogUtil::OnOrderReturn(&rtn, session_.GetTunnelInfo());

        return true;
    }
    TNL_LOG_ERROR("OrderCancelReject get ClOrdID error.");
    return false;
}

bool MYFixRouter::operator()(const class FIX8::KGI::Reject *msg) const
{
    FIX8::KGI::Text text;
    if (msg->get(text))
    {
        if (text.is_valid())
        {
            TNL_LOG_WARN("FIX Reject: %s", text.get().c_str());
        }
    }
    return false;
}

bool MYFixSession::handle_logon(const unsigned seqnum,
    const FIX8::Message *msg)
{
    if (p_trd_)
    {
        p_trd_->loggoned_ = my_handle_logon(seqnum, msg);
        if (p_trd_->loggoned_)
        {
            TunnelUpdateState(GetTunnelInfo().qtm_name.c_str(), QtmState::API_READY);
        }
        return p_trd_->loggoned_;
    }
    else
    {
        return my_handle_logon(seqnum, msg);
    }
}

bool MYFixSession::my_handle_logon(const unsigned seqnum,
    const FIX8::Message *msg)
{
    do_state_change(States::st_logon_received);
    const bool reset_given(
        msg->have(Common_ResetSeqNumFlag)
            && msg->get<reset_seqnum_flag>()->get());
    sender_comp_id sci; // so this should be our tci
    msg->Header()->get(sci);
    target_comp_id tci; // so this should be our sci
    msg->Header()->get(tci);
    SessionID id(_ctx._beginStr, tci(), sci());

    if (_connection->get_role() == Connection::cn_initiator)
    {
        if (id != _sid)
        {
            glout_warn << "Inbound TargetCompID not recognised (" << tci
                << "), expecting (" << _sid.get_senderCompID() << ')';
            if (_loginParameters._enforce_compids)
            {
                stop();
                do_state_change(States::st_session_terminated);
                return false;
            }
        }

        checkSematics(seqnum, msg);
        heartbeat_interval hbi;
        msg->get(hbi);
        _connection->set_hb_interval(hbi());
        do_state_change(States::st_continuous);
        slout_info << "Client setting heartbeat interval to " << hbi();
    }
    else // acceptor
    {
        default_appl_ver_id davi;
        msg->get(davi);

        if (_sci() != tci())
        {
            glout_warn << "Inbound TargetCompID not recognised (" << tci
                << "), expecting (" << _sci << ')';
            if (_loginParameters._enforce_compids)
            {
                stop();
                do_state_change(States::st_session_terminated);
                return false;
            }
        }

        if (!_loginParameters._clients.empty())
        {
            auto itr(_loginParameters._clients.find(sci()));
            bool iserr(false);
            if (itr == _loginParameters._clients.cend())
            {
                glout_error << "Remote (" << sci << ") not found (" << id
                    << "). NOT authorised to proceed.";
                iserr = true;
            }

            if (!iserr && get<1>(itr->second) != Poco::Net::IPAddress()
                && get<1>(itr->second)
                    != _connection->get_peer_socket_address().host())
            {
                glout_error << "Remote (" << get<0>(itr->second) << ", " << sci
                    << ") NOT authorised to proceed ("
                    << _connection->get_peer_socket_address().toString()
                    << ").";
                iserr = true;
            }

            if (iserr)
            {
                stop();
                do_state_change(States::st_session_terminated);
                return false;
            }

            glout_info << "Remote (" << get<0>(itr->second) << ", " << sci
                << ") authorised to proceed ("
                << _connection->get_peer_socket_address().toString()
                << ").";
        }

        // important - these objects can't be created until we have a valid SessionID
        if (_sf)
        {
            if (!_logger
                && !(_logger = _sf->create_logger(_sf->_ses,
                    Configuration::session_log, &id)))
            {
                glout_warn << "Warning: no session logger defined for " << id;
            }

            if (!_plogger
                && !(_plogger = _sf->create_logger(_sf->_ses,
                    Configuration::protocol_log, &id)))
            {
                glout_warn << "Warning: no protocol logger defined for " << id;
            }

            if (!_persist)
            {
                f8_scoped_spin_lock guard(_per_spl,
                    _connection->get_pmodel() == pm_coro);
                if (!(_persist = _sf->create_persister(_sf->_ses, &id,
                    reset_given)))
                {
                    glout_warn << "Warning: no persister defined for " << id;
                }
            }

        }
        else
        {
            glout_error << "Error: SessionConfig object missing in session";
        }

        slout_info << "Connection from "
            << _connection->get_peer_socket_address().toString();

        if (reset_given) // ignore version restrictions on this behaviour
        {
            slout_info << "Resetting sequence numbers";
            _next_send_seq = _next_receive_seq = 1;
        }
        else
        {
            recover_seqnums();
            if (_req_next_send_seq)
                _next_send_seq = _req_next_send_seq;
            if (_req_next_receive_seq)
                _next_receive_seq = _req_next_receive_seq;
        }

        if (authenticate(id, msg))
        {
            _sid = id;
            enforce(seqnum, msg);
            send(generate_logon(_connection->get_hb_interval(), davi()));
            do_state_change(States::st_continuous);
        }
        else
        {
            slout_error << id << " failed authentication";
            stop();
            do_state_change(States::st_session_terminated);
            return false;
        }

        if (_loginParameters._login_schedule.is_valid()
            && !_loginParameters._login_schedule.test())
        {
            slout_error << id << " Session unavailable. Login not accepted.";
            stop();
            do_state_change(States::st_session_terminated);
            return false;
        }
    }

    slout_info << "Heartbeat interval is " << _connection->get_hb_interval();

    _timer.schedule(_hb_processor, 1000);	// check every second

    return true;
}

bool MYFixSession::handle_logout(const unsigned seqnum,
    const FIX8::Message *msg)
{
    TNL_LOG_INFO("handle_logout seqnum: %d", seqnum);
    Session::handle_logout(seqnum, msg);
    if (p_trd_)
    {
        p_trd_->loggoned_ = false;

        FIX8::KGI::Text logout_text;
        if (msg->get(logout_text))
        {
            TNL_LOG_INFO("kgi_fix_tunnel Log out info: %s.",
                logout_text.get().c_str());
        }

        TunnelUpdateState(GetTunnelInfo().qtm_name.c_str(), QtmState::LOG_OUT);
    }
    return true;
}

bool MYFixSession::handle_reject(const unsigned seqnum,
    const FIX8::Message *msg)
{
    //TNL_LOG_INFO("handle_reject seqnum: %d", seqnum);
    return checkSematics(seqnum, msg) || msg->process(router_);
}

bool MYFixSession::handle_heartbeat(const unsigned seqnum,
    const FIX8::Message *msg)
{
    checkSematics(seqnum, msg);

    if (_state == FIX8::States::st_test_request_sent)
        _state = FIX8::States::st_continuous;
    return true;
}

bool MYFixSession::handle_test_request(const unsigned seqnum,
    const FIX8::Message *msg)
{
    TNL_LOG_INFO("handler_test_request seqnum: %d", seqnum);
    checkSematics(seqnum, msg);

    FIX8::test_request_id testReqID;
    msg->get(testReqID);
    send(generate_heartbeat(testReqID()));
    return true;
}

bool MYFixSession::handle_application(const unsigned seqnum,
    const FIX8::Message *&msg)
{
    //TNL_LOG_INFO("handle_application seqnum: %d", seqnum);
    return checkSematics(seqnum, msg) || msg->process(router_);
}

bool MYFixSession::handle_resend_request(const unsigned seqnum,
    const FIX8::Message *msg)
{
    TNL_LOG_INFO("handle_resend_request: seqnum: %d", seqnum);

    FIX8::poss_dup_flag l_poss_dup;
    if (msg->get(l_poss_dup))
    {
        if (l_poss_dup())
        {
            return false;
        }
    }

    if (_state != FIX8::States::st_resend_request_received)
    {

        FIX8::begin_seq_num begin;
        FIX8::end_seq_num end;

        if (!_persist || !msg->get(begin) || !msg->get(end))
        {
            //cout << "generate_sequence_reset " << _next_send_seq << endl;
            send(generate_sequence_reset(_next_send_seq + 1, true));
        }
        else if ((begin() > end() && end()) || begin() == 0)
        {
            send(generate_reject(seqnum, "Invalid begin or end resend seqnum"));
        }
        else
        {
            TNL_LOG_INFO("Resend request from %d to %d.", begin.get(),
                end.get());

            _state = FIX8::States::st_resend_request_received;

            FIX8::begin_seq_num l_beginSeqNum;
            unsigned l_beginSeqNumInt = _next_send_seq;
            if (msg->get(l_beginSeqNum))
            {
                l_beginSeqNumInt = l_beginSeqNum();
            }

            send(generate_sequence_reset(_next_send_seq, true), true,
                l_beginSeqNumInt);
        }

    }
    return true;
}

bool MYFixSession::handle_sequence_reset(const unsigned seqnum,
    const FIX8::Message *msg)
{
    TNL_LOG_INFO("handle_sequence_reset seqnum: %d", seqnum);
    has_resend_ = false;
    return Session::handle_sequence_reset(seqnum, msg);
}

bool MYFixSession::exchange_sequence_check(const unsigned seqnum,
    const FIX8::Message *msg)
{
    //std::cout << "seqnum:" << seqnum << " next_target_seq:" << _next_receive_seq
    //<< " next_sender_seq:" << _next_send_seq << " state:" << _state
    //<< " next_receive_seq:" << _next_receive_seq << endl;

    if (seqnum > _next_receive_seq)
    {
        if (!has_resend_)
        {
            _state = States::st_resend_request_sent;
            send(generate_resend_request(_next_receive_seq, seqnum));
        }
        return false;
    }

    if (seqnum < _next_receive_seq)
    {
        FIX8::poss_dup_flag pdf(false);
        msg->Header()->get(pdf);
        if (!pdf())	// poss dup not set so bad
            throw FIX8::MsgSequenceTooLow(seqnum, _next_receive_seq);
        FIX8::sending_time st;
        msg->Header()->get(st);
        FIX8::orig_sending_time ost;
        if (msg->Header()->get(ost) && ost() > st())
        {
            ostringstream ostr;
            ost.print(ostr);
            throw FIX8::BadSendingTime(ostr.str());
        }
    }

    return true;
}

bool MYFixSession::checkSematics(const unsigned seqnum,
    const FIX8::Message *msg)
{
    if (FIX8::States::is_established(_state))
    {
        if (_state != FIX8::States::st_logon_received)
        {
            compid_check(seqnum, msg, _sid);
        }
        if (msg->get_msgtype() != FIX8::Common_MsgType_SEQUENCE_RESET
            && exchange_sequence_check(seqnum, msg))
        {
            return false;
        }
    }
    return true;
}

FIX8::Message *MYFixSession::generate_resend_request(const unsigned begin,
    const unsigned end)
{
    FIX8::Message *msg(create_msg(FIX8::Common_MsgType_RESEND_REQUEST));
    *msg << new FIX8::begin_seq_num(begin) << new FIX8::end_seq_num(end);
    if (has_resend_)
    {
        *msg->Header() << new FIX8::poss_dup_flag(true);
    }
    else
    {
        has_resend_ = true;
    }
    return msg;
}

ClOrdidToSnMap::iterator MYFixSession::GetClOrdIdPos(std::string key)
{
    return p_trd_->serialno_book_.find(key);
}

bool MYFixSession::ItIsEnd(ClOrdidToSnMap::iterator it)
{
    return it == p_trd_->serialno_book_.end();
}

const Tunnel_Info MYFixSession::GetTunnelInfo()
{
    return p_trd_->tunnel_info_;
}

void MYFixSession::SentToOrderRespondCallBackHandler(T_OrderRespond *rsp)
{
    if (p_trd_->OrderRespond_call_back_handler_)
    {
        p_trd_->OrderRespond_call_back_handler_(rsp);
    }
}

void MYFixSession::SentToCancelRespondCallBackHandler(T_CancelRespond *rsp)
{
    if (p_trd_->CancelRespond_call_back_handler_)
    {
        p_trd_->CancelRespond_call_back_handler_(rsp);
    }
}

void MYFixSession::SentToOrderReturnCallBackHandler(T_OrderReturn *rsp)
{
    if (p_trd_->OrderReturn_call_back_handler_)
    {
        p_trd_->OrderReturn_call_back_handler_(rsp);
    }
}

void MYFixSession::SentToTradeReturnCallBackHandler(T_TradeReturn *rsp)
{
    if (p_trd_->TradeReturn_call_back_handler_)
    {
        p_trd_->TradeReturn_call_back_handler_(rsp);
    }
}

long MYFixSession::GetEntrustNo(const std::string &order_id)
{
    size_t ac_len = p_trd_->account_.length();
    size_t ord_len = order_id.length();
    std::string tmp = order_id.substr(ac_len + 2, ord_len - 1);
    return atol(tmp.c_str());
}

void MYFixSession::UpdatePositionData(T_TradeReturn &trade)
{
    p_trd_->position_->add_new_trade(trade);
}
