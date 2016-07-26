
#include "security_type.h"
#include <tinyxml.h>
#include <tinystr.h>

using namespace quote_agent;

quote_source_setting::quote_source_setting(void)
{
	is_match_quote = false;
	shmdatakeyfile = "";
	shmdataflagkeyfile = "";
	semkeyfile = "";
}

quote_source_setting::~quote_source_setting(void)
{
}
		
