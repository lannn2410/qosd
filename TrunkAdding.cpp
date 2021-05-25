#include "TrunkAdding.h"
#include "Constants.h"
#include "HeapData.hpp"
#include "mappedheap.hpp"


TrunkAdding::TrunkAdding(Network * g) : IterativeGreedy(g)
{
}

TrunkAdding::~TrunkAdding()
{
}

int TrunkAdding::getSolution()
{
	int re = 0;

	while (!isCut()) {
		re = 0;
		int upper = potentialPaths.size() * Constants::T;
		g->resetCurrentWeight();
		//cout << "\t intiate edges" << endl;
		initiateEdges();

		int delay = overallDelay;

		vector<double> useHeapData(heapData);
		vector<int> length(pathLengths);
		DescendingOrder<double> hd(&useHeapData[0]);
		MappedHeap<DescendingOrder<double>> heap(heapIndex, hd);

		vector<map<int, vector<int>>> upDelay(increaseDelay); // increase delay of path by edge:  path->edge->level->delay
		vector<vector<int>> upOverallDelay(initialOverallDelay); // edge->level->delay

		//cout << "\t select solution" << endl;
		while (delay < upper) {
			int heapIdx = heap.top();
			pair<int, int> edgeLevel = heapIdx2edgeLevel[heapIdx];
			int edgeId = edgeLevel.first;
			pair<int, int> tmp = listEdges[edgeId];

			if (g->reachMaxLevel(tmp.first, tmp.second)) {
				heap.pop();
				continue;
			}

			int upLevel = edgeLevel.second;
			int gain = upOverallDelay[edgeId][upLevel];
			delay += gain;
			re += upLevel + 1;
			for (int i=0; i<upLevel+1; ++i)
				g->increaseQoSlevel(tmp.first, tmp.second);

			vector<int> nextDelay;
			g->getIncreasingDelays(tmp.first, tmp.second, &nextDelay);
			vector<int> levelHeapIdx = edgeLevel2HeapIdx[edgeId];
			
			upOverallDelay[edgeId].clear();
			for (int i = 0; i < levelHeapIdx.size(); ++i) {
				useHeapData[levelHeapIdx[i]] = 0;
				heap.heapify(levelHeapIdx[i]);
			}

			vector<int> tmpPaths = relatedPaths[edgeId];
			
			#pragma omp parallel for
			for (int i = 0; i < tmpPaths.size(); ++i) {
				int pathId = tmpPaths[i];
				length[pathId] += upDelay[pathId][edgeId][upLevel];

				vector<int> tmpEdges = paths[pathId];
				for (int j = 0; j < tmpEdges.size(); ++j) {
					int e = tmpEdges[j];
					if (e != edgeId) {
						vector<int> tmp = upDelay[pathId][e];
						for (int jj = 0; jj < tmp.size(); ++jj) {
							int oldValue = tmp[jj];
							int newValue = min(Constants::T - length[pathId], oldValue);
							if (tmp[jj] != newValue) {
								upDelay[pathId][e][jj] = newValue;
								#pragma omp critical
								{
									upOverallDelay[e][jj] -= oldValue - newValue;
									useHeapData[edgeLevel2HeapIdx[e][jj]] -= ((double)(oldValue - newValue)) / (jj + 1);
									heap.heapify(edgeLevel2HeapIdx[e][jj]);
								}
							}
						}
					}
					else {
						upDelay[pathId][e].clear();
						for (int jj = 0; jj < nextDelay.size(); ++jj) {
							int newValue = min(Constants::T - length[pathId], nextDelay[jj]);
							upDelay[pathId][e].push_back(newValue);
							#pragma omp critical
							{
								if (upOverallDelay[e].size() < jj + 1)
									upOverallDelay[e].push_back(newValue);
								else
									upOverallDelay[e][jj] += newValue;
								useHeapData[edgeLevel2HeapIdx[e][jj]] += ((double)newValue) / (jj + 1);
								heap.heapify(edgeLevel2HeapIdx[e][jj]);
							}
						}
					}
				}
			}
		}
		//cout << "\t check cut" << endl;
	}
	return re;
}

void TrunkAdding::initiateEdges()
{
	int id = listEdges.size();
	for (int i = potentialId; i < potentialPaths.size(); ++i) {
		paths.push_back(vector<int>());
		increaseDelay.push_back(map<int, vector<int>>());
		vector<int> path = potentialPaths[i].second;
		int length = potentialPaths[i].first;
		pathLengths.push_back(length);
		overallDelay += length;

		for (int j = 0; j < path.size() - 1; ++j) {
			int startId = path[j];
			int endId = path[j + 1];
			map<int, int> tmp = mapPairNodes2EdgeId[startId];
			vector<int> upDelays;
			g->getIncreasingDelays(startId, endId, &upDelays);
			vector<int> gains;
			vector<double> fracs;
			for (int jj = 0; jj < upDelays.size(); ++jj) {
				gains.push_back(min(upDelays[jj], Constants::T - length));
				fracs.push_back((double)(gains[jj]) / (jj + 1));
			}

			if (tmp.find(endId) == tmp.end()) {
				listEdges.push_back(pair<int, int>(startId, endId));
				mapPairNodes2EdgeId[startId][endId] = id;

				if (!g->isDirectedGraph())
					mapPairNodes2EdgeId[endId][startId] = id;

				relatedPaths.push_back(vector<int>());
				relatedPaths[id].push_back(i);
				paths[i].push_back(id);
				increaseDelay[i][id] = gains;

				edgeLevel2HeapIdx.push_back(vector<int>());
				for (int jj = 0; jj < gains.size(); ++jj) {
					heapIndex.push_back(heapIndex.size());
					heapIdx2edgeLevel.push_back(pair<int, int>(id, jj));
					edgeLevel2HeapIdx[id].push_back(heapIdx2edgeLevel.size() - 1);
					heapData.push_back(fracs[jj]);
				}

				++id;
				initialOverallDelay.push_back(gains);
			}
			else {
				int edgeId = tmp[endId];
				paths[i].push_back(edgeId);
				increaseDelay[i][edgeId] = gains;
				for (int jj = 0; jj < gains.size(); ++jj) {
					initialOverallDelay[edgeId][jj] += gains[jj];
					heapData[edgeLevel2HeapIdx[edgeId][jj]] += fracs[jj];
				}
				relatedPaths[edgeId].push_back(i);
			}
		}
	}

	potentialId = potentialPaths.size();
}
