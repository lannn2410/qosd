#pragma once
#include "QoSalgorithm.h"

class IterativeGreedy : public QoSalgorithm
{
public:
	IterativeGreedy(Network * g);
	~IterativeGreedy();

	int getSolution();
	bool isCut(); // check whether all transactions are cut, if not, adding paths whose length < T into potential paths set

protected:
	vector<pair<int, vector<int>>> potentialPaths;
	void initiateEdges();

	// used for greedy algorithm with potentialPaths
	vector<pair<int, int>> listEdges;
	map<int, map<int, int>> mapPairNodes2EdgeId;
	vector<int> initialMarginalGain;
	int overallDelay = 0; // overall delay of potential paths
	vector<vector<int>> relatedPaths; // edge Id -> ids of potential path that contains this edge
	vector<vector<int>> paths; // path id -> list of edge ids on this path
	vector<int> pathLengths; // intial length (delay) of a potential path
	vector<map<int, int>> increaseDelay; // increase delay of path by edge:  path->edge->delay
	int potentialId = 0; // used to add edges' information without re-adding from beginning
};

