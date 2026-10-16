#ifndef _BOARD_H_
#define _BOARD_H_
#include "dijkstra.h"
#include "tsp.h"
#include "graph.h"

/*
  Returns the graph of size m with type triangular
*/
struct graph_t* graph_make_triangular(size_t m);
struct graph_t* graph_make_circular(size_t m);
struct graph_t* graph_make_holey(size_t m);
/*
  Returns an empty graph with n_vertices vertices
*/
struct graph_t* graph_create(unsigned int n_vertices);

/*
  Returns direction if edge e exist in the graph g
  else returns 0
*/
enum dir_t graph_exist_edge(struct graph_t* g, struct edge_t e);

/*
  Remove edge e from graph g. Returns 0 if successful
  else positive integer
*/
int graph_remove_edge(struct graph_t* g, struct edge_t e);

/*
  Add edge e to graph g, d indicate the direction of edge e.
  Returns 0 if successful else positive integer
*/
int graph_add_edge(struct graph_t* g, vertex_t fr, vertex_t to, enum dir_t d);

/*
  Returns the neighbor's vertex of the given vertex in the direction d in the graph g
 */
// Améliorer graph_neighbor_direction pour obtenir une complexité en O(m) et plus en O(g -> num_vertices)
vertex_t graph_neighbor_direction(struct graph_t* g, vertex_t v, enum dir_t d);

/*
  Frees graph g
*/
void graph_free(struct graph_t** g);

/*
  Write in file name a dot file representing g. append == 0 creates a new file while append != 0 append to the existing file
*/
void graph_dot(struct graph_t* g, struct edge_t* removed, int removed_size, vertex_t black_player_pos, vertex_t white_player_pos, char* name, int append);

/*
  Copy the graph src in the graph dest
 */
void graph_cpy(struct graph_t* dest, struct graph_t* src);

void move_print(struct move_t m);

unsigned int graph_get_num_vertices(struct graph_t* g);

unsigned int graph_get_num_edges(struct graph_t* g);

unsigned int graph_get_num_objectives(struct graph_t* g);

#endif
