#include "opt.h"
#include <limits.h>
int max(int x, int y)
{
    return (x < y) ? y : x;
}
int min(int x, int y)
{
    return (x < y) ? x : y;
}


struct coup_t minimax(state* st, int depth, int alpha, int beta, int player, struct move_t mv)
{
  struct coup_t meilleur_coup;
  assert(player == 0 || player == 1);
  
  if (is_terminal(st) || depth == 0)
    return (struct coup_t){evaluation_state(st), get_last_move(st)};
   
  if (player == 1) {  // Maximizing player
    meilleur_coup.value = INT_MIN;
    
    struct state_t* children = get_children(st, get_all_possible_moves);
    int nb_children = get_nb_children(st);
    
    for (int i = 0; i < nb_children; i++) {
      struct coup_t coup = minimax(children[i], depth - 1, alpha, beta, 1 - player, mv);
      
      if (coup.value > meilleur_coup.value) {
        meilleur_coup.value = coup.value;
        meilleur_coup.mv = get_last_move(children[i]); // Store the direct child move, not the grandchild move
      }
      
      alpha = (alpha > coup.value) ? alpha : coup.value;
      if (beta <= alpha)
        break;  // Alpha-beta pruning
    }
    
    for (int i = 0; i < nb_children; i++) {
      free_state(children[i]);
    }
    free(children);
    
    return meilleur_coup;
  }
  else {  // Minimizing player (player == 0)
    meilleur_coup.value = INT_MAX;
    
    struct state_t** children = get_children(st, get_all_possible_moves);
    int nb_children = get_nb_children(st);
    
    for (int i = 0; i < nb_children; i++) {
      struct coup_t coup = minimax(children[i], depth - 1, alpha, beta, 1 - player, mv);
      
      if (coup.value < meilleur_coup.value) {
        meilleur_coup.value = coup.value;
        meilleur_coup.mv = get_last_move(children[i]); // Store the direct child move, not the grandchild move
      }
      
      beta = (beta < coup.value) ? beta : coup.value;
      if (beta <= alpha)
        break;  // Alpha-beta pruning
    }
    
    for (int i = 0; i < nb_children; i++) {
      free_state(children[i]);
    }
    free(children);
    
    return meilleur_coup;
  }
}
