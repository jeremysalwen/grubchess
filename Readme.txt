GrubChess

Grubchess infuses the power of grubs into a chess engine.

You can play against the engine, or have it play against itself (Or even play yourself!).

The implemented engine is:

 - Fixed depth minimax w/ alpha-beta pruning.
 - Quiescence search with the stand-pat heuristic. (This is important for rating).
 - Transposition table using a from-scratch linear probing hash table (This is important for speed).
 - Evaluation is a weighted sum of three terms: material, activity (total possible moves), and points for pawn advancement.


It can search to depth 6 (actually deeper due to quiescence search) in a reasonable amount of time.

It doesn't support UCI or any interoperability so I'm not sure of its rating.

It was mostly written on a plane flight, and the UI is editing the code and recompiling :)  Most significantly, at the bottom of grubchess.c, you can switch to computer vs computer or player vs player mode by changing the argument to play_chess().

Apache 2.0 Licensed.
