#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED
#include "move.h"
#include "stdafx.h"
#include "board.h"
#include "movegen.h"
namespace Search{
	struct Config{
		int depth=0; // in ply
		int wTime=0;
		int bTime=0;
		int winc=0;
		int binc=0;
		bool infinite=false;
		int movetime; //in ms, ends up being used as final value in iterative deepening.
		double endTime;
	};
	struct LINE {
		int cmove=0;              // Number of moves in the line.
		Move argmove[255];  // The line.
	};
	
	typedef struct LINE LINE;
	typedef struct Config Config;
	int alphabetaHelper(Board * board, int alpha, int beta, int depth, int ply, LINE * pline);
	int quiesce(Board * board, int alpha, int beta, int ply);
	Move getBestMove(Board * board);
	Move getAlphabetaMove(Board * board, int depth, LINE * line);
	void setConfig(Config * config);
	bool isPositionFutile(Board *b, int alpha, int beta, int depthSearched, int depthToGo, int curEval);
	bool isMoveFutile(Board * b, int depthSearched, int depthToGo, int movesSearched, Move move, int alpha, int beta, int curEval);
	void calculateMovetime(Board* b);
	Move iterativeDeepening(Board * board);
	void orderMoves(ExtMove moves[], Board * board, int numMoves, int curDepth, int depth, int idx, bool white);
	
}


extern U64 whiteHeuristic[64][64];
extern U64 blackHeuristic[64][64];
#endif