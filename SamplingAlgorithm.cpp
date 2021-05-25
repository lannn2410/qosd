#include "SamplingAlgorithm.h"
#include "Constants.h"
#include <algorithm>
#include "HeapData.hpp"
#include "mappedheap.hpp"

SamplingAlgorithm::SamplingAlgorithm(Network * g) : QoSalgorithm(g)
{
	availableSol = 1000;
}

SamplingAlgorithm::~SamplingAlgorithm()
{
}

int SamplingAlgorithm::getSolution()
{
	int sol = 0;
	while (!isCut()) {
		int n = getNumberOfSamples();
		samplePaths.clear();
		g->samplePaths(n, &samplePaths, &uncutTrans);

		initiateEdges();
		vector<int> edgeIds;
		for (int i = 0; i < listEdges.size(); i++) {
			edgeIds.push_back(i);
		}

		if (edgeIds.empty()) continue;

		DescendingOrder<double> hd(&marginalGain[0]);
		MappedHeap<DescendingOrder<double>> heap(edgeIds, hd);
		// make k selection
		for (int i = 0; i < Constants::K; ++i) {
			int edgeId = heap.top();
			pair<int, int> tmp = listEdges[edgeId];
			if (g->reachMaxLevel(tmp.first, tmp.second)) {
				heap.pop();
				continue;
			}

			double gain = marginalGain[edgeId];
			overallObjective += gain;
			sol++;
			g->increaseQoSlevel(tmp.first, tmp.second);

			int nextDelay = g->getIncreasingDelayOfEdge(tmp.first, tmp.second);
			marginalGain[edgeId] = 0;
			heap.heapify(edgeId);

			vector<int> tmpPaths = relatedPaths[edgeId];
			
			#pragma omp parallel for
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
							#pragma omp critical
							{
								increaseDelay[pathId][e] = newValue;
								marginalGain[e] -= (((double)(oldValue - newValue)) / samplePaths[pathId].prob);
								heap.heapify(e);
							}
						}
					}
					else {
						int newValue = min(Constants::T - pathLengths[pathId], nextDelay);
						increaseDelay[pathId][e] = newValue;
						#pragma omp critical
						{
							marginalGain[edgeId] += (((double)newValue) / samplePaths[pathId].prob);
							heap.heapify(e);
						}
					}
				}
			}
		}
	}
	return sol;
}

void SamplingAlgorithm::setAvaiSol(int sol)
{
	availableSol = sol;
}

int SamplingAlgorithm::getNumberOfSamples()
{
	return Constants::NUMBER_OF_TRANSACTIONS;
	//return availableSol * g->getNumberOfNodes();
	//return Constants::NUMBER_OF_SAMPLES;
}

void SamplingAlgorithm::initiateEdges()
{
	int id = 0;
	listEdges.clear();
	paths.clear();
	increaseDelay.clear();
	pathLengths.clear();
	overallObjective = 0.0;
	mapPairNodes2EdgeId.clear();
	relatedPaths.clear();
	marginalGain.clear();

	for (int i = 0; i < samplePaths.size(); ++i) {
		paths.push_back(vector<int>());
		increaseDelay.push_back(map<int, int>());

		vector<int> path = samplePaths[i].path;
		int length = samplePaths[i].length;
		double prob = samplePaths[i].prob;
		pathLengths.push_back(length);
		overallObjective += ((double)length)/prob;

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
				marginalGain.push_back(((double)gain)/prob);
			}
			else {
				int edgeId = tmp[endId];
				paths[i].push_back(edgeId);
				increaseDelay[i][edgeId] = gain;
				marginalGain[edgeId] += (((double)gain)/prob);
				relatedPaths[edgeId].push_back(i);
			}
		}
	}
}

bool SamplingAlgorithm::isCut()
{
	vector<pair<int, int>> * transactions = g->getListOfTransaction();
	bool re = true;
	int tmp = transactions->size();
	uncutTrans.clear();
	
	#pragma omp parallel for
	for (int i = 0; i < tmp; ++i) {
		vector<int> path;
		int sId = (*transactions)[i].first;
		int eId = (*transactions)[i].second;
		int length = g->shortestPath(sId, eId, &path);
		if (length < Constants::T) {
			re = false;
			#pragma omp critical
			{
				re = false;
				uncutTrans.push_back(i);
			}
		}
	}
	return re;
}
