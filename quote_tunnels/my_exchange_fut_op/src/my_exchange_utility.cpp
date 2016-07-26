#include "my_exchange_datatype_inner.h"
#include "my_exchange_utility.h"
#include <sstream>

std::string OrderUtil::MYGetCurrentDateString()
{

    struct timeb timebuffer;
    ftime( &timebuffer );

    struct tm newtime;

    // Convert to local time.
    tm *p = localtime_r(&timebuffer.time, &newtime );
    if (!p)
    {
        return "20140101";
    }

    std::stringstream ss;
    ss << newtime.tm_year + 1900;
    ss.width(2); ss.fill('0'); ss << newtime.tm_mon + 1;
    ss.width(2); ss.fill('0'); ss << newtime.tm_mday;

    return ss.str();
}

std::string OrderUtil::GetCurrentDateTimeString()
{
    struct timeb timebuffer;
    ftime( &timebuffer );

    struct tm newtime;

    // Convert to local time.
    tm *p = localtime_r(&timebuffer.time, &newtime );
    if (!p)
    {
        return "20140101_123456";
    }

    std::stringstream ss;
    ss << newtime.tm_year + 1900;
    ss.width(2); ss.fill('0'); ss << newtime.tm_mon + 1;
    ss.width(2); ss.fill('0'); ss << newtime.tm_mday << "_";
    ss.width(2); ss.fill('0'); ss << newtime.tm_hour;
    ss.width(2); ss.fill('0'); ss << newtime.tm_min;
    ss.width(2); ss.fill('0'); ss << newtime.tm_sec;

    return ss.str();
}
