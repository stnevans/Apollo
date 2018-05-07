#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED
#include "move.h"
#include "stdafx.h"
#include "board.h"

namespace Search{
	struct Config{
		int depth=0; // in ply
		int wTime=0;
		int bTime=0;
		int winc=0;
		int binc=0;
		bool infinite=false;
		int movetime; //in ms, ends up being used as final value in iterative deepeening.
		double endTime;
	};
	typedef struct Config Config;
	void init();
	int quiesce(Board * board, int alpha, int beta);
	Move getBestMove(Board * board);
	Move getMinimaxMove(Board * board);
	Move getAlphabetaMove(Board * board, int depth);
	void setConfig(Config * config);
	bool isPositionFutile(Board *b, int alpha, int beta, int depthSearched, int depthToGo);
	bool isMoveFutile(Board * b, int depthSearched, int depthToGo, int movesSearched, Move move, int alpha, int beta, int curEval);
	void calculateMovetime(Board* b);
	Move iterativeDeepening(Board * board);
}
#endif