#include "board.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

vertex_t idx_2(size_t ligne, size_t colonne, size_t m);
struct graph_t* graph_make_triangular(size_t m)
{
    if (m < 2)
        m = 2;
    struct graph_t* g = graph_create(3 * m * m - 3 * m + 1);
    g->type = TRIANGULAR;
    g->num_edges = 0;
    g->start[0] = 0;
    g->start[1] = g->num_vertices - 1;
    g->num_objectives = 4;
    g->objectives = malloc(g->num_objectives * sizeof(vertex_t));
    g->objectives[0] = 2 * m + 2;
    g->objectives[1] = 3 * m + 1;
    g->objectives[2] = g->num_vertices - 2 * m - 3;
    g->objectives[3] = g->num_vertices - 3 * m - 2;
    vertex_t k = 0;
    for (unsigned int i = 0; i < m - 1; i++) {
        for (unsigned int j = 0; j < m + i; j++) {
            if (i < 2 * m - 2) {
                if (i < m - 1 || j > 0) {
                    graph_add_edge(g, k, k + m + i, NW);
                }
                if (i < m - 1 || j < m + i - 1) {
                    graph_add_edge(g, k, k + m + i + 1, NE);
                }
            }
            if (j < m + i - 1) {
                graph_add_edge(g, k, k + 1, E);
            }
            k++;
        }
    }
    for (vertex_t i = m - 1; i > 0; i--) {
        for (vertex_t j = 0; j < m + i; j++) {

            if (j > 0) {
                graph_add_edge(g, k, k + m + i - 1, NW);
            }
            if (j < m + i - 1) {
                graph_add_edge(g, k, k + m + i, NE);
                graph_add_edge(g, k, k + 1, E);
            }
            k++;
        }
    }
    for (vertex_t j = 0; j < m - 1; j++) {
        if (j < m - 1)
            graph_add_edge(g, k, k + 1, E);
        k++;
    }
    assert(k == 3 * m * m - 3 * m);

    gsl_spmatrix_uint* coo = g->t;
    g->t = gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
    gsl_spmatrix_uint_free(coo);

    return g;
}
struct graph_t* graph_make_circular(size_t m)
{
    if (m < 3)
        m = 3;
    struct graph_t* g = malloc(sizeof(struct graph_t));
    g->type = CYCLIC;
    g->num_vertices = 12 * m - 18;
    g->num_edges = 0;
    g->t = gsl_spmatrix_uint_alloc(g->num_vertices, g->num_vertices);
    g->start[0] = 0;
    g->start[1] = g->num_vertices - 1;
    g->num_objectives = 4;
    g->objectives = malloc(g->num_objectives * sizeof(vertex_t));
    g->objectives[0] = 2 * m + 2;
    g->objectives[1] = 2 * m + 3;
    g->objectives[2] = g->num_vertices - 2 * m - 4;
    g->objectives[3] = g->num_vertices - 2 * m - 3;
    vertex_t k = 0;
    for (vertex_t j = 0; j < m; j++) {
        if (j < m - 1)
            graph_add_edge(g, k, k + 1, E);
        graph_add_edge(g, k, k + m, NW);
        graph_add_edge(g, k, k + m + 1, NE);
        k++;
    }
    for (vertex_t j = 0; j < m + 1; j++) {
        if (j <= 1 || j >= m - 1) {
            if (j < m - 1) {
                graph_add_edge(g, k, k + m + 1, NW);
            }
            if (j == m)
                graph_add_edge(g, k, k + 3, NW);
            if (j == 0) {
                graph_add_edge(g, k, k + m + 2, NE);
            }
            if (j > 1) {
                graph_add_edge(g, k, k + 4, NE);
            }
        }
        if (j != m)
            graph_add_edge(g, k, k + 1, E);
        k++;
    }
    for (unsigned int i = 2; i < m - 1; i++) {
        graph_add_edge(g, k, k + 1, E);
        graph_add_edge(g, k, k + 4, NW);
        graph_add_edge(g, k, k + 5, NE);
        k++;
        graph_add_edge(g, k, k + 4, NW);
        k++;
        graph_add_edge(g, k, k + 4, NE);
        graph_add_edge(g, k, k + 1, E);
        k++;
        graph_add_edge(g, k, k + 3, NW);
        graph_add_edge(g, k, k + 4, NE);
        k++;
    }
    for (unsigned int i = m - 1; i > 2; i--) {
        graph_add_edge(g, k, k + 1, E);
        graph_add_edge(g, k, k + 4, NE);
        k++;
        graph_add_edge(g, k, k + 3, NW);
        graph_add_edge(g, k, k + 4, NE);
        k++;
        graph_add_edge(g, k, k + 4, NW);
        graph_add_edge(g, k, k + 5, NE);
        graph_add_edge(g, k, k + 1, E);
        k++;
        graph_add_edge(g, k, k + 4, NW);
        k++;
    }
    graph_add_edge(g, k, k + 1, E);
    graph_add_edge(g, k, k + 4, NE);
    k++;
    graph_add_edge(g, k, k + 3, NW);
    graph_add_edge(g, k, k + 4, NE);
    k++;
    graph_add_edge(g, k, k + m + 1, NW);
    graph_add_edge(g, k, k + m + 2, NE);
    graph_add_edge(g, k, k + 1, E);
    k++;
    graph_add_edge(g, k, k + m + 1, NW);
    k++;
    for (vertex_t j = 0; j < m + 1; j++) {
        if (j < m)
            graph_add_edge(g, k, k + 1, E);
        if (j != 0)
            graph_add_edge(g, k, k + m, NW);
        if (j != m)
            graph_add_edge(g, k, k + m + 1, NE);
        k++;
    }
    for (vertex_t j = 0; j < m; j++) {
        if (j < m - 1)
            graph_add_edge(g, k, k + 1, E);
        k++;
    }

    gsl_spmatrix_uint* coo = g->t;
    g->t = gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
    gsl_spmatrix_uint_free(coo);

    return g;
}
int In(vertex_t center, size_t radius, struct graph_t* g, vertex_t point)
{
    if (bfs(g, center, point) < (int)radius)
        return 1;
    return 0;
}

size_t first_of_ligne(size_t ligne, size_t m)
{
    size_t index;
    if (ligne < m) {
        index = 0;
        for (size_t i = 0; i < ligne; i++)
            index += m + i;
    } else {
        ++ligne;
        index = 3 * m * m - 3 * m;
        for (size_t i = 0; i < 2 * m - ligne - 1; i++)
            index -= m + i;
        index -= (m - 1 + (2 * m - ligne - 1));
    }
    return (vertex_t)index;
}
vertex_t idx_2(size_t ligne, size_t colonne, size_t m)
{
    return (vertex_t)(colonne + first_of_ligne(ligne, m));
}

vertex_t* represent(size_t m)
{
    vertex_t* representant = malloc(sizeof(vertex_t) * 7);
    representant[0] = idx_2(m - 1, m - 1, m);
    representant[1] = idx_2(m - 1, m + (m / 3) - 1 + (m / 3) - 1, m);
    representant[2] = idx_2(m - 1, m - (m / 3) - 1 - (m / 3) + 1, m);
    representant[3] = idx_2(m / 3, 2 * (m / 3) - (m / 3), m);
    representant[4] = idx_2(m / 3, 2 * (m / 3) + (m / 3) - 1, m);
    representant[5] = idx_2(2 * m - 2 - (m / 3), m + (m / 3) - (2 * (m / 3)) + (m / 3) - 1, m);
    representant[6] = idx_2(2 * m - 2 - (m / 3), m + (m / 3) - (2 * (m / 3)) - (m / 3), m);
    return representant;
}
int exist_graph_vertices(size_t m, vertex_t v, struct graph_t* g, vertex_t* representant)
{
    for (int i = 0; i < 7; i++) {
        if ((In(representant[i], (m / 3 - 1), g, v))) {
            return 0;
        }
    }
    return 1;
}

void create_existant_vertices(int* exist, size_t m, struct graph_t* g, vertex_t* representant)
{
    for (size_t i = 0; i < g->num_vertices; i++)
        exist[i] = (exist_graph_vertices(m, i, g, representant)) ? 1 : 0;
}

struct graph_t* graph_make_holey(size_t m)
{
    if (m % 3 != 0)
        m = (m / 3) * 3;
    if (m < 6)
        m = 6;
    struct graph_t* triang = graph_make_triangular(m);
    struct graph_t* g = graph_create((2 * m * (m / 3)) + (18 * m) - 48);
    g->type = HOLEY;
    g->num_edges = 0;
    g->start[0] = 0;
    g->start[1] = g->num_vertices - 1;
    g->num_objectives = 4;
    g->objectives = malloc(g->num_objectives * sizeof(vertex_t));
    int* exist = malloc(sizeof(int) * triang->num_vertices);
    vertex_t* representant = represent(m);
    create_existant_vertices(exist, m, triang, representant);
    vertex_t num_vertices = 0;
    vertex_t* map = malloc(sizeof(vertex_t) * triang->num_vertices);
    for (vertex_t i = 0; i < triang->num_vertices; i++)
        map[i] = exist[i] ? num_vertices++ : (vertex_t)-1;
    for (vertex_t i = 0; i < triang->num_vertices; i++) {
        for (vertex_t j = i; j < triang->num_vertices; j++) {
            if (graph_exist_edge(triang, (struct edge_t) { i, j }) && (map[i] != (vertex_t)-1) && (map[j] != (vertex_t)-1))
                graph_add_edge(g, map[i], map[j], graph_exist_edge(triang, (struct edge_t) { i, j }));
        }
    }
    g->objectives[0] = map[idx_2(m - 1, m + (m / 3) - 1, m)];
    g->objectives[1] = map[idx_2(m - 1, m - (m / 3) - 1, m)];
    g->objectives[2] = map[idx_2(m / 3, 2 * (m / 3) - 1, m)];
    g->objectives[3] = map[idx_2(2 * m - 2 - (m / 3), m + (m / 3) - (2 * (m / 3)), m)];
    graph_free(&triang);
    free(representant);
    free(map);
    free(exist);

    gsl_spmatrix_uint* coo = g->t;
    g->t = gsl_spmatrix_uint_compress(coo, GSL_SPMATRIX_CSR);
    gsl_spmatrix_uint_free(coo);

    return g;
}

struct graph_t* graph_create(unsigned int n_vertices)
{
    struct graph_t* g = malloc(sizeof(struct graph_t));
    g->num_vertices = n_vertices;
    g->t = gsl_spmatrix_uint_alloc(g->num_vertices, g->num_vertices);
    g->type = TRIANGULAR;
    g->num_edges = 0;
    g->start[0] = 0;
    g->start[1] = 0;
    g->num_objectives = 0;
    g->objectives = NULL;

    return g;
}

enum dir_t graph_exist_edge(struct graph_t* g, struct edge_t e)
{
    return gsl_spmatrix_uint_get(g->t, e.fr, e.to);
}

int graph_remove_edge(struct graph_t* g, struct edge_t e)
{
    if (!graph_exist_edge(g, e))
        return 1;

    enum dir_t* edge = gsl_spmatrix_uint_ptr(g->t, e.fr, e.to);
    assert(edge != NULL);
    *edge = 0;
    edge = gsl_spmatrix_uint_ptr(g->t, e.to, e.fr);
    *edge = 0;
    g->num_edges--;
    return 0;
}

int graph_add_edge(struct graph_t* g, vertex_t fr, vertex_t to, enum dir_t d)
{
    if (!strcmp(gsl_spmatrix_uint_type(g->t), "COO")) {
        gsl_spmatrix_uint_set(g->t, fr, to, d);
        gsl_spmatrix_uint_set(g->t, to, fr, opposite_dir(d));
    } else {
        enum dir_t* edge = gsl_spmatrix_uint_ptr(g->t, fr, to);
        assert(edge != NULL);
        *edge = d;
        edge = gsl_spmatrix_uint_ptr(g->t, to, fr);
        *edge = opposite_dir(d);
    }
    g->num_edges++;
    return 0;
}

vertex_t graph_neighbor_direction(struct graph_t* g, vertex_t v, enum dir_t d)
{
  /* if(d == NO_EDGE) */
  /*   return g -> num_vertices; */
  /* if(d == E || d == NE || d == NW){ */
  /*   for (unsigned int j = v; j < g->num_vertices; j++) */
  /*     if (gsl_spmatrix_uint_get(g->t, v, j) == d) */
  /* 	return j; */
  /* } */
  /* else{ */
  /*   for (unsigned int j = v; j > 0; j--) */
  /*     if (gsl_spmatrix_uint_get(g->t, v, j) == d) */
  /* 	return j; */
  /*   if (gsl_spmatrix_uint_get(g->t, v, 0) == d) */
  /*     return 0; */
  /* } */
  for (unsigned int j = 0; j < g->num_vertices; j++)
      if (gsl_spmatrix_uint_get(g->t, v, j) == d)
	return j;
  return g->num_vertices;
}

void graph_free(struct graph_t** g)
{
    if ((*g) != NULL) {
        gsl_spmatrix_uint_free((*g)->t);
        if ((*g)->objectives != NULL)
            free((*g)->objectives);
        free(*g);
        *g = NULL;
    }
}

void graph_dot(struct graph_t* g, struct edge_t* removed, int removed_size, vertex_t black_player_pos, vertex_t white_player_pos, char* name, int append)
{
    FILE* f;
    if (append)
        f = fopen(name, "a");
    else
        f = fopen(name, "w");
    if (!f) {
        fputs("error opening file", stderr);
        exit(1);
    }

    fprintf(f, "graph g {\n  graph [bgcolor=gray];\n  node[style=filled];\n");

    for (unsigned int i = 0; i < g->num_vertices; i++) {
        unsigned int j = 0;
        while (j < g->num_objectives && i != g->objectives[j])
            j = j + 1;
        if (j < g->num_objectives)
            fprintf(f, "  %d [color = red,fontcolor=white,fillcolor=red,shape=circle,fixedsize=true,width=0.5];\n", i);
        else if (i == g->start[0])
            fprintf(f, "  %d [fontcolor=white,fillcolor=darkgray,shape=circle,fixedsize=true,width=0.5,label=\"%d\nBS\"];\n", i, i);
        else if (i == g->start[1])
            fprintf(f, "  %d [fontcolor=white,fillcolor=darkgray,shape=circle,fixedsize=true,width=0.5,label=\"%d\nWS\"];\n", i, i);
        else
            fprintf(f, "  %d [fontcolor=white,fillcolor=darkgray,shape=circle,fixedsize=true,width=0.5];\n", i);
        if (i == black_player_pos)
            fprintf(f, "  %d [fillcolor=black];\n", i);
        if (i == white_player_pos)
            fprintf(f, "  %d [fontcolor=black,fillcolor=white];\n", i);
    }
    for (unsigned int i = 0; i < g->num_vertices; i++) {
        for (unsigned int j = 0; j < i; j++) {
            struct edge_t e = { i, j };
            if (graph_exist_edge(g, e))
                /*
                fprintf(f, "  %d -- %d [label=\"%d %d\"];\n", i, j, graph_exist_edge(g, (struct edge_t) { e.fr, e.to }), graph_exist_edge(g, (struct edge_t) { e.to, e.fr }));
                */
                fprintf(f, "  %d -- %d;\n", i, j);
        }
    }
    for (int i = 0; i < removed_size; i++) {
        fprintf(f, "  %d -- %d [color=gray];\n", removed[i].fr, removed[i].to);
    }

    fprintf(f, "}\n");

    fclose(f);
}

void graph_cpy(struct graph_t* dest, struct graph_t* src)
{
    dest->type = src->type;
    dest->num_vertices = src->num_vertices;
    dest->num_edges = src->num_edges;
    dest->t = gsl_spmatrix_uint_alloc_nzmax(src->num_vertices, src->num_vertices, 0.1 * src->num_vertices * src->num_vertices, GSL_SPMATRIX_CSR);
    gsl_spmatrix_uint_memcpy(dest->t, src->t);
    for (int i = 0; i < NUM_PLAYERS; i++)
        dest->start[i] = src->start[i];
    dest->num_objectives = src->num_objectives;
    dest->objectives = malloc(src->num_objectives * sizeof(vertex_t));
    for (unsigned int i = 0; i < src->num_objectives; i++)
        dest->objectives[i] = src->objectives[i];
}

void move_print(struct move_t m)
{
    if (m.c == 0)
        printf("Color : BLACK\n");
    else
        printf("Color : WHITE\n");
    if (m.t == 1) {
        printf("Type : WALL\n");
        printf("Edge 1 : %d-%d\n", m.e[0].fr, m.e[0].to);
        printf("Edge 2 : %d-%d\n", m.e[1].fr, m.e[1].to);
        printf("Player location : %d\n", m.m);
    } else if (m.t == 2) {
        printf("Type : MOVE\n");
        printf("Player location : %d\n", m.m);
    } else
        printf("Type : NO_TYPE\n");
}

unsigned int graph_get_num_vertices(struct graph_t* g)
{
    return g->num_vertices;
}

unsigned int graph_get_num_edges(struct graph_t* g)
{
    return g->num_edges;
}

unsigned int graph_get_num_objectives(struct graph_t* g)
{
    return g->num_objectives;
}
