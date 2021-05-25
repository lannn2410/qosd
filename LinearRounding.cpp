#include "LinearRounding.h"
#include <algorithm>
#include "QoScommon.h"
#include "Constants.h"
#include <math.h>

LinearRounding::LinearRounding(Network * g) : LinearDelayAlgorithm(g)
{
}

LinearRounding::~LinearRounding()
{
}

void LinearRounding::rounding()
{
	int num = g->getNumberOfNodes();
	int k = 1.0/(1.0-exp(-1.0)) * (1 + log((double)num/delta) - 1.0/Constants::T);

	while (!isCut()) {
		g->resetCurrentWeight();
		sol = 0;
		for (int i = 0; i < listEdges->size(); ++i) {
			int upLevel = 0;
			if (result[i] > floor(result[i])) {
				double p = k * (result[i] - floor(result[i]));
				int r = QoScommon::getInstance()->randomInThread() % 1000;
				if (r < p * 1000) {
					upLevel = ceil(result[i]);
				}
				else
					upLevel = floor(result[i]);
			}
			else
				upLevel = result[i];
			sol += upLevel;
			pair<int, int> edge = (*listEdges)[i];
			for (int j = 0; j < upLevel; ++j) {
				g->increaseQoSlevel(edge.first, edge.second);
			}
		}
	}
}
