#include "interface.h"

bool read_move(Move *move){
    fflush(stdin); fscanf(stdin, "%d", &(move->from));
    fflush(stdin); bool is_jump = (fgetc(stdin) == 'x');
    fflush(stdin); fscanf(stdin, "%d", &(move->to));
            
    char next;
    move->num_jumps = 0;
    if (is_jump){
        int tmp = 0;
        next = getchar(); // on lit le '\n'
        next = getchar(); // on lit le '\n'
        while (next != ')'){
            fflush(stdin); fscanf(stdin, "%d", &tmp);
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
