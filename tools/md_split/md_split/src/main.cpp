#include <stdlib.h>
#include <iostream>
#include <cstddef>
#include <fstream>
#include <map>
#include <unordered_set>
#include "util.h"

#define MAIN_LOG_DBG(format, args...) printf(format, ##args)

int main(int argc, char* argv[])
{
    printf("Usage: binary_split dat-file [contract_name ...]\n");

    if (argc < 2) return 1;

	bool filter_flag = false;
	std::unordered_set<std::string> filter_contracts;
    if (argc > 2)
    {
		filter_flag = true;
		for (int i = 2; i < argc; ++i)
		{
			filter_contracts.insert(argv[i]);
		}
    }

    std::string file_name = argv[1];
    std::ifstream f_in(file_name.c_str(), std::ios_base::binary);
	SaveFileHeaderStruct f_header;
    memset(&f_header, 0, sizeof(f_header));

    // 错误处理
    if (!f_in)
    {
        MAIN_LOG_DBG("file %s open failed.\n", file_name.c_str());
        return 1;
    }

    f_in.read((char *) &f_header, sizeof(f_header));
    // 错误处理
    if (f_header.data_length == 0)
    {
        MAIN_LOG_DBG("file %s read header failed.\n", file_name.c_str());
        return 1;
    }

    MAIN_LOG_DBG("data count of file(%s) is %d.\n", file_name.c_str(), f_header.data_count);

    std::map<std::string, FILE *> f_out_of_binary;
    std::map<std::string, int> data_count_binary;

    // get symbol offset in save data structure
    int symbol_offset = GetSymbolOffset(f_header.data_type, f_header.data_length);

    if (symbol_offset < 8)
    {
        MAIN_LOG_DBG("can't get symbol from data.\n");
        return 2;
    }

    char buffer[10240];
    int item_len = f_header.data_length;

	// 进度显示
	std::cout << "begin process, waiting..." << std::endl;
	test_progress_display pd(f_header.data_count);

    for (int i = 0; i < f_header.data_count; ++i)
    {
        f_in.read(buffer, item_len);
		pd += 1;
		std::string contract_name = std::string(buffer + symbol_offset);

		if (filter_flag && filter_contracts.find(contract_name) == filter_contracts.end())
		{
			continue;
		}

        std::map<std::string, FILE *>::iterator it_b = f_out_of_binary.find(contract_name);
        std::map<std::string, int>::iterator it_count = data_count_binary.find(contract_name);
        if (it_b == f_out_of_binary.end())
        {
            std::string file_sec = file_name.substr(0, file_name.size() - 4);
            std::string file_postfix = "_" + contract_name + ".dat";
            file_sec.append(file_postfix);
            FILE *f_out = fopen(file_sec.c_str(), "wb+");
            it_b = f_out_of_binary.insert(std::make_pair(contract_name, f_out)).first;

            fwrite(&f_header, sizeof(f_header), 1, it_b->second);

            it_count = data_count_binary.insert(std::make_pair(contract_name, 0)).first;
        }
        UTIL_SaveDataToFile(buffer, item_len, it_count->second, it_b->second);
    }

    f_in.close();
    return 0;
}

