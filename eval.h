#ifndef EVAL_H_INCLUDED
#define EVAL_H_INCLUDED
#include "move.h"
#include "board.h"
#include "stdafx.h"
namespace Eval{
	int noMovesEvaluate(Board * b, U8 ply);
	int evaluate(Board * b, U8 ply);
	int materialEvaluate(BoardInfo *b, bool whiteToMove);
	int basicEvaluate(Board *b, U8 ply);
}

#endif