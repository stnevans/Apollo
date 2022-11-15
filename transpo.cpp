#include "transpo.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

tt_entry * tt;
int numEntries;
long long ttSize;

void TT::clear(){
	memset(tt,0,ttSize);
}
void TT::setSize(long long size){
	free(tt);
	if(size < sizeof(tt_entry)){
		size = 0;
		return;
	}
	numEntries = size/sizeof(tt_entry)-1;
	tt =  (tt_entry *) malloc(size);	
	memset(tt,0,size);
	ttSize = size;
}

tt_entry * TT::probe(U64 key){
	return &tt[key%numEntries];
}
#include "uci.h"
void TT::save(U64 key, int eval, U8 flags, Move bestMove, U8 depth, U8 ply){
	tt_entry * entry = &tt[key%numEntries];
	//if should replace:
	if(depth >= entry->depth || depth >4){
		//Replace if the keys are different, 
		if(!(entry->depth == depth) || !(entry->hash == key) || (!(entry->flags == TT_EXACT) && flags == TT_EXACT)){
			entry->hash = key;
			//adjust mate scores to store the distance-to-mate
			if(eval < (INT_MIN+1200)){
				entry->eval = eval-ply;
			}else if(eval > -(INT_MIN+1200)){
				entry->eval = eval+ply;
			}else{
				entry->eval = eval;
			}
			entry->depth = depth;
			entry->bestMove=bestMove;
			entry->flags=flags;
		}
	
	}
}