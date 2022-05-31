#include "timekeeper.h"

TimeKeeper *create_timekeeper(double targetFPS){
    TimeKeeper *keeper                  = malloc(sizeof(TimeKeeper));
    keeper->draw_time                   = 0;
    keeper->handle_events_time          = 0;

    Uint32 ticks = SDL_GetTicks();
    keeper->last_draw_ticks             = ticks;
    keeper->last_handle_events_ticks    = ticks;
    keeper->last_update_ticks           = ticks;
    keeper->last_frame_ticks            = ticks;
    
    keeper->last_delay                  = 0;
    keeper->frame_length                = 0;
    keeper->frame_length_raw            = 0;
    keeper->currentFPS                  = 0;
    keeper->update_time                 = 0;
    keeper->draw_time                   = 0;
    keeper->handle_events_time          = 0;
    
    set_timekeeper_framerate(keeper, targetFPS);

    // FPS AVERAGE
    for (int i = 0; i < NUM_AVERAGES; i++){
        keeper->last_frame_lengths_array[i] = keeper->target_frame_length;
    }
    keeper->average_index           = 0;
    keeper->last_frame_lengths_sum          = keeper->target_frame_length * NUM_AVERAGES;

    // HANDLE EVENTS AVERAGE
    keeper->average_index = 0;
    keeper->handle_events_average = 0.;
    for (int i = 0; i < NUM_AVERAGES; i++){
        keeper->handle_events_array[i] = keeper->handle_events_average;
    }
    keeper->handle_events_sum = NUM_AVERAGES * keeper->handle_events_average;
    
    // UPDATE AVERAGE
    keeper->update_average = 0.;
    for (int i = 0; i < NUM_AVERAGES; i++){
        keeper->update_array[i] = keeper->update_average;
    }
    keeper->update_sum = NUM_AVERAGES * keeper->update_average;

    // DRAW AVERAGE
    keeper->draw_average = 7.;
    for (int i = 0; i < NUM_AVERAGES; i++){
        keeper->draw_array[i] = keeper->draw_average;
    }
    keeper->draw_sum = NUM_AVERAGES * keeper->draw_average;

    keeper->frame_length_average = 7.;

    return keeper;
}
void set_timekeeper_framerate(TimeKeeper *keeper, double FPS){
    keeper->targetFPS = FPS;
    keeper->target_frame_length = (Uint32)((1/FPS) * 1000);
}
void update_timekeeper_handle(TimeKeeper *keeper){
    keeper->last_handle_events_ticks = SDL_GetTicks();
    keeper->handle_events_time = keeper->last_handle_events_ticks - keeper->last_frame_ticks;

    // Handle events average 
    keeper->handle_events_sum = keeper->handle_events_sum - keeper->handle_events_array[keeper->average_index] + keeper->handle_events_time;
    keeper->handle_events_array[keeper->average_index] = keeper->handle_events_time;

    keeper->handle_events_average = keeper->handle_events_sum / NUM_AVERAGES_FL;
}
void update_timekeeper_update(TimeKeeper *keeper){
    keeper->last_update_ticks = SDL_GetTicks();
    keeper->update_time = keeper->last_update_ticks - keeper->last_handle_events_ticks;

    // update average 
    keeper->update_sum = keeper->update_sum - keeper->update_array[keeper->average_index] + keeper->update_time;
    keeper->update_array[keeper->average_index] = keeper->update_time;

    keeper->update_average = keeper->update_sum / NUM_AVERAGES_FL;
}
void update_timekeeper_draw(TimeKeeper *keeper){
    keeper->last_draw_ticks = SDL_GetTicks();
    keeper->draw_time = keeper->last_draw_ticks - keeper->last_update_ticks;

    // draw average 
    keeper->draw_sum = keeper->draw_sum - keeper->draw_array[keeper->average_index] + keeper->draw_time;
    keeper->draw_array[keeper->average_index] = keeper->draw_time;

    keeper->draw_average = keeper->draw_sum / NUM_AVERAGES_FL;

    keeper->frame_length_raw = keeper->handle_events_time + keeper->update_time + keeper->draw_time;
    keeper->frame_length_average = keeper->handle_events_average + keeper->update_average + keeper->draw_average;

}

void timekeeper_limit(TimeKeeper *keeper){
    keeper->last_delay = keeper->target_frame_length - keeper->frame_length_raw;
    if ((int)keeper->last_delay < 0) {
        keeper->last_delay = 0;
    }
    else {
        SDL_Delay(keeper->last_delay);
    }
}
void timekeeper_computeFPS(TimeKeeper *keeper){
    Uint32 now = SDL_GetTicks();
    keeper->frame_length = now - keeper->last_frame_ticks;

    
    keeper->last_frame_lengths_sum = keeper->last_frame_lengths_sum - keeper->last_frame_lengths_array[keeper->average_index] + keeper->frame_length;
    keeper->last_frame_lengths_array[keeper->average_index] = keeper->frame_length;
    keeper->average_index = (keeper->average_index + 1) % NUM_AVERAGES;

    keeper->currentFPS = 1000. * NUM_AVERAGES / ((int) keeper->last_frame_lengths_sum);
    keeper->last_frame_ticks = now;
}

void _timekeeper_draw_debug_step(int *y, SDL_Renderer *renderer, TTF_Font *font, char text[], SDL_Color fg, SDL_Color bg){
    SDL_Surface *surface = TTF_RenderText_Shaded(font, text, fg, bg);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    int w = 0;
    int h = 0;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect dest_rect = {0,*y,w,h};
	
    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
    //SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    *y += h;
}
void _timekeeper_custom_format(char out_text[], char start_text[], double ms){
    if ((int)(ms) < 10) sprintf(out_text, " %s %.2f ", start_text, ms);
    else sprintf(out_text, " %s%.2f ", start_text, ms);
}
void timekeeper_draw_debug_info(TimeKeeper *keeper, SDL_Renderer *renderer, TTF_Font *font){
    // Colors
    SDL_Color white = {255,255,255};
    SDL_Color green = {99, 152, 53};
    SDL_Color autre = {192, 178, 34};
    SDL_Color blue  = {72, 148, 151};
    SDL_Color red   = {187, 40, 30};
    SDL_Color purple = {127, 102, 133};
    
    // Strings
    char frame_text[20];
    char update_text[20];
    char draw_text[20];
    char handle_text[20];
    char fps_text[20];
    _timekeeper_custom_format(frame_text,   "FRAME  : "  ,keeper->frame_length_average);
    _timekeeper_custom_format(update_text,  "UPDATE : "  ,keeper->update_average);
    _timekeeper_custom_format(draw_text,    "RENDER : "  ,keeper->draw_average);
    _timekeeper_custom_format(handle_text,  "HANDLE : "  ,keeper->handle_events_average);
    _timekeeper_custom_format(fps_text,     "FPS    : "  ,keeper->currentFPS);

    // Drawing
    int y = 0;
    _timekeeper_draw_debug_step(&y, renderer, font, fps_text, white, autre);
    _timekeeper_draw_debug_step(&y, renderer, font, frame_text, white, green);
    _timekeeper_draw_debug_step(&y, renderer, font, handle_text, white, blue);
    _timekeeper_draw_debug_step(&y, renderer, font, update_text, white, red);
    _timekeeper_draw_debug_step(&y, renderer, font, draw_text, white, purple);
}
void destroy_timekeeper(TimeKeeper *keeper){
    free(keeper);
}