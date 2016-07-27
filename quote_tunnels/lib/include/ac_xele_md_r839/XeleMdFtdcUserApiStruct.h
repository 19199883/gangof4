/////////////////////////////////////////////////////////////////////////
///@system Xele-MD
///@company AcceleCom.Nanjing
///@file XELEFtdcUserApiStruct.h
///@brief 定义了客户端接口使用的业务数据结构
///@history 
/////////////////////////////////////////////////////////////////////////

#if !defined(_XELE_FTDCSTRUCT_H)
#define _XELE_FTDCSTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XeleMdFtdcUserApiDataType.h"
#pragma pack(push,1)

///系统用户登录请求
struct CXeleMdFtdcReqUserLoginField
{
    ///交易用户代码
    TXeleMdFtdcUserIDType UserID;
    ///经纪公司编号
    TXeleMdFtdcBrokerIDType BrokerID;
    ///密码
    TXeleMdFtdcPasswordType Password;
    ///协议信息
    TXeleMdFtdcProtocolInfoType ProtocolInfo;

};
///系统用户登录应答
struct CXeleMdFtdcRspUserLoginField
{
    ///交易日
    TXeleMdFtdcDateType TradingDay;
    ///经纪公司编号
    TXeleMdFtdcBrokerIDType BrokerID;
    ///交易用户代码
    TXeleMdFtdcUserIDType UserID;
    ///登录成功时间
    TXeleMdFtdcTimeType LoginTime;
    ///用户最大本地报单号
    TXeleMdFtdcUserOrderLocalIDType MaxOrderLocalID;
    ///交易系统名称
    TXeleMdFtdcTradingSystemNameType TradingSystemName;
    ///数据中心代码
    TXeleMdFtdcDataCenterIDType DataCenterID;
    ///会员私有流当前长度
    TXeleMdFtdcSequenceNoType PrivateFlowSize;
    ///交易员私有流当前长度
    TXeleMdFtdcSequenceNoType UserFlowSize;
};
///用户登出请求
struct CXeleMdFtdcReqUserLogoutField
{
    ///经纪公司编号
    TXeleMdFtdcBrokerIDType BrokerID;
    ///交易用户代码
    TXeleMdFtdcUserIDType UserID;
};
///用户登出请求
struct CXeleMdFtdcRspUserLogoutField
{
    ///经纪公司编号
    TXeleMdFtdcBrokerIDType BrokerID;
    ///交易用户代码
    TXeleMdFtdcUserIDType UserID;
};

///用户口令修改
struct CXeleMdFtdcUserPasswordUpdateField
{
    ///经纪公司编号
    TXeleMdFtdcBrokerIDType BrokerID;
    ///交易用户代码
    TXeleMdFtdcUserIDType UserID;
    ///旧密码
    TXeleMdFtdcPasswordType OldPassword;
    ///新密码
    TXeleMdFtdcPasswordType NewPassword;
};

///响应信息
struct CXeleMdFtdcRspInfoField
{
    ///错误代码
    TXeleMdFtdcErrorIDType ErrorID;
    ///错误信息
    TXeleMdFtdcErrorMsgType ErrorMsg;
};

///信息分发
struct CXeleMdFtdcDisseminationField
{
    ///序列系列号
    TXeleMdFtdcSequenceSeriesType SequenceSeries;
    ///序列号
    TXeleMdFtdcSequenceNoType SequenceNo;
};
///出入金结果
struct CXeleMdFtdcInvestorAccountDepositResField
{
    ///经纪公司编号
    TXeleMdFtdcBrokerIDType BrokerID;
    ///用户代码
    TXeleMdFtdcUserIDType UserID;
    ///投资者编号
    TXeleMdFtdcInvestorIDType InvestorID;
    ///资金帐号
    TXeleMdFtdcAccountIDType AccountID;
    ///资金流水号
    TXeleMdFtdcAccountSeqNoType AccountSeqNo;
    ///金额
    TXeleMdFtdcMoneyType Amount;
    ///出入金方向
    TXeleMdFtdcAccountDirectionType AmountDirection;
    ///可用资金
    TXeleMdFtdcMoneyType Available;
    ///结算准备金
    TXeleMdFtdcMoneyType Balance;
};
///行情基础属性
struct CXeleMdFtdcMarketDataBaseField
{
    ///交易日
    TXeleMdFtdcDateType TradingDay;
    ///结算组代码
    TXeleMdFtdcSettlementGroupIDType SettlementGroupID;
    ///结算编号
    TXeleMdFtdcSettlementIDType SettlementID;
    ///昨结算
    TXeleMdFtdcPriceType PreSettlementPrice;
    ///昨收盘
    TXeleMdFtdcPriceType PreClosePrice;
    ///昨持仓量
    TXeleMdFtdcLargeVolumeType PreOpenInterest;
    ///昨虚实度
    TXeleMdFtdcRatioType PreDelta;
};
///行情静态属性
struct CXeleMdFtdcMarketDataStaticField
{
    ///今开盘
    TXeleMdFtdcPriceType OpenPrice;
    ///最高价
    TXeleMdFtdcPriceType HighestPrice;
    ///最低价
    TXeleMdFtdcPriceType LowestPrice;
    ///今收盘
    TXeleMdFtdcPriceType ClosePrice;
    ///涨停板价
    TXeleMdFtdcPriceType UpperLimitPrice;
    ///跌停板价
    TXeleMdFtdcPriceType LowerLimitPrice;
    ///今结算
    TXeleMdFtdcPriceType SettlementPrice;
    ///今虚实度
    TXeleMdFtdcRatioType CurrDelta;
};
///行情最新成交属性
struct CXeleMdFtdcMarketDataLastMatchField
{
    ///最新价
    TXeleMdFtdcPriceType LastPrice;
    ///数量
    TXeleMdFtdcVolumeType Volume;
    ///成交金额
    TXeleMdFtdcMoneyType Turnover;
    ///持仓量
    TXeleMdFtdcLargeVolumeType OpenInterest;
};
///行情最优价属性
struct CXeleMdFtdcMarketDataBestPriceField
{
    ///申买价一
    TXeleMdFtdcPriceType BidPrice1;
    ///申买量一
    TXeleMdFtdcVolumeType BidVolume1;
    ///申卖价一
    TXeleMdFtdcPriceType AskPrice1;
    ///申卖量一
    TXeleMdFtdcVolumeType AskVolume1;
};
///行情申买二、三属性
struct CXeleMdFtdcMarketDataBid23Field
{
    ///申买价二
    TXeleMdFtdcPriceType BidPrice2;
    ///申买量二
    TXeleMdFtdcVolumeType BidVolume2;
    ///申买价三
    TXeleMdFtdcPriceType BidPrice3;
    ///申买量三
    TXeleMdFtdcVolumeType BidVolume3;
};
///行情申卖二、三属性
struct CXeleMdFtdcMarketDataAsk23Field
{
    ///申卖价二
    TXeleMdFtdcPriceType AskPrice2;
    ///申卖量二
    TXeleMdFtdcVolumeType AskVolume2;
    ///申卖价三
    TXeleMdFtdcPriceType AskPrice3;
    ///申卖量三
    TXeleMdFtdcVolumeType AskVolume3;
};
///行情申买四、五属性
struct CXeleMdFtdcMarketDataBid45Field
{
    ///申买价四
    TXeleMdFtdcPriceType BidPrice4;
    ///申买量四
    TXeleMdFtdcVolumeType BidVolume4;
    ///申买价五
    TXeleMdFtdcPriceType BidPrice5;
    ///申买量五
    TXeleMdFtdcVolumeType BidVolume5;
};
///行情申卖四、五属性
struct CXeleMdFtdcMarketDataAsk45Field
{
    ///申卖价四
    TXeleMdFtdcPriceType AskPrice4;
    ///申卖量四
    TXeleMdFtdcVolumeType AskVolume4;
    ///申卖价五
    TXeleMdFtdcPriceType AskPrice5;
    ///申卖量五
    TXeleMdFtdcVolumeType AskVolume5;
};
///行情更新时间属性
struct CXeleMdFtdcMarketDataUpdateTimeField
{
    ///合约代码
    TXeleMdFtdcInstrumentIDType InstrumentID;
    ///最后修改时间
    TXeleMdFtdcTimeType UpdateTime;
    ///最后修改毫秒
    TXeleMdFtdcMillisecType UpdateMillisec;
};
///深度行情
struct CXeleMdFtdcDepthMarketDataField
{
    ///合约代码
    TXeleMdFtdcInstrumentIDType InstrumentID;
    ///最后修改时间
    TXeleMdFtdcTimeType UpdateTime;
    ///最后修改毫秒
    TXeleMdFtdcMillisecType UpdateMillisec;
    char __padding1[4];
    ///今开盘
    TXeleMdFtdcPriceType OpenPrice;
    ///最高价
    TXeleMdFtdcPriceType HighestPrice;
    ///最低价
    TXeleMdFtdcPriceType LowestPrice;
    ///今收盘
    TXeleMdFtdcPriceType ClosePrice;
    ///涨停板价
    TXeleMdFtdcPriceType UpperLimitPrice;
    ///跌停板价
    TXeleMdFtdcPriceType LowerLimitPrice;
    ///今结算
    TXeleMdFtdcPriceType SettlementPrice;
    ///今虚实度
    TXeleMdFtdcRatioType CurrDelta;
    char __padding2[4];
    ///最新价
    TXeleMdFtdcPriceType LastPrice;
    ///数量
    TXeleMdFtdcVolumeType Volume;
    ///成交金额
    TXeleMdFtdcMoneyType Turnover;
    ///持仓量
    TXeleMdFtdcLargeVolumeType OpenInterest;
    char __padding3[4];
    ///申买价一
    TXeleMdFtdcPriceType BidPrice1;
    ///申买量一
    TXeleMdFtdcVolumeType BidVolume1;
    ///申卖价一
    TXeleMdFtdcPriceType AskPrice1;
    ///申卖量一
    TXeleMdFtdcVolumeType AskVolume1;
    char __padding4[4];
    ///申买价二
    TXeleMdFtdcPriceType BidPrice2;
    ///申买量二
    TXeleMdFtdcVolumeType BidVolume2;
    ///申买价三
    TXeleMdFtdcPriceType BidPrice3;
    ///申买量三
    TXeleMdFtdcVolumeType BidVolume3;
    char __padding5[4];
    ///申卖价二
    TXeleMdFtdcPriceType AskPrice2;
    ///申卖量二
    TXeleMdFtdcVolumeType AskVolume2;
    ///申卖价三
    TXeleMdFtdcPriceType AskPrice3;
    ///申卖量三
    TXeleMdFtdcVolumeType AskVolume3;
    char __padding6[4];
    ///申买价四
    TXeleMdFtdcPriceType BidPrice4;
    ///申买量四
    TXeleMdFtdcVolumeType BidVolume4;
    ///申买价五
    TXeleMdFtdcPriceType BidPrice5;
    ///申买量五
    TXeleMdFtdcVolumeType BidVolume5;
    char __padding7[4];
    ///申卖价四
    TXeleMdFtdcPriceType AskPrice4;
    ///申卖量四
    TXeleMdFtdcVolumeType AskVolume4;
    ///申卖价五
    TXeleMdFtdcPriceType AskPrice5;
    ///申卖量五
    TXeleMdFtdcVolumeType AskVolume5;

};

#pragma pack(pop)

#endif
