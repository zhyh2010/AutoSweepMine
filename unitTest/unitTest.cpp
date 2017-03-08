#include "gtest/gtest.h"
#include "unitTest.h"

void FindSweepMineTest::SetUp(){
	
}

void FindSweepMineTest::TearDown(){

}

TEST_F(FindSweepMineTest, FIND){
	autoSweepMine.FindMineProgram();
	ASSERT_TRUE(autoSweepMine.SweepMineProgramInfo.MineWinHandle != nullptr);
}


