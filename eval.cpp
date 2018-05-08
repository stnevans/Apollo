#include "eval.h"
#include "board.h"
#include <limits.h>
#include "bitboard.h"
#include "movegen.h"
//  PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
int naivePieceValue[] = {100,301,305,500,900,1000000};
int centralization[] = {0,0,0,0,0,0,0,0,
						0,2,2,2,2,2,2,0,
						0,2,4,4,4,4,2,0,
						0,2,5,8,8,5,2,0,
						0,2,5,8,8,5,2,0,
						0,2,4,4,4,4,2,0,
						0,2,2,2,2,2,2,0,
						0,0,0,0,0,0,0,0};
					   
//int materialEvalCache = -1;
//U8 materialCount;
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
	//Doubled Pawns
	for(int i = FILE_A; i < FILE_H; i++){
		if((whitePawns& maskFile[i]) != 0){
			eval-=50*(popcnt(whitePawns&maskFile[i])-1);
		}
		if((blackPawns & maskFile[i]) != 0){
			eval+=50*(popcnt(blackPawns&maskFile[i])-1); 
		}
	}
	
	//Isolated Pawns 
	for(int i = FILE_B; i < FILE_G; i++){
		if(((whitePawns & maskFile[i]) !=0) && (whitePawns&maskFile[i-1] == 0) && ((whitePawns&maskFile[i+1]) == 0)){
			eval-=50*(popcnt(whitePawns&maskFile[i]));
		}
		if((blackPawns & maskFile[i] !=0) && (blackPawns&maskFile[i-1] == 0) && ((blackPawns&maskFile[i+1]) == 0)){
			eval+=50*(popcnt(blackPawns&maskFile[i])); 
		}
	}
	//Passed pawns
	for(int i = FILE_B; i < FILE_G; i++){
		if(((whitePawns & maskFile[i]) !=0) && (blackPawns&maskFile[i-1] == 0) && ((blackPawns&maskFile[i+1]) == 0)){
			eval+=60;
		}
		if((blackPawns & maskFile[i] !=0) && (whitePawns&maskFile[i-1] == 0) && ((whitePawns&maskFile[i+1]) == 0)){
			eval-=60;
		}
	}
	//Passed a pawns
	if(((whitePawns & maskFile[FILE_A]) !=0) && (blackPawns&maskFile[FILE_B] == 0)){
		eval+=60;
	}
	if((blackPawns & maskFile[FILE_A] !=0) && ((whitePawns&maskFile[FILE_B]) == 0)){
		eval-=60;
	}
	
	//Passed h pawns
	if(((whitePawns & maskFile[FILE_H]) !=0) && (blackPawns&maskFile[FILE_G] == 0)){
		eval+=60;
	}
	if((blackPawns & maskFile[FILE_H] !=0) && ((whitePawns&maskFile[FILE_G]) == 0)){
		eval-=60;
	}
	
	//king safety
	//Terrible, terrible, terrible way of approximating if endgame.
	if(info->moveNumber < 30){
		if(whiteKings & maskFile[FILE_E] != 0 || whiteKings & maskFile[FILE_D]){
			eval+=20;
		}
		if(blackKings & maskFile[FILE_E] != 0 || blackKings & maskFile[FILE_D]){
			eval-=20;
		}
		/*if(whiteKings & maskFile[FILE_A] == 0){
			if(whitePawns & (whiteKings << 9) != 0){
				
			eval+=10;
			}
		}*/
	}
	
	//open files
	
	
	
	//Mobility is good
	//Move moves[255];
	//U8 moveCount = getPseudoLegalMoves(b,moves);
	
	if(info->whiteToMove){
		return eval; 
	}else{
		return -eval;
	}
}
int Eval::materialEvaluate(BoardInfo * b, bool whiteToMove){//TODO debug why drawn positions seemed to trigger isCheckmate?!
	int eval = 0;
	BoardInfo * info=b;
	eval+=naivePieceValue[PAWN]*(popcnt(info->WhitePawnBB)-popcnt(info->BlackPawnBB));
	eval+=naivePieceValue[KNIGHT]*(popcnt(info->WhiteKnightBB)-popcnt(info->BlackKnightBB));
	eval+=naivePieceValue[BISHOP]*(popcnt(info->WhiteBishopBB)-popcnt(info->BlackBishopBB));
	eval+=naivePieceValue[ROOK]*(popcnt(info->WhiteRookBB)-popcnt(info->BlackRookBB));
	eval+=naivePieceValue[QUEEN]*(popcnt(info->WhiteQueenBB)-popcnt(info->BlackQueenBB));
	if(whiteToMove){
		return eval; 
	}else{
		return -eval;
	}
}
int Eval::evaluate(Board * b){
	return basicEvaluate(b);	
}