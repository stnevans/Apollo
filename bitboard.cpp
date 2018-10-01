#include "bitboard.h"
#include <stdio.h>
#include <stdlib.h>
#include "bbmagic.h"
 U64 maskRank[8]={};
 U64 maskFile[8]={};
 U64 clearRank[8]={};
 U64 clearFile[8]={};
 U64 getSquare[64]={};
 char alge[3]={};
 
U64 rookSlides[64]={};
U64 bishopSlides[64]={};

 U64 lowestOneBit(U64 i){
	return i & -((S64) i);
}
U64 squaresBetween[64][64]={};
U64 alignedBB[64][64]={};

void Bitboard::initAttacks(){
	for(int i = 0; i <64; i++){
		rookSlides[i] = getRookAttacks(i,0);
		bishopSlides[i]=getBishopAttacks(i,0);
	}
	
	for(int i = 0; i < 64; i++){
		for(int j = 0; j < 64; j++){
			squaresBetween[i][j] = (getRookAttacks(i,getSquare[j]))&(getRookAttacks(j,getSquare[i]));
			alignedBB[i][j] = (getRookAttacks(i,0))&(getRookAttacks(j,0))|getSquare[i]|getSquare[j];
			if(getFile(i) != getFile(j) && getRank(i) != getRank(j)){
				squaresBetween[i][j] = (getBishopAttacks(i,getSquare[j]))&(getBishopAttacks(j,getSquare[i]));
				alignedBB[i][j] = (getBishopAttacks(i,0))&(getBishopAttacks(j,0))|getSquare[i]|getSquare[j];
			}
		}
	}
}
#ifdef _WIN64
#define USE_POPCNT
#endif
U8 popcnt(U64 bb){
	#ifdef _WIN32
	#ifdef USE_POPCNT
		return __popcnt64(bb);
	#else
		   int count = 0;
	   while (bb) {
		   count++;
		  bb &= bb - 1; // reset LS1B
	   }
	   return count;
	#endif
	#else
		return __builtin_popcountll(bb);
	#endif
}



int trailingZeroCount(U64 bb) {
   static const int lookup67[67+1] = {
      64,  0,  1, 39,  2, 15, 40, 23,
       3, 12, 16, 59, 41, 19, 24, 54,
       4, -1, 13, 10, 17, 62, 60, 28,
      42, 30, 20, 51, 25, 44, 55, 47,
       5, 32, -1, 38, 14, 22, 11, 58,
      18, 53, 63,  9, 61, 27, 29, 50,
      43, 46, 31, 37, 21, 57, 52,  8,
      26, 49, 45, 36, 56,  7, 48, 35,
       6, 34, 33, -1 };
   return lookup67[(bb & -bb) % 67];
}

char * getAlgebraicPos(U8 loc){
	if(loc == ENPASSANT_NONE){
		alge[0]='-';
		alge[1]='\0';
		return alge;
	}
	alge[0] = (loc % 8)+'a';
	alge[1]= (loc /8)+'1';
	alge[2]='\0';
	return alge;
}	
 U8 algebraicPosToLoc(const char * pos){
	if(*pos=='-'){
		return ENPASSANT_NONE;
	}
	return (pos[0]-'a')+(pos[1]-'0'-1)*8;
}
 void initBBUtils(){
	for(int i = 0; i < 8; i++){maskRank[i]=0;}
	for (int i = 0; i < 64; i++) {
		getSquare[i] = 1LL << i;
		
		maskRank[getRank(i)] |= getSquare[i];
		maskFile[getFile(i)] |= getSquare[i];
	}
	for (int i = 0; i < 8; i++) {
		clearRank[i] = ~maskRank[i];
		clearFile[i] = ~maskFile[i];
	}
}

 U8 getSq(int rank, int file){return rank*8+file;}
 int getFile(U8 sq){return sq % 8;}
 int getRank(U8 sq){return sq/8;}