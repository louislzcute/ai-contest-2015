#include "HeuristicBase.h"
#include "BiconnectedComponents.h"
#include "..\AI_Template\MyAI.h"
#include "MyTimer.h"


CHeuristicBase::CHeuristicBase()
{
}

CHeuristicBase::~CHeuristicBase()
{
}

void CHeuristicBase::exploreToPathLongestPath(TBlock board[], Pos2D &pos, vector<TMove> &cPath, vector<TMove> &oldPath, int &length, int depth)
{
	assert(getBlock(board, pos) == BLOCK_PLAYER_1 || getBlock(board, pos) == BLOCK_PLAYER_2);
	assert(depth >= 0);
	auto availMoves = getAvailableMoves(board, pos);
	bool bOk;
	if (depth == 0 || availMoves.size() == 0 || CMyTimer::getInstance()->timeUp()) {
		int l = CHeuristicBase::getEstimatedLengthOfTheLongestPath(board, pos);
		if (cPath.size() + l > oldPath.size() + length){
			length = l;
			oldPath = vector<TMove>(cPath);
		}
	}
	else
		for (unsigned int i = 0; i < availMoves.size(); i++){
			bOk = move(board, pos, availMoves[i]); assert(bOk);
			pos = pos.move(availMoves[i]);
			cPath.push_back(availMoves[i]);

			exploreToPathLongestPath(board, pos, cPath, oldPath, length, depth - 1);

			TMove back = getOpositeDirection(availMoves[i]);
			bOk = move(board, pos, back, true); assert(bOk);
			pos = pos.move(back);
			cPath.pop_back();
		}
}

TMove CHeuristicBase::getFirstMoveOfTheLongestPath(const TBlock boardData[], const Pos2D &pos, const int nDepth /*= 0*/)
{
	TBlock board[BOARD_SIZE];
	memcpy(board, boardData, BOARD_SIZE*sizeof(TBlock));
	vector<TMove> c(nDepth), l(nDepth);
	c.clear();
	l.clear();
	int length = 0;
	Pos2D p = pos;
	exploreToPathLongestPath(board, p, c, l, length, nDepth);
	assert(l.size() > 0);
	return l[0];
}

vector<TMove> CHeuristicBase::getFirstMovesOfTheLongestPath(const TBlock boardData[], const Pos2D &pos)
{
	TBlock board[BOARD_SIZE];
	memcpy(board, boardData, BOARD_SIZE*sizeof(TBlock));
	int iMax = 0;
	vector<TMove> result(4);
	result.clear();

	bool bMoves[4];
	{
		vector<TMove> availMoves = getAvailableMoves(boardData, pos, bMoves);
		if (availMoves.size() == 1) {
			result.push_back(availMoves[0]);
		}
	}

	int lengths[4] = { -1 };
	int maxLength = -1;
	for (unsigned int iMove = 1; iMove <= 4; iMove++){
		if (!bMoves[iMove - 1])
			continue;

		bool bOk = move(board, pos, iMove); assert(bOk);
		lengths[iMove - 1] = getEstimatedLengthOfTheLongestPath(board, pos.move(iMove));
		if (lengths[iMove - 1] > maxLength){
			iMax = iMove;
			maxLength = lengths[iMove - 1];
		}
		bOk = move(board, pos.move(iMove), getOpositeDirection(iMove), true);
		assert(bOk);
	}
	DEBUG(cout << "estimated length = " << maxLength << endl);
	for (unsigned int i = 0; i < 4; i++){
		if (lengths[i] == maxLength)
			result.push_back(i + 1);
	}
	return result;
}

int CHeuristicBase::getEstimatedLengthOfTheLongestPath(TBlock const board[], const Pos2D &playerPos)
{
	return CBiconnectedComponents::getEstimatedLength(board, playerPos);
}

// return the player will win : PLAYER_1 or PLAYER_2 or OBSERVER if unknown
int CHeuristicBase::evaluateBoard(const TBlock _board[121], const Pos2D &_player1, const Pos2D &_player2, const TPlayer next, int &point) {
	assert(getBlock(_board, _player1) == BLOCK_PLAYER_1);
	assert(getBlock(_board, _player2) == BLOCK_PLAYER_2);
	if (isIsolated(_board, _player1, _player2)){
		int length1 = CHeuristicBase::getEstimatedLengthOfTheLongestPath(_board, _player1);
		int length2 = CHeuristicBase::getEstimatedLengthOfTheLongestPath(_board, _player2);
		if (length1 > length2) {
			point = (length1 - length2 + 1)*(MAX_POINTS - MIN_POINTS) / 4 / (max(length1, length2));
			return PLAYER_1;
		}
		else if (length1 < length2) {
			point = (length2 - length1 + 1)*(MAX_POINTS - MIN_POINTS) / 4 / (max(length1, length2));
			return PLAYER_2;
		}
		else if (next == PLAYER_1) {
			point = (MAX_POINTS - MIN_POINTS) / 4 / (length1 + 2);
			return PLAYER_2;
		}
		else {
			point = (MAX_POINTS - MIN_POINTS) / 4 / (length1 + 2);
			return PLAYER_1;
		}
	}
	return OBSERVER;
}

int CHeuristicBase::simpleRateBoard(TBlock board[], const Pos2D &_p1, const Pos2D &_p2, const TPlayer next){
#ifdef _DEBUG
	TBlock backup[BOARD_SIZE];
	memcpy(backup, board, BOARD_SIZE*sizeof(TBlock));
#endif // _DEBUG

	// not isolated mode, not a leaf node
	assert(next == PLAYER_1 || next == PLAYER_2);
	assert(getBlock(board, _p1) == BLOCK_PLAYER_1 && getBlock(board, _p2) == BLOCK_PLAYER_2);
	int result = 0;
	int n1 = 0, n2 = 0;

	static TBlock dBoard1[BOARD_SIZE], dBoard2[BOARD_SIZE];
	static TBlock board1[BOARD_SIZE], board2[BOARD_SIZE];

	memcpy(dBoard1, board, BOARD_SIZE*sizeof(TBlock));
	memcpy(dBoard2, board, BOARD_SIZE*sizeof(TBlock));
	fillDistance(dBoard1, _p1);
	fillDistance(dBoard2, _p2);

	memcpy(board1, board, BOARD_SIZE*sizeof(TBlock));
	memcpy(board2, board, BOARD_SIZE*sizeof(TBlock));

	for (int i = 0; i < BOARD_SIZE; i++){
		if (getBit(dBoard1[i], SPECIAL_BIT) && getBit(dBoard2[i], SPECIAL_BIT)){
			if (dBoard1[i] > dBoard2[i])
			{
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else if (dBoard1[i] < dBoard2[i]){
				board2[i] = BLOCK_ENEMY_AREA;
			}
			else if (next == PLAYER_1){
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else
				board2[i] = BLOCK_ENEMY_AREA;
		}
	}

	dBoard1[_p1] = BLOCK_PLAYER_1;
	dBoard2[_p2] = BLOCK_PLAYER_2;

	n1 = CBiconnectedComponents::getEstimatedLength(board1, _p1);
	n2 = CBiconnectedComponents::getEstimatedLength(board2, _p2);
	if (n1 + n2 == 0){
		if (next == PLAYER_1)
			return (MAX_POINTS + 3 * MIN_POINTS) / 4 - 1;
		else
			return (MAX_POINTS * 3 + MIN_POINTS) / 4 + 1;
	}
	DEBUG(cout << "n1/n2 = " << n1 << "/" << n2 << endl);
	result = n1 *(MAX_POINTS - MIN_POINTS) / (n1 + n2) / 2 + MIN_POINTS / 2;
#ifdef _DEBUG
	assert(memcmp(board, backup, BOARD_SIZE*sizeof(TBlock)) == 0);
#endif // _DEBUG

	return result;
}

int CHeuristicBase::voronoiRateBoard(TBlock board[], const Pos2D &_p1, const Pos2D &_p2, const int next)
{
#ifdef _DEBUG
	TBlock backup[BOARD_SIZE];
	memcpy(backup, board, BOARD_SIZE*sizeof(TBlock));
#endif // _DEBUG

	// not isolated mode, not a leaf node
	assert(next == PLAYER_1 || next == PLAYER_2);
	assert(getBlock(board, _p1) == BLOCK_PLAYER_1 && getBlock(board, _p2) == BLOCK_PLAYER_2);
	int result = 0;
	int n1 = 0, n2 = 0;

	static TBlock dBoard1[BOARD_SIZE], dBoard2[BOARD_SIZE];
	memcpy(dBoard1, board, BOARD_SIZE*sizeof(TBlock));
	memcpy(dBoard2, board, BOARD_SIZE*sizeof(TBlock));
	fillDistance(dBoard1, _p1);
	fillDistance(dBoard2, _p2);

	for (int i = 0; i < BOARD_SIZE; i++){
		if (getBit(dBoard1[i], SPECIAL_BIT) && getBit(dBoard2[i], SPECIAL_BIT)){
			if (dBoard1[i] > dBoard2[i])
			{
				n1++;
			}
			else if (dBoard1[i] < dBoard2[i]){
				n2++;
			}
			else if (next == PLAYER_1){
				n1++;
			}
			else
			{
				n2++;
			}
		}
	}
	DEBUG(cout << "n1/n2 = " << n1 << "/" << n2 << endl);
	if (n1 + n2 != 0)
		result = n1 *(MAX_POINTS - MIN_POINTS) / (n1 + n2) / 2 + MIN_POINTS / 2;
	else if (next == PLAYER_2)
		result = (MAX_POINTS * 3 + MIN_POINTS) / 4 + 1;
	else
		result = (MAX_POINTS + 3 * MIN_POINTS) / 4 - 1;
#ifdef _DEBUG
	assert(memcmp(board, backup, BOARD_SIZE*sizeof(TBlock)) == 0);
#endif // _DEBUG

	return result;
}

int CHeuristicBase::treeOfChambersRateBoard(TBlock board[], const Pos2D &_p1, const Pos2D &_p2, const TPlayer next){
#ifdef _DEBUG
	TBlock backup[BOARD_SIZE];
	memcpy(backup, board, BOARD_SIZE*sizeof(TBlock));
#endif // _DEBUG

	// not isolated mode, not a leaf node
	assert(!isIsolated(board, _p1, _p2));
	assert(next == PLAYER_1 || next == PLAYER_2);
	assert(getBlock(board, _p1) == BLOCK_PLAYER_1 && getBlock(board, _p2) == BLOCK_PLAYER_2);
	int result = 0;
	int n1 = 0, n2 = 0;

	static TBlock dBoard1[BOARD_SIZE], dBoard2[BOARD_SIZE];
	static TBlock board1[BOARD_SIZE], board2[BOARD_SIZE];
	static TBlock oBoard1[BOARD_SIZE], oBoard2[BOARD_SIZE];
	memcpy(dBoard1, board, BOARD_SIZE*sizeof(TBlock));
	memcpy(board1, board, BOARD_SIZE*sizeof(TBlock));
	memcpy(dBoard2, board, BOARD_SIZE*sizeof(TBlock));
	memcpy(board2, board, BOARD_SIZE*sizeof(TBlock));
	fillDistance(dBoard1, _p1);
	fillDistance(dBoard2, _p2);

	for (int i = 0; i < BOARD_SIZE; i++){
		if (dBoard1[i] > SPECIAL_BLOCK && dBoard2[i] > SPECIAL_BLOCK){
			if (dBoard1[i] > dBoard2[i])
			{
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else if (dBoard1[i] < dBoard2[i]){
				board2[i] = BLOCK_ENEMY_AREA;
			}
			else if (next == PLAYER_1){
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else
				board2[i] = BLOCK_ENEMY_AREA;
		}
	}
	board1[_p2] = BLOCK_ENEMY_AREA;
	board2[_p1] = BLOCK_ENEMY_AREA;

	if (n1 + n2 == 0){
		if (next == PLAYER_1)
			return (MAX_POINTS + 3 * MIN_POINTS) / 4 - 1;
		else
			return (MAX_POINTS * 3 + MIN_POINTS) / 4 + 1;
	}

	DEBUG(cout << "n1/n2 = " << n1 << "/" << n2 << endl);
	result = n1 *(MAX_POINTS - MIN_POINTS) / (n1 + n2) / 2 + MIN_POINTS / 2;
#ifdef _DEBUG
	assert(memcmp(board, backup, BOARD_SIZE*sizeof(TBlock)) == 0);
#endif // _DEBUG

	return result;
}


/*
void CHeuristicBase::constructNewGraphUsedInRateBoard(const TBlock oBoard[], const Pos2D &_p, vector<Area> &areas, int* numberOfConnectionsToEnemyArea, set<Edge> &edgesOfCode)
{
static bool visited[BOARD_SIZE];
memset(visited, 0, BOARD_SIZE);
int startArea = findCode(oBoard[_p.to1D()]);
vector<bool> foundAreas(MAX_N_AREAS, false);

queue<Pos1D> queueOfAreas;
queueOfAreas.push(_p.to1D());
while (!queueOfAreas.empty()){
Pos1D a = queueOfAreas.front();
queueOfAreas.pop();
foundAreas[findCode(oBoard[a])] = true;

queue<Pos1D> queueInAArea;
queueInAArea.push(a);
while (!queueInAArea.empty()){
Pos1D v = queueInAArea.front();
queueInAArea.pop();

for (TMove direction = 1; direction <= 4; direction++){
// check the specialty of the block
TBlock block = getBlock(oBoard, Pos2D(v).move(direction));
if ((block < SPECIAL_BLOCK || block == BLOCK_OUT_OF_BOARD) && block != BLOCK_ENEMY_AREA)
continue;
Pos1D u = Pos2D(v).move(direction).to1D();
if (visited[u])
continue;

if (oBoard[u] == oBoard[v]){// same area
queueInAArea.push(u);
visited[u] = true;
}
else if (block != BLOCK_ENEMY_AREA){ // maybe found a new area?
int code = findCode(block);
if (!foundAreas[code]){
foundAreas[code] = true;
queueOfAreas.push(u);
}
int code1 = findCode(oBoard[u]);
int code2 = findCode(oBoard[v]);
edgesOfCode.insert(Edge(code1, code2));
AdjArea adjArea;
adjArea.codeOfAdjArea = code2;
adjArea.connections = u;
areas[code1].adjAreas.insert(adjArea);

adjArea.codeOfAdjArea = code1;
adjArea.connections = v;
areas[code2].adjAreas.insert(adjArea);
}
else { // block == BLOCK_ENEMY_AREA. this is a block of enemy's area
numberOfConnectionsToEnemyArea[findCode(oBoard[v])]++;
edgesOfCode.insert(Edge(findCode(oBoard[v]), -1));
AdjArea adjArea;
adjArea.codeOfAdjArea = -1;
adjArea.connections = v;
areas[findCode(oBoard[v])].adjAreas.insert(adjArea);
}
}
}
}
}

int CHeuristicBase::calculatePotentialPoint(const TBlock * board, const Pos2D & playerPos, TBlock * oBoard, TBlock * distanceBoard)
{
CBiconnectedComponentsOutput output;
CBiconnectedComponents::biconnectedComponents(board, playerPos, &output);
// construct areas' connections with each other and enemy area (-2 block)
set<Edge> edgesOfCode;
static int numberOfConnectionsToEnemyArea[MAX_N_AREAS];

// areasCode store the code of area(s) which have the largest connections with the enemy areas
// and we consider it as the only area(s) with the "battle field"
static vector<int> areasCode(MAX_N_AREAS);
areasCode.clear();
memset(numberOfConnectionsToEnemyArea, 0, MAX_N_AREAS * sizeof(int));

// construct new graph
constructNewGraphUsedInRateBoard(oBoard, playerPos, areas, numberOfConnectionsToEnemyArea, edgesOfCode);

// build the areasCode
int max_ = *max_element(numberOfConnectionsToEnemyArea, numberOfConnectionsToEnemyArea + areas.size());
for (unsigned int i = 0; i < areas.size(); i++){
if (numberOfConnectionsToEnemyArea[i] == max_)
areasCode.push_back(i);
}

assert(max_ > 0);	// max_ == 0 -> the 2 players are isolated
assert(areasCode.size() > 0); // this assert exists because player 1 and player 2 are not isolated

// this vector stores the positions in each area (in areasCode) that have minimum distance to the player pos
static vector<Pos1D> verticesInTheAreaWithMinDistance(BOARD_SIZE);
verticesInTheAreaWithMinDistance.clear();

// calculate the vector verticesInTheAreaWithMinDistance
for (vector<int>::iterator iArea = areasCode.begin(); iArea != areasCode.end(); iArea++){
// find the minDistance between p and each vertex in the areas[*iArea]
TBlock minDistance = SPECIAL_BLOCK + 200i64;
for (int iVertex = 0; iVertex < BOARD_SIZE; iVertex++)
if (areas[*iArea].inTheAreas[iVertex] && (minDistance > distanceBoard[iVertex]))
minDistance = distanceBoard[iVertex];
assert(minDistance != SPECIAL_BLOCK + 200);
// find the vertex that have distance = minDistance
for (int iVertex = 0; iVertex < BOARD_SIZE; iVertex++)
if (areas[*iArea].inTheAreas[iVertex] && (minDistance == distanceBoard[iVertex])){
verticesInTheAreaWithMinDistance.push_back(iVertex);
}
}

// for each position in verticesInTheAreaWithMinDistance; we find a shortest path to it. go to it and
//  calculate maxLength = max(maxLength, estimated length of longest path)
int maxLength = -1;
for (vector<int>::iterator iVertex = verticesInTheAreaWithMinDistance.begin();
iVertex != verticesInTheAreaWithMinDistance.end(); iVertex++){
static TBlock board2[BOARD_SIZE];
memcpy(board2, board, BOARD_SIZE*sizeof(TBlock));
Pos1D v = *iVertex;

Pos2D end(v);
vector<TMove> path = findShortestPath(board, playerPos, end);
Pos2D p = playerPos;
for (unsigned int i = 0; i < path.size(); i++){
bool bOk = move(board2, p, path[i]); assert(bOk);
p = p.move(path[i]);
}
assert(p == end);
maxLength = max(maxLength, (int)path.size() + CBiconnectedComponents::getEstimatedLength(board2, p));
}
assert(maxLength >= 0);

// now we fill up the areas in areasCode with BLOCK_OBSTACLE
TBlock tempBoard[BOARD_SIZE];
memcpy(tempBoard, board, sizeof(TBlock)*BOARD_SIZE);
for (unsigned int j = 0; j < areasCode.size(); j++){
for (unsigned int i = 0; i < BOARD_SIZE; i++){
if (areas[areasCode[j]].inTheAreas[i])
tempBoard[i] = BLOCK_OBSTACLE;
}
}

tempBoard[playerPos.to1D()] = board[playerPos.to1D()];
int maxLengthWithoutBatterField = CHeuristicBase::getEstimatedLengthOfTheLongestPath(tempBoard, playerPos);
maxLength = max(maxLength, maxLengthWithoutBatterField);

return maxLength;
}
*/

void CHeuristicBase::sortMoves(vector<TMove> &moves, TBlock* board, const Pos2D &_p1, const Pos2D &_p2, const TPlayer next)
{
	static CMyAI* ai = CMyAI::getInstance();
	assert(moves.size() <= 4);
	int points[4];
	bool bOk;

	int i = 0;
	for (vector<TMove>::iterator m = moves.begin(); m != moves.end(); m++){
		if (next == PLAYER_1){
			bOk = move(board, _p1, *m, false); assert(bOk);
			points[i] = ai->searcher.heuristic.rateBoard(board, _p1.move(*m), _p2, PLAYER_2);
			bOk = move(board, _p1.move(*m), getOpositeDirection(*m), true); assert(bOk);
		}
		else{
			bOk = move(board, _p2, *m, false); assert(bOk);
			points[i] = ai->searcher.heuristic.rateBoard(board, _p1, _p2.move(*m), PLAYER_1);
			bOk = move(board, _p2.move(*m), getOpositeDirection(*m), true); assert(bOk);
		}

		i++;
	}
	for (unsigned int i = 0; i < moves.size(); i++){
		for (unsigned int j = i + 1; j < moves.size(); j++){
			if (next == PLAYER_1){
				if (points[j] > points[i]){
					int temp = points[i];
					points[i] = points[j];
					points[j] = temp;

					temp = moves[i];
					moves[i] = moves[j];
					moves[j] = temp;
				}
			}
			else {
				if (points[j] < points[i]){
					int temp = points[i];
					points[i] = points[j];
					points[j] = temp;

					temp = moves[i];
					moves[i] = moves[j];
					moves[j] = temp;
				}
			}
		}
	}
}

int CHeuristicBase::pureTreeOfChamber(TBlock board[], const Pos2D &_p1, const Pos2D &_p2, const int next)
{
#ifdef _DEBUG
	TBlock backup[BOARD_SIZE];
	memcpy(backup, board, BOARD_SIZE*sizeof(TBlock));
#endif // _DEBUG

	// not isolated mode, not a leaf node
	assert(!isIsolated(board, _p1, _p2));
	assert(next == PLAYER_1 || next == PLAYER_2);
	assert(getBlock(board, _p1) == BLOCK_PLAYER_1 && getBlock(board, _p2) == BLOCK_PLAYER_2);
	int result = 0;
	int n1 = 0, n2 = 0;

	static TBlock dBoard1[BOARD_SIZE], dBoard2[BOARD_SIZE];
	static TBlock board1[BOARD_SIZE], board2[BOARD_SIZE];
	static TBlock oBoard1[BOARD_SIZE], oBoard2[BOARD_SIZE];
	memcpy(dBoard1, board, BOARD_SIZE*sizeof(TBlock));
	memcpy(board1, board, BOARD_SIZE*sizeof(TBlock));
	memcpy(dBoard2, board, BOARD_SIZE*sizeof(TBlock));
	memcpy(board2, board, BOARD_SIZE*sizeof(TBlock));
	fillDistance(dBoard1, _p1);
	fillDistance(dBoard2, _p2);

	for (int i = 0; i < BOARD_SIZE; i++){
		if (dBoard1[i] > SPECIAL_BLOCK && dBoard2[i] > SPECIAL_BLOCK){
			if (dBoard1[i] > dBoard2[i])
			{
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else if (dBoard1[i] < dBoard2[i]){
				board2[i] = BLOCK_ENEMY_AREA;
			}
			else if (next == PLAYER_1){
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else
				board2[i] = BLOCK_ENEMY_AREA;
		}
	}
	board1[_p2] = BLOCK_ENEMY_AREA;
	board2[_p1] = BLOCK_ENEMY_AREA;

	if (n1 + n2 == 0){
		if (next == PLAYER_1)
			return (MAX_POINTS + 3 * MIN_POINTS) / 4 - 1;
		else
			return (MAX_POINTS * 3 + MIN_POINTS) / 4 + 1;
	}

	DEBUG(cout << "n1/n2 = " << n1 << "/" << n2 << endl);
	result = n1 *(MAX_POINTS - MIN_POINTS) / (n1 + n2) / 2 + MIN_POINTS / 2;
#ifdef _DEBUG
	assert(memcmp(board, backup, BOARD_SIZE*sizeof(TBlock)) == 0);
#endif // _DEBUG

	return result;
}

