#include "choosen_move.h"

// Au premier appel mettre beta à - 1 et alpha à -2
// player == 1 => appel à max, c'est nous qui jouons
// player == 0 => appel à min, c'est le joueur adverse qui joue
struct coup_t minimax(struct state_t* st, int depth, int alpha, int beta, int player, struct move_t mv);
struct coup_t{
  int value;
  struct move_t mv;
};
