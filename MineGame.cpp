#include "MineGame.h"
#include <exception>
#include <windows.h>
#include "utils/WriteToBMP.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

string tSweepMineProgram::MineWinClass = "扫雷";
string tSweepMineProgram::MineWinName = "扫雷";
int tSweepMineProgram::MineCellLen = 16;

#define _SHOWIMAGE

#ifdef _SHOWIMAGE
#define SHOWIMAGE(WINNAME, IMAGENAME){	\
	namedWindow(#WINNAME, CV_WINDOW_NORMAL); \
	imshow(#WINNAME, IMAGENAME);	\
	cvWaitKey();	\
}
#else
#define SHOWIMAGE(WINNAME, IMAGENAME)
#endif

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
	SHOWIMAGE(src_ori, src_ori);

	Mat src;
	cvtColor(src_ori, src, CV_RGB2GRAY);
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
		cout << result.at<float>(0, 0) << " in the " << i << "th position" << endl;
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
		cout << result.at<float>(0, 0) << " in the " << i << "th position" << endl;		
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
	Mat roi = src_ori(MineMatrixInfo.MineMatrixArea);
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
	Mat roi = src_ori(MineMatrixInfo.MineMatrixMineNumArea);
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

#define _SHOWIMAGE

#ifdef _SHOWIMAGE
#define SHOWIMAGE(WINNAME, IMAGENAME){	\
	namedWindow(#WINNAME, CV_WINDOW_NORMAL); \
	imshow(#WINNAME, IMAGENAME);	\
	cvWaitKey();	\
}
#else
#define SHOWIMAGE(WINNAME, IMAGENAME)
#endif

void AutoSweepMine::PreHandleMineFaces(Mat & BitmapFace_ori){
	Rect FaceArea = PreHandleMineFaces_FindBasicFaceArea(BitmapFace_ori);
	BitmapFace_ori = BitmapFace_ori(FaceArea);
}

MineStatus AutoSweepMine::GetMineMatrixCellStatusByRowAndCol(int row, int col){
	Rect MineCell(MineMatrixInfo.MineMatrixArea.x + SweepMineProgramInfo.MineCellLen * (col - 1),
		MineMatrixInfo.MineMatrixArea.y + SweepMineProgramInfo.MineCellLen * (row - 1),
		SweepMineProgramInfo.MineCellLen,
		SweepMineProgramInfo.MineCellLen);
	Mat src = src_ori.clone();
	rectangle(src, MineCell, Scalar(255, 255, 0), 2);
	SHOWIMAGE(MineCell, src);

	Mat roi = src(MineCell);
	SHOWIMAGE(roi, roi);
	


	return UNKNOWN;
}

void AutoSweepMine::DoAutoSweepMine(){
	try{
		FindMineProgram();
		GetMineMatrixAndMineNumFaceArea();
		GetMineMatrixAndMineNumFaceAreaBitmapToFile();
		GetMineMatrixInfo();

		while (1){
			GetMineMatrixCellStatusByRowAndCol(1, 2);
			GetMineMatrixAndMineNumFaceAreaBitmapToFile();
			src_ori = imread("tmp.bmp");
			//Sleep(1000);
			cout << "hello" << endl;
		}
	}
	catch (exception & e){
		cerr << e.what() << endl;
	}	
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
	Mat roi = src_ori(MineMatrixInfo.MineMatrixFaceArea);
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





