#pragma once

#include <string.h>
#include "../src/repairer.h"
#include <gtest/gtest.h>

/*
 * one normal process.
 * the first is with buy dir.
 * the second is with instrument different from the first's
 */
TEST(RepairerTest, FindStartPointNormalOneVictim)
{
	repairer rep;
	ASSERT_STREQ(rep.victim_.c_str(), "");
	ASSERT_FALSE(rep.working_);
	ASSERT_EQ(rep.seq_no_,-1);

	MDPack data1;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	bool found = rep.find_start_point(data1);
	ASSERT_TRUE(!found);
	ASSERT_STREQ(rep.victim_.c_str(),"ag1705");
	ASSERT_FALSE(rep.working_);
	ASSERT_TRUE(!found);
	rep.seq_no_ = data1.seqno/10;

	MDPack data2;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 20;
	found = rep.find_start_point(data2);
	ASSERT_TRUE(found);
	ASSERT_STREQ(rep.victim_.c_str(),"");
	ASSERT_TRUE(rep.working_);
	rep.seq_no_ = data2.seqno/10;
}

/*
 * a normal process.
 * the first data with buy dir
 * the second data with buy dir and its instrument is same as the first's
 * the third data with buy dir and its instrument is different from the front two's
 */
TEST(RepairerTest, FindStartPointNormalTwoVictims)
{
	repairer rep;
	ASSERT_STREQ(rep.victim_.c_str(), "");
	ASSERT_FALSE(rep.working_);

	MDPack data1;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	bool found = rep.find_start_point(data1);
	ASSERT_TRUE(!found);
	ASSERT_STREQ(rep.victim_.c_str(),"ag1705");
	ASSERT_FALSE(rep.working_);
	ASSERT_TRUE(!found);
	rep.seq_no_ = data1.seqno/10;

	MDPack data2;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1705");
	data2.seqno = 20;
	found = rep.find_start_point(data2);
	ASSERT_FALSE(found);
	ASSERT_STREQ(rep.victim_.c_str(),"ag1705");
	ASSERT_FALSE(rep.working_);
	rep.seq_no_ = data2.seqno/10;

	MDPack data3;
	data3.direction = SHFE_FTDC_D_Buy;
	strcpy(data3.instrument,"ag1706");
	data3.seqno = 30;
	found = rep.find_start_point(data3);
	ASSERT_TRUE(found);
	ASSERT_STREQ(rep.victim_.c_str(),"");
	ASSERT_TRUE(rep.working_);
	rep.seq_no_ = data3.seqno/10;
}


/*
 * the first data with sell dir
 * the second data with buy dir
 * the third data with sell dir and its instrument is different form the sedond's
 */
TEST(RepairerTest, FindStartPointFirstwithSellDir)
{
	repairer rep;
	ASSERT_STREQ(rep.victim_.c_str(), "");
	ASSERT_FALSE(rep.working_);

	MDPack data1;
	data1.direction = SHFE_FTDC_D_Sell;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	bool found = rep.find_start_point(data1);
	ASSERT_TRUE(!found);
	ASSERT_STREQ(rep.victim_.c_str(),"");
	ASSERT_FALSE(rep.working_);
	ASSERT_TRUE(!found);
	rep.seq_no_ = data1.seqno/10;

	MDPack data2;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1705");
	data2.seqno = 20;
	found = rep.find_start_point(data2);
	ASSERT_FALSE(found);
	ASSERT_STREQ(rep.victim_.c_str(),"ag1705");
	ASSERT_FALSE(rep.working_);
	rep.seq_no_ = data2.seqno/10;

	MDPack data3;
	data3.direction = SHFE_FTDC_D_Buy;
	strcpy(data3.instrument,"ag1706");
	data3.seqno = 30;
	found = rep.find_start_point(data3);
	ASSERT_TRUE(found);
	ASSERT_STREQ(rep.victim_.c_str(),"");
	ASSERT_TRUE(rep.working_);
	rep.seq_no_ = data3.seqno/10;
}


TEST(RepairerTest, FindStartPointPackLoss)
{
	repairer rep;
	ASSERT_STREQ(rep.victim_.c_str(), "");
	ASSERT_FALSE(rep.working_);

	MDPack data1;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	bool found = rep.find_start_point(data1);
	ASSERT_TRUE(!found);
	ASSERT_STREQ(rep.victim_.c_str(),"ag1705");
	ASSERT_FALSE(rep.working_);
	ASSERT_TRUE(!found);
	rep.seq_no_ = data1.seqno/10;

	MDPack data2;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 30;
	found = rep.find_start_point(data2);
	ASSERT_FALSE(found);
	ASSERT_STREQ(rep.victim_.c_str(),"ag1706");
	ASSERT_FALSE(rep.working_);
	rep.seq_no_ = data2.seqno/10;

	MDPack data3;
	data3.direction = SHFE_FTDC_D_Buy;
	strcpy(data3.instrument,"ag1707");
	data3.seqno = 40;
	found = rep.find_start_point(data3);
	ASSERT_TRUE(found);
	ASSERT_STREQ(rep.victim_.c_str(),"");
	ASSERT_TRUE(rep.working_);
	rep.seq_no_ = data3.seqno/10;
}
