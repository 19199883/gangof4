#pragma once

#ifndef DLL_PUBLIC
#define DLL_PUBLIC  __attribute__ ((visibility("default")))
#endif

#include <string>

#pragma pack(push)
#pragma pack(8)

// 上期全息委托
typedef char TShfeFtdcInstrumentIDType[31];
typedef char TShfeFtdcDirectionType;
typedef double TShfeFtdcPriceType;
typedef int TShfeFtdcVolumeType;
///买
#define THOST_FTDC_D_Buy  '0'
#define SHFE_FTDC_D_Buy   '0'
///卖
#define THOST_FTDC_D_Sell '1'
#define SHFE_FTDC_D_Sell  '1'

struct DLL_PUBLIC CShfeFtdcMBLMarketDataField
{
    ///合约代码
    TShfeFtdcInstrumentIDType InstrumentID;
    ///买卖方向
    TShfeFtdcDirectionType Direction;
    ///价格
    TShfeFtdcPriceType Price;
    ///数量
    TShfeFtdcVolumeType Volume;
};
struct DLL_PUBLIC SHFEQuote
{
    CShfeFtdcMBLMarketDataField field;
    bool isLast;

    SHFEQuote(const CShfeFtdcMBLMarketDataField &d, bool last)
        : field(d), isLast(last)
    {
    }
    SHFEQuote(bool last)
    {
        isLast = last;
    }
    SHFEQuote()
    {
        isLast = false;
    }

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        return "0";
    }
};

#pragma pack(pop)
