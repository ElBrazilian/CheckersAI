#include "moves.h"



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

// void generate_eaten_pos(Move *move){
//     move->positions[0] = move->from;
    
//     PossibleMoves possible_moves;
//     init_possible_moves(&possible_moves);
//     compute_moves_from_pos((IA_color == WHITE) ? BLACK : WHITE, board, move->from, &possible_moves);

//     for (int i = 0; i < possible_moves.current_move; i++){
//         if (possible_moves.all_possible_moves[i]->to == move->to && possible_moves.all_possible_moves[i]->from == move->from){
//             for (int j = 0; j < possible_moves.current_move; j++){
//                 move->positions[j] = possible_moves.all_possible_moves[i]->positions[j];
//                 move->jumps_pos[j] = possible_moves.all_possible_moves[i]->jumps_pos[j];
//             }
//             move->positions[possible_moves.current_move] = possible_moves.all_possible_moves[i]->positions[possible_moves.current_move];
//             move->num_jumps = possible_moves.all_possible_moves[i]->num_jumps;
//             break;
//         }
//     }

//     destroy_possible_moves(&possible_moves);
// }
void reset_possible_moves(int new_max_jumps, PossibleMoves *possible_moves){
    // for (int i = 0; i < possible_moves->current_move; i++){
    //     free(possible_moves->all_possible_moves[i]);
    // }
    possible_moves->current_move = 0;
    possible_moves->current_max_jumps = new_max_jumps;
}
void add_move_with_jumps(int jump_num, int positions[], int eaten[], PossibleMoves *possible_moves){
    if (jump_num > possible_moves->current_max_jumps){
        reset_possible_moves(jump_num, possible_moves);
    } 

    // possible_moves->all_possible_moves[possible_moves->current_move]                        = malloc(sizeof(Move));
    possible_moves->all_possible_moves[possible_moves->current_move].num_jumps             = jump_num;

    for (int i = 0; i < jump_num; i++){
        possible_moves->all_possible_moves[possible_moves->current_move].positions[i]      = positions[i];
        possible_moves->all_possible_moves[possible_moves->current_move].jumps_pos[i]      = eaten[i];
    }
    possible_moves->all_possible_moves[possible_moves->current_move].positions[jump_num]   = positions[jump_num];
    possible_moves->all_possible_moves[possible_moves->current_move].from                  = positions[0];
    possible_moves->all_possible_moves[possible_moves->current_move].to                    = positions[jump_num];

    possible_moves->current_move++;
}
void add_move_without_jumps(int from, int to, PossibleMoves *possible_moves){
    // possible_moves->all_possible_moves[possible_moves->current_move]            = malloc(sizeof(Move));
    possible_moves->all_possible_moves[possible_moves->current_move].num_jumps = 0;
    possible_moves->all_possible_moves[possible_moves->current_move].from      = from;
    possible_moves->all_possible_moves[possible_moves->current_move].to        = to;

    possible_moves->current_move++;
}
void rec_compute_jumps(int color, int tab[sizeX][sizeY], int jump_num, int positions[], int eaten[], PossibleMoves *possible_moves){
    int x = coordX(positions[jump_num]), y = coordY(positions[jump_num]);
    int jumping_piece = tab[x][y];
    bool can_jump = false;

    for (int dir=0; dir<4; dir++) {
        int dx = directions[dir].dx, dy = directions[dir].dy;

        if (is_king(jumping_piece)){
            // on commence par regarder si on peut "manger"
            int nx = x+dx, ny = y+dy;
            while ((1 <= nx && nx < sizeX-1 && 1 <= ny && ny < sizeY-1) && !are_opponents(jumping_piece, tab[nx][ny])){
                nx += dx;
                ny += dy;
            }

            // check if current one is actually opponent and if next one is empty
            if (are_opponents(jumping_piece, tab[nx][ny]) && !is_eaten(tab[nx][ny])){
                if (tab[nx + dx][ny + dy] == EMPTY){
                    can_jump = true;

                    int ex = nx, ey = ny; // position of the eaten piece

                    eaten[jump_num] = std_position(ex, ey);
                    tab[ex][ey] |= EATEN; 
                    tab[x][y] = EMPTY;

                    nx += dx;
                    ny += dy;
                    while ((1 <= nx && nx < sizeX-1 && 1 <= ny && ny < sizeY-1) && tab[nx][ny] == EMPTY) {
                        // do jump
                        tab[nx][ny] = jumping_piece;
                        positions[jump_num+1] = std_position(nx, ny);

                        rec_compute_jumps(color, tab, jump_num+1, positions, eaten, possible_moves);

                        // Undo jump
                        tab[nx][ny] = EMPTY;

                        nx += dx;
                        ny += dy;
                    }
                    tab[ex][ey] = opponent_color(jumping_piece);
                    tab[x][y] = jumping_piece;
                }
            }

        } else {
            int jumped_piece = tab[x+dx][y+dy];
            int nnx = x+2*dx, nny = y + 2 * dy;
            if ((0 <= nnx && nnx < sizeX && 0 <= nny && nny <= sizeY) && are_opponents(jumping_piece, jumped_piece) && tab[x+2*dx][y+2*dy]==EMPTY) {
                can_jump = true;
                positions[jump_num+1] = std_position(nnx, nny);
                eaten[jump_num] = std_position(x+dx, y+dy);
                
                // do piece jump
                tab[nnx][nny] = tab[x][y];
                tab[x+dx][y+dy] = EMPTY;
                tab[x][y] = EMPTY;

                rec_compute_jumps(color, tab, jump_num+1, positions, eaten, possible_moves);

                // undo piece jupmp
                tab[nnx][nny] = EMPTY;
                tab[x+dx][y+dy] = jumped_piece;
                tab[x][y] = jumping_piece;
            }
        }
    }
    if (!can_jump && jump_num > 0) { // feuille de l'arbre des prises : on évalue le coup
        // si c'est un nombre de prises égal à avant: on ajoute, sinon on clear et on recommence
        if (jump_num >= possible_moves->current_max_jumps){
            add_move_with_jumps(jump_num, positions, eaten, possible_moves);
        }
    }
}
void compute_possible_moves_from_pos(int c, int tab[sizeX][sizeY], int from_pos, PossibleMoves *possible_moves){
    int x = coordX(from_pos), y = coordY(from_pos);
    int piece = tab[x][y];

    for (int dir=0; dir<4; dir++) {
        int dx = directions[dir].dx, dy = directions[dir].dy;
        int nx = x + dx, ny = y + dy;

        if (is_king(piece)){
            // si c'est une dame
            while ((0 <= nx && nx < sizeX && 0 <= ny && ny < sizeY) && tab[nx][ny] == EMPTY){
                add_move_without_jumps(std_position(x, y), std_position(nx, ny), possible_moves);
                
                nx += dx;
                ny += dy;
            }
        } else if (dy == forward_direction(color(piece))){
            if (tab[nx][ny] == EMPTY) {
                add_move_without_jumps(std_position(x, y), std_position(nx, ny), possible_moves);
            }
        }
    }
}
void compute_moves_from_pos(int color, int tab[sizeX][sizeY], int start_pos, PossibleMoves *possible_moves){
    int positions[MAX_JUMPS+1];
    int eaten[MAX_JUMPS];

    positions[0] = start_pos;
    rec_compute_jumps(color, tab, 0, positions, eaten, possible_moves);

    if (possible_moves->current_max_jumps == 0) compute_possible_moves_from_pos(color, tab, start_pos, possible_moves);
}

void compute_possible_moves(int current_turn_color, int tab[sizeX][sizeY], PossibleMoves *possible_moves){
    reset_possible_moves(0, possible_moves);
    
    for (int pos = 1; pos <= 50; pos++){
        int x = coordX(pos), y = coordY(pos);
        if (color(tab[x][y]) == current_turn_color){
            compute_moves_from_pos(current_turn_color, tab, pos, possible_moves);
        }
    }
    
    // for (int i = 0; i < current_move; i++){
    //     send_move(*(all_possible_moves[i]));
    // }

    // fprintf(stderr, "\nMax jumps: %d | total moves : %d\n", possible_moves->current_max_jumps, possible_moves->current_move); fflush(stderr);
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
        tab[coordX(move.to)][coordY(move.to)] = promote_king(tab[coordX(move.to)][coordY(move.to)]);
}
void init_possible_moves(PossibleMoves *possible_moves){
    // possible_moves->all_possible_moves = malloc(MAX_POSSIBLE_MOVES * sizeof(Move *));
    // for (int i = 0; i < MAX_POSSIBLE_MOVES; i++){
    //     possible_moves->all_possible_moves[i] = NULL;
    // }
    possible_moves->current_move = 0;
    possible_moves->current_max_jumps = 0;
}
void destroy_possible_moves(PossibleMoves *possible_moves){
    return; // TOREMOVE
    // for (int i = 0; i < possible_moves->current_move; i++){
    //     free(possible_moves->all_possible_moves[i]);
    // }
    // free(possible_moves->all_possible_moves);
}