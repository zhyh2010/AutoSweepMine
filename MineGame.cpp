#include "MineGame.h"
#include <exception>

string SweepMineProgram::MineWinClass = "É¨À×";
string SweepMineProgram::MineWinName = "É¨À×";

void AutoSweepMine::FindMineProgram(){
	HWND hwnd = FindWindow(SweepMineProgramInfo.MineWinClass.c_str(), 
		SweepMineProgramInfo.MineWinName.c_str());
	if (hwnd == nullptr){
		throw std::exception("Ã»ÓÐÕÒµ½É¨À×³ÌÐò!!!");
	}
	SweepMineProgramInfo.MineWinHandle = hwnd;
}








