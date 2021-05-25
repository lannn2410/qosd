#pragma once
#include "QoSalgorithm.h"

class HopIncremental : public QoSalgorithm
{
public:
	HopIncremental(Network * g);
	~HopIncremental();

	int getSolution();
	void setFeasibleSol(int x);
private:
	bool isCutWithHopsLimit(int hops);
	unsigned long long getNumberOfSample(int hops);
	void initiateEdges();

	int feasibleSol;
	vector<pair<int, vector<int>>> samplePaths;

	// used for greedy algorithm with potentialPaths
	vector<pair<int, int>> listEdges;
	map<int, map<int, int>> mapPairNodes2EdgeId;
	vector<int> marginalGain;
	int overallDelay; // overall delay of potential paths
	vector<vector<int>> relatedPaths; // edge Id -> ids of potential path that contains this edge
	vector<vector<int>> paths; // path id -> list of edge ids on this path
	vector<int> pathLengths; // intial length (delay) of a sample path
	vector<map<int, int>> increaseDelay; // increase delay of path by edge:  path->edge->delay
};

