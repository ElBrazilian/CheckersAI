#include "board.h"


int board[12][12];
int IA_color;
const struct direction directions[4] = {{+1,+1},{+1,-1},{-1,+1},{-1,-1}};

int color(int piece)             { return piece & COLOR_MASK; }
int opponent_color(int piece)    { return piece ^ COLOR_MASK; }
int are_opponents(int a, int b)  { return  (color(a)|color(b))==COLOR_MASK; }
int are_same_color(int a, int b) { return color(a) == color(b); }
int is_king(int piece)           { return (piece & KING) != 0; }
int is_eaten(int piece)          { return (piece & EATEN) != 0; }
int promote_king(int piece)      { return piece | KING; }
int forward_direction(int color) { return color==BLACK ? 1 : -1; }

int std_position(int x, int y)   { return (x-1)/2 + 5*(y-1) + 1; }
int coordY(int pos)              { return (pos-1)/5 + 1; }
int coordX(int pos)              { return (pos-1)%5*2 + 2 - (pos-1)/5%2; }

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
    for (int pos= 1; pos<=20; pos++) board[coordX(pos)][coordY(pos)] = BLACK;
    for (int pos=31; pos<=50; pos++) board[coordX(pos)][coordY(pos)] = WHITE;
}
void print_board(int tab[sizeX][sizeY])
{
    fprintf(stderr, "   0  1 2 3 4 5 6 7 8 9 10 11\n");
    for (int y=0; y<sizeY; y++) {
        fprintf(stderr, "%2d ", y);
        for (int x=0; x<sizeX; x++){
            if (tab[x][y] == EMPTY) fprintf(stderr, "  ");
            else fprintf(stderr, "%d ",tab[x][y]);
        }
        fprintf(stderr, "\n");
    }
    fflush(stderr);
}

#define PAWN_VALUE 100
#define KING_ADDITIONNAL_VALUE 500
#define DIST_FACTOR 5


int eval_board(int tab[sizeX][sizeY]){
    int res = 0;
    int ones = 0, twos = 0;
    for (int pos = 1; pos < 51; pos++){
        int x = coordX(pos), y = coordY(pos);
        int piece = tab[x][y];

        if (piece != EMPTY){
            int tmp_res = 0;
            if (color(piece) == IA_color){
                ones++;
                tmp_res += PAWN_VALUE;

                // distance au centre
                int x_dist = 0; 
                if (x < 5) x_dist = 5 - x;
                else x_dist = x - 6;

                tmp_res += DIST_FACTOR * x_dist;
                if (is_king(piece)) tmp_res += KING_ADDITIONNAL_VALUE;
            } else {
                twos++;
                tmp_res -= PAWN_VALUE;

                int x_dist = 0; 
                if (x < 5) x_dist = 5 - x;
                else x_dist = x - 6;

                tmp_res -= DIST_FACTOR * x_dist;
                if (is_king(piece)) tmp_res -= KING_ADDITIONNAL_VALUE;
            }
            res += tmp_res;
        }
    }
    // fprintf(stderr, "%d %d\n", ones, twos); fflush(stderr);
    if (ones == 0) return -WIN; // l'IA a perdu
    else if (twos == 0) return WIN;
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