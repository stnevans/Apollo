#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED
#include <string>
#include <sstream>
#include <iostream>
#include "board.h"

double get_wall_time();
double get_cpu_time();

namespace UCI{
	int divide(Board* b, int depth);
	void perft(Board * board, std::istringstream * parser);
	int perft(Board *b, int depth);
	char* getMoveString(Move m, char* ret);
	int appendMoveString(Move m, char* ret);
	Move toMove(Board * board, std::string move);
	void setPosition(Board * b, BoardInfo* info, std::istringstream* parser);
	bool loop();
	void go(Board * board, std::istringstream *parser);
}
#endif
