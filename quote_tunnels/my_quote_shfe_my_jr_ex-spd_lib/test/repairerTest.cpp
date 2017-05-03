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
	MDPack data1;
	data1.direction = SHFE_FTDC_D_Buy;
	strcpy(data1.instrument,"ag1705");
	data1.seqno = 0;
	bool found = rep.find_start_point(data1);
	EXPECT_EQ(found,false);

}


TEST(RepairerTest, FindStartPointNormalTwoVictims)
{

}


TEST(RepairerTest, FindStartPointFirstwithSellDir)
{

}



TEST(RepairerTest, FindStartPointPackLoss)
{

}
