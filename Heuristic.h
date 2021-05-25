#pragma once
#include "IncrementalGreedy.h"

class Heuristic : public IncrementalGreedy
{
public:
	Heuristic(Network * g);
	~Heuristic();

	int getSolution();

private:
	bool isCut();
	void initiateEdges();
};

