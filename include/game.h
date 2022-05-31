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

// Player A
#define PLAYERA_NAME "Player A"
#define PLAYERA_POSX 0
#define PLAYERA_POSY 4
#define PLAYERA_COLOR 255,0,0,255

// Player B
#define PLAYERB_NAME "Player B"
#define PLAYERB_POSX 8
#define PLAYERB_POSY 4
#define PLAYERB_COLOR 0,255,0,255

#define AVAILABLE_POSITION_COLOR 180,180,180,255

typedef struct {
    int x, y;
} Point;

typedef struct {
    char name[MAX_NAME_SIZE];
    Point *pos;

} Player;

typedef struct {
    Player *playerA;
    Player *playerB;
    Player *current_turn;


} Game;

#include "app.h"

// POINT
Point *create_point(int x, int y);
void delete_point(Point *point);

// PLAYER
Player *create_player(char name[], Point *base_pos);
bool player_can_move_pawn(Player *player, Game *game); // TODO 
bool wall_block_path(Game *game, Point a, Point b); // returns true if a wall blocks the path
void update_next_pawn_position(Game *game, int *current_overflow_index, int i, int dx, int dy); // helper function for available_pawn_positions
void update_available_pawn_positions(Game *game); // fils available positions with the possible positions
bool player_can_place_pawn(Game *game, Point p); // TODO: Return true if the player current dragging a pawn can place here 
void delete_player(Player *player);


// GAMEX
Game *create_game(char playerA_name[], Point *playerA_pos, char playerB_name[], Point *playerB_pos);
void delete_game(Game *game);

/**
 * @brief transforms a grid position (3,4) for instance to a pixel position on the screen
 */
void player_pos_to_pixel_pos(Player *player, Point *pixel_pos);
void grid_pos_to_pixel_pos(Point *grid_pos, Point *pixel_pos);
void pixel_pos_to_player_pos(Point *pixel_pos, Point *player_pos);


/**
 * @brief change the current player playing
 */
void draw_game(App *app);
void draw_players(App *app);
void draw_grid(App *app);

#endif