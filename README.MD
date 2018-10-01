# Apollo

## About
Apollo is a UCI compatible chess engine created in c++. Apollo is built as a traditional chess engine, meaning CPU heavy with alpha-beta search and a hardcoded evaluation function at the tree leaves.

## Usage
`./apollo` Runs apollo

`uci` Tells apollo to use the uci protocol and list the available options.

`setoption name Hash value 64` Tells apollo to allocate 64MB for the transpostion table.

`ucinewgame` Tells apollo to set up a blank board.

`go depth 10` Starts a search for the best move 10 ply deep.

`go movetime 1000` Starts a search that runs until 1000 milliseconds pass.

# Techniques Used
#### Search Algorithms
- Alpha-beta Principal Variation Search
- Iterative Deepening
- Aspiration Windows
- Time management algorithm 
- Quiescence search
#### Move Generation
- Magic bitboards
- Pseudo-legal move generation
- Pinned pieces optimizations
#### Move Ordering
- History heuristic
- Killer move heuristic
- Countermove Bonus
- Most Valuable Victim - Least Valuable Aggressor (MVVLVA)
- Transposition Table
#### Extensions and Reductions
- Late Move Reductions (LMR)
#### Pruning
- Futility Pruning
- Reverse-Futility Pruning
- Null Move Pruning
- Delta Pruning
#### Evaluation
- Passed Pawns
- Isolated Pawns
- Doubled Pawns
- Open Files
- Semi-Open Files
- Centralization of Pieces
- Pawn shield for king safety
- Material Evaluation
- Separate Endgame Evaluation
#### Board Representation
- Bitboards
- Zobrist Hashing
#### Uci Extensions
- Perft (`perft [depth]`) : Counts how many moves are possible from a given position.
- Debug (`d`)  : Displays  debug information such as the current evaluation, move suggested by the transposition table, and more.


## Compilation
  - Compile with cl: `cl uci.cpp bbmagic.cpp board.cpp movegen.cpp bitboard.cpp testing.cpp eval.cpp search.cpp zobrist.cpp transpo.cpp /Ox /Feapollo /EHsc`
  - Compile with g++: `g++ -std=c++11 uci.cpp bbmagic.cpp board.cpp movegen.cpp bitboard.cpp testing.cpp eval.cpp search.cpp zobrist.cpp transpo.cpp -O3 -o apollo`

  
## Credit
Much inspiration has been taken from the chessprogramming wiki, the bot Godot, and some from Carballo and the CPW Engine.