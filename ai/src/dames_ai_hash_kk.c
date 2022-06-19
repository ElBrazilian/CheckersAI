#define _XOPEN_SOURCE

#include "board.h"
#include "moves.h"
#include "interface.h"

#define INF 999999


#define BWIN WIN
#define MAXV 32767
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

long int uuid=0;


void init(int argc, char *argv[], bool use_load);

typedef uint64_t signature;
struct
__attribute__((packed))
_hash_type 
{
  uint64_t sig;
  int16_t v_inf,v_sup;
  Move bmove;
  uint8_t base;
  uint8_t dist;
};
typedef struct _hash_type hash_type;
#define NB_BITS 26
#define HASH_SIZE (1<<NB_BITS)
#define HASH_MASK (HASH_SIZE-1)
uint64_t hashesw[sizeX][sizeY];
uint64_t hashesb[sizeX][sizeY];
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
      hashesb[i][j]=rand_v();
    }
  hash_v=(hash_type *)calloc(HASH_SIZE,sizeof(hash_type));
  init_hashv=rand_v();
}

void check_hash(uint64_t hv, int tab[sizeX][sizeY])
{
  uint64_t hv2=init_hashv;
  int x,y;
  for (x=1;x<sizeX-1;x++)
    for (y=1;y<sizeY-1;y++)
      if (color(tab[x][y])==WHITE) hv2^=hashesw[x][y];
      else if (color(tab[x][y])==BLACK) hv2^=hashesb[x][y];
  if (hv2!=hv) {
    printf("%ld %lx %lx\n",uuid,hv,hv2);
    print_board(tab);
    exit(-1);
  }
}

int retrieve_v_hash(uint64_t hv,int dist,int *v_inf,int *v_sup,Move *bmove)
{
  int ind=hv&HASH_MASK;
  if (hash_v[ind].sig==hv) {
    if ((hash_v[ind].dist==dist)

	||
	((hash_v[ind].v_inf==hash_v[ind].v_sup)&&(abs(hash_v[ind].v_inf)>=BWIN))

) {
      *v_inf=hash_v[ind].v_inf;
      *v_sup=hash_v[ind].v_sup;
      *bmove=hash_v[ind].bmove;
      return true;
    }
    *bmove=hash_v[ind].bmove;
    return false;
  }
  return false;
}

void store_v_hash_both(uint64_t hv,int v,int dist,int base,Move move)
{
  int ind = hv&HASH_MASK;

  if ((hash_v[ind].base!=base)||(hash_v[ind].dist<=dist)) {
    hash_v[ind].v_inf=(int16_t)v;
    hash_v[ind].v_sup=(int16_t)v;
    hash_v[ind].sig=hv;
    hash_v[ind].base=(uint8_t)base;
    hash_v[ind].bmove=move;
    hash_v[ind].dist=(uint8_t)dist;
    };
}
void store_v_hash(uint64_t hv,int alpha,int beta,int g,
		  int dist,int base,Move move)
{
  int ind = hv&HASH_MASK;

  if ((hash_v[ind].base!=base)||(hash_v[ind].dist<=dist)) {
    if ((hash_v[ind].sig!=hv) || (hash_v[ind].dist!=dist)) {
      /* Not an update. Have to initialize/reset everything */
      hash_v[ind].v_inf=-MAXV;
      hash_v[ind].v_sup=MAXV;
      hash_v[ind].dist=(uint8_t)dist;
      hash_v[ind].sig=hv;
    }
    hash_v[ind].base=(uint8_t)base;
    hash_v[ind].bmove=move;
    if ((g>alpha)&&(g<beta))
      {hash_v[ind].v_inf=(int16_t)g;hash_v[ind].v_sup=(int16_t)g;}
    else if (g<=alpha) hash_v[ind].v_sup=(int16_t)g;
    else if (g>=beta) hash_v[ind].v_inf=(int16_t)g;
    }
}

int alphabeta(int alpha, int beta, int tab[sizeX][sizeY], int depth, int base, int max_depth, Move *best_move, uint64_t hv){

    int a = alpha,b = beta;
    int v_inf,v_sup;
    int do_bmove = false;
    Move bmove, lmove;
    
    uuid++;
    if (retrieve_v_hash(hv,max_depth-depth,&v_inf,&v_sup,&bmove)) {
        if (depth==base) {*best_move=bmove;}
        if (v_inf==v_sup) return v_inf; /* Exact evaluation */
        if (v_inf>=beta) return v_inf; /* Beta cut */
        if (v_sup<=alpha)  return v_sup; /* Alpha cut */
        alpha=max(alpha,v_inf);
        beta=min(beta,v_sup);
        do_bmove = true;
    }



    int eval = eval_board(tab);
    if (abs(eval) == WIN || depth == max_depth){
        return eval;
    }

    int v = -1;
    if ((depth % 2) == 0){
        // l'IA qui joue
        // Noeud de type MAX
        v = -INF;
        a = alpha;
        PossibleMoves possible_moves;
        init_possible_moves(&possible_moves);
        compute_possible_moves(IA_color, tab, &possible_moves);

        int tabcopy[sizeX][sizeY];
        for (int i = 0; i < possible_moves.current_move; i++){
            // play next move
            for (int x = 0; x < sizeX; x++){
                for (int y = 0; y < sizeY; y++){
                    tabcopy[x][y] = tab[x][y];
                }
            }
            Move cur_move;
            if (do_bmove){
                cur_move = bmove;
                do_bmove = false;
                i--;
            } else {
                cur_move = possible_moves.all_possible_moves[i];
            }
            
            update_board(tabcopy, cur_move);

            signature nhv = hv;
            // compute next hash
            if (IA_color == WHITE){
                nhv ^= hashesw[coordX(cur_move.from)][coordY(cur_move.from)];
                nhv ^= hashesw[coordX(cur_move.to)][coordY(cur_move.to)];
                for (int k = 0; k < cur_move.num_jumps; k++){
                    nhv ^= hashesb[coordX(cur_move.jumps_pos[k])][coordY(cur_move.jumps_pos[k])];
                }
            } else {
                nhv ^= hashesb[coordX(cur_move.from)][coordY(cur_move.from)];
                nhv ^= hashesb[coordX(cur_move.to)][coordY(cur_move.to)];
                for (int k = 0; k < cur_move.num_jumps; k++){
                    nhv ^= hashesw[coordX(cur_move.jumps_pos[k])][coordY(cur_move.jumps_pos[k])];
                }
            }
            check_hash(nhv, tabcopy);
            int next = alphabeta(a, beta, tabcopy, depth+1, base, max_depth, best_move, nhv);
            if (next > v) v = next; // v = max(next, v)
            
            // if (v >= beta)
            //     return v;
            // }
            if (v > a) {
                a = v;
                if (depth == base){
                    // fprintf(stderr, "YOYOYO\n"); fflush(stderr);
                    lmove = cur_move;
                    best_move->from = cur_move.from;
                    best_move->to = cur_move.to;
                    best_move->num_jumps = cur_move.num_jumps;
                    for (int j = 0; j < best_move->num_jumps; j++){
                        best_move->jumps_pos[j] = cur_move.jumps_pos[j];
                        best_move->positions[j] = cur_move.positions[j];
                    }
                    if (best_move->jumps_pos > 0) best_move->positions[best_move->num_jumps] = cur_move.positions[best_move->num_jumps];
                }    
            } // alpha = max(alpha, v)
        }

        destroy_possible_moves(&possible_moves);
    } else {
        // noeud de type MIN
        // L'autre joeuru joue
        v = INF;
        PossibleMoves possible_moves;
        init_possible_moves(&possible_moves);
        compute_possible_moves((IA_color == WHITE) ? BLACK : WHITE, tab, &possible_moves);

        int tabcopy[sizeX][sizeY];
        for (int i = 0; i < possible_moves.current_move; i++){
            // play next move
            for (int x = 0; x < sizeX; x++){
                for (int y = 0; y < sizeY; y++){
                    tabcopy[x][y] = tab[x][y];
                }
            }

            Move cur_move;
            if (do_bmove){
                cur_move = bmove;
                do_bmove = false;
                i--;
            } else {
                cur_move = possible_moves.all_possible_moves[i];
            }
            update_board(tabcopy, cur_move);
            signature nhv = hv;
            // compute next hash
            if (IA_color == BLACK){
                nhv ^= hashesw[coordX(cur_move.from)][coordY(cur_move.from)];
                nhv ^= hashesw[coordX(cur_move.to)][coordY(cur_move.to)];
                for (int k = 0; k < cur_move.num_jumps; k++){
                    nhv ^= hashesb[coordX(cur_move.jumps_pos[k])][coordY(cur_move.jumps_pos[k])];
                }
            } else {
                nhv ^= hashesb[coordX(cur_move.from)][coordY(cur_move.from)];
                nhv ^= hashesb[coordX(cur_move.to)][coordY(cur_move.to)];
                for (int k = 0; k < cur_move.num_jumps; k++){
                    nhv ^= hashesw[coordX(cur_move.jumps_pos[k])][coordY(cur_move.jumps_pos[k])];
                }
            }
            check_hash(nhv, tabcopy);
            int next = alphabeta(alpha, beta, tabcopy, depth+1, base, max_depth, best_move, nhv);
            if (next < v) v = next; // v = min(next, v)
            
            // if (alpha >= v){
            //     return v;
            // }

            if (v < b) {
                b = v; // beta = min(beta, v)
                if (depth == base){
                    // fprintf(stderr, "YOYOYO\n"); fflush(stderr);
                    lmove = cur_move;
                    best_move->from = cur_move.from;
                    best_move->to = cur_move.to;
                    best_move->num_jumps = cur_move.num_jumps;
                    for (int j = 0; j < best_move->num_jumps; j++){
                        best_move->jumps_pos[j] = cur_move.jumps_pos[j];
                        best_move->positions[j] = cur_move.positions[j];
                    }
                    if (best_move->jumps_pos > 0) best_move->positions[best_move->num_jumps] = cur_move.positions[best_move->num_jumps];
                }
            }
        }

        destroy_possible_moves(&possible_moves);
    }
// void store_v_hash(uint64_t hv,int alpha,int beta,int g,
// 		  int dist,int base,Move move)
    store_v_hash(hv,alpha,beta,v,max_depth-depth,base,lmove);
    return v;
}
int compute_move(Move *move){
    int base = 0;
    int maxdepth = 0;
    int ret = -INF;

    clock_t time,otime;
    double ftime;

    signature hv;
    init_hash();
    hv=init_hashv;
    for (int x=1;x<sizeX-1;x++)
        for (int y=1;y<sizeY-1;y++)
        if (color(board[x][y])==WHITE) hv^=hashesw[x][y];
        else if (color(board[x][y])==BLACK) hv^=hashesb[x][y];

    check_hash(hv, board);
    fprintf(stderr, "salut\n"); fflush(stderr);

    maxdepth=base;
    otime=clock();
    do {
      maxdepth++;
      ret = alphabeta(-INF, INF, board, 0, base, maxdepth, move,hv);
      time=clock()-otime;
      ftime=(double)time/(double)CLOCKS_PER_SEC;
      fprintf(stderr, "ret=%d base=%d maxdepth=%d time=%f\n", ret, base, maxdepth, ftime); fflush(stderr);

    } while ((ftime<=1.0)&&(maxdepth<15)&&(abs(ret) != WIN));

    return ret;
    // int eval = alphabeta(-INF, INF, board, 0, 0, 2, move);
    // fprintf(stderr, "%d\n", eval);

}


int main(int argc, char *argv[]){
    init(argc, argv, false);

    Move move;
    init_move(&move);

    sleep(5);

    int turn = WHITE;
    int eval = 0;
    while (true){
        if (turn == IA_color){
            // fprintf(stderr, "pre start compute\n"); fflush(stderr);
            eval = compute_move(&move);
            // fprintf(stderr, "sending move\n"); fflush(stderr);
            send_move(move);
        } else {
            read_move(&move);
        }
        update_board(board, move);
        print_board(board);
        fprintf(stderr, "%d jumps\n", move.num_jumps); fflush(stderr);
        fprintf(stderr, "eval is %d\n", eval); fflush(stderr);
        fprintf(stderr, "debug move: from %d to %d with %d jumps : (", move.from, move.to, move.num_jumps);
        for (int i = 0; i < move.num_jumps; i++){
            fprintf(stderr, "%d ", move.jumps_pos[i]);
        }
        fprintf(stderr, ")\n"); fflush(stderr);
        turn = opponent_color(turn);
        // print_board();
    }



    return 0;
}


void init(int argc, char *argv[], bool use_load){
    

    IA_color = (int)(argv[1][0] - '0');
    if (IA_color == 1) IA_color = WHITE;
    else IA_color = BLACK;

    init_board();
    if (use_load) load_board();
}