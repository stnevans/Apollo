#include "board.h"
#include <stdlib.h>
#include <stdio.h> // io
#include <sstream> //stremm stuff
#include <cstring> // For std::memset, std::memcmp
#include "bbmagic.h"
#include "movegen.h"
#include "move.h"
BoardInfo boards[MAX_MOVECOUNT] = {};

//Current TODO.  Implement Zobrist Hashing, move gen, transpo table, faster version of memcpy,
//TODO:
//Zobrist Hashing
//Transpo Table
//Faster than memcpy?
//Cache is attacked bitboard
//If not, cache is attacked bits. Meaning we only check for a cache hit
//Eventually maybe test for check by only analyzing last move.
//http://mediocrechess.blogspot.com/2007/01/guide-futile-attempts-with-futility.html
//http://chessprogramming.wikispaces.com/Selectivity
//http://chessprogramming.wikispaces.com/Aspiration%20Windows
//https://web.archive.org/web/20071031100114/http://www.brucemo.com:80/compchess/programming/pv.htm

//TODO check repetition

bool Board::isDraw(){
	Move moves[MAX_MOVES];
	if(getAllLegalMoves(this, moves) == 0){
		if(!(isSquareAttacked(boardInfo, boardInfo->WhiteKingBB, true)
				|| isSquareAttacked(boardInfo, boardInfo->BlackKingBB, false))){
			return true;
		}
	}
	return false;
}
bool Board::legal() {
	if (!boardInfo->whiteToMove){
		return !isSquareAttacked(currentBoard(), boardInfo->WhiteKingBB, true);
	}
	return !isSquareAttacked(currentBoard(), boardInfo->BlackKingBB, false);
}
bool Board::isCheckmate(){
	Move moves[MAX_MOVES];
	if(isOwnKingInCheck()){
		if(getAllLegalMoves(this, moves) == 0){
			return true;
		}
	}
	return false;
}
//We are in check if it is our turn and they can take our king
bool Board::isOwnKingInCheck(){
	if(boardInfo->whiteToMove){
		return isSquareAttacked(currentBoard(), boardInfo->WhiteKingBB, true);
	}
	return isSquareAttacked(currentBoard(), boardInfo->BlackKingBB, false);
}

BitBoard Board::getAllPiecesBitBoard(bool isWhiteToMove){
	if(isWhiteToMove){
		return boardInfo->BlackPiecesBB;
	}else{
		return boardInfo->WhitePiecesBB;
	}
}
BitBoard* Board::getBitBoard(PieceType type, bool isWhiteToMove){
	if(isWhiteToMove){
		switch(type){
			case PAWN:
				return &boardInfo->WhitePawnBB;
			case KNIGHT:
				return &boardInfo->WhiteKnightBB;
			case BISHOP:
				return &boardInfo->WhiteBishopBB;
			case ROOK:
				return &boardInfo->WhiteRookBB;
			case KING:
				return &boardInfo->WhiteKingBB;
			case QUEEN:
				return &boardInfo->WhiteQueenBB;
			default:
				return NULL;
		}		
	}else{
		switch(type){
			case PAWN:
				return &boardInfo->BlackPawnBB;
			case KNIGHT:
				return &boardInfo->BlackKnightBB;
			case BISHOP:
				return &boardInfo->BlackBishopBB;
			case ROOK:
				return &boardInfo->BlackRookBB;
			case KING:
				return &boardInfo->BlackKingBB;
			case QUEEN:
				return &boardInfo->BlackQueenBB;
			default:
				return NULL;
		}		
	}
	
}
Board& Board::readFromFen(std::string& fenStr, BoardInfo* board){
  std::memset(this, 0, sizeof(Board));
  std::memset(board, 0, sizeof(BoardInfo));
  board->enPassantLoc=ENPASSANT_NONE;
  boardInfo = board;
  std::istringstream ss(fenStr);
  std::string token; 
  int up = 7; int over = 0;
  while(std::getline(ss,token,'/')){
	  const char * row = token.c_str();
	  over = 0;
	  while(*row !='\0'){
		  if(*row > '0' && *row < '9'){
			over+=*row-'0';
		  }else{
			  if(*row ==' '){break;}
			  U64 square = getSquare[up*8+over];
			  if(*row=='p'){
				board->BlackPawnBB |= square;  
			  }else if(*row=='P'){
				  board->WhitePawnBB |= square;
			  }else if(*row == 'n'){
				  board->BlackKnightBB |= square;
			  }else if(*row=='N'){
				  board->WhiteKnightBB |= square;
			  }else if(*row == 'b'){
				  board->BlackBishopBB |= square;
			  }else if(*row=='B'){
				  board->WhiteBishopBB |= square;
			  }else if(*row == 'r'){
				  board->BlackRookBB |= square;
			  }else if(*row == 'R'){
				  board->WhiteRookBB |= square;
			  }else if(*row == 'k'){
				  board->BlackKingBB |=square;
			  }else if(*row=='K'){
				  board->WhiteKingBB |= square;
			  }else if(*row=='q'){
				  board->BlackQueenBB |= square;
			  }else if(*row=='Q'){
				  board->WhiteQueenBB |=square;
			  }else{
				  printf("Unknown Fen Character: %s\n", row);
			  }
			  over++;
		  }
		  row++;
	  }
	  up--;
  }
  //Squares are done.
  updateSpecialBB(board);
  
  std::istringstream specialIn(token);
  std::string subToken;
  std::getline(specialIn, subToken, ' ');//Getting rid of the last piece info
  
  //Parse specials
  int i = 0;
  while(std::getline(specialIn, subToken, ' ')){
	  const char* cstr = subToken.c_str();
	  if(i==0){//To Move?
		board->whiteToMove=((*(cstr)=='w'));  
	  }else if(i==1){//Castling
		  board->whiteKingCastle=(strstr(cstr,"K") 	!=0?1:0);
		  board->whiteQueenCastle=(strstr(cstr,"Q")!=0?1:0);
		  board->blackKingCastle=(strstr(cstr,"k")!=0?1:0);
		  board->blackQueenCastle=(strstr(cstr,"q")!=0?1:0);
	  }else if(i==2){//En Passant
		  board->enPassantLoc = algebraicPosToLoc(subToken.c_str());
	  }else if(i==3){//fifty move
		
		  board->fiftyMoveRule = atoi(subToken.c_str());
		  
	  }else if(i==4){
		  board->moveNumber = atoi(subToken.c_str())*2-1;
		  if(!board->whiteToMove){
			  board->moveNumber++;
		  }
	  }
	  
	  i++;
  }
  return *this;
}

void Board::updateSpecialBB(BoardInfo * board){
	board->WhitePiecesBB = board->WhiteKingBB | board->WhiteBishopBB | board->WhiteKnightBB | board->WhiteQueenBB | board->WhitePawnBB | board->WhiteRookBB;
	board->BlackPiecesBB = board->BlackKingBB | board->BlackBishopBB | board->BlackKnightBB | board->BlackQueenBB | board->BlackPawnBB | board->BlackRookBB;
	board->AllPiecesBB = board->WhitePiecesBB | board->BlackPiecesBB;
}
std::string Board::getFen(){
  int emptyCnt;
  std::ostringstream ss;
  for (int r = RANK_8; r >= RANK_1; --r)
  {
      for (int f = FILE_A; f <= FILE_H; ++f)
      {
		  U8 loc = getSq(r,f);
		  for(emptyCnt = 0; f <= FILE_H && ((boardInfo->AllPiecesBB & getSquare[loc]) == 0);){
			  ++emptyCnt;
			  f++;
			  loc = getSq(r,f);
		  }
		  if(emptyCnt != 0){
			  ss << emptyCnt;
		  }
		  if(f <= FILE_H){
			  ss << getPieceAtChar(loc);
		  }
	  }
	  if(r > RANK_1){
		  ss << '/';
	  }
  }
  ss << " ";
  ss << ((boardInfo->whiteToMove) ? "w" : "b");
  ss << " ";
  if(boardInfo->whiteKingCastle){
	  ss << 'K';
  }
  if(boardInfo->whiteQueenCastle){
	  ss<< 'Q';
  }
  if(boardInfo->blackKingCastle){
	  ss << 'k';
  }
  if(boardInfo->blackQueenCastle){
	  ss << 'q';
  }
  ss << " ";
  ss << getAlgebraicPos(boardInfo->enPassantLoc);
  ss << " ";
  ss << std::to_string(boardInfo->fiftyMoveRule).c_str();
  ss << " ";
  ss << std::to_string((1 + (boardInfo->moveNumber))/2);
	return ss.str();
}
char Board::getPieceAtChar(U8 loc){
	U64 square = getSquare[loc];
	if((boardInfo->WhitePiecesBB & square) != 0){
		if((boardInfo->WhitePawnBB & square) != 0){
			return 'P';
		}else if((boardInfo->WhiteKingBB & square) != 0){
			return 'K';
		}else if((boardInfo->WhiteBishopBB & square) != 0){
			return 'B';
		}else if((boardInfo->WhiteKnightBB & square) != 0){
			return 'N';
		}else if((boardInfo->WhiteQueenBB & square) != 0){
			return 'Q';
		}else if((boardInfo->WhiteRookBB & square) != 0){
			return 'R';
		}else{
			return 'W';
		}
	}else if((boardInfo->BlackPiecesBB & square) != 0){
		if((boardInfo->BlackPawnBB & square) != 0){
			return 'p';
		}else if((boardInfo->BlackKingBB & square) != 0){
			return 'k';
		}else if((boardInfo->BlackBishopBB & square) != 0){
			return 'b';
		}else if((boardInfo->BlackKnightBB & square) != 0){
			return 'n';
		}else if((boardInfo->BlackQueenBB & square) != 0){
			return 'q';
		}else if((boardInfo->BlackRookBB & square) != 0){
			return 'r';
		}else{
			return 'b';
		}
	}else{
		return ' ';
	}
}
int capt = 0;

void printMove1(Move m){
	char from[3], to[3];
	char* arr = getAlgebraicPos(from_sq(m));
	from[0] = arr[0];from[1]=arr[1];from[2]=arr[2];
	
	arr = getAlgebraicPos(to_sq(m));
	to[0] = arr[0];to[1]=arr[1];to[2]=arr[2];
	printf("%s %s\n",from,to);
}
void Board::makeMove(Move move){
	//TODO test this speed vs manually copying. No idea what's faster. also maybe *newInfo = *board Info
	//Copy current BoardInfo
	BoardInfo* newInfo = &boards[boardInfo->moveNumber+1];
	memcpy(newInfo,boardInfo,sizeof(BoardInfo));
	newInfo->moveNumber+=1;
	newInfo->fiftyMoveRule+=1;
	newInfo->enPassantLoc=ENPASSANT_NONE;
	newInfo->previousBoard = boardInfo;
	
	boardInfo = newInfo;
	BitBoard opponentBB = getAllPiecesBitBoard(boardInfo->whiteToMove);
	U8 moveDest = to_sq(move);
	U8 moveStart = from_sq(move);
	//If capture, we clear the piece from the opponents BB.
	if((getSquare[moveDest] & opponentBB) != 0 || type_of(move) == ENPASSANT){
		U8 captureSquare = moveDest;
		if(type_of(move) == ENPASSANT){
			//The idea is moving up/down a row from the destination. 
			captureSquare = (boardInfo->whiteToMove) ? (moveDest - 8) : (moveDest + 8); //TODO, this can't be right
			//newInfo->enPassantLoc=captureSquare;
		}
		if(boardInfo->whiteToMove){
			newInfo->BlackBishopBB&=~getSquare[captureSquare];
			newInfo->BlackKingBB&=~getSquare[captureSquare];
			newInfo->BlackKnightBB&=~getSquare[captureSquare];
			newInfo->BlackPawnBB&=~getSquare[captureSquare];
			newInfo->BlackRookBB&=~getSquare[captureSquare];
			newInfo->BlackQueenBB&=~getSquare[captureSquare];
		}else{
			newInfo->WhiteBishopBB&=~getSquare[captureSquare];
			newInfo->WhiteKingBB&=~getSquare[captureSquare];
			newInfo->WhiteKnightBB&=~getSquare[captureSquare];
			newInfo->WhitePawnBB&=~getSquare[captureSquare];
			newInfo->WhiteRookBB&=~getSquare[captureSquare];
			newInfo->WhiteQueenBB&=~getSquare[captureSquare];
		}
		newInfo->fiftyMoveRule=0;
	}
	BitBoard * pieceToClear = getBitBoard(PieceMoved(move),newInfo->whiteToMove);
	*pieceToClear &=~getSquare[moveStart];
	
	if(boardInfo->whiteToMove){
		switch(PieceMoved(move)){
			case PAWN:
				newInfo->fiftyMoveRule=0;
				//CHECK EN PASSANT
				if((getRank(moveDest) == RANK_4) && (getRank(moveStart) == RANK_2)){
					newInfo->enPassantLoc=moveStart+8;
				}
				//If promotion, we add a piece rather than a pawn in the moveDest
				if(type_of(move) == PROMOTION){
					switch(promotion_type(move)){
						case QUEEN:
							boardInfo->WhiteQueenBB|=getSquare[moveDest];
							break;
						case ROOK:
							boardInfo->WhiteRookBB|=getSquare[moveDest];
							break;
						case KNIGHT:
							boardInfo->WhiteKnightBB|=getSquare[moveDest];
							break;
						case BISHOP:
							boardInfo->WhiteBishopBB|=getSquare[moveDest];
							break;	
					}
				}else{
					boardInfo->WhitePawnBB|=getSquare[moveDest];	
				}
				
				break;
			case KNIGHT:
				boardInfo->WhiteKnightBB|=getSquare[moveDest];
				break;
			case BISHOP:
				boardInfo->WhiteBishopBB|=getSquare[moveDest];
				break;
			case ROOK:
				if(moveStart== SQ_H1){
					newInfo->whiteKingCastle=false;
				}else if(moveStart==SQ_A1){
					newInfo->whiteQueenCastle=false;
				}else if(moveStart==SQ_H8){
					newInfo->blackKingCastle=false;
				}else if(moveStart==SQ_A8){
					newInfo->blackQueenCastle=false;
				}
				boardInfo->WhiteRookBB|=getSquare[moveDest];
				break;
			case KING:
				newInfo->whiteHasCastled=true;
				newInfo->whiteKingCastle=false;
				newInfo->whiteQueenCastle=false;
				if(type_of(move) == CASTLING){
					//If Kingside, assumes e1g1 would be castling
					U8 rookFrom, rookTo;
					if(moveDest == SQ_G1){
						rookFrom = SQ_H1;
						rookTo = SQ_F1;
					}else{//Queenside 
						rookFrom = SQ_A1;
						rookTo=SQ_D1;
					}
					boardInfo->WhiteRookBB|=getSquare[rookTo];
					boardInfo->WhiteRookBB^=getSquare[rookFrom];
				}
				boardInfo->WhiteKingBB|=getSquare[moveDest];
				break;
			case QUEEN:
				boardInfo->WhiteQueenBB|=getSquare[moveDest];
				break;
			default:
				printf("ERROR: IMPOSSIBLE PIECE MAKE MOVE\n");
		}		
	}else{
		switch(PieceMoved(move)){
			case PAWN:
				newInfo->fiftyMoveRule=0;
				if((getRank(moveDest) == RANK_5) && (getRank(moveStart) == RANK_7)){
					newInfo->enPassantLoc=moveStart-8;
				}
				if(type_of(move) == PROMOTION){
					switch(promotion_type(move)){
						case QUEEN:
							boardInfo->BlackQueenBB|=getSquare[moveDest];
							break;
						case ROOK:
							boardInfo->BlackRookBB|=getSquare[moveDest];
							break;
						case KNIGHT:
							boardInfo->BlackKnightBB|=getSquare[moveDest];
							break;
						case BISHOP:
							boardInfo->BlackBishopBB|=getSquare[moveDest];
							break;	
					}
				}else{
					boardInfo->BlackPawnBB|=getSquare[moveDest];
				}
				break;
			case KNIGHT:
				boardInfo->BlackKnightBB|=getSquare[moveDest];
				break;
			case BISHOP:
				boardInfo->BlackBishopBB|=getSquare[moveDest];
				break;
			case ROOK:
				boardInfo->BlackRookBB|=getSquare[moveDest];
				break;
			case KING:
				newInfo->blackHasCastled=true;
				newInfo->blackKingCastle=false;
				newInfo->blackQueenCastle=false;
				if(type_of(move) == CASTLING){
					//If Kingside
					U8 rookFrom, rookTo;
					if(moveDest == SQ_G8){
						rookFrom = SQ_H8;
						rookTo = SQ_F8;
					}else{//Queenside 
						rookFrom = SQ_A8;
						rookTo=SQ_D8;
					}
					boardInfo->BlackRookBB|=getSquare[rookTo];
					boardInfo->BlackRookBB^=getSquare[rookFrom];
				}
				boardInfo->BlackKingBB|=getSquare[moveDest];				
				break;
			case QUEEN:
				boardInfo->BlackQueenBB|=getSquare[moveDest];
				break;
			default:
				printf("ERROR: IMPOSSIBLE PIECE MAKE MOVE\n");
		}				
		}
		newInfo->whiteToMove=!newInfo->whiteToMove;

		updateSpecialBB(newInfo);
		
}

void Board::undoMove(){
	boardInfo=boardInfo->previousBoard;
}

void Board::makeNullMove(){
	BoardInfo* newInfo = &boards[boardInfo->moveNumber+1];
	memcpy(newInfo,boardInfo,sizeof(BoardInfo));
	newInfo->moveNumber+=1;
	newInfo->fiftyMoveRule+=1;
	newInfo->enPassantLoc=ENPASSANT_NONE;
	newInfo->previousBoard = boardInfo;
	
	newInfo->whiteToMove=!newInfo->whiteToMove;
	boardInfo=newInfo;
	//updateSpecialBB(newInfo);
}

BoardInfo* Board::currentBoard(){
	return boardInfo;
}/*
#include "bbmagic.h"
int main(){
	    initBBUtils();
		initBBMagic();
        std::string pm("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        BoardInfo bi;
        Board b;
        b.readFromFen(pm,&bi);
        printf("%s\n",b.getFen().c_str());      
        Move testMove = createMove(getSq(1,4),getSq(3,4),PAWN);
        b.makeMove(testMove);
        printf("%s\n",b.getFen().c_str()); 
		testMove = createMove(getSq(6,3),getSq(4,3),PAWN);
        b.makeMove(testMove);
        printf("%s\n",b.getFen().c_str()); 
	return 1;
}*/
