#include "LinearPartition.h"
#include "QoScommon.h"
#include "HeapData.hpp"
#include "mappedheap.hpp"
#include "Constants.h"

LinearPartition::LinearPartition(Network * g) : LinearDelayAlgorithm(g)
{
	mapInversePair2EdgeId = g->getMapInversePair2EdgeId();
}

LinearPartition::~LinearPartition()
{
}

void LinearPartition::rounding()
{
	epsilon = Constants::T / 4;
	int num = g->getNumberOfNodes();
	int isAssigned = 0;
	partition = new int[num];
	fill(partition, partition + num, -1);
	int par = -1;

	int d = QoScommon::getInstance()->randomInThread() % (Constants::T / 2 - epsilon) + epsilon;

	vector<int> nodeIdx;
	for (int i = 0; i < num; ++i)
		nodeIdx.push_back(i);

	while (isAssigned < num) {
		par++;
		vector<int> tmp;
		for (int i = 0; i < num; ++i) {
			if (partition[i] == -1)
				tmp.push_back(i);
		}

		int r = QoScommon::getInstance()->randomInThread() % tmp.size();
		int u = tmp[r];

		// use to store both distance from u to other node or other node to u
		vector<double> distance; // measure distance from other node to u/ distance from u to other node
		for (int i = 0; i < num; i++) {
			distance.push_back(Constants::T);
		}

		distance[u] = 0;
		AscendingOrder<double> hd(&distance[0]);
		MappedHeap<AscendingOrder<double>> heap(nodeIdx, hd);

		while (!heap.empty()) {
			int nodeId = heap.pop();

			if (distance[nodeId] < d) {
				partition[nodeId] = par;
				isAssigned++;

				// distance from u -> nodeId
				map<int, int> mapTmp = (*mapPairNodes2EdgeId)[nodeId];
				for (map<int, int>::iterator it = mapTmp.begin(); it != mapTmp.end(); ++it) {
					int neighborId = it->first;
					double w = g->getEdgeLinearWeight(nodeId, neighborId, result[it->second]);
					if (partition[neighborId] == -1 && distance[neighborId] > distance[nodeId] + w) {
						distance[neighborId] = distance[nodeId] + w;
						heap.heapify(neighborId);
					}
				}
			}
			else break;
		}
	}

	for (int i = 0; i < listEdges->size(); ++i) {
		pair<int, int> e = (*listEdges)[i];
		if (partition[e.first] != partition[e.second]) {
			int level = Constants::T - 1;
			for (int j = 0; j < level; ++j)
				g->increaseQoSlevel(e.first, e.second);
			sol += level;
		}
	}

	if (!isCut()) sol = -1;
}
