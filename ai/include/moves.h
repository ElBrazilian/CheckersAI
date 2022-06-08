#ifndef H_MOVES
#define H_MOVES

#include "board.h"

typedef struct Move {
    int from;
    int to;

    int num_jumps;
    int jumps_pos[MAX_JUMPS];
    int positions[MAX_JUMPS + 1];
} Move;

typedef struct PossibleMoves {
    Move **all_possible_moves;
    int current_move;
    int current_max_jumps;
} PossibleMoves;


void init_move(Move *move);
void generate_eaten_pos(Move *move);
void reset_possible_moves(int new_max_jumps, PossibleMoves *possible_moves);
void add_move_with_jumps(int jump_num, int positions[], int eaten[], PossibleMoves *possible_moves);
void add_move_without_jumps(int from, int to, PossibleMoves *possible_moves);
void rec_compute_jumps(int color, int tab[sizeX][sizeY], int jump_num, int positions[], int eaten[], PossibleMoves *possible_moves);
void compute_moves_from_pos(int color, int tab[sizeX][sizeY], int start_pos, PossibleMoves *possible_moves);
void compute_possible_moves(int current_turn_color, int tab[sizeX][sizeY], PossibleMoves *possible_moves);
void init_possible_moves(PossibleMoves *possible_moves);
void destroy_possible_moves(PossibleMoves *possible_moves);


void update_board(int tab[sizeX][sizeY], Move move);

#endif 