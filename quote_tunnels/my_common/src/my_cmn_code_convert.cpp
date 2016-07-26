/*
 * codeconvert.cpp
 *
 *  Created on: 2012-7-4
 *      Author: oliver
 */

#include <stddef.h>
#include <string.h>
#include "my_cmn_code_convert.h"

using namespace my_cmn;

CodeConvert::CodeConvert( const char *input, const char * fromCodeset, const char * toCodeset )
{
	if (input == NULL || fromCodeset == NULL || toCodeset == NULL)
	{
		iconvPtr = NULL;
		outputBuf = new char[1];
		outputBuf[0] = '\0';
        return;
	}
	
	// 获取输入字符串长度，估算输出长度
	size_t inLen = strlen(input);
	size_t outLen = 2 * inLen + 1;

	// allocate memery
	outputBuf = new char[outLen];
	memset(outputBuf, 0, outLen);

	// get the conv pointer
	iconvPtr = iconv_open(toCodeset, fromCodeset);

	// 用临时指针指向输入/输出字符串（iconv函数要修改这两个指针）
	char *inT = const_cast<char *>(input);
	char *outT = outputBuf;

	// check the convert pointer, and convert the string.
	if (iconvPtr && iconv_t(-1) != iconvPtr)
	{
		iconv(iconvPtr, &inT, &inLen, &outT, &outLen);
	}
}

CodeConvert::~CodeConvert(void)
{
	if (iconvPtr != NULL)
	{
		iconv_close(iconvPtr);
	}
	
	delete [] outputBuf;
}

const char * CodeConvert::DestString()
{
	return outputBuf;
}

