#include "board.h"
#include "moves.h"
#include "interface.h"

#include <time.h>

#define INF 999999

void init(int argc, char *argv[], bool use_load);

int alphabeta(int alpha, int beta, int tab[sizeX][sizeY], int depth, int base, int max_depth, Move *best_move){
    int eval = eval_board(tab);
    if (abs(eval) == WIN || depth == max_depth){
        return eval;
    }

    int v = -1;
    if ((depth % 2) == 0){
        // l'IA qui joue
        // Noeud de type MAX
        v = -INF;
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
            update_board(tabcopy, possible_moves.all_possible_moves[i]);

            int next = alphabeta(alpha, beta, tabcopy, depth+1, base, max_depth, best_move);
            if (next > v) v = next; // v = max(next, v)
            
            if (v >= beta){
                return v;
            }
            if (v > alpha) {
                alpha = v;
                if (depth == base){
                    // fprintf(stderr, "YOYOYO\n"); fflush(stderr);
                    best_move->from = possible_moves.all_possible_moves[i].from;
                    best_move->to = possible_moves.all_possible_moves[i].to;
                    best_move->num_jumps = possible_moves.all_possible_moves[i].num_jumps;
                    for (int j = 0; j < best_move->num_jumps; j++){
                        best_move->jumps_pos[j] = possible_moves.all_possible_moves[i].jumps_pos[j];
                        best_move->positions[j] = possible_moves.all_possible_moves[i].positions[j];
                    }
                    if (best_move->jumps_pos > 0) best_move->positions[best_move->num_jumps] = possible_moves.all_possible_moves[i].positions[best_move->num_jumps];
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
            update_board(tabcopy, possible_moves.all_possible_moves[i]);

            int next = alphabeta(alpha, beta, tabcopy, depth+1, base, max_depth, best_move);
            if (next < v) v = next; // v = min(next, v)
            
            if (alpha >= v){
                return v;
            }

            if (v < beta) {
                beta = v; // beta = min(beta, v)
                if (depth == base){
                    // fprintf(stderr, "YOYOYO\n"); fflush(stderr);
                    best_move->from = possible_moves.all_possible_moves[i].from;
                    best_move->to = possible_moves.all_possible_moves[i].to;
                    best_move->num_jumps = possible_moves.all_possible_moves[i].num_jumps;
                    for (int j = 0; j < best_move->num_jumps; j++){
                        best_move->jumps_pos[j] = possible_moves.all_possible_moves[i].jumps_pos[j];
                        best_move->positions[j] = possible_moves.all_possible_moves[i].positions[j];
                    }
                    if (best_move->jumps_pos > 0) best_move->positions[best_move->num_jumps] = possible_moves.all_possible_moves[i].positions[best_move->num_jumps];
                }
            }
        }

        destroy_possible_moves(&possible_moves);
    }

    return v;
}
int compute_move(Move *move){
    int base = 0;
    int maxdepth = 0;
    int ret = -INF;

    clock_t time,otime;
    double ftime;

    maxdepth=base;
    otime=clock();
    do {
      maxdepth++;
      ret = alphabeta(-INF, INF, board, 0, base, maxdepth, move);
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