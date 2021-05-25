#pragma once
#include "IterativeGreedy.h"

class TrunkAdding : public IterativeGreedy
{
public:
	TrunkAdding(Network * g);
	~TrunkAdding();

	int getSolution();

private:
	void initiateEdges();
	vector<map<int, vector<int>>> increaseDelay; // increase delay of path by edge:  path->edge->level->delay
	vector<vector<int>> initialOverallDelay; // edge->level->delay

	vector<int> heapIndex;
	vector<vector<int>> edgeLevel2HeapIdx; // edge -> heap Indices
	vector<pair<int, int>> heapIdx2edgeLevel; // heapIdx -> edge,level
	vector<double> heapData;
};

