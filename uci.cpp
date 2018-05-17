#include <string>
#include <iostream>
#include <sstream>
#include "board.h"
#include "movegen.h"
#include "bitboard.h"
#include "uci.h"
#include "eval.h"
#include "search.h"
#include "bbmagic.h"
#ifdef __linux__
#include <stdio.h>
#include "string.h"
#endif
using namespace std;
std::string StartPositionFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
//  Windows
#ifdef _WIN32
#include <Windows.h>
double get_wall_time(){
    LARGE_INTEGER time,freq;
    if (!QueryPerformanceFrequency(&freq)){
        //  Handle error
        return 0;
    }
    if (!QueryPerformanceCounter(&time)){
        //  Handle error
        return 0;
    }
    return (double)time.QuadPart / freq.QuadPart;
}
double get_cpu_time(){
    FILETIME a,b,c,d;
    if (GetProcessTimes(GetCurrentProcess(),&a,&b,&c,&d) != 0){
        //  Returns total user time.
        //  Can be tweaked to include kernel times as well.
        return
            (double)(d.dwLowDateTime |
            ((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
    }else{
        //  Handle error
        return 0;
    }
}

//  Posix/Linux
#else
#include <time.h>
#include <sys/time.h>
double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}
#endif


char* UCI::getMoveString(Move m, char* ret){
	char from[3], to[3];
	char* arr = getAlgebraicPos(from_sq(m));
	from[0] = arr[0];from[1]=arr[1];from[2]=arr[2];
	
	arr = getAlgebraicPos(to_sq(m));
	to[0] = arr[0];to[1]=arr[1];to[2]=arr[2];
	if(promotion_type(m)!=PAWN && promotion_type(m) != KING){
		switch(promotion_type(m)){
			case KNIGHT:
				snprintf(ret,sizeof(ret),"%s%sn",from,to);
				break;
			case QUEEN:
				snprintf(ret,sizeof(ret),"%s%sq",from,to);
				break;
			case ROOK:
				snprintf(ret,sizeof(ret),"%s%sr",from,to);
				break;
			case BISHOP:
				snprintf(ret,sizeof(ret),"%s%sb",from,to);
				break;
			default:
				snprintf(ret,sizeof(ret),"%s%su",from,to);
				break;
			}
	}else{
		snprintf(ret,sizeof(ret),"%s%s",from,to);
	}
	return ret;
}
Move UCI::toMove(Board * board, std::string move){
	//printf("%s\n", move.c_str());
	char curMove[100];
	ExtMove moves[MAX_MOVES];
	U8 count = getAllLegalMoves(board,moves);
	for(int i = 0; i < count; i++){
		memset(&curMove[0], 0, sizeof(curMove));
		Move move1 = moves[i].move;
		//printf("Compare %s : %s\n", move.c_str(), getMoveString(move1,curMove)); 
		if(strcmp(move.c_str(),getMoveString(move1,curMove)) == 0){
			//printf("RET\n");
			return move1;
		}
	}
	return -1;//BRITTLE. Checked in SetPosition. Only currently works because Moves use few enough bits the top can't be negative
}


void UCI::setPosition(Board * board, BoardInfo* info, istringstream* parser){
	std::string token, fen;
	(*parser) >> token;
	if(token=="startpos"){
		fen=StartPositionFEN;
		(*parser) >> token;
	}else if(token=="fen"){
		 while ((*parser) >> token && token != "moves"){
            fen += token + " ";
		 }
	}else{
		return;
	}
	board->readFromFen(fen,info);
	while((*parser) >> token){
		Move m = toMove(board, token);
		if(m != -1){
			board->makeMove(m);
		}else{
			break;
		}
	}
}
void printUciOptions(){
	
}
void setOption(istringstream *parser){

}
void UCI::go(Board * board, istringstream *parser){
		char buffer[100];
	string token;
	Search::Config cfg;
	while((*parser) >> token){
		if(token == "depth"){
			(*parser) >> cfg.depth;
		}else if(token =="wtime"){
			(*parser) >> cfg.wTime;
		}else if(token=="btime"){
			(*parser) >> cfg.bTime;
		}else if(token == "winc"){
			(*parser) >> cfg.winc;
		}else if(token == "binc"){
			(*parser) >> cfg.binc;
		}else if(token == "movetime"){
			(*parser) >> cfg.movetime;
		}
	}
	Search::setConfig(&cfg);
	Move bestMove = Search::getBestMove(board);
	printf("bestmove %s\n", getMoveString(bestMove,buffer));
}
int UCI::perft(Board* b, int depth) {
	if (depth == 0){
		return 1;
	}
	ExtMove moveList[MAX_MOVES]={};
	int num_moves = getAllLegalMoves(b, moveList);
	int count = 0;	
	for (int i = 0; i < num_moves; i++) {
		b->makeMove(moveList[i].move);
		count += perft(b, depth - 1);
		b->undoMove();
	}
	
	return count;
}

int UCI::divide(Board* b, int depth) {
	ExtMove moveList[MAX_MOVES]={};
	int num_moves = getAllLegalMoves(b, moveList);
	int count = 0;
	char buffer[100];
	for (int i = 0; i < num_moves; i++) {
		b->makeMove(moveList[i].move);
		int countc = perft(b, depth - 1);
		count+= countc;
		printf("%s : %i\n", getMoveString(moveList[i].move,buffer),countc);
		b->undoMove();
	}
	return count;
}

void UCI::perft(Board * board, istringstream * parser){
	int depth;
	(*parser) >> depth;
	double time = get_wall_time();
	int numNodes = divide(board, depth);
	double newTime = get_wall_time();
	printf("---------------\n");
	printf("Nodes: %i\n" , numNodes);
	printf("Took %fs. nps: %f.\n",newTime-time,numNodes/(newTime-time));
}
#include "bitboard.h"
#include "move.h"
bool UCI::loop(){
	Board b;
	BoardInfo info;
	std::string token, cmd;
	b.readFromFen(StartPositionFEN,&info);
	while(true){
		getline(cin,cmd);
		istringstream parser(cmd);
		parser >> skipws >> token;
		if(token == "quit"){
			return false;
		}
		if(token == "uci"){
			std::cout << "id name Apollo 1.1\n";
			std::cout << "id author Stuart Nevans Locke\n\n";
			printUciOptions();
			std::cout << "uciok" << std::endl;
		}else if(token == "ucinewgame"){
			b.readFromFen(StartPositionFEN,&info);
		}else if(token == "isready"){
			std::cout << "readyok" << std::endl;
		}else if(token == "position"){
			setPosition(&b,&info,&parser);
		}else if(token=="option"){
			setOption(&parser);
		}else if(token == "go"){
			go(&b,&parser);
		}else if(token == "perft"){
			perft(&b,&parser);
		}else if(token == "eval"){
			printf("Current Position: %i\n",Eval::evaluate(&b));
		}else if(token == "d"){
			printf("FEN: %s\nIs checkmate: %i\nIs draw: %i\nZobrist: %llx\nEval %i\n", b.getFen().c_str(), b.isCheckmate(), b.isDraw(), b.currentBoard()->zobrist, Eval::evaluate(&b));
		}
	}
	return true;

}


