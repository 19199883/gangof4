#pragma once

#ifndef DLL_PUBLIC
#define DLL_PUBLIC  __attribute__ ((visibility("default")))
#endif

#include <string>

#pragma pack(push)
#pragma pack(8)

// 茂源股票衍生数据-包头
struct DLL_PUBLIC my_hs300_idx_header
{
    char cMsgType;        // message type. reserved field
    int nBodyLength;  // message body length
};

// 茂源股票衍生数据-包体
struct DLL_PUBLIC my_hs300_idx
{
    char code[20];
    int time;  // HHMMSSmmm
    double index_raw;
    double index_est;
    double ratio;

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        char buf[64];
        sprintf(buf, "%02d:%02d:%02d.%03d", 
			time / 10000000,
            (time / 100000) % 100,
            (time / 1000) % 100,
            time % 1000);
        return std::string(buf);
    }
};

#pragma pack(pop)
