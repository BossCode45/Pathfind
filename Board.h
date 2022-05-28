#pragma once

#include <vector>
#include <array>

using std::vector;
using std::array;

enum SolveState
{
	GENERATING,
	SOLVING,
	FINISHED
};

struct Board
{
	int state;

	vector<vector<int>> boardValues;
	int eX; int eY; int sX; int sY; int boardX; int boardY;

	int cX; int cY;

	vector<vector<bool>> moved; vector<vector<bool>> discovered;

	vector<vector<vector<int>>> from; vector<vector<bool>> path;
};
