#pragma once
#include "LinearAlgorithm.h"

class LinearRounding : public LinearDelayAlgorithm
{
public:
	LinearRounding(Network * g);
	~LinearRounding();

	void rounding();

private:
	double delta = 0.1;
};

