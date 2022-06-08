#ifndef H_INTERACE
#define H_INTERACE

#include <stdbool.h>

#include "moves.h"

bool read_move(Move *move);
void send_move(Move move);

#endif