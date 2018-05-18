#include "search.h"
#include "move.h"
#include "eval.h"
#include "movegen.h"
#include <limits.h>
#include "board.h"
#include "movegen.h"
#include "bitboard.h"
#include "uci.h" //For timing

#ifdef __linux__
#include <stdio.h>
#include "string.h"
#endif

Search::Config* cfg;
void Search::setConfig(Search::Config * config){
	cfg=config;
}

constexpr int futilitySize = 4; 
int futilityMoves[] = {0,150,200,400,600};

bool inline isCapture(BoardInfo * b, Move m){
	//must be prior to playing move
	if(b->whiteToMove){
		return ((getSquare[to_sq(m)] & b->BlackPiecesBB) != 0);
	}else{
		return ((getSquare[to_sq(m)] & b->WhitePiecesBB) != 0);
	}
}

/*
* Returns the best move according to our wonderful engine. A valid config is assumed to be set previously.
*/
Move Search::getBestMove(Board * board){
	calculateMovetime(board);
	return iterativeDeepening(board);
}



/*
* If wtime or btime are being used, calculates the time to use on the current move.
* If infinite search is used, the movetime is INT_MAX. 
*/ 
void Search::calculateMovetime(Board* b){
	Config * config = cfg;
	if(config->depth != 0){
		config->movetime = INT_MAX;
	}
	if(config->infinite){
		config->movetime=INT_MAX;
	}
	BoardInfo * info = b->currentBoard();
	int moveNum = info->moveNumber;
	int toMoveTime = 0;
	if(config->wTime != 0 || config->bTime != 0){
		if(b->currentBoard()->whiteToMove){
			toMoveTime = config->wTime;
		}else{
			toMoveTime = config->bTime;
		}
		int expectedMoves = 0;
		if(moveNum < 10){
			expectedMoves=50;
		}else if(moveNum < 30){
			expectedMoves = 65;
		}else if(moveNum < 50){
			expectedMoves = 80;
		}else if(moveNum < 70){
			expectedMoves = 90;
		}else if(moveNum < 100){
			expectedMoves = 110;
		}else if(moveNum < 200){
			expectedMoves = 210;
		}else{
			expectedMoves = 320;
		}	
		int movesToGo = expectedMoves-moveNum;
		config->movetime = toMoveTime/movesToGo;
	}
	cfg->endTime = get_wall_time()+((cfg->movetime)/1000.0);
}
//Used for move ordering and getting pv
Search::LINE lastPv;

//Used for move ordering
U64 whiteHeuristic[64][64]={{0}};
U64 blackHeuristic[64][64]={{0}};
constexpr int MAX_DEPTH = 200;
Move killerMoves[MAX_DEPTH][2] = {};

int startDepth=0; //Used to determine how long until we hit horizon in an alphabeta search
double endTime; //Used to determine when to leave deepening
int score = 0; //Used to communicate the score of the best move returned by ab search.

U64 nodeCount = 0; //Just nodes where eval is called

bool currentlyFollowingPv;
bool canDoNullMove;
const int nullMoveEndgame = 350;
/*
* The main search function. 
*/
Move Search::iterativeDeepening(Board * board){
	
	endTime = cfg->endTime;
	char buffer[100];
	Move bestMove=-1;
	double startTime = get_wall_time();
	
	//I wonder what it would be like to know c++ and use efficient techniques rather than this?
	for(int i = 0; i < 64; i++){
		for(int j = 0; j < 64; j++){
			whiteHeuristic[i][j]=0;
			blackHeuristic[i][j]=0;
		}
	}
	for(int depth = 1; depth < MAX_DEPTH; depth++){
		nodeCount = 0;
		LINE line;
		currentlyFollowingPv = true;
		canDoNullMove = true;
		//Move curMove = getAlphabetaMove(board, depth, &line);
		Move curMove = alphabetaHelper(board,INT_MIN+500,INT_MAX-500,depth,&line);
		curMove = line.argmove[0];
		if(get_wall_time() >= endTime){
			return bestMove;
		}
		bestMove = curMove;
		
		//Print info about the search we just did
		//Not really the true nps.
		if(!board->isCheckmate()){
			printf("info depth %i score cp %i nodes %llu nps %lu time %i pv", depth, score,nodeCount, (int) (nodeCount/(get_wall_time() - startTime)),(int) ((get_wall_time() - startTime)*1000));
		}
		for(int j = 0; j < line.cmove; j++){
			printf(" %s",UCI::getMoveString(line.argmove[j],buffer));
		}
		printf("\n");
		
		
		if(cfg->depth!=0){
			if(cfg->depth <= depth){
				return bestMove;
			}
		}
		
		lastPv = line;
		//Check uci stop command?
	}
	cfg->movetime=0;
	cfg->depth=0;
	cfg->wTime = 0;
	cfg->bTime = 0;
	
	return bestMove;
}


int Search::alphabetaHelper(Board * board, int alpha, int beta, int depth, LINE * pline){
	LINE line;
	int alphaHits = 0;
	Move bestMove;
	
	int curEval = Eval::evaluate(board);

	if(depth <= 0 || board->isCheckmate() || board->isDraw()){
		currentlyFollowingPv=false;
		if(depth <= 0){
			pline->cmove = 0;
		}else{
			score = curEval;
			return curEval;
		}
		nodeCount++;
		return quiesce(board,alpha,beta);
	}
	
	if(get_wall_time() >= endTime){
		return INT_MIN;
	}
	//Hope to prune!
	if(Search::isPositionFutile(board,alpha,beta,startDepth-depth,depth)){
		return beta;
	}
	
	//Null Move
	if(canDoNullMove && !currentlyFollowingPv && board->currentSideMaterial() > 1000 && !board->isOwnKingInCheck() && depth > 2){
		LINE useless;
		canDoNullMove=false;
		board->makeNullMove();
		int reduction = 3;
		if(depth > 5){
			reduction = 4;
		}
		int val = -alphabetaHelper(board, -beta, -alpha, depth-reduction, &useless);
		board->undoMove();
		canDoNullMove=true;
		if(val >= beta){return beta;}
	}

	canDoNullMove=true;

	ExtMove moves[MAX_MOVES];
	U8 moveCount = getAllLegalMoves(board,moves);
	bool whiteToMove = board->currentBoard()->whiteToMove;
	for(int i = 0; i < moveCount; i++){
		orderMoves(moves,board,moveCount,startDepth-depth,depth,i,whiteToMove);

		//if(isMoveFutile(board,startDepth-depth,depth,i,moves[i],alpha,beta,curEval)){
		//	continue;
		//}

		board->makeMove(moves[i].move);
		int val;
		
		//Attempt at LMR -- no LMR in endgame because moves that appear suboptimal are often better than expected.
		//Note: LMR can sometimes cause pv lines to be too short. A problem.
		//if(i < 2*moveCount/3 || depth < 3 || board->currentSideMaterial() < 1000){
			val = -alphabetaHelper(board, -beta, -alpha, depth-1, &line);
		//}else if(i < 3*moveCount/4) {
	//		val = -alphabetaHelper(board, -beta, -alpha, depth-2, &line);
		//}
		board->undoMove();
		
		//Beta Cutoff
		if(val >= beta){
			//History Heuristic Update
			if(whiteToMove){
				whiteHeuristic[from_sq(moves[i])][to_sq(moves[i])] += depth*depth;
			}else{
				blackHeuristic[from_sq(moves[i])][to_sq(moves[i])] += depth*depth;
			}
			
			//Killer move update
			killerMoves[depth][1] = killerMoves[depth][0];
			killerMoves[depth][0] = moves[i].move;
			
			score = beta;
			return beta;
		}
		//Update the pv
		if(val > alpha){
			alpha = val;
			pline->argmove[0] = moves[i].move;
			memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(Move));
			pline->cmove = line.cmove + 1;
			alphaHits++;
			bestMove = moves[i].move;
		}
	}
	if(alphaHits != 0){
		if(whiteToMove){
				whiteHeuristic[from_sq(bestMove)][to_sq(bestMove)] += depth*depth;
		}else{
			blackHeuristic[from_sq(bestMove)][to_sq(bestMove)] += depth*depth;
		}
	}
	score = alpha;
	return alpha;
}
//Ignoring in the pv for now.
int Search::quiesce(Board * board, int alpha, int beta){
	int curEval = Eval::evaluate(board);

	if(curEval >= beta){
		return beta;
	}
	if(curEval > alpha){
		alpha = curEval;
	}
	

	ExtMove moves[MAX_MOVES];
	U8 moveCount = Movegen::getAllCaptures(board,moves);
	for(int i = 0; i < moveCount; i++){
		board->makeMove(moves[i].move);
		int score = -quiesce(board, -beta, -alpha);
		board->undoMove();
		if(score >= beta){
			return beta;
		}
		if(alpha > score){
			alpha = curEval;
		}
	}
	return alpha;
}
void Search::orderMoves(ExtMove moves[], Board * board, int numMoves, int curDepthSearched, int depth, int idx, bool whiteToMove){
	//if tt:probe(board) 
	
	//Order by pv move without testing if we are in the pv.
	if(currentlyFollowingPv && depth > 1){
		for(int i = idx; i < numMoves; i++){
			if(lastPv.argmove[curDepthSearched] == moves[i].move){
				ExtMove temp = moves[i];
				moves[i] = moves[idx];
				moves[idx] = temp;
				return;
			}
		}
	}
	
	int max = moves[idx].score;
	int maxIdx = idx;
	for(int i = idx+1; i < numMoves; i++){
		if(killerMoves[depth][0] ==moves[i].move || killerMoves[depth][1] == moves[i].move){
			ExtMove temp = moves[i];
			moves[i] = moves[idx];
			moves[idx] = temp;
			return;
		}
		if(moves[i].score == 0){
			if(whiteToMove){
				moves[i].score = whiteHeuristic[from_sq(moves[i].move)][to_sq(moves[i].move)];
			}else{
				moves[i].score = blackHeuristic[from_sq(moves[i].move)][to_sq(moves[i].move)];	
			}
		}
		if(moves[i].score > max){
			max = moves[i].score;
			maxIdx = i;
		}
	}
	if(maxIdx > idx){
		ExtMove temp = moves[maxIdx];
		moves[maxIdx] = moves[idx];
		moves[idx] = temp;
	}

	return;
}

/*
* Called before movegenning. Basically tests if it's feasible for any move to make it bad enough that the value is below beta.
*/
//TODO make sure this doesn't screw things up.
bool Search::isPositionFutile(Board *b, int alpha, int beta, int depthSearched, int depthToGo){
	if(depthToGo > futilitySize || depthSearched == 0){
		return false;
	}
	if(b->isOwnKingInCheck()){
		return false;
	}
	int curEval = Eval::evaluate(b);
	int futilityValue = futilityMoves[depthToGo];
	return curEval-futilityValue > beta;
}
/*
* Implements futility pruning using futilityMoves[]. 
* Method is called before calling alphabeta on the move being considered.
* Note, if one non-capture is futile they all must be.
*/
//Maybe in the if start futility pruning at 1 rather than passing moves searched 
bool Search::isMoveFutile(Board * b, int depthSearched, int depthToGo, int movesSearched, Move move, int alpha, int beta, int curEval){
	if(depthToGo > futilitySize || depthSearched == 0){//4 is size of futilityMoves
		return false;
	}
	if(movesSearched == 0){
		return false;
	}
	if(b->isOwnKingInCheck()){
		return false;
	}
	if(b->isMoveCheck(move)){
		return false;
	}
	BoardInfo * boardInfo = b->currentBoard();
	
	//Pushing to 7th rank is scary
	if((PieceMoved(move) == PAWN)){
		if(boardInfo->whiteToMove){
			if(getRank(to_sq(move)) > 6){
				return false;
			}
		}else{
			if(getRank(to_sq(move)) < 3){
				return false;
			}
		}
	}
	//Capture moves are not futile:
	int futilityValue = futilityMoves[depthToGo];

	if(isCapture(boardInfo,move)){return b->staticExchange(move)+curEval+futilityValue < alpha;}
	return (curEval+futilityValue) < alpha;
}