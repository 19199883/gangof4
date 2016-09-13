#ifndef DCE_QUOTE_HANDLE_H_
#define DCE_QUOTE_HANDLE_H_

#include "my_quote_save.h "


std::string BestAndDeepToString(const SaveData_MDBestAndDeep * const p_data);
std::string ArbiToString(const SaveData_Arbi * const p_data);
std::string TenEntrustToString(const SaveData_MDTenEntrust * const p_data);
std::string OrderStatisticToString(const SaveData_MDOrderStatistic * const p_data);
std::string RealTimePriceToString(const SaveData_MDRealTimePrice * const p_data);
std::string MarchPriceQtyToString(const SaveData_MDMarchPriceQty * const p_data);


#endif // DCE_QUOTE_HANDLE_H_
