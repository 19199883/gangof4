#pragma once

/*
定义行情类别
*/
enum quote_category_options
{
	//	CFfexFtdcDepthMarketData
	SPIF = 0,

	//	MDBestAndDeep_MY
	CF = 1,

	Stock = 2,

	// CShfeFtdcMBLMarketDataField
	FullDepth = 3,

	MDOrderStatistic = 4,
};

