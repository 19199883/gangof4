#include "xspeed_trade_interface.h"

#include <unistd.h>
#include <stdlib.h>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <unordered_map>

#include "my_protocol_packager.h"
#include "check_schedule.h"
#include "my_cmn_util_funcs.h"
#include "xspeed_data_formater.h"

#include "qtm_with_code.h"

using namespace std;
using namespace DFITCXSPEEDAPI;
using namespace my_cmn;

struct PositionSaveInFile {
	int direction;
	int volume;
};

void MYXSpeedSpi::ReportErrorState(int api_error_no, const std::string &error_msg)
{
    if (api_error_no == 0)
    {
        return;
    }
    if (!cfg_.IsKnownErrorNo(api_error_no))
    {
        char err_msg[127];
        sprintf(err_msg, "api error no: %d, error msg: %s", api_error_no, error_msg.c_str());
        update_state(tunnel_info_.qtm_name.c_str(), TYPE_TCA, QtmState::UNDEFINED_API_ERROR, err_msg);
        TNL_LOG_INFO("update_state: name: %s, State: %d, Description: %s.", tunnel_info_.qtm_name.c_str(), QtmState::UNDEFINED_API_ERROR, err_msg);
    }
}

void MYXSpeedSpi::SaveToFile()
{
	std::string file = "pos_" + tunnel_info_.account + "_" + my_cmn::GetCurrentDateString() + ".txt";
	ofstream f_out(file);
	if (!f_out) {
		TNL_LOG_ERROR("Open %s failed.", file.c_str());
		return;
	}
	std::unordered_map<std::string, PositionSaveInFile> pos_map;
	auto mit = pos_map.begin();
	for (auto it = pos_info_.begin(); it != pos_info_.end(); it++) {
		mit = pos_map.find(it->instrumentID);
		if (mit != pos_map.end()) {
			mit->second.volume += it->volume;
		} else {
			PositionSaveInFile tmp;
			tmp.direction = it->buySellType;
			tmp.volume = it->volume;
			pos_map.insert(make_pair(it->instrumentID, tmp));
		}
	}
	for (mit = pos_map.begin(); mit != pos_map.end(); mit++) {
		f_out << mit->first << ',' << mit->second.direction - 1 << ',' << mit->second.volume << std::endl;
	}
	f_out.close();
}

void MYXSpeedSpi::StartPositionBackUp()
{
	DFITCPositionDetailField qry_param;
	memset(&qry_param, 0, sizeof(DFITCPositionField));
	strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
	qry_param.instrumentType = DFITC_COMM_TYPE;
    backup_ = true;
	api_->ReqQryPositionDetail(&qry_param);
}

MYXSpeedSpi::MYXSpeedSpi(const TunnelConfigData &cfg)
    : cfg_(cfg), backup_(false)
{
    connected_ = false;
    logoned_ = false;
    in_init_state_ = true;
    is_ready_ = false;

    pos_qry_step = 0;
    order_qry_step = 0;
    trade_qry_step = 0;

    md_spi = NULL;
    md_api = NULL;
    md_in_init_stat = true;

    // whether it is need to support cancel all order when init
    have_handled_unterminated_orders_ = true;
    if (cfg_.Initial_policy_param().cancel_orders_at_start)
    {
        have_handled_unterminated_orders_ = false;
    }

    finish_query_canceltimes_ = false;
    if (cfg_.Compliance_check_param().init_cancel_times_at_start == 0)
    {
        finish_query_canceltimes_ = true;
    }

    session_id_ = 0;
    max_order_ref_ = 0;

    xspeed_trade_context_.InitOrderRef(cfg_.App_cfg().orderref_prefix_id);

    // 从配置解析参数
    if (!ParseConfig())
    {
        return;
    }

    char qtm_tmp_name[32];
    memset(qtm_tmp_name, 0, sizeof(qtm_tmp_name));
    sprintf(qtm_tmp_name, "xspeed_mm_%s_%u", tunnel_info_.account.c_str(), getpid());
    tunnel_info_.qtm_name = qtm_tmp_name;

    std::thread init_md_thread(&MYXSpeedSpi::InitMDInterface, this);
    init_md_thread.detach();

    // create
    api_ = DFITCTraderApi::CreateDFITCTraderApi();
    if (!api_)
    {
        TNL_LOG_ERROR("CreateDFITCTraderApi failed, return null");
        return;
    }

    // set front address
    for (const std::string &v : cfg.Logon_config().front_addrs)
    {
        char *addr_tmp = new char[v.size() + 1];
        memcpy(addr_tmp, v.c_str(), v.size() + 1);
        api_->Init(addr_tmp, this);
        TNL_LOG_WARN("RegisterFront, addr: %s", addr_tmp);
        delete[] addr_tmp;

        break;
    }
}

MYXSpeedSpi::~MYXSpeedSpi(void)
{
    if (api_)
    {
        api_->Release();
        api_ = NULL;
    }
}

bool MYXSpeedSpi::ParseConfig()
{
    // 用户密码
    tunnel_info_.account = cfg_.Logon_config().investorid;
    pswd_ = cfg_.Logon_config().password;
    return true;
}

void MYXSpeedSpi::ReqLogin()
{
    sleep(3);

    DFITCUserLoginField login_data;
    memset(&login_data, 0, sizeof(DFITCUserLoginField));
    strncpy(login_data.accountID, tunnel_info_.account.c_str(), sizeof(login_data.accountID));
    strncpy(login_data.passwd, pswd_.c_str(), sizeof(login_data.passwd));

    api_->ReqUserLogin(&login_data);
    TNL_LOG_INFO("ReqLogin:  \n%s", XSpeedDatatypeFormater::ToString(&login_data).c_str());
}

void MYXSpeedSpi::OnFrontConnected()
{
    TNL_LOG_INFO("OnFrontConnected.");
    connected_ = true;

    DFITCUserLoginField login_data;
    memset(&login_data, 0, sizeof(DFITCUserLoginField));
    strncpy(login_data.accountID, tunnel_info_.account.c_str(), sizeof(login_data.accountID));
    strncpy(login_data.passwd, pswd_.c_str(), sizeof(login_data.passwd));

    // 成功登录后，断开不再重新登录
    if (in_init_state_)
    {
        api_->ReqUserLogin(&login_data);
    }
}

void MYXSpeedSpi::OnFrontDisconnected(int nReason)
{
    connected_ = false;
    logoned_ = false;
    is_ready_ = false;

    TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::DISCONNECT);

    TNL_LOG_WARN("OnFrontDisconnected, nReason=%d", nReason);
}

void MYXSpeedSpi::OnRspUserLogin(struct DFITCUserLoginInfoRtnField* pf, struct DFITCErrorRtnField* pe)
{
    TNL_LOG_INFO("OnRspUserLogin:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    if (pe == NULL && pf->loginResult == 0)
    {
        session_id_ = pf->sessionID;
        max_order_ref_ = pf->initLocalOrderID;
        xspeed_trade_context_.SetOrderRef(max_order_ref_);

        logoned_ = true;
        in_init_state_ = false;

        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_SUCCESS);

        // trade day query
        DFITCTradingDayField qry_trade_day;
        memset(&qry_trade_day, 0, sizeof(qry_trade_day));
        int ret = api_->ReqTradingDay(&qry_trade_day);
        TNL_LOG_INFO("ReqTradingDay, return:%d", ret);

        // interface moved from trade api to md api in version XSpeedAPI_V1.0.9.14_sp4
//        // subscribe all quote notice
//        DFITCQuoteSubscribeField sub_quote_param;
//        memset(&sub_quote_param, 0, sizeof(sub_quote_param));
//        strncpy(sub_quote_param.accountID, user_.c_str(), sizeof(DFITCAccountIDType));
//        strncpy(sub_quote_param.exchangeID, MY_TNL_EXID_DCE, sizeof(DFITCExchangeIDType));
//
//        ret = api_->ReqQuoteSubscribe(&sub_quote_param);
//        TNL_LOG_INFO("ReqQuoteSubscribe, return:%d", ret);

        if (!HaveFinishQueryOrders())
        {
            std::thread qry_thread(std::bind(&MYXSpeedSpi::QueryOrdersForInitial, this));
            qry_thread.detach();
        }
        else
        {
            is_ready_ = true;
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
        }
    }
    else
    {
        if (pe)
        {
            int standard_error_no = cfg_.GetStandardErrorNo(pe->nErrorID);
            TNL_LOG_WARN("OnRspUserLogin, external errorid: %d; Internal errorid: %d", pe->nErrorID, standard_error_no);
        }

        // 重新登陆
        std::thread req_login(&MYXSpeedSpi::ReqLogin, this);
        req_login.detach();
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_ON_FAIL);
    }
}

void MYXSpeedSpi::OnRspUserLogout(struct DFITCUserLogoutInfoRtnField* pf, struct DFITCErrorRtnField* pe)
{
    TNL_LOG_INFO("OnRspUserLogout:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    try
    {
        logoned_ = false;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::LOG_OUT);
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspUserLogout.");
    }
}

void MYXSpeedSpi::OnRspInsertOrder(struct DFITCOrderRspDataRtnField* pf, struct DFITCErrorRtnField* pe)
{
    TNL_LOG_DEBUG("OnRspInsertOrder:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRspInsertOrder before tunnel is ready.");
        return;
    }

    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        OrderRefDataType OrderRef = 0;
        const XSpeedOrderInfo *p = NULL;
        char entrust_status = MY_TNL_OS_ERROR;

        if (!pf)
        {
            TNL_LOG_WARN("OnRspOrderInsert, pInputOrder is null.");
            return;
        }

        if (pe)
        {
            if (pe->nErrorID != 0) {
                ReportErrorState(pe->nErrorID, pe->errorMsg);
            }
            api_err_no = pe->nErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
            OrderRef = pe->localOrderID;

            TNL_LOG_WARN("OnRspOrderInsert, api_err_no: %d; my_err: %d; OrderRef: %lld", api_err_no, standard_error_no, OrderRef);
        }
        else
        {
            OrderRef = pf->localOrderID;
            entrust_status = XSpeedFieldConvert::GetMYOrderStatus(pf->orderStatus);
        }

        p = xspeed_trade_context_.GetOrderInfoByLocalID(OrderRef);
        if (p)
        {
            T_OrderRespond rsp;
            XSpeedPacker::OrderRespond(standard_error_no, p->serial_no, 0, entrust_status, rsp);

            if (standard_error_no != TUNNEL_ERR_CODE::RESULT_SUCCESS)
            {
                // 报单失败，报告合规检查
                ComplianceCheck_OnOrderInsertFailed(
                    tunnel_info_.account.c_str(),
                    OrderRef,
                    p->exchange_code,
                    p->stock_code,
                    p->volume,
                    p->speculator,
                    p->open_close,
                    p->order_type);
            }

            // 应答
            if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&rsp);
            LogUtil::OnOrderRespond(&rsp, tunnel_info_);
        }
        else
        {
            TNL_LOG_INFO("can't get original place order info of order ref: %lld", OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspOrderInsert.");
    }
}

void MYXSpeedSpi::OnRspCancelOrder(struct DFITCOrderRspDataRtnField* pf, struct DFITCErrorRtnField* pe)
{
    TNL_LOG_DEBUG("OnRspCancelOrder:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRspCancelOrder before tunnel is ready.");
        return;
    }

    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        OrderRefDataType OrderRef = 0;
        const XSpeedOrderInfo *p = NULL;

        if (!pf)
        {
            TNL_LOG_WARN("OnRspCancelOrder, pInputOrder is null.");
            return;
        }

        if (pe)
        {
            if (pe->nErrorID != 0) {
                ReportErrorState(pe->nErrorID, pe->errorMsg);
            }
            api_err_no = pe->nErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
            OrderRef = pe->localOrderID;

            TNL_LOG_WARN("OnRspCancelOrder, api_err_no: %d; my_err: %d; OrderRef: %lld", api_err_no, standard_error_no, OrderRef);
        }
        else
        {
            OrderRef = pf->localOrderID;
        }

        p = xspeed_trade_context_.GetOrderInfoByLocalID(OrderRef);
        if (p)
        {
            long cancel_serial_no = p->Pop_cancel_serial_no();
            if (cancel_serial_no != 0)
            {
                T_CancelRespond rsp;
                XSpeedPacker::CancelRespond(standard_error_no, cancel_serial_no, p->entrust_no, rsp);

                if (standard_error_no != TUNNEL_ERR_CODE::RESULT_SUCCESS)
                {
                    // 撤单拒绝，报告合规检查
                    ComplianceCheck_OnOrderCancelFailed(
                        tunnel_info_.account.c_str(),
                        p->stock_code,
                        OrderRef);
                }

                // 应答
                if (CancelRespond_call_back_handler_) CancelRespond_call_back_handler_(&rsp);
                LogUtil::OnCancelRespond(&rsp, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("cancel order: %ld from outside.", p->serial_no);
            }
        }
        else
        {
            TNL_LOG_INFO("can't get original place order info of order ref: %lld", OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspCancelOrder.");
    }
}

// obsolete: can't get cost of yesterday position, so use interface ReqQryPositionDetail
void MYXSpeedSpi::OnRspQryPosition(struct DFITCPositionInfoRtnField* pf, struct DFITCErrorRtnField* pe, bool bIsLast)
{
    TNL_LOG_DEBUG("OnRspQryPosition: bIsLast=%d \n%s \n%s", bIsLast,
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());
}

struct PositionDetailPred
{
    PositionDetailPred(const PositionDetail &v)
        : v_(v)
    {
    }
    bool operator()(const PositionDetail &l)
    {
        return v_.direction == l.direction && strcmp(v_.stock_code, l.stock_code) == 0;
    }

private:
    const PositionDetail v_;
};

void MYXSpeedSpi::QueryOptionPosition()
{
    usleep(1000 * 1000); // Query frequency can't be too fast

    DFITCPositionDetailField qry_param;
    memset(&qry_param, 0, sizeof(DFITCPositionField));
    strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
    qry_param.instrumentType = DFITC_OPT_TYPE;

    pos_qry_step = 2;
    int api_res = api_->ReqQryPositionDetail(&qry_param);
    TNL_LOG_DEBUG("ReqQryPosition - ret=%d - %s", api_res, XSpeedDatatypeFormater::ToString(&qry_param).c_str());
    if (api_res != 0)
    {
        pos_qry_step = 0;

        T_PositionReturn ret;
        ret.error_no = api_res;
        if (QryPosReturnHandler_) QryPosReturnHandler_(&ret);
        LogUtil::OnPositionRtn(&ret, tunnel_info_);
    }
}

void MYXSpeedSpi::OnRspQryPositionDetail(struct DFITCPositionDetailRtnField * pf, struct DFITCErrorRtnField * pe, bool bIsLast)
{
    TNL_LOG_DEBUG("OnRspQryPositionDetail: bIsLast=%d \n%s \n%s", bIsLast,
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

	if (backup_) {
		if (pf) {
			pos_info_.push_back(*pf);
		}
		if (bIsLast) {
			if (pf->instrumentType == DFITC_COMM_TYPE) {
				usleep(1000 * 1000); // Query frequency can't be too fast

				DFITCPositionDetailField qry_param;
				memset(&qry_param, 0, sizeof(DFITCPositionField));
				strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
				qry_param.instrumentType = DFITC_OPT_TYPE;

				api_->ReqQryPositionDetail(&qry_param);
			} else {
				SaveToFile();
				backup_ = false;
			}
		}
	} else {
		T_PositionReturn ret;

		// respond error
		if (pe && pe->nErrorID != 0) {
			ret.error_no = -1;
			if (QryPosReturnHandler_)
				QryPosReturnHandler_(&ret);
			LogUtil::OnPositionRtn(&ret, tunnel_info_);
			pos_qry_step = 0;
			return;
		}

		if (pf && pf->volume > 0) {
			PositionDetail pos;
			memset(&pos, 0, sizeof(pos));
			strncpy(pos.stock_code, pf->instrumentID, sizeof(pf->instrumentID));

			if (pf->buySellType == DFITC_SPD_BUY) {
				pos.direction = MY_TNL_D_BUY;
			} else {
				pos.direction = MY_TNL_D_SELL;
			}

			// 今仓，当前的仓位
			pos.position = pf->volume;
			pos.position_avg_price = pf->volume * pf->openPrice; // now is total cost
			if (memcmp(trade_day, pf->matchedDate, sizeof(trade_day)) != 0) {
				pos.yestoday_position = pf->volume;
				pos.yd_position_avg_price = pf->volume * pf->openPrice; // now is total cost
			}
			std::vector<PositionDetail>::iterator it = std::find_if(
			    position_detail_.begin(), position_detail_.end(),
					PositionDetailPred(pos));
			if (it == position_detail_.end()) {
			    position_detail_.push_back(pos);
			} else {
				it->position += pos.position;
				it->position_avg_price += pos.position_avg_price; // now is total cost
				it->yestoday_position += pos.yestoday_position;
				it->yd_position_avg_price += pos.yd_position_avg_price; // now is total cost
			}
		}

		bool qry_finish = false;
		if (bIsLast) {
			if (pos_qry_step == 1) {
				// query option
				std::thread qry_position(&MYXSpeedSpi::QueryOptionPosition,
						this);
				qry_position.detach();
			} else if (pos_qry_step == 2) {
				qry_finish = true;
				pos_qry_step = 0;
			} else {
				TNL_LOG_WARN(
						"OnRspQryPositionDetail when pos_qry_step(0-init;1-fut;2-opt):%d",
						pos_qry_step.load());
				pos_qry_step = 0;
			}
		}

		if (qry_finish) {
			CheckAndSaveYestodayPosition();
			TNL_LOG_INFO("receive all position qry result, send to client");
			ret.datas.swap(position_detail_);
			ret.error_no = 0;
			if (QryPosReturnHandler_)
				QryPosReturnHandler_(&ret);
			LogUtil::OnPositionRtn(&ret, tunnel_info_);
		}
	}
}

void MYXSpeedSpi::OnRspCustomerCapital(struct DFITCCapitalInfoRtnField* pf, struct DFITCErrorRtnField* pe, bool bIsLast)
{
    // no request, shouldn't return
    TNL_LOG_INFO("OnRspCustomerCapital: bIsLast=%d \n%s \n%s", bIsLast,
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());
}

void MYXSpeedSpi::OnRspQryExchangeInstrument(struct DFITCExchangeInstrumentRtnField* pf, struct DFITCErrorRtnField* pe,
bool bIsLast)
{
    // no request, shouldn't return
    TNL_LOG_INFO("OnRspQryExchangeInstrument: bIsLast=%d \n%s \n%s", bIsLast,
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());
    ContractInfo item;
    memset(&item, 0, sizeof(item));
    strcpy(item.TradeDate, my_cmn::GetCurrentDateString().c_str());
    TNL_LOG_DEBUG("instrumentID: %s, instrumentExpiration: %s", pf->instrumentID, pf->instrumentExpiration);
    strcpy(item.stock_code, pf->instrumentID);
    strcpy(item.ExpireDate, pf->instrumentExpiration);
    contract_info_.push_back(item);

    if (bIsLast == true) {
    	T_ContractInfoReturn rtn;
    	memset(&rtn, 0, sizeof(rtn));
    	rtn.error_no = 0;
    	rtn.datas.insert(rtn.datas.end(), contract_info_.begin(), contract_info_.end());
    	if (QryContractInfoHandler_) QryContractInfoHandler_(&rtn);
    	LogUtil::OnContractInfoRtn(&rtn, tunnel_info_);
    }
}

void MYXSpeedSpi::OnRspBillConfirm(struct DFITCBillConfirmRspField* pf, struct DFITCErrorRtnField* pe)
{
    // no request, shouldn't return
    TNL_LOG_INFO("OnRspBillConfirm:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());
}

void MYXSpeedSpi::OnRtnErrorMsg(struct DFITCErrorRtnField* pe)
{
    TNL_LOG_DEBUG("OnRtnErrorMsg:  \n%s", XSpeedDatatypeFormater::ToString(pe).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRtnErrorMsg before tunnel is ready.");
        return;
    }

    try
    {
        if (pe && pe->sessionID == session_id_)
        {
            int api_err_no = pe->nErrorID;
            int standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
            OrderRefDataType OrderRef = pe->localOrderID;
            const XSpeedOrderInfo *p = xspeed_trade_context_.GetOrderInfoByLocalID(OrderRef);
            if (p)
            {
                long cancel_serial_no = p->Pop_cancel_serial_no();
                if (cancel_serial_no != 0)
                {
                    T_CancelRespond rsp;
                    XSpeedPacker::CancelRespond(standard_error_no, cancel_serial_no, p->entrust_no, rsp);

                    if (standard_error_no != TUNNEL_ERR_CODE::RESULT_SUCCESS)
                    {
                        // 撤单拒绝，报告合规检查
                        ComplianceCheck_OnOrderCancelFailed(
                            tunnel_info_.account.c_str(),
                            p->stock_code,
                            OrderRef);
                    }

                    // 应答
                    if (CancelRespond_call_back_handler_) CancelRespond_call_back_handler_(&rsp);
                    LogUtil::OnCancelRespond(&rsp, tunnel_info_);
                }
                // xspeed 如果单据的状态为全成/失败/已撤/已经报入，那么不向上层相应，需要检测撤单数量
                // 例如 : A报单，A还未成交，B撤A，当B到达交易所前，A已经全成，此时B会返回错误，
                // 如果向交易程序应答，交易程序会coredump，这里做过滤
                else if ((MY_TNL_OS_WITHDRAWED == p->entrust_status)
                    || (MY_TNL_OS_COMPLETED == p->entrust_status)
                    || (MY_TNL_OS_ERROR == p->entrust_status)
                    || (MY_TNL_OS_REPORDED == p->entrust_status))
                {
                    // 撤单拒绝，报告合规检查
                    ComplianceCheck_OnOrderCancelFailed(
                        tunnel_info_.account.c_str(),
                        p->stock_code,
                        OrderRef);
                    TNL_LOG_WARN(" OnRtnErrorMsg : status error. order ref: %lld, entrust status:%c.",
                        OrderRef, p->entrust_status);
                }
                else
                {
                    T_OrderRespond rsp;
                    XSpeedPacker::OrderRespond(standard_error_no, p->serial_no, 0, MY_TNL_OS_ERROR, rsp);

                    if (standard_error_no != TUNNEL_ERR_CODE::RESULT_SUCCESS)
                    {
                        // 报单失败，报告合规检查
                        ComplianceCheck_OnOrderInsertFailed(
                            tunnel_info_.account.c_str(),
                            OrderRef,
                            p->exchange_code,
                            p->stock_code,
                            p->volume,
                            p->speculator,
                            p->open_close,
                            p->order_type);
                    }

                    // 应答
                    if (OrderRespond_call_back_handler_) OrderRespond_call_back_handler_(&rsp);
                    LogUtil::OnOrderRespond(&rsp, tunnel_info_);
                }
            }
            else
            {
                TNL_LOG_INFO("can't get original place order info of order ref: %lld", OrderRef);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnErrorMsg.");
    }
}

void MYXSpeedSpi::OnRtnMatchedInfo(struct DFITCMatchRtnField* pf)
{
    TNL_LOG_DEBUG("OnRtnMatchedInfo:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRtnMatchedInfo before tunnel is ready.");
        return;
    }

    try
    {
        if (pf && pf->sessionID == session_id_)
        {
            OrderRefDataType OrderRef = pf->localOrderID;
            const XSpeedOrderInfo *p = xspeed_trade_context_.GetOrderInfoByLocalID(OrderRef);
            if (p)
            {
                p->entrust_no = atoll(pf->OrderSysID);

                p->entrust_status = MY_TNL_OS_PARTIALCOM;
                p->volume_remain -= pf->matchedAmount;

                if (0 == p->volume_remain)
                {
                    p->entrust_status = MY_TNL_OS_COMPLETED;
                    //全成，报告合规检查
                    ComplianceCheck_OnOrderFilled(
                        tunnel_info_.account.c_str(),
                        OrderRef);
                }

                // 委托回报
                T_OrderReturn order_return;
                XSpeedPacker::OrderReturn(pf, p, order_return);
                if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
                LogUtil::OnOrderReturn(&order_return, tunnel_info_);

                // match回报
                T_TradeReturn trade_return;
                XSpeedPacker::TradeReturn(pf, p, trade_return);
                if (TradeReturn_call_back_handler_) TradeReturn_call_back_handler_(&trade_return);
                LogUtil::OnTradeReturn(&trade_return, tunnel_info_);
            }
            else
            {
                TNL_LOG_INFO("can't get original place order info of order ref: %lld", OrderRef);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnMatchedInfo.");
    }
}

void MYXSpeedSpi::OnRtnOrder(struct DFITCOrderRtnField* pf)
{
    TNL_LOG_DEBUG("OnRtnOrder:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRtnOrder before tunnel is ready.");
        return;
    }

    try
    {
        if (pf && pf->sessionID == session_id_)
        {
            OrderRefDataType OrderRef = pf->localOrderID;
            const XSpeedOrderInfo *p = xspeed_trade_context_.GetOrderInfoByLocalID(OrderRef);
            if (p)
            {
                // xspeed版本存在消息乱序情况，这里进行过滤
                // 例如 : 一个报单先收到全成状态，然后收到已经进入队列状态
                int entrust_status = p->entrust_status;
                if ((MY_TNL_OS_WITHDRAWED == entrust_status)
                    || (MY_TNL_OS_COMPLETED == entrust_status)
                    || (MY_TNL_OS_ERROR == entrust_status))
                {
                    TNL_LOG_WARN("entrust status error. order ref: %lld, entrust status:%d, order status:%d ",
                        OrderRef, entrust_status, pf->orderStatus);
                    return;
                }

                p->entrust_no = atoll(pf->OrderSysID);
                p->entrust_status = XSpeedFieldConvert::GetMYOrderStatus(pf->orderStatus);

                //返回废单是进行回滚
                if (p->entrust_status == MY_TNL_OS_ERROR)
                {
                    ComplianceCheck_OnOrderInsertFailed(
                        tunnel_info_.account.c_str(),
                        OrderRef,
                        p->exchange_code,
                        p->stock_code,
                        p->volume,
                        p->speculator,
                        p->open_close,
                        p->order_type);
                }

                // 已撤，报告合规检查
                if (p->entrust_status == MY_TNL_OS_WITHDRAWED)
                {
                    ComplianceCheck_OnOrderCanceled(
                        tunnel_info_.account.c_str(),
                        pf->instrumentID,
                        OrderRef,
                        p->exchange_code,
                        pf->cancelAmount,
                        p->speculator,
                        p->open_close);
                }

                // 全成，报告合规检查
                if (p->entrust_status == MY_TNL_OS_COMPLETED)
                {
                    ComplianceCheck_OnOrderFilled(
                        tunnel_info_.account.c_str(),
                        OrderRef);
                }

                // 委托回报
                T_OrderReturn order_return;
                XSpeedPacker::OrderReturn(pf, p, order_return);
                if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
                LogUtil::OnOrderReturn(&order_return, tunnel_info_);
            }
            else
            {
                TNL_LOG_INFO("can't get original place order info of order ref: %lld", OrderRef);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnOrder.");
    }
}

void MYXSpeedSpi::OnRtnCancelOrder(struct DFITCOrderCanceledRtnField* pf)
{
    TNL_LOG_DEBUG("OnRtnCancelOrder:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRtnCancelOrder before tunnel is ready.");
        return;
    }

    try
    {
        if (pf && pf->sessionID == session_id_)
        {
            OrderRefDataType OrderRef = pf->localOrderID;
            const XSpeedOrderInfo *p = xspeed_trade_context_.GetOrderInfoByLocalID(OrderRef);
            if (p)
            {
                p->entrust_status = XSpeedFieldConvert::GetMYOrderStatus(pf->orderStatus);

                // 已撤，报告合规检查
                if (p->entrust_status == MY_TNL_OS_WITHDRAWED)
                {
                    ComplianceCheck_OnOrderCanceled(
                        tunnel_info_.account.c_str(),
                        p->stock_code,
                        OrderRef,
                        p->exchange_code,
                        pf->cancelAmount,
                        p->speculator,
                        p->open_close);
                }

                // 委托回报
                T_OrderReturn order_return;
                XSpeedPacker::OrderReturn(pf, p, order_return);
                if (OrderReturn_call_back_handler_) OrderReturn_call_back_handler_(&order_return);
                LogUtil::OnOrderReturn(&order_return, tunnel_info_);
            }
            else
            {
                TNL_LOG_INFO("can't get original place order info of order ref: %lld", OrderRef);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnCancelOrder.");
    }
}

void MYXSpeedSpi::QueryOptionOrder()
{
    sleep(1); // Query frequency can't be too fast

    DFITCOrderField qry_param;
    memset(&qry_param, 0, sizeof(DFITCOrderField));
    strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
    qry_param.instrumentType = DFITC_OPT_TYPE;

    order_qry_step = 2;
    int api_res = api_->ReqQryOrderInfo(&qry_param);
    TNL_LOG_DEBUG("ReqQryOrderInfo - ret=%d - %s", api_res, XSpeedDatatypeFormater::ToString(&qry_param).c_str());
    if (api_res != 0)
    {
        T_OrderDetailReturn ret;
        order_qry_step = 0;
        ret.error_no = api_res;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}

void MYXSpeedSpi::OnRspQryOrderInfo(struct DFITCOrderCommRtnField* pf, struct DFITCErrorRtnField* pe, bool bIsLast)
{
    TNL_LOG_DEBUG("OnRspQryOrderInfo: bIsLast=%d \n%s \n%s", bIsLast,
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    if (!HaveFinishQueryOrders())
    {
        return HandleQueryOrders(pf, pe, bIsLast);
    }

    T_OrderDetailReturn ret;
    // respond error
    if (pe && pe->nErrorID != 0)
    {
        ReportErrorState(pe->nErrorID, pe->errorMsg);
        ret.error_no = -1;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
        order_qry_step = 0;
        return;
    }

    if (pf && pf->orderAmount > 0)
    {
        OrderDetail od;
        strncpy(od.stock_code, pf->instrumentID, sizeof(od.stock_code));
        od.entrust_no = atoll(pf->OrderSysID);
        od.order_kind = MY_TNL_OPT_LIMIT_PRICE;
        od.direction = XSpeedFieldConvert::GetMYBuySell(pf->buySellType);
        od.open_close = XSpeedFieldConvert::GetMYOpenClose(pf->openClose);
        od.speculator = XSpeedFieldConvert::GetMYHedgeType(pf->speculator);
        od.entrust_status = XSpeedFieldConvert::GetMYOrderStatus(pf->orderStatus);
        od.limit_price = pf->insertPrice;
        od.volume = pf->orderAmount;
        od.volume_traded = pf->matchedAmount;
        od.volume_remain = pf->orderAmount - pf->matchedAmount;
        order_detail_.push_back(od);
    }

    bool qry_finish = false;
    if (bIsLast)
    {
        if (order_qry_step == 1)
        {
            // query option
            std::thread qry_order(&MYXSpeedSpi::QueryOptionOrder, this);
            qry_order.detach();
        }
        else if (order_qry_step == 2)
        {
            qry_finish = true;
            order_qry_step = 0;
        }
        else
        {
            TNL_LOG_WARN("OnRspQryOrderInfo when order_qry_step(0-init;1-fut;2-opt):%d", order_qry_step.load());
            order_qry_step = 0;
        }
    }

    if (qry_finish)
    {
        TNL_LOG_INFO("receive all order qry result, send to client");
        ret.datas.swap(order_detail_);
        ret.error_no = 0;
        if (QryOrderDetailReturnHandler_) QryOrderDetailReturnHandler_(&ret);
        LogUtil::OnOrderDetailRtn(&ret, tunnel_info_);
    }
}

void MYXSpeedSpi::ProcessAfterGetAllQuotes()
{
    // cancel all quotes
    if (!have_handled_unterminated_orders_)
    {
        for (const DFITCQuoteOrderRtnField &quote_field : unterminated_quotes_)
        {
            DFITCCancelOrderField cancle_quote;
            memset(&cancle_quote, 0, sizeof(cancle_quote));

            // 报单之后与前置连接断开或者 API 客户端软件关闭重启，只能通过柜台委托号进行撤单
            strncpy(cancle_quote.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
            strncpy(cancle_quote.instrumentID, quote_field.instrumentID, sizeof(DFITCInstrumentIDType));
            //cancle_quote.localOrderID = quote_field.localOrderID;
            cancle_quote.spdOrderID = quote_field.spdOrderID;

            int cancel_quote_ret = api_->ReqQuoteCancel(&cancle_quote);
            TNL_LOG_INFO("ReqQuoteCancel for cancel unterminated orders, return %d", cancel_quote_ret);

            usleep(20 * 1000);
        }
    }
    unterminated_quotes_.clear();

    // query order detail parameter
    DFITCOrderField qry_param;
    memset(&qry_param, 0, sizeof(DFITCOrderField));
    strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
    qry_param.instrumentType = DFITC_COMM_TYPE;

    //主动查询所有报单
    while (true)
    {
        order_qry_step = 1;
        int qry_result = api_->ReqQryOrderInfo(&qry_param);
        TNL_LOG_INFO("ReqQryOrder for cancel unterminated orders, return %d", qry_result);
        if (qry_result != 0)
        {
            order_qry_step = 0;

            // retry if failed, wait some seconds
            sleep(2);
            continue;
        }

        break;
    }
}

void MYXSpeedSpi::OnRspQryQuoteOrderInfo(struct DFITCQuoteOrderRtnField* pf, struct DFITCErrorRtnField* pe, bool bIsLast)
{
    TNL_LOG_DEBUG("OnRspQryQuoteOrderInfo: bIsLast=%d \n%s \n%s", bIsLast,
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    {
        std::unique_lock<std::mutex> lock(terminate_order_sync_);
        if (pf && pf->bOrderAmount > 0 && !IsQuoteTerminate(pf))
        {
            unterminated_quotes_.push_back(*pf);
        }

        if (bIsLast && !HaveFinishQueryOrders())
        {
            std::thread process_after_get_quotes(&MYXSpeedSpi::ProcessAfterGetAllQuotes, this);
            process_after_get_quotes.detach();
        }
    }
}

bool MYXSpeedSpi::IsQuoteTerminate(struct DFITCQuoteOrderRtnField *pf)
{
    return pf &&
        (DFITC_SPD_CANCELED == pf->orderStatus             //全部撤单
        || DFITC_SPD_PARTIAL_CANCELED == pf->orderStatus   //部成部撤
        || DFITC_SPD_FILLED == pf->orderStatus             //全部成交
        || DFITC_SPD_ERROR == pf->orderStatus              //错误(废单错误)
        );
}

bool MYXSpeedSpi::IsOrderTerminate(struct DFITCOrderCommRtnField *pf)
{
    return pf &&
        (DFITC_SPD_CANCELED == pf->orderStatus             //全部撤单
        || DFITC_SPD_PARTIAL_CANCELED == pf->orderStatus   //部成部撤
        || DFITC_SPD_FILLED == pf->orderStatus             //全部成交
        || DFITC_SPD_ERROR == pf->orderStatus              //错误(废单错误)
        );
}

void MYXSpeedSpi::InitMDInterface()
{
    // create
    md_api = DFITCXSPEEDMDAPI::DFITCMdApi::CreateDFITCMdApi();

    if (!md_api)
    {
        TNL_LOG_ERROR("CreateDFITCMdApi failed, return null");
        return;
    }

    md_spi = new MYMdSpi(this);

    // set front address
    const std::string &quote_addr = cfg_.Logon_config().quote_front_addr;
    char *addr_tmp = new char[quote_addr.size() + 1];
    memcpy(addr_tmp, quote_addr.c_str(), quote_addr.size() + 1);
    md_api->Init(addr_tmp, md_spi);
    TNL_LOG_WARN("RegisterFront, addr: %s", addr_tmp);
    delete[] addr_tmp;
}

void MYXSpeedSpi::OnMDFrontConnected()
{
    TNL_LOG_INFO("OnMDFrontConnected.");

    DFITCUserLoginField login_data;
    memset(&login_data, 0, sizeof(DFITCUserLoginField));
    strncpy(login_data.accountID, tunnel_info_.account.c_str(), sizeof(login_data.accountID));
    strncpy(login_data.passwd, pswd_.c_str(), sizeof(login_data.passwd));

    // 成功登录后，断开不再重新登录
    if (in_init_state_)
    {
        int ret = md_api->ReqUserLogin(&login_data);
        TNL_LOG_WARN("MD - ReqUserLogin, return:%d", ret);
    }
    else
    {
        TNL_LOG_WARN("MD - have logined once, shouldn't call ReqUserLogin again");
    }
}

void MYXSpeedSpi::OnMDFrontDisconnected(int reason)
{
    TNL_LOG_INFO("OnMDFrontDisconnected, reason:%d - 0x%x.", reason, reason);
}

void MYXSpeedSpi::MDReqLogin()
{
    sleep(30);

    DFITCUserLoginField login_data;
    memset(&login_data, 0, sizeof(DFITCUserLoginField));
    strncpy(login_data.accountID, tunnel_info_.account.c_str(), sizeof(login_data.accountID));
    strncpy(login_data.passwd, pswd_.c_str(), sizeof(login_data.passwd));

    int ret = md_api->ReqUserLogin(&login_data);
    TNL_LOG_INFO("MDReqLogin, ret:%d  \n%s", ret, XSpeedDatatypeFormater::ToString(&login_data).c_str());
}

void MYXSpeedSpi::OnMDRspUserLogin(struct DFITCUserLoginInfoRtnField* pf, struct DFITCErrorRtnField* rsp)
{
    if (rsp == NULL && pf->loginResult == 0)
    {
        md_in_init_stat = false;

        // interface moved from trade api to md api in version XSpeedAPI_V1.0.9.14_sp4
        // subscribe all quote notice
        char * ppInstrumentID[1];
        ppInstrumentID[0] = new char[2];
        strcpy(ppInstrumentID[0], "*");

        int ret = md_api->SubscribeForQuoteRsp(ppInstrumentID, 1, 0);
        TNL_LOG_INFO("SubscribeForQuoteRsp, return:%d", ret);
    }
    else
    {
        if (rsp)
        {
            TNL_LOG_WARN("OnMDRspUserLogin, nErrorID:%d; errorMsg:%s", rsp->nErrorID, rsp->errorMsg);
        }

        // 重新登陆
        std::thread req_login(&MYXSpeedSpi::MDReqLogin, this);
        req_login.detach();
    }
}

void MYXSpeedSpi::OnMDRspUserLogout(struct DFITCUserLogoutInfoRtnField* pf, struct DFITCErrorRtnField* rsp)
{
    TNL_LOG_INFO("OnMDRspUserLogout: \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(rsp).c_str());
}

void MYXSpeedSpi::OnMDRspError(struct DFITCErrorRtnField* rsp)
{
    TNL_LOG_DEBUG("OnMDRspError:  \n%s", XSpeedDatatypeFormater::ToString(rsp).c_str());
}

void MYXSpeedSpi::OnMDRspSubForQuoteRsp(struct DFITCSpecificInstrumentField* pf, struct DFITCErrorRtnField* rsp)
{
    TNL_LOG_INFO("OnMDRspSubForQuoteRsp:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(rsp).c_str());
}

void MYXSpeedSpi::OnMDRspUnSubForQuoteRsp(struct DFITCSpecificInstrumentField* pf, struct DFITCErrorRtnField* rsp)
{
    TNL_LOG_INFO("OnMDRspUnSubForQuoteRsp:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(rsp).c_str());
}

void MYXSpeedSpi::OnMDRtnForQuoteRsp(struct DFITCQuoteSubscribeRtnField* pf)
{
    TNL_LOG_INFO("OnMDRtnForQuoteRsp:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());

    if (pf)
    {
        T_RtnForQuote rtn_forquote;
        memcpy(rtn_forquote.stock_code, pf->instrumentID, sizeof(rtn_forquote.stock_code));
        memcpy(rtn_forquote.for_quote_id, pf->quoteID, sizeof(rtn_forquote.for_quote_id));
        memcpy(rtn_forquote.for_quote_time, pf->quoteTime, sizeof(rtn_forquote.for_quote_time));
        rtn_forquote.for_quote_time[sizeof(rtn_forquote.for_quote_time) - 1] = '\0';

        if (RtnForQuoteHandler_) RtnForQuoteHandler_(&rtn_forquote);

        LogUtil::OnRtnForQuote(&rtn_forquote, tunnel_info_);
    }
}

bool MYXSpeedSpi::IsCancelOrder(struct DFITCOrderCommRtnField *pf)
{
    return pf &&
        (DFITC_SPD_CANCELED == pf->orderStatus             //全部撤单
        || DFITC_SPD_PARTIAL_CANCELED == pf->orderStatus   //部成部撤
        || DFITC_SPD_IN_CANCELING == pf->orderStatus       //撤单中
        );
}
void MYXSpeedSpi::HandleQueryOrders(struct DFITCOrderCommRtnField *pf, struct DFITCErrorRtnField * pe, bool bIsLast)
{
    bool qry_finish = false;
    if (bIsLast)
    {
        if (order_qry_step == 1)
        {
            // query option
            std::thread qry_order(&MYXSpeedSpi::QueryOptionOrder, this);
            qry_order.detach();
        }
        else if (order_qry_step == 2)
        {
            qry_finish = true;
            order_qry_step = 0;
        }
        else
        {
            TNL_LOG_WARN("ReqQryOrderInfo when order_qry_step(0-init;1-fut;2-opt):%d", order_qry_step.load());
            order_qry_step = 0;
        }
    }

    if (!have_handled_unterminated_orders_)
    {
        std::unique_lock<std::mutex> lock(terminate_order_sync_);
        if (pf && pf->orderAmount > 0 && !IsOrderTerminate(pf))
        {
            unterminated_orders_.push_back(*pf);
        }

        if (qry_finish && unterminated_orders_.empty())
        {
            have_handled_unterminated_orders_ = true;
            TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::HANDLED_UNTERMINATED_ORDERS_SUCCESS);
        }
    }

    if (!finish_query_canceltimes_)
    {
        std::unique_lock<std::mutex> lock(stats_canceltimes_sync_);
        if (pf && pf->orderAmount > 0 && IsCancelOrder(pf))
        {
            std::map<std::string, int>::iterator it = cancel_times_of_contract.find(pf->instrumentID);
            if (it == cancel_times_of_contract.end())
            {
                cancel_times_of_contract.insert(std::make_pair(pf->instrumentID, 1));
            }
            else
            {
                ++it->second;
            }
        }

        if (qry_finish)
        {
            // stats cancle times after terminate all orders, or terminate process will produce new cancel action
            if (have_handled_unterminated_orders_)
            {
                for (std::map<std::string, int>::iterator it = cancel_times_of_contract.begin();
                    it != cancel_times_of_contract.end(); ++it)
                {
                    ComplianceCheck_SetCancelTimes(tunnel_info_.account.c_str(), it->first.c_str(), it->second);
                }
                finish_query_canceltimes_ = true;
                TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::QUERY_CANCEL_TIMES_SUCCESS);
            }
            cancel_times_of_contract.clear();
        }
    }

    if (qry_finish)
    {
        qry_order_finish_cond_.notify_one();
    }

    if (HaveFinishQueryOrders())
    {
        is_ready_ = true;
        TunnelUpdateState(tunnel_info_.qtm_name.c_str(), QtmState::API_READY);
    }

    TNL_LOG_INFO("OnRspQryOrder before tunnel is ready, order(%s-%ld), last:%d",
        pf->instrumentID, pf->localOrderID, bIsLast);

    return;
}

void MYXSpeedSpi::QueryOptionTrade()
{
    usleep(1000 * 1000); // Query frequency can't be too fast

    DFITCMatchField qry_param;
    memset(&qry_param, 0, sizeof(DFITCMatchField));
    strncpy(qry_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
    qry_param.instrumentType = DFITC_OPT_TYPE;

    trade_qry_step = 2;
    int api_res = api_->ReqQryMatchInfo(&qry_param);
    TNL_LOG_DEBUG("ReqQryMatchInfo - ret=%d - %s", api_res, XSpeedDatatypeFormater::ToString(&qry_param).c_str());
    if (api_res != 0)
    {
        trade_qry_step = 0;

        T_TradeDetailReturn ret;
        ret.error_no = api_res;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

void MYXSpeedSpi::OnRspQryMatchInfo(struct DFITCMatchedRtnField* pf, struct DFITCErrorRtnField* pe, bool bIsLast)
{
    TNL_LOG_DEBUG("OnRspQryMatchInfo: bIsLast=%d \n%s \n%s", bIsLast,
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    T_TradeDetailReturn ret;

    // respond error
    if (pe && pe->nErrorID != 0)
    {
        ret.error_no = -1;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
        trade_qry_step = 0;
        return;
    }

    if (pf && pf->matchedAmount > 0)
    {
        TradeDetail td;
        strncpy(td.stock_code, pf->instrumentID, sizeof(td.stock_code));
        td.entrust_no = 0;
        td.direction = XSpeedFieldConvert::GetMYBuySell(pf->buySellType);
        td.open_close = XSpeedFieldConvert::GetMYOpenClose(pf->openClose);
        td.speculator = XSpeedFieldConvert::GetMYHedgeType(pf->speculator);
        td.trade_price = pf->matchedPrice;
        td.trade_volume = pf->matchedAmount;
        strncpy(td.trade_time, pf->matchedTime, sizeof(DFITCDateType));
        trade_detail_.push_back(td);
    }

    bool qry_finish = false;
    if (bIsLast)
    {
        if (trade_qry_step == 1)
        {
            // query option
            std::thread qry_trade(&MYXSpeedSpi::QueryOptionTrade, this);
            qry_trade.detach();
        }
        else if (trade_qry_step == 2)
        {
            qry_finish = true;
            trade_qry_step = 0;
        }
        else
        {
            TNL_LOG_WARN("OnRspQryMatchInfo when trade_qry_step(0-init;1-fut;2-opt):%d", trade_qry_step.load());
            trade_qry_step = 0;
        }
    }

    if (qry_finish)
    {
        TNL_LOG_INFO("receive all trade qry result, send to client");
        ret.datas.swap(trade_detail_);
        ret.error_no = 0;
        if (QryTradeDetailReturnHandler_) QryTradeDetailReturnHandler_(&ret);
        LogUtil::OnTradeDetailRtn(&ret, tunnel_info_);
    }
}

void MYXSpeedSpi::OnRspTradingDay(struct DFITCTradingDayRtnField* pf)
{
    TNL_LOG_INFO("OnRspTradingDay:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());
    if (pf) memcpy(trade_day, pf->date, sizeof(trade_day));
}

void MYXSpeedSpi::OnRspQuoteInsert(struct DFITCQuoteRspField* pf, struct DFITCErrorRtnField* pe)
{
    TNL_LOG_DEBUG("OnRspQuoteInsert:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRspQuoteInsert before tunnel is ready.");
        return;
    }

    if (pe && pe->nErrorID != 0) {
        ReportErrorState(pe->nErrorID, pe->errorMsg);
    }

    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        const XSpeedQuoteInfo *p = NULL;
        OrderRefDataType QuoteRef = 0;
        char entrust_status = MY_TNL_OS_ERROR;
        long entrust_no = 0;
        if (!pf)
        {
            TNL_LOG_WARN("OnRspQuoteInsert, DFITCQuoteRspField pointer is null.");
            return;
        }

        if (pe)
        {
            api_err_no = pe->nErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
            QuoteRef = pe->localOrderID;

            TNL_LOG_WARN("OnRspQuoteInsert, api_err_no: %d; my_err: %d; OrderRef: %lld", api_err_no, standard_error_no, QuoteRef);
        }
        else
        {
            QuoteRef = pf->localOrderID;
            entrust_no = 0;
            entrust_status = MY_TNL_OS_REPORDED;
        }

        p = xspeed_trade_context_.GetQuoteInfoByOrderRef(QuoteRef);
        if (p)
        {
            p->entrust_no = entrust_no;
            T_InsertQuoteRespond rsp;
            XSpeedPacker::QuoteRespond(standard_error_no, p->serial_no, entrust_no, entrust_status, rsp);

            if (InsertQuoteRespondHandler_) InsertQuoteRespondHandler_(&rsp);
            LogUtil::OnInsertQuoteRespond(&rsp, tunnel_info_);
        }
        else
        {
            TNL_LOG_INFO("can't get original quote info of localID: %lld", QuoteRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspQuoteInsert.");
    }
}

void MYXSpeedSpi::OnRtnQuoteInsert(struct DFITCQuoteRtnField* pf)
{
    TNL_LOG_DEBUG("OnRtnQuoteInsert:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRtnQuoteInsert before tunnel is ready.");
        return;
    }

    try
    {
        if (pf && pf->sessionID == session_id_)
        {
            OrderRefDataType QuoteRef = pf->localOrderID;
            const XSpeedQuoteInfo *p = xspeed_trade_context_.GetQuoteInfoByOrderRef(QuoteRef);
            if (p)
            {
                p->entrust_no = atoll(pf->orderSysID);
                char entrust_status = XSpeedFieldConvert::GetMYOrderStatus(pf->orderStatus);
                if (entrust_status == MY_TNL_OS_ERROR
                    || entrust_status == MY_TNL_OS_COMPLETED
                    || entrust_status == MY_TNL_OS_WITHDRAWED)
                {
                    // update
                    p->entrust_status = entrust_status;
                    p->buy_entrust_status = entrust_status;
                    p->sell_entrust_status = entrust_status;

                    // 回报
                    T_QuoteReturn quote_return;
                    XSpeedPacker::QuoteReturn(MY_TNL_D_BUY, pf->instrumentID, p, quote_return);
                    if (QuoteReturnHandler_) QuoteReturnHandler_(&quote_return);
                    LogUtil::OnQuoteReturn(&quote_return, tunnel_info_);

                    XSpeedPacker::QuoteReturn(MY_TNL_D_SELL, pf->instrumentID, p, quote_return);
                    if (QuoteReturnHandler_) QuoteReturnHandler_(&quote_return);
                    LogUtil::OnQuoteReturn(&quote_return, tunnel_info_);
                }
                else if (entrust_status == MY_TNL_OS_REPORDED)
                {
                    T_QuoteReturn quote_return;

                    memset(&quote_return, 0, sizeof(quote_return));
                    quote_return.entrust_no = p->entrust_no;
                    memcpy(quote_return.stock_code, pf->instrumentID, sizeof(quote_return.stock_code));
                    quote_return.serial_no = p->serial_no;
                    quote_return.direction = MY_TNL_D_BUY;

                    quote_return.entrust_status = MY_TNL_OS_REPORDED;
                    quote_return.open_close = p->buy_open_close;
                    quote_return.speculator = p->buy_speculator;
                    quote_return.volume = p->buy_volume;
                    quote_return.limit_price = p->buy_limit_price;
                    quote_return.volume_remain = p->buy_volume_remain;

                    if (QuoteReturnHandler_) QuoteReturnHandler_(&quote_return);
                    LogUtil::OnQuoteReturn(&quote_return, tunnel_info_);

                    quote_return.direction = MY_TNL_D_SELL;

                    quote_return.open_close = p->sell_open_close;
                    quote_return.speculator = p->sell_speculator;
                    quote_return.volume = p->sell_volume;
                    quote_return.limit_price = p->sell_limit_price;
                    quote_return.volume_remain = p->sell_volume_remain;

                    if (QuoteReturnHandler_) QuoteReturnHandler_(&quote_return);
                    LogUtil::OnQuoteReturn(&quote_return, tunnel_info_);
                }
            }
            else
            {
                TNL_LOG_INFO("can't get original quote info of localID: %lld", QuoteRef);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnQuoteInsert.");
    }
}

void MYXSpeedSpi::OnRtnQuoteMatchedInfo(struct DFITCQuoteMatchRtnField* pf)
{
    TNL_LOG_DEBUG("OnRtnQuoteMatchedInfo:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRtnQuoteMatchedInfo before tunnel is ready.");
        return;
    }

    try
    {
        if (pf && pf->sessionID == session_id_)
        {
            OrderRefDataType QuoteRef = pf->localOrderID;
            const XSpeedQuoteInfo *p = xspeed_trade_context_.GetQuoteInfoByOrderRef(QuoteRef);
            if (p)
            {
                char entrust_status = XSpeedFieldConvert::GetMYOrderStatus(pf->orderStatus);
                char dir = XSpeedFieldConvert::GetMYBuySell(pf->buySellType);

                // update
                p->entrust_status = entrust_status;
                if (dir == MY_TNL_D_BUY)
                {
                    p->buy_entrust_status = MY_TNL_OS_PARTIALCOM;
                    p->buy_volume_remain -= pf->matchedAmount;
                    if (p->buy_volume_remain == 0) p->buy_entrust_status = MY_TNL_OS_COMPLETED;
                }
                else
                {
                    p->sell_entrust_status = MY_TNL_OS_PARTIALCOM;
                    p->sell_volume_remain -= pf->matchedAmount;
                    if (p->sell_volume_remain == 0) p->sell_entrust_status = MY_TNL_OS_COMPLETED;
                }

                // 回报 quote return
                T_QuoteReturn quote_return;
                XSpeedPacker::QuoteReturn(dir, pf->instrumentID, p, quote_return);
                if (QuoteReturnHandler_) QuoteReturnHandler_(&quote_return);
                LogUtil::OnQuoteReturn(&quote_return, tunnel_info_);

                // 回报 trade return
                T_QuoteTrade quote_trade;
                XSpeedPacker::QuoteTrade(dir, pf, p, quote_trade);
                if (QuoteTradeHandler_) QuoteTradeHandler_(&quote_trade);
                LogUtil::OnQuoteTrade(&quote_trade, tunnel_info_);
            }
            else
            {
                TNL_LOG_INFO("can't get original quote info of localID: %lld", QuoteRef);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnQuoteMatchedInfo.");
    }
}

void MYXSpeedSpi::OnRspQuoteCancel(struct DFITCQuoteRspField* pf, struct DFITCErrorRtnField* pe)
{
    TNL_LOG_DEBUG("OnRspQuoteCancel:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRspQuoteCancel before tunnel is ready.");
        return;
    }

    if (pe && pe->nErrorID != 0) {
        ReportErrorState(pe->nErrorID, pe->errorMsg);
    }

    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        OrderRefDataType OrderRef = 0;
        const XSpeedQuoteInfo *p = NULL;

        if (!pf)
        {
            TNL_LOG_WARN("OnRspQuoteCancel, quote respond field is null.");
            return;
        }

        if (pe)
        {
            api_err_no = pe->nErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);
            OrderRef = pe->localOrderID;

            TNL_LOG_WARN("OnRspQuoteCancel, api_err_no: %d; my_err: %d; OrderRef: %lld", api_err_no, standard_error_no, OrderRef);
        }
        else
        {
            OrderRef = pf->localOrderID;
        }

        p = xspeed_trade_context_.GetQuoteInfoByOrderRef(OrderRef);
        if (p)
        {
            long cancel_serial_no = p->Pop_cancel_serial_no();
            if (cancel_serial_no != 0)
            {
                T_CancelQuoteRespond rsp;
                XSpeedPacker::CancelQuoteRespond(standard_error_no, cancel_serial_no, p->entrust_no, rsp);

                // 应答
                if (CancelQuoteRespondHandler_) CancelQuoteRespondHandler_(&rsp);
                LogUtil::OnCancelQuoteRespond(&rsp, tunnel_info_);
            }
            else
            {
                TNL_LOG_WARN("cancel quote: %ld from outside.", p->serial_no);
            }
        }
        else
        {
            TNL_LOG_INFO("can't get original quote info of order ref: %lld", OrderRef);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspQuoteCancel.");
    }
}

void MYXSpeedSpi::OnRtnQuoteCancel(struct DFITCQuoteCanceledRtnField* pf)
{
    TNL_LOG_DEBUG("OnRtnQuoteCancel:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRtnQuoteCancel before tunnel is ready.");
        return;
    }

    try
    {
        // sessionID is diff from login
        if (pf)
        {
            OrderRefDataType OrderRef = pf->localOrderID;
            const XSpeedQuoteInfo *p = xspeed_trade_context_.GetQuoteInfoByOrderRef(OrderRef);
            if (p)
            {
                p->entrust_status = XSpeedFieldConvert::GetMYOrderStatus(pf->orderStatus);

                // 回报
                if (pf->buySellType == DFITC_SPD_BUY)
                {
                    // update
                    p->buy_entrust_status = p->entrust_status;
                    T_QuoteReturn quote_return;
                    XSpeedPacker::QuoteReturn(MY_TNL_D_BUY, pf->instrumentID, p, quote_return);
                    if (QuoteReturnHandler_) QuoteReturnHandler_(&quote_return);
                    LogUtil::OnQuoteReturn(&quote_return, tunnel_info_);
                }

                if (pf->buySellType == DFITC_SPD_SELL)
                {
                    // update
                    p->sell_entrust_status = p->entrust_status;
                    T_QuoteReturn quote_return;
                    XSpeedPacker::QuoteReturn(MY_TNL_D_SELL, pf->instrumentID, p, quote_return);
                    if (QuoteReturnHandler_) QuoteReturnHandler_(&quote_return);
                    LogUtil::OnQuoteReturn(&quote_return, tunnel_info_);
                }
            }
            else
            {
                TNL_LOG_INFO("can't get original quote info of quote ref: %lld", OrderRef);
            }
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRtnQuoteCancel.");
    }
}

void MYXSpeedSpi::OnRtnExchangeStatus(struct DFITCExchangeStatusRtnField* pf)
{
    TNL_LOG_DEBUG("OnRtnExchangeStatus:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());
}

void MYXSpeedSpi::OnRspCancelAllOrder(struct DFITCCancelAllOrderRspField* pf, struct DFITCErrorRtnField* pe)
{
    TNL_LOG_DEBUG("OnRspCancelAllOrder:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());
}

void MYXSpeedSpi::OnRspForQuote(struct DFITCForQuoteRspField* pf, struct DFITCErrorRtnField* pe)
{
    TNL_LOG_DEBUG("OnRspForQuote:  \n%s \n%s",
        XSpeedDatatypeFormater::ToString(pf).c_str(),
        XSpeedDatatypeFormater::ToString(pe).c_str());

    if (!TunnelIsReady())
    {
        TNL_LOG_WARN("OnRspForQuote before tunnel is ready.");
        return;
    }

    if (pe && pe->nErrorID != 0) {
        ReportErrorState(pe->nErrorID, pe->errorMsg);
    }

    try
    {
        int standard_error_no = 0;
        int api_err_no = 0;
        if (!pf)
        {
            TNL_LOG_WARN("OnRspForQuote, p_data is null.");
            return;
        }
        OrderRefDataType OrderRef = pf->lRequestID;
        if (pe)
        {
            api_err_no = pe->nErrorID;
            standard_error_no = cfg_.GetStandardErrorNo(api_err_no);

            if (OrderRef < 0 && standard_error_no != 0 && pe->requestID > 0)
            {
                OrderRef = pe->requestID;
            }

            TNL_LOG_WARN("OnRspForQuote, api_err_code: %d; my_err_code: %d; request_id: %ld", api_err_no, standard_error_no, OrderRef);
        }

        long sn = xspeed_trade_context_.GetForquoteSerialNoOfOrderRef(OrderRef);
        if (sn != 0)
        {
            T_RspOfReqForQuote rsp;
            rsp.serial_no = sn;
            rsp.error_no = standard_error_no;

            if (RspOfReqForQuoteHandler_) RspOfReqForQuoteHandler_(&rsp);

            LogUtil::OnRspOfReqForQuote(&rsp, tunnel_info_);
        }
        else
        {
            TNL_LOG_INFO("can't find forquote by request id.", pf->lRequestID);
        }
    }
    catch (...)
    {
        TNL_LOG_ERROR("unknown exception in OnRspForQuote.");
    }
}

void MYXSpeedSpi::OnRtnForQuote(struct DFITCForQuoteRtnField* pf)
{
    TNL_LOG_DEBUG("OnRtnForQuote:  \n%s", XSpeedDatatypeFormater::ToString(pf).c_str());
}

//如果已经存储昨仓数据，直接从文件中读取，否则，存储到文件中
void MYXSpeedSpi::CheckAndSaveYestodayPosition()
{
    // calc avg price
    for (PositionDetail &v : position_detail_)
    {
        if (v.position > 0) v.position_avg_price = v.position_avg_price / v.position;
        if (v.yestoday_position > 0) v.yd_position_avg_price = v.yd_position_avg_price / v.yestoday_position;
    }

    // 存储文件名：yd_pos_账号_日期.rec
    std::string yd_pos_file_name = "yd_pos_" + tunnel_info_.account + "_" + std::string(trade_day) + ".rec";
    int ret = access(yd_pos_file_name.c_str(), F_OK);

    bool file_exist = (ret == 0);

    if (file_exist)
    {
        LoadYestodayPositionFromFile(yd_pos_file_name);
    }
    else
    {
        SaveYestodayPositionToFile(yd_pos_file_name);
    }
}

void MYXSpeedSpi::LoadYestodayPositionFromFile(const std::string &file)
{
    std::ifstream yd_pos_fs(file.c_str());
    if (!yd_pos_fs)
    {
        TNL_LOG_ERROR("open file failed when LoadYestodayPositionFromFile");
        return;
    }
    char r[255];
    while (yd_pos_fs.getline(r, 255))
    {
        std::vector<std::string> fields;
        my_cmn::MYStringSplit(r, fields, ',');

        // 格式：合约，方向，昨仓量，开仓均价
        if (fields.size() != 4)
        {
            TNL_LOG_ERROR("yestoday position data in wrong format, line: %s, file: %s", r, file.c_str());
            continue;
        }
        PositionDetail pos;
        memset(&pos, 0, sizeof(pos));
        strncpy(pos.stock_code, fields[0].c_str(), sizeof(pos.stock_code));
        pos.direction = fields[1][0];
        pos.yestoday_position = atoi(fields[2].c_str());
        pos.yd_position_avg_price = atof(fields[3].c_str());

        std::vector<PositionDetail>::iterator it = std::find_if(position_detail_.begin(),position_detail_.end(), PositionDetailPred(pos));
        if (it == position_detail_.end())
        {
            position_detail_.push_back(pos);
        }
        else
        {
            it->yestoday_position = pos.yestoday_position;
            it->yd_position_avg_price = pos.yd_position_avg_price;
        }
    }
}

void MYXSpeedSpi::SaveYestodayPositionToFile(const std::string &file)
{
    std::ofstream yd_pos_fs(file.c_str());
    if (!yd_pos_fs)
    {
        TNL_LOG_ERROR("open file failed when SaveYestodayPositionToFile");
        return;
    }

    for (PositionDetail v : position_detail_)
    {
        if (v.yestoday_position > 0)
        {
            // 格式：合约，方向，昨仓量，开仓均价
            yd_pos_fs << v.stock_code << ","
                << v.direction << ","
                << v.yestoday_position << ","
                << fixed << setprecision(16) << v.yd_position_avg_price << std::endl;
        }
    }
}

void MYXSpeedSpi::QueryOrdersForInitial()
{
    DFITCQuoteOrderField qry_quote_param;
    memset(&qry_quote_param, 0, sizeof(qry_quote_param));
    strncpy(qry_quote_param.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));

    std::vector<DFITCOrderCommRtnField> unterminated_orders_t;
    while (!HaveFinishQueryOrders())
    {
        // cancel all quotes
        while (true)
        {
            int qry_quote_ret = api_->ReqQryQuoteOrderInfo(&qry_quote_param);
            TNL_LOG_INFO("ReqQryQuoteOrderInfo for cancel unterminated quotes, return %d", qry_quote_ret);
            if (qry_quote_ret != 0)
            {
                // retry if failed, wait some seconds
                sleep(2);
                continue;
            }
            break;
        }

        {
            std::unique_lock<std::mutex> lock(terminate_order_sync_);
            qry_order_finish_cond_.wait_for(lock, std::chrono::seconds(10));
            unterminated_orders_t.swap(unterminated_orders_);
        }

        //遍历 unterminated_orders 间隔20ms（流控要求），发送撤单请求
        if (!have_handled_unterminated_orders_)
        {
            // cancel all orders
            for (const DFITCOrderCommRtnField &order_field : unterminated_orders_t)
            {
                DFITCCancelOrderField cancle_order;
                memset(&cancle_order, 0, sizeof(DFITCCancelOrderField));

                // 报单之后与前置连接断开或者 API 客户端软件关闭重启，只能通过柜台委托号进行撤单
                strncpy(cancle_order.accountID, tunnel_info_.account.c_str(), sizeof(DFITCAccountIDType));
                strncpy(cancle_order.instrumentID, order_field.instrumentID, sizeof(DFITCInstrumentIDType));
                //cancle_order.localOrderID = order_field.localOrderID;
                cancle_order.spdOrderID = order_field.spdOrderID;
                int cancel_ret = api_->ReqCancelOrder(&cancle_order);
                TNL_LOG_INFO("ReqCancelOrder for cancel unterminated orders, return %d", cancel_ret);

                usleep(20 * 1000);
            }
            unterminated_orders_t.clear();

            //全部发送完毕后，等待 ， 判断 handle_flag , 如没有完成，则retry
            sleep(2);
        }
    }
}
