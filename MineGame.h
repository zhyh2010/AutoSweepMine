#ifndef MINEGAME_H_
#define MINEGAME_H_

#define USINGUNITTEST

#include <vector>
#include <string>
#include <windows.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

enum MineStatus{
	UNKNOWN = 0, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, MINE
};

struct tMineCell{
	int RowId;
	int ColId;
	MineStatus Mine;
};

enum tFaceStatus{
	CONTINUE = 0, SUCCESS, FAILURE
};

struct tSweepMineProgram{
	static string MineWinClass;
	static string MineWinName;
	static int MineCellLen;
	HWND MineWinHandle;
	tSweepMineProgram() :MineWinHandle(nullptr){}
};

struct tMineMatrix{
	int Rows;
	int Cols;
	int MineNums;
	tFaceStatus FaceStatus;
	Rect MineMatrixArea;
	Rect MineMatrixMineNumArea;
	Rect MineMatrixFaceArea;
	RECT MineMatrixAndMineNumFaceArea;
};

class AutoSweepMine{
public:
	void FindMineProgram();
	void GetMineMatrixInfo();

	void GetMineMatrixAndMineNumFaceArea();

	// 获取雷区
	void GetMineMatrixArea(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixAreaRoughPosition(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixAreaFinePosition();

	// 获取雷计数区
	void GetMineMatrixMineNumArea(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixMineNumAreaRoughPosition(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixMineNumAreaFinePosition();

	// 获取脸区
	void GetMineMatrixFaceArea(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixFaceAreaRoughPosition(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixFaceAreaFinePosition();


	void GetMineMatrixAndMineNumFaceAreaBitmapToFile();
	vector<vector<Point>> ExtractContoursForMineMatrixAreaBitmap();
	vector<double> GetContoursAreaArray(vector<vector<Point>> & contours);

	int MatchMineNum(Mat BitmapNum);
	void PreHandleMineNums(Mat & BitmapNum);

	MineStatus GetMineMatrixCellStatusByRowAndCol(int row, int col);

	void DoAutoSweepMine();


#ifdef USINGUNITTEST
public:
#else
private:
#endif
	int GetMineMatrixRows();
	int GetMineMatrixCols();
	int GetMineMatrixMineNums();
	tFaceStatus GetMineMatrixFaceStatus();
	void PreHandleMineFaces(Mat & BitmapFace);
	Rect PreHandleMineFaces_FindBasicFaceArea(Mat & BitmapFace);

	vector<vector<tMineCell>> MineMatrix;
	tSweepMineProgram SweepMineProgramInfo;
	tMineMatrix MineMatrixInfo;
	cv::Mat src_ori;
};

#endif
