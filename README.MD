# Apollo

## About
Apollo is a UCI compatible chess engine created in c++.
Currently it can correctly generate moves, and run a simple material evaluation function on Alpha-Beta and Minimax trees.
The move generation is about 8x slower than stockfish.
For current tasks, see board.cpp, the TODO are listed there.

## Compile
  - Compile with cl: `cl uci.cpp bbmagic.cpp board.cpp movegen.cpp bitboard.cpp testing.cpp eval.cpp search.cpp /Ox /Feapollo`
  - Compile with g++: `g++ -std=c++11 uci.cpp bbmagic.cpp board.cpp movegen.cpp bitboard.cpp testing.cpp eval.cpp search.cpp -O3 -o apollo`
