#pragma once
#include "..\MineGame.h"
#include "gtest\gtest.h"
#include "iostream"

class FindSweepMineMatrixParaTest : public testing::Test{
public:
	virtual void SetUp();
	virtual void TearDown();

public:
	AutoSweepMine autoSweepMine;
};