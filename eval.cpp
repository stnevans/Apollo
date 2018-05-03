#include "eval.h"
#include "board.h"
#include <limits.h>
//  PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
int naivePieceValue[] = {100,290,310,500,900,1000000};
U8 popcnt(U64 bb){
	#ifdef _WIN32
		return __popcnt64(bb);
	#else
		return __builtin_popcountll(bb);
	#endif
}
int secondEvaluate(Board * b){
	if(b->isDraw()){
		return 0;
	}
	if(b->isCheckmate()){
		return INT_MIN+1000+b->currentBoard()->moveNumber;
	}
	int eval = 0;
	BoardInfo * info=b->currentBoard();
	eval+=naivePieceValue[PAWN]*(popcnt(info->WhitePawnBB)-popcnt(info->BlackPawnBB));
	eval+=naivePieceValue[KNIGHT]*(popcnt(info->WhiteKnightBB)-popcnt(info->BlackKnightBB));
	eval+=naivePieceValue[BISHOP]*(popcnt(info->WhiteBishopBB)-popcnt(info->BlackBishopBB));
	eval+=naivePieceValue[ROOK]*(popcnt(info->WhiteRookBB)-popcnt(info->BlackRookBB));
	eval+=naivePieceValue[QUEEN]*(popcnt(info->WhiteQueenBB)-popcnt(info->BlackQueenBB));
	if(info->whiteToMove){
		return eval; 
	}else{
		return -eval;
	}
}
int Eval::naiveEvaluate(Board * b){//TODO debug why drawn positions seemed to trigger isCheckmate?!
	if(b->isDraw()){
		return 0;
	}
	if(b->isCheckmate()){
		return INT_MIN+1000+b->currentBoard()->moveNumber;
	}
	int eval = 0;
	BoardInfo * info=b->currentBoard();
	eval+=naivePieceValue[PAWN]*(popcnt(info->WhitePawnBB)-popcnt(info->BlackPawnBB));
	eval+=naivePieceValue[KNIGHT]*(popcnt(info->WhiteKnightBB)-popcnt(info->BlackKnightBB));
	eval+=naivePieceValue[BISHOP]*(popcnt(info->WhiteBishopBB)-popcnt(info->BlackBishopBB));
	eval+=naivePieceValue[ROOK]*(popcnt(info->WhiteRookBB)-popcnt(info->BlackRookBB));
	eval+=naivePieceValue[QUEEN]*(popcnt(info->WhiteQueenBB)-popcnt(info->BlackQueenBB));
	if(info->whiteToMove){
		return eval; 
	}else{
		return -eval;
	}
}
int Eval::evaluate(Board * b){
	return naiveEvaluate(b);	
}