#pragma once
#include <vector>
#include <map>
#include "QoScommon.h"
#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>

using namespace std;

class Network
{
public:
	Network();
	~Network();

	void generateRandomNetwork(int numberOfNodes, double p, bool isDirected);
	void readNetworkFromFile(int numberOfNodes, string file, bool isDirected = true); // file has format edges: start_id end_id
	void readNetworkLargeFile(int numberOfNodes, string file, bool isDirected = true);
	int shortestPath(int startId, int endId, vector<int> * path, map<int,map<int,bool>> * excludedEdges = nullptr);
	void shortestPaths(int startId, int endId, vector<vector<int>> * path); // used for large file
	void shortestPaths(int startId, int endId, vector<vector<int>> * path, IloNumArray * weight); // used for linear rounding solutions
	bool shortestPath(int startId, int endId, int hops); // used for hop incremental algorithm, does path whose length < T and hops count < h exists?
	double shortestPath(int startId, int endId, vector<int> * path, IloNumArray * weight); // used for linear rounding solutions

	void clear();

	vector<pair<int, int>> * getListOfTransaction();
	
	int getIncreasingDelayOfEdge(int startId, int endId);
	int getIncreasingDelayOfEdgeByLevel(int startId, int endId, int l); // increasing delay if adjusting l level into edge
	void getIncreasingDelays(int startId, int endId, vector<int> * gains);

	void resetCurrentWeight();
	int getCurrentWeight(int startId, int endId);
	void increaseQoSlevel(int startId, int endId);
	int getInitialLength(vector<int> * path);
	int getCurrentLength(vector<int> * path);
	bool reachMaxLevel(int startId, int endId);
	double getGamma();
	int getNumberOfNodes();
	bool isDirectedGraph();
	void generateTransaction();

	int samplePath(int hops, vector<int> * path); // used for hop incremental algorithm
	unsigned long long getMinimumGain(int hops); // used for hop incremental algorithm

	void samplePaths(int numberOfSamples, vector<SamplePath> * samplePaths, vector<int> * uncutTrans); // used for sampling algorithm

	vector<pair<int, int>> * getListEdges();
	map<int, map<int, int>> * getMapNodePair2EdgeId();
	map<int, map<int, int>> * getMapInversePair2EdgeId();
	vector<pair<double, double>> * getListEdgeParameters();
	double getEdgeLinearWeight(int startId, int endId, IloNum edgeLevel); // used only for linear programming solutions

	int getMaxLevel(int startId, int endId); // used for delay function = 4

private:
	int numberOfNodes;
	bool isDirected;
	vector<int> degree;
	double gamma = 1;
	QoScommon * commonInstance;
	map<int, int> mapNodeId; // map from node true id -> ordered id (used only when reading graph from file)
	
	map<int, int> * outgoingEdgeWeights; // used to store edges' weight at first
	map<int, int> * currentEdgeWeights; // used to store edges' weight when there is QoS adjustment
	
	// the same as outgoingEdgeWeights and currentEdgeWeights but store incomming edges
	map<int, int> * incommingEdgeWeights;
	map<int, int> * currentIncommingEdgeWeights;  
	
	map<int, int> * currentQoSlevel; // used to store edges' QoS level when there is QoS adjustment
	vector<pair<int, int>> listOfTransactions;

	vector<int> nodeIdx; // used only for heap map when finding shortest path
	
	map<int, int> * mapDelayFunc; // used only when the delay function is 4 (mix between 1,2,3)
	map<int, int> * mapMaxLevel;	// used only when the delay function is 4

	// used for LP programming
	vector<pair<int,int>> listEdges;
	vector<pair<double, double>> listEdgeParameters; // edge id -> beta,alpha (beta x + alpha)
	map<int, map<int, int>> mapNodePair2EdgeId; // start id -> end id -> edge id
	map<int, map<int, int>> mapInversePair2EdgeId; // end id -> start id -> edge id

	void findPath(int u, int v, int ** parent, vector<int> * path);
	void inCommingTree(int u, int * parent);

	void recalculateShortestPaths(int nodeId, vector<int> * children, vector<int> * re); // used for faster way to find shortest paths
};

