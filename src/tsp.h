#include "graph.h"
#include <stdio.h>


vertex_t *tsp(struct graph_t *g, vertex_t start, enum dir_t last_dir, vertex_t end, int *objectif_reached, size_t *distance);


vertex_t *get_best_start(struct graph_t *g, enum dir_t last_dir, vertex_t end, int *objectif_reached, size_t *distance, vertex_t *start_possibility, size_t size_of_start_possible);


struct automate_state_t;


void get_A_neighbors(struct graph_t* g, struct automate_state_t q, struct automate_state_t* neighbors, int* nb_s);


void get_voisins(struct graph_t* g, vertex_t pos, enum dir_t last_dir, vertex_t* neighbors, enum dir_t *dir, int* nb_s);
