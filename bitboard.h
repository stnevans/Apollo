#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED
#include "stdafx.h"

typedef U64 BitBoard;
U8 popcnt(U64 bb);
U8 getSq(int rank, int file);
int getFile(U8 sq);
int getRank(U8 sq);	
U8 algebraicPosToLoc(const char * pos);
char* getAlgebraicPos(U8 loc);
void initBBUtils();
const U8 ENPASSANT_NONE = 111;
U64 lowestOneBit(U64 i);
int trailingZeroCount(U64 bb);//TODO improve this

namespace Bitboard{
	void initAttacks();
}
extern U64 rookSlides[64];
extern U64 bishopSlides[64];

constexpr bool moreThanOneOccupant(U64 bb) {
  return bb & (bb - 1);
}

enum Square : U8 {
  SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
  SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
  SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
  SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
  SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
  SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
  SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
  SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
  SQ_NONE,

  SQUARE_NB = 64
};

enum File : int {
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NB
};

enum Rank : int {
  RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NB
};
extern U64 alignedBB[64][64];
extern U64 squaresBetween[64][64];
extern U64 maskRank[8];
extern U64 maskFile[8];
extern U64 clearRank[8];
extern U64 clearFile[8];
extern U64 getSquare[64];
#endif