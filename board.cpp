#include "board.h"
#include <stdlib.h>
#include <stdio.h> // io
#include <sstream> //stream stuff
#include <cstring> // For std::memset, std::memcmp
#include "bbmagic.h"
#include "movegen.h"
#include "move.h"
#include "zobrist.h"
#include <algorithm> //for std::max
BoardInfo boards[MAX_MOVECOUNT] = {};

//Current TODO.  Speed up movegen (only generate legal? make capture generator?), Add various caches(is attacked, isCheck, ...)
//TODO:
//Cache is attacked bitboard
//If not, cache is attacked bits. Meaning we only check for a cache hit
//Eventually maybe test for check by only analyzing last move.
//http://chessprogramming.wikispaces.com/Selectivity

//U64 unobstructedQueenMoves[] = {9313761861428380670,180779649147209727,289501704256556799,578721933553179903,1157442771889699071,2314886638996058367,4630054752952049919,9332167099941961983,4693051017133293315,9386102034266586887,325459994840334094,578862399937642268,1157444424410136376,2315169224285290352,4702396038313476064,9404792076610109376,2382695595002233605,4765391190004533002,9530782384287321621,614821794360007722,1157867469642086484,2387511058328678568,4775021017129017424,9550042029946290336,1227517888156599561,2455035776330041874,4910072647893521700,9820426766485563977,1266167049021314194,2460276499726510116,4920271520198053960,9840541936589512848,649930115027110161,1299860234365964834,2600000848492045380,5272058195805358472,10544115296394056209,2641485423861834786,5210912158452303940,10421541742416269448,361412783554236705,722826670915068482,1517430560419759236,3034580745374500872,6068881115283853584,12137481855085847073,5827939256702092354,11583539444389546116,287952221336838465,576187017162015362,1080597919557780484,2089419720071055368,4107063321064181776,8142350518772179232,16212923818971513409,13907045970195547266,18375536441101992321,18376667877509071362,18378650374858081284,18382614274322728968,18390542064695644176,18406396550208102432,18437825145767870784,18428906217826189953};
int Board::staticExchange(Move m){
	PieceType type = PAWN;
	U64 to = getSquare[to_sq(m)];
	if((to & (boardInfo->WhitePawnBB | boardInfo->BlackPawnBB)) != 0){
		type = PAWN;
	}else if((to & (boardInfo->WhiteKnightBB | boardInfo->BlackKnightBB)) != 0){
		type = KNIGHT;
	}else if((to & (boardInfo->WhiteBishopBB | boardInfo->BlackBishopBB)) != 0){
		type = BISHOP;
	}else if((to & (boardInfo->WhiteRookBB | boardInfo->BlackRookBB)) != 0){
		type = ROOK;
	}else if((to & (boardInfo->WhiteQueenBB | boardInfo->BlackQueenBB)) != 0){
		type = QUEEN;
	}
	return staticExchange(from_sq(m), to_sq(m), PieceMoved(m), type); 
}
//Static exchange is copied off chessprogramming
int seeValues[] = {100,300,300,500,900,999999};
int Board::staticExchange(U8 from, U8 to, PieceType moving, PieceType captured){
	int gain[32];
	int d = 0;
	U64 xrays = boardInfo->WhiteKnightBB | boardInfo->BlackKnightBB | boardInfo->WhiteBishopBB | boardInfo->BlackBishopBB | boardInfo->WhiteRookBB | boardInfo->BlackRookBB | boardInfo->WhiteQueenBB | boardInfo->BlackQueenBB;
	U64 fromSquare = getSquare[from];
	U64 attackTo = getIndexAttacks(currentBoard(), to);
	U64 occupied = boardInfo->AllPiecesBB;
	gain[d]  = seeValues[captured];
	do {
		U64 side;
		if(((d % 2 != 0) && boardInfo->whiteToMove) || ((d % 2 == 0) && !boardInfo->whiteToMove)){
			side = boardInfo->WhitePiecesBB;
		}else{
			side = boardInfo->BlackPiecesBB;
		}
		
		d++;
		gain[d] = seeValues[moving]-gain[d-1];
		if( (-gain[d-1] > gain[d] ? -gain[d-1] : gain[d]) < 0){break;}
		attackTo ^= fromSquare;
		occupied ^= fromSquare;
		if((fromSquare & xrays) != 0){
			attackTo |= getXrayAttacks(currentBoard(), to, occupied);
		}
		fromSquare = 0;
		PieceType type;
		
		U64 startLocs = 0;
		if((attackTo & side & (boardInfo->WhitePawnBB | boardInfo->BlackPawnBB)) != 0){
			type = PAWN;
			startLocs = attackTo & side & (boardInfo->WhitePawnBB | boardInfo->BlackPawnBB);
		}else if((attackTo & side & (boardInfo->WhiteKnightBB | boardInfo->BlackKnightBB)) != 0){
			type = KNIGHT;
			startLocs = attackTo & side & (boardInfo->WhiteKnightBB | boardInfo->BlackKnightBB);
		}else if((attackTo & side & (boardInfo->WhiteBishopBB | boardInfo->BlackBishopBB)) != 0){
			type = BISHOP;
			startLocs = attackTo & side & (boardInfo->WhiteBishopBB | boardInfo->BlackBishopBB);
		}else if((attackTo & side & (boardInfo->WhiteRookBB | boardInfo->BlackRookBB)) != 0){
			type = ROOK;
			startLocs = attackTo & side & (boardInfo->WhiteRookBB | boardInfo->BlackRookBB);
		}else if((attackTo & side & (boardInfo->WhiteQueenBB | boardInfo->BlackQueenBB)) != 0){
			type = QUEEN;
			startLocs = attackTo & side & (boardInfo->WhiteQueenBB | boardInfo->BlackQueenBB);
		}else if((attackTo & side & (boardInfo->WhiteKingBB | boardInfo->BlackKingBB)) != 0){
			type = KING;
			startLocs = attackTo & side & (boardInfo->WhiteKingBB| boardInfo->BlackKingBB);
		}
		moving=type;
		fromSquare = lowestOneBit(startLocs);
	}while(fromSquare != 0);
	while(--d){
		gain[d-1] = - ((-gain[d-1] > gain[d] ? -gain[d-1] : gain[d]));
	}
	return gain[0];
}



bool Board::isMoveCheck(Move m){
	fastMakeMove(m);
	bool result = isOwnKingInCheck();
	undoMove();
	return result;
}

bool validCache;
bool isInCheckCache;
int Board::currentSideMaterial(){
	int KNIGHT = 300;
	int BISHOP = 320;
	int ROOK = 500;
	int QUEEN = 960;
	if(boardInfo->whiteToMove){
		return popcnt(boardInfo->WhiteKnightBB)*KNIGHT+popcnt(boardInfo->WhiteRookBB)*ROOK+popcnt(boardInfo->WhiteBishopBB)*BISHOP+popcnt(boardInfo->WhiteQueenBB)*QUEEN;
	}else{
		return popcnt(boardInfo->BlackKnightBB)*KNIGHT+popcnt(boardInfo->BlackRookBB)*ROOK+popcnt(boardInfo->BlackBishopBB)*BISHOP+popcnt(boardInfo->BlackQueenBB)*QUEEN;
	}
}

int Board::totalMaterial(){
	int KNIGHT = 300;
	int BISHOP = 320;
	int ROOK = 500;
	int QUEEN = 960;
	return popcnt(boardInfo->WhiteKnightBB)*KNIGHT+popcnt(boardInfo->WhiteRookBB)*ROOK+popcnt(boardInfo->WhiteBishopBB)*BISHOP+popcnt(boardInfo->WhiteQueenBB)*QUEEN + popcnt(boardInfo->BlackKnightBB)*KNIGHT+popcnt(boardInfo->BlackRookBB)*ROOK+popcnt(boardInfo->BlackBishopBB)*BISHOP+popcnt(boardInfo->BlackQueenBB)*QUEEN;
}

//Should make sure this works
bool Board::isDraw(){
	ExtMove moves[MAX_MOVES];
	if(getAllLegalMovesSize(this, moves) == 0){
		if(!(isSquareAttacked(boardInfo, boardInfo->WhiteKingBB, true)
				|| isSquareAttacked(boardInfo, boardInfo->BlackKingBB, false))){
			return true;
		}
	}
	int rep = 0;
	for(int i = boardInfo->moveNumber-boardInfo->fiftyMoveRule; i < boardInfo->moveNumber-2; i++){
		if(boards[i].zobrist == boardInfo->zobrist){
			rep++;
		}
		if(rep >= 2){
			return true;
		}
	}
	return false;}

bool Board::isRepetition(){
	int rep = 0;
	for(int i = boardInfo->moveNumber-boardInfo->fiftyMoveRule; i < boardInfo->moveNumber-2; i++){
		if(boards[i].zobrist == boardInfo->zobrist){
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
	ExtMove moves[MAX_MOVES];
	if(isOwnKingInCheck()){
		if(getAllLegalMovesSize(this, moves) == 0){
			return true;
		}
	}
	return false;
}
//We are in check if it is our turn and they can take our king
bool Board::isOwnKingInCheck(){
	//if(validCache){
	//	return isInCheckCache;
	//}
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
  if(board->moveNumber > MAX_MOVECOUNT){
	board->moveNumber =1;
  }
  board->zobrist = initKeyFromBoard(this);
  validCache = false;
  setPinnedPieces();
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
  if(!(boardInfo->blackQueenCastle || boardInfo->blackKingCastle || boardInfo->whiteQueenCastle || boardInfo->whiteKingCastle)){
	  ss<< '-';
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
/*
* Makes a move as quickly as possible. To do this, it ignores zobrist and castling variables.
* If any speed increase, probably small
*/
void Board::fastMakeMove(Move move){
	BoardInfo* newInfo = &boards[boardInfo->moveNumber+1];
	memcpy(newInfo,boardInfo,sizeof(BoardInfo));
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
			captureSquare = (boardInfo->whiteToMove) ? (moveDest - 8) : (moveDest + 8); 
			//newInfo->enPassantLoc=captureSquare;
		}
		U64 square = getSquare[captureSquare];
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
	}
	BitBoard * pieceToClear = getBitBoard(PieceMoved(move),newInfo->whiteToMove);
	*pieceToClear &=~getSquare[moveStart];
	
	
	
	
	if(boardInfo->whiteToMove){
		switch(PieceMoved(move)){
			case PAWN:
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
				
				boardInfo->WhiteRookBB|=getSquare[moveDest];
				break;
			case KING:
				
				if(type_of(move) == CASTLING){
					//If Kingside, assumes e1g1 would be castling. Pretty easy to break.
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
		}		
	}else{
		switch(PieceMoved(move)){
			case PAWN:
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
				
		}				
	}
	newInfo->whiteToMove=!newInfo->whiteToMove;
	updateSpecialBB(newInfo);
	validCache=false;
}
void Board::setPinnedPieces(){
	U64 pinned = 0;
	U64 rookAttackers;
	U64 bishopAttackers;
	U8 king;
	if(boardInfo->whiteToMove){	
		king = trailingZeroCount(boardInfo->WhiteKingBB);
		rookAttackers = rookSlides[trailingZeroCount(boardInfo->WhiteKingBB)] & (boardInfo->BlackRookBB | boardInfo->BlackQueenBB);
		bishopAttackers = bishopSlides[trailingZeroCount(boardInfo->WhiteKingBB)] & (boardInfo->BlackBishopBB | boardInfo->BlackQueenBB); 
	}else{
		king = trailingZeroCount(boardInfo->BlackKingBB);
		rookAttackers = rookSlides[trailingZeroCount(boardInfo->BlackKingBB)] & (boardInfo->WhiteRookBB | boardInfo->WhiteQueenBB);
		bishopAttackers = bishopSlides[trailingZeroCount(boardInfo->BlackKingBB)] & (boardInfo->WhiteBishopBB | boardInfo->WhiteQueenBB); 
	}
	U64 allAttackers = rookAttackers | bishopAttackers;
	while(allAttackers != 0){
		U64 lowBit = lowestOneBit(allAttackers);
		U64 piecesBetween=squaresBetween[king][trailingZeroCount(lowBit)]&boardInfo->AllPiecesBB;
		if((piecesBetween != 0) && !moreThanOneOccupant(piecesBetween)){
			pinned |= piecesBetween;
		}
		allAttackers ^= lowBit;
	}
	boardInfo->pinnedPieces = pinned;
}

void Board::makeMove(Move move){
	//TODO test this speed vs manually copying. No idea what's faster. also maybe *newInfo = *board Info
	//Copy current BoardInfo
	validCache = false;
	BoardInfo* newInfo = &boards[boardInfo->moveNumber+1];
	memcpy(newInfo,boardInfo,sizeof(BoardInfo));
	newInfo->moveNumber+=1;
	newInfo->fiftyMoveRule+=1;
	if(boardInfo->enPassantLoc!=ENPASSANT_NONE){
		newInfo->zobrist^=passantColumn[getFile(boardInfo->enPassantLoc)];
	}
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
		U64 square = getSquare[captureSquare];
		PieceType type;
		if(boardInfo->whiteToMove){
			if((boardInfo->BlackPawnBB & square) != 0){
					type=PAWN;
			}else if((boardInfo->BlackKingBB & square) != 0){
				type=KING;
			}else if((boardInfo->BlackBishopBB & square) != 0){
				type=BISHOP;
			}else if((boardInfo->BlackKnightBB & square) != 0){
				type=KNIGHT;
			}else if((boardInfo->BlackQueenBB & square) != 0){
				type=QUEEN;
			}else if((boardInfo->BlackRookBB & square) != 0){
				type=ROOK;
			}
			newInfo->BlackBishopBB&=~getSquare[captureSquare];
			newInfo->BlackKingBB&=~getSquare[captureSquare];
			newInfo->BlackKnightBB&=~getSquare[captureSquare];
			newInfo->BlackPawnBB&=~getSquare[captureSquare];
			newInfo->BlackRookBB&=~getSquare[captureSquare];
			newInfo->BlackQueenBB&=~getSquare[captureSquare];
		}else{
			if((boardInfo->WhitePawnBB & square) != 0){
				type=PAWN;
			}else if((boardInfo->WhiteKingBB & square) != 0){
				type=KING;
			}else if((boardInfo->WhiteBishopBB & square) != 0){
				type=BISHOP;
			}else if((boardInfo->WhiteKnightBB & square) != 0){
				type=KNIGHT;
			}else if((boardInfo->WhiteQueenBB & square) != 0){
				type=QUEEN;
			}else if((boardInfo->WhiteRookBB & square) != 0){
				 type=ROOK;
			}
			newInfo->WhiteBishopBB&=~getSquare[captureSquare];
			newInfo->WhiteKingBB&=~getSquare[captureSquare];
			newInfo->WhiteKnightBB&=~getSquare[captureSquare];
			newInfo->WhitePawnBB&=~getSquare[captureSquare];
			newInfo->WhiteRookBB&=~getSquare[captureSquare];
			newInfo->WhiteQueenBB&=~getSquare[captureSquare];
		}
		boardInfo->zobrist^= getSquareKey(!boardInfo->whiteToMove,captureSquare, type);

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
				if(newInfo->enPassantLoc!=ENPASSANT_NONE){
					newInfo->zobrist^=passantColumn[getFile(newInfo->enPassantLoc)];
				}
				//If promotion, we add a piece rather than a pawn in the moveDest
				if(type_of(move) == PROMOTION){
					newInfo->zobrist^=getSquareKey(true,moveStart,PAWN);
					switch(promotion_type(move)){
						case QUEEN:
							boardInfo->WhiteQueenBB|=getSquare[moveDest];
							newInfo->zobrist^=getSquareKey(true,moveDest,QUEEN);
							break;
						case ROOK:
							boardInfo->WhiteRookBB|=getSquare[moveDest];
							newInfo->zobrist^=getSquareKey(true,moveDest,ROOK);
							break;
						case KNIGHT:
							boardInfo->WhiteKnightBB|=getSquare[moveDest];
							newInfo->zobrist^=getSquareKey(true,moveDest,KNIGHT);
							break;
						case BISHOP:
							boardInfo->WhiteBishopBB|=getSquare[moveDest];
							newInfo->zobrist^=getSquareKey(true,moveDest,BISHOP);
							break;	
					}
				}else{
					boardInfo->WhitePawnBB|=getSquare[moveDest];	
					newInfo->zobrist^=getKeyForMove(true,moveStart,moveDest,PAWN);
				}
				break;
			case KNIGHT:
				boardInfo->WhiteKnightBB|=getSquare[moveDest];
				newInfo->zobrist^=getKeyForMove(true,moveStart,moveDest,KNIGHT);
				break;
			case BISHOP:
				boardInfo->WhiteBishopBB|=getSquare[moveDest];
				newInfo->zobrist^=getKeyForMove(true,moveStart,moveDest,BISHOP);
				break;
			case ROOK:
				
				if(moveStart== SQ_H1){
					newInfo->whiteKingCastle=false;
					if(boardInfo->whiteKingCastle){
						newInfo->zobrist^=whiteKingSideCastling;
					}
				}else if(moveStart==SQ_A1){
					if(boardInfo->whiteQueenCastle){
						newInfo->zobrist^=whiteQueenSideCastling;
					}
					newInfo->whiteQueenCastle=false;
				}
				newInfo->zobrist^=getKeyForMove(true,moveStart,moveDest,ROOK);
				boardInfo->WhiteRookBB|=getSquare[moveDest];
				break;
			case KING:
				newInfo->whiteHasCastled=true;
				if(boardInfo->whiteKingCastle){
					newInfo->zobrist^=whiteKingSideCastling;
				}
				if(boardInfo->whiteQueenCastle){
					newInfo->zobrist^=whiteQueenSideCastling;
				}
				newInfo->whiteKingCastle=false;
				newInfo->whiteQueenCastle=false;
				if(type_of(move) == CASTLING){
					//If Kingside, assumes e1g1 would be castling. Pretty easy to break.
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
					newInfo->zobrist^=getKeyForMove(true,rookFrom,rookTo,ROOK);
				}
				newInfo->zobrist^=getKeyForMove(true,moveStart,moveDest,KING);
				boardInfo->WhiteKingBB|=getSquare[moveDest];
				break;
			case QUEEN:
				newInfo->zobrist^=getKeyForMove(true,moveStart,moveDest,QUEEN);
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
				
				if(newInfo->enPassantLoc!=ENPASSANT_NONE){
					newInfo->zobrist^=passantColumn[getFile(newInfo->enPassantLoc)];
				}
				if(type_of(move) == PROMOTION){
					newInfo->zobrist^=getSquareKey(false,moveStart,PAWN);

					switch(promotion_type(move)){
						case QUEEN:
							boardInfo->BlackQueenBB|=getSquare[moveDest];
							newInfo->zobrist^=getSquareKey(false,moveDest,QUEEN);
							break;
						case ROOK:
							boardInfo->BlackRookBB|=getSquare[moveDest];
							newInfo->zobrist^=getSquareKey(false,moveDest,ROOK);
							break;
						case KNIGHT:
							boardInfo->BlackKnightBB|=getSquare[moveDest];
							newInfo->zobrist^=getSquareKey(false,moveDest,KNIGHT);
							break;
						case BISHOP:
							boardInfo->BlackBishopBB|=getSquare[moveDest];
							newInfo->zobrist^=getSquareKey(false,moveDest,BISHOP);
							break;	
					}
				}else{
					newInfo->zobrist^=getKeyForMove(false,moveStart,moveDest,PAWN);
					boardInfo->BlackPawnBB|=getSquare[moveDest];
				}
				break;
			case KNIGHT:
				newInfo->zobrist^=getKeyForMove(false,moveStart,moveDest,KNIGHT);
				boardInfo->BlackKnightBB|=getSquare[moveDest];
				break;
			case BISHOP:
				newInfo->zobrist^=getKeyForMove(false,moveStart,moveDest,BISHOP);
				boardInfo->BlackBishopBB|=getSquare[moveDest];
				break;
			case ROOK:
				if(moveStart==SQ_H8){
					if(boardInfo->blackKingCastle){
						newInfo->zobrist^=blackKingSideCastling;
					}
					newInfo->blackKingCastle=false;
				}else if(moveStart==SQ_A8){
					if(boardInfo->blackQueenCastle){
						newInfo->zobrist^=blackQueenSideCastling;
					}
					newInfo->blackQueenCastle=false;
				}
				newInfo->zobrist^=getKeyForMove(false,moveStart,moveDest,ROOK);
				boardInfo->BlackRookBB|=getSquare[moveDest];
				break;
			case KING:
				newInfo->blackHasCastled=true;
				if(boardInfo->whiteKingCastle){
					newInfo->zobrist^=blackKingSideCastling;
				}
				if(boardInfo->whiteQueenCastle){
					newInfo->zobrist^=blackQueenSideCastling;
				}
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
					newInfo->zobrist^=getKeyForMove(false,rookFrom,rookTo,ROOK);

					boardInfo->BlackRookBB|=getSquare[rookTo];
					boardInfo->BlackRookBB^=getSquare[rookFrom];
				}
				newInfo->zobrist^=getKeyForMove(false,moveStart,moveDest,KING);

				boardInfo->BlackKingBB|=getSquare[moveDest];				
				break;
			case QUEEN:
				newInfo->zobrist^=getKeyForMove(false,moveStart,moveDest,QUEEN);
				boardInfo->BlackQueenBB|=getSquare[moveDest];
				break;
			default:
				printf("ERROR: IMPOSSIBLE PIECE MAKE MOVE\n");
		}				
	}
	newInfo->whiteToMove=!newInfo->whiteToMove;
	newInfo->zobrist^=whiteMove;
	updateSpecialBB(newInfo);
	newInfo->lastMove = move;
	setPinnedPieces();
}

void Board::undoMove(){
	boardInfo=boardInfo->previousBoard;
	validCache=false;
}

void Board::makeNullMove(){
	BoardInfo* newInfo = &boards[boardInfo->moveNumber+1];
	memcpy(newInfo,boardInfo,sizeof(BoardInfo));
	newInfo->moveNumber+=1;
	newInfo->fiftyMoveRule+=1;
	newInfo->enPassantLoc=ENPASSANT_NONE;
	newInfo->previousBoard = boardInfo;
	newInfo->zobrist^=whiteMove;

	if(boardInfo->enPassantLoc!=ENPASSANT_NONE){
		newInfo->zobrist^=passantColumn[getFile(boardInfo->enPassantLoc)];
	}
	newInfo->whiteToMove=!newInfo->whiteToMove;
	boardInfo=newInfo;
	newInfo->lastMove = 0;
	validCache=false;
	setPinnedPieces();
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
