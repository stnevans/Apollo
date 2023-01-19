#include "board.h"
#include <cassert>
#include <stdio.h>
#include <string.h>
#include "bbmagic.h"
#include "move.h"
#include "movegen.h"
#include "uci.h"
#include "search.h"
#include "bitboard.h"
#include "zobrist.h"
void printMove(Move m){
	char from[3], to[3];
	char* arr = getAlgebraicPos(from_sq(m));
	from[0] = arr[0];from[1]=arr[1];from[2]=arr[2];
	
	arr = getAlgebraicPos(to_sq(m));
	to[0] = arr[0];to[1]=arr[1];to[2]=arr[2];
	if(promotion_type(m)!=PAWN && promotion_type(m) != KING){
		switch(promotion_type(m)){
			case KNIGHT:
					printf("%s%sn ",from,to);
					break;
			case QUEEN:
				printf("%s%sq ",from,to);
				break;
			case ROOK:
				printf("%s%sr ",from,to);
				break;
			case BISHOP:
				printf("%s%sb ",from,to);
				break;
			default:
				printf("%s%su ",from,to);
				break;
			}
	}else{
		printf("%s%s ",from,to);
	}
}
void printMoves(Move moves[], int amt){
	for(int i  = 0; i < amt; i++){
		printMove(moves[i]);
		printf("\n");
	}
}
int check;

int perft(Board* b, int depth) {
	if (depth == 0){
		return 1;
	}
	
	ExtMove moveList[MAX_MOVES]={};
	int num_moves = getAllLegalMoves(b, moveList);
	int count = 0;
	
	for (int i = 0; i < num_moves; i++) {
		b->makeMove(moveList[i].move);
		count += perft(b, depth - 1);
		b->undoMove();
	}
	
	return count;
}


int main(){
	BoardInfo bi;
	Board b;
	std::string startFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	initBBUtils();
	initBBMagic();
	b.readFromFen(startFen,&bi);
	
	/*//printf("%s %i\n", b.getFen().c_str(), b.currentBoard()->moveNumber);

	assert(strcmp(b.getFen().c_str(),startFen.c_str()) == 0);
	int fromSq = getSq(RANK_2,FILE_E);
	int toSq = getSq(RANK_4,FILE_E);
	PieceType moveType = PAWN;
	Move testMove = createMove(fromSq,toSq,moveType);
	assert(from_sq(testMove) == fromSq);
	assert(to_sq(testMove) == toSq);
	assert(type_of(testMove) == NORMAL);
	assert(PieceMoved(testMove) == moveType);
	b.makeMove(testMove);
	//testPerft(&b);
	
	printf("Testing making basic pawn moves\n");
	//printf("%s %i\n", b.getFen().c_str(), b.currentBoard()->moveNumber);
	assert(strcmp(b.getFen().c_str(),"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1") == 0);
	testMove = createMove(getSq(RANK_7,FILE_D),getSq(RANK_5,FILE_D),PAWN);
	b.makeMove(testMove);
	assert(strcmp(b.getFen().c_str(),"rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2") == 0);

	
	printf("Testing Perft\n");
	startFen = ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	b.readFromFen(startFen,&bi);
	//assert(perft(&b,2)==400);
	//assert(perft(&b,5)==4865609);
	startFen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
	b.readFromFen(startFen,&bi);
	//assert(perft(&b,4)==2103487);
	startFen = ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
	b.readFromFen(startFen,&bi);
	//assert(perft(&b,5)==674624);
	
	startFen="n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1";
	b.readFromFen(startFen,&bi);
	printf("Passed Tests\n");*/
	Bitboard::initAttacks();
	UCI::loop();
	return 0;
}
