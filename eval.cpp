#include "eval.h"
#include "board.h"
#include <limits.h>
#include "bitboard.h"
//  PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
int naivePieceValue[] = {100,290,310,500,900,1000000};
int centralization[] = {0,0,0,0,0,0,0,0,
						0,2,2,2,2,2,2,0,
						0,2,4,4,4,4,2,0,
						0,2,4,7,7,4,2,0,
						0,2,4,7,7,4,2,0,
						0,2,4,4,4,4,2,0,
						0,2,2,2,2,2,2,0,
						0,0,0,0,0,0,0,0};
/*int whiteEndgamePawnSquares[] = 
					  {0,0,0,0,0,0,0,0,
					   -10,-10,-10,-10,-10,-10,-10,-10,
					   0,0,0,0,0,0,0,0};*/
U8 popcnt(U64 bb){
	#ifdef _WIN32
		return __popcnt64(bb);
	#else
		return __builtin_popcountll(bb);
	#endif
}

int centralizationValue(U64 bb){
	int eval = 0;
	while(bb != 0){
		U64 loc = trailingZeroCount(bb);
		eval+=centralization[loc];
		//printf("bb: %llx %llx %lli\n",bb,1LL<<loc, loc);
		bb ^= ( U64(1LL << loc));
	}
	return eval;
}

int Eval::basicEvaluate(Board * b){
	if(b->isDraw()){
		return 0;
	}
	if(b->isCheckmate()){
		return INT_MIN+1000+b->currentBoard()->moveNumber;
	}
	int eval = 0;
	BoardInfo * info=b->currentBoard();
	
	U64 whitePawns = info->WhitePawnBB;
	U64 blackPawns = info->BlackPawnBB;
	U64 whiteKnights = info->WhiteKnightBB;
	U64 blackKnights = info->BlackKnightBB;
	U64 whiteBishops = info->WhiteBishopBB;
	U64 blackBishops = info->BlackBishopBB;
	U64 whiteRooks = info->WhiteRookBB;
	U64 blackRooks = info->BlackRookBB;
	U64 whiteQueens = info->WhiteQueenBB;
	U64 blackQueens = info->BlackQueenBB;
	U64 whiteKings = info->WhiteKingBB;
	U64 blackKings = info->BlackKingBB;
	
	
	
	eval+=naivePieceValue[PAWN]*(popcnt(whitePawns)-popcnt(blackPawns));
	eval+=naivePieceValue[KNIGHT]*(popcnt(whiteKnights)-popcnt(blackKnights));
	eval+=naivePieceValue[BISHOP]*(popcnt(whiteBishops)-popcnt(blackBishops));
	eval+=naivePieceValue[ROOK]*(popcnt(whiteRooks)-popcnt(blackRooks));
	eval+=naivePieceValue[QUEEN]*(popcnt(whiteQueens)-popcnt(blackQueens));
	
	eval+=centralizationValue(whitePawns);
	eval-=centralizationValue(blackPawns);
	eval+=centralizationValue(whiteKnights);
	eval-=centralizationValue(blackKnights);
	eval+=centralizationValue(whiteBishops);
	eval-=centralizationValue(blackBishops);
//	eval+=centralizationValue(whiteRooks);
//	eval-=centralizationValue(blackRooks);
//	eval+=centralizationValue(whiteQueens);
//	eval+=centralizationValue(blackQueens);
//	eval+=centralizationValue(whiteKings);
//	eval+=centralizationValue(blackKings);

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
	return basicEvaluate(b);	
}