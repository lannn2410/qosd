#include "LinearAlgorithm.h"
#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
#include "Constants.h"

LinearDelayAlgorithm::LinearDelayAlgorithm(Network * g) : QoSalgorithm(g)
{

}

LinearDelayAlgorithm::~LinearDelayAlgorithm()
{
	
}

void LinearDelayAlgorithm::solveLP()
{
	IloCplex cplex(env);
	IloModel model(env);
	
	cplex.extract(model);
	cplex.setOut(env.getNullStream());
	
	IloNumVarArray var(env, listEdges->size(), 0, Constants::MAX_LEVEL, ILOFLOAT);
	result = IloNumArray(env, listEdges->size());

	IloExpr obj(env);
	for (int i = 0; i < listEdges->size(); ++i) {
		obj += var[i];
	}

	model.add(IloMinimize(env, obj));
	vector<pair<int, int>> * transactions = g->getListOfTransaction();
	
	bool resolve = false;
	do {
		resolve = false;
		cplex.exportModel("abc.lp");
		cplex.solve();
		if (cplex.getStatus() == IloAlgorithm::Optimal) {
			cout << "Obj value: " << cplex.getObjValue() << endl;
			cplex.getValues(var, result);

			#pragma omp parallel for
			for (int i = 0; i < transactions->size(); ++i) {
				pair<int, int> trans = (*transactions)[i];

				// -------- new code but error!!!
				/*vector<vector<int>> paths;
				g->shortestPaths(trans.first, trans.second, &paths, &result);
				if (paths.size() > 0) {
					#pragma omp critical
					{
						resolve = true;
						IloExpr expr(env);
						for (int jj = 0; jj < paths.size(); ++jj) {
							vector<int> path = paths[jj];

							double thres = Constants::T;

							for (int j = 0; j < path.size() - 1; ++j) {
								int sId = path[j];
								int eId = path[j + 1];
								int edgeId = (*mapPairNodes2EdgeId)[sId][eId];

								pair<int, int> edgeParameter = (*listEdgeParameters)[edgeId];
								expr += edgeParameter.first * var[edgeId];

								thres -= edgeParameter.second;
							}

							model.add(IloRange(env, thres, expr));
						}
					}
					
				}*/

				vector<int> path;
				double length = g->shortestPath(trans.first, trans.second, &path, &result);
				if (length < Constants::T) {
					#pragma omp critical
					{
						resolve = true;
						IloExpr expr(env);

						double thres = Constants::T;

						for (int j = 0; j < path.size() - 1; ++j) {
							int sId = path[j];
							int eId = path[j + 1];
							int edgeId = (*mapPairNodes2EdgeId)[sId][eId];

							pair<int, int> edgeParameter = (*listEdgeParameters)[edgeId];
							expr += edgeParameter.first * var[edgeId];
							
							thres -= edgeParameter.second;
						}

						testPaths.push_back(path);
						model.add(IloRange(env, thres, expr));
					}
				}
			}
		}
		else
			resolve = true;
	} while (resolve);

	//env.end();
}

void LinearDelayAlgorithm::initiate()
{
	listEdges = g->getListEdges();
	mapPairNodes2EdgeId = g->getMapNodePair2EdgeId();
	listEdgeParameters = g->getListEdgeParameters();
}

int LinearDelayAlgorithm::getSolution()
{
	sol = 0;
	initiate();
	solveLP();
	rounding();
	env.end();
	return sol;
}

void LinearDelayAlgorithm::testSolution()
{
	for (int i = 0; i < testPaths.size(); ++i) {
		vector<int> path = testPaths[i];
		int length = 0;
		for (int j = 0; j < path.size() - 1; ++j) {
			int sId = path[j];
			int eId = path[j + 1];
			length += g->getCurrentWeight(sId, eId);
		}
		if (length < Constants::T)
			cout << "Error!";
	}
}
