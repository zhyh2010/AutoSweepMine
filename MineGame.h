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
	UNKNOWN = 0, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, MINE, SAFE, FLAG, INVALID
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
	void DoAutoSweepMine();
	void DoAutoSweepMine_WithMemory();

#ifdef USINGUNITTEST
public:
#else
private:
#endif
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
	void ResetGame();

	void GetMineMatrixAndMineNumFaceAreaBitmapToFile();
	vector<vector<Point>> ExtractContoursForMineMatrixAreaBitmap();
	vector<double> GetContoursAreaArray(vector<vector<Point>> & contours);

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
	bool AdvanceSearchAlgorithm(int row, int col, MineStatus status);
	bool AdvanceSearchAlgorithm_JudgeWithNeighbours(int row, int col, MineStatus status, int cur_row, int cur_col);
	bool OperateByMatrixMineCells(int row, int col);
	pair<int, int> lastUnknownPos;
	void ResetSearchPath();
	vector<tMineCell> LocalSearchPath;
	void SetMineCellStatus(int row, int col, MineStatus status);
	int UnknownNums;

	// 扫雷算法
	void BruteSearch();
	void BruteSearchWithVector();
	void ReadMemory();
	void GambleStratry();

	// 日志功能
	void LogEveryStepInfo(int row, int col, MineStatus status, int FlagAround, MineStatus neighbour_status, int mines_in_common_cells, int diff_cells, bool isMines);
	void LogEveryStepInfo(int row, int col, MineStatus status, int FlagAround, bool isMines);
	static int stepId;

	tSweepMineProgram SweepMineProgramInfo;
	tMineMatrix MineMatrixInfo;
	cv::Mat src_ori;
};

#endif
