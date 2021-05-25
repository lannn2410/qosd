#pragma once
#include "QoSalgorithm.h"

class LinearDelayAlgorithm : public QoSalgorithm
{
public:
	LinearDelayAlgorithm(Network * g);
	~LinearDelayAlgorithm();

	int getSolution();

	void testSolution(); // used for testing solution of other alg with the LP

protected:
	void solveLP();
	void initiate();
	virtual void rounding() = 0;

	vector<pair<int, int>> * listEdges;
	vector<pair<double, double>> * listEdgeParameters;
	map<int, map<int, int>> * mapPairNodes2EdgeId;
	IloNumArray result;
	int sol;
	IloEnv env;

	vector<vector<int>> testPaths; // used for testing between sampling solution and LP solution
};

