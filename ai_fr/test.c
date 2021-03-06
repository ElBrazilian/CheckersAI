#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <unistd.h> 

#define MAX_JUMPS 25

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

int board[1+12+1][1+12+1];
const int sizeX=12, sizeY=12;
struct direction { int dx, dy; };
const struct direction directions[4] = {{+1,+1},{+1,-1},{-1,+1},{-1,-1}};

void do_piece_jump(int x, int y, int dx, int dy)
{
    board[x+2*dx][y+2*dy] = board[x][y];
    board[x][y] = EMPTY;
    board[x+dx][y+dy] = EMPTY;
}

void undo_piece_jump(int x, int y, int dx, int dy, int taken_piece)
{
    board[x][y] = board[x+2*dx][y+2*dy];
    board[x+2*dx][y+2*dy] = EMPTY;
    board[x+dx][y+dy] = taken_piece;
}

int std_position(int x, int y)   { return (x-1)/2 + 5*(y-1) + 1; }
int coordY(int pos)              { return (pos-1)/5 + 1; }
int coordX(int pos)              { return (pos-1)%5*2 + 2 - (pos-1)/5%2; }
int get_piece(int pos)           { return board[coordX(pos)][coordY(pos)]; }
void set_piece(int pos, int val) { board[coordX(pos)][coordY(pos)] = val; }

void init_board(void)
{
    for (int x=0; x<sizeX; x++) for (int y=0; y<sizeY; y++)
        board[x][y] = EMPTY;
    for (int x=0; x<sizeX; x++)
        board[x][0] = board[x][sizeY-1] = OUTSIDE;
    for (int y=0; y<sizeY; y++)
        board[0][y] = board[sizeX-1][y] = OUTSIDE;
    for (int pos= 1; pos<=20; pos++) set_piece(pos,BLACK);
    for (int pos=31; pos<=50; pos++) set_piece(pos,WHITE);
}
void print_board(void)
{
    fprintf(stderr, "   1 2 3 4 5 6 7 8 9 10\n");
    for (int y=1; y<sizeY-1; y++) {
        fprintf(stderr, "%2d ", y);
        for (int x=1; x<sizeX-1; x++)
            fprintf(stderr, "%d ",board[x][y]);
        fprintf(stderr, "\n");
    }
    fflush(stderr);
}
int eval_board(){
    int res = 0;
    for (int pos = 1; pos < 51; pos++){
        int x = coordX(pos), y = coordY(pos);
        int piece = board[x][y];

        if (piece != EMPTY){
            int sign = (color(piece) == IA_color) ? 1 : -1;
            int weight = (is_king(piece)) ? 2 : 1;
            res += sign * weight;
        }
    }
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

int best_move_positions[MAX_JUMPS];
int eaten_positions[MAX_JUMPS + 1];
int best_nb_jumps=0;
void memorize_best_jump(int positions[MAX_JUMPS], int eaten[MAX_JUMPS + 1], int nb_jumps)
{
    memcpy(best_move_positions, positions, sizeof(best_move_positions));
    memcpy(eaten_positions, eaten, sizeof(eaten_positions));
    best_nb_jumps = nb_jumps;
}

void memorize_best_move(int from, int to)
{
    best_move_positions[0] = from;
    best_move_positions[1] = to;
    best_nb_jumps = 0;
}
int best_jump(int positions[], int eaten[],  int jump_num, int best_eval_so_far, int alpha, int beta)
{
    int x = coordX(positions[jump_num]), y = coordY(positions[jump_num]);
    // print_board();

    int jumping_piece = board[x][y];
    bool can_jump = false;

    // if (is_eaten(jumping_piece)) {printf("WTF\n"); exit(1);}

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

                        best_eval_so_far = best_jump(positions, eaten, jump_num+1, best_eval_so_far, alpha, beta);

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
            if (are_opponents(jumping_piece, jumped_piece) && board[x+2*dx][y+2*dy]==EMPTY) {
                can_jump = true;
                positions[jump_num+1] = std_position(x+2*dx,y+2*dy);
                eaten[jump_num] = std_position(x+dx, y+dy);
                do_piece_jump(x, y, dx, dy);
                best_eval_so_far = best_jump(positions, eaten, jump_num+1, best_eval_so_far, alpha, beta);
                undo_piece_jump(x, y, dx, dy, jumped_piece);
            }
        }
    }
    if (!can_jump) { // feuille de l'arbre des prises : on ??value le coup
        int eval = jump_num * 10000 + eval_board(); // remplacez par votre ??valuation (alphabeta ou autre) !
        if (eval > best_eval_so_far) {
            best_eval_so_far = eval;
            memorize_best_jump(positions, eaten, jump_num);
        }
    }
    return best_eval_so_far;
}
bool piece_can_jump(int pos){
    int piece = get_piece(pos);
    int x = coordX(pos), y = coordY(pos);

    bool can_jump = false;
    for (int dir=0; dir<4; dir++) {
        int dx = directions[dir].dx, dy = directions[dir].dy;

        if (is_king(piece)){
            int nx = x+dx, ny = y+dy;
            while ((1 <= nx && nx < sizeX-1 && 1 <= ny && ny < sizeY-1) && !are_opponents(piece, board[nx][ny])){
                nx += dx;
                ny += dy;
            }

            // check if current one is actually opponent and if next one is empty
            if (are_opponents(piece, board[nx][ny])){
                if (board[nx + dx][ny + dy] == EMPTY){
                    can_jump = true;
                }
            }

        } else {
            int neigh = board[x+dx][y+dy];
            if (are_opponents(piece, neigh) && board[x+2*dx][y+2*dy] == EMPTY){
                can_jump = true;
            }
        }
    }
    return can_jump;
}
bool someone_can_jump(int color)
{
    for (int pos=1; pos<=50; pos++) 
        if (are_same_color(get_piece(pos),color) && piece_can_jump(pos))
            return true;
    return false;
}
int best_move(int from_pos, int best_eval_so_far, int alpha, int beta)
{
    int piece = get_piece(from_pos);
    int x = coordX(from_pos), y = coordY(from_pos);
    for (int dir=0; dir<4; dir++) {
        int dx = directions[dir].dx, dy = directions[dir].dy;
        if (!is_king(piece) && dy!=forward_direction(color(piece)))
            continue; // les pions ne vont que vers l'avant
        if (board[x+dx][y+dy]==EMPTY) {
            int eval = rand(); // remplacez par votre ??valuation
            if (eval > best_eval_so_far) {
                best_eval_so_far = eval;
                memorize_best_move(from_pos, std_position(x+dx,y+dy));
            }
        }
    }
    return best_eval_so_far;
}
//////////////////////////////////////// END //////////////////////////

// Read and update moves
typedef struct Move {
    int from;
    int to;

    int num_jumps;
    int jumps_pos[MAX_JUMPS];
    int positions[MAX_JUMPS + 1];
} Move;
void init_move(Move *move){
    move->from = 0;
    move->to = 0;
    move->num_jumps = 0;
    for (int i = 0; i < MAX_JUMPS; i++){
        move->jumps_pos[i] = 0;
        move->positions[i] = 0;
    }
    move->positions[MAX_JUMPS] = 0;
}

void generate_eaten_pos(Move *move){
    int positions[MAX_JUMPS + 1];
    int eaten[MAX_JUMPS];
    for (int i = 0; i < MAX_JUMPS; i++){
        positions[i] = -1;
        eaten[i] = -1;
    }
    positions[MAX_JUMPS] = -1;

    // move->positions[best_nb_jumps] = positions[best_nb_jumps];

    positions[0] = move->from;
    best_jump(positions, eaten, 0, 0, 0,0);

    for (int i = 0; i < best_nb_jumps; i++){
        move->positions[i] = best_move_positions[i];
        move->jumps_pos[i] = eaten_positions[i];
    }
    move->positions[best_nb_jumps] = best_move_positions[best_nb_jumps];
    move->num_jumps = best_nb_jumps;
}
// return true if it's a jump, false if it's a regular move
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
void update_board(Move move){
    int c = color(board[coordX(move.from)][coordY(move.from)]);
    board[coordX(move.to)][coordY(move.to)] = c;
    board[coordX(move.from)][coordY(move.from)] = EMPTY;

    for (int i = 0; i < move.num_jumps; i++){
        int x = coordX(move.jumps_pos[i]), y = coordY(move.jumps_pos[i]);
        board[x][y] = EMPTY;
    }

    /* promotion en dame */
    if ((c==BLACK && move.to>28) || (c==WHITE && move.to<5))
        set_piece(move.to,promote_king(board[coordX(move.to)][coordY(move.to)]));
}
void compute_move(Move *move){
    int alpha = -100000, beta = 1000000;
    int best_eval_so_far = -10000000;
    int positions[MAX_JUMPS + 1];
    int eaten[MAX_JUMPS];
    for (int i = 0; i < MAX_JUMPS; i++){
        positions[i] = -1;
        eaten[i] = -1;
    }
    positions[MAX_JUMPS] = -1;

    fprintf(stderr, "start compute\n"); fflush(stderr);

    // if someone can jump, jump
    if (someone_can_jump(IA_color)) {
        for (int pos=1; pos<=50; pos++)
            if (are_same_color(get_piece(pos),IA_color) && piece_can_jump(pos)) {
                positions[0] = pos;
                best_eval_so_far = best_jump(positions, eaten, 0, best_eval_so_far, alpha, beta);
                // if (best_eval_so_far > alpha) alpha = best_eval_so_far;
            }
    } else {
        for (int pos=1; pos<=32; pos++)
            if (are_same_color(get_piece(pos),IA_color)) {
                best_eval_so_far = best_move(pos, best_eval_so_far, alpha, beta);
                // if (best_eval_so_far > alpha) alpha = best_eval_so_far;
            }
    }

    // Write the result to move
    for (int i = 0; i < best_nb_jumps; i++){
        move->positions[i] = best_move_positions[i];
        move->jumps_pos[i] = eaten_positions[i];
    }
    move->positions[best_nb_jumps] = best_move_positions[best_nb_jumps];
    move->num_jumps = best_nb_jumps;
    move->from = best_move_positions[0];
    move->to = best_move_positions[(best_nb_jumps == 0) ? 1 : best_nb_jumps];

    fprintf(stderr, "done compute\n"); fflush(stderr);

}

int main(int argc, char *argv[]){
    IA_color = (int)(argv[1][0] - '0');
    if (IA_color == 1) IA_color = WHITE;
    else IA_color = BLACK;

    init_board();
    // load_board();

    Move move;
    init_move(&move);
    // read_move(&move);

    sleep(5);
    fprintf(stderr, "ready\n"); fflush(stderr);
    fprintf(stderr, "%d\n", IA_color); fflush(stderr);

    int turn = WHITE;
    while (true){
        if (turn == IA_color){
            fprintf(stderr, "pre start compute\n"); fflush(stderr);
            compute_move(&move);
            fprintf(stderr, "sending move\n"); fflush(stderr);
            send_move(move);
        } else {
            read_move(&move);
        }
        update_board(move);
        turn = opponent_color(turn);
    }



    return 0;
}