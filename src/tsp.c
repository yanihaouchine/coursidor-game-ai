#include "tsp.h"
vertex_t graph_neighbor_direction(struct graph_t* g, vertex_t v, enum dir_t d);

struct automate_state_t {
    vertex_t pos;
    enum dir_t last_dir;
    int obj_r;
};


struct queue {
    struct automate_state_t* data;
    size_t size;
    size_t start;
    size_t cap;
};

struct queue* create_queue()
{
    struct queue* q = malloc(sizeof(struct queue));
    q->data = malloc(sizeof(void*));
    q->size = 0;
    q->start = 0;
    q->cap = 1;
    return q;
}
void destroy_queue(struct queue* q)
{
  free(q->data);
  free(q);
}

void enqueue(struct queue* q,  struct automate_state_t v)
{
  if (q->size == q->cap) {
    size_t old_cap = q->cap;
    q->cap = old_cap * 2;
    struct automate_state_t* new_data = malloc(q->cap * sizeof(*new_data));
    for (size_t i = 0; i < q->size; i++)
      new_data[i] = q->data[(q->start + i) % old_cap];
    free(q->data);
    q->data = new_data;
    q->start = 0;
  }

  q->data[(q->start + q->size) % q->cap] = v;
  q->size++;
}

struct automate_state_t dequeue(struct queue* q)
{
  struct automate_state_t v = q->data[q->start];
  q->start = (q->start + 1) % q->cap;
  q->size--;
  return v;
}

int is_not_empty_queue(struct queue* q)
{
  return q->size != 0;
}

int get_neighbors(struct graph_t* graph, vertex_t v, vertex_t* neighbors) // À améliorer
{
    int k = 0;

    for (unsigned int i = 0; i < graph->num_vertices; i++)
        if (gsl_spmatrix_uint_get(graph->t, i, v)) {
            neighbors[k] = i;
            k = k + 1;
        }

    return k;
}





enum dir_t norme(enum dir_t d1, enum dir_t d2)
{
  return d1 < d2 ? d2 - d1 : d1 - d2;
}

int max_lens(enum dir_t d1, enum dir_t d2)
{
    if (d1 == NO_EDGE || d2 == NO_EDGE)
        return 0;
    int angle = norme(d1 , d2);
    if (angle == 0 )
        return 2;
    if (angle == 1 )
        return 1;
    return 0;
}

int is_obj(vertex_t k, struct graph_t* g)
{
  for (unsigned int i = 0; i < g->num_objectives; i++)
    if (k == g->objectives[i])
      return (int)i;
  return -1;
}

vertex_t get_vertices_direction_depth(struct graph_t* g, vertex_t v, enum dir_t d, int depth)
{
  if (v >= g->num_vertices || d == NO_EDGE)
    return g->num_vertices;
  vertex_t new_v = graph_neighbor_direction(g, v, d);
  if(new_v >= g -> num_vertices) return g -> num_vertices;
  if (depth <= 0)
    return new_v;
  return get_vertices_direction_depth(g, new_v, d, depth - 1);
}



void get_A_neighbors(struct graph_t* g, struct automate_state_t q, struct automate_state_t* neighbors, int* nb_s) // On suppose que 2 objectifs sont à une distnce supérieur à 3
{
  *nb_s = 0;
  vertex_t v;
  int reached = -1;
  for (enum dir_t d = FIRST_DIR; d <= LAST_DIR; d++) {
    int max_depth = max_lens(d, q.last_dir);
    reached = -1;
    for (int depth = 0; depth <= max_depth; depth++) {
      v = get_vertices_direction_depth(g, q.pos, d, depth);
      if( v >= g -> num_vertices ) continue;
      int obj_idx = is_obj(v ,g);
      int new_obj_r = q.obj_r;
      if(obj_idx != -1){
	reached = obj_idx;
	if((new_obj_r & ( 1 << obj_idx)) == 0)
	  new_obj_r = new_obj_r | (1 << obj_idx);
      }
      if(reached != -1){ //On peut sauter audessus des objectifs.
	obj_idx = reached;
	if((new_obj_r & ( 1 << obj_idx)) == 0)
	  new_obj_r = new_obj_r | (1 << obj_idx);
      }
      neighbors[*nb_s] = (struct automate_state_t){v , d, new_obj_r};
      (*nb_s)++;
    }
  }
}


void get_voisins(struct graph_t* g, vertex_t pos, enum dir_t last_dir, vertex_t* neighbors, enum dir_t *dir, int* nb_s)
{
   *nb_s = 0;
  vertex_t v;
  for (enum dir_t d = FIRST_DIR; d <= LAST_DIR; d++) {
    int max_depth = max_lens(d, last_dir);
    for (int depth = 0; depth <= max_depth; depth++) {
      v = get_vertices_direction_depth(g, pos, d, depth);
      if( v >= g -> num_vertices ) continue;
      neighbors[*nb_s] = v;
      dir[*nb_s] = d;
      (*nb_s)++;
    }
  }
}
  
  

int A_idx(struct automate_state_t q, struct graph_t* g)
{
  return (q.pos * (NUM_DIRS + 1) + q.last_dir) * (1 << g->num_objectives) + q.obj_r;
}

int is_end_A_state(struct automate_state_t q, int all_obj, vertex_t end)
{
  return q.pos == end && all_obj - 1 == q.obj_r;
}

struct automate_state_t *create_a_state(struct automate_state_t q)
{
  struct automate_state_t *q_p = malloc(sizeof(struct automate_state_t));
  *q_p = q;
  return q_p;
}
int equal_state(struct automate_state_t q0, struct automate_state_t q)
{
  return q.pos == q0.pos && q.last_dir == q0.last_dir && q.obj_r == q0.obj_r;
}

vertex_t* A_best_path(struct graph_t* g, struct automate_state_t q0, vertex_t end, size_t* size_of_path)
{
  *size_of_path = 0;
  int all_obj = 1 << g->num_objectives;
  size_t nb_states = (g -> num_vertices)*(NUM_DIRS + 1) * all_obj;
  int* visited = calloc(nb_states, sizeof(int));
  struct queue* f = create_queue();
  visited[A_idx(q0, g)] = 1;
  struct automate_state_t *previous =  malloc(sizeof( struct automate_state_t)* nb_states);
  previous[A_idx(q0,g)] = q0;
  struct automate_state_t neighbors[20];
  int nb_s;
  enqueue(f, q0);
  while (is_not_empty_queue(f)) {
    struct automate_state_t q = (dequeue(f));
    if(is_end_A_state(q, all_obj, end)){
      struct automate_state_t q1 = q;
      *size_of_path = 0;
      while(!equal_state(q0, q1)){
	*size_of_path += 1;
	q1 = previous[A_idx(q1,g)];
      }
      vertex_t* best_path = malloc(sizeof(vertex_t) * (*size_of_path + 1));
      best_path[*size_of_path] = q.pos;
      struct automate_state_t prev = q;
      for (size_t i = 0; i < *size_of_path; i++) {
	prev = previous[A_idx(prev, g)];
	best_path[*size_of_path - i -1] = prev.pos;
      }
      ++(*size_of_path);
      free(previous);
      free(visited);
      destroy_queue(f);
      return best_path;
    }
    get_A_neighbors(g, q, neighbors, &nb_s);
    for (int i = 0; i < nb_s; i++) {
      if (!visited[A_idx(neighbors[i], g)]) {
	visited[A_idx(neighbors[i], g)] = 1;
	previous[A_idx(neighbors[i], g)] = q;
	enqueue(f, neighbors[i]);
      }

    }
  }
      
  free(previous);
  free(visited);
  destroy_queue(f);
  *size_of_path = 0;
  return NULL;
}




vertex_t *tsp(struct graph_t *g, vertex_t start, enum dir_t last_dir, vertex_t end, int *objectif_reached, size_t *distance)
{
  int obj_r = 0;
  for(unsigned int i = 0; i < g -> num_objectives; i++)
    if(objectif_reached[i] == 1) obj_r += 1 << i;
  struct automate_state_t q0 = (struct automate_state_t){start, last_dir, obj_r};
  return A_best_path(g, q0, end, distance);
}




vertex_t *get_best_start(struct graph_t *g, enum dir_t last_dir, vertex_t end, int *objectif_reached, size_t *distance, vertex_t *start_possibility, size_t size_of_start_possible)
{
  vertex_t *best_path =tsp(g,start_possibility[0],last_dir,end,objectif_reached,distance);
  for(size_t i = 1; i < size_of_start_possible; i++)
    {
      size_t path_size;
      vertex_t *path = tsp(g,start_possibility[i],last_dir,end,objectif_reached,&path_size);
      if(path_size < *distance){
	free(best_path);
	*distance = path_size;
	best_path = path;
      }
      else
	free(path);
    }
  return best_path;
}
