/**
 * @file timekeeper.h
 * @author Nathan Schiffmacher (nathan.schiffmacher@student.isae-supaero.fr)
 * @brief 
 * @date 24-05-2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define NUM_AVERAGES 500
#define NUM_AVERAGES_FL 500.0

typedef struct {
    double targetFPS;
    Uint32 target_frame_length; // in MS

    Uint32 last_handle_events_ticks;
    Uint32 last_update_ticks;
    Uint32 last_draw_ticks;
    Uint32 last_frame_ticks;
    Uint32 last_delay;
    Uint32 frame_length_raw; // without the delay
    Uint32 frame_length;

    // FPS computing average
    Uint32 last_frame_lengths_array[NUM_AVERAGES];
    Uint32 last_frame_lengths_sum;
    double currentFPS;

    // frame_length_raw computing average
    double frame_length_average;

    Uint32 handle_events_time;
    // handle_events_time computing average
    Uint32 handle_events_array[NUM_AVERAGES];
    Uint32 handle_events_sum;
    int average_index;
    double handle_events_average;
    
    Uint32 update_time;
    // update_time computing average
    Uint32 update_array[NUM_AVERAGES];
    Uint32 update_sum;
    double update_average;

    Uint32 draw_time;
    // draw_time computing average
    Uint32 draw_array[NUM_AVERAGES];
    Uint32 draw_sum;
    double draw_average;
} TimeKeeper;

/**
 * @brief Creates and initializes a timekeeper object
 * 
 * @param targetFPS 
 * @return TimeKeeper* 
 */
TimeKeeper *create_timekeeper(double targetFPS);
/**
 * @brief Set the target FPS accordingly
 * 
 * @param keeper 
 * @param FPS 
 */
void set_timekeeper_framerate(TimeKeeper *keeper, double FPS);
/**
 * @brief needs to be called after handle_events(); to register the time it takes
 * 
 * @param keeper 
 */
void update_timekeeper_handle(TimeKeeper *keeper);
/**
 * @brief needs to be called after update(); to register the time it takes
 * 
 * @param keeper 
 */
void update_timekeeper_update(TimeKeeper *keeper);
/**
 * @brief needs to be called after draw(); to register the time it takes
 * 
 * @param keeper 
 */
void update_timekeeper_draw(TimeKeeper *keeper);
/**
 * @brief waits the necessary amount of time to stay at the target framerate
 * 
 * @param keeper 
 */
void timekeeper_limit(TimeKeeper *keeper);

/**
 * @brief Computes the length of the frame + the framerate
 * 
 * @param keeper 
 */
void timekeeper_computeFPS(TimeKeeper *keeper);
/**
 * @brief Draw informations on FPS and the time taken for the different operations
 * 
 * @param keeper 
 * @param renderer 
 */
void timekeeper_draw_debug_info(TimeKeeper *keeper, SDL_Renderer *renderer, TTF_Font *font);
/**
 * @brief Destroys the timekeeper object
 * 
 * @param keeper 
 */
void destroy_timekeeper(TimeKeeper *keeper);



#endif


