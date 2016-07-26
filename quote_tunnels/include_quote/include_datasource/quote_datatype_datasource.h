#pragma once

#include <string>

#define DLL_PUBLIC  __attribute__ ((visibility("default")))

#pragma pack(push)
#pragma pack(8)

// 茂源衍生数据-包头
struct my_datasource_header
{
    char cMsgType;    // message type. reserved field
    int nBodyLength;  // message body length
};

struct cStruct_my_data_each
{
    char item[64];
    double value;
};

struct cStruct_my_data_each_symbol
{
    char symbol[64];
    int time;
    int item_count;
    cStruct_my_data_each detail[50];
};

// 茂源衍生数据-包体
struct my_datasource_data
{
    char data_item[64];
    int symbol_count;
    cStruct_my_data_each_symbol symbol_detail[20];

    // HH:MM:SS.mmm
    std::string GetQuoteTime() const
    {
        return "";
    }
};


#define MAX_INNER_LEN  16384
/* 16384 byte = 16KB */
#define MAX_INNER_SYM_LEN 4

struct inner_quote_fmt {
    int msg_len;
    char inner_symbol[MAX_INNER_SYM_LEN];
    char inner_quote_addr[MAX_INNER_LEN];
};



#pragma pack(pop)

