#include "FindSweepMineMatrixParaTest.h"
#include "..\utils\WriteToBMP.h"
#include "opencv2\opencv.hpp"
#include <iostream>

using namespace cv;
using namespace std;

#define DRAW_REGION_IN_BITMAP(RECTNAME){ \
	Mat src = autoSweepMine.src_ori.clone(); \
	Rect MineAreaRect = autoSweepMine.MineMatrixInfo.##RECTNAME; \
 	rectangle(src, MineAreaRect, Scalar(255, 0, 0), 3); \
 	putText(src, #RECTNAME, Point(MineAreaRect.x + MineAreaRect.width / 2, \
 		MineAreaRect.y + MineAreaRect.height / 2), CV_FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0), 2); \
	imshow(#RECTNAME, src); \
 	cvWaitKey(); \
 }

void FindSweepMineMatrixParaTest::SetUp(){
	autoSweepMine.FindMineProgram();
	ASSERT_TRUE(autoSweepMine.SweepMineProgramInfo.MineWinHandle != nullptr);
}

void FindSweepMineMatrixParaTest::TearDown(){

}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA){
	autoSweepMine.GetMineMatrixAndMineNumFaceArea();
	ASSERT_TRUE(autoSweepMine.MineMatrixInfo.MineMatrixAndMineNumFaceArea.bottom > 0 &&
		autoSweepMine.MineMatrixInfo.MineMatrixAndMineNumFaceArea.right > 0);
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_SAVEBITMAPTOFILE){
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixAndMineNumFaceArea());
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixAndMineNumFaceAreaBitmapToFile());
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_EXTRACTCONTOURS){
	ASSERT_NO_THROW(autoSweepMine.ExtractContoursForMineMatrixAreaBitmap());
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_GetMineMatrixArea){
	auto contours = autoSweepMine.ExtractContoursForMineMatrixAreaBitmap();
	auto ContoursAreaArray = autoSweepMine.GetContoursAreaArray(contours);
	autoSweepMine.GetMineMatrixArea(contours, ContoursAreaArray);

	DRAW_REGION_IN_BITMAP(MineMatrixArea);	
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_GetMineMatrixMineNumArea){
	auto contours = autoSweepMine.ExtractContoursForMineMatrixAreaBitmap();
	auto ContoursAreaArray = autoSweepMine.GetContoursAreaArray(contours);
	autoSweepMine.GetMineMatrixMineNumArea(contours, ContoursAreaArray);

	DRAW_REGION_IN_BITMAP(MineMatrixMineNumArea);

	Mat roi = autoSweepMine.src_ori(autoSweepMine.MineMatrixInfo.MineMatrixMineNumArea);
	namedWindow("roi", CV_WINDOW_NORMAL);
	imshow("roi", roi);
	cvWaitKey();
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_GetMineMatrixFaceArea){
	auto contours = autoSweepMine.ExtractContoursForMineMatrixAreaBitmap();
	auto ContoursAreaArray = autoSweepMine.GetContoursAreaArray(contours);
	autoSweepMine.GetMineMatrixFaceArea(contours, ContoursAreaArray);

	DRAW_REGION_IN_BITMAP(MineMatrixFaceArea);
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_GetMineMatrixMineNums){
	ASSERT_TRUE(autoSweepMine.GetMineMatrixMineNums() == 99);
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_GetMineMatrixInfo){
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixInfo());
	ASSERT_TRUE(autoSweepMine.MineMatrixInfo.Rows == 16);
	ASSERT_TRUE(autoSweepMine.MineMatrixInfo.Cols == 30);
	ASSERT_TRUE(autoSweepMine.MineMatrixInfo.MineNums == 99);
	ASSERT_TRUE(autoSweepMine.MineMatrixInfo.FaceStatus == CONTINUE);
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_GetMineMatrixCellStatusByRowAndCol){
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixInfo());
	ASSERT_TRUE(autoSweepMine.GetMineMatrixCellStatusByRowAndCol(1, 1) == UNKNOWN);
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_SetNearestUnknownCells){
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixAndMineNumFaceArea());
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixAndMineNumFaceAreaBitmapToFile());
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixInfo());
	autoSweepMine.SetNearestUnknownCellsFlag(1, 2);
	ASSERT_TRUE(autoSweepMine.GetMineMatrixCellStatusByRowAndCol(1, 2) == FLAG);
	Sleep(1000);
	autoSweepMine.SetNearestUnknownCellsFlag(1, 2);
	Sleep(1000);
	autoSweepMine.SetNearestUnknownCellsFlag(1, 2);
	ASSERT_TRUE(autoSweepMine.GetMineMatrixCellStatusByRowAndCol(1, 2) == UNKNOWN);
	Sleep(1000);
	autoSweepMine.SetNearestUnknownCellsSafe(1, 2);
	ASSERT_FALSE(autoSweepMine.GetMineMatrixCellStatusByRowAndCol(1, 2) == UNKNOWN);
}

TEST_F(FindSweepMineMatrixParaTest, DISABLED_FINDWINDOWMINEMATRIXAREA_DoAutoSweepMine){
	ASSERT_NO_THROW(autoSweepMine.DoAutoSweepMine());
}

TEST_F(FindSweepMineMatrixParaTest, /*DISABLED_*/FINDWINDOWMINEMATRIXAREA_ReadMemory){
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixAndMineNumFaceArea());
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixAndMineNumFaceAreaBitmapToFile());
	ASSERT_NO_THROW(autoSweepMine.GetMineMatrixInfo());
	ASSERT_NO_THROW(autoSweepMine.ReadMemory());
}



