#pragma once
#include "QoSalgorithm.h"
#include "QoScommon.h"

class SamplingAlgorithm : public QoSalgorithm
{
public:
	SamplingAlgorithm(Network * g);
	~SamplingAlgorithm();
	int getSolution();
	void setAvaiSol(int sol);
private:
	int getNumberOfSamples();

	void initiateEdges();
	bool isCut();

	vector<SamplePath> samplePaths;

	// used for greedy algorithm with sampled paths
	vector<pair<int, int>> listEdges;
	map<int, map<int, int>> mapPairNodes2EdgeId;
	vector<double> marginalGain;
	double overallObjective = 0; // overall objective = sum delay(p)/prob(p)
	vector<vector<int>> relatedPaths; // edge Id -> ids of potential path that contains this edge
	vector<vector<int>> paths; // path id -> list of edge ids on this path
	vector<int> pathLengths; // intial length (delay) of a sample path
	vector<map<int, int>> increaseDelay; // increase delay of path by edge:  path->edge->delay

	vector<int> uncutTrans;
	int availableSol;
};

