#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED
#include "stdafx.h"
#include "move.h"
#include "board.h"
U64 getSquareKey(bool pieceMovedWhite, U8 loc, PieceType piece);
U64 getKeyForMove(bool pieceMovedWhite, U8 fromLoc, U8 toLoc, PieceType piece);
U64 initKeyFromBoard(Board * b);
extern const U64 whiteKingSideCastling;
extern const U64 whiteQueenSideCastling;
extern const U64 blackKingSideCastling;
extern const U64 blackQueenSideCastling ;
extern const U64 passantColumn[];
extern const U64 whiteMove;
#endif