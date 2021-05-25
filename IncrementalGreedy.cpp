#include "IncrementalGreedy.h"
#include <algorithm>
#include "Constants.h"
#include "HeapData.hpp"
#include "mappedheap.hpp"

using namespace std;

IncrementalGreedy::IncrementalGreedy(Network * g) : IterativeGreedy(g)
{
}

IncrementalGreedy::~IncrementalGreedy()
{
}

int IncrementalGreedy::getSolution()
{
	int re = 0;

	while (!isCut()) {
		int upper = potentialPaths.size() * Constants::T;
		initiateEdges();
		vector<int> edgeIds;
		for (int i = 0; i < listEdges.size(); i++) {
			edgeIds.push_back(i);
		}

		DescendingOrder<int> hd(&initialMarginalGain[0]);
		MappedHeap<DescendingOrder<int>> heap(edgeIds, hd);

		while (overallDelay < upper) {
			int edgeId = heap.top();
			int gain = initialMarginalGain[edgeId];
			overallDelay += gain;
			re++;
			pair<int, int> tmp = listEdges[edgeId];
			g->increaseQoSlevel(tmp.first, tmp.second);

			int nextDelay = g->getIncreasingDelayOfEdge(tmp.first, tmp.second);
			initialMarginalGain[edgeId] = 0;
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
							initialMarginalGain[e] -= oldValue - newValue;
							heap.heapify(e);
						}
					}
					else {
						int newValue = min(Constants::T - pathLengths[pathId], nextDelay);
						increaseDelay[pathId][e] = newValue;
						initialMarginalGain[edgeId] += newValue;
						heap.heapify(e);
					}
				}
			}
		}
	}
	return re;
}

void IncrementalGreedy::initiateEdges()
{
	int id = 0;
	listEdges.clear();
	mapPairNodes2EdgeId.clear();
	initialMarginalGain.clear();
	overallDelay = 0;
	relatedPaths.clear();
	paths.clear();
	pathLengths.clear();
	increaseDelay.clear();

	for (int i = 0; i < potentialPaths.size(); ++i) {
		paths.push_back(vector<int>());
		increaseDelay.push_back(map<int, int>());
		vector<int> path = potentialPaths[i].second;
		int length = potentialPaths[i].first;
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
				if (!g->isDirectedGraph())
					mapPairNodes2EdgeId[endId][startId] = id;
				relatedPaths.push_back(vector<int>());
				relatedPaths[id].push_back(i);
				paths[i].push_back(id);
				increaseDelay[i][id] = gain;
				++id;
				initialMarginalGain.push_back(gain);
			}
			else {
				int edgeId = tmp[endId];
				paths[i].push_back(edgeId);
				increaseDelay[i][edgeId] = gain;
				initialMarginalGain[edgeId] += gain;
				relatedPaths[edgeId].push_back(i);
			}
		}
	}
}

bool IncrementalGreedy::isCut()
{
	bool re = true;
	vector<pair<int, int>> * transactions = g->getListOfTransaction();
	potentialPaths.clear();

	#pragma omp parallel for
	for (int i = 0; i < transactions->size(); ++i) {
		map<int, map<int, bool>> excludedEdges;
		int sId = (*transactions)[i].first;
		int eId = (*transactions)[i].second;
		int length = 0;

		do {
			vector<int> path;
			length = g->shortestPath(sId, eId, &path, &excludedEdges);
			if (length < Constants::T) {
				int initalLength = g->getCurrentLength(&path);
				
				int tmp = QoScommon::getInstance()->randomInThread() % (path.size() - 1);
				excludedEdges[path[tmp]][path[tmp + 1]] = true;
				if (!g->isDirectedGraph())
					excludedEdges[path[tmp + 1]][path[tmp]] = true;
				
				#pragma omp critical
				{
					potentialPaths.push_back(pair<int, vector<int>>(initalLength, path));
					re = false;
				}
			}

		} while (length < Constants::T);
	}
	return re;
}
