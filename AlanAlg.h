#pragma once
#include<string>

using namespace std;

class AlanAlg
{
public:
	AlanAlg();

	void convertGraph();
	int getSAPsol();
	int getMIAsol();
	int getTAGsol();

	~AlanAlg();
private:
	int getResultFromLog(string log);
	string graphformat;
	string graphbin;
	const char * formatCmd;
	string pairsFile;
};

