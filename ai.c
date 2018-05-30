#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "grubchess.h"
#include "ai.h"
#include "hashtable.h"

#define SCORE_FRAC 100
const int CLASSIC_PIECE_VALUE[] = {0,1,3,3,5,9,1000};
const int CHECKMATE_SCORE_THRESHOLD = 500 * SCORE_FRAC;
int score_material(const Board* board) {
  int total = 0;
  for(int rank=0; rank<BOARD_WIDTH; rank++) {
    for(int file=0; file<BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      Square square = get_square(board, pos);
      int valence = square.color == WHITE? 1 : -1;
      total += valence * CLASSIC_PIECE_VALUE[square.piece]*SCORE_FRAC;
    }
  }
  return total;
}

int score_see(const Board* board) {
  Board fixed_move = *board;
  ThreatsBoard threats[NUM_COLORS];
  for(int rank=0; rank<BOARD_WIDTH; rank++) {
    for(int file=0; file<BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      for(int color=0; color<NUM_COLORS; color++) {
        *get_threat_board(&threats[color], pos) = 0;
      }
    }
  }
  
  for(int rank=0; rank<BOARD_WIDTH; rank++) {
    for(int file=0; file<BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      Square square = get_square(&fixed_move, pos);
      ThreatsBoard* t = &threats[square.color];

      for(int color=0; color<NUM_COLORS; color++) {
        fixed_move.move = color;
        set_square(&fixed_move, pos, (Square) {square.piece, color});
        valid_moves_from(&fixed_move, pos, sum_threats_callback, t);
      }
      set_square(&fixed_move, pos, square);
    }
  }
  
  int total_score = 0;
  for(int rank=0; rank<BOARD_WIDTH; rank++) {
    for(int file=0; file<BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      Square square = get_square(board, pos);
      if(square.piece != EMPTY) {
        const int valence = square.color == WHITE ? 1:-1;
        const int white_threat = *get_threat_board(&threats[WHITE], pos);
        const int black_threat = *get_threat_board(&threats[BLACK], pos);
        const int threat = white_threat - black_threat;
        //printf("total threat! %c %d %d %d %d\n", square_to_char(square), white_threat, black_threat, threat, threat*valence);
        const int piece_value = valence * CLASSIC_PIECE_VALUE[square.piece];
        int multiplier = SCORE_FRAC;
        //Undefended pieces are worth 80% of their value on your turn,
        //20% of their value on enemy turn.
        if(threat * valence < 0) {
          if(square.color == board->move) {
            multiplier = SCORE_FRAC * 8 / 10;
          } else {
            multiplier = SCORE_FRAC * 2 / 10;
          }
        }
        total_score += piece_value * multiplier;
      }
    }
  }

  return total_score;
}



void count_moves_callback(const Board* board, Position from, Position to, void* data) {
  int* moves = (int*)data;
  (*moves)++;
}

int score_activity(const Board* board) {
  Board fixed_move = *board;

  int possible_moves[NUM_COLORS] = {0};
  for(int color=0; color<NUM_COLORS; color++) {
    fixed_move.move = color;
    valid_moves(&fixed_move, count_moves_callback, &possible_moves[color]);
  }
  
  int score = possible_moves[WHITE] - possible_moves[BLACK];
  return score*SCORE_FRAC / 100;
}

int score_pawn_advancement(const Board* board) {
  int total_score = 0;
  for(int rank=0; rank<BOARD_WIDTH; rank++) {
    for(int file=0; file<BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      Square square = get_square(board, pos);
      int valence = square.color == WHITE? 1:-1;
      int target_rank = (square.color == WHITE) * 7;
      if(square.piece == PAWN) {
        int distance = abs(pos.rank - target_rank);
        int score = 3-distance;
        if(score<0) score = 0;
        total_score += score * valence;
      }
    }
  }
  return total_score * SCORE_FRAC / 3;
}

int score(const Board* board) {
  return score_see(board) + score_activity(board) + score_pawn_advancement(board);
}

bool score_is_checkmate(int score) {
  return score < -CHECKMATE_SCORE_THRESHOLD || score > CHECKMATE_SCORE_THRESHOLD;
}

typedef struct SearchCallbackData {
  HashTable* table;
  int max_depth;
  int alphabeta[NUM_COLORS];
  Move* best_move;
} SearchCallbackData;



void search_callback(const Board* board, Position from, Position to, void* d) {
  SearchCallbackData* data = (SearchCallbackData*)d;
  if(data->alphabeta[WHITE] >= data->alphabeta[BLACK]) {
    //printf("Pruned %d %d\n", data->alphabeta[WHITE], data->alphabeta[BLACK]);
    return;
  }

  int valence = board->move == WHITE? 1: -1;

  Board new_board = *board;
  //print_move(board, from, to);
  apply_valid_move(&new_board, from, to);
  Move child_moves[data->max_depth];
  int new_score = minimax_score(data->table, &new_board, data->max_depth, data->alphabeta[WHITE], data->alphabeta[BLACK], child_moves);
  
  //printf("valence %d best_score %d %d %d move %d\n", valence, data->alphabeta[WHITE], data->alphabeta[BLACK], new_score, board->move);

  Move move = {from, to};
  if((new_score - data->alphabeta[board->move]) * valence > 0) {
    data->alphabeta[board->move] = new_score;
    data->best_move[0] = move;
    for(int i=0; i<data->max_depth; i++) {
      data->best_move[i+1] = child_moves[i];
    }
  }
  //printf("valence %d best_score %d %d %d gap %d\n", valence, data->alphabeta[WHITE], data->alphabeta[BLACK], new_score, (new_score - data->alphabeta[board->move]));
}


int move_order_comparator(const void* m1, const void* m2, void* b) {
  const Move* move1 = (const Move*) m1;
  const Move* move2 = (const Move*) m2;
  const Board* board = (const Board*) b;
  Square square1 = get_square(board, move1->to);
  Square square2 = get_square(board, move2->to);

  int capture_diff = CLASSIC_PIECE_VALUE[square2.piece] - CLASSIC_PIECE_VALUE[square1.piece];

  return capture_diff;
}

void update_table(HashTable* table, const Board* board, int score, int depth) {
  if(table != NULL) {
    insert_hashtable(table, board, score, depth);
  }
}

int minimax_score(HashTable* table, const Board* board, int max_depth, int alpha, int beta, Move* best_move) {
  Move nullmove = {{0,0},{0,0}};

  if(table != NULL) {
    Entry* entry = lookup_hashtable(table, board);
    // Make sure the depth of the cached entry is at least as much as our current search.
    if(entry != NULL && max_depth <= entry->depth) {
      return entry->score;
    }
  }
  
  //printf("Searching, with depth %d\n", max_depth);
  //print_board(board);
  int my_score = score(board); // Default score is our heuristic function.
  if(max_depth ==0 || my_score > CHECKMATE_SCORE_THRESHOLD || my_score < -CHECKMATE_SCORE_THRESHOLD) {
    //printf("Leaf node score %d!\n", my_score);
    // TODO maybe cache leaf nodes?
    return my_score;
  }
    
  SearchCallbackData data;
  data.table = table;
  data.max_depth = max_depth-1;
  data.alphabeta[WHITE] = alpha;
  data.alphabeta[BLACK] = beta;
  data.best_move = best_move;
  valid_moves_sorted(board, move_order_comparator, search_callback, &data);
  if(data.alphabeta[WHITE] == alpha && data.alphabeta[BLACK] == beta) {
    int score = data.alphabeta[board->move];
    update_table(table, board, score, max_depth);
    return score;
  }

  //printf("Tree node score %d\n", data.best_score);
  int score = data.alphabeta[board->move];
  update_table(table, board, score, max_depth);
  return score;
}
