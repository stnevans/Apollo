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
					   
int blackEndgame[] = {  0,0,0,0,0,0,0,0,
						50,50,50,50,50,50,50,50,
						15,15,20,20,20,20,15,25,
						10,10,20,30,30,20,10,10,
						5,5,10,25,25,10,5,5,
						0,0,0,20,20,0,0,0,
						0,0,0,0,0,0,0,0,
						0,0,0,0,0,0,0,0,
						0,0,0,0,0,0,0,0
};

int whiteEndgame[] = {  0,0,0,0,0,0,0,0,
						0,0,0,0,0,0,0,0,
						0,0,0,0,0,0,0,0,
						0,0,0,20,20,0,0,0,
						5,5,10,25,25,10,5,5,
						10,10,20,30,30,20,10,10,
						15,15,20,20,20,20,15,25,
						50,50,50,50,50,50,50,50,
						0,0,0,0,0,0,0,0						
};


					   
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
	const int OPEN_FILE_VALUE = 25;
	const int SEMI_OPEN_FILE_VALUE = 20;
	const int DOUBLE_PAWNS_PENALTY = 15;
	const int ISOLATED_PAWNS_PENATLY = 15;
	const int PASSED_PAWN_BONUS = 25;
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
	
	
	//Material
	eval+=naivePieceValue[PAWN]*(popcnt(whitePawns)-popcnt(blackPawns));
	eval+=naivePieceValue[KNIGHT]*(popcnt(whiteKnights)-popcnt(blackKnights));
	eval+=naivePieceValue[BISHOP]*(popcnt(whiteBishops)-popcnt(blackBishops));
	eval+=naivePieceValue[ROOK]*(popcnt(whiteRooks)-popcnt(blackRooks));
	eval+=naivePieceValue[QUEEN]*(popcnt(whiteQueens)-popcnt(blackQueens));
	
	//Centralization of pieces
	int material = b->totalMaterial();
	if(totalMaterial <= 1400){
		eval+=whiteEndgame(whitePawns);
		eval-=blackEndgame(blackPawns);
	}else{
		eval+=centralizationValue(whitePawns);
		eval-=centralizationValue(blackPawns);
	}
	eval+=centralizationValue(whiteKnights);
	eval-=centralizationValue(blackKnights);
	eval+=centralizationValue(whiteBishops);
	eval-=centralizationValue(blackBishops);
	
	//Doubled Pawns
	for(int i = FILE_A; i < FILE_H; i++){
		if((whitePawns& maskFile[i]) != 0){
			eval-=DOUBLE_PAWNS_PENALTY*(popcnt(whitePawns&maskFile[i])-1);
		}
		if((blackPawns & maskFile[i]) != 0){
			eval+=DOUBLE_PAWNS_PENALTY*(popcnt(blackPawns&maskFile[i])-1); 
		}
	}
	
	//Isolated Pawns 
	for(int i = FILE_B; i < FILE_G; i++){
		if(((whitePawns & maskFile[i]) !=0) && (whitePawns&maskFile[i-1] == 0) && ((whitePawns&maskFile[i+1]) == 0)){
			eval-=ISOLATED_PAWNS_PENATLY*(popcnt(whitePawns&maskFile[i]));
		}
		if((blackPawns & maskFile[i] !=0) && (blackPawns&maskFile[i-1] == 0) && ((blackPawns&maskFile[i+1]) == 0)){
			eval+=ISOLATED_PAWNS_PENATLY*(popcnt(blackPawns&maskFile[i])); 
		}
	}
	
	
	//Passed pawns
	for(int i = FILE_B; i < FILE_G; i++){
		if(((whitePawns & maskFile[i]) !=0) && (blackPawns&maskFile[i-1] == 0) && ((blackPawns&maskFile[i+1]) == 0)){
			eval+=PASSED_PAWN_BONUS;
		}
		if((blackPawns & maskFile[i] !=0) && (whitePawns&maskFile[i-1] == 0) && ((whitePawns&maskFile[i+1]) == 0)){
			eval-=PASSED_PAWN_BONUS;
		}
	}
	//Passed a pawns
	if(((whitePawns & maskFile[FILE_A]) !=0) && (blackPawns&maskFile[FILE_B] == 0)){
		eval+=PASSED_PAWN_BONUS;
	}
	if((blackPawns & maskFile[FILE_A] !=0) && ((whitePawns&maskFile[FILE_B]) == 0)){
		eval-=PASSED_PAWN_BONUS;
	}
	
	//Passed h pawns
	if(((whitePawns & maskFile[FILE_H]) !=0) && (blackPawns&maskFile[FILE_G] == 0)){
		eval+=PASSED_PAWN_BONUS;
	}
	if((blackPawns & maskFile[FILE_H] !=0) && ((whitePawns&maskFile[FILE_G]) == 0)){
		eval-=PASSED_PAWN_BONUS;
	}
	
	
	//king safety here
	
	
	//open and semiopen files
	for(int i = FILE_A; i < FILE_H; i++){
		if(((whiteRooks | whiteQueens)& maskFile[i]) != 0){
			if((whitePawns & maskFile[i]) == 0){
				if((blackPawns & maskFile[i]) == 0){
					eval+=OPEN_FILE_VALUE;
				}else{
					eval+=SEMI_OPEN_FILE_VALUE;
				}
			}
		}
		if(((blackRooks| blackQueens) & maskFile[i]) != 0){
			if((blackPawns & maskFile[i]) == 0){
				if(whitePawns & maskFile[i] == 0){
					eval-=OPEN_FILE_VALUE;
				}else{
					eval-=SEMI_OPEN_FILE_VALUE;
				}
			}
		}
	}
	
	//bishops
	
	
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