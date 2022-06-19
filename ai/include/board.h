#ifndef H_BOARD
#define H_BOARD

#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <unistd.h> 


#define MAX_JUMPS 25
#define MAX_POSSIBLE_MOVES 1200
#define WIN 15999

/////////////////// START //////////////////////////
#define EMPTY    0
#define BLACK    1
#define WHITE    2
#define COLOR_MASK 3
#define KING     4  // NB: les anglosaxons sont machistes
#define OUTSIDE  8
#define EATEN    16

#define sizeX 12
#define sizeY 12

#define INF 16500

struct direction { int dx, dy; };
extern const struct direction directions[4];
extern int board[sizeX][sizeY];
extern int IA_color;

int color(int piece);
int opponent_color(int piece);
int are_opponents(int a, int b);
int are_same_color(int a, int b);
int is_king(int piece);
int is_eaten(int piece);
int promote_king(int piece);
int forward_direction(int color);


int std_position(int x, int y);
int coordY(int pos);
int coordX(int pos);
int get_piece(int pos);
void set_piece(int pos, int val);

void init_board(void);
void print_board(int tab[sizeX][sizeY]);
int eval_board(int tab[sizeX][sizeY]);
void load_board();



#endif