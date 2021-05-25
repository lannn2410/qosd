#include "QoSalgorithm.h"
#include "Constants.h"

QoSalgorithm::QoSalgorithm(Network * graph)
{
	g = graph;
}

QoSalgorithm::~QoSalgorithm()
{
}

bool QoSalgorithm::isCut()
{
	vector<pair<int, int>> * transactions = g->getListOfTransaction();
	bool re = true;
	int tmp = transactions->size();
	#pragma omp parallel for
	for (int i = 0; i < tmp; ++i) {
		vector<int> path;
		int sId = (*transactions)[i].first;
		int eId = (*transactions)[i].second;
		int length = g->shortestPath(sId, eId, &path);
		if (length < Constants::T) {
			#pragma omp critical
			{
				re = false;
				tmp = -1;
			}
		}
	}
	return re;
}
