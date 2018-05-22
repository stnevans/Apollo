#include "stdafx.h"
#include "move.h"

struct tt_entry {
    U64  hash;
    int  eval;
    U8	 depth;
    U8   flags;
    Move  bestMove;
};
typedef tt_entry tt_entry;
enum ettflag {
    TT_EXACT,
    TT_ALPHA,
    TT_BETA
};

namespace TT{
	void setSize(int kb);
	tt_entry * probe(U64 key);
	void save(U64 key, int eval, U8 flags, Move bestMove, U8 depth); 
	void nextGeneration();
}