#include "movegen.h"
#include "bitboard.h"
#include "bbmagic.h"
Move moveList[MAX_MOVES]={};
U64 kingMoves[] =
			{ 0x303L, 0x707L, 0xe0eL, 0x1c1cL, 0x3838L, 0x7070L, 0xe0e0L, 0xc0c0L, 0x30303L, 0x70707L, 0xe0e0eL, 0x1c1c1cL, 0x383838L, 0x707070L, 0xe0e0e0L, 0xc0c0c0L, 0x3030300L, 0x7070700L, 0xe0e0e00L, 0x1c1c1c00L, 0x38383800L, 0x70707000L, 0xe0e0e000L, 0xc0c0c000L, 0x303030000L, 0x707070000L, 0xe0e0e0000L, 0x1c1c1c0000L, 0x3838380000L, 0x7070700000L, 0xe0e0e00000L, 0xc0c0c00000L, 0x30303000000L, 0x70707000000L, 0xe0e0e000000L, 0x1c1c1c000000L, 0x383838000000L, 0x707070000000L, 0xe0e0e0000000L, 0xc0c0c0000000L, 0x3030300000000L, 0x7070700000000L, 0xe0e0e00000000L, 0x1c1c1c00000000L, 0x38383800000000L, 0x70707000000000L, 0xe0e0e000000000L, 0xc0c0c000000000L, 0x303030000000000L, 0x707070000000000L, 0xe0e0e0000000000L, 0x1c1c1c0000000000L, 0x3838380000000000L, 0x7070700000000000L, 0xe0e0e00000000000L, 0xc0c0c00000000000L, 0x303000000000000L, 0x707000000000000L, 0xe0e000000000000L, 0x1c1c000000000000L, 0x3838000000000000L, 0x7070000000000000L, 0xe0e0000000000000L, 0xc0c0000000000000L };
	
U64 knightMoves[] =
			{ 0x20400L, 0x50800L, 0xa1100L, 0x142200L, 0x284400L, 0x508800L, 0xa01000L, 0x402000L, 0x2040004L, 0x5080008L, 0xa110011L, 0x14220022L, 0x28440044L, 0x50880088L, 0xa0100010L, 0x40200020L, 0x204000402L, 0x508000805L, 0xa1100110aL, 0x1422002214L, 0x2844004428L, 0x5088008850L, 0xa0100010a0L, 0x4020002040L, 0x20400040200L, 0x50800080500L, 0xa1100110a00L, 0x142200221400L, 0x284400442800L, 0x508800885000L, 0xa0100010a000L, 0x402000204000L, 0x2040004020000L, 0x5080008050000L, 0xa1100110a0000L, 0x14220022140000L, 0x28440044280000L, 0x50880088500000L, 0xa0100010a00000L, 0x40200020400000L, 0x204000402000000L, 0x508000805000000L, 0xa1100110a000000L, 0x1422002214000000L, 0x2844004428000000L, 0x5088008850000000L, 0xa0100010a0000000L, 0x4020002040000000L, 0x400040200000000L, 0x800080500000000L, 0x1100110a00000000L, 0x2200221400000000L, 0x4400442800000000L, 0x8800885000000000L, 0x100010a000000000L, 0x2000204000000000L, 0x4020000000000L, 0x8050000000000L, 0x110a0000000000L, 0x22140000000000L, 0x44280000000000L, 0x88500000000000L, 0x10a00000000000L, 0x20400000000000L };
Move * getMoveList(){
	return moveList;	
}
U64 pseudoLegalKnightMoveDestinations(U8 loc, U64 targets) {
	return knightMoves[loc] & targets;
}
	
U64 pseudoLegalKingMoveDestinations(U8 loc, U64 targets) {
	return kingMoves[loc] & targets;
}

U8 getAllBlackMoves(BoardInfo * boardInfo, Move list[]){
	int idx = 0;
	idx+= getBlackBishopMoves(boardInfo, list, idx);
	idx+=getBlackKnightMoves(boardInfo, list, idx);
	idx+= getBlackKingMoves(boardInfo, list, idx);
	idx+=getBlackRookMoves(boardInfo, list, idx);
	idx+=getBlackQueenMoves(boardInfo, list, idx);
	idx+=getBlackPawnMoves(boardInfo, list, idx);

	return idx;
}

U8 getAllWhiteMoves(BoardInfo* boardInfo, Move list[]){
	int idx = 0;
	idx+= getWhiteBishopMoves(boardInfo, list, idx);
	idx+=getWhiteKnightMoves(boardInfo, list, idx);
	idx+= getWhiteKingMoves(boardInfo, list, idx);
	idx+=getWhiteRookMoves(boardInfo, list, idx);
	idx+=getWhiteQueenMoves(boardInfo, list, idx);
	idx+=getWhitePawnMoves(boardInfo, list, idx);
	return idx;
}
U8 getAllPseudoLegalMoves(BoardInfo * boardInfo, Move list[]){
	if(boardInfo->whiteToMove){
		return getAllWhiteMoves(boardInfo, list);
	}
	return getAllBlackMoves(boardInfo, list);
}
//Pretty incredible. This is 5x slower.
U8 getAllLegalMoves(Board* b, Move list[]){
	//WRONG: THIS IS NOT FOR LEGAL BUT RATHER QUISIENCE
	//For pseudo legal moves (r : list)
	//Also actually generate by piece rather than calling PseudoLegal.
	//If KING, CHECK.
	//If Bishop of same color, check in check.
	//If knight of same color, check in check.
	//If rook, if ending row ==KING_ROW| ending column==KING_COLUMN
	//Queen, could micro-optimize via rook and bishop
	//Pawn, if near enemy king.
	
	//TODO
	//https://chess.stackexchange.com/questions/16890/efficient-ways-to-go-from-pseduo-legal-to-fully-legal-move-generation?rq=1
	//https://peterellisjones.com/posts/generating-legal-chess-moves-efficiently/
	//https://chess.stackexchange.com/questions/15705/c-vs-java-engine-move-generation-performance
	
	//FOR NOW:
	U8 count = getAllPseudoLegalMoves(b->currentBoard(), list);
	int j = 0;
	for (int i = 0; i < count; i++) {
		b->makeMove(list[i]);
		if(b->legal()){
			list[j++] = list[i];
		}else{
		}
		b->undoMove();
		
	}
	return j;

}

U8 getWhiteKingMoves(BoardInfo* b, Move moves[], int index) {
	U64 king = b->WhiteKingBB;
	
	int num_moves_generated = 0;
	
	U8 from_loc = trailingZeroCount(king);
	U64 movelocs = pseudoLegalKingMoveDestinations(from_loc, ~b->WhitePiecesBB);
	
	while (movelocs != 0L) {
		U64 to = lowestOneBit(movelocs);
		U8 to_loc = trailingZeroCount(to);
		Move move =createMove(from_loc, to_loc, KING);
		moves[index + num_moves_generated] = move;
		num_moves_generated++;
		movelocs &= ~to;
	}
	
	if (b->whiteKingCastle) {
		if ( (king << 1 & b->AllPiecesBB) == 0L
				&& (king << 2 & b->AllPiecesBB) == 0L) {
			if ( (king << 3 & b->WhiteRookBB) != 0L) {
				if (!isSquareAttacked(b, king, true)
						&& !isSquareAttacked(b, king << 1,
								true)
						&& !isSquareAttacked(b, king << 2,
								true)) {
					moves[index + num_moves_generated] =
							createMove(from_loc, from_loc + 2, KING, KING, CASTLING);
					num_moves_generated++;
				}
			}
		}
	}
	
	if (b->whiteQueenCastle) {
		if ( (king >> 1 & b->AllPiecesBB) == 0L
				&& (king>> 2 & b->AllPiecesBB) == 0L
				&& (king>> 3 & b->AllPiecesBB) == 0L) {
			if ( (king>> 4 & b->WhiteRookBB) != 0L) {
				if ( !isSquareAttacked(b, king, true)
						&& !isSquareAttacked(b, king >> 1,
								true)
						&& !isSquareAttacked(b, king >> 2,
								true)) {
					moves[index + num_moves_generated] =
							createMove(from_loc, from_loc - 2, KING, KING, CASTLING);
					num_moves_generated++;
				}
			}
		}
	}
	
	return num_moves_generated;
}
U8 getBlackKingMoves(BoardInfo* b, Move moves[], int index) {
	U64 king = b->BlackKingBB;
	
	int num_moves_generated = 0;
	
	U8 from_loc = trailingZeroCount(king);
	U64 movelocs = pseudoLegalKingMoveDestinations(from_loc, ~b->BlackPiecesBB);
	
	while (movelocs != 0L) {
		U64 to = lowestOneBit(movelocs);
		U8 to_loc = trailingZeroCount(to);
		Move move = createMove(from_loc, to_loc, KING);
		moves[index + num_moves_generated] = move;
		num_moves_generated++;
		movelocs &= ~to;
	}
	
	if (b->blackKingCastle) {
		if ( (b->BlackKingBB << 1 & b->AllPiecesBB) == 0L
				&& (b->BlackKingBB << 2 & b->AllPiecesBB) == 0L) {
			if ( (b->BlackKingBB << 3 & b->BlackRookBB) != 0L) {
				if ( !isSquareAttacked(b, b->BlackKingBB, false)
						&& !isSquareAttacked(b, b->BlackKingBB << 1,
								false)
						&& !isSquareAttacked(b, b->BlackKingBB << 2,
								false)) {
					moves[index + num_moves_generated] =
							createMove(from_loc, from_loc + 2, KING, KING, CASTLING);
					num_moves_generated++;
				}
			}
		}
	}
	
	if (b->blackQueenCastle) {
		if ( (b->BlackKingBB >> 1 & b->AllPiecesBB) == 0L
				&& (b->BlackKingBB >> 2 & b->AllPiecesBB) == 0L
				&& (b->BlackKingBB >> 3 & b->AllPiecesBB) == 0L) {
			if ( (b->BlackKingBB >> 4 & b->BlackRookBB) != 0L) {
				if ( !isSquareAttacked(b, b->BlackKingBB, false)
						&& !isSquareAttacked(b, b->BlackKingBB >> 1,
								false)
						&& !isSquareAttacked(b, b->BlackKingBB >> 2,
								false)) {
					moves[index + num_moves_generated] =
							createMove(from_loc, from_loc - 2, KING,
									KING, CASTLING);
					num_moves_generated++;
				}
			}
		}
	}
	
	return num_moves_generated;
}
U8 getWhiteKnightMoves(BoardInfo *b, Move moves[], int index) {
	U64 knights = b->WhiteKnightBB;
	U8 num_moves_generated = 0;
	
	while (knights != 0L) {
		U64 from = lowestOneBit(knights);
		U8 from_loc = trailingZeroCount(from);
		U64 movelocs = pseudoLegalKnightMoveDestinations(from_loc, ~b->WhitePiecesBB);
		
		while (movelocs != 0L) {
			U64 to = lowestOneBit(movelocs);
			U8 to_loc = trailingZeroCount(to);
			Move move =
					createMove(from_loc, to_loc, KNIGHT);
			moves[index + num_moves_generated] = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		knights &= ~from;
	}
	return num_moves_generated;
}


U8 getBlackKnightMoves(BoardInfo* b,Move moves[], int index) {
	U64 knights = b->BlackKnightBB;
	int num_moves_generated = 0;
	
	while (knights != 0L) {
		U64 from = lowestOneBit(knights);
		U8 from_loc = trailingZeroCount(from);
		U64 movelocs = pseudoLegalKnightMoveDestinations(from_loc, ~b->BlackPiecesBB);
		
		while (movelocs != 0L) {
			U64 to = lowestOneBit(movelocs);
			U8 to_loc = trailingZeroCount(to);
			Move move =
					createMove(from_loc, to_loc, KNIGHT);
			moves[index + num_moves_generated] = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		knights &= ~from;
	}
	return num_moves_generated;
}
U8 getWhitePawnMoves(BoardInfo * b, Move list[], int listIdx){
	U64 pawns = b->WhitePawnBB;
	int moveGenCount = 0;
	
	while(pawns != 0){
		U64 fromBB = lowestOneBit(pawns);
		U8 fromLoc = trailingZeroCount(fromBB);
		
		//If pawn is on seventh rank
		if((fromBB & maskRank[RANK_7]) != 0){
			//If pawn is not on A file, and there is a piece to capture upper left 
			if(((fromBB & maskFile[FILE_A]) == 0) &&  (((fromBB << 7 ) & (b->BlackPiecesBB)) != 0)){
				U8 destLoc = trailingZeroCount(fromBB << 7);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
			}
			//If pawn not on H file, and there is a piece to capture upper right
			if(((fromBB & maskFile[FILE_H]) ==0) && (((fromBB <<9) & (b->BlackPiecesBB)) != 0)){
				U8 destLoc = trailingZeroCount(fromBB << 9);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
			}
			//If next square clear
			if(((fromBB << 8) & (b->AllPiecesBB)) == 0){
				U8 destLoc = trailingZeroCount(fromBB << 8);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
			}
		}else{
			//Pawn not on seventh rank
			
			//Leftwards captures.
			if(((fromBB & maskFile[FILE_A]) == 0) && ((((fromBB << 7) & (b->BlackPiecesBB)) != 0) || (trailingZeroCount(fromBB << 7) == (b->enPassantLoc)))){
				U8 destLoc = trailingZeroCount(fromBB << 7);
				Move m;
				if(destLoc == (b->enPassantLoc)){
					m = createMove(fromLoc, destLoc, PAWN, KING, ENPASSANT);
				}else{
					m = createMove(fromLoc, destLoc, PAWN);
				}
				list[listIdx+moveGenCount] = m;			
				moveGenCount++;
			}
			
			//Rightwards captures
			if(((fromBB & maskFile[FILE_H]) == 0) && ((((fromBB << 9) & (b->BlackPiecesBB)) != 0) || (trailingZeroCount(fromBB << 9) == (b->enPassantLoc)))){
				U8 destLoc = trailingZeroCount(fromBB << 9);
				Move m;
				if(destLoc == (b->enPassantLoc)){
					m = createMove(fromLoc, destLoc, PAWN, KING, ENPASSANT);
				}else{
					m = createMove(fromLoc, destLoc, PAWN);
				}
				list[listIdx+moveGenCount] = m;			
				moveGenCount++;
			}
			Move m;
			//Normal move
			bool nextSquareClear = false;
			if(((fromBB << 8) & (b->AllPiecesBB)) == 0){
				nextSquareClear=true;
				U8 destLoc = trailingZeroCount(fromBB << 8);
				m = createMove(fromLoc, destLoc, PAWN);
				list[listIdx+moveGenCount] = m;			
				moveGenCount++;
			}
			//2 Squares
			if(((fromBB & maskRank[RANK_2]) != 0) && nextSquareClear && (((fromBB << 16) & (b->AllPiecesBB)) == 0)){
				U8 destLoc = trailingZeroCount(fromBB << 16);
				m = createMove(fromLoc, destLoc, PAWN);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
			}
		}
		pawns = pawns &~fromBB;
	}
	return moveGenCount;
}


U8 getBlackPawnMoves(BoardInfo * b, Move list[], int listIdx){
	U64 pawns = b->BlackPawnBB;
	int moveGenCount = 0;
	
	while(pawns != 0){
		U64 fromBB = lowestOneBit(pawns);
		U8 fromLoc = trailingZeroCount(fromBB);
		
		//If pawn is on second rank
		if((fromBB & maskRank[RANK_2]) != 0){
			//If pawn is not on A file, and there is a piece to capture upper left 
			if(((fromBB & maskFile[FILE_H]) == 0) &&  (((fromBB >> 7) & (b->WhitePiecesBB)) != 0)){
				U8 destLoc = trailingZeroCount(fromBB >> 7);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
			}
			//If pawn not on H file, and there is a piece to capture upper right
			if(((fromBB & maskFile[FILE_A]) ==0) && (((fromBB >>9) & (b->WhitePiecesBB)) != 0)){
				U8 destLoc = trailingZeroCount(fromBB >>9);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
			}
			//If next square clear
			if(((fromBB >>8) & (b->AllPiecesBB)) == 0){
				U8 destLoc = trailingZeroCount(fromBB >> 8);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
			}
		}else{
			//Pawn not on seventh rank
			
			//Leftwards captures.
			if(((fromBB & maskFile[FILE_H]) == 0) && ((((fromBB >> 7) & (b->WhitePiecesBB)) != 0) || (trailingZeroCount(fromBB >> 7) == (b->enPassantLoc)))){
				U8 destLoc = trailingZeroCount(fromBB >> 7);
				Move m;
				if(destLoc == (b->enPassantLoc)){
					m = createMove(fromLoc, destLoc, PAWN, KING, ENPASSANT);
				}else{
					m = createMove(fromLoc, destLoc, PAWN);
				}
				list[listIdx+moveGenCount] = m;			
				moveGenCount++;
			}
			
			//Rightwards captures
			if(((fromBB & maskFile[FILE_A]) == 0) && ((((fromBB >> 9) & (b->WhitePiecesBB)) != 0) || (trailingZeroCount(fromBB >> 9) == (b->enPassantLoc)))){
				U8 destLoc = trailingZeroCount(fromBB >> 9);
				Move m;
				if(destLoc == (b->enPassantLoc)){
					m = createMove(fromLoc, destLoc, PAWN, KING, ENPASSANT);
				}else{
					m = createMove(fromLoc, destLoc, PAWN);
				}
				list[listIdx+moveGenCount] = m;			
				moveGenCount++;
			}
			//Normal move
			bool nextSquareClear = false;
			if(((fromBB >> 8) & (b->AllPiecesBB)) == 0){
				nextSquareClear=true;
				Move m;
				U8 destLoc = trailingZeroCount(fromBB >> 8);
				m = createMove(fromLoc, destLoc, PAWN);
				list[listIdx+moveGenCount] = m;			
				moveGenCount++;
			}
			//2 Squares
			if(((fromBB & maskRank[RANK_7]) != 0) && nextSquareClear && (((fromBB >> 16) & (b->AllPiecesBB)) == 0)){
				Move m;
				U8 destLoc = trailingZeroCount(fromBB >> 16);
				m = createMove(fromLoc, destLoc, PAWN);
				list[listIdx+moveGenCount] = m;
				moveGenCount++;
			}
		}
		pawns = pawns &~fromBB;
	}
	return moveGenCount;
}

U8 getWhiteBishopMoves(BoardInfo * b, Move moves[], int index) {
	U64 bishops = b->WhiteBishopBB;
	U8 num_moves_generated = 0;
	
	while (bishops != 0L) {
		U64 from = lowestOneBit(bishops);
		U8 from_loc = trailingZeroCount(from);
		U64 movelocs =
				getBishopAttacks(from_loc, b->AllPiecesBB & ~from);
		movelocs &= ~b->WhitePiecesBB;
		
		while (movelocs != 0L) {
			U64 to = lowestOneBit(movelocs);
			U8 to_loc = trailingZeroCount(to);
			Move move =
					createMove(from_loc, to_loc, BISHOP);
			moves[index + num_moves_generated] = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		bishops &= ~from;
	}
	return num_moves_generated;
}
U8 getBlackBishopMoves(BoardInfo* b,Move moves[], int index) {
	U64 bishops = b->BlackBishopBB;
	U8 num_moves_generated = 0;
	
	while (bishops != 0L) {
		U64 from = lowestOneBit(bishops);
		U8 from_loc = trailingZeroCount(from);
		U64 movelocs =
				getBishopAttacks(from_loc, b->AllPiecesBB & ~from);
		movelocs &= ~b->BlackPiecesBB;
		
		while (movelocs != 0L) {
			U64 to = lowestOneBit(movelocs);
			U8 to_loc = trailingZeroCount(to);
			Move move =
					createMove(from_loc, to_loc, BISHOP);
			moves[index + num_moves_generated] = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		bishops &= ~from;
	}
	return num_moves_generated;
}
U8 getWhiteRookMoves(BoardInfo* b, Move moves[], int index) {
	U64 rooks = b->WhiteRookBB;
	int num_moves_generated = 0;
	
	while (rooks != 0L) {
		U64 from = lowestOneBit(rooks);
		U8 from_loc = trailingZeroCount(from);
		U64 movelocs = getRookAttacks(from_loc, b->AllPiecesBB & ~from);
		movelocs &= ~b->WhitePiecesBB;
		
		while (movelocs != 0L) {
			U64 to = lowestOneBit(movelocs);
			U8 to_loc = trailingZeroCount(to);
			Move move =
					createMove(from_loc, to_loc, ROOK);
			moves[index + num_moves_generated] = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		rooks &= ~from;
	}
	return num_moves_generated;
}

U8 getBlackRookMoves(BoardInfo *b, Move moves[], int index) {
	U64 rooks = b->BlackRookBB;
	U8 num_moves_generated = 0;
	
	while (rooks != 0L) {
		U64 from = lowestOneBit(rooks);
		U8 from_loc = trailingZeroCount(from);
		U64 movelocs = getRookAttacks(from_loc, b->AllPiecesBB & ~from);
		movelocs &= ~b->BlackPiecesBB;
		
		while (movelocs != 0L) {
			U64 to = lowestOneBit(movelocs);
			U8 to_loc = trailingZeroCount(to);
			Move move =
					createMove(from_loc, to_loc, ROOK);
			moves[index + num_moves_generated] = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		rooks &= ~from;
	}
	return num_moves_generated;
}


U8 getWhiteQueenMoves(BoardInfo* b, Move moves[], int index) {
	U64 queens = b->WhiteQueenBB;
	int num_moves_generated = 0;
	
	while (queens != 0L) {
		U64 from = lowestOneBit(queens);
		U8 from_loc = trailingZeroCount(from);
		U64 movelocs =
				getQueenAttacks(from_loc, b->AllPiecesBB & ~from);
		
		movelocs &= ~b->WhitePiecesBB;
		
		while (movelocs != 0L) {
			U64 to = lowestOneBit(movelocs);
			U8 to_loc = trailingZeroCount(to);
			Move move =
					createMove(from_loc, to_loc, QUEEN);
			moves[index + num_moves_generated] = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		queens &= ~from;
	}
	return num_moves_generated;
}

U8 getBlackQueenMoves(BoardInfo * b, Move moves[], int index) {
	U64 queens = b->BlackQueenBB;
	U8 num_moves_generated = 0;
	
	while (queens != 0L) {
		U64 from = lowestOneBit(queens);
		U8 from_loc = trailingZeroCount(from);
		U64 movelocs =
				getQueenAttacks(from_loc, b->AllPiecesBB & ~from);
		movelocs &= ~b->BlackPiecesBB;
		
		while (movelocs != 0L) {
			U64 to = lowestOneBit(movelocs);
			U8 to_loc = trailingZeroCount(to);
			Move move =
					createMove(from_loc, to_loc, QUEEN);
			moves[index + num_moves_generated] = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		queens &= ~from;
	}
	return num_moves_generated;
}

U8 Movegen::getAllCaptures(BoardInfo * b, Move moves[]){
	return 1;
}