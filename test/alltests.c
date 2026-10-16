#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"

int bfs(struct graph_t* graph, vertex_t start, vertex_t end);

void _exit_if_error(void* p)
{
    char* error;
    if ((error = dlerror()) != NULL) {
        fputs(error, stderr);
        dlclose(p);
        exit(1);
    }
}

// Returns a function obtained with dlsym and tests if there is an error
void* _get_func(void* p, char* s)
{
    char* (*func)(void) = dlsym(p, s);
    _exit_if_error(p);
    return func;
}

void test_client_tristan()
{
    printf("%s", __func__);

    struct graph_t* g = malloc(sizeof(struct graph_t));
    g->type = TRIANGULAR;
    g->num_vertices = 7;
    g->num_edges = 12;
    g->t = gsl_spmatrix_uint_alloc(g->num_vertices, g->num_vertices);
    g->start[0] = 0;
    g->start[1] = g->num_vertices - 1;
    g->num_objectives = 2;
    g->objectives = malloc(g->num_objectives * sizeof(vertex_t));
    g->objectives[0] = 2;
    g->objectives[1] = 4;

    gsl_spmatrix_uint_set(g->t, 0, 1, E);
    gsl_spmatrix_uint_set(g->t, 0, 2, SW);
    gsl_spmatrix_uint_set(g->t, 0, 3, SE);

    gsl_spmatrix_uint_set(g->t, 1, 0, W);
    gsl_spmatrix_uint_set(g->t, 1, 3, SW);
    gsl_spmatrix_uint_set(g->t, 1, 4, SE);

    gsl_spmatrix_uint_set(g->t, 2, 0, NE);
    gsl_spmatrix_uint_set(g->t, 2, 3, E);
    gsl_spmatrix_uint_set(g->t, 2, 5, SE);

    gsl_spmatrix_uint_set(g->t, 3, 0, NW);
    gsl_spmatrix_uint_set(g->t, 3, 1, NE);
    gsl_spmatrix_uint_set(g->t, 3, 2, W);
    gsl_spmatrix_uint_set(g->t, 3, 4, E);
    gsl_spmatrix_uint_set(g->t, 3, 5, SW);
    gsl_spmatrix_uint_set(g->t, 3, 6, SE);

    gsl_spmatrix_uint_set(g->t, 4, 1, NW);
    gsl_spmatrix_uint_set(g->t, 4, 3, W);
    gsl_spmatrix_uint_set(g->t, 4, 6, SW);

    gsl_spmatrix_uint_set(g->t, 5, 2, NW);
    gsl_spmatrix_uint_set(g->t, 5, 3, NE);
    gsl_spmatrix_uint_set(g->t, 5, 6, E);

    gsl_spmatrix_uint_set(g->t, 6, 3, NW);
    gsl_spmatrix_uint_set(g->t, 6, 4, NE);
    gsl_spmatrix_uint_set(g->t, 6, 5, W);

    gsl_spmatrix_uint* coo = g->t;
    g->t = gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
    gsl_spmatrix_uint_free(coo);
    struct graph_t* g_B = malloc(sizeof(struct graph_t));
    graph_cpy(g_B, g);

    vertex_t pos_B = g->start[0];
    vertex_t pos_W = g->start[1];

    void* player_B = dlopen("install/tristan.so", RTLD_LAZY);
    if (!player_B) {
        fputs(dlerror(), stderr);
        exit(1);
    }

    void (*initialize_B)(unsigned int, struct graph_t*);
    initialize_B = _get_func(player_B, "initialize");
    struct move_t (*play_B)(const struct move_t);
    play_B = _get_func(player_B, "play");
    void (*finalize_B)();
    finalize_B = _get_func(player_B, "finalize");

    initialize_B(BLACK, g_B);

    struct move_t move;
    move.c = WHITE;
    move.t = NO_TYPE;
    move.m = g->num_vertices;
    move.e[0] = (struct edge_t) { g->num_vertices + 1, g->num_vertices + 2 };
    move.e[1] = (struct edge_t) { g->num_vertices + 3, g->num_vertices + 4 };

    struct edge_t removed[g->num_edges];
    int removed_size = 0;
    char name[18] = "test_tristan0.dot";
    graph_dot(g, removed, removed_size, pos_B, pos_W, name, 0);
    for (int i = 0; i < 9; i++) {
        move.c = WHITE;
        move = play_B(move);
        if (move.t == MOVE)
            pos_B = move.m;
        else if (move.t == WALL) {
            graph_remove_edge(g, move.e[0]);
            graph_remove_edge(g, move.e[1]);
            removed[removed_size] = move.e[0];
            removed[removed_size + 1] = move.e[1];
            removed_size = removed_size + 2;
        }
        name[12] = '0' + i + 1;
        graph_dot(g, removed, removed_size, pos_B, pos_W, name, 0);
    }

    graph_free(&g);
    finalize_B(BLACK);
    dlclose(player_B);

    printf("\tTest passed\n");
}

void test_client_steven()
{
    printf("%s", __func__);

    struct graph_t* g = graph_make_triangular(3);
    g->start[1] = graph_neighbor_direction(g, g->start[0], NW);

    struct graph_t* g_B = malloc(sizeof(struct graph_t));
    graph_cpy(g_B, g);

    vertex_t pos_B = g->start[0];
    vertex_t pos_W = g->start[1];

    void* player_B = dlopen("install/steven.so", RTLD_LAZY);
    if (!player_B) {
        fputs(dlerror(), stderr);
        exit(1);
    }

    void (*initialize_B)(unsigned int, struct graph_t*);
    initialize_B = _get_func(player_B, "initialize");
    struct move_t (*play_B)(const struct move_t);
    play_B = _get_func(player_B, "play");
    void (*finalize_B)();
    finalize_B = _get_func(player_B, "finalize");

    initialize_B(BLACK, g_B);

    struct edge_t removed[g->num_edges];
    int removed_size = 0;
    graph_remove_edge(g, (struct edge_t) { g->start[0], graph_neighbor_direction(g, g->start[0], E) });
    graph_remove_edge(g, (struct edge_t) { g->start[0], graph_neighbor_direction(g, g->start[0], NE) });
    graph_remove_edge(g_B, (struct edge_t) { g_B->start[0], graph_neighbor_direction(g_B, g_B->start[0], E) });
    graph_remove_edge(g_B, (struct edge_t) { g_B->start[0], graph_neighbor_direction(g_B, g_B->start[0], NE) });
    removed[removed_size] = (struct edge_t) { g->start[0], graph_neighbor_direction(g, g->start[0], E) };
    removed[removed_size + 1] = (struct edge_t) { g->start[0], graph_neighbor_direction(g, g->start[0], NE) };
    removed_size = removed_size + 2;

    struct move_t move;
    move.c = WHITE;
    move.t = NO_TYPE;
    move.m = g->num_vertices;
    move.e[0] = (struct edge_t) { g->num_vertices + 1, g->num_vertices + 2 };
    move.e[1] = (struct edge_t) { g->num_vertices + 3, g->num_vertices + 4 };

    char name[17] = "test_steven0.dot";
    graph_dot(g, removed, removed_size, pos_B, pos_W, name, 0);
    for (int i = 0; i < 1; i++) {
        move.c = WHITE;
        move = play_B(move);
        if (move.t == MOVE)
            pos_B = move.m;
        else if (move.t == WALL) {
            graph_remove_edge(g, move.e[0]);
            graph_remove_edge(g, move.e[1]);
            removed[removed_size] = move.e[0];
            removed[removed_size + 1] = move.e[1];
            removed_size = removed_size + 2;
        }
        name[11] = '0' + i + 1;
        graph_dot(g, removed, removed_size, pos_B, pos_W, name, 0);
    }

    graph_free(&g);
    finalize_B(BLACK);
    dlclose(player_B);

    printf("\tTest passed\n");
}

void test_dot()
{
    printf("%s", __func__);

    struct graph_t* g = malloc(sizeof(struct graph_t));
    g->type = TRIANGULAR;
    g->num_vertices = 3;
    g->num_edges = 3;
    g->t = gsl_spmatrix_uint_alloc(g->num_vertices, g->num_vertices);
    g->start[0] = 0;
    g->start[1] = g->num_vertices - 1;
    g->num_objectives = 1;
    g->objectives = malloc(g->num_objectives * sizeof(vertex_t));
    g->objectives[0] = 1;
    gsl_spmatrix_uint_set(g->t, 0, 1, 1);
    gsl_spmatrix_uint_set(g->t, 1, 0, 1);
    gsl_spmatrix_uint_set(g->t, 2, 1, 1);
    gsl_spmatrix_uint_set(g->t, 1, 2, 1);
    gsl_spmatrix_uint_set(g->t, 0, 2, 1);
    gsl_spmatrix_uint_set(g->t, 2, 0, 1);

    graph_dot(g, NULL, 0, g->start[0], g->start[1], "test.dot", 0);

    graph_free(&g);

    printf("\tTest passed\n");
}

void test_dot_triangular()
{
    printf("%s", __func__);

    struct graph_t* g = graph_make_triangular(4);
    graph_dot(g, NULL, 0, g->start[0], g->start[1], "test_triangular.dot", 0);
    graph_free(&g);

    printf("\tTest passed\n");
}
void test_dot_circular()
{
    printf("%s", __func__);

    struct graph_t* g = graph_make_circular(6);
    graph_dot(g, NULL, 0, g->start[0], g->start[1], "test_circular.dot", 0);
    graph_free(&g);

    printf("\tTest passed\n");
}

void test_dot_holey()
{
    printf("%s", __func__);

    struct graph_t* g = graph_make_holey(6);
    graph_dot(g, NULL, 0, g->start[0], g->start[1], "test_holey.dot", 0);
    graph_free(&g);

    printf("\tTest passed\n");
}

void test_tsp()
{
  test_dot_holey();
  struct graph_t *g = graph_make_holey(6);
  size_t distance = 0;
  int obj[4] = {0};
  vertex_t *d = tsp(g, g -> start[0], E, g -> start[1],obj, &distance);
  printf("%ld\n",distance);
  for(size_t i = 0; i < distance; i++)
    printf("%u ",d[i]);
  free(d);
  graph_free(&g);
  printf("\nOK\n");
}
  

void test_dijkstra()
{
    printf("%s", __func__);

    struct graph_t* g = graph_make_triangular(3);
    for (unsigned int i = 0; i < g->num_vertices; i++)
        assert(bfs(g, i, i) == 0);
    for (unsigned int i = 0; i < g->num_vertices; i++)
        for (unsigned int d = 1; d < NUM_DIRS + 1; d++)
            if (graph_neighbor_direction(g, i, d) != g->num_vertices)
                assert(bfs(g, i, graph_neighbor_direction(g, i, d)) == 1);

    assert(bfs(g, 0, 0) == 0);
    assert(bfs(g, 0, 1) == 1);
    assert(bfs(g, 0, 2) == 2);
    assert(bfs(g, 0, 3) == 1);
    assert(bfs(g, 0, 4) == 1);
    assert(bfs(g, 0, 5) == 2);
    assert(bfs(g, 0, 6) == 3);
    assert(bfs(g, 0, 7) == 2);
    assert(bfs(g, 0, 8) == 2);
    assert(bfs(g, 0, 9) == 2);
    assert(bfs(g, 0, 10) == 3);
    assert(bfs(g, 0, 11) == 4);
    assert(bfs(g, 0, 12) == 3);
    assert(bfs(g, 0, 13) == 3);
    assert(bfs(g, 0, 14) == 3);
    assert(bfs(g, 0, 15) == 4);
    assert(bfs(g, 0, 16) == 4);
    assert(bfs(g, 0, 17) == 4);
    assert(bfs(g, 0, 18) == 4);

    graph_remove_edge(g, (struct edge_t) { 0, 3 });
    graph_remove_edge(g, (struct edge_t) { 0, 4 });

    assert(bfs(g, 0, 0) == 0);
    assert(bfs(g, 0, 1) == 1);
    assert(bfs(g, 0, 2) == 2);
    assert(bfs(g, 0, 3) == 3);
    assert(bfs(g, 0, 4) == 2);
    assert(bfs(g, 0, 5) == 2);
    assert(bfs(g, 0, 6) == 3);
    assert(bfs(g, 0, 7) == 4);
    assert(bfs(g, 0, 8) == 3);
    assert(bfs(g, 0, 9) == 3);
    assert(bfs(g, 0, 10) == 3);
    assert(bfs(g, 0, 11) == 4);
    assert(bfs(g, 0, 12) == 4);
    assert(bfs(g, 0, 13) == 4);
    assert(bfs(g, 0, 14) == 4);
    assert(bfs(g, 0, 15) == 4);
    assert(bfs(g, 0, 16) == 5);
    assert(bfs(g, 0, 17) == 5);
    assert(bfs(g, 0, 18) == 5);

    graph_free(&g);

    printf("\tTest passed\n");
}

int main(void)
{
    test_client_tristan();
    test_client_steven();
    test_dot();
    test_dot_triangular();
    test_dot_circular();
    test_dot_holey();
    test_dijkstra();
    test_tsp();
    return 0;
}
