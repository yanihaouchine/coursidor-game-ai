#include "choosen_move.h"


int good_connexity(struct graph_t *g, vertex_t start_1, vertex_t start_2, int *obj_r1, int *obj_r2)
{
  if((bfs(g, start_1, g -> start[0]) == -1 || bfs(g , start_2, g -> start[1]) == -1) )
    return 0;
  for(unsigned int i = 0; i < g -> num_objectives; i++){
    if((!obj_r1[i] && (bfs(g, start_1, g -> objectives[i]) == - 1)) || (!obj_r2[i] && (bfs(g, start_2, g -> objectives[i]) == - 1)) ) return 0;
  }
  return 1;
}

int is_possible_w_moves(struct state_t *st, struct edge_t e[2])
{
  for(int i = 0; i < 2; i++)
    if(e[i].fr >= get_board_state(st) -> num_vertices || e[i].to >= get_board_state(st) -> num_vertices)
      return 0;
  vertex_t pos1 = get_pos_state(st);
  int *obj_r1 = get_obj_state(st);
  switch_player(st);
  vertex_t pos2 = get_pos_state(st);
  int *obj_r2 = get_obj_state(st);
  switch_player(st);
  struct graph_t *g = malloc(sizeof(*g));
  graph_cpy(g, get_board_state(st));
  graph_remove_edge(g, e[0]);
  graph_remove_edge(g, e[1]);
  int res = good_connexity(g, pos1, pos2, obj_r1, obj_r2);
  graph_free(&g);
  return res;
}




struct move_t *init_move(enum player_color_t c, enum move_type_t t, vertex_t m, struct edge_t e[2])
{
  struct move_t *mv = malloc(sizeof(*mv));
  mv -> c = c;
  mv -> t = t;
  mv -> m = m;
  mv -> e[0] = e[0];
  mv -> e[1] = e[1];
  return mv;
}


struct move_t **get_all_possible_moves(struct state_t *st, int *num_moves)
{
  vertex_t n = get_board_state(st) -> num_vertices;
  *num_moves = 0;
  struct edge_t e[2];
  e[1] = (struct edge_t){n, n};
  e[0] = e[1];
  int nb_s = 0;
  vertex_t neighbors[20];
  get_voisins(get_board_state(st), get_pos_state(st), get_last_dir_state(st),neighbors, &nb_s);
  switch_player(st);
  int *obj = get_obj_state(st);
  unsigned int num_obj = get_board_state(st) -> num_objectives;
  unsigned int count = 0;
  for(unsigned int i = 0; i < num_obj; i++)
    count += obj[i];
  int wall = ( count > (num_obj / 2) ) ? 1 : 0;
  if(wall)
    {
      for(vertex_t i = 0; i < n; i++)
	{
	  for( enum dir_t d = FIRST_DIR; d <= LAST_DIR; d++){
	    vertex_t j1 = graph_neighbor_direction(get_board_state(st), i, d);
	    enum dir_t next = d + 1;
	    if(d == LAST_DIR)
	      next = FIRST_DIR;
	    vertex_t j2 = graph_neighbor_direction(get_board_state(st), i, next);
	    struct edge_t e1 = {i,j1};
	    struct edge_t e2 = {i,j2};
	    e[0] = e1;
	    e[1] = e2;
	    if(is_possible_w_moves(st, e)) ++(*num_moves);
	  }
	}
    }
  struct move_t **moves = malloc(sizeof(struct move_t *)*(*num_moves + nb_s));
  int r = 0;
  if(wall)
    {
      for(vertex_t i = 0; i < n; i++)
	{
	  for( enum dir_t d = FIRST_DIR; d <= LAST_DIR; d++){
	    vertex_t j1 = graph_neighbor_direction(get_board_state(st), i, d);
	    enum dir_t next = d + 1;
	    if(d == LAST_DIR)
	      next = FIRST_DIR;
	    vertex_t j2 = graph_neighbor_direction(get_board_state(st), i, next);
	    struct edge_t e1 = {i,j1};
	    struct edge_t e2 = {i,j2};
	    e[0] = e1;
	    e[1] = e2;
	    if(is_possible_w_moves(st, e)){
	      e[0] = e1;
	      e[1] = e2;
	      moves[r] = init_move(get_id_state(st) , WALL, n, e);
	      ++r;
	    }
	  }
	}
    }
  for(int i = 0; i < nb_s; i++)
    moves[*num_moves + i] = init_move(get_id_state(st) , MOVE, neighbors[i], e);
  *num_moves += nb_s;
  return moves;
}

