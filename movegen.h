#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED
#include "stdafx.h"
#include "move.h" //For moves
#include "board.h"//For boardinfo

const U8 MAX_MOVES = 255;

struct ExtMove{
	Move move;
	int value;
};

inline bool operator<(const ExtMove& f, const ExtMove& s) {
  return f.value < s.value;
}
U64 pseudoLegalKnightMoveDestinations(U8 loc, U64 targets);
U64 pseudoLegalKingMoveDestinations(U8 loc, U64 targets) ;
U8 getWhiteKingMoves(BoardInfo* b, Move moves[], int index);
U8 getBlackKingMoves(BoardInfo* b, Move moves[], int index);
U8 getWhiteKnightMoves(BoardInfo *b, Move moves[], int index);
U8 getBlackKnightMoves(BoardInfo *b, Move moves[], int index);
U8 getWhiteBishopMoves(BoardInfo *b, Move moves[], int index);
U8 getBlackBishopMoves(BoardInfo *b, Move moves[], int index);
U8 getBlackRookMoves(BoardInfo *b, Move moves[], int index);
U8 getWhiteRookMoves(BoardInfo *b, Move moves[], int index);
U8 getBlackQueenMoves(BoardInfo *b, Move moves[], int index);
U8 getWhiteQueenMoves(BoardInfo *b, Move moves[], int index);
U8 getWhitePawnMoves(BoardInfo *b, Move moves[], int index);
U8 getBlackPawnMoves(BoardInfo *b, Move moves[], int index);

Move* getMoveList();

U8 getAllPseudoLegalMoves(BoardInfo * boardInfo, Move list[]);
U8 getAllLegalMoves(Board* boardInfo, Move list[]);

namespace Movegen{
	U8 getAllCaptures(BoardInfo * b, Move moves[]);
}
#endif