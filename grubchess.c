#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "grubchess.h"
#include "ai.h"

char PIECE_SYMBOLS[] = {' ', 'p', 'n', 'b', 'r', 'q', 'k'};
char* COLOR_NAMES[] = {"WHITE", "BLACK"};

enum Color enemy_color(enum Color color) {
  if(color == BLACK) {
    return WHITE;
  } else {
    return BLACK;
  }
};


Square get_square(const Board* board, Position position) {
  return board->squares[position.rank * BOARD_WIDTH + position.file];
}

void set_square(Board* board, Position position, Square value) {
  board->squares[position.rank * BOARD_WIDTH + position.file] = value;
}

bool position_valid(Position position) {
  if(position.rank<0 || position.rank > 7) return false;
  if(position.file<0 || position.file > 7) return false;
  return true;
}
bool occupied(const Board* board, Position position) {
  return get_square(board, position).piece != EMPTY;
}

bool occupies(const Board* board, Position position, enum Color color) {
  Square square = get_square(board, position);
  return square.color == color && square.piece != EMPTY;
}

bool empty(const Board* board, Position position) {
  return !occupied(board, position);
}

int advance_rank(enum Color color) {
  switch(color) {
    case WHITE:
      return 1;
    case BLACK:
      return -1;
    default:
      printf("Error Bad shit\n");
      exit(1);
  }
}


void fill_rank(Board* board, int rank, Square square) {
  for(int i=0; i<BOARD_WIDTH; i++) {
    Position pos = {rank, i};
    set_square(board, pos, square);
  }
}

void reset_board(Board* board) {
  board->move = WHITE; // White to move.
  
  enum Piece piecerow[BOARD_WIDTH] = {ROOK, KNIGHT, BISHOP, KING, QUEEN, BISHOP, KNIGHT, ROOK };

  // 8th Rank
  for(int i=0; i<BOARD_WIDTH; i++) {
    Position pos = {7, i};
    Square sqr = {piecerow[i], BLACK};
    set_square(board, pos, sqr);
  };

  // 7th Rank
  const Square black_pawn = {PAWN, BLACK};
  fill_rank(board, 6, black_pawn);

  const Square empty = {EMPTY, BLACK}; // Black is the void

  // 6th through 3rd rank empty
  for(int rank = 2; rank < 6; rank++) {
    fill_rank(board, rank, empty);
  }
  
  // 2nd Rank
  const Square white_pawn = {PAWN, WHITE};
  fill_rank(board, 1, white_pawn);


  // 1st Rank
  for(int i=0; i<BOARD_WIDTH; i++) {
    Position pos = {0, i};
    Square sqr = {piecerow[i], WHITE};
    set_square(board, pos, sqr);
  };

}

bool square_valid(Square square) {
  if(square.piece<0 || square.piece > NUM_PIECES) {
    return false;
  }
  if(square.color <0 || square.color > NUM_COLORS) {
    return false;
  }
  return true;
}

bool board_valid(const Board* board) {
  for(int i=0; i<BOARD_WIDTH; i++) {
    for(int j=0; j<BOARD_WIDTH; j++) {
      Position pos = {i, j};
      if(!square_valid(get_square(board, pos))) {
        return false;
      }
    }
  }
  return true;
}

bool position_equal(Position p1, Position p2) {
  return p1.rank == p2.rank && p1.file == p2.file;
}

bool move_equal(Move m1, Move m2) {
  return position_equal(m1.from, m2.from) && position_equal(m1.to, m2.to);
}

char square_to_char(Square square) {
  char symbol = PIECE_SYMBOLS[square.piece];
  if(square.color == WHITE) { // Caps for White
    symbol += 'A' - 'a';
  }
  return symbol;           
}

void print_position(Position position) { // chess notation.
  printf("%c%d", 'a' + position.file, position.rank + 1);
}

void print_board(const Board* board) {
  printf("----------\n");
  for(int rank = BOARD_WIDTH - 1; rank >=0; rank--) {
    printf("|");
    for(int file = 0; file < BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      printf("%c", square_to_char(get_square(board, pos)));
    }
    printf("|\n");
  }
  printf("----------\n");
  printf("%s to play.\n", COLOR_NAMES[board->move]);
}


void apply_valid_move(Board* board, Position from, Position to) {
  Square square =  get_square(board, from);
  if(square.piece == PAWN) {
    if(to.rank == 7 || to.rank == 0) {
      square.piece = QUEEN;
    }
  }
  set_square(board, to, square);
  
  Square empty = {EMPTY, BLACK};
  set_square(board, from, empty);

  board->move = enemy_color(board->move);
}

bool winning_move(const Board* board, Position to) {
  return get_square(board, to).piece==KING;
}



bool try_move_peaceful(const Board* board, Position from, Position to, ValidMovesCallback callback, void* callback_data) {
  if(!position_valid(to)) {
    return false;
  }

  if(empty(board, to)) {
    callback(board, from, to, callback_data);
    return true;
  }
  return false;
}

bool try_move_capture(const Board* board, Position from, Position to, ValidMovesCallback callback, void* callback_data) {
  if(!position_valid(to)) {
    return false;
  }

  if(occupies(board, to, enemy_color(board->move))) {
    callback(board, from, to, callback_data);
    return true;
  }
  return false;
}

bool try_move_any(const Board* board, Position from, Position to, ValidMovesCallback callback, void* callback_data) {
  if(!position_valid(to)) {
    return false;
  }

  if(!occupies(board, to, board->move)) {
    callback(board, from, to, callback_data);
    return true;
  }
  return false;
}
      
void valid_moves_from(const Board* board, Position position, ValidMovesCallback callback, void* callback_data) {
  Square square = get_square(board, position);
  if(square.color != board->move) { // You can only move your own pieces!
    return;
  }
  
  switch(square.piece) {
    case EMPTY:
      return;
    case PAWN:
      {
        Position front = {position.rank + advance_rank(square.color), position.file};
        try_move_peaceful(board, position, front, callback, callback_data);

        if((board->move == WHITE && position.rank == 1)
           || (board->move == BLACK && position.rank==6)) {
          Position two_ahead = {position.rank + advance_rank(square.color)*2, position.file};
          try_move_peaceful(board, position, two_ahead, callback, callback_data);
        }

        Position left = {position.rank + advance_rank(square.color), position.file-1};
        try_move_capture(board, position, left, callback, callback_data);

        Position right = {position.rank + advance_rank(square.color), position.file+1};
        try_move_capture(board, position, right, callback, callback_data);
      }
      break;
    case KNIGHT:
      for(int i=0; i<2; i++) {
        for(int j=0; j<2; j++) {
          int pm_2 = i*4 -2; // {-2, 2}
          int pm_1 = j*2 -1; // {-1, 1}
          Position tall_pos = {position.rank + pm_2, position.file + pm_1};
          try_move_any(board,position, tall_pos, callback, callback_data);

          Position wide_pos = {position.rank + pm_1, position.file + pm_2};
          try_move_any(board, position, wide_pos, callback, callback_data);
        }
      }
      break;
    case BISHOP:
      for(int i = 0; i<2; i++) {
        for(int j = 0; j<2; j++) {
          int rank_step = i *2-1; // {-1, 1}
          int file_step = j *2-1; // {-1, 1}
          for(int i=1; i<7; i++) {
            Position pos = {position.rank + rank_step * i, position.file + file_step *i};
            try_move_capture(board, position, pos, callback, callback_data);
            if(!try_move_peaceful(board, position, pos, callback, callback_data)) {
              break; // We can't jump over pieces.
            }
          }
        }
      }
      break;
    case ROOK:
      for(int i = 0; i<2; i++) {
        int inc = i *2-1; // {-1, 1}
        for(int i=1; i<7; i++) {
          Position pos = {position.rank + i * inc, position.file};
          try_move_capture(board, position, pos, callback, callback_data);
            if(!try_move_peaceful(board, position, pos, callback, callback_data)) {
              break; // We can't jump over pieces.
            }
        }
        for(int i=1; i<7; i++) {
          Position pos = {position.rank, position.file +i * inc};
          try_move_capture(board, position, pos, callback, callback_data);
          if(!try_move_peaceful(board, position, pos, callback, callback_data)) {
            break; // We can't jump over pieces.
          }
        }
      }
      break;
    case QUEEN:
      for(int fwd=-1; fwd<2; fwd++) {
        for(int side=-1; side<2; side++) {
          if(fwd == 0 && side==0) {
            continue;
          }
          for(int i=1; i<7; i++) {
            Position pos = {position.rank + i * fwd, position.file + i*side};
            try_move_capture(board, position, pos, callback, callback_data);
            if(!try_move_peaceful(board, position, pos, callback, callback_data)) {
              break; // We can't jump over pieces.
            }
          }
        }
      }
      break;
    case KING:
      for(int fwd=-1; fwd<2; fwd++) {
        for(int side=-1; side<2; side++) {
          if(fwd == 0 && side==0) {
            continue;
          }
          Position pos = {position.rank + fwd, position.file * side};
          try_move_any(board, position, pos, callback, callback_data);
        }
      }
      break;
    default:
      printf("Unable to handle piece type %d\n", square.piece);
      break;
  }
}

void valid_moves(const Board* board, ValidMovesCallback callback, void* callback_data) {
  for(int rank = 0; rank<BOARD_WIDTH; rank++) {
    for(int file = 0; file<BOARD_WIDTH; file++) {
      Position pos = {rank, file};
      valid_moves_from(board, pos, callback, callback_data);
    }
  }
}

void print_move(const Board* board, Position from, Position to) {
  printf("%c ", square_to_char(get_square(board, from)));
  print_position(from);
  printf("->");
  print_position(to);
  printf("\n");
}

void print_move_t(const Board* board, Move move) {
  print_move(board, move.from, move.to);
}

void print_move_callback(const Board* board, Position from, Position to, void* data) {
  print_move(board, from, to);
}

void save_move_callback(const Board* board, Position from, Position to, void* data) {
  Position** dat = (Position**)data;
  *(*dat)++ = from;
  *(*dat)++ = to;
  
}

typedef struct FindMoveData {
  bool found;
  Move move;
} FindMoveData;

void move_found_callback(const Board* board, Position from, Position to, void* data) {
  FindMoveData* target = (FindMoveData*) data;
  Move suggested = {from, to};
  if(move_equal(suggested, target->move)) {
    target->found=true;
  }
}

bool move_valid(const Board* board, Move move) {
  if(!position_valid(move.from) || !position_valid(move.to)) {
    return false;
  }
  
  FindMoveData data;
  data.found = false;
  data.move = move;
  valid_moves_from(board,  move.from, move_found_callback, &data);
  return data.found;
}


Move random_move(const Board* board) {
    Position move_buffer[1000];
    Position* moves_ptr = move_buffer;
    valid_moves(board, save_move_callback, &moves_ptr);
    int nmoves = (moves_ptr - move_buffer)/2;
    int chosen = rand() % nmoves;
    printf("Found %d moves, picking move %d\n", nmoves, chosen);
    Position from = move_buffer[chosen*2];
    Position to = move_buffer[chosen*2 +1];
    Move move = {from, to};
    return move;
}

Move minimax_engine(const Board* board) {
  Move best_move;
  int best_score = minimax_score(board, 2, &best_move);
  printf("Found move with score %d\n", best_score);
  return best_move;
}

Move human_engine(const Board* board) {
  Move move;
  do {
    printf("Enter move: ");
    
    int from_rank, to_rank;
    char from_file, to_file;
    scanf("%c%d %c%d", &from_file, &from_rank, &to_file, &to_rank);
    
    move.from.rank = from_rank - 1;
    move.from.file = from_file - 'a';
    move.to.rank = to_rank - 1;
    move.to.file = to_file - 'a';
  } while(!move_valid(board, move));
  return move; 
}

Move human_vs_computer_engine(const Board* board) {
  if(board->move == WHITE) {
    return human_engine(board);
  } else {
    return minimax_engine(board);
  }
}

typedef Move EngineCallback(const Board* board);

void play_chess(Board* board, EngineCallback* engine) {
  int game_length=0;
  while(true) {
    printf("%d Moves played so far\n", game_length);
    print_board(board);

    Move move =  engine(board);
    print_move_t(board, move);
    if(winning_move(board, move.to)) {
      printf("Found winning move!");
      break;
    }
    apply_valid_move(board, move.from, move.to);
    game_length++;
  }
}

int main(int argc, char** argv) {
  srand(time(NULL));
  printf("Welcome to GrubChess! Time to get grubby!\n");

  Board board;
  reset_board(&board);
  play_chess(&board, minimax_engine);
  print_board(&board); 
}