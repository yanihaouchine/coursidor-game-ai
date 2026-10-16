#include "yani_test.h"

struct player_t {
  const char* name;
  unsigned int id;
  struct graph_t* board;
  enum dir_t dir;
  int num_wall;
  vertex_t pos;
  int* objectives_reached; // array of booleans : objectives_reached[i] = 1 if board->objectives[i] have been reached and 0 otherwise.
  struct move_t mv;
};

unsigned int my_id;

struct player_t p[NUM_PLAYERS];

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
   struct state_t *st = create_root(eval, &p[my_id], &p[compute_next_player(my_id)]);
   
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
