/**
 * @file game.h
 * @author Nathan Schiffmacher (nathan.schiffmacher@student.isae-supaero.fr)
 * @brief 
 * @date 24-05-2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */ 

#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "options.h"
#include "SDL_render.h"

#define GRID_SIZE 10
#define NUM_PAWNS 20
#define MAX_NAME_SIZE 150
#define MAX_AVAILABLE_POSITIONS 10

// DRAW OPTIONS
#define DRAW_DEBUG_ON_START false

#define GRID_OFFSET 1/20 // percentage of the SIZE
#define GRID_EFFECTIVE_SIZE (SIZE - 2 * SIZE * GRID_OFFSET) //(SIZE * (1 - 2 * GRID_OFFSET))
#define GRID_OFFSET_SIZE SIZE * GRID_OFFSET

#define EVEN_COLOR 157,126,104,255
#define ODD_COLOR 133,94,66,255

#define GRID_CELL_SIZE (GRID_EFFECTIVE_SIZE/GRID_SIZE) // INCLUDE WALL

#define OUTER_GRID_COLOR 0,0,0,255
#define OUTER_GRID_WIDTH_PRC 1 / 30
#define OUTER_GRID_WIDTH (GRID_CELL_SIZE * OUTER_GRID_WIDTH_PRC)


#define PLAYER_RADIUS_PRC 6/10 // prc of the grid size
#define PLAYER_RADIUS (GRID_CELL_SIZE * PLAYER_RADIUS_PRC / 2) 

#define PLAYER_HOVER_PRC 12 / 10
#define PLAYER_HOVER_DIST_SQ (PLAYER_RADIUS * PLAYER_HOVER_PRC) * (PLAYER_RADIUS * PLAYER_HOVER_PRC)

#define DAME_LINE_LEN_PRC 4 / 10
#define DAME_LINE_LEN (PLAYER_RADIUS * DAME_LINE_LEN_PRC)

#define PLAYER_INDICATOR_WIDTH WIDTH * 2 / 10

// Player A
#define PLAYERA_NAME "Odd player"
#define PLAYERA_COLOR 0,0,0,255

// Player B
#define PLAYERB_NAME "Even player"
#define PLAYERB_COLOR 255,255,255,255

#define AVAILABLE_POSITION_COLOR 180,180,180,255

typedef struct {
    int x, y;
} Point;

typedef struct Pawn Pawn;
typedef struct Player Player;
struct Pawn {
    Player *owner;

    Point pos;
    bool is_dame;
};
struct Player {
    char name[MAX_NAME_SIZE];

    Pawn **pawns;

};

typedef struct {
    Player *playerA;
    Player *playerB;
    Player *current_turn;

    Pawn *dragged_pawn;
    Pawn ***grid;
} Game;

#include "app.h"

// POINT
Point *create_point(int x, int y);
void delete_point(Point *point);

// PLAYER
Player *create_player(char name[], bool even_player);
bool wall_block_path(Game *game, Point a, Point b); // returns true if a wall blocks the path
void update_next_pawn_position(Game *game, int *current_overflow_index, int i, int dx, int dy); // helper function for available_pawn_positions
void update_available_pawn_positions(Game *game); // fils available positions with the possible positions
void delete_player(Player *player);

//PAWN
Pawn *create_pawn(Player *owner, int x, int y);
void delete_pawn(Pawn *pawn);

bool player_can_play(App *app, Pawn *pawn);
void drag_pawn(App *app, Pawn *pawn);
bool can_place_pawn_here(App *app, int gx, int gy);
void place_pawn(App *app, int gx, int gy);
void reset_pawn_position(App *app);
void change_turn(App *app);


// GAMEX
Game *create_game(char playerA_name[], char playerB_name[]);
void delete_game(Game *game);


/**
 * @brief change the current player playing
 */
void draw_game(App *app);
void draw_pawn(App *app, Pawn *pawn);
void draw_players(App *app);
void draw_grid(App *app);

int distance_to_closest_pawn(App *app, int *gx, int *gy); // distance squared actually


#endif