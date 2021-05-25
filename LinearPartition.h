#pragma once
#include "LinearAlgorithm.h"

class LinearPartition : public LinearDelayAlgorithm
{
public:
	LinearPartition(Network * g);
	~LinearPartition();

	void rounding();
private:
	int epsilon;
	int * partition; // node id -> partition this node belong to
	map<int, map<int, int>> * mapInversePair2EdgeId;
};

