#include "opt.h"
#include <limits.h>

void initialize(unsigned int player_id, struct graph_t* graph);
struct move_t play(const struct move_t previous_move);
