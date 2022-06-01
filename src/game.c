#include "game.h"


// POIN
Point *create_point(int x, int y){
    Point *point  = malloc(sizeof(Point));
    point->x       = x;
    point->y       = y;

    return point;
}

void delete_point(Point *point){
    free(point);
}


// PLAYER
Player *create_player(char name[], bool even_player){
    Player *player  = malloc(sizeof(Player));
    strcpy(player->name, name);
    
    player->pawns = malloc(NUM_PAWNS * sizeof(Pawn *));
    // Create even player pawns
    int i = 0;
    if (even_player){
        for (int y = GRID_SIZE - 4; y < GRID_SIZE; y++){
            for (int x = 0; x < GRID_SIZE; x++){
                if ((x + y) % 2 != 0){
                    player->pawns[i] = create_pawn(player, x, y);
                    i++;
                }
            }
        }
    } else {
        for (int y = 0; y < 4; y++){
            for (int x = 0; x < GRID_SIZE; x++){
                if ((x + y) % 2 != 0){
                    player->pawns[i] = create_pawn(player, x, y);
                    i++;
                }
            }
        }
    }

    return player;
}
void update_next_pawn_position(Game *game, int *current_overflow_index, int i, int dx, int dy){
    

}
void update_available_pawn_positions(Game *game){
    
}

void delete_player(Player *player){
    free(player);
}

Pawn *create_pawn(Player *owner, int x, int y){
    Pawn *pawn      = malloc(sizeof(Pawn));
    pawn->owner     = owner;
    pawn->pos.x     = x;
    pawn->pos.y     = y;
    pawn->is_dame   = false; 
    return pawn;
}

bool player_can_play(App *app, Pawn *pawn){
    return pawn->owner == app->game->current_turn;
}

void drag_pawn(App *app, Pawn *pawn){
    app->game->dragged_pawn = pawn;
    app->game->grid[pawn->pos.x][pawn->pos.y] = NULL; // Pawn isnt on the grid currently, it's above it
}
bool can_place_pawn_here(App *app, int gx, int gy){
    // For now, we just check if the tile is empty
    if (app->game->dragged_pawn->pos.x == gx && app->game->dragged_pawn->pos.y == gy) return false;
    if (app->game->grid[gx][gy] != NULL) return false;

    return true;
}
void reset_pawn_position(App *app){
    int x = app->game->dragged_pawn->pos.x;
    int y = app->game->dragged_pawn->pos.y;
    place_pawn(app, x, y);
}
void place_pawn(App *app, int gx, int gy){
    app->game->grid[gx][gy] = app->game->dragged_pawn;
    app->game->dragged_pawn->pos.x = gx;
    app->game->dragged_pawn->pos.y = gy;
    app->game->dragged_pawn = NULL;

}
void change_turn(App *app){
    app->game->current_turn = (app->game->playerB == app->game->current_turn) ? app->game->playerA : app->game->playerB; 
}

void delete_pawn(Pawn *pawn){
    free(pawn);
}

int min(int a, int b){
    if (a > b) return b;
    return a;
}
int max(int a, int b){
    if (a > b) return a;
    return b;
}

void swap_points(Point *a, Point *b){
    Point tmp = {a->x, a->y};
    *a = *b;
    *b = tmp;
}


// GAME
void update_grid(Game *game, Player *player){
    for (int i = 0; i < NUM_PAWNS; i++){
        int x = player->pawns[i]->pos.x;
        int y = player->pawns[i]->pos.y;

        game->grid[x][y] = player->pawns[i];
    }
}
Game *create_game(char playerA_name[], char playerB_name[]){
    Game *game = malloc(sizeof(Game));    
    game->playerA = create_player(playerA_name, false);
    game->playerB = create_player(playerB_name, true);
    game->current_turn = game->playerA;

    game->grid = malloc(GRID_SIZE * sizeof(Pawn **));
    for (int i = 0; i < GRID_SIZE; i++){
        game->grid[i] = malloc(GRID_SIZE * sizeof(Pawn *));
        for (int j = 0; j < GRID_SIZE; j++){
            game->grid[i][j] = NULL;
        }
    }

    update_grid(game, game->playerA);
    update_grid(game, game->playerB);

    return game;
}

void delete_game(Game *game){
    delete_player(game->playerA);
    delete_player(game->playerB);
    
    free(game);
}

int distance_to_closest_pawn(App *app, int *gx, int *gy){
    *gx = (app->mouse_pos->x - GRID_OFFSET_SIZE) / GRID_CELL_SIZE;
    *gy = (app->mouse_pos->y - GRID_OFFSET_SIZE) / GRID_CELL_SIZE;

    int px = GRID_OFFSET_SIZE + *gx * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
    int py = GRID_OFFSET_SIZE + *gy * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;

    int dx = app->mouse_pos->x - px;
    int dy = app->mouse_pos->y - py;
    return dx * dx + dy * dy;
}


void draw_game(App *app){
    // Game *game = app->game;
    // Point p;
    // SDL_Rect r = {0, 0, WIDTH, GRID_CELL_SIZE};

    // DEBUG DRAW TO SHOW WHICH PLAYER IS PLAYING
    draw_grid(app);
    draw_players(app);
}

void draw_pawn(App *app, Pawn *pawn){
    if (pawn->owner == app->game->playerA)   SDL_SetRenderDrawColor(app->renderer, PLAYERA_COLOR);
    else                                SDL_SetRenderDrawColor(app->renderer, PLAYERB_COLOR);
    
    int x, y;
    if (pawn != app->game->dragged_pawn) {
        x = GRID_OFFSET_SIZE + pawn->pos.x * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
        y = GRID_OFFSET_SIZE + pawn->pos.y * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
    } else {
        x = app->mouse_pos->x; y = app->mouse_pos->y;
    }

    SDL_RenderFillCircle(app->renderer, x, y, PLAYER_RADIUS);

    // DRAW DAMES
    if (pawn->owner == app->game->playerA)   SDL_SetRenderDrawColor(app->renderer, PLAYERB_COLOR);
    else                                SDL_SetRenderDrawColor(app->renderer, PLAYERA_COLOR);
    

    if (pawn->is_dame){
        if (pawn != app->game->dragged_pawn){
            x = GRID_OFFSET_SIZE + pawn->pos.x * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
            y = GRID_OFFSET_SIZE + pawn->pos.y * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
        } else {
            x = app->mouse_pos->x; y = app->mouse_pos->y;
        }

        // draw vertical line
        SDL_RenderDrawLine(app->renderer, x, y - DAME_LINE_LEN, x, y + DAME_LINE_LEN);
        // draw horizontal line
        SDL_RenderDrawLine(app->renderer, x - DAME_LINE_LEN, y, x + DAME_LINE_LEN, y);
    }
}
void draw_player(App *app, Player *player){
    if (player == app->game->playerA)   SDL_SetRenderDrawColor(app->renderer, PLAYERA_COLOR);
    else                                SDL_SetRenderDrawColor(app->renderer, PLAYERB_COLOR);
    
    for (int i = 0; i < NUM_PAWNS; i++){
        Pawn *pawn = player->pawns[i];
        if (pawn != app->game->dragged_pawn)
            draw_pawn(app, pawn);
    }
}
void draw_players(App *app){
    // Point p;
    // Game *game = app->game;
    draw_player(app, app->game->playerA);
    draw_player(app, app->game->playerB);

    if (app->game->dragged_pawn != NULL) draw_pawn(app, app->game->dragged_pawn);

}



void draw_grid(App *app){
    // draw game background
    SDL_Rect r = {0,0,WIDTH,HEIGHT};
    SDL_SetRenderDrawColor(app->renderer, EVEN_COLOR);
    SDL_RenderFillRect(app->renderer, &r);

    // Draw the current player as a rectangle at the top
    r.x = WIDTH / 2 - PLAYER_INDICATOR_WIDTH / 2;
    r.y = 0;
    r.w = PLAYER_INDICATOR_WIDTH;
    r.h = GRID_OFFSET_SIZE;
    if (app->game->current_turn == app->game->playerA)      SDL_SetRenderDrawColor(app->renderer, PLAYERA_COLOR);
    else                                                    SDL_SetRenderDrawColor(app->renderer, PLAYERB_COLOR);
    SDL_RenderFillRect(app->renderer, &r);
    
    r.x = GRID_OFFSET_SIZE;
    r.y = GRID_OFFSET_SIZE;
    r.w = GRID_CELL_SIZE;
    r.h = GRID_CELL_SIZE;
    // Draw the actual grid
    for (int y = 0; y < GRID_SIZE; y++){
        for (int x = 0; x < GRID_SIZE; x++){
            if ((x + y) % 2 != 0) {
                SDL_SetRenderDrawColor(app->renderer, ODD_COLOR);
                SDL_RenderFillRect(app->renderer, &r);
            }                  
            r.x += GRID_CELL_SIZE;
        }
        r.y += GRID_CELL_SIZE;
        r.x = GRID_OFFSET_SIZE;
    }
    
    // draw outer grid
    SDL_SetRenderDrawColor(app->renderer, OUTER_GRID_COLOR);
    // Left grid wall
    r.x = GRID_OFFSET_SIZE - OUTER_GRID_WIDTH;
    r.y = GRID_OFFSET_SIZE - OUTER_GRID_WIDTH;
    r.w = OUTER_GRID_WIDTH;
    r.h = GRID_SIZE * GRID_CELL_SIZE + 2 * OUTER_GRID_WIDTH;
    SDL_RenderFillRect(app->renderer, &r);
    // right grid wall
    r.x = GRID_OFFSET_SIZE + GRID_SIZE * GRID_CELL_SIZE;
    SDL_RenderFillRect(app->renderer, &r);
    // top grid wall
    r.x = GRID_OFFSET_SIZE;
    r.y = GRID_OFFSET_SIZE - OUTER_GRID_WIDTH;
    r.w = GRID_SIZE * GRID_CELL_SIZE;
    r.h = OUTER_GRID_WIDTH;
    SDL_RenderFillRect(app->renderer, &r);
    // down grid wall
    r.y = GRID_OFFSET_SIZE + GRID_SIZE * GRID_CELL_SIZE;
    SDL_RenderFillRect(app->renderer, &r);

   
    
}
