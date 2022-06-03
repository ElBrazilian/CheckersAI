#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

/***********************************************************/
/* on code les pions par des bits séparés                  */

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

/****************************************************************/
/* le damier est ici une grille 12x12 (bordure extérieure)      */
/* On définit des fonctions pour convertir la notation standard */

int board[1+12+1][1+12+1];
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
    for (int x=0; x<sizeX; x++)
        board[x][0] = board[x][sizeY-1] = OUTSIDE;
    for (int y=0; y<sizeY; y++)
        board[0][y] = board[sizeX-1][y] = OUTSIDE;
    for (int pos= 1; pos<=20; pos++) set_piece(pos,BLACK);
    for (int pos=31; pos<=50; pos++) set_piece(pos,WHITE);
}
void print_board(void)
{
    for (int y=1; y<sizeY-1; y++) {
        for (int x=1; x<sizeX-1; x++)
            printf("%d",board[x][y]);
        printf("\n");
    }
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
            if (dy == forward_direction(color(piece))){
                int neigh = board[x+dx][y+dy];
                if (are_opponents(piece, neigh) && board[x+2*dx][y+2*dy] == EMPTY){
                    can_jump = true;
                }
            }
        }
    }
    return can_jump;
}

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
int best_move_positions[12000];
int best_nb_jumps=0;

void memorize_best_jump(int positions[12000], int nb_jumps)
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

int best_jump(int positions[], int jump_num, int best_eval_so_far, int alpha, int beta)
{
    int x = coordX(positions[jump_num]), y = coordY(positions[jump_num]);
    printf("best jump from (%d, %d)\n", x, y);
    // print_board();

    int jumping_piece = board[x][y];
    bool can_jump = false;

    // if (is_eaten(jumping_piece)) {printf("WTF\n"); exit(1);}

    for (int dir=0; dir<4; dir++) {
        int dx = directions[dir].dx, dy = directions[dir].dy;

        if (is_king(jumping_piece)){
            // on commence par regarder si on peut "manger"
            int nx = x+dx, ny = y+dy;
            while ((1 <= nx && nx < sizeX-1 && 1 <= ny && ny < sizeY-1) && !are_opponents(jumping_piece, board[nx][ny]) && (board[nx][ny] == EMPTY || !is_eaten(board[nx][ny]))){
                nx += dx;
                ny += dy;
            }

            // check if current one is actually opponent and if next one is empty
            if (are_opponents(jumping_piece, board[nx][ny])){
                if (board[nx + dx][ny + dy] == EMPTY){
                    // Si on peut manger, ie board[x][y] est celui qui mange et board[nx][ny] est un adversaire et board[nx+dx][ny+dy] est vide 
                    can_jump = true;
                    // positions[jump_num+1] = std_position(nx + dx, ny + dy);

                    int ex = nx, ey = ny; // position of the eaten piece
                    board[ex][ey] |= EATEN; // SHOULD BE EATEN
                    board[x][y] = EMPTY;

                    // DO JUMP
                    // board[nx+dx][ny+dy] = board[x][y];
                    // board[x][y] = EMPTY;
                    // board[nx][ny] = EMPTY;

                    nx += dx;
                    ny += dy;
                    while ((1 <= nx && nx < sizeX-1 && 1 <= ny && ny < sizeY-1) && board[nx][ny] == EMPTY) {
                        // do jump
                        board[nx][ny] = jumping_piece;
                        printf("%d\n", jump_num);
                        positions[jump_num+1] = std_position(nx, ny);

                        best_eval_so_far = best_jump(positions, jump_num+1, best_eval_so_far, alpha, beta);

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
                best_eval_so_far = best_jump(positions, jump_num+1, best_eval_so_far, alpha, beta);
                undo_piece_jump(x, y, dx, dy, jumped_piece);
            }
        }
    }
    if (!can_jump) { // feuille de l'arbre des prises : on évalue le coup
        int eval = jump_num * 10000; // remplacez par votre évaluation (alphabeta ou autre) !
        if (eval > best_eval_so_far) {
            best_eval_so_far = eval;
            memorize_best_jump(positions, jump_num);
        }
    }
    return best_eval_so_far;
}


int main(){
    IA_color = WHITE;
    load_board();

    board[5][4] = promote_king(get_piece(std_position(5,4)));


    print_board();

    int from = std_position(5,4);
    int best_eval_so_far = -1;
    int alpha = INT_MIN, beta = INT_MAX;
    int pos[12000];
    pos[0] = from;
    int max_eval = best_jump(pos, 0, best_eval_so_far, alpha, beta);
    printf("(%d, %d) = %d => %d (best_nb_jumps = %d)\n", coordX(from), coordY(from), board[coordX(from)][coordY(from)], max_eval, best_nb_jumps);
    for (int i = 0; i < best_nb_jumps + 1; i++){
        printf("(%d, %d): %d\n", coordX(pos[i]), coordY(pos[i]), pos[i]);
    }


    return 0;
}