#include "movegen.h"
#include "bitboard.h"
#include "bbmagic.h"
#include "search.h"//for the move ordering heuristics.
Move moveList[MAX_MOVES]={};
U64 kingMoves[] =
			{ 0x303L, 0x707L, 0xe0eL, 0x1c1cL, 0x3838L, 0x7070L, 0xe0e0L, 0xc0c0L, 0x30303L, 0x70707L, 0xe0e0eL, 0x1c1c1cL, 0x383838L, 0x707070L, 0xe0e0e0L, 0xc0c0c0L, 0x3030300L, 0x7070700L, 0xe0e0e00L, 0x1c1c1c00L, 0x38383800L, 0x70707000L, 0xe0e0e000L, 0xc0c0c000L, 0x303030000L, 0x707070000L, 0xe0e0e0000L, 0x1c1c1c0000L, 0x3838380000L, 0x7070700000L, 0xe0e0e00000L, 0xc0c0c00000L, 0x30303000000L, 0x70707000000L, 0xe0e0e000000L, 0x1c1c1c000000L, 0x383838000000L, 0x707070000000L, 0xe0e0e0000000L, 0xc0c0c0000000L, 0x3030300000000L, 0x7070700000000L, 0xe0e0e00000000L, 0x1c1c1c00000000L, 0x38383800000000L, 0x70707000000000L, 0xe0e0e000000000L, 0xc0c0c000000000L, 0x303030000000000L, 0x707070000000000L, 0xe0e0e0000000000L, 0x1c1c1c0000000000L, 0x3838380000000000L, 0x7070700000000000L, 0xe0e0e00000000000L, 0xc0c0c00000000000L, 0x303000000000000L, 0x707000000000000L, 0xe0e000000000000L, 0x1c1c000000000000L, 0x3838000000000000L, 0x7070000000000000L, 0xe0e0000000000000L, 0xc0c0000000000000L };
	
U64 knightMoves[] =
			{ 0x20400L, 0x50800L, 0xa1100L, 0x142200L, 0x284400L, 0x508800L, 0xa01000L, 0x402000L, 0x2040004L, 0x5080008L, 0xa110011L, 0x14220022L, 0x28440044L, 0x50880088L, 0xa0100010L, 0x40200020L, 0x204000402L, 0x508000805L, 0xa1100110aL, 0x1422002214L, 0x2844004428L, 0x5088008850L, 0xa0100010a0L, 0x4020002040L, 0x20400040200L, 0x50800080500L, 0xa1100110a00L, 0x142200221400L, 0x284400442800L, 0x508800885000L, 0xa0100010a000L, 0x402000204000L, 0x2040004020000L, 0x5080008050000L, 0xa1100110a0000L, 0x14220022140000L, 0x28440044280000L, 0x50880088500000L, 0xa0100010a00000L, 0x40200020400000L, 0x204000402000000L, 0x508000805000000L, 0xa1100110a000000L, 0x1422002214000000L, 0x2844004428000000L, 0x5088008850000000L, 0xa0100010a0000000L, 0x4020002040000000L, 0x400040200000000L, 0x800080500000000L, 0x1100110a00000000L, 0x2200221400000000L, 0x4400442800000000L, 0x8800885000000000L, 0x100010a000000000L, 0x2000204000000000L, 0x4020000000000L, 0x8050000000000L, 0x110a0000000000L, 0x22140000000000L, 0x44280000000000L, 0x88500000000000L, 0x10a00000000000L, 0x20400000000000L };
Move * getMoveList(){
	return moveList;	
}

PieceType getBlackPieceOnSquare(BoardInfo * boardInfo, U64 mask){
	//Assumes we know there is a blackpiece being captured
	PieceType type = EMPTY;
	if((boardInfo->BlackPawnBB &mask) != 0){
		type=PAWN;
	}else if((boardInfo->BlackKnightBB & mask) != 0){
		type = KNIGHT;
	}else if((boardInfo->BlackBishopBB& mask) != 0){
		type=BISHOP;
	}else if((boardInfo->BlackRookBB& mask) != 0){
		type=ROOK;
	}else  if((boardInfo->BlackQueenBB & mask) != 0){
		type=QUEEN;
	}
	return type;
}

PieceType getWhitePieceOnSquare(BoardInfo * boardInfo, U64 mask){
	//Assumes we know there is a Whitepiece being captured
	PieceType type = EMPTY;
	if((boardInfo->WhitePawnBB & mask) != 0){
		type=PAWN;
	}else if((boardInfo->WhiteKnightBB & mask) != 0){
		type = KNIGHT;
	}else if((boardInfo->WhiteBishopBB& mask) != 0){
		type=BISHOP;
	}else if((boardInfo->WhiteRookBB& mask) != 0){
		type=ROOK;
	}else  if((boardInfo->WhiteQueenBB & mask) != 0){
		type=QUEEN;
	}
	return type;
}
//PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
int mvvlvaValues[] = {1000000,2000000,3000000,4000000,5000000,10000000};
BoardInfo * info;//Set me before calling addMove. TODO
void addMove(Move move, ExtMove moves[],int index, bool isWhitePieceMoving, BoardInfo * info){
	PieceType capturedPiece;
	moves[index].move = move;
	//Not entirely convinced history should be considered at the move generation. This means the first level will never have any history, and generally the leftmost branch gets no benefit. Still, I couldn't think of a better time to do this in terms of efficiency. Sorting by a value rather than recalculating every time. Could try switching it.
	if(isWhitePieceMoving){
		moves[index].score = blackHeuristic[from_sq(move)][to_sq(move)];
		capturedPiece= getBlackPieceOnSquare(info, getSquare[to_sq(move)]);
	}else{
		moves[index].score = whiteHeuristic[from_sq(move)][to_sq(move)];
		capturedPiece = getWhitePieceOnSquare(info, getSquare[to_sq(move)]);
	}
	
	//mvvlva
	if(capturedPiece != EMPTY){
		moves[index].move = setCapture(move);
		moves[index].score = mvvlvaValues[capturedPiece] + PieceMoved(move);
	}
}

U64 pseudoLegalKnightMoveDestinations(U8 loc, U64 targets) {
	return knightMoves[loc] & targets;
}
	
U64 pseudoLegalKingMoveDestinations(U8 loc, U64 targets) {
	return kingMoves[loc] & targets;
}

U8 getAllBlackMoves(BoardInfo * boardInfo, ExtMove list[]){
	int idx = 0;
	idx+= getBlackBishopMoves(boardInfo, list, idx);
	idx+=getBlackKnightMoves(boardInfo, list, idx);
	idx+=getBlackRookMoves(boardInfo, list, idx);
	idx+=getBlackQueenMoves(boardInfo, list, idx);
	idx+=getBlackPawnMoves(boardInfo, list, idx);
	idx+= getBlackKingMoves(boardInfo, list, idx);

	return idx;
}

U8 getAllWhiteMoves(BoardInfo* boardInfo, ExtMove list[]){
	int idx = 0;
	idx+= getWhiteBishopMoves(boardInfo, list, idx);
	idx+=getWhiteKnightMoves(boardInfo, list, idx);
	idx+=getWhiteRookMoves(boardInfo, list, idx);
	idx+=getWhiteQueenMoves(boardInfo, list, idx);
	idx+=getWhitePawnMoves(boardInfo, list, idx);
	idx+= getWhiteKingMoves(boardInfo, list, idx);

	return idx;
}
U8 getAllPseudoLegalMoves(BoardInfo * boardInfo, ExtMove list[]){
	if(boardInfo->whiteToMove){
		return getAllWhiteMoves(boardInfo, list);
	}
	return getAllBlackMoves(boardInfo, list);
}
/*
U64 diag[] = {9241421688590303744,36099303471056128,141012904249856,550848566272,6480472064,1108177604608,283691315142656,72624976668147712,4620710844295151618,9241421688590368773,36099303487963146,141017232965652,1659000848424,283693466779728,72624976676520096,145249953336262720,2310355422147510788,4620710844311799048,9241421692918565393,36100411639206946,424704217196612,72625527495610504,145249955479592976,290499906664153120,1155177711057110024,2310355426409252880,4620711952330133792,9241705379636978241,108724279602332802,145390965166737412,290500455356698632,580999811184992272,577588851267340304,1155178802063085600,2310639079102947392,4693335752243822976,9386671504487645697,326598935265674242,581140276476643332,1161999073681608712,288793334762704928,577868148797087808,1227793891648880768,2455587783297826816,4911175566595588352,9822351133174399489,1197958188344280066,2323857683139004420,144117404414255168,360293502378066048,720587009051099136,1441174018118909952,2882348036221108224,5764696068147249408,11529391036782871041,4611756524879479810,567382630219904,1416240237150208,2833579985862656,5667164249915392,11334324221640704,22667548931719168,45053622886727936,18049651735527937};
U64 straights[] = {72340172838076926,144680345676153599,289360691352306943,578721382704613631,1157442765409227007,2314885530818453759,4629771061636907263,9259542123273814271,72340172838141697,144680345676218114,289360691352370948,578721382704676616,1157442765409287952,2314885530818510624,4629771061636955968,9259542123273846656,72340172854722817,144680345692733954,289360691368756228,578721382720800776,1157442765424889872,2314885530833068064,4629771061649424448,9259542123282137216,72340177099489537,144680349920788994,289360695563387908,578721386848585736,1157442769418981392,2314885534559772704,4629771064841355328,9259542125404520576,72341263759769857,144681432302879234,289361769389097988,578722443561535496,1157443791906410512,2314886488596160544,4629771881975660608,9259542668734660736,72619448791531777,144958522117980674,289636668770878468,578992962076674056,1157705548688265232,2315130721911447584,4629981068357812288,9259681761250541696,143834816922583297,215893514783949314,360010910506681348,648245701952145416,1224715284843073552,2377654450624929824,4683532782188642368,9295289445316067456,18374969058471772417,18375251637271921154,18375816794872218628,18376947110072813576,18379207740474003472,18383729001276383264,18392771522881142848,18410856566090662016};*/
U64 potentiallyPinned[] = {9313761861428380670,180779649147209727,289501704256556799,578721933553179903,1157442771889699071,2314886638996058367,4630054752952049919,9332167099941961983,4693051017133293315,9386102034266586887,325459994840334094,578862399937642268,1157444424410136376,2315169224285290352,4702396038313476064,9404792076610109376,2382695595002233605,4765391190004533002,9530782384287321621,614821794360007722,1157867469642086484,2387511058328678568,4775021017129017424,9550042029946290336,1227517888156599561,2455035776330041874,4910072647893521700,9820426766485563977,1266167049021314194,2460276499726510116,4920271520198053960,9840541936589512848,649930115027110161,1299860234365964834,2600000848492045380,5272058195805358472,10544115296394056209,2641485423861834786,5210912158452303940,10421541742416269448,361412783554236705,722826670915068482,1517430560419759236,3034580745374500872,6068881115283853584,12137481855085847073,5827939256702092354,11583539444389546116,287952221336838465,576187017162015362,1080597919557780484,2089419720071055368,4107063321064181776,8142350518772179232,16212923818971513409,13907045970195547266,18375536441101992321,18376667877509071362,18378650374858081284,18382614274322728968,18390542064695644176,18406396550208102432,18437825145767870784,18428906217826189953};


//Pretty incredible. This is 5x slower than pseudo legal.
U8 getAllLegalMoves(Board* b, ExtMove list[]){
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
	bool check = b->isOwnKingInCheck();
	int j = 0;
	U64 mask;
	if(b->currentBoard()->whiteToMove){
		mask = potentiallyPinned[trailingZeroCount(b->currentBoard()->WhiteKingBB)];
	}else{
		mask = potentiallyPinned[trailingZeroCount(b->currentBoard()->BlackKingBB)];
	}
	if(check){
		for (int i = 0; i < count; i++) {
			b->fastMakeMove(list[i].move);
			if(b->legal()){
				list[j++] = list[i];
			}
			b->undoMove();
		}
	}else{
		for (int i = 0; i < count; i++) {
			if(mask & getSquare[from_sq(list[i])] || PieceMoved(list[i]) == KING){
				b->fastMakeMove(list[i].move);
				if(b->legal()){
					list[j++] = list[i];
				}
				b->undoMove();
			}else{
				list[j++] = list[i];
			}
		}
	}

	return j;

}

U8 getWhiteKingMoves(BoardInfo* b, ExtMove moves[], int index) {
	U64 king = b->WhiteKingBB;
	
	int num_moves_generated = 0;
	
	U8 from_loc = trailingZeroCount(king);
	U64 movelocs = pseudoLegalKingMoveDestinations(from_loc, ~b->WhitePiecesBB);
	
	while (movelocs != 0L) {
		U64 to = lowestOneBit(movelocs);
		U8 to_loc = trailingZeroCount(to);
		Move move =createMove(from_loc, to_loc, KING);
		addMove(move,moves,index+num_moves_generated,true,b);
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
					addMove(createMove(from_loc, from_loc + 2, KING, KING, CASTLING),moves,index+num_moves_generated,true,b);
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
					addMove(createMove(from_loc, from_loc - 2, KING, KING, CASTLING),moves,index+num_moves_generated,true,b);
					num_moves_generated++;
				}
			}
		}
	}
	
	return num_moves_generated;
}
U8 getBlackKingMoves(BoardInfo* b, ExtMove moves[], int index) {
	U64 king = b->BlackKingBB;
	
	int num_moves_generated = 0;
	
	U8 from_loc = trailingZeroCount(king);
	U64 movelocs = pseudoLegalKingMoveDestinations(from_loc, ~b->BlackPiecesBB);
	
	while (movelocs != 0L) {
		U64 to = lowestOneBit(movelocs);
		U8 to_loc = trailingZeroCount(to);
		Move move = createMove(from_loc, to_loc, KING);
		addMove(move,moves,index+num_moves_generated,false,b);
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
					addMove(createMove(from_loc, from_loc + 2, KING, KING, CASTLING),moves,index+num_moves_generated,false,b);
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
					addMove(createMove(from_loc, from_loc - 2, KING,KING, CASTLING),moves,index+num_moves_generated,false,b);
					num_moves_generated++;
				}
			}
		}
	}
	
	return num_moves_generated;
}
U8 getWhiteKnightMoves(BoardInfo *b, ExtMove moves[], int index) {
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
			addMove(move,moves,index+num_moves_generated,true,b);
			//moves[index + num_moves_generated].move = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		knights &= ~from;
	}
	return num_moves_generated;
}


U8 getBlackKnightMoves(BoardInfo* b,ExtMove moves[], int index) {
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
			//moves[index + num_moves_generated].move = move;
			addMove(move,moves,index+num_moves_generated,false,b);
			num_moves_generated++;
			movelocs &= ~to;
		}
		knights &= ~from;
	}
	return num_moves_generated;
}
U8 getWhitePawnMoves(BoardInfo * b, ExtMove moves[], int listIdx){
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
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				//list[listIdx+moveGenCount].move = m;
				addMove(m,moves,listIdx+moveGenCount,true,b);

				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				//list[listIdx+moveGenCount].move = m;
				addMove(m,moves,listIdx+moveGenCount,true,b);

				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				//list[listIdx+moveGenCount].move = m;
				addMove(m,moves,listIdx+moveGenCount,true,b);
				moveGenCount++;
			}
			//If pawn not on H file, and there is a piece to capture upper right
			if(((fromBB & maskFile[FILE_H]) ==0) && (((fromBB <<9) & (b->BlackPiecesBB)) != 0)){
				U8 destLoc = trailingZeroCount(fromBB << 9);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				//list[listIdx+moveGenCount].move = m;
				addMove(m,moves,listIdx+moveGenCount,true,b);

				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				//list[listIdx+moveGenCount].move = m;
				addMove(m,moves,listIdx+moveGenCount,true,b);
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				//list[listIdx+moveGenCount].move = m;
				addMove(m,moves,listIdx+moveGenCount,true,b);
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
			}
			//If next square clear
			if(((fromBB << 8) & (b->AllPiecesBB)) == 0){
				U8 destLoc = trailingZeroCount(fromBB << 8);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;
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
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;			
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
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;			
				moveGenCount++;
			}
			Move m;
			//Normal move
			bool nextSquareClear = false;
			if(((fromBB << 8) & (b->AllPiecesBB)) == 0){
				nextSquareClear=true;
				U8 destLoc = trailingZeroCount(fromBB << 8);
				m = createMove(fromLoc, destLoc, PAWN);
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;			
				moveGenCount++;
			}
			//2 Squares
			if(((fromBB & maskRank[RANK_2]) != 0) && nextSquareClear && (((fromBB << 16) & (b->AllPiecesBB)) == 0)){
				U8 destLoc = trailingZeroCount(fromBB << 16);
				m = createMove(fromLoc, destLoc, PAWN);
				addMove(m,moves,listIdx+moveGenCount,true,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
			}
		}
		pawns = pawns &~fromBB;
	}
	return moveGenCount;
}


U8 getBlackPawnMoves(BoardInfo * b, ExtMove moves[], int listIdx){
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
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
			}
			//If pawn not on H file, and there is a piece to capture upper right
			if(((fromBB & maskFile[FILE_A]) ==0) && (((fromBB >>9) & (b->WhitePiecesBB)) != 0)){
				U8 destLoc = trailingZeroCount(fromBB >>9);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
			}
			//If next square clear
			if(((fromBB >>8) & (b->AllPiecesBB)) == 0){
				U8 destLoc = trailingZeroCount(fromBB >> 8);
				Move m = createMove(fromLoc, destLoc, PAWN, KNIGHT, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, QUEEN, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, BISHOP, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
				m = createMove(fromLoc, destLoc, PAWN, ROOK, PROMOTION);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
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
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;			
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
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;			
				moveGenCount++;
			}
			//Normal move
			bool nextSquareClear = false;
			if(((fromBB >> 8) & (b->AllPiecesBB)) == 0){
				nextSquareClear=true;
				Move m;
				U8 destLoc = trailingZeroCount(fromBB >> 8);
				m = createMove(fromLoc, destLoc, PAWN);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;			
				moveGenCount++;
			}
			//2 Squares
			if(((fromBB & maskRank[RANK_7]) != 0) && nextSquareClear && (((fromBB >> 16) & (b->AllPiecesBB)) == 0)){
				Move m;
				U8 destLoc = trailingZeroCount(fromBB >> 16);
				m = createMove(fromLoc, destLoc, PAWN);
				addMove(m,moves,listIdx+moveGenCount,false,b);
				//list[listIdx+moveGenCount].move = m;
				moveGenCount++;
			}
		}
		pawns = pawns &~fromBB;
	}
	return moveGenCount;
}

U8 getWhiteBishopMoves(BoardInfo * b, ExtMove moves[], int index) {
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
			addMove(move,moves,index+num_moves_generated,true,b);
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		bishops &= ~from;
	}
	return num_moves_generated;
}
U8 getBlackBishopMoves(BoardInfo* b,ExtMove moves[], int index) {
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
			addMove(move,moves,index+num_moves_generated,false,b);
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		bishops &= ~from;
	}
	return num_moves_generated;
}
U8 getWhiteRookMoves(BoardInfo* b, ExtMove moves[], int index) {
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
			addMove(move,moves,index+num_moves_generated,true,b);
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		rooks &= ~from;
	}
	return num_moves_generated;
}

U8 getBlackRookMoves(BoardInfo *b, ExtMove moves[], int index) {
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
			addMove(move,moves,index+num_moves_generated,false,b);
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		rooks &= ~from;
	}
	return num_moves_generated;
}


U8 getWhiteQueenMoves(BoardInfo* b, ExtMove moves[], int index) {
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
			addMove(move,moves,index+num_moves_generated,true,b);
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		queens &= ~from;
	}
	return num_moves_generated;
}

U8 getBlackQueenMoves(BoardInfo * b, ExtMove moves[], int index) {
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
			addMove(move,moves,index+num_moves_generated,false,b);
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		queens &= ~from;
	}
	return num_moves_generated;
}
//TODO needs to be vastly improved
U8 Movegen::getAllCaptures(Board * b, ExtMove moves[]){
	U8 movess = getAllLegalMoves(b,moves);
	U64 piecesToCap;
	if(b->currentBoard()->whiteToMove){
		piecesToCap = b->currentBoard()->BlackPiecesBB;
	}else{
		piecesToCap = b->currentBoard()->WhitePiecesBB;
	}
	
	int j = 0;
	for(int i = 0; i < movess; i++){
		 if((getSquare[to_sq(moves[i])] & piecesToCap) != 0 || type_of(moves[i]) == PROMOTION){
			if(b->staticExchange(moves[i].move) > 0){
				moves[j++] = moves[i];
			}
		 }
	}
	return j;
}

/*
U8 getBlackQueenCaptures(BoardInfo * b, ExtMove moves[], int index) {
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
			moves[index + num_moves_generated].move = move;
			num_moves_generated++;
			movelocs &= ~to;
		}
		
		queens &= ~from;
	}
	return num_moves_generated;
}*/