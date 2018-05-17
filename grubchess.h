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
} Board;

typedef struct Move {
  Position from;
  Position to;
} Move;
Square get_square(const Board* board, Position position);
void set_square(Board* board, Position position, Square value);
void apply_valid_move(Board* board, Position from, Position to);
bool occupied(const Board* board, Position position);

void print_board(const Board* board);
void print_move(const Board* board, Position from, Position to);
typedef void ValidMovesCallback(const Board*, Position, Position, void*);
void valid_moves_from(const Board* board, Position position, ValidMovesCallback callback, void* callback_data);
void valid_moves(const Board* board, ValidMovesCallback callback, void* callback_data);

#endif
