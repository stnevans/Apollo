#include "board.h"
#include "bbmagic.h"
U8 rookShifts[] =
			{ 12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12 };
U8 bishopShifts[] =
			{ 6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6 };
U64 magicRookNumbers[] =
			{ 0x1080108000400020L, 0x40200010004000L, 0x100082000441100L, 0x480041000080080L, 0x100080005000210L, 0x100020801000400L, 0x280010000800200L, 0x100008020420100L, 0x400800080400020L, 0x401000402000L, 0x100801000200080L, 0x801000800800L, 0x800400080080L, 0x800200800400L, 0x1000200040100L, 0x4840800041000080L, 0x20008080004000L, 0x404010002000L, 0x808010002000L, 0x828010000800L, 0x808004000800L, 0x14008002000480L, 0x40002100801L, 0x20001004084L, 0x802080004000L, 0x200080400080L, 0x810001080200080L, 0x10008080080010L, 0x4000080080040080L, 0x40080020080L, 0x1000100040200L, 0x80008200004124L, 0x804000800020L, 0x804000802000L, 0x801000802000L, 0x2000801000800804L, 0x80080800400L, 0x80040080800200L, 0x800100800200L, 0x8042000104L, 0x208040008008L, 0x10500020004000L, 0x100020008080L, 0x2000100008008080L, 0x200040008008080L, 0x8020004008080L, 0x1000200010004L, 0x100040080420001L, 0x80004000200040L, 0x200040100140L, 0x20004800100040L, 0x100080080280L, 0x8100800400080080L, 0x8004020080040080L, 0x9001000402000100L, 0x40080410200L, 0x208040110202L, 0x800810022004012L, 0x1000820004011L, 0x1002004100009L, 0x41001002480005L, 0x81000208040001L, 0x4000008201100804L, 0x2841008402L };
U64 magicBishopNumbers[] =
			{ 0x1020041000484080L, 0x20204010a0000L, 0x8020420240000L, 0x404040085006400L, 0x804242000000108L, 0x8901008800000L, 0x1010110400080L, 0x402401084004L, 0x1000200810208082L, 0x20802208200L, 0x4200100102082000L, 0x1024081040020L, 0x20210000000L, 0x8210400100L, 0x10110022000L, 0x80090088010820L, 0x8001002480800L, 0x8102082008200L, 0x41001000408100L, 0x88000082004000L, 0x204000200940000L, 0x410201100100L, 0x2000101012000L, 0x40201008200c200L, 0x10100004204200L, 0x2080020010440L, 0x480004002400L, 0x2008008008202L, 0x1010080104000L, 0x1020001004106L, 0x1040200520800L, 0x8410000840101L, 0x1201000200400L, 0x2029000021000L, 0x4002400080840L, 0x5000020080080080L, 0x1080200002200L, 0x4008202028800L, 0x2080210010080L, 0x800809200008200L, 0x1082004001000L, 0x1080202411080L, 0x840048010101L, 0x40004010400200L, 0x500811020800400L, 0x20200040800040L, 0x1008012800830a00L, 0x1041102001040L, 0x11010120200000L, 0x2020222020c00L, 0x400002402080800L, 0x20880000L, 0x1122020400L, 0x11100248084000L, 0x210111000908000L, 0x2048102020080L, 0x1000108208024000L, 0x1004100882000L, 0x41044100L, 0x840400L, 0x4208204L, 0x80000200282020cL, 0x8a001240100L, 0x2040104040080L };
			
			
const U64 b_down = 0x00000000000000ffL;
const U64 b_up = 0xff00000000000000L;
const U64 b_right = 0x0101010101010101L;
const U64 b_left = 0x8080808080808080L;

// Thicker border for the knight generation in BBMagic
const U64 b2_down = 0x000000000000ffffL;
const U64 b2_up = 0xffff000000000000L;
const U64 b2_right = 0x0303030303030303L;
const U64 b2_left = 0xC0C0C0C0C0C0C0C0L;

// Even thicker, only bottom
const U64 b3_down = 0x0000000000ffffffL;
const U64 b3_up = 0xffffff0000000000L;

// Second-from top and second-from bottom ranks
const U64 r2_down = 0x000000000000ff00L;
const U64 r2_up = 0x00ff000000000000L;
U64 rook[64] = {};
U64 rookMagic[64][4096] = {};
U64 rookMask[64] = {};
U64 bishop[64] = {};
U64 bishopMask[64] = {};
U64 bishopMagic[64][512] = {};
U64 knight[64] ={};
U64 king[64] = {};
U64 whitePawn[64] = {};
U64 blackPawn[64] = {};


U64 squareAttackedAux(U64 square, U64 all, int shift, U64 border) {
	U64 ret = 0;
	while ( (square & border) == 0) {
		if (shift > 0)
			square <<= shift;
		else
			square >>= -shift;
		ret |= square;
		
		if ( (square & all) != 0)
			break;
	}
	return ret;
	}



U64 squareAttackedAux(U64 square, int shift, U64 border) {
	if ( (square & border) == 0) {
		if (shift > 0){
			square <<= shift;
		}else{
			square >>= (-shift);
		}
		return square;
	}
	return 0;
}
U64 squareAttackedAuxSlider(U64 square, int shift, U64 border) {
	U64 ret = 0;
	while ( (square & border) == 0) {
		if (shift > 0)
			square <<= shift;
		else
			square >>= -shift;
		ret |= square;
	}
	return ret;
}
U64 squareAttackedAuxSliderMask(U64 square, int shift, U64 border) {
	U64 ret = 0;
	while ( (square & border) == 0) {
		if (shift > 0)
			square <<= shift;
		else
			square >>= -shift;
		if ( (square & border) == 0)
			ret |= square;
	}
	return ret;
}

U64 generatePieces(U32 index, U32 bits, U64 mask) {
	U32 i;
	U64 lsb;
	U64 result = 0L;
	for (i = 0; i < bits; i++) {
		lsb = lowestOneBit(mask);
		mask ^= lsb;
		if ( (index & (1 << i)) != 0)
			result |= lsb;
	}
	return result;
}

U32 transform(U64 b, U64 magic, U8 bits) {
	return (U32) ( (b * magic) >> (64 - bits));
}


bool isSquareAttacked(BoardInfo* b, U64 square, bool doesBlackAttackSquare) {
	return isIndexAttacked(b, trailingZeroCount(square), doesBlackAttackSquare);
}
//Not do whitePiecesAttack square.
bool isIndexAttacked(BoardInfo* b, U8 i, bool whiteToMove) {
	if (i < 0 || i > 63)
		return false;
	U64 others = (whiteToMove ? b->BlackPiecesBB : b->WhitePiecesBB);
	U64 all = b->AllPiecesBB;
	
	if ( ( (whiteToMove ? whitePawn[i] : blackPawn[i])
			& (b->WhitePawnBB | b->BlackPawnBB) & others) != 0){
		return true;
	}
	if ( (king[i] & (b->WhiteKingBB | b->BlackKingBB) & others) != 0){
		return true;
	}
	if ( (knight[i] & (b->WhiteKnightBB | b->BlackKnightBB) & others) != 0){
		return true;
	}
	if ( (getRookAttacks(i, all)
			& ( (b->WhiteRookBB | b->BlackRookBB) | (b->WhiteQueenBB | b->BlackQueenBB)) & others) != 0){
		return true;
	}
	if ( (getBishopAttacks(i, all)
			& ( (b->WhiteBishopBB | b->BlackBishopBB) | (b->WhiteQueenBB | b->BlackQueenBB)) & others) != 0){
		return true;
	}
	return false;
}
bool isIndexAttackedWithoutKing(BoardInfo* b, U8 i, bool whiteToMove) {
	if (i < 0 || i > 63)
		return false;
	U64 others = (whiteToMove ? b->BlackPiecesBB : b->WhitePiecesBB);
	U64 all = b->AllPiecesBB^ (whiteToMove?b->WhiteKingBB : b->BlackKingBB);
	
	if ( ( (whiteToMove ? whitePawn[i] : blackPawn[i])
			& (b->WhitePawnBB | b->BlackPawnBB) & others) != 0){
		return true;
	}
	if ( (king[i] & (b->WhiteKingBB | b->BlackKingBB) & others) != 0){
		return true;
	}
	if ( (knight[i] & (b->WhiteKnightBB | b->BlackKnightBB) & others) != 0){
		return true;
	}
	if ( (getRookAttacks(i, all)
			& ( (b->WhiteRookBB | b->BlackRookBB) | (b->WhiteQueenBB | b->BlackQueenBB)) & others) != 0){
		return true;
	}
	if ( (getBishopAttacks(i, all)
			& ( (b->WhiteBishopBB | b->BlackBishopBB) | (b->WhiteQueenBB | b->BlackQueenBB)) & others) != 0){
		return true;
	}
	return false;
}
U64 getIndexAttacks(BoardInfo* b, int i) {
	if (i < 0 || i > 63)
		return 0;
	U64 all = b->AllPiecesBB;
	
	return  ((((b->BlackPiecesBB & whitePawn[i])
			| (b->WhitePiecesBB& blackPawn[i])) & (b->WhitePawnBB | b->BlackPawnBB))
			| (king[i] & (b->WhiteKingBB | b->BlackKingBB))
			| (knight[i] & (b->WhiteKnightBB | b->BlackKnightBB))
			| (getRookAttacks(i, all) & ( (b->WhiteRookBB | b->BlackRookBB) | (b->WhiteQueenBB | b->BlackQueenBB)))
			| (getBishopAttacks(i, all) & ( (b->WhiteBishopBB | b->BlackBishopBB) | (b->WhiteQueenBB | b->BlackQueenBB))));
}
/*
U64 getOpponentXrays(BoardInfo * b, int loc, bool considerWhitePieces){
	return getXrayAttacks(b, loc, considerWhitePieces ? b->WhitePiecesBB : b->BlackPiecesBB);
}*/

U64 getXrayAttacks(BoardInfo* b, int i) {
	return getXrayAttacks(b, i, b->AllPiecesBB);
}

U64 getXrayAttacks(BoardInfo* b, int i, U64 all) {
	if (i < 0 || i > 63)
		return 0;
	return ( (getRookAttacks(i, all) & ( (b->WhiteRookBB | b->BlackRookBB) | (b->WhiteQueenBB | b->BlackQueenBB))) | (getBishopAttacks(
			i, all) & ( (b->WhiteBishopBB | b->BlackBishopBB) | (b->WhiteQueenBB | b->BlackQueenBB))))
			& all;
}

U64 getXrayAttacksSlidings(BoardInfo* b, int i, U64 all);
U64 getXrayAttacksSliding(BoardInfo* b, int i) {
	return getXrayAttacksSlidings(b, i, b->AllPiecesBB);
}

U64 getXrayAttacksSlidings(BoardInfo* b, int i, U64 all) {
	if (i < 0 || i > 63)
		return 0;
	
	return ( (getRookAttacks(i, all) & ( (b->WhiteRookBB | b->BlackRookBB) | (b->WhiteQueenBB | b->BlackQueenBB))) | (getBishopAttacks(
			i, all) & ( (b->WhiteBishopBB | b->BlackBishopBB) | (b->WhiteQueenBB | b->BlackQueenBB))));
}

U64 getRookAttacks(int index, U64 all) {
	int i =
			transform(all & rookMask[index], magicRookNumbers[index],
					rookShifts[index]);
	return rookMagic[index][i];
}


U64 getBishopAttacks(int index, U64 all) {
	int i =
			transform(all & bishopMask[index], magicBishopNumbers[index],
					bishopShifts[index]);
	return bishopMagic[index][i];
}


U64 getQueenAttacks(int index, U64 all) {
	return getRookAttacks(index, all) | getBishopAttacks(index, all);
}



U64 getRookShiftAttacks(U64 square, U64 all) {
	return squareAttackedAux(square, all, +8, b_up)
			| squareAttackedAux(square, all, -8, b_down)
			| squareAttackedAux(square, all, -1, b_right)
			| squareAttackedAux(square, all, +1, b_left);
}
U64 getBishopShiftAttacks(U64 square, U64 all) {
	return squareAttackedAux(square, all, +9, b_up | b_left)
			| squareAttackedAux(square, all, +7, b_up | b_right)
			| squareAttackedAux(square, all, -7, b_down | b_left)
			| squareAttackedAux(square, all, -9, b_down | b_right);
}

void initBBMagic(){
	U64 square = 1;
	U8 i = 0;
	while(square != 0){
		rook[i]=
		squareAttackedAuxSlider(square, 8, b_up) | 	squareAttackedAuxSlider(square, -8, b_down) 
		| squareAttackedAuxSlider(square, -1, b_right) | squareAttackedAuxSlider(square, 1, b_left);
		rookMask[i] = squareAttackedAuxSliderMask(square, +8, b_up) //
					| squareAttackedAuxSliderMask(square, -8, b_down) //
					| squareAttackedAuxSliderMask(square, -1, b_right) //
					| squareAttackedAuxSliderMask(square, +1, b_left);
					
		bishop[i] =
					squareAttackedAuxSlider(square, +9, b_up | b_left) //
							| squareAttackedAuxSlider(square, +7, b_up
									| b_right) //
							| squareAttackedAuxSlider(square, -7, b_down
									| b_left) //
							| squareAttackedAuxSlider(square, -9, b_down
									| b_right);
		bishopMask[i] =
					squareAttackedAuxSliderMask(square, +9, b_up | b_left) //
							| squareAttackedAuxSliderMask(square, +7, b_up
									| b_right) //
							| squareAttackedAuxSliderMask(square, -7, b_down
									| b_left) //
							| squareAttackedAuxSliderMask(square, -9, b_down
									| b_right);
		knight[i] = squareAttackedAux(square, +17, b2_up | b_left) //
				| squareAttackedAux(square, +15, b2_up | b_right) //
				| squareAttackedAux(square, -15, b2_down | b_left) //
				| squareAttackedAux(square, -17, b2_down | b_right) //
				| squareAttackedAux(square, +10, b_up | b2_left) //
				| squareAttackedAux(square, +6, b_up | b2_right) //
				| squareAttackedAux(square, -6, b_down | b2_left) //
				| squareAttackedAux(square, -10, b_down | b2_right);
				
		whitePawn[i] = squareAttackedAux(square, 7, b_up | b_right) //
					| squareAttackedAux(square, 9, b_up | b_left);
		
		blackPawn[i] = squareAttackedAux(square, -7, b_down | b_left) //
				| squareAttackedAux(square, -9, b_down | b_right);
		
		king[i] = squareAttackedAux(square, +8, b_up) //
				| squareAttackedAux(square, -8, b_down) //
				| squareAttackedAux(square, -1, b_right) //
				| squareAttackedAux(square, +1, b_left) //
				| squareAttackedAux(square, +9, b_up | b_left) //
				| squareAttackedAux(square, +7, b_up | b_right) //
				| squareAttackedAux(square, -7, b_down | b_left) //
				| squareAttackedAux(square, -9, b_down | b_right);	
		
		int rookPositions = (1 << rookShifts[i]);
		for (int j = 0; j < rookPositions; j++) {
			U64 pieces =
					generatePieces(j, rookShifts[i], rookMask[i]);
			int magicIndex = transform(pieces, magicRookNumbers[i], rookShifts[i]);
			rookMagic[i][magicIndex] = getRookShiftAttacks(square, pieces);
		}
		
		int bishopPositions = (1 << bishopShifts[i]);
		for (int j = 0; j < bishopPositions; j++) {
			U64 pieces =
					generatePieces(j, bishopShifts[i], bishopMask[i]);
			int magicIndex =
					transform(pieces, magicBishopNumbers[i], bishopShifts[i]);
			bishopMagic[i][magicIndex] = getBishopShiftAttacks(square, pieces);
		}
		
		square <<= 1;
		i++;
		
	}
	square = 1<<1;
}


