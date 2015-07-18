#pragma once
#include "..\AI_Template\include/ai/defines.h"
#include "..\AI_Template\include/ai/AI.h"
#include "..\AI_Template\mydefine.h"
#include "Pos2D.h"

class Edge{
public:
	Pos1D u, v;

	Edge() :u(0), v(0){}
	Edge(Pos1D _v1, Pos1D _v2) : u(_v1), v(_v2) {	}

	Edge operator =(const Edge &e) { u = e.u; v = e.v; return *this; }

	bool operator==(const Edge &e) const {
		return (u == e.u && v == e.v) || (u == e.v && v == e.u);
	}

	bool operator!=(const Edge &e) const {
		return !((u == e.u && v == e.v) || (u == e.v && v == e.u));
	}

	void rearange(){
		if (v < u)
			std::swap(v, u);
	}

	int hash() const{
		int v1 = min(u, v);
		int v2 = max(u, v);
		return v2 * 10000 + v1;
	}

	bool operator < (const Edge& e) const
	{
		return (hash() < e.hash());
	}
};

class AdjArea{
public:
	int codeOfAdjArea;
	Pos1D connections;
	AdjArea operator=(const AdjArea &a){
		codeOfAdjArea = a.codeOfAdjArea;
		connections = a.connections;
	}

	bool operator==(const AdjArea &e) const {
		return codeOfAdjArea == e.codeOfAdjArea && connections == e.connections;
	}

	bool operator<(const AdjArea &e) const {
		return codeOfAdjArea < e.codeOfAdjArea || (codeOfAdjArea == e.codeOfAdjArea && connections < e.connections);
	}

	bool operator>(const AdjArea &e) const {
		return codeOfAdjArea > e.codeOfAdjArea || (codeOfAdjArea == e.codeOfAdjArea && connections > e.connections);
	}
};

class Area{
public:
	int code;
	bool inTheAreas[BOARD_SIZE];
	int nVertices;
	set<AdjArea> adjAreas;

	Area(){
		nVertices = 0;
		memset(inTheAreas, 0, BOARD_SIZE);
	}

	void insert(const Pos1D &u){
		if (!inTheAreas[u]){
			inTheAreas[u] = true;
			nVertices++;
		}
	}

	void erase(const Pos1D &u){
		if (inTheAreas[u]){
			inTheAreas[u] = false;
			nVertices--;
		}
	}

	Area operator =(const Area &e) {
		code = e.code;
		nVertices = e.nVertices;
		memcpy(inTheAreas, e.inTheAreas, BOARD_SIZE);
		adjAreas = set<AdjArea>(e.adjAreas);
		return *this;
	}

	bool operator==(const Area &e) const {
		return nVertices == e.nVertices;
	}

	bool operator>(const Area &e) const {
		return nVertices > e.nVertices;
	}

	bool operator<(const Area &e) const {
		return nVertices < e.nVertices;
	}

	bool operator!=(const Area &e) const {
		return nVertices > e.nVertices;
	}
};

class CBiconnectedComponents
{
public:
	CBiconnectedComponents();
	~CBiconnectedComponents();

	// input
	Pos2D playerPos;
	int* oBoard;
	vector<Area> areas;
	// data

	bool visited[BOARD_SIZE];
	int parrent[BOARD_SIZE];
	int d[BOARD_SIZE];
	int low[BOARD_SIZE];
	int iCount;
	int nComponents;
	stack<Edge> myStack;

	static int getEstimatedLength(int const board[], const Pos2D &playerPos);

	static void constructNewGraph(const Pos2D &playerPos, int * outBoard, set<Edge> &edgesOfCode, vector<Area> &areas);

	static int findLengthOfLongestPath(const int _oBoard[], const vector<Area> &areas, const set<Edge> &edgesOfCode, int startArea, const int &startPos);
	static int calculateLengthOfPath(const int _oBoard[], const vector<Area> &areas, const set<Edge> &edgesOfCode, const vector<int> &path, const int &startPos);
	static void visitNode(const int _oBoard[], const vector<Area> &areas, const set<Edge> &edgesOfCode,
		vector<int> &cPath, int &cLength, vector<int> &lPath, int &lLength, vector<bool> &visitted, const int cCode, const int &startPos);

	static vector<Area> biconnectedComponents(int const board[], const Pos2D &playerPos, int outBoard[]);
	void dfsVisit(const Pos1D & u);
	void createNewArea(const Pos1D &v1, const Pos1D &v2);
	void adjection(bool out[], Pos1D const &u);

	static int rateBoardForAPlayer(int const oBoard[], const Pos2D &playerPos);
};

