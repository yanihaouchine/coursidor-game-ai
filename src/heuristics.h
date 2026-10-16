#include "graph.h"
#include "move.h"

struct move_t naive_move(struct graph_t* board, vertex_t pos, unsigned int color, enum dir_t* dir, struct move_t previous_move);
