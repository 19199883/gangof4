#ifndef KMDS_QUOTE_HANDLE_H_
#define KMDS_QUOTE_HANDLE_H_

#include "my_quote_save.h "


std::string KMDS_QuoteToString(const SaveData_StockQuote_KMDS * const p);
std::string KMDS_QuoteToString(const SaveData_IndexQuote_KMDS * const p);
std::string KMDS_QuoteToString(const SaveData_Option_KMDS * const p);
std::string KMDS_QuoteToString(const SaveData_FutureQuote_KMDS * const p);
std::string KMDS_QuoteToString(const SaveData_OrderQueue_KMDS * const p);
std::string KMDS_QuoteToString(const SaveData_PerEntrust_KMDS * const p);
std::string KMDS_QuoteToString(const SaveData_PerBargain_KMDS * const p);



#endif // KMDS_QUOTE_HANDLE_H_
