
#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "stdafx.h"
#include "bitboard.h"
#include <string>
#include "move.h"

struct BoardInfo{
	U64 zobrist;
	//Current stuff
	U16 moveNumber;
	bool whiteToMove;
	
	//Castling
	U8 whiteKingCastle;
	U8 whiteQueenCastle;
	U8 blackKingCastle;
	U8 blackQueenCastle;
	U8 whiteHasCastled;
	U8 blackHasCastled;
	
	//Pawn stuff
	U8 fiftyMoveRule;
	U8 enPassantLoc;
	
	//Position stuff
	BitBoard WhiteKingBB;
	BitBoard WhiteQueenBB;
	BitBoard WhiteRookBB;
	BitBoard WhiteBishopBB;
	BitBoard WhiteKnightBB;
	BitBoard WhitePawnBB;

	BitBoard BlackKingBB;
	BitBoard BlackQueenBB;
	BitBoard BlackRookBB;
	BitBoard BlackBishopBB;
	BitBoard BlackKnightBB;
	BitBoard BlackPawnBB;
	Move lastMove;
	
	//TODO: Consider not doing this. Instead recalculate when BoardInfo is loaded. 
	BitBoard WhitePiecesBB;
	BitBoard BlackPiecesBB;
	BitBoard AllPiecesBB;
	
	U64 pinnedPieces;
	BoardInfo * previousBoard;//in Transpo table need depth, full board info, value, best move
};

const U16 MAX_MOVECOUNT = 512;
class Board{
	public: 
		bool isDraw();
		bool isOwnKingInCheck();
		bool legal();
		bool isCheckmate();
		bool isMoveCheck(Move m);
		bool isRepetition();
		int currentSideMaterial();
		int totalMaterial();

		int staticExchange(U8 from, U8 to, PieceType moving, PieceType captured);
		int staticExchange(Move m);

		std::string getFen();
		Board& readFromFen(std::string& fen, BoardInfo* board);
		
		
		void fastMakeMove(Move move);
		void makeMove(Move m);
		void undoMove();
		void makeNullMove();
		BoardInfo * currentBoard();
	private:
		void updateSpecialBB(BoardInfo * board);
		char getPieceAtChar(U8 loc);
		BitBoard* getBitBoard(PieceType type, bool whiteToMove);
		BitBoard getAllPiecesBitBoard(bool whiteToMove);
		BoardInfo* boardInfo;
		void setPinnedPieces();
	
};
extern bool validCache;
#endif