#include "QoScommon.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <cstring>

using namespace std;

QoScommon * QoScommon::instance = nullptr;

QoScommon::QoScommon()
{
	seed = new int[10000];
	for (int i = 0; i < 10000; i++) {
		seed[i] = rand();
	}
}

QoScommon::~QoScommon()
{
}

QoScommon * QoScommon::getInstance()
{
	if (!instance)
		instance = new QoScommon();
	return instance;
}

unsigned QoScommon::randomInThread()
{
	unsigned tmp = seed[omp_get_thread_num() % 10000];
	tmp = tmp * 17931 + 7391;
	seed[omp_get_thread_num() % 10000] = tmp;
	return tmp;
}

double QoScommon::nChoosek(unsigned n, unsigned k)
{
	if (k > n) return 0;
	if (k == 0) return 1;
	double re = n;
	for (int i = 2; i <= k; i++) {
		re *= (n - i + 1);
		re /= i;
	}
	return re;
}

int QoScommon::delayConcave(int level, int t)
{
	double a = (exp(t - 1) - 1.0) / 10.0;
	return ceil(log(level * a + 1)) + 1;
}

int QoScommon::delayConvex(int level)
{
	return level*level + 1;
}

int QoScommon::getMaxLevel(int delayFunc, int t)
{
	switch (delayFunc) {
	case 1:
		return (t - 1);
		break;
	case 2:
		return 10;
		break;
	case 3:
		return ceil(sqrt(t - 1));
		break;
	case 5: // use only for cutting
		return 1;
		break;
	default:
		break;
	}
	return 0;
}

string QoScommon::exec(const char * cmd)
{
	/*std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
	if (!pipe) throw std::runtime_error("popen() failed!");
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
			result += buffer.data();
	}
	return result;*/
	return "";
}
