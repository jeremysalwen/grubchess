#include <stdbool.h>
#include <limits.h>
#include <stdio.h>

#include "grubchess.h"
#include "ai.h"

#define SCORE_FRAC 10
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

typedef struct ThreatsBoard {
  int squares[BOARD_WIDTH*BOARD_WIDTH];
} ThreatsBoard;

int* get_threat_board(ThreatsBoard* board, Position pos) {
  return &board->squares[pos.rank*BOARD_WIDTH + pos.file];
}

void sum_threats_callback(const Board* board, Position from, Position to, void* data) {
  ThreatsBoard* threats = (ThreatsBoard*)data;
  Square square = get_square(board, to);
  if(square.piece != EMPTY) {
    int* threatlevel = get_threat_board(threats, to);
    *threatlevel += 1;
  }
}
int score_threats(const Board* board) {
  Board fixed_move = *board;
  ThreatsBoard white_threats;
  ThreatsBoard black_threats;
  for(int rank=0; rank<BOARD_WIDTH; rank++) {
    for(int file=0; file<BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      *get_threat_board(&white_threats, pos) = 0;
      *get_threat_board(&black_threats, pos) = 0;
    }
  }
  
  for(int rank=0; rank<BOARD_WIDTH; rank++) {
    for(int file=0; file<BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      Square square = get_square(&fixed_move, pos);
      ThreatsBoard* threats = square.color==WHITE?&white_threats : &black_threats;

      fixed_move.move = WHITE;
      Square white_square = square;
      white_square.color = WHITE;
      set_square(&fixed_move, pos, white_square);
      valid_moves_from(&fixed_move, pos, sum_threats_callback, threats);
      
      fixed_move.move = BLACK;
      Square black_square = square;
      black_square.color = BLACK;
      set_square(&fixed_move, pos, black_square);
      valid_moves_from(&fixed_move, pos, sum_threats_callback, threats);

      set_square(&fixed_move, pos, square);
    }
  }
  int total_score = 0;
  for(int rank=0; rank<BOARD_WIDTH; rank++) {
    for(int file=0; file<BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      Square square = get_square(&fixed_move, pos);
      if(square.piece != EMPTY) {
        int valence = square.color == WHITE ? 1:-1;
        int white_threat = *get_threat_board(&white_threats, pos);
        int black_threat = *get_threat_board(&black_threats, pos);
        int threat = white_threat - black_threat;
        //printf("total threat! %c %d %d %d %d\n", square_to_char(square), white_threat, black_threat, threat, threat*valence);
        if(threat * valence < 0) {
          total_score -= valence * CLASSIC_PIECE_VALUE[square.piece] * SCORE_FRAC;
        }
      }
    }
  }
  
  // Remove 80% of score for undefended pieces.
  total_score = total_score * 8 / 10;
  //printf("total score %d\n", total_score);
  return total_score;
}

int score(const Board* board) {
  return score_material(board) + score_threats(board);
}

bool score_is_checkmate(int score) {
  return score < -CHECKMATE_SCORE_THRESHOLD || score > CHECKMATE_SCORE_THRESHOLD;
}

typedef struct SearchCallbackData {
  int max_depth;
  int alphabeta[NUM_COLORS];
  Move best_move;
} SearchCallbackData;



void search_callback(const Board* board, Position from, Position to, void* d) {
  SearchCallbackData* data = (SearchCallbackData*)d;
  if(data->alphabeta[WHITE] >= data->alphabeta[BLACK]) {
    //printf("Pruned %d %d\n", data->alphabeta[WHITE], data->alphabeta[BLACK]);
    //return;
  }

  int valence = board->move == WHITE? 1: -1;

  Board new_board = *board;
  //print_move(board, from, to);
  apply_valid_move(&new_board, from, to);
  Move ignored_move; // We don't care what the best child move is
  int new_score = minimax_score(&new_board, data->max_depth, data->alphabeta[WHITE], data->alphabeta[BLACK], &ignored_move);
 
  
  //printf("valence %d best_score %d %d %d move %d\n", valence, data->alphabeta[WHITE], data->alphabeta[BLACK], new_score, board->move);

  Move move = {from, to};
  if((new_score - data->alphabeta[board->move]) * valence > 0) {
    data->alphabeta[board->move] = new_score;
    data->best_move = move;
  }
  //printf("valence %d best_score %d %d %d gap %d\n", valence, data->alphabeta[WHITE], data->alphabeta[BLACK], new_score, (new_score - data->alphabeta[board->move]));
}


int minimax_score(const Board* board, int max_depth, int alpha, int beta, Move* best_move) {
  Move nullmove = {{0,0},{0,0}};

  //printf("Searching, with depth %d\n", max_depth);
  //print_board(board);
  int my_score = score(board); // Default score is our heuristic function.
  if(max_depth ==0 || my_score > CHECKMATE_SCORE_THRESHOLD || my_score < -CHECKMATE_SCORE_THRESHOLD) {
    //printf("Leaf node score %d!\n", my_score);
    *best_move = nullmove;
    return my_score;
  }
    
  SearchCallbackData data;
  data.max_depth = max_depth-1;
  data.alphabeta[WHITE] = alpha;
  data.alphabeta[BLACK] = beta;
  valid_moves(board, search_callback, &data);
  if(data.alphabeta[WHITE] == alpha && data.alphabeta[BLACK] == beta) {
    data.best_move = nullmove;
    return 0;
  }

  *best_move = data.best_move;
  //printf("Tree node score %d\n", data.best_score);
  return data.alphabeta[board->move];
}

