#if !defined(QUOTE_DATATYPE_SHFE_DEEP_H_)
#define QUOTE_DATATYPE_SHFE_DEEP_H_

// 上期全息委托
typedef char TShfeFtdcInstrumentIDType[31];
typedef char TShfeFtdcDirectionType;
typedef double TShfeFtdcPriceType;
typedef int TShfeFtdcVolumeType;
///买
#define THOST_FTDC_D_Buy '0'
///卖
#define THOST_FTDC_D_Sell '1'

#pragma pack(push)
#pragma pack(8)

struct CShfeFtdcMBLMarketDataField
{
	///合约代码
	TShfeFtdcInstrumentIDType	InstrumentID;
	///买卖方向
	TShfeFtdcDirectionType	Direction;
	///价格
	TShfeFtdcPriceType	Price;
	///数量
	TShfeFtdcVolumeType	Volume;
};
struct SHFEQuote
{
	CShfeFtdcMBLMarketDataField field;
	bool isLast;
};
#pragma pack(pop)

#endif  //QUOTE_DATATYPE_SHFE_DEEP_H_
