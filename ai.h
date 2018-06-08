/*
Copyright 2018 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef AI_H
#define AI_H

#include "grubchess.h"
#include "hashtable.h"

#define WORST_POSSIBLE_SCORE -1000000
#define BEST_POSSIBLE_SCORE 1000000
int minimax_score(HashTable* table, const Board* board, int max_depth, int alpha, int beta, Move* best_move);

#endif
