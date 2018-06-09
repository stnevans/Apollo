
#ifndef BBMAGIC_H_INCLUDED
#define BBMAGIC_H_INCLUDED
#include "board.h"
#include "stdafx.h"


U64 getBishopShiftAttacks(U64 square, U64 all);
U64 getRookShiftAttacks(U64 square, U64 all);
U64 getQueenAttacks(int index, U64 all);
U64 getBishopAttacks(int index, U64 all);
U64 getRookAttacks(int index, U64 all);
U64 getXrayAttacks(BoardInfo* b, int i, U64 all);
U64 getXrayAttacks(BoardInfo* b, int i);
U64 getIndexAttacks(BoardInfo* b, int i);
bool isIndexAttacked(BoardInfo* b, U8 loc, bool whiteToMove);
bool isSquareAttacked(BoardInfo* b, U64 square, bool whiteToMove);
U32 transform(U64 b, U64 magic, U8 bits);
U64 generatePieces(U32 index, U32 bits, U64 mask);
U64 squareAttackedAuxSliderMask(U64 square, int shift, U64 border);
U64 squareAttackedAuxSlider(U64 square, int shift, U64 border);
U64 squareAttackedAux(U64 square, int shift, U64 border);
U64 squareAttackedAux(U64 square, U64 all, int shift, U64 border);
U64 getXrayAttacksSliding(BoardInfo* b, int i);
void initBBMagic();
bool isIndexAttackedWithoutKing(BoardInfo* b, U8 i, bool whiteToMove) ;
#endif
