#include "AlanAlg.h"
#include "Constants.h"
#include "QoScommon.h"
#include <cstring>

using namespace std;

AlanAlg::AlanAlg()
{
	graphformat = "graph.el";
	graphbin = "graph.bin";

	string tmp = " ../multi-pcut-master/preproc " + graphformat + " " + graphbin;
	formatCmd = tmp.c_str();

	pairsFile = "pairs.txt";
}

void AlanAlg::convertGraph()
{
	system(formatCmd);
}

int AlanAlg::getSAPsol()
{
	convertGraph();
	string tmp = "../multi-pcut-master/pip -G " + graphbin + " -p " + pairsFile + " -T " + to_string(Constants::T) + " -S";
	const char * runcmd = tmp.c_str();
	string log = QoScommon::getInstance()->exec(runcmd);
	
	// get the result
	return getResultFromLog(log);
}

int AlanAlg::getMIAsol()
{
	convertGraph();
	string tmp = "../multi-pcut-master/pip -G " + graphbin + " -p " + pairsFile + " -T " + to_string(Constants::T) + " -M";
	const char * runcmd = tmp.c_str();
	string log = QoScommon::getInstance()->exec(runcmd);

	// get the result
	return getResultFromLog(log);
}

int AlanAlg::getTAGsol()
{
	convertGraph();
	string tmp = "../multi-pcut-master/pip -G " + graphbin + " -p " + pairsFile + " -T " + to_string(Constants::T) + " -T";
	const char * runcmd = tmp.c_str();
	string log = QoScommon::getInstance()->exec(runcmd);

	// get the result
	return getResultFromLog(log);
}


AlanAlg::~AlanAlg()
{
}

int AlanAlg::getResultFromLog(string log)
{
	char * re = new char[log.length()];
	strcpy(re, log.c_str());
	char * line = strtok(re, "\n");
	vector<string> lines;
	while (line != NULL) {
		string tmp(line);
		lines.push_back(tmp);
		line = strtok(NULL, "\n");
	}

	string lineRe = lines[lines.size() - 3];
	char * hmm = new char[lineRe.length()];
	strcpy(hmm, lineRe.c_str());
	char * e = strtok(hmm, " ");
	vector<string> ele;
	while (e != NULL) {
		string tmp(e);
		ele.push_back(tmp);
		e = strtok(NULL, " ");
	}

	return stoi(ele[ele.size() - 1]);
}
