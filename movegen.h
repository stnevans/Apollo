#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED
#include "stdafx.h"
#include "move.h" //For moves
#include "board.h"//For boardinfo

const U8 MAX_MOVES = 255;

struct ExtMove{
	Move move;
	int score;
};

constexpr U8 from_sq(ExtMove m) {
  return U8((m.move >> 6) & 0x3F);
}

constexpr U8 to_sq(ExtMove m) {
  return U8(m.move & 0x3F);
}

constexpr MoveType type_of(ExtMove m) {
  return MoveType((m.move >> 18) & 0x3);
}

constexpr PieceType promotion_type(ExtMove m) {
  return PieceType((m.move >> 15) & 0x7);
}

constexpr PieceType PieceMoved(ExtMove m) {
  return PieceType((m.move >> 12) & 0x7);
}
inline bool operator<(const ExtMove& f, const ExtMove& s) {
  return f.score < s.score;
}
U8 getAllLegalMovesSize(Board* b, ExtMove list[]);
U64 pseudoLegalKnightMoveDestinations(U8 loc, U64 targets);
U64 pseudoLegalKingMoveDestinations(U8 loc, U64 targets) ;
U8 getWhiteKingMoves(BoardInfo* b, ExtMove moves[], int index);
U8 getBlackKingMoves(BoardInfo* b, ExtMove moves[], int index);
U8 getWhiteKnightMoves(BoardInfo *b, ExtMove moves[], int index);
U8 getBlackKnightMoves(BoardInfo *b, ExtMove moves[], int index);
U8 getWhiteBishopMoves(BoardInfo *b, ExtMove moves[], int index);
U8 getBlackBishopMoves(BoardInfo *b, ExtMove moves[], int index);
U8 getBlackRookMoves(BoardInfo *b, ExtMove moves[], int index);
U8 getWhiteRookMoves(BoardInfo *b, ExtMove moves[], int index);
U8 getBlackQueenMoves(BoardInfo *b, ExtMove moves[], int index);
U8 getWhiteQueenMoves(BoardInfo *b, ExtMove moves[], int index);
U8 getWhitePawnMoves(BoardInfo *b, ExtMove moves[], int index);
U8 getBlackPawnMoves(BoardInfo *b, ExtMove moves[], int index);
void addMove(Move move, ExtMove moves[],int index, bool isWhitePieceMoving, BoardInfo * info);
Move* getMoveList();

U8 getAllPseudoLegalMoves(BoardInfo * boardInfo, ExtMove list[]);
U8 getAllLegalMoves(Board* boardInfo, ExtMove list[]);

namespace Movegen{
	U8 getAllCaptures(Board * b, ExtMove moves[]);
}
#endif