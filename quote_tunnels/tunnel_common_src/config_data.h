#ifndef MY_TRADE_TUNNEL_CONFIGDATA_H_
#define MY_TRADE_TUNNEL_CONFIGDATA_H_

#include <list>
#include <string>
#include <map>
#include <unordered_map>
#include <mxml.h>

enum TunnelProviderType
{
	PROVIDERTYPE_CTP       = 1,
	PROVIDERTYPE_DCE       = 2,
	PROVIDERTYPE_FEMAS     = 3,
	PROVIDERTYPE_ZEUSING   = 4,
	PROVIDERTYPE_KSG       = 5,
    	PROVIDERTYPE_CITICS_HS = 6,
    	PROVIDERTYPE_ESUNNY    = 7,
    	PROVIDERTYPE_XELE      = 8,
    	PROVIDERTYPE_REM       = 9,
    	PROVIDERTYPE_FIX_IB    = 10,
    	PROVIDERTYPE_IB_API    = 11,
   	PROVIDERTYPE_FIX_KGI   = 12,
    	PROVIDERTYPE_LTS       = 13,
    	PROVIDERTYPE_SGIT      = 14,
};

struct AppConfig
{
	std::string program_title;
	int provider_type;
	int orderref_prefix_id;
	// added on 20141224, for publish tunnel information
	int share_memory_key;
};

// 登录信息
struct LogonConfig
{
	std::list<std::string> front_addrs; // 前置机地址列表
	std::string brokerid;
    std::string investorid;
	std::string clientid;
	std::string password;
    std::string exch_code;
    // added for zeusing
    std::string UserProductInfo;
    std::string AuthCode;
    // added for xele
    std::string ParticipantID;
    std::string BusinessUnit;
    std::string InterfaceProductInfo;
    std::string ProtocolInfo;
    int DataCenterID;
    char ClientType;
    std::string quote_front_addr; // to recv forquote notice for market making
    std::string mac_address;
	
    // added for citics stock interface
    std::string client_name;
    std::string local_ip;
    std::string mac_code;
    std::string op_entrust_way;
    std::string identity_type;
    std::string password_type;
};

// 合规检查参数
struct ComplianceCheckParam
{
	// 开仓限制
	int max_open_of_speculate;	// 最大投机开仓数
	int max_open_of_arbitrage;	// 最大套利开仓数
	int max_open_of_total;		// 最大开仓总数

	// 撤单限制
	int cancel_warn_threshold;	// 撤单告警阈值，到达后，拦截开仓指令
	int cancel_upper_limit;		// 撤单上限，达到该次数后，拦截撤单指令
    int init_cancel_times_at_start; // 0: no, init with 0; 1: yes, query orders to init cancel times; default is 1;
	std::string switch_mask;
};

// 外部错误号对照结构
struct OuterErrorInfo
{
	int error_no;			// ZEUSING 错误码
	int standard_error_no;	// 映射到的标准错误号
	std::string error_info;	// 外部错误信息
};

// 外部错误号和标准错误号的映射表
typedef std::unordered_map<int, int> OuterErrorNoToStandardErroNo;
typedef OuterErrorNoToStandardErroNo::iterator OuterErrorNoToStandardErroNoIt;
typedef OuterErrorNoToStandardErroNo::const_iterator OuterErrorNoToStandardErroNoCit;

struct InitialPolicy
{
    bool cancel_orders_at_start;
    std::string time_to_query_pos;
};

// common functions
mxml_node_t *LoadFileInTree(const std::string &file_name);
std::string GetOneNodeStringValue(mxml_node_t *node, const std::string &sub_node_name);

// 配置数据管理
class TunnelConfigData
{
public:
	void Load(const std::string &cfg_file);

	// 根据外部系统错误号，获取标准错误号
	int GetStandardErrorNo(int outer_error_no);
	// 对外提供的数据访问接口
	const AppConfig & App_cfg() const { return app_cfg_; }
	const LogonConfig & Logon_config() const { return logon_config_; }
	const ComplianceCheckParam & Compliance_check_param() const { return compliance_check_param_; }
	const InitialPolicy & Initial_policy_param() const { return initial_policy_param_;}
    // check if known error no(which don't need report to oss)
    bool IsKnownErrorNo(int api_error_no)
    {
        if (api_error_no < 0 || api_error_no > max_known_error_no_)
        {
            return false;
        }

        return known_error_no_[api_error_no] == 1;
    }


private:
	// 加载应用配置
	void LoadTunnelCfg(const std::string &cfg_file);

	// 加载错误码配置
	void LoadErrCfg(const std::string &cfg_file);

    void ParseLoginConfig(mxml_node_t *login_node);
    void ParseComplianceCheckParam(mxml_node_t *cc_node, mxml_node_t *top);
    void ParseInitialPolicy(mxml_node_t *policy_node);

    AppConfig app_cfg_;
	LogonConfig logon_config_;
	ComplianceCheckParam compliance_check_param_;
	OuterErrorNoToStandardErroNo outer_error_to_standard_error_;
	InitialPolicy initial_policy_param_;

    char *known_error_no_;
    int max_known_error_no_;
};

#endif // MY_TRADE_TUNNEL_CONFIGDATA_H_
