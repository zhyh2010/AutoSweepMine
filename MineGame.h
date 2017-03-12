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
	UNKNOWN = 0, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, MINE, SAFE, FLAG
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

	// 雷区
	void GetMineMatrixArea(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixAreaRoughPosition(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixAreaFinePosition();
	MineStatus GetMineMatrixCellStatusByRowAndCol(int row, int col);
	void PreHandleMineCell(Mat & MineCellBitmap);
	int GetMineMatrixRows();
	int GetMineMatrixCols();
	vector<int> ExtractMineCellNumFeature(Mat MineNumBitmap);

	// 雷计数区
	void GetMineMatrixMineNumArea(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixMineNumAreaRoughPosition(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixMineNumAreaFinePosition();
	int MatchMineNum(Mat BitmapNum);
	void PreHandleMineNums(Mat & BitmapNum);
	int GetMineMatrixMineNums();

	// 脸区
	void GetMineMatrixFaceArea(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixFaceAreaRoughPosition(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray);
	void GetMineMatrixFaceAreaFinePosition();
	void PreHandleMineFaces(Mat & BitmapFace);
	Rect PreHandleMineFaces_FindBasicFaceArea(Mat & BitmapFace);
	tFaceStatus GetMineMatrixFaceStatus();

	void GetMineMatrixAndMineNumFaceAreaBitmapToFile();
	vector<vector<Point>> ExtractContoursForMineMatrixAreaBitmap();
	vector<double> GetContoursAreaArray(vector<vector<Point>> & contours);

	void DoAutoSweepMine();

	// 获取周围邻域信息
	int GetNearestCells(int row, int col, MineStatus status);
	void SetNearestAroundCellsFlagOrNot(int row, int col, bool isFlag);
	void SetNearestUnknownCellsSafe(int row, int col);
	void SetNearestUnknownCellsFlag(int row, int col);
	void UpdateMineMatrixBitmap();

	// 高级扫雷算法
	int GetMaxMineNumsInCommonNearestCells(int row, int col, int neighbor_row, int neighbor_col);
	int GetDiffNearestCells(int row, int col, int neighbor_row, int neighbor_col);
	void SetDiffNearestCellsFlagOrNot(int row, int col, int neighbor_row, int neighbor_col, bool isFlag);

	// 扫雷算法
	void BruteSearch();
	void ReadMemory();
#ifdef USINGUNITTEST
public:
#else
private:
#endif
	vector<vector<tMineCell>> MineMatrix;
	tSweepMineProgram SweepMineProgramInfo;
	tMineMatrix MineMatrixInfo;
	cv::Mat src_ori;
};

#endif
