#include "heuristics.h"
#include "graph.h"
#include "move.h"

struct vertex_t move_three_times_direction(struct graph_t* g, vertex_t pos,
    enum dir_t d);

struct move_t
naive_move(struct graph_t* board, vertex_t pos, unsigned int color,
    enum dir_t* dir, struct move_t previous_move)
{
    struct move_t new_move;
    new_move.c = color;
    new_move.t = MOVE;
    if (gsl_spmatrix_uint_get((board->t), pos, pos + 1) == E
        && previous_move.m != pos + 1)
        new_move.m = pos + 1;
    else if (previous_move.m != pos - 1)
        new_move.m = pos - 1;
    else
        new_move.m = pos;
    *dir = E;

    return new_move;
}
