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
int eval_board(int player_color){
    int res = 0;
    for (int pos = 1; pos < 51; pos++){
        int x = coordX(pos), y = coordY(pos);
        int piece = board[x][y];

        if (piece != EMPTY){
            int sign = (color(piece) == player_color) ? 1 : -1;
            int weight = (is_king(piece)) ? 2 : 1;
            res += sign * weight;
        }
    }
    return res;
}

int best_move_positions[12000];
int eaten_positions[12000];
int best_nb_jumps=0;
void memorize_best_jump(int positions[12000], int eaten[12000], int nb_jumps)
{
    memcpy(best_move_positions, positions, sizeof(best_move_positions));
    best_nb_jumps = nb_jumps;
}

void memorize_best_move(int from, int to)
{
    best_move_positions[0] = from;
    best_move_positions[1] = to;
    best_nb_jumps = 0;
}
int best_jump(int player_color, int positions[], int eaten[],  int jump_num, int best_eval_so_far, int alpha, int beta)
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

                        best_eval_so_far = best_jump(player_color, positions, eaten, jump_num+1, best_eval_so_far, alpha, beta);

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
                do_piece_jump(x, y, dx, dy);
                best_eval_so_far = best_jump(player_color, positions, eaten, jump_num+1, best_eval_so_far, alpha, beta);
                undo_piece_jump(x, y, dx, dy, jumped_piece);
            }
        }
    }
    if (!can_jump) { // feuille de l'arbre des prises : on évalue le coup
        int eval = jump_num * 10000 + eval_board(player_color); // remplacez par votre évaluation (alphabeta ou autre) !
        if (eval > best_eval_so_far) {
            best_eval_so_far = eval;
            memorize_best_jump(positions, eaten, jump_num);
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

void generate_eaten_pos(Move *move){
    move->positions[0] = move->from;
    int positions[MAX_JUMPS + 1];
    int eaten[MAX_JUMPS];
    best_jump(0, positions, eaten, 0, 0, 0,0);

    for (int i = 0; i < best_nb_jumps; i++){
        move->positions[i] = positions[i];
        move->jumps_pos[i] = eaten[i];
    }
    move->positions[best_nb_jumps] = positions[best_nb_jumps];
}
// return true if it's a jump, false if it's a regular move
bool read_move(Move *move){
    fflush(stdin); fscanf(stdin, "%d", &(move->from));
    fflush(stdin); bool is_jump = (fgetc(stdin) == 'x');
    fflush(stdin); fscanf(stdin, "%d", &(move->to));

    if (is_jump) generate_eaten_pos(move);
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
}


int main(int argc, char *argv[]){
    IA_color = (int)(argv[1][0] - '0');
    init_board();

    int moves = 6;
    int tmp[6][2] = {
        {16,21},
        {21,26},
        {20,25},
        {11,16},
        {6,11},
        {17,22}
    };

    // quand on envoie, faut mettre les pos intermédiaires
    Move move;

    for (int i = 0; i < moves; i++){
        read_move(&move);
        update_board(move);
        // print_board();
        // fprintf(stderr, "done reading move\n"); fflush(stderr);

        move.from = tmp[i][0];
        move.to = tmp[i][1];
        move.num_jumps = 0;
        send_move(move);
        update_board(move);
        // print_board();
        // fprintf(stderr, "done sending move\n"); fflush(stderr);
    }



    return 0;
}