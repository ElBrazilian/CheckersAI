/**
 * @file main.h
 * @author Nathan Schiffmacher (nathan.schiffmacher@student.isae-supaero.fr)
 * @brief 
 * @date 24-05-2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game.h"
#include "app.h"
#include "options.h"


/**
 * @brief Initializes the app object and the SDL
 * 
 * @param app 
 */
void initialize_app(App *app);
/**
 * @brief Destroys the app and the SDL
 * 
 * @param app 
 */
void destroy_app(App *app);

void update(App *app);
void handle_events(App *app);
void draw(App *app);
#endif