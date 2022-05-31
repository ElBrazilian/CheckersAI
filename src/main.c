#include "main.h"


void initialize_app(App *app){
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Create the window
    app->window = SDL_CreateWindow(app->windowTitle, BASE_WINDOW_X_POS, BASE_WINDOW_Y_POS, WIDTH, HEIGHT, app->windowFlags);
    if (!app->window){
        printf("Failed to open the window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Create the renderer
    app->renderer = SDL_CreateRenderer(app->window, -1, app->rendererFlags);
    if (!app->renderer){
        printf("Failed to open the renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Create the timekeeper object
    app->keeper = create_timekeeper(TARGET_FPS);

    // Load the debug font and init TTF
    TTF_Init();
    app->debug_font = TTF_OpenFont("fonts/SpaceMono-Regular.ttf", DEBUG_FONT_SIZE);
    app->debug_draw = DRAW_DEBUG_ON_START;

    // Create game
    app->game = create_game(
        PLAYERA_NAME, create_point(PLAYERA_POSX, PLAYERA_POSY),
        PLAYERB_NAME, create_point(PLAYERB_POSX, PLAYERB_POSY)
    );
    app->mouse_pos      = create_point(0,0);
    app->mouse_state    = 0;

    app->continuer              = true;

}

void destroy_app(App *app){
    destroy_timekeeper(app->keeper);
    // delete_game(app->game);

    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    TTF_Quit();
    SDL_Quit();
    free(app);
}

int main(int argc, char *argv[]){
    // Creating and initializing the app
    App *app = malloc(sizeof(App));
    app->rendererFlags  = SDL_RENDERER_ACCELERATED;
    app->windowFlags    = 0;
    strcpy(app->windowTitle, BASE_WINDOW_TITLE);

    initialize_app(app);

    while(app->continuer){
        // update mouse pos
        app->mouse_state = SDL_GetMouseState(&(app->mouse_pos->x), &(app->mouse_pos->y));

        // HANDLING EVENTS
        handle_events(app);
        update_timekeeper_handle(app->keeper);

        // UPDATE 
        update(app);
        update_timekeeper_update(app->keeper);

        // DRAW
        SDL_SetRenderDrawColor(app->renderer, 0,0,0,255);
        SDL_RenderClear(app->renderer);
        draw(app);

        if (app->debug_draw) timekeeper_draw_debug_info(app->keeper, app->renderer, app->debug_font);
        SDL_RenderPresent(app->renderer);
        update_timekeeper_draw(app->keeper);

        // DELAY TO STAY AT TARGET FPS
        timekeeper_limit(app->keeper);
        timekeeper_computeFPS(app->keeper);

        //printf("FPS: %f\n", app->keeper->currentFPS);
        // printf("frame: %d\n", app->keeper->frame_length_raw);
    }


    // Destroy everything
    destroy_app(app);
    return 0;
}


void update(App *app){
}

void handle_events(App *app){
    SDL_Event e;
    while(SDL_PollEvent(&e)){
        if (e.type == SDL_QUIT){
            app->continuer = false;
        } else if (e.type == SDL_KEYDOWN){
            if (e.key.keysym.sym == SDLK_d){
                app->debug_draw = !app->debug_draw;
            } else if (e.key.keysym.sym == SDLK_ESCAPE){
                app->continuer = false;
            } 
        } else if (e.type == SDL_MOUSEBUTTONDOWN){
            if (e.button.button == SDL_BUTTON_LEFT){
                
                
            } else if (e.button.button == SDL_BUTTON_RIGHT){
                
                
            }
        } else if (e.type == SDL_MOUSEBUTTONUP){
            if (e.button.button == SDL_BUTTON_LEFT){
                
                
            }
        }
    }
}

void draw(App *app){
    draw_game(app);
}