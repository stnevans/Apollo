#ifndef EVAL_H_INCLUDED
#define EVAL_H_INCLUDED
#include "move.h"
#include "board.h"
#include "stdafx.h"
namespace Eval{
	int evaluate(Board * b);
	int materialEvaluate(BoardInfo *b, bool whiteToMove);
	int basicEvaluate(Board *b);
}

#endif