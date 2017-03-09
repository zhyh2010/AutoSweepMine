#include "FindSweepMineMatrixParaTest.h"
#include "..\utils\WriteToBMP.h"
#include "opencv2\opencv.hpp"

void FindSweepMineMatrixParaTest::SetUp(){
	autoSweepMine.FindMineProgram();
	ASSERT_TRUE(autoSweepMine.SweepMineProgramInfo.MineWinHandle != nullptr);
}

void FindSweepMineMatrixParaTest::TearDown(){

}

TEST_F(FindSweepMineMatrixParaTest, FINDWINDOWMINEMATRIXAREA){
	autoSweepMine.GetMineMatrixArea();
	ASSERT_TRUE(autoSweepMine.MineMatrixInfo.MineMatrixArea.bottom > 0 &&
		autoSweepMine.MineMatrixInfo.MineMatrixArea.right > 0);
}

TEST_F(FindSweepMineMatrixParaTest, FINDWINDOWMINEMATRIXAREA_SAVEBITMAPTOFILE){
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixArea());
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixAreaBitmapToFile());
}

TEST_F(FindSweepMineMatrixParaTest, FINDWINDOWMINEMATRIXAREA_EXTRACTCONTOURS){
	ASSERT_NO_THROW(autoSweepMine.ExtractContoursForMineMatrixAreaBitmap());
}


