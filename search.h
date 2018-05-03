#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED
#include "move.h"
#include "stdafx.h"
#include "board.h"

namespace Search{
	struct Config{
		int depth=0; // in ply
		int wtime=0;
		int btime=0;
		int winc=0;
		int binc=0;
	};
	typedef struct Config Config;
	Move getBestMove(Board * board);
	Move getMinimaxMove(Board * board);
	Move getAlphabetaMove(Board * board);
	void setConfig(Config * config);
	
}
#endif