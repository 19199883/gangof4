#ifndef CME_QUOTE_HANDLE_H_
#define CME_QUOTE_HANDLE_H_

#include "my_quote_save.h "


std::string CME_QuoteToString(const SaveData_depthMarketData * const p);

std::string CME_QuoteToString(const SaveData_realTimeData * const p);

std::string CME_QuoteToString(const SaveData_orderbookData * const p);

std::string CME_QuoteToString(const SaveData_tradeVolume * const p);


#endif // CME_QUOTE_HANDLE_H_
