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
Player *create_player(char name[], Point *base_pos){
    Player *player  = malloc(sizeof(Player));
    strcpy(player->name, name);
    player->pos     = base_pos;

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
    free(player->pos);
    free(player);
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
Game *create_game(char playerA_name[], Point *playerA_pos, char playerB_name[], Point *playerB_pos){
    Game *game = malloc(sizeof(Game));
    game->playerA = create_player(playerA_name, playerA_pos);
    game->playerB = create_player(playerB_name, playerB_pos);
    game->current_turn = game->playerA;

    return game;
}

void delete_game(Game *game){
    delete_player(game->playerA);
    delete_player(game->playerB);
    
    free(game);
}

void player_pos_to_pixel_pos(Player *player, Point *pixel_pos){
    pixel_pos->x = GRID_OFFSET_SIZE + player->pos->x * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
    pixel_pos->y = GRID_OFFSET_SIZE + player->pos->y * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
}
void grid_pos_to_pixel_pos(Point *grid_pos, Point *pixel_pos){
    pixel_pos->x = GRID_OFFSET_SIZE + grid_pos->x * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
    pixel_pos->y = GRID_OFFSET_SIZE + grid_pos->y * GRID_CELL_SIZE + GRID_CELL_SIZE / 2;
}
void pixel_pos_to_player_pos(Point *pixel_pos, Point *player_pos){
    player_pos->x = (pixel_pos->x - GRID_OFFSET_SIZE) / GRID_CELL_SIZE;
    player_pos->y = (pixel_pos->y - GRID_OFFSET_SIZE) / GRID_CELL_SIZE;
}

void draw_game(App *app){
    // Game *game = app->game;
    // Point p;
    // SDL_Rect r = {0, 0, WIDTH, GRID_CELL_SIZE};

    // DEBUG DRAW TO SHOW WHICH PLAYER IS PLAYING
    draw_grid(app);
    draw_players(app);
}



void draw_players(App *app){
    // Point p;
    // Game *game = app->game;


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
            if ((x + y) % 2 == 0) {
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
