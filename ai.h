#ifndef AI_H
#define AI_H

#include "grubchess.h"

int minimax_score(const Board* board, int max_depth, Move* best_move);

#endif
