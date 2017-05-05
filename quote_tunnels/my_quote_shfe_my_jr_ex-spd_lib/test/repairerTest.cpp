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

/*
 * sell queue is empty
 */
TEST(RepairerTest, PullReadyDataSellQueueEmpty)
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
	//
	// first invoke
	rep.pull_ready_data();
	ASSERT_EQ(3,rep.buy_queue_.size());
	ASSERT_STREQ(data1.instrument,rep.buy_queue_[0].content.instrument);
	ASSERT_EQ(data1.count,rep.buy_queue_[0].content.count);
	ASSERT_EQ(0,rep.sell_queue_.size());
	ASSERT_EQ(0,rep.ready_queue_.size());
}


/*
 * buy queue is empty
 */
TEST(RepairerTest, PullReadyDataBuyQueueEmpty)
{
	repairer rep;

	// items in sell queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Sell;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	data1.count = 3;
	rep.sell_queue_.push_back(dataEx1);
	//
	// first invoke
	rep.pull_ready_data();
	ASSERT_EQ(0,rep.buy_queue_.size());
	ASSERT_EQ(0,rep.sell_queue_.size());
	ASSERT_EQ(0,rep.ready_queue_.size());
}

/*
 * the item's instrument in buy queue is equal to the 
 * first item's instrument in buy queue.
 *
 * sell_queue[0].instrument == buy_queue[0].instrument
 * sell_queue[0].instrument == buy_queue[1].instrument
 */
TEST(RepairerTest, PullReadyDataPullFrontTwoItemsOfBuyQueue)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	data1.count = 120;
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1705");
	data2.seqno = 20;
	data2.count = 12;
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

	// first invoke
	rep.sell_queue_.push_back(dataEx4);
	rep.pull_ready_data();
	// buy queue
	ASSERT_EQ(1,rep.buy_queue_.size());
	ASSERT_STREQ(data3.instrument,rep.buy_queue_[0].content.instrument);
	ASSERT_EQ(data3.count,rep.buy_queue_[0].content.count);
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(3,rep.ready_queue_.size());
	ASSERT_STREQ(data1.instrument,rep.ready_queue_[0].content.instrument);
	ASSERT_EQ(data1.count,rep.ready_queue_[0].content.count);
	ASSERT_STREQ(data2.instrument,rep.ready_queue_[1].content.instrument);
	ASSERT_EQ(data2.count,rep.ready_queue_[1].content.count);
	ASSERT_STREQ(data4.instrument,rep.ready_queue_[2].content.instrument);
	ASSERT_EQ(data4.count,rep.ready_queue_[2].content.count);
}


/*
 * discard the first item in buy queue  
 * pull the second item in buy queue  
 *
 * sell_queue[0].instrument > buy_queue[0].instrument
 * sell_queue[0].instrument == buy_queue[1].instrument
 */
TEST(RepairerTest, PullReadyDataPullSecondItemOfBuyQueue)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	data1.count = 12;
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
	strcpy(data4.instrument,"ag1706");
	data4.seqno = 40;
	data4.count = 3;

	// first invoke
	rep.sell_queue_.push_back(dataEx4);
	rep.pull_ready_data();
	// buy queue
	ASSERT_EQ(0,rep.buy_queue_.size());
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(3,rep.ready_queue_.size());
	ASSERT_STREQ(data2.instrument,rep.ready_queue_[0].content.instrument);
	ASSERT_EQ(data2.count,rep.ready_queue_[0].content.count);
	ASSERT_STREQ(data3.instrument,rep.ready_queue_[1].content.instrument);
	ASSERT_EQ(data3.count,rep.ready_queue_[1].content.count);
	ASSERT_STREQ(data4.instrument,rep.ready_queue_[2].content.instrument);
	ASSERT_EQ(data4.count,rep.ready_queue_[2].content.count);
}


/*
 * the instrusmet of items in sell queue is less than all 
 * instruments of items in buy queue.
 *
 * sell_queue[0].instrument < buy_queue[0].instrument
 *
 * items in buy queue:
 *		the first item("ag1706")
 *		the second item("ag1707")
 * items in sell queue:
 *		the first item("ag1705")
 */
TEST(RepairerTest, PullReadyDataPullNone1)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1706");
	data1.seqno = 10;
	data1.count = 12;
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1707");
	data2.seqno = 20;
	data2.count = 12;
	// sell queue
	MDPackEx dataEx4;
	MDPack &data4 = dataEx4.content;
	data4.direction = SHFE_FTDC_D_Sell;
	strcpy(data4.instrument,"ag1705");
	data4.seqno = 40;
	data4.count = 3;

	// first invoke
	rep.buy_queue_.push_back(dataEx1);
	rep.buy_queue_.push_back(dataEx2);
	rep.sell_queue_.push_back(dataEx4);
	rep.pull_ready_data();
	// buy queue
	ASSERT_EQ(2,rep.buy_queue_.size());
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(0,rep.ready_queue_.size());
}


/*
 * sell_queue[0].instrument > buy_queue[0].instrument
 * sell_queue[0].instrument < buy_queue[1].instrument
 *
 * items in buy queue:
 *		the first item("ag1706")
 *		the second item("ag1707")
 * items in sell queue:
 *		the first item("ag1705")
 */
TEST(RepairerTest, PullReadyDataPullNone2)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	data1.count = 12;
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1707");
	data2.seqno = 20;
	data2.count = 120;
	MDPackEx dataEx3;
	MDPack &data3 = dataEx3.content;
	data3.direction = SHFE_FTDC_D_Buy;
	strcpy(data3.instrument,"ag1708");
	data3.seqno = 30;
	data3.count = 12;
	rep.buy_queue_.push_back(dataEx1);
	rep.buy_queue_.push_back(dataEx2);
	rep.buy_queue_.push_back(dataEx3);
	// sell queue
	MDPackEx dataEx4;
	MDPack &data4 = dataEx4.content;
	data4.direction = SHFE_FTDC_D_Sell;
	strcpy(data4.instrument,"ag1706");
	data4.seqno = 40;
	data4.count = 3;

	// first invoke
	rep.sell_queue_.push_back(dataEx4);
	rep.pull_ready_data();
	// buy queue
	ASSERT_EQ(2,rep.buy_queue_.size());
	ASSERT_STREQ(data2.instrument,rep.buy_queue_[0].content.instrument);
	ASSERT_EQ(data2.count,rep.buy_queue_[0].content.count);
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(0,rep.ready_queue_.size());
}

/*
 * pull damaged data.
 * buy data damaged
 */
TEST(RepairerTest, PullReadyDataPullDamagedData1)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1706");
	data1.seqno = 10;
	data1.count = 120;
	ASSERT_FALSE(dataEx1.damaged);
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 20;
	data2.count = 120;
	// sell queue
	MDPackEx dataEx4;
	MDPack &data4 = dataEx4.content;
	data4.direction = SHFE_FTDC_D_Sell;
	strcpy(data4.instrument,"ag1706");
	data4.seqno = 40;
	data4.count = 3;
	ASSERT_FALSE(dataEx4.damaged);

	// first invoke
	dataEx1.damaged = true;
	rep.buy_queue_.push_back(dataEx1);
	rep.buy_queue_.push_back(dataEx2);
	rep.sell_queue_.push_back(dataEx4);
	rep.pull_ready_data();
	// buy queue
	ASSERT_EQ(0,rep.buy_queue_.size());
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(3,rep.ready_queue_.size());
	ASSERT_STREQ(data1.instrument,rep.ready_queue_[0].content.instrument);
	ASSERT_EQ(data1.count,rep.ready_queue_[0].content.count);
	ASSERT_TRUE(rep.ready_queue_[0].damaged);
	ASSERT_STREQ(data2.instrument,rep.ready_queue_[1].content.instrument);
	ASSERT_EQ(data2.count,rep.ready_queue_[1].content.count);
	ASSERT_TRUE(rep.ready_queue_[1].damaged);
	ASSERT_STREQ(data4.instrument,rep.ready_queue_[2].content.instrument);
	ASSERT_EQ(data4.count,rep.ready_queue_[2].content.count);
	ASSERT_TRUE(rep.ready_queue_[2].damaged);
}

/*
 * pull damaged data.
 * sell data damaged
 */
TEST(RepairerTest, PullReadyDataPullDamagedData2)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1706");
	data1.seqno = 10;
	data1.count = 120;
	ASSERT_FALSE(dataEx1.damaged);
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 20;
	data2.count = 120;
	ASSERT_FALSE(dataEx2.damaged);
	// sell queue
	MDPackEx dataEx4;
	ASSERT_FALSE(dataEx4.damaged);
	MDPack &data4 = dataEx4.content;
	data4.direction = SHFE_FTDC_D_Sell;
	strcpy(data4.instrument,"ag1706");
	data4.seqno = 40;
	data4.count = 3;

	// first invoke
	rep.buy_queue_.push_back(dataEx1);
	rep.buy_queue_.push_back(dataEx2);
	dataEx4.damaged = true;
	rep.sell_queue_.push_back(dataEx4);
	rep.pull_ready_data();
	// buy queue
	ASSERT_EQ(0,rep.buy_queue_.size());
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(3,rep.ready_queue_.size());
	ASSERT_STREQ(data1.instrument,rep.ready_queue_[0].content.instrument);
	ASSERT_EQ(data1.count,rep.ready_queue_[0].content.count);
	ASSERT_TRUE(rep.ready_queue_[0].damaged);
	ASSERT_STREQ(data2.instrument,rep.ready_queue_[1].content.instrument);
	ASSERT_EQ(data2.count,rep.ready_queue_[1].content.count);
	ASSERT_TRUE(rep.ready_queue_[1].damaged);
	ASSERT_STREQ(data4.instrument,rep.ready_queue_[2].content.instrument);
	ASSERT_EQ(data4.count,rep.ready_queue_[2].content.count);
	ASSERT_TRUE(rep.ready_queue_[2].damaged);
}

/*
 * buy queue in empty
 */
TEST(RepairerTest, NormalProcBuyDataBuyQueueEmpty)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1706");
	data1.seqno = 10;
	data1.count = 120;
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 20;
	data2.count = 12;
	// sell queue
	MDPackEx dataEx4;
	MDPack &data4 = dataEx4.content;
	data4.direction = SHFE_FTDC_D_Sell;
	strcpy(data4.instrument,"ag1706");
	data4.seqno = 30;
	data4.count = 3;

	// first invoke
	rep.normal_proc_buy_data(data1);
	// buy queue
	ASSERT_EQ(1,rep.buy_queue_.size());
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(0,rep.ready_queue_.size());
}


/*
 * the instrumen of new data is less than the instrument of last item in buy Queue
 */
TEST(RepairerTest, NormalProcBuyDataNewDataLessThanLastItemOfBuyQueue)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	data1.count = 120;
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 20;
	data2.count = 120;
	MDPackEx dataEx3;
	MDPack &data3 = dataEx3.content;
	data3.direction = SHFE_FTDC_D_Buy;
	strcpy(data3.instrument,"ag1704");
	data3.seqno = 30;
	data3.count = 120;
	// sell queue
	MDPackEx dataEx4;
	MDPack &data4 = dataEx4.content;
	data4.direction = SHFE_FTDC_D_Sell;
	strcpy(data4.instrument,"ag1706");
	data4.seqno = 40;
	data4.count = 3;

	// first invoke
	rep.buy_queue_.push_back(dataEx1);
	rep.buy_queue_.push_back(dataEx2);
	rep.sell_queue_.push_back(dataEx4);
	rep.normal_proc_buy_data(data3);
	// buy queue
	ASSERT_EQ(1,rep.buy_queue_.size());
	ASSERT_STREQ(data3.instrument,rep.buy_queue_[0].content.instrument);
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(2,rep.ready_queue_.size());
	ASSERT_STREQ(data2.instrument,rep.ready_queue_[0].content.instrument);
	ASSERT_EQ(data2.count,rep.ready_queue_[0].content.count);
	ASSERT_STREQ(data4.instrument,rep.ready_queue_[1].content.instrument);
	ASSERT_EQ(data4.count,rep.ready_queue_[1].content.count);
}


/*
 * the instrumen of new data is equal to the instrument of last item in buy Queue
 */
TEST(RepairerTest, NormalProcBuyDataNewDataEqualtoLastItemOfBuyQueue)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1706");
	data1.seqno = 10;
	data1.count = 120;
	ASSERT_FALSE(dataEx1.damaged);
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 20;
	data2.count = 12;
	ASSERT_FALSE(dataEx2.damaged);
	// sell queue
	MDPackEx dataEx4;
	MDPack &data4 = dataEx4.content;
	data4.direction = SHFE_FTDC_D_Sell;
	strcpy(data4.instrument,"ag1706");
	data4.seqno = 40;
	data4.count = 3;

	// first invoke
	rep.buy_queue_.push_back(dataEx1);
	rep.normal_proc_buy_data(data2);
	// buy queue
	ASSERT_EQ(2,rep.buy_queue_.size());
	ASSERT_STREQ(data1.instrument,rep.buy_queue_[0].content.instrument);
	ASSERT_EQ(data1.count,rep.buy_queue_[0].content.count);
	ASSERT_STREQ(data2.instrument,rep.buy_queue_[1].content.instrument);
	ASSERT_EQ(data2.count,rep.buy_queue_[1].content.count);
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(0,rep.ready_queue_.size());
}


/*
 * the instrumen of new data is greater than the instrument of last item in buy Queue
 */
TEST(RepairerTest, NormalProcBuyDataNewDataGreaterThanLastItemOfBuyQueue)
{
	repairer rep;

	// items in buy queue
	MDPackEx dataEx1;
	MDPack &data1 = dataEx1.content;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 10;
	data1.count = 120;
	ASSERT_FALSE(dataEx1.damaged);
	MDPackEx dataEx2;
	MDPack &data2 = dataEx2.content;
	data2.direction = SHFE_FTDC_D_Buy;
	strcpy(data2.instrument,"ag1706");
	data2.seqno = 20;
	data2.count = 120;
	ASSERT_FALSE(dataEx2.damaged);
	// sell queue
	MDPackEx dataEx4;
	MDPack &data4 = dataEx4.content;
	data4.direction = SHFE_FTDC_D_Sell;
	strcpy(data4.instrument,"ag1706");
	data4.seqno = 40;
	data4.count = 3;

	// first invoke
	rep.buy_queue_.push_back(dataEx1);
	rep.normal_proc_buy_data(data2);
	// buy queue
	ASSERT_EQ(2,rep.buy_queue_.size());
	ASSERT_STREQ(data1.instrument,rep.buy_queue_[0].content.instrument);
	ASSERT_EQ(data1.count,rep.buy_queue_[0].content.count);
	ASSERT_STREQ(data2.instrument,rep.buy_queue_[1].content.instrument);
	ASSERT_EQ(data2.count,rep.buy_queue_[1].content.count);
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(0,rep.ready_queue_.size());
}

// TODO:
/*
 * buy queue:
 *		data1:ag1705
 *		data2:ag1707
 * sell queue:
 *		data1:ag1707
 * victim:ag1704
 * new data1:ag1704
 * new data2:ag1705
 */
TEST(RepairerTest, RepairBuyDataOfThereAreTwoVictims)
{
	repairer rep;
	char victim[] = "ag1704";
	rep.victim_ = victim;

	// items in buy queue
	MDPackEx buyDataEx1;
	MDPack &buyData1 = buyDataEx1.content;
	buyData1.direction = SHFE_FTDC_D_Buy;
	strcpy(buyData1.instrument,"ag1705");
	buyData1.seqno = 10;
	buyData1.count = 12;

	MDPackEx buyDataEx2;
	MDPack &buyData2 = buyDataEx2.content;
	buyData2.direction = SHFE_FTDC_D_Buy;
	strcpy(buyData2.instrument,"ag1707");
	buyData2.seqno = 20;
	buyData2.count = 120;

	MDPackEx newDataEx1;
	MDPack &newData1 = newDataEx1.content;
	newData1.direction = SHFE_FTDC_D_Buy;
	strcpy(newData1.instrument,"ag1704");
	newData1.seqno = 20;
	newData1.count = 120;

	MDPackEx newDataEx2;
	MDPack &newData2 = newDataEx2.content;
	newData2.direction = SHFE_FTDC_D_Buy;
	strcpy(newData2.instrument,"ag1705");
	newData2.seqno = 20;
	newData2.count = 120;

	// sell queue
	MDPackEx sellDataEx1;
	MDPack &sellData1 = sellDataEx1.content;
	sellData1.direction = SHFE_FTDC_D_Sell;
	strcpy(sellData1.instrument,"ag1707");
	sellData1.seqno = 40;
	sellData1.count = 3;

	// first invoke
	rep.buy_queue_.push_back(buyDataEx1);
	rep.buy_queue_.push_back(buyDataEx2);
	rep.sell_queue_.push_back(sellDataEx1);
	rep.repair_buy_data(newData1);
	ASSERT_STREQ(victim,rep.victim_.c_str());
	// buy queue
	ASSERT_EQ(2,rep.buy_queue_.size());
	ASSERT_STREQ(buyData1.instrument,rep.buy_queue_[0].content.instrument);
	ASSERT_EQ(buyData1.count,rep.buy_queue_[0].content.count);
	// sell queue
	ASSERT_EQ(1,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(0,rep.ready_queue_.size());

	rep.repair_buy_data(newData2);
	ASSERT_STREQ("",rep.victim_.c_str());
	// buy queue
	ASSERT_EQ(1,rep.buy_queue_.size());
	ASSERT_STREQ(newData2.instrument,rep.buy_queue_[0].content.instrument);
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(2,rep.ready_queue_.size());
	ASSERT_STREQ(buyData2.instrument,rep.ready_queue_[0].content.instrument);
	ASSERT_STREQ(sellData1.instrument,rep.ready_queue_[1].content.instrument);
}



/*
 * there is two sell data items for the given instrument
 */
TEST(RepairerTest, NormalProcessSellDataOfTwoSellData)
{
	repairer rep;

	// items in buy queue
	MDPackEx buyDataEx1;
	MDPack &buyData1 = buyDataEx1.content;
	buyData1.direction = SHFE_FTDC_D_Buy;
	strcpy(buyData1.instrument,"ag1705");
	buyData1.seqno = 10;
	buyData1.count = 120;

	MDPackEx buyDataEx2;
	MDPack &buyData2 = buyDataEx2.content;
	buyData2.direction = SHFE_FTDC_D_Buy;
	strcpy(buyData2.instrument,"ag1705");
	buyData2.seqno = 20;
	buyData2.count = 120;

	// sell queue
	MDPackEx sellDataEx1;
	MDPack &sellData1 = sellDataEx1.content;
	sellData1.direction = SHFE_FTDC_D_Sell;
	strcpy(sellData1.instrument,"ag1705");
	sellData1.seqno = 40;
	sellData1.count = 120;

	MDPackEx sellDataEx2;
	MDPack &sellData2 = sellDataEx2.content;
	sellData2.direction = SHFE_FTDC_D_Sell;
	strcpy(sellData2.instrument,"ag1705");
	sellData2.seqno = 40;
	sellData2.count = 3;
	
	// first invoke
	rep.buy_queue_.push_back(buyDataEx1);
	rep.buy_queue_.push_back(buyDataEx2);
	rep.normal_proc_sell_data(sellData1);
	// buy queue
	ASSERT_EQ(2,rep.buy_queue_.size());
	ASSERT_STREQ(buyData1.instrument,rep.buy_queue_[0].content.instrument);
	ASSERT_EQ(buyData1.count,rep.buy_queue_[0].content.count);
	// sell queue
	ASSERT_EQ(1,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(0,rep.ready_queue_.size());

	// second invoke
	rep.normal_proc_sell_data(sellData2);
	// buy queue
	ASSERT_EQ(0,rep.buy_queue_.size());
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(4,rep.ready_queue_.size());
	ASSERT_STREQ(buyData1.instrument,rep.ready_queue_[0].content.instrument);
	ASSERT_STREQ(buyData2.instrument,rep.ready_queue_[1].content.instrument);
	ASSERT_STREQ(sellData1.instrument,rep.ready_queue_[2].content.instrument);
	ASSERT_STREQ(sellData2.instrument,rep.ready_queue_[3].content.instrument);
}


/*
 * there is only one sell data item for instrument
 */
TEST(RepairerTest, NormalProcessSellDataOfOneSellData)
{
	repairer rep;

	// items in buy queue
	MDPackEx buyDataEx1;
	MDPack &buyData1 = buyDataEx1.content;
	buyData1.direction = SHFE_FTDC_D_Buy;
	strcpy(buyData1.instrument,"ag1705");
	buyData1.seqno = 10;
	buyData1.count = 120;

	MDPackEx buyDataEx2;
	MDPack &buyData2 = buyDataEx2.content;
	buyData2.direction = SHFE_FTDC_D_Buy;
	strcpy(buyData2.instrument,"ag1705");
	buyData2.seqno = 20;
	buyData2.count = 120;

	// sell queue
	MDPackEx sellDataEx1;
	MDPack &sellData1 = sellDataEx1.content;
	sellData1.direction = SHFE_FTDC_D_Sell;
	strcpy(sellData1.instrument,"ag1705");
	sellData1.seqno = 40;
	sellData1.count = 12;

	// first invoke
	rep.buy_queue_.push_back(buyDataEx1);
	rep.buy_queue_.push_back(buyDataEx2);
	rep.normal_proc_sell_data(sellData1);
	// buy queue
	ASSERT_EQ(0,rep.buy_queue_.size());
	// sell queue
	ASSERT_EQ(0,rep.sell_queue_.size());
	// ready queue
	ASSERT_EQ(3,rep.ready_queue_.size());
	ASSERT_STREQ(buyData1.instrument,rep.ready_queue_[0].content.instrument);
	ASSERT_STREQ(buyData2.instrument,rep.ready_queue_[1].content.instrument);
	ASSERT_STREQ(sellData1.instrument,rep.ready_queue_[2].content.instrument);
}
