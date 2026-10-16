#include "opt.h"
#include <limits.h>



node_t tree[10000000];





unsigned int my_id;


char const* get_player_name()
{
    return "Yani";
}

void initialize(unsigned int player_id, struct graph_t* graph)
{
  vertex_t n = graph -> num_vertices;
    assert(player_id == 0 || player_id == 1);
    char opponent_name[10] = "Opponent1";
    int n_wall = (graph->num_edges / 16) + ((graph->num_edges % 16) > 0);
    for (int i = 0; i < NUM_PLAYERS; i++) {
        opponent_name[8] = '0' + i;
        p[i].name = opponent_name;
        p[i].id = i;
        p[i].board = graph;
        p[i].dir = NO_EDGE;
        p[i].num_wall = n_wall;
        p[i].pos = graph->start[i];
        p[i].objectives_reached = calloc(graph->num_objectives, sizeof(int));
	p[i].mv.c = i;
	p[i].mv.t = NO_TYPE;
	p[i].mv.m = n;
	p[i].mv.e[0] = (struct edge_t){n,n};
	p[i].mv.e[1] = (struct edge_t){n,n};
    }
    p[player_id].name = get_player_name();
    my_id = player_id;
}

int compute_next_player(int current_player)
{
    return (current_player + 1) % NUM_PLAYERS;
}

int eval(struct state_t *st)
{
  size_t distance_0;
  size_t distance_1;
  tsp(get_board_state(st),get_pos_state(st) , get_last_dir_state(st), get_board_state(st)->start[get_id_state(st)], get_obj_state(st), &distance_0);
  switch_player(st);
   tsp(get_board_state(st),get_pos_state(st) , get_last_dir_state(st), get_board_state(st)->start[get_id_state(st)], get_obj_state(st), &distance_1);
   switch_player(st);
   return (int)distance_0 - (int)distance_1;
}





      

struct move_t play(const struct move_t previous_move)
{
   if (previous_move.t == WALL) {
        graph_remove_edge(p[0].board, previous_move.e[0]);
        graph_remove_edge(p[0].board, previous_move.e[1]);
        p[previous_move.c].num_wall = p[previous_move.c].num_wall - 1;
    } else if (previous_move.t == MOVE) {
        p[previous_move.c].pos = previous_move.m;
        for (unsigned int i = 0; i < p[0].board->num_objectives; i++) {
            if (previous_move.m == p[0].board->objectives[i])
                p[previous_move.c].objectives_reached[i] = 1;
        }
    }
   p[previous_move.c].mv = previous_move;
   struct state_t *st = create_state(eval, &p[my_id], &p[compute_next_player(my_id)]);
   
   struct move_t mv = minimax(st, 1, INT_MIN, INT_MAX, my_id, previous_move).mv;
   if(mv.t == NO_TYPE)
      mv = minimax(st, 2, INT_MIN, INT_MAX, my_id, previous_move).mv;
   free_state(st);
   move_print(mv);
   return mv;
}
void finalize()
{
    graph_free(&(p[0].board));
    for (int i = 0; i < NUM_PLAYERS; i++) {
        free(p[i].objectives_reached);
    }
    return;
}






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

typedef struct state{
  enum color_t id;
  int v;
  struct graph_t *g;
  int obj[2];
  struct move_t mv;
  vertex_t last_pos[2];
  int nb_c;
  enum dir_t prev_dir[2];
  int nb_w;
  int idx_parent;
  int idx_first_child;
  int idx_child[1500];
}node_t;


int good_connexity(struct graph_t *g, vertex_t start_1, vertex_t start_2, int obj_r1, int obj_r2)
{
  if((bfs(g, start_1, g -> start[0]) == -1 || bfs(g , start_2, g -> start[1]) == -1) )
    return 0;
  for(unsigned int i = 0; i < g -> num_objectives; i++){
    if((!(obj_r1 & (1 << i)) && (bfs(g, start_1, g -> objectives[i]) == - 1)) || (!((obj_r2) & 1 << i) && (bfs(g, start_2, g -> objectives[i]) == - 1)) ) return 0;
  }
  return 1;
}
int is_possible_w_moves(node_t n, struct edge_t e[2])
{
  for(int i = 0; i < 2; i++)
    if(e[i].fr >= n.g -> num_vertices || e[i].to >= n.g -> num_vertices)
      return 0;
  vertex_t pos1 = n.last_pos[id];
  int obj_r1 = n.obj[id];
  vertex_t pos2 = n.last_pos[1 - id]; 
  int obj_r2 = n.pos[1 - id];
  graph_remove_edge(n.g, e[0]);
  graph_remove_edge(n.g, e[1]);
  int res = good_connexity(g, pos1, pos2, obj_r1, obj_r2);
  graph_add_edge(n.g, e[0]);
  graph_add_edge(n.g, e[1]);
  return res;
}


node_t create_root( enum color_t id;
		     struct graph_t *g;
		     int nb_w;)
{
  node_t r = malloc(sizeof(*r));
  r.id = id;
  r.idx = id;
  r.v = id ? INT_MIN : INT_MAX;
  r.g = malloc(sizeof(*(r.g)));
  graph_cpy(r.g, g);
  r.obj = 0;
  r.last_pos = g ->start[id];
  r.mv = {.c = id, .t = MOVE ,.m = g -> start[id]};
  r.mv.e[0] = g -> num_vertices;
  r.mv.e[1] = g -> num_vertices;
  r.nb_c = 0;
  r.prev_dir = NO_EDGE;
  r.nb_w = nb_w;
  r.idx_parent = r.idx_first_child = -1;
  r.idx_child = {-1};
  return r;
}

node_t create_node(node_t n)
{
  struct new_n
  new_n.id = 1 - n.id;
  new_n.v = id ? INT_MIN : INT_MAX;
  new_n.g = malloc(sizeof(*(n.g)));
  graph_copy(new_n.g, n.g);
  new_n.last_pos[new_n.id] = n.last_pos[new_n.id];
  new_n.last_pos[n.id] = n.last_pos[id];
  new_n.obj[n.id] = n.obj[n.id];
  new_n.obj[new_n.id] = new_n.obj[new_n.obj];
  new_n.mv.c = get_parent(n).c;
  new_n.mv.t = get_parent(n).mv.t;
  new_n.mv.m = get_parent(n).mv.t;
  new_n.mv.e[0] = n.mv.e[0];
  new_n.mv.e[1] = n.mv.e[1];
  new_n.nb_c = 0;
  new_n.prev_dir[n.id] = n.prev_dir[n.id];
  new_n.prev_dir[new_n.id] = n.prev_dir[new_n.id];
  new_n.nb_w = get_parent(n).nb_w;
  new_n.idx = get_idx(n);
  new_n.idx_parent = n.idx;
  new_n.idx_first_child = -1;
  new_n.idx_child = {-1};
}

void create_children(node_t n)
{
  vertex_t num_v = n.g -> num_vertices;
  struct edge_t e[2];
  e[1] = (struct edge_t){num_v, num_v};
  e[0] = e[1];
  vertex_t neighbors[20];
  int nb_s = 0;
  enum_dir_t dir[20];
  get_voisins(n.g, n.mv.m, n.prev_dir[n.id], neighbors, dir, &nb_s);
  for(vertex_t i = 0; i < n; i++)
    {
      for( enum dir_t d = FIRST_DIR; d <= LAST_DIR; d++){
	vertex_t j1 = graph_neighbor_direction(n.g, i, d);
	enum dir_t next = d + 1;
	if(d == LAST_DIR)
	  next = FIRST_DIR;
	vertex_t j2 = graph_neighbor_direction(get_board_state(st), i, next);
	struct edge_t e1 = {i,j1};
	struct edge_t e2 = {i,j2};
	e[0] = e1;
	e[1] = e2;
	if(is_possible_w_moves(st, e)){
	  new_n = create_node(n);
	  new_n.mv.t = WALL;
	  new_n.mv.e[0] = e[0];
	  new_n.mv.e[1] = e[1];
	  --new_n.nb_w;
	  graph_remove_edge(new_n.n, e[0]);
	  graph_remove_edge(new_n.n, e[1]);
	  tree[new_n.idx] = new_n;
      }
    }
    }
  for(int i = 0; i < nb_s; i++){
    new_n = create_node(n);
    new_n.mv.last_pos[n.id] = neighbors[i];
    new_n.prev_dir[n.id] = dir[i];
    new_n.mv.m = neighbors[i];
    new_n.mv.t = MOVE;
    tree[new_n.idx] = new_n;
  }
}
