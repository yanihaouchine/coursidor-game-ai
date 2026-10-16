#include "state.h"
struct state_t {
  int (*evaluation)(struct state_t*);
  struct i_player_t* i_p_current;
  struct i_player_t* i_p_adversary;
  int nb_children;
};

struct state_t* create_root(int (*evaluation)(struct state_t*),  void* i_p_current, void *i_p_adversary)
{
  struct state_t* st = malloc(sizeof(*st));
  st->evaluation = evaluation;
  st->i_p_current = create_i_player(i_p_current);
  st->i_p_adversary = create_i_player(i_p_adversary);
  st->nb_children = 0;
  return st;
}


struct state_t* create_state(int (*evaluation)(struct state_t*),  void* i_p_current, void *i_p_adversary)
{
  struct state_t* st = malloc(sizeof(*st));
  st->evaluation = evaluation;
  st->i_p_current = create_i_player(i_p_current);
  st->i_p_adversary = create_i_player(i_p_adversary);
  st->nb_children = 0;
  return st;
}


/* struct state_t* create_state(int (*evaluation)(struct state_t*),  void* i_p_current, void *i_p_adversary) */
/* { */
/*   struct state_t* st = malloc(sizeof(*st)); */
/*   st->evaluation = evaluation; */
/*   st->i_p_current = (struct i_player_t *)(i_p_current); */
/*   st->i_p_adversary = (struct i_player_t*)(i_p_adversary); */
/*   st->nb_children = 0; */
/*   return st; */
/* } */


void free_state(struct state_t* st)
{
  free_i_player(st->i_p_current);
  free_i_player(st->i_p_adversary);
  free(st);
}

void modif_eval_process(struct state_t* st, int (*new_evaluation)(struct state_t*))
{
  st->evaluation = new_evaluation;
}

int is_terminal(struct state_t* st)
{
  return is_winner(st->i_p_current);
}

int evaluation_state(struct state_t* st)
{
  return get_id_state(st) ? -1*st->evaluation(st) : st -> evaluation(st);
}

struct state_t* new_state(struct state_t* st, struct move_t* mv)
{
  struct i_player_t *new_p_adv; 
  if(mv -> t == WALL)
    new_p_adv = if_I_play(st -> i_p_adversary, mv);
  else
    new_p_adv = create_i_player(st -> i_p_adversary);
  struct i_player_t *new_i_p_current = if_I_play(st->i_p_current, mv);
  struct state_t *new_st = create_state(st -> evaluation, new_i_p_current, new_p_adv);
  free_i_player(new_p_adv);
  free_i_player(new_i_p_current);
  return new_st;
}

struct state_t** get_children(struct state_t* st, struct move_t** (*get_next_moves)(struct state_t* s, int* number_of_moves))
{
    int num_moves;
    struct move_t** next_moves = get_next_moves(st, &num_moves);
    struct state_t** children = malloc(sizeof(struct state_t*) * num_moves);
    for (int i = 0; i < num_moves; i++){
        children[i] = new_state(st, next_moves[i]);
	free(next_moves[i]);
	switch_player(children[i]);
    }
    free(next_moves);
    st->nb_children = num_moves;
    return children;
}

int get_nb_children(struct state_t* st)
{
    return st->nb_children;
}

struct graph_t *get_board_state(struct state_t *st)
{
  return get_board_i_player(st -> i_p_current);
}

vertex_t get_pos_state(struct state_t *st)
{
  return get_pos_i_player(st -> i_p_current);
}

unsigned int get_id_state(struct state_t *st)
{
  return get_id_i_player(st -> i_p_current);
}

enum dir_t get_last_dir_state(struct state_t *st)
{
  return get_last_dir_i_p(st -> i_p_current);
}

int *get_obj_state(struct state_t *st)
{
  return get_obj_i_p(st -> i_p_current);
}

void switch_player(struct state_t *st)
{
  struct i_player_t* tmp = st -> i_p_current;
  st -> i_p_current = st -> i_p_adversary;
  st -> i_p_adversary = tmp;
}

int get_num_wall_st(struct state_t *st)
{
  return get_num_wall_i_p(st -> i_p_current);
}

struct move_t get_last_move(struct state_t *st)
{
  return  get_last_move_i_p(st -> i_p_current);
}
