#ifndef AI_H
#define AI_H

#include "grubchess.h"
#include "hashtable.h"

#define WORST_POSSIBLE_SCORE -100000
#define BEST_POSSIBLE_SCORE 100000
int minimax_score(HashTable* table, const Board* board, int max_depth, int alpha, int beta, Move* best_move);

#endif
