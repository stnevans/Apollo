# Apollo

## About
Apollo is a UCI compatible chess engine created in c++. It uses the traditional approach of chess engines, meaning it uses alpha-beta pruning with negamax. Various types of further pruning are added in an attempt to keep the branching factor low.
For more information on the exact methods, see the source files.

## Compile
  - Compile with cl: `cl uci.cpp bbmagic.cpp board.cpp movegen.cpp bitboard.cpp testing.cpp eval.cpp search.cpp zobrist.cpp transpo.cpp /Ox /Feapollo /EHsc`
  - Compile with g++: `g++ -std=c++11 uci.cpp bbmagic.cpp board.cpp movegen.cpp bitboard.cpp testing.cpp eval.cpp search.cpp zobrist.cpp transpo.cpp -O3 -o apollo`

## Credit
Much inspiration has been taken from the chessprogramming wiki, the bot Godot, and some from Carballo and the CPW Engine.