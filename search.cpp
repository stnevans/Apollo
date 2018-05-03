#include "search.h"
#include "move.h"
#include "eval.h"
#include "movegen.h"
#include <limits.h>
#include "board.h"
#include "movegen.h"
Search::Config cfg;
Search::Config * Search::getConfig(){
	return &cfg;
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
		int val = -minimaxHelper(board,cfg.depth-1);
		board->undoMove();
		if(val > max){
			max = val;
			bestMove = moves[i];
		}
	}
	return bestMove;
}

int alphabetaHelper(Board * board, int alpha, int beta, int depth){
	if(depth == 0 || board->isCheckmate() || board->isDraw()){return Eval::evaluate(board);}
	Move moves[MAX_MOVES];
	U8 moveCount = getAllLegalMoves(board,moves);
	for(int i = 0; i < moveCount; i++){
		board->makeMove(moves[i]);
		int val = -alphabetaHelper(board, -beta, -alpha, depth-1);
		board->undoMove();
		if(val >= beta){
			return beta;
		}
		if(val > alpha){
			alpha = val;
		}
	}
	return alpha;
}
Move Search::getAlphabetaMove(Board * board){
	Move moves[MAX_MOVES];
	U8 moveCount = getAllLegalMoves(board,moves);
	int max = INT_MIN;
	Move bestMove;
	for(int i = 0; i < moveCount; i++){
		board->makeMove(moves[i]);
		int val = -alphabetaHelper(board,INT_MIN+500,INT_MAX-500,cfg.depth-1);
		board->undoMove();
		if(val > max){
			max = val;
			bestMove = moves[i];
		}
	}
	return bestMove;
}

Move Search::getBestMove(Board * board){
	return getAlphabetaMove(board);
}