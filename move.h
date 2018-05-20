#ifndef MOVE_H_INCLUDED
#define MOVE_H_INCLUDED
#include "stdafx.h"
/// A move needs 16 bits to be stored
///
/// Lower bit means LSB. 
/// bit  0- 5: destination square (from 0 to 63)
/// bit  6-11: origin square (from 0 to 63)
/// bit 12-14: PieceType moved
/// bit 15-17: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
/// bit 18-19: special move flag: none(0), promotion (1), en passant (2), castling (3)
/// bit 21: capture flag
/// NOTE: EN-PASSANT bit is set only when a pawn can be captured
///
/// Special cases are MOVE_NONE and MOVE_NULL. We can sneak these in because in
/// any normal move destination square is always different from origin square
/// while MOVE_NONE and MOVE_NULL have the same origin and destination square.


enum MoveType {
  NORMAL,
  PROMOTION = 1,
  ENPASSANT = 2,
  CASTLING  = 3
};

enum PieceType {
  PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, EMPTY
};

typedef U32 Move;

constexpr Move createMove(U8 from, U8 to, PieceType pieceMoved){
	return (pieceMoved<<12)+(from<<6)+to;
}

constexpr Move createMove(U8 from, U8 to, PieceType pieceMoved, PieceType promote, MoveType type){
	return (type<<18)+(promote<<15)+(pieceMoved<<12)+(from<<6)+to;
}
constexpr U8 from_sq(Move m) {
  return U8((m >> 6) & 0x3F);
}

constexpr U8 to_sq(Move m) {
  return U8(m & 0x3F);
}

constexpr MoveType type_of(Move m) {
  return MoveType((m >> 18) & 0x3);
}

constexpr PieceType promotion_type(Move m) {
  return PieceType((m >> 15) & 0x7);
}

constexpr PieceType PieceMoved(Move m) {
  return PieceType((m >> 12) & 0x7);
}
constexpr bool isCapture(Move m){
	return (bool) ((m>>21)&0x1);
}
constexpr Move setCapture(Move m){
	return (m | (1 << 21));
}


#endif