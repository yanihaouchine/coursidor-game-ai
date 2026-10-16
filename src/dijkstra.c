#include "dijkstra.h"
#define MAX_NEIGHBOR 6
vertex_t graph_neighbor_direction(struct graph_t* g, vertex_t v, enum dir_t d);
struct queue {
    vertex_t* data;
    size_t size;
    size_t start;
    size_t cap;
};

struct queue* v_create_queue()
{
    struct queue* q = malloc(sizeof(struct queue));
    q->data = malloc(sizeof(vertex_t)*16);
    q->size = 0;
    q->start = 0;
    q->cap = 1;
    return q;
}
void v_destroy_queue(struct queue* q)
{
  free(q->data);
  free(q);
}

void v_enqueue(struct queue* q, vertex_t v)
{
    if (q->size == q->cap) {
        size_t old_cap = q->cap;
        q->cap = old_cap * 2;
        vertex_t* new_data = malloc(q->cap * sizeof(*new_data));
        for (size_t i = 0; i < q->size; i++)
            new_data[i] = q->data[(q->start + i) % old_cap];
        free(q->data);
        q->data = new_data;
        q->start = 0;
    }

    q->data[(q->start + q->size) % q->cap] = v;
    q->size++;
}

vertex_t v_dequeue(struct queue* q)
{
    vertex_t v = q->data[q->start];
    q->start = (q->start + 1) % q->cap;
    q->size--;
    return v;
}

int v_is_not_empty_queue(struct queue* q)
{
    return q->size != 0;
}

int v_get_neighbors(struct graph_t* graph, vertex_t v, vertex_t* neighbors) // À améliorer
{
    int k = 0;

    for (unsigned int i = 0; i < graph->num_vertices; i++)
        if (gsl_spmatrix_uint_get(graph->t, i, v)) {
            neighbors[k] = i;
            k = k + 1;
        }

    return k;
}



int bfs(struct graph_t* graph, vertex_t start, vertex_t end)
{
  int* visited = calloc( (graph->num_vertices), sizeof(int));
    struct queue* q = v_create_queue();
    int* distance = malloc(sizeof(int) * (graph->num_vertices));
    for (unsigned int i = 0; i < graph->num_vertices; i++)
        distance[i] = -1;
    v_enqueue(q, start);
    visited[start] = 1;
    distance[start] = 0;
    vertex_t* neighbors = malloc(sizeof(vertex_t) * (6));
    while (v_is_not_empty_queue(q)) {
        vertex_t v = v_dequeue(q);
        if (v == end) {
            free(neighbors);
            free(visited);
            v_destroy_queue(q);
            int d = distance[v];
            free(distance);
            return d;
        }
        int num_neighbors = v_get_neighbors(graph, v, neighbors);
        for (int i = 0; i < num_neighbors; i++) {
            if (visited[neighbors[i]] == 0) {
                v_enqueue(q, neighbors[i]);
                visited[neighbors[i]] = 1;
                distance[neighbors[i]] = distance[v] + 1;
            }
        }
    }
    free(neighbors);
    free(visited);
    v_destroy_queue(q);
    int d = distance[end];
    free(distance);
    return d;
}

