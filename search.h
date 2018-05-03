#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED
#include "move.h"
#include "stdafx.h"
#include "board.h"

namespace Search{
	struct Config{
		int depth; // in ply
		
	};
	typedef struct Config Config;
	Move getBestMove(Board * board);
	Move getMinimaxMove(Board * board);
	Move getAlphabetaMove(Board * board);
	Config * getConfig();
	
}
#endif