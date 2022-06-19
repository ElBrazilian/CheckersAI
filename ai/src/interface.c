#include "interface.h"

bool read_move(Move *move){
    int fres; 
    fflush(stdin); fres = fscanf(stdin, "%d", &(move->from));
    if (fres != 1) { fprintf(stderr, "ERROR read_move fscanf #1\n"); fflush(stderr); exit(1);}
    fflush(stdin); bool is_jump = (fgetc(stdin) == 'x');
    fflush(stdin); fres = fscanf(stdin, "%d", &(move->to));
    if (fres != 1) { fprintf(stderr, "ERROR read_move fscanf #2\n"); fflush(stderr); exit(1);}
            
    char next;
    move->num_jumps = 0;
    if (is_jump){
        int tmp = 0;
        next = getchar(); // on lit le '\n'
        next = getchar(); // on lit le '\n'
        while (next != ')'){
            fflush(stdin); fres = fscanf(stdin, "%d", &tmp);
            if (fres != 1) { fprintf(stderr, "ERROR read_move fscanf #3\n"); fflush(stderr); exit(1);}

            fprintf(stderr, "READ %d\n", tmp); fflush(stderr);
            next = getchar(); // On lit le prochain ' ' ou ')'

            move->jumps_pos[move->num_jumps] = tmp;
            move->num_jumps++;
        }   
        next = getchar(); // read \n

    }


    
    return is_jump;
}
void send_move(Move move){
    if (move.num_jumps == 0){
        printf("%d-%d\n", move.from, move.to);
    } else {
        printf("%dx%d\n(", move.from, move.to);
        for (int i = 0; i < move.num_jumps; i++){
            if (i < move.num_jumps-1) printf("%d ", move.jumps_pos[i]);
            else printf("%d)\n", move.jumps_pos[i]);
        }
    }
    fflush(stdout);
}
