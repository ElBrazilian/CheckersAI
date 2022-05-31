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
                if ((x + y) % 2 == 0){
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
bool player_can_move_pawn(Player *player, Game *game){
    return true;
}

void update_next_pawn_position(Game *game, int *current_overflow_index, int i, int dx, int dy){
    

}
void update_available_pawn_positions(Game *game){
    
}
bool player_can_place_pawn(Game *game, Point p){
     
    return true;
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
Game *create_game(char playerA_name[], char playerB_name[]){
    Game *game = malloc(sizeof(Game));
    game->playerA = create_player(playerA_name, false);
    game->playerB = create_player(playerB_name, true);
    game->current_turn = game->playerA;

    return game;
}

void delete_game(Game *game){
    delete_player(game->playerA);
    delete_player(game->playerB);
    
    free(game);
}


void draw_game(App *app){
    // Game *game = app->game;
    // Point p;
    // SDL_Rect r = {0, 0, WIDTH, GRID_CELL_SIZE};

    // DEBUG DRAW TO SHOW WHICH PLAYER IS PLAYING
    draw_grid(app);
    draw_players(app);
}


void draw_player(App *app, Player *player){
    if (player == app->game->playerA)   SDL_SetRenderDrawColor(app->renderer, PLAYERA_COLOR);
    else                                SDL_SetRenderDrawColor(app->renderer, PLAYERB_COLOR);
    
    for (int i = 0; i < NUM_PAWNS; i++){
        Pawn *pawn = player->pawns[i];

        int x = GRID_OFFSET_SIZE + pawn->pos.x * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
        int y = GRID_OFFSET_SIZE + pawn->pos.y * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;

        SDL_RenderFillCircle(app->renderer, x, y, PLAYER_RADIUS);
    }

    // DRAW DAMES
    
}
void draw_players(App *app){
    // Point p;
    // Game *game = app->game;
    draw_player(app, app->game->playerA);
    draw_player(app, app->game->playerB);


}



void draw_grid(App *app){
    // draw game background

    SDL_Rect r = {0,0,WIDTH,HEIGHT};
    SDL_SetRenderDrawColor(app->renderer, EVEN_COLOR);
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
