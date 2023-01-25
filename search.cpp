/*
* This file contains the code for the actual search. It uses iterative deepening with alphabeta pruning and a negamax search.
* An aspiration window is used with a window of 50 centipawns from the previous search.
* The other pruning techniques used are:
* - Null Move Pruning
* - Futility Pruning
* The quiesence search uses very aggressive delta pruning
* Move Ordering depends on the following methods:
*  Quiet Moves:
*   - History Heuristic
*   - Killer Moves, 2 are stored
*   - Countermove Heuristic
*  Non-Quiet Moves:
*   - Captures: MVVLVA
*   - Promotions: Same as Quiet
*   - Checks: Same as Quiet
* -
*/
#include "search.h"
#include "move.h"
#include "eval.h"
#include "movegen.h"
#include <limits.h>
#include "board.h"
#include "movegen.h"
#include "bitboard.h"
#include "uci.h" //For timing
#include "transpo.h"
#include <stdio.h>
#ifdef __linux__
#include "string.h"
#endif

Search::Config* cfg;
void Search::setConfig(Search::Config * config){
	cfg=config;
}

constexpr int futilitySize = 4; 
int futilityMoves[] = {0,100,200,400,600};
int reverseFutilityValues[] = {0,200,330,600,800};
//int razor[]={0,200,350};
bool inline isCapture(BoardInfo * b, Move m){
	//must be prior to playing move
	if(b->whiteToMove){
		return ((getSquare[to_sq(m)] & b->BlackPiecesBB) != 0);
	}else{
		return ((getSquare[to_sq(m)] & b->WhitePiecesBB) != 0);
	}
}

/*
* Returns the best move according to our wonderful engine. A valid config is assumed to be set previously.
*/
Move Search::getBestMove(Board * board){
	calculateMovetime(board);
	return iterativeDeepening(board);
}



/*
* If wtime or btime are being used, calculates the time to use on the current move.
* If infinite search is used, the movetime is INT_MAX. 
*/ 
void Search::calculateMovetime(Board* b){
	Config * config = cfg;
	if(config->depth != 0){
		config->movetime = INT_MAX;
	}
	if(config->infinite){
		config->movetime=INT_MAX;
	}
	BoardInfo * info = b->currentBoard();
	int moveNum = info->moveNumber;
	int inc = 0;
	int toMoveTime = 0;
	if(config->wTime != 0 || config->bTime != 0){
		if(b->currentBoard()->whiteToMove){
			toMoveTime = config->wTime;
			inc = config->winc;
		}else{
			toMoveTime = config->bTime;
			inc = config->binc;
		}
		int expectedMoves = 0;
		if(moveNum <= 10){
			expectedMoves = 50;
		}else if(moveNum <= 30){
			expectedMoves = 60;
		}else if(moveNum <= 40){
			expectedMoves = 70;
		}else if(moveNum <= 50){
			expectedMoves = 80;
		}else if(moveNum <= 70){
			expectedMoves = 100;
		}else if(moveNum <= 100){
			expectedMoves = 120;
		}else{
			expectedMoves = moveNum + 40;
		}

		//CORRECTION: The division by 2 here converts plies into moves
		int movesToGo = (expectedMoves-moveNum)/2;
		config->movetime = toMoveTime/movesToGo;
	}
	cfg->endTime = get_wall_time()+((cfg->movetime)/1000.0);
}
//Used for move ordering and getting pv
Search::LINE lastPv;

//Used for move ordering
U64 whiteHeuristic[64][64]={{0}};
U64 blackHeuristic[64][64]={{0}};

U64 counterMove[64][64]={{0}};
constexpr int MAX_DEPTH = 200;
Move killerMoves[MAX_DEPTH][2] = {};

int startDepth=0; //Used to determine how long until we hit horizon in an alphabeta search
double endTime; //Used to determine when to leave deepening
int score = 0; //Used to communicate the score of the best move returned by ab search.

U64 nodeCount = 0; //Just nodes where eval is called
bool currentlyFollowingPv;
bool canDoNullMove;
const int nullMoveEndgame = 350;
/*
* The main search function. 
*/

U64 optimalMoveOrder = 0;
U64 badMoveOrder=0;

int mateIn(int eval){
	if(eval < INT_MIN+1200){
		return (-eval+(INT_MIN+1000))/2;
	}else if(eval > -(INT_MIN+1200)){
		return (-eval-(INT_MIN+1000)+1)/2;
	}
	return 0;
}
bool saced = false;

Move Search::iterativeDeepening(Board * board){
	
	endTime = cfg->endTime;
	Move bestMove=-1;
	double startTime = get_wall_time();
	double totalUsedTime = 0;
	
	//I wonder what it would be like to know c++ and use efficient techniques rather than this?
	for(int i = 0; i < 64; i++){
		for(int j = 0; j < 64; j++){
			whiteHeuristic[i][j]=0;
			blackHeuristic[i][j]=0;
			counterMove[i][j]=0;
		}
	}
	int eval;
	nodeCount = 0; //moving nodeCount outside loop so it doesn't get reset each deepening (fixes false nps)
	for(int depth = 1; depth < MAX_DEPTH; depth++){
		optimalMoveOrder=0;badMoveOrder=0;
		LINE pvLine;
		startDepth=depth;
		currentlyFollowingPv = true;
		canDoNullMove = true;
		
		//Aspiration window is handled here.
		//One idea, get "perfect" root move ordering, meaning save more than just the best move, but rather the best few moves for better ordering. next iteration.
		if(depth > 1){	
			int newEval;
			newEval = alphabetaHelper(board,eval-50,eval+50,depth,0,&pvLine);
			if((newEval <= eval-50) || (newEval >= eval+50)){
				currentlyFollowingPv=true;
				newEval = alphabetaHelper(board,INT_MIN+500,INT_MAX-500,depth,0,&pvLine);
			}
			eval = newEval;
		}else{
			eval = alphabetaHelper(board,INT_MIN+500,INT_MAX-500,depth,0,&pvLine);
			
		}
		
		bestMove = (pvLine.cmove > 0) ? pvLine.argmove[0] : TT::probe(board->currentBoard()->zobrist)->bestMove;
		if(get_wall_time() >= endTime){
			return bestMove;
		}

		totalUsedTime = get_wall_time() - startTime;
		//Print info about the search we just did
		char pvbuffer[4096]; //We'll never realistically need more than this
		char* bufferpointer = &pvbuffer[0];
		for(int i = 0; i < pvLine.cmove; i++){
			bufferpointer += UCI::appendMoveString(pvLine.argmove[i],bufferpointer);
		}
		if(mateIn(score) != 0){
			printf("info depth %i score mate %i nodes %llu nps %llu time %u pv%s\n", depth, mateIn(score),nodeCount, (U64) (nodeCount/totalUsedTime),(U32) ((totalUsedTime)*1000), pvbuffer);
		}else{
			if(!board->isCheckmate()){
				printf("info depth %i score cp %i nodes %llu nps %llu time %u pv%s\n", depth, score,nodeCount, (U64) (nodeCount/totalUsedTime),(U32) ((totalUsedTime)*1000), pvbuffer);
			}
		}
		fflush(stdout);
		if(cfg->depth!=0){
			if(cfg->depth <= depth){
				return bestMove;
			}
		}
		
		//If there is not enough time to go far into the next depth, save that time for later.
		if(totalUsedTime * 1.6 >= endTime - startTime){
			return bestMove;
		}
		
		lastPv = pvLine;
		//Check uci stop command?
	}
	cfg->movetime=0;
	cfg->depth=0;
	cfg->wTime = 0;
	cfg->bTime = 0;
	
	return bestMove;
}
//Further idea::TODO:: don't generate all moves, just play tt move. then generate moves later. The idea is that we hope for a beta cutoff. Also might be worth it to do the same for killerMoves, but probably not because it seems like they would often be illegal.
int Search::alphabetaHelper(Board * board, int alpha, int beta, int depth, int ply, LINE * pline){
	LINE line;
	LINE useless;
	line.cmove = 0;
	useless.cmove = 0;
	int alphaHits = 0;

	ExtMove moves[MAX_MOVES];
	Move bestMove=0;

	BoardInfo* currentBoard = board->currentBoard();
	tt_entry* entry = TT::probe(currentBoard->zobrist);

	if(board->isRepetition() && entry->hash==currentBoard->zobrist && entry->depth >= depth){//Note this is buggy, it should only check if there's a repetition since the original move number.
		if(currentlyFollowingPv){
			//TODO: figure out how to get the rest of the move loop into the pv
			//memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(Move));
			//pline->cmove = line.cmove + 1;
			pline->argmove[0] = entry->bestMove;
			pline->cmove = 1;
		}else{
			pline->cmove = 0;
		}
		return 0;
	}
	
	if(depth <= 0){
		currentlyFollowingPv=false;
		pline->cmove = 0;
		nodeCount++;
		return quiesce(board,alpha,beta,ply);
	}
	//Theoretically, if a shortcut function for determining if any legal move exists, full move generation can be pushed back even further
	//However, seeing as significant changes to move generation are planned, I'll leave that up to you. ;)
	U8 moveCount = getAllLegalMoves(board,moves);
	if(moveCount == 0){
		currentlyFollowingPv=false;
		pline->cmove = 0;
		score = Eval::noMovesEvaluate(board,ply);
		return score;
	}

	//Handle transposition table here:
	//The issue with copying a table in the mainline is repetition is ignored. Not fully sure why however.
	if(entry->hash == currentBoard->zobrist){
		if(entry->depth >= depth){
			int eval = entry->eval;
			//adjust mate score to obtain an accurate distance-to-mate
			if(eval < INT_MIN+1200){
				eval += ply;
			}else if(eval > -(INT_MIN+1200)){
				eval -= ply;
			}
			//if(eval <= beta && eval >= alpha){
				//If the best move is the move in the transpositon table, set the pv accordingly
				if((entry->flags) == TT_EXACT && (!currentlyFollowingPv)){
					pline->cmove = 1;
					pline->argmove[0] = entry->bestMove;
					return eval;
				}else if((entry->flags)== TT_BETA){
					if(!currentlyFollowingPv){
						if(eval >= beta){
							pline->cmove = 1;
							pline->argmove[0] = entry->bestMove;
							return beta;
						}
					}
				}else if((entry->flags )== TT_ALPHA){
					if(!currentlyFollowingPv){
						if(eval <= alpha){
							pline->cmove = 1;
							pline->argmove[0] = entry->bestMove;
							return alpha;
						}
					}
				}
			//}
		}
	}else if(depth > 8){
		//INTERNAL ITERATIVE DEEPENING (IID). Might not add strength in the current setup, especially if the hash table is set to only accept larger depths in which case this likely is only a waste of time.   
		alphabetaHelper(board,alpha,beta,4,ply,&useless);
	}

	//Removing (nodeCount < 3000 or 7500) (functionally nodeCount < INF now) to be more aggressive on timeouts
	//nodeCount also does not measure the same things it did earlier
	//What was happening previously was the engine would often be over the nodeCount,
	//and was therefore forced to search the entire remaining depth instead of quiting out.
	if(get_wall_time() >= endTime){
		return INT_MIN;
	}
	int curEval = Eval::evaluate(board,ply);
	//Hope to prune!
	if(Search::isPositionFutile(board,alpha,beta,startDepth-depth,depth,curEval)){
		return beta;
	}
	
	
	
	//Null Move
	if(canDoNullMove && !currentlyFollowingPv && board->currentSideMaterial() > 1000 && !board->isOwnKingInCheck()){
		canDoNullMove=false;
		board->makeNullMove();
		int reduction = depth/4+3;
		
		int val = -alphabetaHelper(board, -beta, -alpha, depth-reduction, ply+1, &useless);
		board->undoMove();
		canDoNullMove=true;
		if(val >= beta){
			return beta;
		}
	}
	canDoNullMove=true;

	
	bool whiteToMove = currentBoard->whiteToMove;
	for(int i = 0; i < moveCount; i++){
		orderMoves(moves,board,moveCount,startDepth-depth,depth,i,whiteToMove);
		if(isMoveFutile(board,startDepth-depth,depth,i,moves[i].move,alpha,beta,curEval)){
			continue;
		}

		board->makeMove(moves[i].move);
		int val=alpha-1;
	
		int newDepth=depth-1;
		//LMR. Ideally we would be a bit more reserved when doing LMR
		if(board->currentSideMaterial() > 100 && i > moveCount/3){
			newDepth-=1;
		}
		//PVS Search
		if(i>0){
			val = -alphabetaHelper(board, -alpha-1, -alpha, newDepth, ply+1, &line);
		}
		if(val > alpha || i <=0){
			val = -alphabetaHelper(board, -beta, -alpha, newDepth, ply+1, &line);
		}
		board->undoMove();
		
		//Beta Cutoff
		
		//Update the pv
		if(val > alpha){
			pline->argmove[0] = moves[i].move;
			memcpy(pline->argmove + 1, line.argmove, line.cmove * sizeof(Move));
			pline->cmove = line.cmove + 1;

			if(i ==0){
				optimalMoveOrder++;
			}else{
				badMoveOrder++;
			}
			bestMove = moves[i].move;
			if(val >= beta){
				//History Heuristic Update
				if(whiteToMove){
					whiteHeuristic[from_sq(moves[i].move)][to_sq(moves[i].move)] += depth*depth;
					if(whiteHeuristic[from_sq(moves[i].move)][to_sq(moves[i].move)] > 50000){
						for(int k = 0; k < 64; k++){
						for(int z = 0; z < 64; z++){
							whiteHeuristic[k][z]=whiteHeuristic[k][z]/2;
						}	
						}
					}
				}else{
					blackHeuristic[from_sq(moves[i].move)][to_sq(moves[i].move)] += depth*depth;
					if(blackHeuristic[from_sq(moves[i].move)][to_sq(moves[i].move)] > 50000){
						for(int k = 0; k < 64; k++){
						for(int z = 0; z < 64; z++){
							blackHeuristic[k][z]=blackHeuristic[k][z]/2;
						}	
						}
					}
				}

				//Killer move update
				killerMoves[depth][1] = killerMoves[depth][0];
				killerMoves[depth][0] = moves[i].move;
				if(!isCapture(moves[i].move)){
					counterMove[from_sq(currentBoard->lastMove)][to_sq(currentBoard->lastMove)] = moves[i].move;
				}
				score = beta;
				TT::save(currentBoard->zobrist, beta, TT_BETA, bestMove, depth, ply);
				return beta;
			}
			alpha = val;
			alphaHits++;	
		}
	}
	score = alpha;

	if(alphaHits != 0){
		if(whiteToMove){
			whiteHeuristic[from_sq(bestMove)][to_sq(bestMove)] += depth*depth;
		}else{
			blackHeuristic[from_sq(bestMove)][to_sq(bestMove)] += depth*depth;
		}
				
		TT::save(currentBoard->zobrist, alpha, TT_EXACT, bestMove, depth, ply);
	}else{
		TT::save(currentBoard->zobrist,alpha,TT_ALPHA,moves[0].move,depth,ply);
		bestMove=moves[0].move;
	}
	return alpha;
}



//Ignoring in the pv for now.
int Search::quiesce(Board * board, int alpha, int beta, int ply){
	//if(board->isOwnKingInCheck()){
	//	return alphabetaHelper(board,alpha,beta,1);
	//}
	int curEval = Eval::evaluate(board,ply);
	if(curEval >= beta){
		return beta;
	}
	if(curEval > alpha){
		alpha = curEval;
	}
	
	ExtMove moves[MAX_MOVES];
	U8 moveCount = Movegen::getAllCaptures(board,moves);
	Move bestMove;
	for(int i = 0; i < moveCount; i++){
		orderMoves(moves,board,moveCount,0,0,i,board->currentBoard()->whiteToMove);
		//delta pruning
		//Endgames, with an extra buffer in case of large piece captured.
		if(board->currentSideMaterial() > 1500){
			//IN quiescence, score represents the static exchange. Way too aggressive delta pruning.
			if((moves[i].score + curEval < alpha) || type_of(moves[i].move) == PROMOTION){
				continue;
			}
		}
		
		board->makeMove(moves[i].move);
		int score = -quiesce(board, -beta, -alpha, ply+1);
		board->undoMove();
		if(score >= beta){
			return beta;
		}
		if(score > alpha){
			bestMove = moves[i].move;
			alpha = score;
		}
	}
		
	//TT::save(currentBoard->zobrist, alpha, TT_EXACT, bestMove, 0);

	return alpha;
}
void Search::orderMoves(ExtMove moves[], Board * board, int numMoves, int curDepthSearched, int depth, int idx, bool whiteToMove){
	if(currentlyFollowingPv){
		if(startDepth-depth < lastPv.cmove){
			Move entryMove = lastPv.argmove[startDepth-depth];
			for(int i = idx; i < numMoves; i++){
				if(entryMove==moves[i].move){
					ExtMove temp = moves[i];
					moves[i] = moves[idx];
					moves[idx] = temp;
					return;
				}
			}
		}
	}
	if(idx < 1){
		//TODO experiment not swapping, not even movegenning.
		tt_entry* entry = TT::probe(board->currentBoard()->zobrist);
		if(entry->hash == board->currentBoard()->zobrist){
			Move entryMove = entry->bestMove;
			for(int i = idx; i < numMoves; i++){
				if(entryMove==moves[i].move){
					ExtMove temp = moves[i];
					moves[i] = moves[idx];
					moves[idx] = temp;
					return;
				}
			}
		}
	}
	
	int max = moves[idx].score;
	int maxIdx = idx;
	for(int i = idx; i < numMoves; i++){
		//Killer moves.
		if(killerMoves[depth][0] ==moves[i].move || killerMoves[depth][1] == moves[i].move){
			ExtMove temp = moves[i];
			moves[i] = moves[idx];
			moves[idx] = temp;
			return;
		}
		//Sort non captures. We use history and a countermove bonus.
		if(!isCapture(moves[i].move)){
			if(whiteToMove){
				moves[i].score = whiteHeuristic[from_sq(moves[i].move)][to_sq(moves[i].move)];
			}else{
				moves[i].score = blackHeuristic[from_sq(moves[i].move)][to_sq(moves[i].move)];	
			}
			//Counter move bonus
			if(moves[i].move == counterMove[from_sq(board->currentBoard()->lastMove)][to_sq(board->currentBoard()->lastMove)]){
				moves[i].score+=2000;
			}
		}
		//We assume captures already have score set based on MVVLVA
		if(moves[i].score > max){
			max = moves[i].score;
			maxIdx = i;
		}
	}
	if(maxIdx > idx){
		ExtMove temp = moves[maxIdx];
		moves[maxIdx] = moves[idx];
		moves[idx] = temp;
	}

	return;
}

/*
* Called before movegenning. Basically tests if it's feasible for any move to make it bad enough that the value is below beta.
*/
bool Search::isPositionFutile(Board *b, int alpha, int beta, int depthSearched, int depthToGo, int curEval){
	if(depthToGo > futilitySize || depthSearched == 0){
		return false;
	}
	if(b->isOwnKingInCheck()){
		return false;
	}
	int futilityValue = reverseFutilityValues[depthToGo];
	return curEval-futilityValue > beta;
}
/*
* Implements futility pruning using futilityMoves[]. 
* Method is called before calling alphabeta on the move being considered.
* Note, if one non-capture is futile they all must be.
*/
//Maybe in the if start futility pruning at 1 rather than passing moves searched 
bool Search::isMoveFutile(Board * b, int depthSearched, int depthToGo, int movesSearched, Move move, int alpha, int beta, int curEval){
	if(depthToGo > futilitySize || depthSearched <= 0){
		return false;
	}
	//Calling the first move futile might not be smart
	if(movesSearched == 0){
		return false;
	}
	
	//Is some king in trouble
	if(b->isOwnKingInCheck()){
		return false;
	}
	if(b->isMoveCheck(move)){
		return false;
	}
	
	
	BoardInfo * boardInfo = b->currentBoard();
	//Pushing to 7th rank is scary
	if((PieceMoved(move) == PAWN)){
		if(boardInfo->whiteToMove){
			if(getRank(to_sq(move)) > 6){
				return false;
			}
		}else{
			if(getRank(to_sq(move)) < 3){
				return false;
			}
		}
	}
	int futilityValue = futilityMoves[depthToGo];
	//Bad captures are futile too.
	if(isCapture(boardInfo,move)){return b->staticExchange(move)+curEval+futilityValue < alpha;}
	return (curEval+futilityValue) < alpha;
}
