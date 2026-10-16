
#include "imaginary_player.h"

struct state_t;
struct state_t* create_root(int (*evaluation)(struct state_t*),  void* i_p_current, void *i_p_adversary);

struct state_t* create_state(int (*evaluation)(struct state_t*),  void* i_p_current, void *i_p_adversary);


void free_state(struct state_t* st);

void modif_eval_process(struct state_t* st, int (*new_evaluation)(struct state_t*));

int is_terminal(struct state_t* st);

int evaluation_state(struct state_t* st);

// La fonction next_moves donne tout les prochains move qu'il faut voir, au début on suppose qu'il ne faut pas mettre de mur
// next_moves[0] représente le nombre de move dasn le tableau next_moves
struct state_t** get_children(struct state_t* st, struct move_t** (*next_moves)(struct state_t* s, int *number_of_moves));

int get_nb_children(struct state_t* st);

struct graph_t *get_board_state(struct state_t *st);

vertex_t get_pos_state(struct state_t *st);

unsigned int get_id_state(struct state_t *st);

enum dir_t get_last_dir_state(struct state_t *st);

int *get_obj_state(struct state_t *st);


void switch_player(struct state_t *st);

int get_num_wall_st(struct state_t *st);


struct move_t get_last_move(struct state_t *st);
