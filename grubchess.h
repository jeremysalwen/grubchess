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
#ifndef GRUBCHESS_H
#define GRUBCHESS_H
enum Piece {
  EMPTY=0,
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
  NUM_PIECES
};
extern char PIECE_SYMBOLS[];

enum Color {
  WHITE,
  BLACK,
  NUM_COLORS
};

extern char* COLOR_NAMES[];

enum Color enemy_color(enum Color color);

#define BOARD_WIDTH 8

typedef struct Position {
  int rank;
  int file;
} Position;

typedef struct Square {
  enum Piece piece;
  enum Color color;
} Square;

typedef struct Board {
  enum Color move;
  Square squares[BOARD_WIDTH * BOARD_WIDTH];

  int en_passant;
  // Second index corresponds to A and H file, respectively.
  bool can_castle[NUM_COLORS][2];

} Board;

typedef struct Move {
  Position from;
  Position to;
} Move;
Square get_square(const Board* board, Position position);
void set_square(Board* board, Position position, Square value);
void apply_valid_move(Board* board, Position from, Position to);
bool occupied(const Board* board, Position position);

bool board_equal(const Board* b1, const Board* b2);
char square_to_char(Square square);
void print_board(const Board* board);
void print_move(const Board* board, Position from, Position to);
typedef void ValidMovesCallback(const Board*, Position, Position, void*);
void valid_moves_from(const Board* board, Position position, ValidMovesCallback callback, void* callback_data);
void valid_moves(const Board* board, ValidMovesCallback callback, void* callback_data);
void valid_moves_sorted(const Board* board, int (compar) (const void*, const void*, void*), ValidMovesCallback callback, void* callback_data);


typedef struct ThreatsBoard {
  int squares[BOARD_WIDTH*BOARD_WIDTH];
} ThreatsBoard;


int* get_threat_board(ThreatsBoard* board, Position pos);

void sum_threats_callback(const Board* board, Position from, Position to, void* data);

#endif
