#include "HeuristicBase.h"
#include "BiconnectedComponents.h"


CHeuristicBase::CHeuristicBase()
{
}


CHeuristicBase::~CHeuristicBase()
{
}

TMove CHeuristicBase::getFirstMoveOfTheLongestPath(const int boardData[], const Pos2D &pos, const int depth)
{
	static int board[BOARD_SIZE];
	memcpy(board, boardData, BOARD_SIZE*sizeof(int));
	int maxLength = -1;
	int iMax = 0;

	vector<TMove> avalableMoves = getAvailableMoves(boardData, pos);
	if (avalableMoves.size() == 1)
		return avalableMoves[0];

	for (unsigned int iMove = 0; iMove < avalableMoves.size(); iMove++){
		TMove i = avalableMoves[iMove];
		bool bOk = move(board, pos, i); assert(bOk);
		int length = getEstimatedLengthOfTheLongestPath(board, pos.move(i));
		if (length > maxLength){
			iMax = i;
			maxLength = length;
		}
		bOk = move(board, pos.move(i), getOpositeDirection(i), true);
		assert(bOk);
	}
	DEBUG(cout << "estimated length = " << maxLength << endl);
	if (maxLength < 15)
		return getALongestPath(boardData, pos)[0];
	else
		return iMax;
}

int CHeuristicBase::getEstimatedLengthOfTheLongestPath(int const board[], const Pos2D &playerPos)
{
	return CBiconnectedComponents::getEstimatedLength(board, playerPos);
}

// return the player will win : PLAYER_1 or PLAYER_2 or OBSERVER if unknown
int CHeuristicBase::evaluateBoard(const int _board[121], const Pos2D &_player1, const Pos2D &_player2, const TPlayer next) {
	assert(getBlock(_board, _player1) == BLOCK_PLAYER_1);
	assert(getBlock(_board, _player2) == BLOCK_PLAYER_2);
	if (isIsolated(_board, _player1, _player2) == -1){
		int length1 = CHeuristicBase::getEstimatedLengthOfTheLongestPath(_board, _player1);
		int length2 = CHeuristicBase::getEstimatedLengthOfTheLongestPath(_board, _player2);
		if (length1 > length2)
			return PLAYER_1;
		else if (length1 < length2)
			return PLAYER_2;
		else if (next == PLAYER_1) // length1 = length2
			return PLAYER_2;
		else
			return PLAYER_1;
	}
	return OBSERVER;
}

int CHeuristicBase::rateBoard2(int board[], const Pos2D &_p1, const Pos2D &_p2, const TPlayer next){
#ifdef _DEBUG
	int backup[BOARD_SIZE];
	memcpy(backup, board, BOARD_SIZE*sizeof(int));
#endif // _DEBUG

	// not isolated mode, not a leaf node
	assert(next == PLAYER_1 || next == PLAYER_2);
	assert(getBlock(board, _p1) == BLOCK_PLAYER_1 && getBlock(board, _p2) == BLOCK_PLAYER_2);
	int result = 0;
	int n1 = 0, n2 = 0;

	static int distanceBoard1[BOARD_SIZE], distanceBoard2[BOARD_SIZE];
	static int board1[BOARD_SIZE], board2[BOARD_SIZE];
	static int oBoard1[BOARD_SIZE], oBoard2[BOARD_SIZE];
	memcpy(distanceBoard1, board, BOARD_SIZE*sizeof(int));
	memcpy(board1, board, BOARD_SIZE*sizeof(int));
	memcpy(distanceBoard2, board, BOARD_SIZE*sizeof(int));
	memcpy(board2, board, BOARD_SIZE*sizeof(int));
	fillDistance(distanceBoard1, _p1);
	fillDistance(distanceBoard2, _p2);

	for (int i = 0; i < BOARD_SIZE; i++){
		if (distanceBoard1[i] > SPECIAL_BLOCK && distanceBoard2[i] > SPECIAL_BLOCK){
			if (distanceBoard1[i] > distanceBoard2[i])
			{
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else if (distanceBoard1[i] < distanceBoard2[i]){
				board2[i] = BLOCK_ENEMY_AREA;
			}
			else if (next == PLAYER_1){
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else
				board2[i] = BLOCK_ENEMY_AREA;
		}
	}

	n1 = CBiconnectedComponents::getEstimatedLength(board1, _p1);
	n2 = CBiconnectedComponents::getEstimatedLength(board2, _p2);
	DEBUG(cout << "n1/n2 = " << n1 << "/" << n2 << endl);
	result = n1 *(MAX_POINTS - MIN_POINTS) / (n1 + n2) + MIN_POINTS;
#ifdef _DEBUG
	assert(memcmp(board, backup, BOARD_SIZE*sizeof(int)) == 0);
#endif // _DEBUG

	return result;
}

int CHeuristicBase::rateBoard(int board[], const Pos2D &_p1, const Pos2D &_p2, const TPlayer next){
#ifdef _DEBUG
	int backup[BOARD_SIZE];
	memcpy(backup, board, BOARD_SIZE*sizeof(int));
#endif // _DEBUG

	// not isolated mode, not a leaf node
	assert(next == PLAYER_1 || next == PLAYER_2);
	assert(getBlock(board, _p1) == BLOCK_PLAYER_1 && getBlock(board, _p2) == BLOCK_PLAYER_2);
	int result = 0;
	int n1 = 0, n2 = 0;

	static int distanceBoard1[BOARD_SIZE], distanceBoard2[BOARD_SIZE];
	static int board1[BOARD_SIZE], board2[BOARD_SIZE];
	static int oBoard1[BOARD_SIZE], oBoard2[BOARD_SIZE];
	memcpy(distanceBoard1, board, BOARD_SIZE*sizeof(int));
	memcpy(board1, board, BOARD_SIZE*sizeof(int));
	memcpy(distanceBoard2, board, BOARD_SIZE*sizeof(int));
	memcpy(board2, board, BOARD_SIZE*sizeof(int));
	fillDistance(distanceBoard1, _p1);
	fillDistance(distanceBoard2, _p2);

	for (int i = 0; i < BOARD_SIZE; i++){
		if (distanceBoard1[i] > SPECIAL_BLOCK && distanceBoard2[i] > SPECIAL_BLOCK){
			if (distanceBoard1[i] > distanceBoard2[i])
			{
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else if (distanceBoard1[i] < distanceBoard2[i]){
				board2[i] = BLOCK_ENEMY_AREA;
			}
			else if (next == PLAYER_1){
				board1[i] = BLOCK_ENEMY_AREA;
			}
			else
				board2[i] = BLOCK_ENEMY_AREA;
		}
	}

	n1 = calculatePotentialPoint(board1, _p1, oBoard1, distanceBoard1);
	n2 = calculatePotentialPoint(board2, _p2, oBoard2, distanceBoard2);
	DEBUG(cout << "n1/n2 = " << n1 << "/" << n2 << endl);
	result = n1*(MAX_POINTS - MIN_POINTS) / (n1 + n2) + MIN_POINTS;
#ifdef _DEBUG
	assert(memcmp(board, backup, BOARD_SIZE*sizeof(int)) == 0);
#endif // _DEBUG

	return result;
}


void CHeuristicBase::constructNewGraphUsedInRateBoard(const int oBoard1[], const Pos2D &_p1, vector<Area> &areas, int* numberOfConnectionsToEnemyArea, set<Edge> &edgesOfCode)
{
	static bool visited[BOARD_SIZE];
	memset(visited, 0, BOARD_SIZE);
	int startArea = findCode(oBoard1[_p1.to1D()]);
	vector<bool> foundAreas(30, false);

	queue<Pos1D> queueOfAreas;
	queueOfAreas.push(_p1.to1D());
	while (!queueOfAreas.empty()){
		Pos1D a = queueOfAreas.front();
		queueOfAreas.pop();
		foundAreas[findCode(oBoard1[a])] = true;

		queue<Pos1D> queueInAArea;
		queueInAArea.push(a);
		while (!queueInAArea.empty()){
			Pos1D v = queueInAArea.front();
			queueInAArea.pop();

			for (TMove direction = 1; direction <= 4; direction++){
				// check the specialty of the block
				int block = getBlock(oBoard1, Pos2D(v).move(direction));
				if ((block < SPECIAL_BLOCK || block == BLOCK_OUT_OF_BOARD) && block != BLOCK_ENEMY_AREA)
					continue;
				Pos1D u = Pos2D(v).move(direction).to1D();
				if (visited[u])
					continue;

				if (oBoard1[u] == oBoard1[v]){// same area
					queueInAArea.push(u);
					visited[u] = true;
				}
				else if (block != BLOCK_ENEMY_AREA){ // maybe found a new area?
					int code = findCode(block);
					if (!foundAreas[code]){
						foundAreas[code] = true;
						queueOfAreas.push(u);
					}
					int code1 = findCode(oBoard1[u]);
					int code2 = findCode(oBoard1[v]);
					edgesOfCode.insert(Edge(code1, code2));
					AdjArea adjArea;
					adjArea.codeOfAdjArea = code2;
					adjArea.connections = u;
					areas[code1].adjAreas.insert(adjArea);

					adjArea.codeOfAdjArea = code1;
					adjArea.connections = v;
					areas[code2].adjAreas.insert(adjArea);
				}
				else { // block == BLOCK_ENEMY_AREA
					numberOfConnectionsToEnemyArea[findCode(oBoard1[v])]++;
					edgesOfCode.insert(Edge(findCode(oBoard1[v]), -1));
					AdjArea adjArea;
					adjArea.codeOfAdjArea = -1;
					adjArea.connections = v;
					areas[findCode(oBoard1[v])].adjAreas.insert(adjArea);
				}
			}
		}
	}
}

int CHeuristicBase::calculatePotentialPoint(const int * board, const Pos2D & playerPos, int * oBoard, int * distanceBoard)
{
	vector<Area> areas = CBiconnectedComponents::biconnectedComponents(board, playerPos, oBoard);
	// construct areas' connections with each other and enemy area (-2 block)
	set<Edge> edgesOfCode;
	static int numberOfConnectionsToEnemyArea[MAXIMUM_NUMBER_OF_AREAS];
	static vector<int> areasCode(MAXIMUM_NUMBER_OF_AREAS);
	areasCode.clear();
	memset(numberOfConnectionsToEnemyArea, 0, MAXIMUM_NUMBER_OF_AREAS * sizeof(int));
	constructNewGraphUsedInRateBoard(oBoard, playerPos, areas, numberOfConnectionsToEnemyArea, edgesOfCode);
	int max_ = *max_element(numberOfConnectionsToEnemyArea, numberOfConnectionsToEnemyArea + areas.size());
	for (unsigned int i = 0; i < areas.size(); i++){
		if (numberOfConnectionsToEnemyArea[i] == max_)
			areasCode.push_back(i);
	}
	assert(areasCode.size() > 0);
	static vector<Pos1D> verticesInTheAreaWithMinDistance(BOARD_SIZE);
	verticesInTheAreaWithMinDistance.clear();

	for (vector<int>::iterator iArea = areasCode.begin(); iArea != areasCode.end(); iArea++){
		// find the minDistance between p and each vertex in the areas[*iArea]
		int minDistance = SPECIAL_BLOCK + 200;
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

	int maxLength = -1;
	for (vector<int>::iterator iVertex = verticesInTheAreaWithMinDistance.begin();
		iVertex != verticesInTheAreaWithMinDistance.end(); iVertex++){
		static int board2[BOARD_SIZE];
		memcpy(board2, board, BOARD_SIZE*sizeof(int));
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
	return maxLength;
}