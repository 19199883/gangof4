#pragma once

#include <string.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <string>

using namespace std;

#pragma pack(push)
#pragma pack(8)

struct IBDepth
{
    char Symbol[64];
    char Exchange[8];
    char Currency[8];
    long long Timestamp;
    double OpenPrice;
    double HighPrice;
    double LowPrice;
    double LastClosePrice;
    double LastPrice;
    int LastSize;
    int TotalVolume;
    double AskPrice[10];
    double BidPrice[10];
    int AskVol[10];
    int BidVol[10];

    IBDepth()
    {
        memset(this, 0, sizeof(IBDepth));
    }

    std::string GetQuoteTime() const
    {
        return "";
    }
};

struct IBTick
{
    char Symbol[64];
    int Position;
    int Operation;
    int Side;
    double Price;
    int Size;
    IBTick()
    {
        memset(Symbol, 0, sizeof(Symbol));
    }
    IBTick(std::string sb, int ps, int o, int s, double p, int sz) :
        Position(ps), Operation(o), Side(s), Price(p), Size(sz)
    {
        memset(Symbol, 0, sizeof(Symbol));
        strcpy(Symbol, sb.c_str());
    }
    std::string GetQuoteTime() const
    {
        return "";
    }
};

#pragma pack(pop)

