#include "Network.h"
#include <iostream>
#include <fstream>
#include "Constants.h"
#include "QoScommon.h"
#include "HeapData.hpp"
#include "mappedheap.hpp"
#include <queue>

Network::Network()
{
	commonInstance = QoScommon::getInstance();
}


Network::~Network()
{
	clear();
}

void Network::generateRandomNetwork(int numberOfNodes, double p, bool isDirected)
{
	clear();
	this->numberOfNodes = numberOfNodes;
	this->isDirected = isDirected;
	outgoingEdgeWeights = new map<int, int>[numberOfNodes];
	currentEdgeWeights = new map<int, int>[numberOfNodes];
	incommingEdgeWeights = new map<int, int>[numberOfNodes];
	currentIncommingEdgeWeights = new map<int, int>[numberOfNodes];
	currentQoSlevel = new map<int, int>[numberOfNodes];
	mapDelayFunc = new map<int, int>[numberOfNodes];
	mapMaxLevel = new map<int, int>[numberOfNodes];

	// write graph to file to use for compare with Alan's algorithm
	ofstream writefile;
	if (!isDirected)
		writefile.open("graph" + to_string(p) + ".el");

	if (!writefile.is_open()) {
		cout << "Error on openning file to write random graph" << endl;
		return;
	}

	if (writefile.is_open())
		writefile << numberOfNodes << " 0" << endl;
	
	for (int i = 0; i < numberOfNodes; i++) {
		nodeIdx.push_back(i);
	}

	#pragma omp parallel for
	for (int i = 0; i < numberOfNodes; ++i) {
		for (int j = isDirected? 0 : i + 1; j < numberOfNodes; ++j) {
			if ((isDirected && i != j) || !isDirected) {
				int r = QoScommon::getInstance()->randomInThread() % 1000;
				if (r < p * 1000) {
					#pragma omp critical
					{
						if (writefile.is_open())
							writefile << i << " " << j << endl;
						// temporary set initial weight is 1
						outgoingEdgeWeights[i][j] = 1;
						currentEdgeWeights[i][j] = 1;

						incommingEdgeWeights[j][i] = 1;
						currentIncommingEdgeWeights[j][i] = 1;

						listEdges.push_back(pair<int, int>(i, j));
						listEdgeParameters.push_back(pair<double, double>(Constants::DELAY_FUNC == 1? 1.0 : Constants::T, 1.0)); // TODO temporary
						mapNodePair2EdgeId[i][j] = listEdges.size() - 1;
						mapInversePair2EdgeId[j][i] = listEdges.size() - 1;
						
						int df = rand() % 2 + 1;
						if (Constants::DELAY_FUNC == 4) { // if delay function is mixed
							mapDelayFunc[i][j] = df; // now only use 1st and 3rd delay function type
						}

						if (!isDirected) {
							outgoingEdgeWeights[j][i] = 1;
							currentEdgeWeights[j][i] = 1;
							incommingEdgeWeights[i][j] = 1;
							currentIncommingEdgeWeights[i][j] = 1;
							mapNodePair2EdgeId[j][i] = listEdges.size() - 1;
							mapInversePair2EdgeId[i][j] = listEdges.size() - 1;
							if (Constants::DELAY_FUNC == 4) { // if delay function is mixed
								mapDelayFunc[j][i] = df;
							}
						}
					}
				}
			}
		}
	}

	if (writefile.is_open())
		writefile.close();
}

void Network::readNetworkFromFile(int numberOfNodes, string file, bool isDirected)
{
	clear();
	this->numberOfNodes = numberOfNodes;
	this->isDirected = isDirected;
	ifstream inputFile;
	inputFile.open(file);
	if (inputFile) {
		outgoingEdgeWeights = new map<int, int>[numberOfNodes];
		currentEdgeWeights = new map<int, int>[numberOfNodes];
		incommingEdgeWeights = new map<int, int>[numberOfNodes];
		currentIncommingEdgeWeights = new map<int, int>[numberOfNodes];
		currentQoSlevel = new map<int, int>[numberOfNodes];

		mapDelayFunc = new map<int, int>[numberOfNodes];
		mapMaxLevel = new map<int, int>[numberOfNodes];

		int startId, endId;
		int zeroId = -1;
		int id = 0;
		int sId, eId; // used to stored ordered id of startId and endId
		while (inputFile >> startId >> endId) {
			sId = mapNodeId[startId];
			if (!sId && startId != zeroId) {
				mapNodeId[startId] = id;
				sId = id;
				if (id == 0)
					zeroId = startId;
				id++;
			}

			eId = mapNodeId[endId];
			if (!eId && endId != zeroId) {
				mapNodeId[endId] = id;
				eId = id;
				id++;
			}

			// temporary set initial weight is 1
			outgoingEdgeWeights[sId][eId] = 1;
			currentEdgeWeights[sId][eId] = 1;

			incommingEdgeWeights[eId][sId] = 1;
			currentIncommingEdgeWeights[eId][sId] = 1;

			listEdges.push_back(pair<int, int>(sId, eId));
			listEdgeParameters.push_back(pair<double, double>(Constants::DELAY_FUNC == 1 ? 1.0 : Constants::T, 1.0)); // TODO temporary
			mapNodePair2EdgeId[sId][eId] = listEdges.size() - 1;
			mapInversePair2EdgeId[eId][sId] = listEdges.size() - 1;

			int df = rand() % 3 + 1; // random choose delay function
			if (Constants::DELAY_FUNC == 4) { // if delay function is mixed
				mapDelayFunc[sId][eId] = df;
			}
			
			if (!isDirected) {
				outgoingEdgeWeights[eId][sId] = 1;
				currentEdgeWeights[eId][sId] = 1;
				incommingEdgeWeights[sId][eId] = 1;
				currentIncommingEdgeWeights[sId][eId] = 1;
				mapNodePair2EdgeId[eId][sId] = listEdges.size() - 1;
				mapInversePair2EdgeId[sId][eId] = listEdges.size() - 1;
				if (Constants::DELAY_FUNC == 4)
					mapDelayFunc[eId][sId] = df;
			}

			
		}

		for (int i = 0; i < numberOfNodes; i++) {
			nodeIdx.push_back(i);
			degree.push_back(outgoingEdgeWeights[i].size());
		}
		sort(degree.begin(), degree.end());

		listOfTransactions.clear();

		inputFile.close();
	}
}

void Network::readNetworkLargeFile(int numberOfNodes, string file, bool isDirected)
{
	clear();
	this->numberOfNodes = numberOfNodes;
	this->isDirected = isDirected;

	ifstream is(file);
	is.seekg(0, is.end);
	long bufSize = is.tellg();
	is.seekg(0, is.beg);
	int item = 0;

	char * buffer = new char[bufSize];

	is.read(buffer, bufSize);
	is.close();

	std::string::size_type sz = 0;
	long sp = 0;
	int startId, endId;
	bool isStart = true;
	int zeroId = -1;
	int id = 0;
	int sId, eId; // used to stored ordered id of startId and endId

	outgoingEdgeWeights = new map<int, int>[numberOfNodes];
	currentEdgeWeights = new map<int, int>[numberOfNodes];
	incommingEdgeWeights = new map<int, int>[numberOfNodes];
	currentIncommingEdgeWeights = new map<int, int>[numberOfNodes];
	currentQoSlevel = new map<int, int>[numberOfNodes];
	mapDelayFunc = new map<int, int>[numberOfNodes];
	mapMaxLevel = new map<int, int>[numberOfNodes];

	// write graph to file to use for compare with Alan's algorithm
	ofstream writefile;
	if (!isDirected) // used to test Alan algorithm
		writefile.open("graph.el");

	if (!writefile.is_open()) {
		cout << "Error on openning file to write random graph" << endl;
		return;
	}

	if (writefile.is_open())
		writefile << numberOfNodes << " 0" << endl;

	while (sp < bufSize) {
		char c = buffer[sp];
		//cout << item << endl;
		item = item * 10 + c - 48;
		sp++;
		if (sp == bufSize || (sp < bufSize && (buffer[sp] == '\t' || buffer[sp] == '\n' || buffer[sp] == '\r'))) {
			sp++;
			while (buffer[sp] < 48 || buffer[sp] > 57)
				sp++;

			if (isStart) {
				startId = item;
				isStart = false;
			}
			else {
				endId = item;
				isStart = true;

				sId = mapNodeId[startId];
				if (!sId && startId != zeroId) {
					mapNodeId[startId] = id;
					sId = id;
					if (id == 0)
						zeroId = startId;
					id++;
				}

				eId = mapNodeId[endId];
				if (!eId && endId != zeroId) {
					mapNodeId[endId] = id;
					eId = id;
					id++;
				}

				/*if (id == numberOfNodes) {
					for (map<int, int>::iterator it = mapNodeId.begin(); it != mapNodeId.end(); ++it)
						cout << it->first << " " << it->second << endl;
				}*/

				if (writefile.is_open())
					writefile << sId << " " << eId << endl;

				// temporary set initial weight is 1
				outgoingEdgeWeights[sId][eId] = 1;
				currentEdgeWeights[sId][eId] = 1;

				incommingEdgeWeights[eId][sId] = 1;
				currentIncommingEdgeWeights[eId][sId] = 1;

				listEdges.push_back(pair<int, int>(sId, eId));
				listEdgeParameters.push_back(pair<double, double>(Constants::DELAY_FUNC == 1 ? 1.0 : Constants::T, 1.0)); // TODO temporary
				mapNodePair2EdgeId[sId][eId] = listEdges.size() - 1;
				mapInversePair2EdgeId[eId][sId] = listEdges.size() - 1;

				//cout << listEdges.size() << " " << startId << " " << endId << " " << sId << " " << eId << endl;

				int df = rand() % 3 + 1; // random choose delay function
				if (Constants::DELAY_FUNC == 4) { // if delay function is mixed
					mapDelayFunc[sId][eId] = df;
					//mapDelayFunc[sId][eId] = df == 1 ? 1 : 3;
				}

				if (!isDirected) {
					outgoingEdgeWeights[eId][sId] = 1;
					currentEdgeWeights[eId][sId] = 1;
					incommingEdgeWeights[sId][eId] = 1;
					currentIncommingEdgeWeights[sId][eId] = 1;
					mapNodePair2EdgeId[eId][sId] = listEdges.size() - 1;
					mapInversePair2EdgeId[sId][eId] = listEdges.size() - 1;
					if (Constants::DELAY_FUNC == 4) { // if delay function is mixed
						//mapDelayFunc[eId][sId] = df == 1 ? 1 : 3;
						mapDelayFunc[eId][sId] = df;
					}
				}
			}

			item = 0;
		}
	}

	//cout << id << endl;

	for (int i = 0; i < numberOfNodes; i++) {
		nodeIdx.push_back(i);
	}

	if (writefile.is_open())
		writefile.close();

	delete[] buffer;
}

int Network::shortestPath(int startId, int endId, vector<int>* path, map<int, map<int, bool>> * excludedEdges)
{
	vector<int> distance;
	//bool * visited = new bool[numberOfNodes];
	unsigned * parent = new unsigned[numberOfNodes];
	for (int i = 0; i < numberOfNodes; i++) {
		distance.push_back(Constants::T);
		//visited[i] = false;
		parent[i] = -1;
	}

	distance[startId] = 0;
	AscendingOrder<int> hd(&distance[0]);
	MappedHeap<AscendingOrder<int>> heap(nodeIdx, hd);

	while (!heap.empty()) {
		int nodeId = heap.pop();
		//visited[nodeId] = true;
		
		if (nodeId == endId && distance[endId] < Constants::T) {
			vector<int> tmp;
			while (parent[nodeId] != -1) {
				tmp.push_back(nodeId);
				nodeId = parent[nodeId];
			}

			path->push_back(startId);
			for (int i = tmp.size() - 1; i >= 0; i--)
				path->push_back(tmp[i]);

			//delete[] visited;
			delete[] parent;

			return distance[endId];
		}

		if (distance[nodeId] >= Constants::T) {
			//delete[] visited;
			delete[] parent;
			return Constants::T;
		}

		map<int, int> mapTmp = currentEdgeWeights[nodeId];
		for (map<int, int>::iterator it = mapTmp.begin(); it != mapTmp.end(); ++it) {
			int neighborId = it->first;
			int w = it->second;
			if ((excludedEdges == nullptr || !(*excludedEdges)[nodeId][neighborId]) 
				&& /*!visited[neighborId] &&*/ (distance[neighborId] > distance[nodeId] + w)) {
				distance[neighborId] = distance[nodeId] + w;
				parent[neighborId] = nodeId;
				heap.heapify(neighborId);
			}
		}
	}

	//delete[] visited;
	delete[] parent;

	return Constants::T;
}

void Network::shortestPaths(int startId, int endId, vector<vector<int>>* path)
{
	vector<int> distance;
	map<int, map<int, bool>> excludedEdges;
	unsigned * parent = new unsigned[numberOfNodes];
	vector<int> * children = new vector<int>[numberOfNodes];
	for (int i = 0; i < numberOfNodes; i++) {
		distance.push_back(Constants::T);
		parent[i] = -1;
	}

	distance[endId] = 0;
	AscendingOrder<int> hd(&distance[0]);
	MappedHeap<AscendingOrder<int>> heap(nodeIdx, hd);

	do {
		//cout << "\t\t refind" << endl;
		while (!heap.empty()) {
			int nodeId = heap.pop();
			if (distance[nodeId] >= Constants::T)
				break;

			map<int, int> mapTmp = currentIncommingEdgeWeights[nodeId];
			for (map<int, int>::iterator it = mapTmp.begin(); it != mapTmp.end(); ++it) {
				int neighborId = it->first;
				int w = it->second;
				if (!excludedEdges[neighborId][nodeId] && distance[neighborId] > distance[nodeId] + w) {
					distance[neighborId] = distance[nodeId] + w;
					if (parent[neighborId] != -1) {
						vector<int> * tmp = &children[parent[neighborId]];
						tmp->erase(find(tmp->begin(), tmp->end(), neighborId));
					}

					parent[neighborId] = nodeId;
					children[nodeId].push_back(neighborId);
					heap.heapify(neighborId);
				}
			}
		}

		if (distance[startId] < Constants::T) {
			vector<int> p;
			int tmp = startId;
			while (parent[tmp] != -1) {
				p.push_back(tmp);
				tmp = parent[tmp];
			}
			p.push_back(endId);
			path->push_back(p);

			//cout << "\t\t\t add path " << startId << " " << endId << endl;

			int r = QoScommon::getInstance()->randomInThread() % (p.size() - 1);
			excludedEdges[p[r]][p[r+1]] = true;
			if (!isDirected)
				excludedEdges[p[r+1]][p[r]] = true;
			
			// remove p[r] node information from children of p[r + 1]
			vector<int> * ch = &children[p[r + 1]];
			//vector<int>::iterator it = find(ch->begin(), ch->end(), p[r]);
			ch->erase(find(ch->begin(), ch->end(), p[r]));

			vector<int> recalNodes;
			recalculateShortestPaths(p[r], children, &recalNodes);

			// clear old information
			for (int i = 0; i < recalNodes.size(); ++i) {
				int n = recalNodes[i];
				distance[n] = Constants::T;
				children[n].clear();
				parent[n] = -1;
			}

			// remove information of nodes that need to be recalculated and add back to heap
			for (int i = 0; i < recalNodes.size(); ++i) {
				int n = recalNodes[i];
				heap.push(n);
				map<int, int> mapTmp = currentEdgeWeights[n];
				for (map<int, int>::iterator it = mapTmp.begin(); it != mapTmp.end(); ++it) {
					int neighborId = it->first;
					int w = it->second;
					if (!excludedEdges[n][neighborId] && distance[n] > distance[neighborId] + w) {
						distance[n] = distance[neighborId] + w;
						if (parent[n] != -1) {
							vector<int> * tmp = &children[parent[n]];
							tmp->erase(find(tmp->begin(), tmp->end(), n));
						}
						parent[n] = neighborId;
						children[neighborId].push_back(n);
						heap.heapify(n);
					}
				}
			}
		}
	} while (distance[startId] < Constants::T);
	

	//delete[] visited;
	delete[] parent;
	delete[] children;
}

void Network::shortestPaths(int startId, int endId, vector<vector<int>>* path, IloNumArray * weight)
{
	vector<double> distance;
	map<int, map<int, bool>> excludedEdges;
	unsigned * parent = new unsigned[numberOfNodes];
	vector<int> * children = new vector<int>[numberOfNodes];
	for (int i = 0; i < numberOfNodes; i++) {
		distance.push_back(Constants::T);
		parent[i] = -1;
	}

	distance[endId] = 0;
	AscendingOrder<double> hd(&distance[0]);
	MappedHeap<AscendingOrder<double>> heap(nodeIdx, hd);

	do {
		//cout << "\t\t refind" << endl;
		while (!heap.empty()) {
			int nodeId = heap.pop();
			if (distance[nodeId] >= Constants::T)
				break;

			map<int, int> mapTmp = currentIncommingEdgeWeights[nodeId];
			for (map<int, int>::iterator it = mapTmp.begin(); it != mapTmp.end(); ++it) {
				int neighborId = it->first;
				int edgeId = mapNodePair2EdgeId[neighborId][nodeId];
				double w = getEdgeLinearWeight(neighborId, nodeId, (*weight)[edgeId]);
				if (!excludedEdges[neighborId][nodeId] && distance[neighborId] > distance[nodeId] + w) {
					distance[neighborId] = distance[nodeId] + w;
					if (parent[neighborId] != -1) {
						vector<int> * tmp = &children[parent[neighborId]];
						tmp->erase(find(tmp->begin(), tmp->end(), neighborId));
					}

					parent[neighborId] = nodeId;
					children[nodeId].push_back(neighborId);
					heap.heapify(neighborId);
				}
			}
		}

		if (distance[startId] < Constants::T) {
			vector<int> p;
			int tmp = startId;
			while (parent[tmp] != -1) {
				p.push_back(tmp);
				tmp = parent[tmp];
			}
			p.push_back(endId);
			path->push_back(p);

			//cout << "\t\t\t add path " << startId << " " << endId << endl;

			int r = QoScommon::getInstance()->randomInThread() % (p.size() - 1);
			excludedEdges[p[r]][p[r + 1]] = true;
			if (!isDirected)
				excludedEdges[p[r + 1]][p[r]] = true;

			// remove p[r] node information from children of p[r + 1]
			vector<int> * ch = &children[p[r + 1]];
			//vector<int>::iterator it = find(ch->begin(), ch->end(), p[r]);
			ch->erase(find(ch->begin(), ch->end(), p[r]));

			vector<int> recalNodes;
			recalculateShortestPaths(p[r], children, &recalNodes);

			// clear old information
			for (int i = 0; i < recalNodes.size(); ++i) {
				int n = recalNodes[i];
				distance[n] = Constants::T;
				children[n].clear();
				parent[n] = -1;
			}

			// remove information of nodes that need to be recalculated and add back to heap
			for (int i = 0; i < recalNodes.size(); ++i) {
				int n = recalNodes[i];
				heap.push(n);
				map<int, int> mapTmp = currentEdgeWeights[n];
				for (map<int, int>::iterator it = mapTmp.begin(); it != mapTmp.end(); ++it) {
					int neighborId = it->first;
					int edgeId = mapNodePair2EdgeId[n][neighborId];
					double w = getEdgeLinearWeight(n, neighborId, (*weight)[edgeId]);
					if (!excludedEdges[n][neighborId] && distance[n] > distance[neighborId] + w) {
						distance[n] = distance[neighborId] + w;
						if (parent[n] != -1) {
							vector<int> * tmp = &children[parent[n]];
							tmp->erase(find(tmp->begin(), tmp->end(), n));
						}
						parent[n] = neighborId;
						children[neighborId].push_back(n);
						heap.heapify(n);
					}
				}
			}
		}
	} while (distance[startId] < Constants::T);


	//delete[] visited;
	delete[] parent;
	delete[] children;
}

bool Network::shortestPath(int startId, int endId, int hops) // return true if there exists path between start id  to end id whose length < T and hop count <= hops
{
	int ** distance = new int*[hops + 1];
	for (int i = 0; i <= hops; ++i) {
		distance[i] = new int[numberOfNodes];
		for (int j = 0; j < numberOfNodes; ++j) {
			distance[i][j] = Constants::T;
		}
		distance[i][startId] = 0;
	}

	for (int i = 0; i < hops; ++i) {
		for (int j = 0; j < numberOfNodes; ++j) {
			map<int, int> edges = currentEdgeWeights[j];
			for (map<int, int>::iterator it = edges.begin(); it != edges.end(); ++it) {
				if (distance[i + 1][it->second] > distance[i][j] + it->second)
					distance[i + 1][it->second] > distance[i][j] + it->second;
			}
		}
	}

	int tmp = distance[hops][endId];
	for (int i = 0; i <= hops; ++i) {
		delete[] distance[i];
	}
	delete[] distance;

	return (tmp < Constants::T);
}

double Network::shortestPath(int startId, int endId, vector<int>* path, IloNumArray * weight)
{
	vector<double> distance;
	bool * visited = new bool[numberOfNodes];
	unsigned * parent = new unsigned[numberOfNodes];
	for (int i = 0; i < numberOfNodes; i++) {
		distance.push_back(Constants::T);
		visited[i] = false;
		parent[i] = -1;
	}

	distance[startId] = 0;
	AscendingOrder<double> hd(&distance[0]);
	MappedHeap<AscendingOrder<double>> heap(nodeIdx, hd);

	while (!heap.empty()) {
		int nodeId = heap.pop();
		visited[nodeId] = true;

		if (nodeId == endId && distance[endId] < Constants::T) {
			vector<int> tmp;
			while (parent[nodeId] != -1) {
				tmp.push_back(nodeId);
				nodeId = parent[nodeId];
			}

			path->push_back(startId);
			for (int i = tmp.size() - 1; i >= 0; i--)
				path->push_back(tmp[i]);

			delete[] visited;
			delete[] parent;

			return distance[endId];
		}

		if (distance[nodeId] >= Constants::T) {
			delete[] visited;
			delete[] parent;
			return Constants::T;
		}

		map<int, int> mapTmp = outgoingEdgeWeights[nodeId];
		for (map<int, int>::iterator it = mapTmp.begin(); it != mapTmp.end(); ++it) {
			int neighborId = it->first;
			int edgeId = mapNodePair2EdgeId[nodeId][neighborId];
			double w = getEdgeLinearWeight(nodeId, neighborId, (*weight)[edgeId]);
			if (!visited[neighborId] && distance[neighborId] > distance[nodeId] + w) {
				distance[neighborId] = distance[nodeId] + w;
				parent[neighborId] = nodeId;
				heap.heapify(neighborId);
			}
		}
	}

	delete[] visited;
	delete[] parent;

	return Constants::T;
}
//
//bool Network::allPairShortestPath(vector<vector<int>> * paths)
//{
//	bool re = true;
//	int ** distance = new int*[numberOfNodes];
//	int ** parent = new int*[numberOfNodes];
//	for (int i = 0; i < numberOfNodes; i++) {
//		distance[i] = new int[numberOfNodes];
//		parent[i] = new int[numberOfNodes];
//		
//		distance[i][i] = 0;
//		parent[i][i] = -1;
//
//		map<int, int> neighbors = currentEdgeWeights[i];
//		for (map<int, int>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
//			distance[i][it->first] = it->second;
//			parent[i][it->first] = i;
//		}
//		
//		for (int j = 0; j < numberOfNodes; ++j) {
//			if (i != j && distance[i][j] == 0) {
//				distance[i][j] = Constants::T + 1;
//				parent[i][j] = -1;
//			}
//		}
//	}
//
//	for (int k = 0; k < numberOfNodes; ++k) {
//		for (int i = 0; i < numberOfNodes; ++i) {
//			#pragma omp parallel for
//			for (int j = 0; j < numberOfNodes; ++j) {
//				if (distance[i][j] > distance[i][k] + distance[k][j]) {
//					distance[i][j] = distance[i][k] + distance[k][j];
//					parent[i][j] = k;
//				}
//			}
//		}
//	}
//
//	for (int i = 0; i < listOfTransactions.size(); ++i) {
//		pair<int, int> trans = listOfTransactions[i];
//		if (distance[trans.first][trans.second] < Constants::T) {
//			vector<int> path;
//			findPath(trans.first, trans.second, parent, &path);
//			path.push_back(trans.second);
//			paths->push_back(path);
//			re = false;
//		}
//	}
//	
//	return re;
//}

vector<pair<int, int>>* Network::getListOfTransaction()
{
	return &listOfTransactions;
}

int Network::getIncreasingDelayOfEdge(int startId, int endId)
{
	int l = currentQoSlevel[startId][endId];
	int df = Constants::DELAY_FUNC != 4 ? Constants::DELAY_FUNC : mapDelayFunc[startId][endId];
	int maxLevel = Constants::DELAY_FUNC != 4 ? Constants::MAX_LEVEL : QoScommon::getInstance()->getMaxLevel(df, Constants::T);

	if (l < maxLevel) {
		switch (df) {
		case 1:
			return 1;
			break;
		case 2:
			return commonInstance->delayConcave(l + 1, Constants::T) - commonInstance->delayConcave(l, Constants::T);
			break;
		case 3:
			return commonInstance->delayConvex(l + 1) - commonInstance->delayConvex(l);
			break;
		case 5:
			return Constants::T;
			break;
		default:
			return 1;
			break;
		}
	}
	else
		return 0;

	/*if (l < Constants::MAX_LEVEL & Constants::DELAY_FUNC != 4) {
		switch (Constants::DELAY_FUNC) {
		case 1:
			return 1;
			break;
		case 2:
			return commonInstance->delayConcave(l + 1, Constants::T) - commonInstance->delayConcave(l, Constants::T);
			break;
		case 3:
			return commonInstance->delayConvex(l + 1) - commonInstance->delayConvex(l);
			break;
		default:
			return 1;
			break;
		}
	}
	else
		return 0;*/
}

int Network::getIncreasingDelayOfEdgeByLevel(int startId, int endId, int l)
{
	int le = currentQoSlevel[startId][endId];
	int df = Constants::DELAY_FUNC != 4 ? Constants::DELAY_FUNC : mapDelayFunc[startId][endId];
	int maxLevel = Constants::DELAY_FUNC != 4 ? Constants::MAX_LEVEL : QoScommon::getInstance()->getMaxLevel(df, Constants::T);
	int inc = min(l, maxLevel - le);
	switch (df) {
	case 1:
		return inc;
		break;
	case 2:
		return commonInstance->delayConcave(le + inc, Constants::T) - commonInstance->delayConcave(le, Constants::T);
		break;
	case 3:
		return commonInstance->delayConvex(le + inc) - commonInstance->delayConvex(le);
		break;
	case 5:
		return inc > 0 ? Constants::T : 0;
	default:
		return inc;
		break;
	}
}

void Network::getIncreasingDelays(int startId, int endId, vector<int>* gains)
{
	int currentLevel = currentQoSlevel[startId][endId];
	int maxLevel = Constants::DELAY_FUNC != 4 ? Constants::MAX_LEVEL 
		: QoScommon::getInstance()->getMaxLevel(mapDelayFunc[startId][endId], Constants::T);
	if (currentLevel < maxLevel) {
		for (int i = 1; i <= maxLevel - currentLevel; ++i) {
			gains->push_back(getIncreasingDelayOfEdgeByLevel(startId, endId, i));
		}
	}
}

void Network::resetCurrentWeight()
{
	#pragma omp parallel for
	for (int i = 0; i < numberOfNodes; ++i) {
		currentEdgeWeights[i] = outgoingEdgeWeights[i];
		currentIncommingEdgeWeights[i] = incommingEdgeWeights[i];
		currentQoSlevel[i].clear();
	}
/*
	delete[] currentQoSlevel;
	currentQoSlevel = new map<int, int>[numberOfNodes];*/
}

int Network::getCurrentWeight(int startId, int endId)
{
	return currentEdgeWeights[startId][endId];
}

void Network::increaseQoSlevel(int startId, int endId)
{
	int tmp2 = currentQoSlevel[startId][endId];
	int maxLevel = Constants::DELAY_FUNC != 4 ? Constants::MAX_LEVEL 
		: QoScommon::getInstance()->getMaxLevel(mapDelayFunc[startId][endId], Constants::T);
	if (tmp2 < maxLevel) {
		int tmp = getIncreasingDelayOfEdge(startId, endId);
		currentEdgeWeights[startId][endId] += tmp;
		currentIncommingEdgeWeights[endId][startId] += tmp;
		tmp2++;
		currentQoSlevel[startId][endId] = tmp2;

		if (!isDirected) {
			currentEdgeWeights[endId][startId] += tmp;
			currentIncommingEdgeWeights[startId][endId] += tmp;
			currentQoSlevel[endId][startId] = tmp2;
		}
	}
}

int Network::getInitialLength(vector<int>* path)
{
	int re = 0;
	for (int i = 0; i < path->size() - 1; ++i) {
		int startId = (*path)[i];
		int endId = (*path)[i + 1];
		re += outgoingEdgeWeights[startId][endId];
	}
	return re;
}

int Network::getCurrentLength(vector<int>* path)
{
	int re = 0;
	for (int i = 0; i < path->size() - 1; ++i) {
		int startId = (*path)[i];
		int endId = (*path)[i + 1];
		re += currentEdgeWeights[startId][endId];
	}
	return re;
}

bool Network::reachMaxLevel(int startId, int endId)
{
	int tmp2 = currentQoSlevel[startId][endId];
	int maxLevel = Constants::DELAY_FUNC != 4 ? Constants::MAX_LEVEL
		: QoScommon::getInstance()->getMaxLevel(mapDelayFunc[startId][endId], Constants::T);
	return (tmp2 == maxLevel);
}

double Network::getGamma()
{
	return gamma;
}

int Network::getNumberOfNodes()
{
	return numberOfNodes;
}

bool Network::isDirectedGraph()
{
	return isDirected;
}

void Network::generateTransaction()
{
	//listOfTransactions.clear();
	// randomly generate list of transaction
	for (int i = 0; i < Constants::NUMBER_OF_TRANSACTIONS - listOfTransactions.size(); ++i) {
		int startId = rand() % numberOfNodes;
		int endId = rand() % numberOfNodes;
		if (startId != endId)
			listOfTransactions.push_back(pair<int, int>(startId, endId));
	}

	ofstream writefile;
	writefile.open("pairs.txt");
	if (writefile.is_open()) {
		for (int i = 0; i < listOfTransactions.size(); ++i) {
			pair<int, int> trans = listOfTransactions[i];
			writefile << trans.first << " " << trans.second << endl;
		}
		writefile.close();
	}
}

int Network::samplePath(int hops, vector<int> * path)
{
	int tmp = QoScommon::getInstance()->randomInThread() % listOfTransactions.size();
	pair<int, int> trans = listOfTransactions[tmp];
	int h = 1, u = trans.first, length = 0;

	do {
		path->push_back(u);
		map<int, int> edge = currentEdgeWeights[u];
		
		if (edge.size() == 0) {
			path->clear();
			return Constants::T;
		}
			
		int r = QoScommon::getInstance()->randomInThread() % edge.size();
		map<int, int>::iterator it = edge.begin();
		advance(it, r);
		u = it->first;
		
		if (find(path->begin(), path->end(), u) != path->end()) {
			path->clear();
			return Constants::T;
		}

		length += it->second;
		++h;
	} while (h <= hops || u != trans.second || length < Constants::T);
	
	if (length >= Constants::T || h > hops)
		path->clear();
	else
		path->push_back(u);
	
	return min(length, Constants::T);
}

unsigned long long Network::getMinimumGain(int hops)
{
	int h = 1;
	unsigned long long re = 1;
	while (h <= hops) {
		re *= degree[numberOfNodes - h];
		++h;
	}
	return re;
}

void Network::samplePaths(int numberOfSamples, vector<SamplePath> * samplePaths, vector<int> * uncutTrans)
{
	vector<pair<int, int>> listOfSampledTrans;
	vector<int> listOfSampleIdx;
	bool * inSamples = new bool[numberOfNodes];
	fill(inSamples, inSamples + numberOfNodes, false);
	vector<int> nodes2getTree;

	for (int i = 0; i < numberOfSamples; ++i) {
		int tmp = QoScommon::getInstance()->randomInThread() % 1000;
		int idx = 0;
		if (tmp < Constants::ALPHA * 1000.0)
			idx = (*uncutTrans)[QoScommon::getInstance()->randomInThread() % uncutTrans->size()];
		else
			idx = QoScommon::getInstance()->randomInThread() % listOfTransactions.size();
		pair<int, int> trans = listOfTransactions[idx];
		
		listOfSampledTrans.push_back(trans);
		listOfSampleIdx.push_back(idx);
		
		if (!inSamples[trans.second]) {
			inSamples[trans.second] = true;
			nodes2getTree.push_back(trans.second);
		}
	}

	map<int, int*> mapTree;

	// build shortest path tree for each nodes in node2getTree, 
	// each tree is stored in array format where each element of array is node parent
	#pragma omp parallel for
	for (int i = 0; i < nodes2getTree.size(); ++i) {
		int * parent = new int[numberOfNodes];
		fill(parent, parent + numberOfNodes, -1);
		inCommingTree(nodes2getTree[i], parent);
		#pragma omp critical
		{
			mapTree[nodes2getTree[i]] = parent;
		}
	}

	double initialProb = 1.0 / listOfTransactions.size();

	#pragma omp parallel for
	for (int i = 0; i < numberOfSamples; ++i) {
		pair<int, int> sampleTrans = listOfSampledTrans[i];
		int idx = listOfSampleIdx[i];
		int length = 0;
		double prob = find(uncutTrans->begin(), uncutTrans->end(), idx) != uncutTrans->end() ? 
			Constants::ALPHA * 1.0/uncutTrans->size() + (1.0 - Constants::ALPHA) * 1.0/listOfTransactions.size():
			(1.0 - Constants::ALPHA) * 1.0 / listOfTransactions.size();
		int u = sampleTrans.first;
		vector<int> path;
		bool * inPath = new bool[numberOfNodes];
		fill(inPath, inPath + numberOfNodes, false);
		path.push_back(u);
		inPath[u] = true;
		int * parent = mapTree[sampleTrans.second];
		do {
			map<int, int> outEdges = currentEdgeWeights[u];
			// if the parent of u is in path already
			if (parent[u] < 0) break;
			if (inPath[parent[u]]) {
				vector<int> tmp;
				for (map<int, int>::iterator it = outEdges.begin(); it != outEdges.end(); ++it)
					if (!inPath[it->first]) tmp.push_back(it->first);
				if (tmp.empty()) break;
				int selected = commonInstance->randomInThread() % tmp.size();
				u = tmp[selected];
				prob *= 1.0 / tmp.size();
			}
			else {
				vector<int> tmp;
				for (map<int, int>::iterator it = outEdges.begin(); it != outEdges.end(); ++it)
					if (!inPath[it->first] && it->first != parent[u]) tmp.push_back(it->first);
				if (tmp.empty()) u = parent[u];
				else {
					double r = (double)(commonInstance->randomInThread() % 10);
					if (r < Constants::ALPHA * 10.0) {
						u = parent[u];
						prob *= Constants::ALPHA;
					}
					else {
						int selected = commonInstance->randomInThread() % tmp.size();
						u = tmp[selected];
						prob *= (1.0 - Constants::ALPHA) / tmp.size();
					}
				}
			}
			length += outEdges[u];
			path.push_back(u);
		} while (u != sampleTrans.second && length < Constants::T);

		if (u == sampleTrans.second && length < Constants::T) {
			SamplePath p;
			p.path = path;
			p.length = length;
			p.prob = prob;
			#pragma omp critical
			{
				samplePaths->push_back(p);
			}
		}

		delete[] inPath;
	}

	for (map<int, int*>::iterator it = mapTree.begin(); it != mapTree.end(); ++it) {
		delete[] it->second;
	}
	delete[] inSamples;
}

vector<pair<int, int>>* Network::getListEdges()
{
	return &listEdges;
}

map<int, map<int, int>>* Network::getMapNodePair2EdgeId()
{
	return &mapNodePair2EdgeId;
}

map<int, map<int, int>>* Network::getMapInversePair2EdgeId()
{
	return &mapInversePair2EdgeId;
}

vector<pair<double, double>>* Network::getListEdgeParameters()
{
	return &listEdgeParameters;
}

void Network::clear()
{
	mapNodeId.clear();
	listOfTransactions.clear();
	listEdges.clear();
	nodeIdx.clear();
	listEdgeParameters.clear();
	mapNodePair2EdgeId.clear();
	mapInversePair2EdgeId.clear();

	if (outgoingEdgeWeights) {
		delete[] outgoingEdgeWeights;
		delete[] currentEdgeWeights;
		delete[] incommingEdgeWeights;
		delete[] currentIncommingEdgeWeights;
		delete[] currentQoSlevel;
		delete[] mapMaxLevel;
		delete[] mapDelayFunc;
	}
}

void Network::findPath(int u, int v, int ** parent, vector<int>* path)
{
	if (u != v) {
		vector<int> path1;
		findPath(u, parent[u][v], parent, &path1);
		for (int i = 0; i < path1.size(); ++i)
			path->push_back(path1[i]);
		vector<int> path2;
		findPath(parent[u][v], v, parent, &path2);
		for (int i = 0; i < path2.size(); ++i)
			path->push_back(path2[i]);
	}
	else
		path->push_back(u);
}

void Network::inCommingTree(int u, int * parent)
{
	vector<int> distance;
	bool * visited = new bool[numberOfNodes];
	for (int i = 0; i < numberOfNodes; i++) {
		distance.push_back(Constants::T);
		visited[i] = false;
	}

	distance[u] = 0;
	AscendingOrder<int> hd(&distance[0]);
	MappedHeap<AscendingOrder<int>> heap(nodeIdx, hd);

	while (!heap.empty()) {
		int nodeId = heap.pop();
		visited[nodeId] = true;

		if (distance[nodeId] >= Constants::T) {
			break;
		}

		map<int, int> mapTmp = currentIncommingEdgeWeights[nodeId];
		for (map<int, int>::iterator it = mapTmp.begin(); it != mapTmp.end(); ++it) {
			int neighborId = it->first;
			int w = it->second;
			if (!visited[neighborId] && distance[neighborId] > distance[nodeId] + w) {
				distance[neighborId] = distance[nodeId] + w;
				parent[neighborId] = nodeId;
				heap.heapify(neighborId);
			}
		}
	}

	delete[] visited;
}

void Network::recalculateShortestPaths(int nodeId, vector<int>* children, vector<int>* re)
{
	re->push_back(nodeId);
	vector<int> * tmp = &children[nodeId];
	for (int i = 0; i < tmp->size(); ++i) {
		recalculateShortestPaths((*tmp)[i], children, re);
	}
}

double Network::getEdgeLinearWeight(int startId, int endId, IloNum edgeLevel)
{
	// TODO
	return ((Constants::DELAY_FUNC == 1? 1:Constants::T) * edgeLevel + 1);
}

int Network::getMaxLevel(int startId, int endId)
{
	return Constants::DELAY_FUNC != 4? Constants::MAX_LEVEL 
		: QoScommon::getInstance()->getMaxLevel(mapDelayFunc[startId][endId], Constants::T);
}

