#include "HopIncremental.h"
#include <algorithm>
#include "Constants.h"
#include "QoScommon.h"
#include "HeapData.hpp"
#include "mappedheap.hpp"

using namespace std;

HopIncremental::HopIncremental(Network * g) : QoSalgorithm(g)
{
}


HopIncremental::~HopIncremental()
{
}

int HopIncremental::getSolution()
{
	int h = 5, sol;
	while (!isCut()) {
		++h;
		sol = 0;
		g->resetCurrentWeight();
		do {
			samplePaths.clear();
			long long n = getNumberOfSample(h);

			long long i = 0;
			#pragma omp parallel for
			for (i = 0; i < n; ++i) {
				// generate paths
				vector<int> path;
				int l = g->samplePath(h, &path);
				if (!path.empty())
				#pragma omp critical
				{
					samplePaths.push_back(pair<int, vector<int>>(l,path));
				}	
			}

			initiateEdges();

			int upper = samplePaths.size() * Constants::T;
			vector<int> edgeIds;
			for (int i = 0; i < listEdges.size(); i++) {
				edgeIds.push_back(i);
			}

			DescendingOrder<int> hd(&marginalGain[0]);
			MappedHeap<DescendingOrder<int>> heap(edgeIds, hd);
			// make k selection
			for (int i = 0; i < Constants::K; ++i) {
				int edgeId = heap.top();
				int gain = marginalGain[edgeId];
				overallDelay += gain;
				sol++;
				pair<int, int> tmp = listEdges[edgeId];
				g->increaseQoSlevel(tmp.first, tmp.second);

				int nextDelay = g->getIncreasingDelayOfEdge(tmp.first, tmp.second);
				marginalGain[edgeId] = 0;
				heap.heapify(edgeId);

				vector<int> tmpPaths = relatedPaths[edgeId];
				for (int i = 0; i < tmpPaths.size(); ++i) {
					int pathId = tmpPaths[i];
					pathLengths[pathId] += increaseDelay[pathId][edgeId];

					vector<int> tmpEdges = paths[pathId];
					for (int j = 0; j < tmpEdges.size(); ++j) {
						int e = tmpEdges[j];
						if (e != edgeId) {
							int oldValue = increaseDelay[pathId][e];
							int newValue = min(Constants::T - pathLengths[pathId], oldValue);
							if (oldValue != newValue) {
								increaseDelay[pathId][e] = newValue;
								marginalGain[e] -= oldValue - newValue;
								heap.heapify(e);
							}
						}
						else {
							int newValue = min(Constants::T - pathLengths[pathId], nextDelay);
							increaseDelay[pathId][e] = newValue;
							marginalGain[edgeId] += newValue;
							heap.heapify(e);
						}
					}
				}
			}

		} while (sol < feasibleSol && !isCutWithHopsLimit(h));
	}
	return min(sol,feasibleSol);
}

void HopIncremental::setFeasibleSol(int x)
{
	feasibleSol = x;
}

bool HopIncremental::isCutWithHopsLimit(int hops)
{
	vector<pair<int, int>> * transactions = g->getListOfTransaction();
	for (int i = 0; i < transactions->size(); ++i) {
		pair<int, int> trans = (*transactions)[i];
		bool tmp = g->shortestPath(trans.first, trans.second, hops);
		if (tmp) return false;
	}
	return true;
}

unsigned long long HopIncremental::getNumberOfSample(int hops)
{
	double ep1 = Constants::EPSILON / 2;
	double ep2 = Constants::EPSILON / 2;
	double delta = Constants::DELTA / feasibleSol;
	double d1 = delta / 2;
	double d2 = delta / 2;
	double nChoosek = QoScommon::getInstance()->nChoosek(g->getNumberOfNodes(), Constants::K);
	double gamma = g->getGamma();
	double tmp1 = log(1.0 / d1) / (ep1*ep1);
	double tmp2 = log(nChoosek / d2) / (ep2*ep2*(1-exp(-gamma))*(1 - exp(-gamma)));
	unsigned long long maxDegree = g->getMinimumGain(hops);
	unsigned long long t = Constants::T;
	unsigned long long tmp = t * t * maxDegree * maxDegree * max(tmp1, tmp2);
	return tmp;
}

// similar to interative greedy algorithm
void HopIncremental::initiateEdges()
{
	int id = 0;
	listEdges.clear();
	paths.clear();
	increaseDelay.clear();
	pathLengths.clear();
	overallDelay = 0;
	mapPairNodes2EdgeId.clear();
	relatedPaths.clear();
	marginalGain.clear();

	for (int i = 0; i < samplePaths.size(); ++i) {
		paths.push_back(vector<int>());
		increaseDelay.push_back(map<int, int>());

		vector<int> path = samplePaths[i].second;
		int length = samplePaths[i].first;
		pathLengths.push_back(length);
		overallDelay += length;

		for (int j = 0; j < path.size() - 1; ++j) {
			int startId = path[j];
			int endId = path[j + 1];
			map<int, int> tmp = mapPairNodes2EdgeId[startId];
			int gain = min(g->getIncreasingDelayOfEdge(startId, endId), Constants::T - length);

			if (tmp.find(endId) == tmp.end()) {
				listEdges.push_back(pair<int, int>(startId, endId));
				mapPairNodes2EdgeId[startId][endId] = id;
				relatedPaths.push_back(vector<int>());
				relatedPaths[id].push_back(i);
				paths[i].push_back(id);
				increaseDelay[i][id] = gain;
				++id;
				marginalGain.push_back(gain);
			}
			else {
				int edgeId = tmp[endId];
				paths[i].push_back(edgeId);
				increaseDelay[i][edgeId] = gain;
				marginalGain[edgeId] += gain;
				relatedPaths[edgeId].push_back(i);
			}
		}
	}
}
