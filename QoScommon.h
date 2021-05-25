#pragma once
#include <omp.h>
#include <vector>
#include <string>

using namespace std;

class QoScommon
{
public:
	QoScommon();
	~QoScommon();

	static QoScommon * getInstance();
	unsigned randomInThread();
	double nChoosek(unsigned n, unsigned k);

	int delayConcave(int level, int t);
	int delayConvex(int level);
	int getMaxLevel(int delayFunc, int t);
	
	std::string exec(const char * cmd);
private:
	static QoScommon * instance;
	int * seed;
};

struct SamplePath {
	int length;
	double prob;
	vector<int> path;
};

