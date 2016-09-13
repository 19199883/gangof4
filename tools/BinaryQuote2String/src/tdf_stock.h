#ifndef TDF_STOCK_QUOTE_HANDLE_H_
#define TDF_STOCK_QUOTE_HANDLE_H_

#include "my_quote_save.h "


std::string TDF_Market_Data_QuoteToString(const SaveData_TDF_MARKET_DATA * const p);
std::string TDF_Index_Data_QuoteToString(const SaveData_TDF_INDEX_DATA * const p);



#endif // TDF_STOCK_QUOTE_HANDLE_H_
