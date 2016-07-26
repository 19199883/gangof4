#include "esunny_position_interface.h"

#include <iostream>
#include <fstream>
#include <unordered_map>

#include "esunny_data_formater.h"
#include "my_cmn_util_funcs.h"
#include "my_cmn_log.h"
#include "field_convert.h"
#include "tunnel_cmn_utility.h"

struct PositionSaveInFile {
	char direction;
	int volume;
};

static std::string ReadAuthCode() {
	char l[1024];
	std::string auth_code;
	ifstream auth_cfg("auth_code.dat");
	while (auth_cfg.getline(l, 1023)) {
		auth_code.append(l);
	}

	return auth_code;
}

template<typename T>
inline std::string GetInstrumentID(const T* info)
{
    std::string contract;

    contract.append(info->CommodityNo);
    contract.append(info->ContractNo);
    if (info->CommodityType == TAPI_COMMODITY_TYPE_OPTION)
    {
        contract.append(1, info->CallOrPutFlag);
        contract.append(info->StrikePrice);
    }

    return contract;
}

MYEsunnyPositionSpi::MYEsunnyPositionSpi(const TunnelConfigData &cfg) :
		cfg_(cfg) {
	TNL_LOG_INFO("Query position function start.");

	user_ = cfg_.Logon_config().investorid;
	pswd_ = cfg_.Logon_config().password;
	file_ = "pos_" + user_ + "_" + my_cmn::GetCurrentDateString() + ".txt";

	TapAPIApplicationInfo auth_info;
	std::string auth_code = ReadAuthCode();
	strncpy(auth_info.AuthCode, auth_code.c_str(), sizeof(auth_info.AuthCode));
	strcpy(auth_info.KeyOperationLogPath, "");

	TAPIINT32 result;
	api_ = CreateTapTradeAPI(&auth_info, result);
	if (!api_ || result != TAPIERROR_SUCCEED) {
		TNL_LOG_WARN("Position - CreateTapTradeAPI result: %d", result);
		return;
	}

	api_->SetAPINotify(this);

	for (const std::string &v : cfg.Logon_config().front_addrs) {
		IPAndPortNum ip_port = ParseIPAndPortNum(v);
		api_->SetHostAddress(ip_port.first.c_str(), ip_port.second);
		TNL_LOG_INFO("Position - SetHostAddress, addr: %s:%d", ip_port.first.c_str(),
				ip_port.second);
		break;
	}

	//登录服务器
	TapAPITradeLoginAuth stLoginAuth;
	memset(&stLoginAuth, 0, sizeof(stLoginAuth));
	strcpy(stLoginAuth.UserNo, user_.c_str());
	strcpy(stLoginAuth.Password, pswd_.c_str());
	stLoginAuth.ISModifyPassword = APIYNFLAG_NO;
	stLoginAuth.ISDDA = APIYNFLAG_NO;

	result = api_->Login(&stLoginAuth);
	if (TAPIERROR_SUCCEED != result) {
		TNL_LOG_ERROR("Position - Login Error, result: %d", result);
	}
}

MYEsunnyPositionSpi::~MYEsunnyPositionSpi() {
    if (api_)
    {
        api_->Disconnect();
        api_ = NULL;
    }
}


void MYEsunnyPositionSpi::OnConnect() {
	TNL_LOG_DEBUG("Position - OnConnect");
}

void MYEsunnyPositionSpi::OnRspLogin(TAPIINT32 errorCode, const TapAPITradeLoginRspInfo *loginRspInfo) {
	TNL_LOG_DEBUG("Position - OnRspLogin");
}

void MYEsunnyPositionSpi::OnAPIReady() {
	TNL_LOG_DEBUG("Position - OnAPIReady");

	TAPIUINT32 sessionId;
	TapAPIPositionQryReq req;
	memset(&req, 0, sizeof(req));
	api_->QryPosition(&sessionId, &req);
}

void MYEsunnyPositionSpi::OnDisconnect(TAPIINT32 reasonCode) {
	TNL_LOG_DEBUG("Position - OnDisconnect");
}

void MYEsunnyPositionSpi::OnRspQryPosition(TAPIUINT32 sessionID,TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIPositionInfo *info) {
	TNL_LOG_DEBUG("Position - OnRspQryPosition");
	if (errorCode == 0 && info) {
		pos_info_.push_back(*info);
	}
	if (isLast == APIYNFLAG_YES) {
		SaveToFile();
		api_->Disconnect();
	}
}

void MYEsunnyPositionSpi::SaveToFile() {
	TNL_LOG_DEBUG("Position - Save to file");
	std::ofstream f_out(file_);
	if (!f_out) {
		TNL_LOG_ERROR("Position - open %s failed", file_.c_str());
		return ;
	}
	std::unordered_map<std::string, PositionSaveInFile> pos_map;
	auto mit = pos_map.begin();
	for (auto it = pos_info_.begin(); it != pos_info_.end(); it++) {
		mit = pos_map.find(GetInstrumentID(&(*it)));
		if (mit != pos_map.end()) {
			mit->second.volume += it->PositionQty;
		} else {
			PositionSaveInFile tmp;
			tmp.direction = it->MatchSide;
			tmp.volume = it->PositionQty;
			pos_map.insert(make_pair(GetInstrumentID(&(*it)), tmp));
		}
	}
	for (mit =  pos_map.begin(); mit != pos_map.end(); mit++) {
		f_out << mit->first << ',' ;
		if (mit->second.direction == TAPI_SIDE_BUY) f_out << "0," << mit->second.volume << std::endl;
		else f_out << "1," << mit->second.volume << std::endl;
	}
	f_out.close();
}
