#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h> 

// return the number of jumps
int read_move(int positions[]){
    fflush(stdin); fscanf(stdin, "%d", &(positions[0]));
    fflush(stdin); bool is_jump = (fgetc(stdin) == 'x');

    fprintf(stderr, "a\n"); fflush(stdout);
    if (!is_jump) {
        fflush(stdin); fscanf(stdin, "%d", &(positions[1]));
        fprintf(stderr, "b\n"); fflush(stdout);
        return 0;
    } else {
        int end = -1;
        fflush(stdin); fscanf(stdin, "%d\n", &end);
        printf("end = %d\n", end); fflush(stdout);

        char buffer[250];
        fflush(stdin); fgets(buffer, 250, stdin);
        fprintf(stderr, "%s\n", buffer);
        return -1;
    }
}

int main(int argc, char *argv[]){
    int AI_color = (int)(argv[1][0] - '0');
    int positions[30];
    int jumps = 0;

    sleep(5);

    // Send a basic move
    printf("%d-%d\n", 33,28); fflush(stdout);

    // receive next move
    jumps = read_move(positions);
    fprintf(stderr, "%d jumps : %d->%d\n", jumps, positions[0], positions[1]); fflush(stderr);

    // send next 
    printf("%d-%d\n", 28,22); fflush(stdout);

    // receive next move
    jumps = read_move(positions);
    fprintf(stderr, "%d jumps\n", jumps); fflush(stderr);

    return 0;
}