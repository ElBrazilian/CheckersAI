#ifndef H_HASH
#define H_HASH

#define _XOPEN_SOURCE

#include "board.h"
#include "moves.h"
#include "interface.h"

#include <stdlib.h>

extern long int uuid;

typedef uint64_t signature;
struct
    __attribute__((packed))
    _hash_type 
{
    uint64_t sig;
    int16_t v_inf,v_sup;
    Move move;
    uint8_t base;
    uint8_t dist;
};

typedef struct _hash_type hash_type;

#define NB_BITS 26
#define HASH_SIZE (1<<NB_BITS)
#define HASH_MASK (HASH_SIZE-1)

extern uint64_t hashesw[sizeX][sizeY];
extern uint64_t hasheswk[sizeX][sizeY];
extern uint64_t hashesb[sizeX][sizeY];
extern uint64_t hashesbk[sizeX][sizeY];
extern uint64_t init_hashv;
extern hash_type *hash_v;

signature rand_v();
void init_hash();
signature compute_hash(int tab[sizeX][sizeY]);
void check_hash(signature hv, int tab[sizeX][sizeY]);
int retrieve_v_hash(signature hv,int dist,int *v_inf,int *v_sup,Move *move);
void store_v_hash_both(signature hv,int v,int dist,int base,Move move);
void store_v_hash(signature hv,int alpha,int beta,int g,
		  int dist,int base,Move move);

#endif