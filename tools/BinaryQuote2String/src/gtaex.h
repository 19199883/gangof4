#ifndef GTA_QUOTE_HANDLE_H_
#define GTA_QUOTE_HANDLE_H_

#include "my_quote_save.h "
#include <unordered_map>

typedef std::unordered_map<std::string, double> InstrumentToAmountMap;

std::string GTAQuoteToString(const SaveData_GTAEX * const p);



#endif // GTA_QUOTE_HANDLE_H_
