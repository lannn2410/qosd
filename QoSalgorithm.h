#pragma once
#include "Network.h"

using namespace std;

class QoSalgorithm
{
public:
	QoSalgorithm(Network * g);
	~QoSalgorithm();

	virtual int getSolution() = 0;

protected:
	Network * g;
	bool isCut();
};

