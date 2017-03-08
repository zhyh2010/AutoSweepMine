#ifndef MINEGAME_H_
#define MINEGAME_H_

#define USINGUNITTEST

#include <vector>
#include <string>
#include <windows.h>

using std::vector;
using std::string;

enum MineStatus{
	UNKNOWN = 0, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, MINE
};

struct MineCell{
	int rows;
	int cols;
	MineStatus Mine;
};

struct SweepMineProgram{
	static string MineWinClass;
	static string MineWinName;
	HWND MineWinHandle;
	SweepMineProgram() :MineWinHandle(nullptr){}
};

class AutoSweepMine{
public:
	void FindMineProgram();

#ifdef USINGUNITTEST
public:
#else
private:
#endif
	vector<vector<MineCell>> MineMatrix;
	SweepMineProgram SweepMineProgramInfo;
};

#endif
