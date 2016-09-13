#ifndef UTIL_H_
#define UTIL_H_

#include "my_quote_save.h "


void ConvertTimval(long long loc_t, int &d, int &t);

std::string ConvertToShortTimval( long long loc_t );

std::string TrimAndToString(const char * const p, int len);

#endif // UTIL_H_
