#pragma once

#include "quote_datatype_cffex_level2.h"
#include "quote_datatype_level1.h"
//#include "quote_datatype_shfe_my.h"
#include "quote_datatype_shfe_deep.h"
#include "quote_datatype_sec_kmds.h"
#include "quote_datatype_tap.h"
#include "quote_datatype_datasource.h"
#include "quote_datatype_ib.h"
#include "quote_sgit_data_type.h"


#include <ctime>
#include <string>
#include "exchange_names.h"
#include "security_type.h"

union semun{
	 int val;
	 struct semid_ds *buf;
	 ushort *array;
 }sem_u;



