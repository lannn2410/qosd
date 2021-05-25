#pragma once
#include "IterativeGreedy.h"

// used for experiment and comparison
class IncrementalGreedy : public IterativeGreedy
{
public:
	IncrementalGreedy(Network * g);
	~IncrementalGreedy();

	int getSolution();
protected:
	void initiateEdges();
	bool isCut();

};

