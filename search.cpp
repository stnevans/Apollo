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
void Search::init(){
	
}
constexpr int futilitySize = 4; 
int futilityMoves[] = {0,150,200,400,600};


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
	//if(b->isMoveCheck(move)){
	//	return false;
	//}
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
	if(boardInfo->whiteToMove){
		if(getSquare[to_sq(move)] & boardInfo->BlackPiecesBB != 0){
			return false;
		}
	}else{
		if(getSquare[to_sq(move)] & boardInfo->WhitePiecesBB != 0){
			return false;
		}
	}
	int futilityValue = futilityMoves[depthToGo];
	return curEval+futilityValue < alpha;
}
//TODO struct line{move,eval}
int minimaxHelper(Board * b, int depth){
	if(depth == 0 || b->isCheckmate() || b->isDraw()){return Eval::evaluate(b);}
	Move moves[MAX_MOVES];
	U8 moveCount = getAllLegalMoves(b,moves);
	int max = INT_MIN;
	for(int i = 0; i < moveCount; i++){
		b->makeMove(moves[i]);
		int val = -minimaxHelper(b,depth-1);
		b->undoMove();
		if(val > max){
			max = val;
		}
	}
	return max;	
}
Move Search::getMinimaxMove(Board* board){
	Move moves[MAX_MOVES];
	U8 moveCount = getAllLegalMoves(board,moves);
	int max = INT_MIN;
	Move bestMove;
	for(int i = 0; i < moveCount; i++){
		board->makeMove(moves[i]);
		int val = -minimaxHelper(board,cfg->depth-1);
		board->undoMove();
		if(val > max){
			max = val;
			bestMove = moves[i];
		}
	}
	return bestMove;
}
int startDepth=0;
int countForReturn=0;
double endTime;//alpha is lower bound, beta upper.
int Search::alphabetaHelper(Board * board, int alpha, int beta, int depth, LINE * pline){
	LINE line;
	int curEval = Eval::evaluate(board);
	if(depth <= 0 || board->isCheckmate() || board->isDraw()){
		if(depth == 0){
			pline->cmove = 0;
		}
		return curEval;
	}
	//printf("alpha: %i beta: %i\n", alpha, beta);
	if(Search::isPositionFutile(board,alpha,beta,startDepth-depth,depth)){
		//printf("Eval: %i A: %i B: %i\n", curEval,alpha,beta);
		return beta;	
	}
	Move moves[MAX_MOVES];
	U8 moveCount = getAllLegalMoves(board,moves);
	for(int i = 0; i < moveCount; i++){
		//if(isMoveFutile(board,,depth,i,moves[i],alpha,beta,curEval)){
		//	continue;
		//}
		board->makeMove(moves[i]);
		int val = -alphabetaHelper(board, -beta, -alpha, depth-1, &line);
		board->undoMove();
		if(val >= beta){
			return beta;
		}
		if(val > alpha){
			alpha = val;
            pline->argmove[0] = moves[i];
            memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(Move));
            pline->cmove = line.cmove + 1;
		}
	}
	return alpha;
}

int score = 0;
Move Search::getAlphabetaMove(Board * board, int depth, LINE * pline){
	startDepth = depth;
	Move moves[MAX_MOVES];
	U8 moveCount = getAllLegalMoves(board,moves);
	int max = INT_MIN;
	Move bestMove;
	LINE line;
	orderMoves(moves,board,moveCount);
	
	char buffer[100] ={};
	for(int i = 0; i < moveCount; i++){
		board->makeMove(moves[i]);
		int val = -alphabetaHelper(board,INT_MIN+500,INT_MAX-500,depth-1,&line);
		board->undoMove();
		if(get_wall_time() >= endTime){
			return 0;
		}
		if(val > max){
			max = val;
			bestMove = moves[i];
			pline->argmove[0] = moves[i];
            memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(Move));
            pline->cmove = line.cmove + 1;
		}
	}
	score = max;
	return bestMove;
}

int quiesce(Board * board, int alpha, int beta){
	int curEval = Eval::evaluate(board);
	if(curEval >= beta){
		return beta;
	}
	if(alpha > curEval){
		alpha = curEval;
	}
	Move moves[MAX_MOVES];
	/*U8 moveCount = Movegen::getAllCaptures(board->currentBoard(),moves);
	for(int i = 0; i < moveCount; i++){
		board->makeMove(moves[i]);
		int score = -quiesce(board, -beta, -alpha);
		board->undoMove();
		if(curEval >= beta){
			return beta;
		}
		if(alpha > curEval){
			alpha = curEval;
		}
	}*/
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
			expectedMoves=70;
		}else if(moveNum < 30){
			expectedMoves = 80;
		}else if(moveNum < 50){
			expectedMoves = 90;
		}else if(moveNum < 70){
			expectedMoves = 100;
		}else if(moveNum < 100){
			expectedMoves = 120;
		}else if(moveNum < 200){
			expectedMoves = 200;
		}else{
			expectedMoves = 320;
		}	
		int movesToGo = expectedMoves-moveNum;
		config->movetime = toMoveTime/movesToGo;
	}
	
}
Search::LINE lastPv;
Move Search::iterativeDeepening(Board * board){
	cfg->endTime = get_wall_time()+((cfg->movetime)/1000.0);
	endTime = cfg->endTime;
	char buffer[100];
	Move bestMove=-1;
	double startTime = get_wall_time();
	printf("time: %f\n",startTime);
	for(int depth = 1; depth < 200; depth++){
		LINE line;
		Move curMove = getAlphabetaMove(board,depth,&line);
		if(get_wall_time() >= endTime){
			printf("time: %f\n",get_wall_time());
			return bestMove;
		}
		bestMove = curMove;
		printf("info depth %i score cp %i time %i pv", depth, score, (int) ((get_wall_time() - startTime)*1000));
		for(int j = 0; j < depth; j++){
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
Move * Search::orderMoves(Move moves[], Board * board, int numMoves){
	Move* unordered = (Move *) malloc(sizeof(Move)*numMoves);
	char buffer[199];
	memcpy(unordered,moves,numMoves*sizeof(Move));
	//if tt:probe(board) 
	for(int i = 0; i < numMoves; i++){
		if(lastPv.argmove[0] == unordered[i]){
			Move temp = unordered[0];
			moves[0] = unordered[i];
			moves[i] = temp;
		}
	}
	free(unordered);
	return moves;
}
Move Search::getBestMove(Board * board){
	calculateMovetime(board);
	return iterativeDeepening(board);
}