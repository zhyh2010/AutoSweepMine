#include "MineGame.h"
#include <exception>
#include <windows.h>
#include "utils/WriteToBMP.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>

using namespace cv;
using namespace std;

string tSweepMineProgram::MineWinClass = "扫雷";
string tSweepMineProgram::MineWinName = "扫雷";
int tSweepMineProgram::MineCellLen = 16;
int AutoSweepMine::stepId = 1;
pair<int, int> AutoSweepMine::Invaliddata = pair<int, int>(0, 0);

#define _TOSCREEN
#ifdef _TOSCREEN
#define mycout cout
#else
ofstream ofs("out.log");
#define mycout ofs
#endif

//#define _SHOWIMAGE

#ifdef _SHOWIMAGE
#define SHOWIMAGE(WINNAME, IMAGENAME){	\
	namedWindow(#WINNAME, CV_WINDOW_NORMAL); \
	imshow(#WINNAME, IMAGENAME);	\
	cvWaitKey();	\
}
#else
#define SHOWIMAGE(WINNAME, IMAGENAME)
#endif

inline void AutoSweepMine::LogEveryStepInfo(int row, int col, MineStatus status, int FlagAround, 
	MineStatus neighbour_status, int mines_in_common_cells, int diff_cells, bool isMines){
	mycout << "The " << stepId++ << "th :  position: ( " << row << ", " << col << " ) status: " << status << " FlagsAround: " << FlagAround
		<< " neigbours_info: status: " << neighbour_status << " mines_in_common_cells: " << mines_in_common_cells << " diff_cells: " << diff_cells
		<< " Set Around " << (isMines ? "Mines":"Safe") << endl;
}


inline void AutoSweepMine::LogEveryStepInfo(int row, int col, MineStatus status, int FlagAround, bool isMines){
	mycout << "The " << stepId++ << "th :  position: ( " << row << ", " << col << " ) status: " << status << " FlagsAround: " << FlagAround << " Set Around "
		<< (isMines ? "Mines":"Safe") << endl;
}


void AutoSweepMine::FindMineProgram(){
	HWND hwnd = FindWindow(SweepMineProgramInfo.MineWinClass.c_str(), 
		SweepMineProgramInfo.MineWinName.c_str());
	if (hwnd == nullptr){
		throw std::exception("没有找到扫雷程序!!!");
	}
	SweepMineProgramInfo.MineWinHandle = hwnd;
}

int AutoSweepMine::GetMineMatrixRows(){
	Mat src = src_ori.clone();
	Mat MineMatrixNum = src(MineMatrixInfo.MineMatrixArea);
	SHOWIMAGE(MineMatrixNum, MineMatrixNum);
	return MineMatrixInfo.MineMatrixArea.height / SweepMineProgramInfo.MineCellLen;
}

int AutoSweepMine::GetMineMatrixCols(){
	Mat src = src_ori.clone();
	Mat MineMatrixNum = src(MineMatrixInfo.MineMatrixArea);
	SHOWIMAGE(MineMatrixNum, MineMatrixNum);
	return MineMatrixInfo.MineMatrixArea.width / SweepMineProgramInfo.MineCellLen;
}

int AutoSweepMine::GetMineMatrixMineNums(){
	Mat src = src_ori.clone();
	Mat MineMatrixNum = src(MineMatrixInfo.MineMatrixMineNumArea);
	SHOWIMAGE(MineMatrixNum, MineMatrixNum);

	Mat firstNum = MineMatrixNum.colRange(0, MineMatrixInfo.MineMatrixMineNumArea.width / 3);
	SHOWIMAGE(firstNum, firstNum);	
	int FirstNum = MatchMineNum(firstNum);

	Mat secondNum = MineMatrixNum.colRange(MineMatrixInfo.MineMatrixMineNumArea.width / 3, 
		MineMatrixInfo.MineMatrixMineNumArea.width * 2 / 3);
	SHOWIMAGE(secondNum, secondNum);
	int SecondNum = MatchMineNum(secondNum);

	Mat thirdNum = MineMatrixNum.colRange(MineMatrixInfo.MineMatrixMineNumArea.width * 2 / 3,
		MineMatrixInfo.MineMatrixMineNumArea.width);
	SHOWIMAGE(thirdNum, thirdNum);
	int ThirdNum = MatchMineNum(thirdNum);

	return FirstNum * 100 + SecondNum * 10 + ThirdNum;
}

// 找到第二大的面积区域就是雷区
void AutoSweepMine::GetMineMatrixArea(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray){		
	GetMineMatrixAreaRoughPosition(contours, ContoursAreaArray);
	GetMineMatrixAreaFinePosition();
}

// 面积相同区域中左边那个部分的是雷数区域， 右边是时间区域
void AutoSweepMine::GetMineMatrixMineNumArea(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray){	
	GetMineMatrixMineNumAreaRoughPosition(contours, ContoursAreaArray);
	GetMineMatrixMineNumAreaFinePosition();
}

// 面积最小的是face区域
void AutoSweepMine::GetMineMatrixFaceArea(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray){	
	GetMineMatrixFaceAreaRoughPosition(contours, ContoursAreaArray);
	GetMineMatrixFaceAreaFinePosition();
}

void AutoSweepMine::GetMineMatrixAndMineNumFaceAreaBitmapToFile(){
	//HDC hDC = ::GetDC(GetDesktopWindow());		// 以窗口左上角开始进行绘制
	HDC hDC = ::GetDC(SweepMineProgramInfo.MineWinHandle);    // 以窗口句柄为基准进行绘制
	bool ret = WriteBmp("tmp.bmp", hDC, MineMatrixInfo.MineMatrixAndMineNumFaceArea);
	::ReleaseDC(SweepMineProgramInfo.MineWinHandle, hDC);
	if (!ret){
		throw std::exception("截取扫雷窗口区域图形失败！！！");
	}
}

// 默认分割出来有 雷区， face区， 雷数目区，时间区， 整体范围区 5分部分轮廓
vector<vector<Point>> AutoSweepMine::ExtractContoursForMineMatrixAreaBitmap(){
	src_ori = imread("tmp.bmp");
	if (src_ori.empty()){
		throw exception("读取扫雷图片失败！！！");
	}
	Mat src = src_ori.clone();
	SHOWIMAGE(src, src);

	cvtColor(src, src, CV_RGB2GRAY);
	threshold(src, src, 0, 255, cv::THRESH_OTSU);
	SHOWIMAGE(bin, src);

	morphologyEx(src, src, MORPH_CLOSE, Mat());
	SHOWIMAGE(close, src); 

	vector<vector<Point>> contours;
	findContours(src, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	for (unsigned int i = 0; i < contours.size(); i++){
		Mat res(src.size(), CV_8U, Scalar(255));
		drawContours(res, vector<vector<Point>>{contours[i]}, -1, Scalar(0, 0, 0), 3);
		SHOWIMAGE(contours, res);
	}

	if (contours.size() != 5){
		throw exception("提取扫雷区域异常，请检查！！！");
	}

	return contours;
}

vector<double> AutoSweepMine::GetContoursAreaArray(vector<vector<Point>> & contours){
	vector<double> ContoursAreaArray;
	for (unsigned int i = 0; i < contours.size(); i++){
		ContoursAreaArray.push_back(boundingRect(contours[i]).area());
	}
	return ContoursAreaArray;
}

void AutoSweepMine::GetMineMatrixAndMineNumFaceArea(){
	RECT rect;
	//bool ret = GetWindowRect(SweepMineProgramInfo.MineWinHandle, &rect);
	bool ret = GetClientRect(SweepMineProgramInfo.MineWinHandle, &rect);
	if (!ret){
		throw std::exception("获取扫雷窗口区域失败！！！");
	}
	MineMatrixInfo.MineMatrixAndMineNumFaceArea = rect;
}

// 根据归一化相关系数法则进行匹配
int AutoSweepMine::MatchMineNum(Mat BitmapNum){
	PreHandleMineNums(BitmapNum);
	vector<string> MineNumBitmapPath{ "../imgs/00.bmp", "../imgs/11.bmp", "../imgs/22.bmp", "../imgs/33.bmp", "../imgs/44.bmp",
		"../imgs/55.bmp", "../imgs/66.bmp", "../imgs/77.bmp", "../imgs/88.bmp", "../imgs/99.bmp"
	};
	int MaxValueId = 0;
	double MaxValue = 0;
	for (int i = 0; i < MineNumBitmapPath.size(); i++){
		Mat standard = imread(MineNumBitmapPath[i]);
		PreHandleMineNums(standard);
		resize(standard, standard, BitmapNum.size());
		Mat result(1, 1, CV_32FC1);
		matchTemplate(BitmapNum, standard, result, TM_CCORR_NORMED);
		//mycout << result.at<float>(0, 0) << " in the " << i << "th position" << endl;
		if (result.at<float>(0, 0) > MaxValue){
			MaxValue = result.at<float>(0, 0);
			MaxValueId = i;
		}
	}
	return MaxValueId;
}

void AutoSweepMine::PreHandleMineNums(Mat & BitmapNum){	
	SHOWIMAGE(BitmapNum, BitmapNum);
	threshold(BitmapNum, BitmapNum, 200, 255, CV_THRESH_BINARY);
	SHOWIMAGE(BitmapNum, BitmapNum);
	morphologyEx(BitmapNum, BitmapNum, MORPH_CLOSE, Mat());
	SHOWIMAGE(close, BitmapNum);
}

void AutoSweepMine::GetMineMatrixInfo(){
	auto contours = ExtractContoursForMineMatrixAreaBitmap();
	auto ContoursAreaArray = GetContoursAreaArray(contours);
	GetMineMatrixMineNumArea(contours, ContoursAreaArray);
	GetMineMatrixArea(contours, ContoursAreaArray);
	GetMineMatrixFaceArea(contours, ContoursAreaArray);

	MineMatrixInfo.Rows = GetMineMatrixRows();
	MineMatrixInfo.Cols = GetMineMatrixCols();
	MineMatrixInfo.MineNums = GetMineMatrixMineNums();	
	MineMatrixInfo.FaceStatus = GetMineMatrixFaceStatus();
}

tFaceStatus AutoSweepMine::GetMineMatrixFaceStatus(){
	Mat src = src_ori.clone();
	Mat MineMatrixFaceArea = src(MineMatrixInfo.MineMatrixFaceArea);
	SHOWIMAGE(MineMatrixFaceArea, MineMatrixFaceArea);

	PreHandleMineFaces(MineMatrixFaceArea);
	SHOWIMAGE(MineMatrixFaceArea, MineMatrixFaceArea);
	
	vector<string> FaceVector{ "../imgs/continue.bmp", "../imgs/success.bmp", "../imgs/failure.bmp" };

	int MaxValueId = 0;
	double MaxValue = 0;
	for (unsigned int i = 0; i < FaceVector.size(); i++){
		Mat standard = imread(FaceVector[i]);	
		SHOWIMAGE(standard, standard);

		PreHandleMineFaces(standard);
		
		SHOWIMAGE(standard, standard);
		resize(standard, standard, MineMatrixFaceArea.size());
		SHOWIMAGE(standard, standard);
		Mat result(1, 1, CV_32FC1);
		matchTemplate(MineMatrixFaceArea, standard, result, TM_CCORR_NORMED);
		//mycout << result.at<float>(0, 0) << " in the " << i << "th position" << endl;		
		if (result.at<float>(0, 0) > MaxValue){
			MaxValue = result.at<float>(0, 0);
			MaxValueId = i;
		}
	}

	return tFaceStatus(MaxValueId);
}

void AutoSweepMine::GetMineMatrixAreaRoughPosition(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray){
	ContoursAreaArray.push_back(0);  // 哨兵
	unsigned int max_area_id = ContoursAreaArray.size() - 1, second_max_area_id = ContoursAreaArray.size() - 1;
	for (unsigned int i = 0; i < ContoursAreaArray.size(); i++){
		if (ContoursAreaArray[i] > ContoursAreaArray[max_area_id])
			max_area_id = i;
		else if (ContoursAreaArray[i] > ContoursAreaArray[second_max_area_id])
			second_max_area_id = i;
	}
	ContoursAreaArray.pop_back();
	MineMatrixInfo.MineMatrixArea = boundingRect(contours[second_max_area_id]);
}

// 这里精细寻找时候的轮廓实际上是在粗匹配基准上得到，所以应该加入粗匹配时候的基准
void AutoSweepMine::GetMineMatrixAreaFinePosition(){
	Mat src = src_ori.clone();
	Mat roi = src(MineMatrixInfo.MineMatrixArea);
	SHOWIMAGE(roi, roi);

	threshold(roi, roi, 200, 255, CV_THRESH_BINARY);
	SHOWIMAGE(roi, roi);

	morphologyEx(roi, roi, MORPH_CLOSE, Mat());
	SHOWIMAGE(roi, roi);

	cvtColor(roi, roi, CV_RGB2GRAY);
	SHOWIMAGE(roi, roi);

	vector<vector<Point>> myContours;
	findContours(roi, myContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	int MaxContoursId = 0;
	double MaxContoursArea = 0;
	for (size_t i = 0; i < myContours.size(); i++){
		if (boundingRect(myContours[i]).area() > MaxContoursArea){
			MaxContoursArea = boundingRect(myContours[i]).area();
			MaxContoursId = i;
		}
	}
	Rect rect = boundingRect(myContours[MaxContoursId]);
	rect.x += MineMatrixInfo.MineMatrixArea.x;
	rect.y += MineMatrixInfo.MineMatrixArea.y;
	MineMatrixInfo.MineMatrixArea = rect;
}

void AutoSweepMine::GetMineMatrixMineNumAreaRoughPosition(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray){
	unsigned int exist_id = 0, same_exist_id = 0;
	for (unsigned int i = 0; i < ContoursAreaArray.size(); i++){
		for (unsigned int j = 0; j < i; j++){
			if (ContoursAreaArray[j] == ContoursAreaArray[i]){
				exist_id = i;
				same_exist_id = j;
			}
		}
	}

	if (boundingRect(contours[exist_id]).x > boundingRect(contours[same_exist_id]).x)
		exist_id = same_exist_id;
	MineMatrixInfo.MineMatrixMineNumArea = boundingRect(contours[exist_id]);
}

void AutoSweepMine::GetMineMatrixMineNumAreaFinePosition(){
	Mat src = src_ori.clone();
	Mat roi = src(MineMatrixInfo.MineMatrixMineNumArea);
	SHOWIMAGE(roi, roi);

	threshold(roi, roi, 200, 255, CV_THRESH_BINARY);
	SHOWIMAGE(roi, roi);

	cvtColor(roi, roi, CV_RGB2GRAY);
	SHOWIMAGE(roi, roi);

	threshold(roi, roi, 128, 0, CV_THRESH_TOZERO_INV);
	SHOWIMAGE(roi, roi);

	morphologyEx(roi, roi, MORPH_CLOSE, Mat());
	SHOWIMAGE(roi, roi);

	vector<vector<Point>> myContours;
	findContours(roi, myContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	int MaxContoursId = 0;
	double MaxContoursArea = 0;
	for (size_t i = 0; i < myContours.size(); i++){
		if (boundingRect(myContours[i]).area() > MaxContoursArea){
			MaxContoursArea = boundingRect(myContours[i]).area();
			MaxContoursId = i;
		}
	}
	Rect rect = boundingRect(myContours[MaxContoursId]);
	rect.x += MineMatrixInfo.MineMatrixMineNumArea.x;
	rect.y += MineMatrixInfo.MineMatrixMineNumArea.y;
	MineMatrixInfo.MineMatrixMineNumArea = rect;
}

void AutoSweepMine::GetMineMatrixFaceAreaRoughPosition(vector<vector<Point>> & contours, vector<double> & ContoursAreaArray){
	ContoursAreaArray.push_back(INT_MAX);
	unsigned int min_area_id = ContoursAreaArray.size() - 1;
	for (unsigned int i = 0; i < ContoursAreaArray.size(); i++){
		if (ContoursAreaArray[i] < ContoursAreaArray[min_area_id])
			min_area_id = i;
	}
	ContoursAreaArray.pop_back();
	MineMatrixInfo.MineMatrixFaceArea = boundingRect(contours[min_area_id]);
}

void AutoSweepMine::GetMineMatrixFaceAreaFinePosition(){
	Mat src = src_ori.clone();
	Mat roi = src(MineMatrixInfo.MineMatrixFaceArea);
	Rect FaceArea = PreHandleMineFaces_FindBasicFaceArea(roi);
	FaceArea.x += MineMatrixInfo.MineMatrixFaceArea.x;
	FaceArea.y += MineMatrixInfo.MineMatrixFaceArea.y;
	MineMatrixInfo.MineMatrixFaceArea = FaceArea;
}

Rect AutoSweepMine::PreHandleMineFaces_FindBasicFaceArea(Mat & BitmapFace_ori){
	Mat BitmapFace = BitmapFace_ori.clone();
	SHOWIMAGE(BitmapFace, BitmapFace);
	threshold(BitmapFace, BitmapFace, 200, 255, CV_THRESH_BINARY);
	SHOWIMAGE(BitmapFace, BitmapFace);
	cvtColor(BitmapFace, BitmapFace, CV_RGB2GRAY);
	SHOWIMAGE(BitmapFace, BitmapFace);
	threshold(BitmapFace, BitmapFace, 200, 0, CV_THRESH_TOZERO_INV);
	SHOWIMAGE(BitmapFace, BitmapFace);
	morphologyEx(BitmapFace, BitmapFace, MORPH_CLOSE, Mat());
	SHOWIMAGE(BitmapFace, BitmapFace);

	vector<vector<Point>> myContours;
	findContours(BitmapFace, myContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	int MaxContoursId = 0;
	double MaxContoursArea = 0;
	for (size_t i = 0; i < myContours.size(); i++){
		if (boundingRect(myContours[i]).area() > MaxContoursArea){
			MaxContoursArea = boundingRect(myContours[i]).area();
			MaxContoursId = i;
		}
	}
	return boundingRect(myContours[MaxContoursId]);
}

void AutoSweepMine::PreHandleMineFaces(Mat & BitmapFace_ori){
	Rect FaceArea = PreHandleMineFaces_FindBasicFaceArea(BitmapFace_ori);
	BitmapFace_ori = BitmapFace_ori(FaceArea);
}

void AutoSweepMine::PreHandleMineCell(Mat & MineCellBitmap){
	Mat background = imread("../imgs/safe.bmp");
	SHOWIMAGE(background, background);

	absdiff(MineCellBitmap, background, MineCellBitmap);
	SHOWIMAGE(MineCellBitmap, MineCellBitmap);


	cvtColor(MineCellBitmap, MineCellBitmap, CV_RGB2GRAY);
	//SHOWIMAGE(MineCellBitmap, MineCellBitmap);
	threshold(MineCellBitmap, MineCellBitmap, 50, 255, CV_THRESH_BINARY);
	SHOWIMAGE(MineCellBitmap, MineCellBitmap);
}

MineStatus AutoSweepMine::GetMineMatrixCellStatusByRowAndCol(int row, int col){	
	Rect MineCell(MineMatrixInfo.MineMatrixArea.x + SweepMineProgramInfo.MineCellLen * (col - 1),
		MineMatrixInfo.MineMatrixArea.y + SweepMineProgramInfo.MineCellLen * (row - 1),
		SweepMineProgramInfo.MineCellLen,
		SweepMineProgramInfo.MineCellLen);
	Mat src = src_ori.clone();

	Mat roi = src(MineCell);
	SHOWIMAGE(roi, roi);

	auto tmp1 = ExtractMineCellNumFeature(roi);
	static vector<vector<int>> FeatureArray{
		vector<int>{0, 54, 148, 54, 0, 54, 148, 54, 0, 54, 148, 54},
		vector<int>{0, 31, 185, 40, 40, 31, 185, 0, 40, 31, 185, 0},
		vector<int>{65, 31, 160, 0, 0, 96, 160, 0, 65, 31, 160, 0},
		vector<int>{62, 31, 163, 0, 62, 31, 163, 0, 0, 31, 163, 62},
		vector<int>{0, 87, 169, 0, 56, 31, 169, 0, 56, 31, 169, 0},
		vector<int>{70, 31, 155, 0, 70, 31, 155, 0, 0, 101, 155, 0},
		vector<int>{0, 103, 153, 0, 0, 103, 153, 0, 72, 31, 153, 0},
		vector<int>{44, 31, 181, 0, 44, 31, 181, 0, 44, 31, 181, 0},
		vector<int>{0, 107, 149, 0, 0, 107, 149, 0, 0, 107, 149, 0},
		vector<int>{221, 31, 0, 4, 221, 31, 0, 4, 77, 31, 0, 148},
		vector<int>{0, 31, 225, 0, 0, 31, 225, 0, 0, 31, 225, 0},
		vector<int>{39, 54, 109, 54, 39, 54, 109, 54, 22, 54, 109, 71},
		vector<int>{0, 54, 148, 54, 0, 54, 148, 54, 0, 54, 148, 54}
	};

	int matchId = int(MINE);
	for (unsigned int i = 0; i < FeatureArray.size(); i++){
		if (FeatureArray[i] == tmp1){
			matchId = i;
			break;
		}
			
	}
		
	return MineStatus(matchId);
}

void AutoSweepMine::DoAutoSweepMine(){
	try{
		FindMineProgram();
		GetMineMatrixAndMineNumFaceArea();
		GetMineMatrixAndMineNumFaceAreaBitmapToFile();
		GetMineMatrixInfo();
	
		BruteSearchWithVector();
	}
	catch (exception & e){
		cerr << e.what() << endl;	
	}	
}

int AutoSweepMine::GetNearestCells(int row, int col, MineStatus status){
	int count = 0;
	for (int i = max(1, row - 1); i <= min(MineMatrixInfo.Rows, row + 1); i++){
		for (int j = max(1, col - 1); j <= min(MineMatrixInfo.Cols, col + 1); j++){
			if (!(i == row && j == col) && (status == GetMineMatrixCellStatusByRowAndCol(i, j))){
				count++;
			}
		}
	}
	return count;
}

void AutoSweepMine::SetNearestUnknownCellsSafe(int row, int col){
	Point2d ClickPt(MineMatrixInfo.MineMatrixArea.x + SweepMineProgramInfo.MineCellLen * (col - 1 + 0.5),
		MineMatrixInfo.MineMatrixArea.y + SweepMineProgramInfo.MineCellLen * (row - 1 + 0.5));
	SendMessage(SweepMineProgramInfo.MineWinHandle, WM_LBUTTONDOWN, 0, MAKELONG(ClickPt.x, ClickPt.y));
	SendMessage(SweepMineProgramInfo.MineWinHandle, WM_LBUTTONUP, 0, MAKELONG(ClickPt.x, ClickPt.y));
}

void AutoSweepMine::SetNearestUnknownCellsFlag(int row, int col){
	Point2d ClickPt(MineMatrixInfo.MineMatrixArea.x + SweepMineProgramInfo.MineCellLen * (col - 1 + 0.5),
		MineMatrixInfo.MineMatrixArea.y + SweepMineProgramInfo.MineCellLen * (row - 1 + 0.5));
	SendMessage(SweepMineProgramInfo.MineWinHandle, WM_RBUTTONDOWN, 0, MAKELONG(ClickPt.x, ClickPt.y));
	SendMessage(SweepMineProgramInfo.MineWinHandle, WM_RBUTTONUP, 0, MAKELONG(ClickPt.x, ClickPt.y));
}

void AutoSweepMine::UpdateMineMatrixBitmap(){
	GetMineMatrixAndMineNumFaceAreaBitmapToFile();
	src_ori = imread("tmp.bmp");
}

void AutoSweepMine::SetNearestAroundCellsFlagOrNot(int row, int col, bool isFlag){
	for (int i = max(1, row - 1); i <= min(MineMatrixInfo.Rows, row + 1); i++){
		for (int j = max(1, col - 1); j <= min(MineMatrixInfo.Cols, col + 1); j++){
			if (!(i == row && j == col) && (UNKNOWN == GetMineMatrixCellStatusByRowAndCol(i, j))){
				if (isFlag)
					SetNearestUnknownCellsFlag(i, j);
				else
					SetNearestUnknownCellsSafe(i, j);
			}
		}
	}
	UpdateMineMatrixBitmap();
}

void AutoSweepMine::BruteSearch(){
	while (1){
		bool IsGetNext = false;
		for (unsigned int i = 1; i <= MineMatrixInfo.Rows; i++){
			for (unsigned int j = 1; j <= MineMatrixInfo.Cols; j++){
				IsGetNext = OperateByMatrixMineCells(i, j) || IsGetNext;
			}
		}

		if (!IsGetNext){			// random click
			SetNearestUnknownCellsSafe(lastUnknownPos.first, lastUnknownPos.second);
			UpdateMineMatrixBitmap();
			mycout << "gamble in position: " << lastUnknownPos.first << " and " << lastUnknownPos.second << endl;
		}

		if (GetMineMatrixFaceStatus() == SUCCESS){
			mycout << "congrutations!!!" << endl;
			break;
		}
	}
}

// 12 维度向量， 分 RGB 三个通道， 每个通道分别记录 0， 128， 192， 255 的个数
vector<int> AutoSweepMine::ExtractMineCellNumFeature(Mat MineNumBitmap){
	vector<int> FeatureMineNum(12, 0);
	for (unsigned int i = 0; i < MineNumBitmap.rows; i++){
		for (unsigned int j = 0; j < MineNumBitmap.cols; j++){
			Vec3b vec = MineNumBitmap.at<Vec3b>(i, j);
			for (unsigned int k = 0; k < 3; k++){
				FeatureMineNum[0 + k * 4] += (vec[k] == 0);
				FeatureMineNum[1 + k * 4] += (vec[k] == 128);
				FeatureMineNum[2 + k * 4] += (vec[k] == 192);
				FeatureMineNum[3 + k * 4] += (vec[k] == 255);
			}			
		}
	}
	return FeatureMineNum;
}

int AutoSweepMine::GetMaxMineNumsInCommonNearestCells(int row, int col, int neighbor_row, int neighbor_col){
	int common_cell_num = 0;
	for (int i = max(1, row - 1); i <= min(MineMatrixInfo.Rows, row + 1); i++){
		for (int j = max(1, col - 1); j <= min(MineMatrixInfo.Cols, col + 1); j++){
			MineStatus status = GetMineMatrixCellStatusByRowAndCol(i, j);
			if (!(i == row && j == col) && (UNKNOWN == status)){
				if (abs(neighbor_row - i) <= 1 && abs(neighbor_col - j) <= 1){
					common_cell_num++;
				}
			}
		}
	}
	int neighbour_MinesAll = GetMineMatrixCellStatusByRowAndCol(neighbor_row, neighbor_col);
	int neighbour_FlagsAll = GetNearestCells(neighbor_row, neighbor_col, FLAG);
	int MinesAll = GetMineMatrixCellStatusByRowAndCol(row, col);
	int FlagsAll = GetNearestCells(row, col, FLAG);
	return min(min(MinesAll - FlagsAll, neighbour_MinesAll - neighbour_FlagsAll), common_cell_num);
}

int AutoSweepMine::GetDiffNearestCells(int row, int col, int neighbor_row, int neighbor_col){
	int diff_cell_num = 0;
	for (int i = max(1, row - 1); i <= min(MineMatrixInfo.Rows, row + 1); i++){
		for (int j = max(1, col - 1); j <= min(MineMatrixInfo.Cols, col + 1); j++){
			MineStatus status = GetMineMatrixCellStatusByRowAndCol(i, j);
			if (!(i == row && j == col) && (UNKNOWN == status)){
				if (!(abs(neighbor_row - i) <= 1 && abs(neighbor_col - j) <= 1)){
					diff_cell_num++;
				}
			}
		}
	}
	return diff_cell_num;
}

void AutoSweepMine::SetDiffNearestCellsFlagOrNot(int row, int col, int neighbor_row, int neighbor_col, bool isFlag){
	for (int i = max(1, row - 1); i <= min(MineMatrixInfo.Rows, row + 1); i++){
		for (int j = max(1, col - 1); j <= min(MineMatrixInfo.Cols, col + 1); j++){
			MineStatus status = GetMineMatrixCellStatusByRowAndCol(i, j);
			if (!(i == row && j == col) && (UNKNOWN == status)){
				if (!(abs(neighbor_row - i) <= 1 && abs(neighbor_col - j) <= 1)){
					if (!isFlag)
						SetNearestUnknownCellsSafe(i, j);
					else
						SetNearestUnknownCellsFlag(i, j);
				}
			}
		}
	}
	UpdateMineMatrixBitmap();
}

// 通过分析得到扫雷内存布局为： 
// 10 10 10 10 . . . . 10
// 10 xx xx xx . . . . 10
// 10 xx 8f xx . . . . 10
// .  .  .  .  . . . . .
// .  .  .  .  . . . . .
// .  .  .  .  . . . . .
// 10 10 10 10 . . . . 10
// 其中， 8f 表示雷的位置
void AutoSweepMine::ReadMemory(){
	DWORD processId;
	GetWindowThreadProcessId(SweepMineProgramInfo.MineWinHandle, &processId); 
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	int baseAddress = 0x01005340;
	byte * MineMatrixMemory = new byte[32 * (MineMatrixInfo.Rows + 2)];

	size_t dwNumberOfBytesRead = 0;
	size_t toRead = 32 * (MineMatrixInfo.Rows + 2);
	size_t hasRead = 0;
	while (dwNumberOfBytesRead < toRead){
		ReadProcessMemory(hProcess, (LPCVOID)(baseAddress + hasRead), MineMatrixMemory + hasRead, toRead, &dwNumberOfBytesRead);
		toRead -= dwNumberOfBytesRead;
		hasRead += dwNumberOfBytesRead;
	}
	
	for (int i = 1; i < MineMatrixInfo.Rows + 2; i++){
		for (int j = 1; j < MineMatrixInfo.Cols + 2; j++){
			if (MineMatrixMemory[i * 32 + j] == 0x8f){
				// lei				
				SetNearestUnknownCellsFlag(i, j);
			}
			else{
				SetNearestUnknownCellsSafe(i, j);
			}
		}
	}

	delete [] MineMatrixMemory;
}

void AutoSweepMine::ResetGame(){
	Point2d ClickPt(MineMatrixInfo.MineMatrixFaceArea.x + MineMatrixInfo.MineMatrixFaceArea.width / 2,
		MineMatrixInfo.MineMatrixFaceArea.y + MineMatrixInfo.MineMatrixFaceArea.height / 2);
	SendMessage(SweepMineProgramInfo.MineWinHandle, WM_LBUTTONDOWN, 0, MAKELONG(ClickPt.x, ClickPt.y));
	SendMessage(SweepMineProgramInfo.MineWinHandle, WM_LBUTTONUP, 0, MAKELONG(ClickPt.x, ClickPt.y));
	UpdateMineMatrixBitmap();
	ResetSearchPath();
}

// 高级扫雷策略	
// 考虑与当前位置相邻的点，如果他们之间的差分区域， 记：
//			remain_mines =  当前区域所有雷数 - 标记数 flag - 公共区域包含的雷数
// 如果：
//  1. remain_mines == 0， 此时所有的未知差分区域安全
//  2. remain_mines == 未知差分区域数目， 查分区域全部都是雷
bool AutoSweepMine::AdvanceSearchAlgorithm(int row, int col, MineStatus status){	
	bool hasGetNext = false;	
	vector<pair<int, int>> search_direction{ pair<int, int>(-1, 0), pair<int, int>(1, 0),
		pair<int, int>(0, -1), pair<int, int>(0, 1) };
	for (unsigned int ii = 0; ii < 4; ii++){
		int cur_row = row + search_direction[ii].first;
		int cur_col = col + search_direction[ii].second;
		if (cur_row < 1 || cur_row > MineMatrixInfo.Rows || cur_col < 1 || cur_col > MineMatrixInfo.Cols)
			continue;

		hasGetNext = AdvanceSearchAlgorithm_JudgeWithNeighbours(row, col, status, cur_row, cur_col) || hasGetNext;
	}
	return hasGetNext;
}

bool AutoSweepMine::OperateByMatrixMineCells(int row, int col){
	MineStatus status = GetMineMatrixCellStatusByRowAndCol(row, col);
	if (status == UNKNOWN){
		lastUnknownPos = pair<int, int>(row, col);
		return false;
	}
	else if (status == MINE){
		ResetGame();
		return false;
	}
	
	RemoveFromSearchPath(row, col);							// 除了 unknown 单元外， 遍历到之后都应该标记为已找到
	if (static_cast<int>(status) >= 1 && static_cast<int>(status) <= 8){		
		int UnknownAround = GetNearestCells(row, col, UNKNOWN);
		if (UnknownAround == 0)
			return false;

		int FlagAround = GetNearestCells(row, col, FLAG);
		if (UnknownAround == static_cast<int>(status)-FlagAround){						// 未知处都是 雷
			LogEveryStepInfo(row, col, status, FlagAround, true);
			SetNearestAroundCellsFlagOrNot(row, col, true);
			return true;
		}
		else if (static_cast<int>(status)-FlagAround == 0){						// safe
			LogEveryStepInfo(row, col, status, FlagAround, false);
			SetNearestAroundCellsFlagOrNot(row, col, false);
			return true;
		}
		else{
			return AdvanceSearchAlgorithm(row, col, status);
		}
	}
	return false;
}

bool AutoSweepMine::AdvanceSearchAlgorithm_JudgeWithNeighbours(int row, int col, MineStatus status, int cur_row, int cur_col){
	MineStatus neighbour_status = GetMineMatrixCellStatusByRowAndCol(cur_row, cur_col);
	int FlagAround = GetNearestCells(row, col, FLAG);
	if (static_cast<int>(neighbour_status) >= 1 && static_cast<int>(neighbour_status) <= 8){			// 正常雷数范围
		int mines_in_common_cells = GetMaxMineNumsInCommonNearestCells(row, col, cur_row, cur_col);
		int diff_cells = GetDiffNearestCells(row, col, cur_row, cur_col);
		if (diff_cells == 0)
			return false;

		if (GetDiffNearestCells(cur_row, cur_col, row, col) != 0)
			return false;

		if (diff_cells == static_cast<int>(status)-FlagAround - mines_in_common_cells){				// 未知处都是 雷
			LogEveryStepInfo(row, col, status, FlagAround, neighbour_status, mines_in_common_cells, diff_cells, true);
			SetDiffNearestCellsFlagOrNot(row, col, cur_row, cur_col, true);
			return true;
		}
		else if (static_cast<int>(status)-FlagAround - mines_in_common_cells == 0){				// safe
			LogEveryStepInfo(row, col, status, FlagAround, neighbour_status, mines_in_common_cells, diff_cells, false);
			SetDiffNearestCellsFlagOrNot(row, col, cur_row, cur_col, false);
			return true;
		}

		FlagAround = GetNearestCells(row, col, FLAG);   // 标记雷区之后， 这个FlagAround 需要同步更新
	}
	return false;
}

void AutoSweepMine::BruteSearchWithVector(){
	ResetSearchPath();
	while (1){
		bool IsGetNext = false;
		for (unsigned int i = 0; i < LocalSearchPath.size(); i++){
			if (LocalSearchPath[i] == Invaliddata)
				continue;
			IsGetNext = OperateByMatrixMineCells(LocalSearchPath[i].first, LocalSearchPath[i].second) || IsGetNext;
		}

		if (!IsGetNext){			// random click
			SetNearestUnknownCellsSafe(lastUnknownPos.first, lastUnknownPos.second);
			UpdateMineMatrixBitmap();
			mycout << "gamble in position: " << lastUnknownPos.first << " and " << lastUnknownPos.second << endl;
		}

		if (GetMineMatrixFaceStatus() == SUCCESS){
			mycout << "congrutations!!!" << endl;
			break;
		}
	}
}

void AutoSweepMine::ResetSearchPath(){
	for (unsigned int i = 1; i <= MineMatrixInfo.Rows; i++){
		for (unsigned int j = 1; j <= MineMatrixInfo.Cols; j++){
			LocalSearchPath.push_back(pair<int, int>(i, j));
		}
	}
}

void AutoSweepMine::RemoveFromSearchPath(int row, int col){
	auto iter = find(LocalSearchPath.begin(), LocalSearchPath.end(), pair<int, int>(row, col));
	if (iter != LocalSearchPath.end()){
		*iter = Invaliddata;
	}
}

void AutoSweepMine::DoAutoSweepMine_WithMemory(){
	try{
		FindMineProgram();
		GetMineMatrixAndMineNumFaceArea();
		GetMineMatrixAndMineNumFaceAreaBitmapToFile();
		GetMineMatrixInfo();

		ReadMemory();
	}
	catch (exception & e){
		cerr << e.what() << endl;
	}
}











