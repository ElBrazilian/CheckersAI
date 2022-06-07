#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <unistd.h> 

#define MAX_JUMPS 25
#define WIN 31217

/////////////////// START //////////////////////////
const int EMPTY   = 0;
const int BLACK   = 1;
const int WHITE   = 2;
const int COLOR_MASK = 3;
const int KING    = 4;  // NB: les anglosaxons sont machistes
const int OUTSIDE = 8;
const int EATEN   = 16;

int IA_color;
int color(int piece)             { return piece & COLOR_MASK; }
int opponent_color(int piece)    { return piece ^ COLOR_MASK; }
int are_opponents(int a, int b)  { return  (color(a)|color(b))==COLOR_MASK; }
int are_same_color(int a, int b) { return color(a) == color(b); }
int is_king(int piece)           { return (piece & KING) != 0; }
int is_eaten(int piece)          { return (piece & EATEN) != 0; }
int promote_king(int piece)      { return piece | KING; }
int forward_direction(int color) { return color==BLACK ? +1 : -1; }

int board[12][12];
const int sizeX=12, sizeY=12;
struct direction { int dx, dy; };
const struct direction directions[4] = {{+1,+1},{+1,-1},{-1,+1},{-1,-1}};

int std_position(int x, int y)   { return (x-1)/2 + 5*(y-1) + 1; }
int coordY(int pos)              { return (pos-1)/5 + 1; }
int coordX(int pos)              { return (pos-1)%5*2 + 2 - (pos-1)/5%2; }
int get_piece(int pos)           { return board[coordX(pos)][coordY(pos)]; }
void set_piece(int pos, int val) { board[coordX(pos)][coordY(pos)] = val; }

void init_board(void)
{
    for (int x=0; x<sizeX; x++) for (int y=0; y<sizeY; y++)
        board[x][y] = EMPTY;

    for (int x=0; x<sizeX; x++){
        board[x][0] = OUTSIDE;
        board[x][sizeY-1] = OUTSIDE;
    }
    for (int y=0; y<sizeY; y++)
        board[0][y] = board[sizeX-1][y] = OUTSIDE;
    for (int pos= 1; pos<=20; pos++) set_piece(pos,BLACK);
    for (int pos=31; pos<=50; pos++) set_piece(pos,WHITE);
}
void print_board(int tab[sizeX][sizeY])
{
    fprintf(stderr, "   1 2 3 4 5 6 7 8 9 10\n");
    for (int y=1; y<sizeY-1; y++) {
        fprintf(stderr, "%2d ", y);
        for (int x=1; x<sizeX-1; x++)
            fprintf(stderr, "%d ",tab[x][y]);
        fprintf(stderr, "\n");
    }
    fflush(stderr);
}
int eval_board(int tab[sizeX][sizeY]){
    int res = 0;
    int ones = 0, twos = 0;
    for (int pos = 1; pos < 51; pos++){
        int x = coordX(pos), y = coordY(pos);
        int piece = tab[x][y];

        if (piece != EMPTY){
            int sign = (color(piece) == IA_color) ? 1 : -1;
            if (sign == 1) ones++; else twos++;
            int weight = (is_king(piece)) ? 2 : 1;
            res += sign * weight;
        }
    }
    // fprintf(stderr, "%d %d\n", ones, twos); fflush(stderr);
    if (ones == 0) return WIN; // l'IA a perdu
    else if (twos == 0) return -WIN;
    return res;
}
void load_board(){
    for (int x=0; x<sizeX; x++) for (int y=0; y<sizeY; y++)
        board[x][y] = EMPTY;
    for (int x=0; x<sizeX; x++)
        board[x][0] = board[x][sizeY-1] = OUTSIDE;
    for (int y=0; y<sizeY; y++)
        board[0][y] = board[sizeX-1][y] = OUTSIDE;
        
    FILE *file = NULL;
    file = fopen("board.txt", "r");
    for (int y=1; y<sizeY-1; y++){
        for (int x=1; x<sizeX-1; x++){
            board[x][y] = (fgetc(file) - '0');
        }
        fgetc(file);
    }
    fclose(file);
}

typedef struct Move {
    int from;
    int to;

    int num_jumps;
    int jumps_pos[MAX_JUMPS];
    int positions[MAX_JUMPS + 1];
} Move;

Move **all_possible_moves;
int current_move = 0;
int max_jumps = 0;






bool read_move(Move *move){
    fflush(stdin); fscanf(stdin, "%d", &(move->from));
    fflush(stdin); bool is_jump = (fgetc(stdin) == 'x');
    fflush(stdin); fscanf(stdin, "%d", &(move->to));

    if (is_jump){
        generate_eaten_pos(move);

    }
    else move->num_jumps = 0;


    
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
void update_board(int tab[sizeX][sizeY], Move move){
    int piece = tab[coordX(move.from)][coordY(move.from)];
    int c = color(piece);
    tab[coordX(move.to)][coordY(move.to)] = piece;
    tab[coordX(move.from)][coordY(move.from)] = EMPTY;

    for (int i = 0; i < move.num_jumps; i++){
        int x = coordX(move.jumps_pos[i]), y = coordY(move.jumps_pos[i]);
        tab[x][y] = EMPTY;
    }

    /* promotion en dame */
    if ((c==BLACK && move.to>45) || (c==WHITE && move.to<6))
        set_piece(move.to,promote_king(tab[coordX(move.to)][coordY(move.to)]));
}

void rec_compute_jumps(int color, int tab[sizeX][sizeY], int jump_num, int positions[], int eaten[]){
    int x = coordX(positions[jump_num]), y = coordY(positions[jump_num]);
    int jumping_piece = board[x][y];
    bool can_jump = false;

    for (int dir=0; dir<4; dir++) {
        int dx = directions[dir].dx, dy = directions[dir].dy;

        if (is_king(jumping_piece)){
            // on commence par regarder si on peut "manger"
            int nx = x+dx, ny = y+dy;
            while ((1 <= nx && nx < sizeX-1 && 1 <= ny && ny < sizeY-1) && !are_opponents(jumping_piece, board[nx][ny])){
                nx += dx;
                ny += dy;
            }

            // check if current one is actually opponent and if next one is empty
            if (are_opponents(jumping_piece, board[nx][ny]) && !is_eaten(board[nx][ny])){
                if (board[nx + dx][ny + dy] == EMPTY){
                    can_jump = true;

                    int ex = nx, ey = ny; // position of the eaten piece

                    eaten[jump_num] = std_position(ex, ey);
                    board[ex][ey] |= EATEN; 
                    board[x][y] = EMPTY;

                    nx += dx;
                    ny += dy;
                    while ((1 <= nx && nx < sizeX-1 && 1 <= ny && ny < sizeY-1) && board[nx][ny] == EMPTY) {
                        // do jump
                        board[nx][ny] = jumping_piece;
                        positions[jump_num+1] = std_position(nx, ny);

                        rec_compute_jumps(color, tab, jump_num+1, positions, eaten);

                        // Undo jump
                        board[nx][ny] = EMPTY;

                        nx += dx;
                        ny += dy;
                    }
                    board[ex][ey] = opponent_color(jumping_piece);
                    board[x][y] = jumping_piece;
                }
            }

        } else {
            int jumped_piece = board[x+dx][y+dy];
            int nnx = x+2*dx, nny = y + 2 * dy;
            if ((0 <= nnx && nnx < sizeX && 0 <= nny && nny <= sizeY) && are_opponents(jumping_piece, jumped_piece) && board[x+2*dx][y+2*dy]==EMPTY) {
                can_jump = true;
                positions[jump_num+1] = std_position(nnx, nny);
                eaten[jump_num] = std_position(x+dx, y+dy);
                
                // do piece jump
                tab[nnx][nny] = tab[x][y];
                tab[x+dx][y+dy] = EMPTY;
                tab[x][y] = EMPTY;

                rec_compute_jumps(color, tab, jump_num+1, positions, eaten);

                // undo piece jupmp
                tab[nnx][nny] = EMPTY;
                tab[x+dx][y+dy] = jumped_piece;
                tab[x][y] = jumping_piece;
            }
        }
    }
    if (!can_jump) { // feuille de l'arbre des prises : on évalue le coup
        

    }
}
int compute_jumps_from_pos(int color, int tab[sizeX][sizeY], int start_pos){
    int positions[MAX_JUMPS+1];
    int eaten[MAX_JUMPS];

    rec_compute_jumps(color, tab, 0, positions, eaten);


}

void compute_possible_moves(int color, int tab[sizeX][sizeY]){
    for (int i = 0; i < current_move; i++){
        free(all_possible_moves[i]);
    }

    current_move = 0;

}


int main(int argc, char *argv[]){
    all_possiblemove
    IA_color = (int)(argv[1][0] - '0');
    if (IA_color == 1) IA_color = WHITE;
    else IA_color = BLACK;

    // init_board();
    load_board();
    compute_possible_moves(1, board);
    return 1;

    Move move;
    init_move(&move);
    // read_move(&move);

    sleep(5);
    // fprintf(stderr, "ready\n"); fflush(stderr);
    // fprintf(stderr, "%d\n", IA_color); fflush(stderr);

    int turn = WHITE;
    while (true){
        if (turn == IA_color){
            // fprintf(stderr, "pre start compute\n"); fflush(stderr);
            compute_move(&move);
            // fprintf(stderr, "sending move\n"); fflush(stderr);
            send_move(move);
        } else {
            read_move(&move);
        }
        update_board(board, move);
        turn = opponent_color(turn);
        // print_board();
    }



    return 0;
}