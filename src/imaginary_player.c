#include "imaginary_player.h"

struct i_player_t {
  char* i_name;
  unsigned int i_id;
  struct graph_t* i_board;
  enum dir_t i_dir;
  int i_num_wall;
  vertex_t i_pos;
  int* i_objectives_reached; // array of booleans : objectives_reached[i] = 1 if board->objectives[i] have been reached and 0 otherwise.
  struct move_t i_mv;
};

struct i_player_t* create_i_player(void* realplayer)
{
  struct i_player_t *real_player = (struct i_player_t*)realplayer;
  struct i_player_t* i_p = malloc(sizeof(*i_p));
  i_p->i_id = real_player->i_id;
  i_p->i_board = graph_create(graph_get_num_vertices(real_player->i_board));
  graph_cpy(i_p->i_board, real_player->i_board);
  i_p->i_dir = real_player->i_dir;
  i_p->i_num_wall = real_player->i_num_wall;
  i_p->i_pos = real_player->i_pos;
  unsigned int num_objectives = graph_get_num_objectives(real_player->i_board);
  i_p->i_objectives_reached = malloc(sizeof(int) * num_objectives);
  for (unsigned int k = 0; k < num_objectives; k++)
    i_p->i_objectives_reached[k] = real_player->i_objectives_reached[k];
  i_p -> i_mv =  real_player -> i_mv;
  return i_p;
}

int is_winner(struct i_player_t* i_p)
{
    unsigned int num_objectives = graph_get_num_objectives(i_p->i_board);
    if (i_p->i_pos != i_p->i_board->start[i_p->i_id])
        return 0;
    for (unsigned int k = 0; k < num_objectives; k++)
        if (i_p->i_objectives_reached[k] == 0)
            return 0;
    return 1;
}

void free_i_player(struct i_player_t* i_p)
{
   
    graph_free(&(i_p->i_board));
    free(i_p->i_objectives_reached);
    free(i_p);
}

struct i_player_t* if_I_play(struct i_player_t* i_p, struct move_t* mv)
{
  struct i_player_t* new_i_p = create_i_player(i_p);
  if (mv->t == MOVE) {
    new_i_p->i_dir = graph_exist_edge(new_i_p->i_board,(struct edge_t){new_i_p->i_pos,mv->m});
    new_i_p->i_pos = mv->m;
  }
  if (mv->t == WALL) {
    graph_remove_edge(new_i_p->i_board, mv->e[0]);
    graph_remove_edge(new_i_p->i_board, mv->e[1]);
    if(mv -> c == i_p -> i_id) new_i_p->i_num_wall--;
  }
  assert(mv -> t != NO_TYPE);
  new_i_p->i_mv = *mv;
  return new_i_p;
}
struct graph_t *get_board_i_player(struct i_player_t *i_p)
{
  return i_p -> i_board;
}
vertex_t get_pos_i_player(struct i_player_t *i_p)
{
  return i_p -> i_pos;
}
unsigned int get_id_i_player(struct i_player_t *i_p)
{
  return i_p -> i_id;
}

enum dir_t get_last_dir_i_p(struct i_player_t *i_p)
{
  return i_p -> i_dir;
}

int *get_obj_i_p(struct i_player_t *i_p)
{
  return i_p -> i_objectives_reached;
}

int get_num_wall_i_p(struct i_player_t *i_p)
{
  return i_p -> i_num_wall;
}

struct move_t get_last_move_i_p(struct i_player_t *i_p)
{
  return i_p -> i_mv;
}
