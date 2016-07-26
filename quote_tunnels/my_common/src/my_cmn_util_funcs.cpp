#include "my_cmn_util_funcs.h"
#include <sys/timeb.h>
#include <time.h>
#include <string>
#include <sstream>

using namespace my_cmn;

// 20130115_123059  2013-01-15 12:30:59
std::string my_cmn::GetCurrentDateTimeString()
{
    struct timeb timebuffer;
    ftime(&timebuffer);

    struct tm newtime;

    // Convert to local time.
    tm *p = localtime_r(&timebuffer.time, &newtime);
    if (!p)
    {
        return "20130115_123059";
    }

    std::stringstream ss;
    ss << newtime.tm_year + 1900;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_mon + 1;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_mday << "_";
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_hour;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_min;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_sec;

    return ss.str();
}

// 20130115_123059_500  2013-01-15 12:30:59.500
std::string my_cmn::GetCurrentDateTimeWithMilisecString()
{
    struct timeb timebuffer;
    ftime(&timebuffer);

    struct tm newtime;

    // Convert to local time.
    tm *p = localtime_r(&timebuffer.time, &newtime);
    if (!p)
    {
        return "20130115_123059_500";
    }

    std::stringstream ss;
    ss << newtime.tm_year + 1900;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_mon + 1;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_mday << "_";
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_hour;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_min;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_sec;
    ss << "_";
    ss.width(3);
    ss.fill('0');
    ss << timebuffer.millitm;

    return ss.str();
}

// 20130115
std::string my_cmn::GetCurrentDateString()
{
    struct timeb timebuffer;
    ftime(&timebuffer);

    struct tm newtime;

    // Convert to local time.
    tm *p = localtime_r(&timebuffer.time, &newtime);
    if (!p)
    {
        return "20130101";
    }

    std::stringstream ss;
    ss << newtime.tm_year + 1900;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_mon + 1;
    ss.width(2);
    ss.fill('0');
    ss << newtime.tm_mday;

    return ss.str();
}

// 12:30:59.500  12:30:59.500
std::string my_cmn::GetCurrentTimeWithMilisecString()
{
    struct timeb timebuffer;
    ftime(&timebuffer);

    struct tm newtime;

    // Convert to local time.
    tm *p = localtime_r(&timebuffer.time, &newtime);
    if (!p)
    {
        return "00:00:00.000";
    }

    char buf[64];
    sprintf(buf, "%02d:%02d:%02d.%03d",
        newtime.tm_hour,
        newtime.tm_min,
        newtime.tm_sec,
        timebuffer.millitm
        );

    return buf;
}

// 123059  12:30:59
int my_cmn::GetCurrentTimeInt()
{
    struct timeb timebuffer;
    ftime(&timebuffer);

    struct tm newtime;

    // Convert to local time.
    tm *p = localtime_r(&timebuffer.time, &newtime);
    if (!p)
    {
        return 123059;
    }

    int t = newtime.tm_hour;
    t *= 100;
    t += newtime.tm_min;
    t *= 100;
    t += newtime.tm_sec;

    return t;
}

// 123059500  12:30:59.500
int my_cmn::GetCurrentTimeWithMilisecInt()
{
    struct timeb timebuffer;
    ftime(&timebuffer);

    struct tm newtime;

    // Convert to local time.
    tm *p = localtime_r(&timebuffer.time, &newtime);
    if (!p)
    {
        return 123059500;
    }

    int t = newtime.tm_hour;
    t *= 100;
    t += newtime.tm_min;
    t *= 100;
    t += newtime.tm_sec;
    t *= 1000;
    t += timebuffer.millitm;

    return t;
}

// 20130115
int my_cmn::GetCurrentDateInt()
{
    struct timeb timebuffer;
    ftime(&timebuffer);

    struct tm newtime;

    // Convert to local time.
    tm *p = localtime_r(&timebuffer.time, &newtime);
    if (!p)
    {
        return 20130101;
    }

    int d = newtime.tm_year + 1900;
    d *= 100;
    d += newtime.tm_mon + 1;
    d *= 100;
    d += newtime.tm_mday;

    return d;
}
