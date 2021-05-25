#include "IterativeGreedy.h"
#include "Constants.h"
#include <algorithm>
#include "HeapData.hpp"
#include "mappedheap.hpp"

using namespace std;

IterativeGreedy::IterativeGreedy(Network * g) : QoSalgorithm(g)
{
}


IterativeGreedy::~IterativeGreedy()
{
}

int IterativeGreedy::getSolution()
{
	int re = 0;
	
	while (!isCut()) {
		re = 0;
		int upper = potentialPaths.size() * Constants::T;
		g->resetCurrentWeight();

		//cout << "\t start initiate edges \n";
		initiateEdges();
		vector<int> edgeIds;
		for (int i = 0; i < listEdges.size(); i++) {
			edgeIds.push_back(i);
		}

		vector<int> length(pathLengths);
		vector<int> marginalGain(initialMarginalGain);

		DescendingOrder<int> hd(&marginalGain[0]);
		MappedHeap<DescendingOrder<int>> heap(edgeIds, hd);
		int delay = overallDelay;
		vector<map<int, int>> up(increaseDelay);

		//cout << "\t start find solution \n";
		//cout << "\t\t " << delay << " " << upper << endl;
		while (delay < upper) {
			int edgeId = heap.top();
			pair<int, int> tmp = listEdges[edgeId];

			if (g->reachMaxLevel(tmp.first, tmp.second)) {
				heap.pop();
				continue;
			}

			//cout << "\t\t select " << edgeId << endl;
			int gain = marginalGain[edgeId];
			delay += gain;
			re++;
			g->increaseQoSlevel(tmp.first, tmp.second);
			
			int nextDelay = g->getIncreasingDelayOfEdge(tmp.first, tmp.second);
			marginalGain[edgeId] = 0;
			heap.heapify(edgeId);

			vector<int> tmpPaths = relatedPaths[edgeId];
			
			//cout << "\t\t\t related paths " << tmpPaths.size() << endl;

			#pragma omp parallel for
			for (int i = 0; i < tmpPaths.size(); ++i) {
				int pathId = tmpPaths[i];
				length[pathId] += up[pathId][edgeId];
				//cout << "\t\t check path " << pathId << endl;
				vector<int> tmpEdges = paths[pathId];
				
				for (int j = 0; j < tmpEdges.size(); ++j) {
					int e = tmpEdges[j];
					//cout << "\t\t\t check edge " << e << endl;
					if (e != edgeId) {
						int oldValue = up[pathId][e];
						int newValue = min(Constants::T - length[pathId], oldValue);
						if (oldValue != newValue) {
							up[pathId][e] = newValue;
							//cout << "\t\t\t\t wait to heapify" << endl;
							#pragma omp critical
							{
								marginalGain[e] -= oldValue - newValue;
								heap.heapify(e);
							}
						}
					}
					else {
						int newValue = min(Constants::T - length[pathId], nextDelay);
						up[pathId][e] = newValue;
						//cout << "\t\t\t\t wait to heapify" << endl;
						#pragma omp critical
						{
							marginalGain[edgeId] += newValue;
							heap.heapify(e);
						}
					}
				}
			}
		}
		//cout << "\t start checking \n";
		//cout << "sol " << re << endl;
	}
	return re;
}

bool IterativeGreedy::isCut()
{
	vector<pair<int, int>> * transactions = g->getListOfTransaction();
	int tmp = potentialPaths.size();

	#pragma omp parallel for
	for (int i = 0; i < transactions->size(); ++i) {
		map<int, map<int, bool>> excludedEdges;
		int sId = (*transactions)[i].first;
		int eId = (*transactions)[i].second;
		int length = 0;
		
		//if (Constants::IS_LARGE_NETWORK) {
			vector<vector<int>> paths;
			g->shortestPaths(sId, eId, &paths);
			if (paths.size() > 0) {
				for (int j = 0; j < paths.size(); ++j) {
					int initalLength = g->getInitialLength(&paths[j]);
					#pragma omp critical
					{
						potentialPaths.push_back(pair<int, vector<int>>(initalLength, paths[j]));
					}
				}
			}
		//}
		//else {
		//	do {
		//		vector<int> path;
		//		//cout << "\t\t\t shortest path " << i << endl;
		//		length = g->shortestPath(sId, eId, &path, &excludedEdges);
		//		//length = g->shortestPath(sId, eId, &path, !Constants::IS_LARGE_NETWORK ? &excludedEdges : nullptr);
		//		if (length < Constants::T) {
		//			int initalLength = g->getInitialLength(&path);
		//			int tmp = QoScommon::getInstance()->randomInThread() % (path.size() - 1);
		//			excludedEdges[path[tmp]][path[tmp + 1]] = true;
		//			if (!g->isDirectedGraph())
		//				excludedEdges[path[tmp + 1]][path[tmp]] = true;

		//			#pragma omp critical
		//			{
		//				//cout << "\t\t\t add path " << i << endl;
		//				potentialPaths.push_back(pair<int, vector<int>>(initalLength, path));
		//			}
		//		}

		//	} while (length < Constants::T);
		//}
	}
	return (tmp == potentialPaths.size());

	/*vector<vector<int>> paths;
	if (!g->allPairShortestPath(&paths)) {
		for (int i = 0; i < paths.size(); ++i) {
			int initalLength = g->getInitialLength(&paths[i]);
			potentialPaths.push_back(pair<int, vector<int>>(initalLength, paths[i]));
		}
		return false;
	}
	return true;*/
}

void IterativeGreedy::initiateEdges()
{
	int id = listEdges.size();
	for (int i = potentialId; i < potentialPaths.size(); ++i) {
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

	potentialId = potentialPaths.size();
}
