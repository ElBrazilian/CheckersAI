#include "hash.h"

long int uuid=0;

uint64_t hashesw[sizeX][sizeY];
uint64_t hasheswk[sizeX][sizeY];
uint64_t hashesb[sizeX][sizeY];
uint64_t hashesbk[sizeX][sizeY];
uint64_t init_hashv;
hash_type *hash_v;

uint64_t rand_v()
{
  return
    (((uint64_t)lrand48())<<62) ^
    (((uint64_t)lrand48())<<31) ^
    ((uint64_t)lrand48());
}

void init_hash()
{
  int i,j;
  for (i=0;i<sizeX;i++)
    for (j=0;j<sizeY;j++) {
      hashesw[i][j]=rand_v();
      hasheswk[i][j]=rand_v();
      hashesb[i][j]=rand_v();
      hashesbk[i][j]=rand_v();
    }
  hash_v=(hash_type *)calloc(HASH_SIZE,sizeof(hash_type));
  init_hashv=rand_v();
}

signature compute_hash(int tab[sizeX][sizeY]){
    signature hv2=init_hashv;
    for (int x=1;x<sizeX-1;x++){
        for (int y=1;y<sizeY-1;y++){
            if (color(tab[x][y])==WHITE) {
                if (is_king(tab[x][y])){
                    hv2^=hasheswk[x][y];
                } else {
                    hv2^=hashesw[x][y];
                }
            }
            else if (color(tab[x][y])==BLACK) {
                if (is_king(tab[x][y])){
                    hv2^=hashesbk[x][y];
                } else {
                    hv2^=hashesb[x][y];
                }
            }
        }
    }
    return hv2;
}
void check_hash(signature hv, int tab[sizeX][sizeY])
{
    signature hv2=compute_hash(tab);
    if (hv2!=hv) {
        printf("%ld %lx %lx\n",uuid,hv,hv2);
        print_board(tab);
        exit(-1);
    }
}

int retrieve_v_hash(signature hv,int dist,int *v_inf,int *v_sup,Move *move)
{
  int ind=hv&HASH_MASK;
  if (hash_v[ind].sig==hv) {
    if (
        (hash_v[ind].dist==dist)
	        ||
	    ((hash_v[ind].v_inf==hash_v[ind].v_sup)&&(abs(hash_v[ind].v_inf)>=WIN))
    ) {
      *v_inf=hash_v[ind].v_inf;
      *v_sup=hash_v[ind].v_sup;
      *move=hash_v[ind].move;
      return true;
    }
    *move=hash_v[ind].move;
    return false;
  }
  return false;
}

void store_v_hash_both(signature hv,int v,int dist,int base,Move move)
{
  int ind = hv&HASH_MASK;
  if ((hash_v[ind].base!=base)||(hash_v[ind].dist<=dist)) {
    hash_v[ind].v_inf=(int16_t)v;
    hash_v[ind].v_sup=(int16_t)v;
    hash_v[ind].sig=hv;
    hash_v[ind].base=(uint8_t)base;
    hash_v[ind].move=move;
    hash_v[ind].dist=(uint8_t)dist;
    };
}
void store_v_hash(signature hv,int alpha,int beta,int g,
		  int dist,int base,Move move){
  
    int ind = hv&HASH_MASK;
    if ((hash_v[ind].base!=base)||(hash_v[ind].dist<=dist)) {
        if ((hash_v[ind].sig!=hv) || (hash_v[ind].dist!=dist)) {
            /* Not an update. Have to initialize/reset everything */
            hash_v[ind].v_inf=-INF;
            hash_v[ind].v_sup=INF;
            hash_v[ind].dist=(uint8_t)dist;
            hash_v[ind].sig=hv;
        }

        hash_v[ind].base=(uint8_t)base;
        hash_v[ind].move=move;
        if ((g>alpha)&&(g<beta)){
            hash_v[ind].v_inf=(int16_t)g;
            hash_v[ind].v_sup=(int16_t)g;
        } else if (g<=alpha){
            hash_v[ind].v_sup=(int16_t)g;
        } else if (g>=beta){
            hash_v[ind].v_inf=(int16_t)g;
        }
    }
}