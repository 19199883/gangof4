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
 * the first data with sell dir
 * the second data with buy dir
 * the third data with instrument different from with the front
 */
TEST(RepairerTest, FindStartPointFitstWithSellDir)
{
	repairer rep;
	ASSERT_STREQ(rep.victim_.c_str(), "");
	ASSERT_FALSE(rep.working_);
	ASSERT_EQ(rep.seq_no_,-1);

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
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 20;
	found = rep.find_start_point(data2);
	ASSERT_FALSE(found);
	ASSERT_STREQ(rep.victim_.c_str(),"ag1706");
	ASSERT_FALSE(rep.working_);
	rep.seq_no_ = data2.seqno/10;

	MDPack data3;
	data3.direction = SHFE_FTDC_D_Buy;
	strcpy(data3.instrument,"ag1707");
	data3.seqno = 30;
	found = rep.find_start_point(data3);
	ASSERT_TRUE(found);
	ASSERT_STREQ(rep.victim_.c_str(),"");
	ASSERT_TRUE(rep.working_);
	rep.seq_no_ = data3.seqno/10;
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

/*
 * package loss happens before the second data.
 */
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


TEST(RepairerTest, LosePkg)
{
	repairer rep;

	MDPack data1;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;

	MDPack data2;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 30;

	MDPack data3;
	data3.direction = SHFE_FTDC_D_Buy;
	strcpy(data3.instrument,"ag1707");
	data3.seqno = 40;

	bool lost = rep.lose_pkg(data1);
	ASSERT_FALSE(lost);
	rep.seq_no_ = data1.seqno/10;

	lost = rep.lose_pkg(data2);
	ASSERT_TRUE(lost);
	rep.seq_no_ = data2.seqno/10;

	lost = rep.lose_pkg(data3);
	ASSERT_FALSE(lost);
	rep.seq_no_ = data3.seqno/10;
}


/*
 * the tiems in buy queue:
 * the first item: instrument(ag1705), count(3)
 * the second item: instrument(ag1706), count(120)
 * the third item: instrument(ag1706), count(12)
 *
 * the items in sell queue:
 * the first invoke:
 *		the first item: instrument(ag1705), count(3)
 * the first invoke:
 *		the second item: instrument(ag1706), count(120)
 *		the third item: instrument(ag1706), count(3)
 */
TEST(RepairerTest, PullReadyDataNormal)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	data1.count = 3;
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 20;
	data2.count = 120;
	MDPackEx dataEx3;
	MDPack &data3 = dataEx3.content;
	data3.direction = SHFE_FTDC_D_Buy;
	strcpy(data3.instrument,"ag1706");
	data3.seqno = 30;
	data3.count = 12;
	rep.buy_queue_.push_back(dataEx1);
	rep.buy_queue_.push_back(dataEx2);
	rep.buy_queue_.push_back(dataEx3);
	// sell queue
	MDPackEx dataEx4;
	MDPack &data4 = dataEx4.content;
	data4.direction = SHFE_FTDC_D_Sell;
	strcpy(data4.instrument,"ag1705");
	data4.seqno = 40;
	data4.count = 3;
	MDPackEx dataEx5;
	MDPack &data5 = dataEx5.content;
	data5.direction = SHFE_FTDC_D_Sell;
	strcpy(data5.instrument,"ag1706");
	data5.seqno = 50;
	data5.count = 120;
	MDPackEx dataEx6;
	MDPack &data6 = dataEx6.content;
	data6.direction = SHFE_FTDC_D_Buy;
	strcpy(data6.instrument,"ag1706");
	data6.seqno = 60;
	data6.count = 12;

	// first invoke
	rep.sell_queue_.push_back(dataEx4);
	rep.pull_ready_data();
	ASSERT_EQ(2,rep.buy_queue_.size());
	ASSERT_STREQ(data2.instrument,rep.buy_queue_[0].content.instrument);
	ASSERT_EQ(data2.count,rep.buy_queue_[0].content.count);
	ASSERT_EQ(0,rep.sell_queue_.size());
	// second invoke
	rep.sell_queue_.push_back(dataEx5);
	rep.sell_queue_.push_back(dataEx6);
	rep.pull_ready_data();
	ASSERT_EQ(0,rep.buy_queue_.size());
	ASSERT_EQ(0,rep.sell_queue_.size());

	ASSERT_EQ(6,rep.ready_queue_.size());
	// first invoke
	ASSERT_STREQ(data1.instrument,rep.ready_queue_[0].content.instrument);
	ASSERT_EQ(data1.count,rep.ready_queue_[0].content.count);
	ASSERT_STREQ(data4.instrument,rep.ready_queue_[1].content.instrument);
	ASSERT_EQ(data4.count,rep.ready_queue_[1].content.count);
	// second invoke
	ASSERT_STREQ(data2.instrument,rep.ready_queue_[2].content.instrument);
	ASSERT_EQ(data2.count,rep.ready_queue_[2].content.count);
	ASSERT_STREQ(data3.instrument,rep.ready_queue_[3].content.instrument);
	ASSERT_EQ(data3.count,rep.ready_queue_[3].content.count);
	ASSERT_STREQ(data5.instrument,rep.ready_queue_[4].content.instrument);
	ASSERT_EQ(data5.count,rep.ready_queue_[4].content.count);
	ASSERT_STREQ(data6.instrument,rep.ready_queue_[5].content.instrument);
	ASSERT_EQ(data6.count,rep.ready_queue_[5].content.count);
	ASSERT_EQ(data6.direction,rep.ready_queue_[5].content.direction);
}
