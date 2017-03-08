#include "..\MineGame.h"
#include "gtest\gtest.h"
#include "iostream"



class FindSweepMineTest : public testing::Test{
public:
	virtual void SetUp();
	virtual void TearDown();

public:
	AutoSweepMine autoSweepMine;
};

