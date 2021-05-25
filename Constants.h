#pragma once
class Constants
{
public:
	Constants();
	~Constants();

	static int NUMBER_OF_TRANSACTIONS;
	static int T;
	static const int NUM_THREAD = 64;
	static int MAX_LEVEL;
	static int DELAY_FUNC;
	static bool IS_LARGE_NETWORK;
	// 1 linear: f(x) = x + 1
	// 2 concave: f(x) = log(1000*x + 1) + 1
	// 3 convex: f(x) = x^2 + 1
	// 4 mix between 2 and 3
	// 5 cut, only 2 levels; level 0  w = 1; level 1 w = T

	// used for hop incremental algorithm
	static double const EPSILON;
	static double const DELTA;
	static const int K = 5;

	// used for sampling algorithm
	static const int NUMBER_OF_SAMPLES = 10000;

	// used for sample algrithm
	static double const ALPHA;
};

