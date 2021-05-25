#include "Heuristic.h"
#include "Constants.h"
#include <algorithm>
#include "mappedheap.hpp"
#include "HeapData.hpp"

Heuristic::Heuristic(Network * g) : IncrementalGreedy(g)
{
}

Heuristic::~Heuristic()
{
}

int Heuristic::getSolution()
{
	int re = 0;

	while (!isCut()) {

		initiateEdges();
		vector<int> edgeIds;
		for (int i = 0; i < listEdges.size(); i++) {
			edgeIds.push_back(i);
		}

		DescendingOrder<int> hd(&initialMarginalGain[0]);
		MappedHeap<DescendingOrder<int>> heap(edgeIds, hd);

		int edgeId = heap.top();
		pair<int, int> tmp = listEdges[edgeId];

		if (g->reachMaxLevel(tmp.first, tmp.second)) {
			heap.pop();
			continue;
		}

		int maxLevel = g->getMaxLevel(tmp.first, tmp.second);

		for (int i = 0; i < maxLevel; ++i) {
			g->increaseQoSlevel(tmp.first, tmp.second);
			++re;
		}
	}
	return re;
}

bool Heuristic::isCut()
{
	bool re = true;
	vector<pair<int, int>> * transactions = g->getListOfTransaction();
	potentialPaths.clear();

	#pragma omp parallel for
	for (int i = 0; i < transactions->size(); ++i) {
		vector<int> path;
		int sId = (*transactions)[i].first;
		int eId = (*transactions)[i].second;
		int length = g->shortestPath(sId, eId, &path);

		if (length < Constants::T) {
			int initalLength = g->getCurrentLength(&path);
			#pragma omp critical
			{
				potentialPaths.push_back(pair<int, vector<int>>(initalLength, path));
				re = false;
			}
		}
	}
	return re;
}

void Heuristic::initiateEdges()
{
	int id = 0;
	listEdges.clear();
	initialMarginalGain.clear();
	pathLengths.clear();

	for (int i = 0; i < potentialPaths.size(); ++i) {
		vector<int> path = potentialPaths[i].second;
		int length = potentialPaths[i].first;
		pathLengths.push_back(length);

		for (int j = 0; j < path.size() - 1; ++j) {
			int startId = path[j];
			int endId = path[j + 1];
			map<int, int> tmp = mapPairNodes2EdgeId[startId];
			int gain = min(g->getIncreasingDelayOfEdge(startId, endId), Constants::T - length);

			if (tmp.find(endId) == tmp.end()) {
				listEdges.push_back(pair<int, int>(startId, endId));
				++id;
				initialMarginalGain.push_back(gain);
			}
			else {
				int edgeId = tmp[endId];
				initialMarginalGain[edgeId] += gain;
			}
		}
	}
}
