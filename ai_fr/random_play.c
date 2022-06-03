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

int IA_color;
int color(int piece)             { return piece & COLOR_MASK; }
int opponent_color(int piece)    { return piece ^ COLOR_MASK; }
int are_opponents(int a, int b)  { return (color(a)|color(b))==COLOR_MASK; }
int are_same_color(int a, int b) { return color(a) == color(b); }
int is_king(int piece)           { return (piece & KING) != 0; }
int promote_king(int piece)      { return piece | KING; }
int forward_direction(int color) { return color==BLACK ? +1 : -1; }

/****************************************************************/
/* le damier est ici une grille 12x12 (bordure extérieure)      */
/* On définit des fonctions pour convertir la notation standard */

int board[1+12+1][1+12+1];
const int sizeX=12, sizeY=12;
struct direction { int dx, dy; };
const struct direction directions[4] = {{+1,+1},{+1,-1},{-1,+1},{-1,-1}};

int std_position(int x, int y)   { return (x-1)/2 + 4*(y-1) + 1; }
int coordY(int pos)              { return (pos-1)/4 + 1; }
int coordX(int pos)              { return (pos-1)%4*2 + 2 - (pos-1)/4%2; }
int get_piece(int pos)           { return board[coordX(pos)][coordY(pos)]; }
void set_piece(int pos, int val) { board[coordX(pos)][coordY(pos)] = val; }

int between_position(int from, int to)
{
    int fromX = coordX(from), fromY = coordY(from);
    int toX = coordX(to), toY = coordY(to);
    int dx = (toX-fromX)/2, dy = (toY-fromY)/2; 
    return std_position(fromX+dx, fromY+dy);
}

void init_board(void)
{
    for (int x=0; x<sizeX; x++) for (int y=0; y<sizeY; y++)
        board[x][y] = EMPTY;
    for (int x=0; x<sizeX; x++)
        board[x][0] = board[x][sizeY-1] = OUTSIDE;
    for (int y=0; y<sizeY; y++)
        board[0][y] = board[sizeX-1][y] = OUTSIDE;
    for (int pos= 1; pos<=12; pos++) set_piece(pos,BLACK);
    for (int pos=21; pos<=32; pos++) set_piece(pos,WHITE);
}

void print_board(void)
{
    for (int y=1; y<sizeY-1; y++) {
        for (int x=1; x<sizeX-1; x++)
            printf("%d",board[x][y]);
        printf("\n");
    }
}

bool piece_can_jump(int pos)
{
    int piece = get_piece(pos);
    int x = coordX(pos), y = coordY(pos);
    bool can_jump = false;
    for (int dir=0; dir<4; dir++) {
        int dx = directions[dir].dx, dy = directions[dir].dy;
        if (!is_king(piece) && dy!=forward_direction(color(piece)))
            continue; // les pions ne vont que vers l'avant
        int neighbor = board[x+dx][y+dy];
        if (are_opponents(piece, neighbor) && board[x+2*dx][y+2*dy]==EMPTY)
            can_jump = true;
    }
    return can_jump;
}

bool someone_can_jump(int color)
{
    for (int pos=1; pos<=32; pos++) 
        if (are_same_color(get_piece(pos),color) && piece_can_jump(pos))
            return true;
    return false;
}

/************************************************************************/

void do_jump(int x, int y, int dx, int dy)
{
    board[x+2*dx][y+2*dy] = board[x][y];
    board[x][y] = EMPTY;
    board[x+dx][y+dy] = EMPTY;
}

void undo_jump(int x, int y, int dx, int dy, int taken_piece)
{
    board[x][y] = board[x+2*dx][y+2*dy];
    board[x+2*dx][y+2*dy] = EMPTY;
    board[x+dx][y+dy] = taken_piece;
}

/*********************************************************************/
/* on mémorise la meilleure prise multiple ici :                     */

int best_move_positions[12];
int best_nb_jumps=0;

void memorize_best_jump(int positions[12], int nb_jumps)
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
/*********************************************************************/
/* Exemple de parcours de l'arbre des prises,                        */
/* ici avec une évaluation aléatoire                                 */

int best_jump(int positions[], int jump_num, int best_eval_so_far, int alpha, int beta)
{
    int x = coordX(positions[jump_num]), y = coordY(positions[jump_num]);
    int jumping_piece = board[x][y];
    bool can_jump = false;
    for (int dir=0; dir<4; dir++) {
        int dx = directions[dir].dx, dy = directions[dir].dy;
        if (!is_king(jumping_piece) && dy!=forward_direction(color(jumping_piece)))
            continue; // les pions ne vont que vers l'avant

        int jumped_piece = board[x+dx][y+dy];
        if (are_opponents(jumping_piece, jumped_piece) && board[x+2*dx][y+2*dy]==EMPTY) {
            can_jump = true;
            positions[jump_num+1] = std_position(x+2*dx,y+2*dy);
            do_jump(x, y, dx, dy);
            best_eval_so_far = best_jump(positions, jump_num+1, best_eval_so_far, alpha, beta);
            undo_jump(x, y, dx, dy, jumped_piece);
        }
    }
    if (!can_jump) { // feuille de l'arbre des prises : on évalue le coup
        int eval = rand(); // remplacez par votre évaluation (alphabeta ou autre) !
        if (eval > best_eval_so_far) {
            best_eval_so_far = eval;
            memorize_best_jump(positions, jump_num);
        }
    }
    return best_eval_so_far;
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
            int eval = rand(); // remplacez par votre évaluation
            if (eval > best_eval_so_far) {
                best_eval_so_far = eval;
                memorize_best_move(from_pos, std_position(x+dx,y+dy));
            }
        }
    }
    return best_eval_so_far;
}


/*********************************************************************/
/* Cherche un coup pour le programme, au hasard...                   */

void IA_play(void)
{
    int best_eval_so_far = -1;
    int alpha = INT_MIN, beta = INT_MAX;
    int move_positions[12];
    if (someone_can_jump(IA_color)) {
        for (int pos=1; pos<=32; pos++)
            if (are_same_color(get_piece(pos),IA_color) && piece_can_jump(pos)) {
                move_positions[0] = pos;
                best_eval_so_far = best_jump(move_positions, 0, best_eval_so_far, alpha, beta);
                if (best_eval_so_far > alpha) alpha = best_eval_so_far;
            }
    } else {
        for (int pos=1; pos<=32; pos++)
            if (are_same_color(get_piece(pos),IA_color)) {
                best_eval_so_far = best_move(pos, best_eval_so_far, alpha, beta);
                if (best_eval_so_far > alpha) alpha = best_eval_so_far;
            }
    }
}

/*********************************************************************/
/* Jouer le coup mémorisé dans best_move_positions                   */

void play_move(void)
{
    int from  = best_move_positions[0];
    int to    = best_move_positions[1];
    int piece = get_piece(from);
    set_piece(from,EMPTY);

    bool print_move = color(piece)==IA_color;
    if (print_move) printf("%d", from);
    if (best_nb_jumps == 0) {
        if (print_move) printf("-%d", to);
    } else {
        for (int i=0; i<best_nb_jumps; i++) {
            from  = best_move_positions[i];
            to    = best_move_positions[i+1];
            set_piece(between_position(from,to),EMPTY);
            if (print_move) printf("x%d",to); 
        }
    }
    set_piece(to,piece);
    if (print_move) { printf("\n"); fflush(stdout); }

    /* promotion en dame */
    if ((color(piece)==BLACK && to>28) || (color(piece)==WHITE && to<5))
        set_piece(to,promote_king(piece));
}

/*********************************************************************/
/* Lecture du coup adverse, dans best_stack                          */
/* Attention, je n'ai pas contrôlé le coup adverse pour faire court, */
/* l'arbitre s'en charge...                                          */

void read_move(void)
{
    int from, to;
    best_nb_jumps = 0;
    scanf("%d",&from);
    best_move_positions[0] = from;
    char move_or_jump = getchar();
    if (move_or_jump=='-') { // déplacement
        scanf("%d",&to);
        best_move_positions[1] = to;
    } else {                 // prise
        do {
            best_nb_jumps += 1;
            scanf("%d",&best_move_positions[best_nb_jumps]);
        } while (getchar()=='x');
    }
}


/*********************************************************************/
/* Déroulement de la partie                                          */

bool has_lost(int turn)
{
    for (int pos=1; pos<=32; pos++)
        if (color(get_piece(pos))==turn)
            return false;
    return true;
}

bool game_over(void) { return has_lost(WHITE) || has_lost(BLACK); }

int main(int argc, char *argv[])
{
    if (argc!=2) {
        fprintf(stderr,"Usage: random_play [1|2]\n");
        fprintf(stderr,"(1: le programme joue en premier avec les noirs,\n");
        fprintf(stderr," 2: le programme joue en 2nd avec les blancs)\n");
        exit(1);
    }
    srand(42);
    IA_color = strcmp(argv[1],"1")==0 ? BLACK : WHITE;
    init_board();
    int turn = BLACK;
    while (!game_over()) {
        if (turn==IA_color) IA_play();
        else read_move();
        play_move();
        turn = opponent_color(turn);
        int d; 
        fscanf(stderr, "%d", &d);
    }
    return 0;
}
