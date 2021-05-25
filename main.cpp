#include<iostream>
#include <omp.h>
#include <time.h>
#include <fstream>
#include <stdlib.h>
#include <string>
#include "Network.h"
#include "IterativeGreedy.h"
#include "Constants.h"
#include "SamplingAlgorithm.h"
#include "TrunkAdding.h"
#include "LinearRounding.h"
#include "LinearPartition.h"
#include "IncrementalGreedy.h"
#include "Heuristic.h"
#include "AlanAlg.h"
#include <time.h>

using namespace std;

#pragma warning(disable: 4996) 

Network * g;
ofstream writefile;

void printResult(bool isDirected, int delayFunction, int noNodes = 1, double p = 0.1, int change = 1) {
	if (change == 3) {
		g->generateRandomNetwork(noNodes, p, isDirected);
	}

	g->generateTransaction();
	Constants::MAX_LEVEL = QoScommon::getInstance()->getMaxLevel(Constants::DELAY_FUNC, Constants::T);

	g->resetCurrentWeight();
	cout << "start IG" << endl;
	IterativeGreedy ig(g);
	long startIg = time(NULL);
	int reIg = -1;
	//int reIg = ig.getSolution();
	long timeIg = time(NULL) - startIg;
	cout << "IG: " << reIg << " " << timeIg << endl;

	g->resetCurrentWeight();
	cout << "start SA" << endl;
	SamplingAlgorithm sa(g);
	sa.setAvaiSol(reIg);
	long startSa = time(NULL);
	int reSa = -1;
	//int reSa = sa.getSolution();
	long timeSa = time(NULL) - startSa;
	cout << "SA: " << reSa << " " << timeSa << endl;

	g->resetCurrentWeight();
	cout << "start TA" << endl;
	TrunkAdding ta(g);
	long startTa = time(NULL);
	int reTa = -1;
	//int reTa = ta.getSolution();
	long timeTa = time(NULL) - startTa;
	cout << "TA: " << reTa << " " << timeTa << endl;
/*
	g->resetCurrentWeight();
	cout << "start InG" << endl;
	IncrementalGreedy ing(g);
	long startIng = time(NULL);
	int reIng = ing.getSolution();
	long timeIng = time(NULL) - startIng;
	cout << "InG: " << reIng << " " << timeIng << endl;*/

	g->resetCurrentWeight();
	cout << "start He" << endl;
	Heuristic he(g);
	long startHe = time(NULL);
	//int reHe = -1;
	int reHe = he.getSolution();
	long timeHe = time(NULL) - startHe;
	cout << "He: " << reHe << " " << timeHe << endl;

	if (change == 3)
		writefile << p << " \t ";

	writefile << Constants::T << " \t " << Constants::NUMBER_OF_TRANSACTIONS << " \t "
		<< to_string(reIg) << " " << to_string(timeIg) << " \t "
		<< to_string(reTa) << " " << to_string(timeTa) << " \t "
		<< to_string(reSa) << " " << to_string(timeSa) << " \t "
		//<< to_string(reIng) << " " << to_string(timeIng) << " \t "
		<< to_string(reHe) << " " << to_string(timeHe) << " \t ";

	int reLr, reLp;
	if (delayFunction == 1 || delayFunction == 5) {
		g->resetCurrentWeight();
		cout << "start LR" << endl;
		LinearRounding lr(g);
		long startLr = time(NULL);
		reLr = -1;
		//reLr = lr.getSolution();
		long timeLr = time(NULL) - startLr;
		writefile << to_string(reLr) << " " << to_string(timeLr) << " \t ";
		cout << "LR: " << reLr << " " << timeLr << endl;

		/*if (delayFunction == 5) {
			cout << "start Alan alg" << endl;
			AlanAlg aa;
			long startSAP = time(NULL);
			int reSAP = aa.getSAPsol();
			long timeSAP = time(NULL) - startSAP;
			
			long startMIA = time(NULL);
			int reMIA = aa.getMIAsol();
			long timeMIA = time(NULL) - startMIA;

			long startTAG = time(NULL);
			int reTAG = aa.getTAGsol();
			long timeTAG = time(NULL) - startTAG;
			writefile << to_string(reSAP) << " " << to_string(timeSAP) << "\t"
				<< to_string(reMIA) << " " << to_string(timeMIA) << "\t"
				<< to_string(reTAG) << " " << to_string(timeTAG) << endl;

		}*/

		/*if (!isDirected) {
			g->resetCurrentWeight();
			cout << "start LP" << endl;
			LinearPartition lp(g);
			long startLp = time(NULL);
			reLp = lp.getSolution();
			long timeLp = time(NULL) - startLp;
			writefile << to_string(reLp) << " " << to_string(timeLp);
			cout << "LP: " << reLp << " " << timeLp << endl;
		}*/
	}

	writefile << endl;
}

void runExperiment(int numberOfNodes, string file, bool isDirected, int delayFunction, 
	int change = 1, int min = 24, int max = 48, int step = 2, bool isLargeFile = false, 
	bool isRandomGraph = false) { 
	// change = 1 => change T; 2 => change number of transactions; 3 => change p (for random graph only)
	// delayFunction = 1 => linear, 2 => concave, 3 => convex

	//default value of T and number of transaction
	Constants::T = 24;
	Constants::NUMBER_OF_TRANSACTIONS = 100;
	Constants::DELAY_FUNC = delayFunction;
	Constants::IS_LARGE_NETWORK = isLargeFile;

	if (!isRandomGraph) {
		if (isLargeFile)
			g->readNetworkLargeFile(numberOfNodes, "data/" + file, isDirected);
		else
			g->readNetworkFromFile(numberOfNodes, "data/" + file, isDirected);
	}

	string outfilename = "result/" + file + "_"
		+ (isDirected ? "directed" : "undirected")
		+ "_delay" + to_string(delayFunction) 
		+ "_" + (change == 1? "changeT" : change == 2? "changeK" : "changeP") + ".txt";

	writefile.open(outfilename);
	if (writefile.is_open()) {
		if (change == 3) writefile << "p \t";

		writefile << "T \t K \t ig \t ta \t sa \t he \t";
		if (delayFunction == 1 || delayFunction == 5) {
			writefile << "lr \t ";
			/*if (delayFunction == 5)
				writefile << "sap \t mia \t tag \t";*/
			//if (!isDirected)
			//	writefile << "lp";
		}
		writefile << endl;

		if (change == 1) {
			for (Constants::T = min; Constants::T <= max; Constants::T += step) {
				printResult(isDirected, delayFunction);
			}
		}
		else if (change == 2) {
			for (Constants::NUMBER_OF_TRANSACTIONS = min; Constants::NUMBER_OF_TRANSACTIONS <= max; Constants::NUMBER_OF_TRANSACTIONS += step) {
				printResult(isDirected, delayFunction);
			}
		}
		else if (change == 3) {
			Constants::T =  3;
			for (double p = 0.1; p <= 1.0; p += 0.1) {
				printResult(isDirected, delayFunction, numberOfNodes, p, change);
			}
		}

		writefile.close();
	}

}

int main() {
	omp_set_num_threads(Constants::NUM_THREAD);
	g = new Network();
	//g->readNetworkFromFile(10876,"p2p-Gnutella04.txt");
	//runExperiment(10876, "p2p-Gnutella04.txt", true, 4, 1, 10, 50, 5);
	//runExperiment(10876, "p2p-Gnutella04.txt", true, 1, false, 50, 500, 50);

	//runExperiment(240, "random", false, 5, 3, 2, 5, 1, false, true);
	//runExperiment(240, "random", true, 4, 3, 2, 5, 1, false, true);
	/*runExperiment(240, "random1", true, 1, 3, 2, 5, 1, false, true);
	runExperiment(240, "random2", true, 2, 3, 2, 5, 1, false, true);
	runExperiment(240, "random3", true, 3, 3, 2, 5, 1, false, true);*/
	
	//runExperiment(10876, "p2p-Gnutella04.txt", true, 3, 1, 10, 50, 5);
	//runExperiment(10876, "p2p-Gnutella04.txt", true, 2, true, 10, 50, 5);
	//runExperiment(10876, "p2p-Gnutella04.txt", false, 1, true, 10, 50, 5);

	//runExperiment(1696415, "as-skitter.txt", false, 1, 1, 5, 12, 1, true);
	//runExperiment(1965206, "roadNet-CA.txt", false, 1, 1, 100, 1000, 100, true);
	runExperiment(1965206, "roadNet-CA.txt", false, 5, 1, 70, 100, 3, true);

	//void runExperiment(int numberOfNodes, string file, bool isDirected, int delayFunction,
	//	int change = 1, int min = 24, int max = 48, int step = 2, bool isLargeFile = false,
	//	bool isRandomGraph = false) 
	return 0;
}