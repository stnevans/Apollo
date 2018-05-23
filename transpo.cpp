#include "transpo.h"
#include <stdio.h>
#include <string.h>

tt_entry * tt;
int numEntries;

U8 generation = 0;
void TT::nextGeneration(){
	generation+=8;//so bottom 3 bits are clear
}
void TT::setSize(int size){
	free(tt);
	if(size < sizeof(tt_entry)){
		size = 0;
		return;
	}
	numEntries = size/sizeof(tt_entry)-1;
	tt =  (tt_entry *) malloc(size);	
	memset(tt,0,size);
	generation = 8;
}

tt_entry * TT::probe(U64 key){
	return &tt[key%numEntries];
}
#include "uci.h"
void TT::save(U64 key, int eval, U8 flags, Move bestMove, U8 depth){
	tt_entry * entry = &tt[key%numEntries];
	//if should replace:
	//if(depth >= entry->depth){
		//Replace if the keys are different, 
		if(!(entry->depth == depth) || !(entry->hash == key) || (!(entry->flags == TT_EXACT) && flags == TT_EXACT)){
			entry->hash = key;
			entry->eval = eval;
			entry->depth = depth;
			entry->bestMove=bestMove;
			entry->flags=flags;
		}
	
	//}
}