#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "graph.h"
#include "move.h"
#include "player.h"

int bfs(struct graph_t* graph, vertex_t start, vertex_t end);

struct player_t {
    const char* name;
    unsigned int id;
    struct graph_t* board;
    enum dir_t dir;
    int num_wall;
    vertex_t pos;
    int* objectives_reached; // array of booleans : objectives_reached[i] = 1 if board->objectives[i] have been reached and 0 otherwise.
};

unsigned int my_id;

struct player_t p[NUM_PLAYERS];

char const* get_player_name()
{
    return "Tristan";
}

void initialize(unsigned int player_id, struct graph_t* graph)
{
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
    }
    p[player_id].name = get_player_name();
    my_id = player_id;
}

// Gives the next player given the current player
int compute_next_player(int current_player)
{
    return (current_player + 1) % NUM_PLAYERS;
}

/* A function to determine the next dir in the clockwise order */
static inline enum dir_t next_dir_inv(enum dir_t d)
{
    return (d == 0) ? 0 : (d == 6) ? FIRST_DIR
                                   : (d + 1);
}

int objectives_are_reachable()
{
    int bool_res = 1;
    unsigned int i = 0;
    while (bool_res && i < NUM_PLAYERS) {
        unsigned int j = 0;
        while (bool_res && j < p[0].board->num_objectives) {
            if (!p[i].objectives_reached[j])
                bool_res = bool_res && (bfs(p[0].board, p[i].pos, p[0].board->objectives[j]) != -1);
            j = j + 1;
        }
        bool_res = bool_res && (bfs(p[0].board, p[i].pos, p[0].board->start[p[i].id]) != -1);
        i = i + 1;
    }

    return bool_res;
}

struct move_t play(const struct move_t previous_move)
{
    srand(time(NULL));
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

    int tmp[NUM_PLAYERS] = { 0 };
    int dist[NUM_PLAYERS]; // distance between pos and dest
    vertex_t dest[NUM_PLAYERS] = { 0 }; // the nearest objective for the player
    int nb_of_objectives_unreached[NUM_PLAYERS] = { 0 };
    for (unsigned int j = 0; j < NUM_PLAYERS; j++) {
        dist[j] = p[0].board->num_edges + 1;
        if (p[0].board->num_objectives) {
            for (unsigned int i = 0; i < p[0].board->num_objectives; i++) {
                if (!p[j].objectives_reached[i]) {
                    tmp[j] = bfs(p[0].board, p[j].pos, p[0].board->objectives[i]);
                    if (tmp[j] <= dist[j]) {
                        dist[j] = tmp[j];
                        dest[j] = p[0].board->objectives[i];
                    }
                    nb_of_objectives_unreached[j] = nb_of_objectives_unreached[j] + 1;
                }
            }
        }
        if (p[0].board->num_objectives == 0 || nb_of_objectives_unreached[j] == 0) {
            dist[j] = bfs(p[0].board, p[j].pos, p[0].board->start[p[j].id]);
            dest[j] = p[0].board->start[p[j].id];
        }
    }

    int dist_after_move[NUM_PLAYERS]; // distance between pos and dest after the move move has been played
    for (unsigned int i = 0; i < NUM_PLAYERS; i++)
        dist_after_move[i] = p[0].board->num_edges + 1;
    struct move_t moves[4 * p[0].board->num_edges + 10]; // pour chaque arête, il y a au maximum 4 moyen de la couper et il y a au maximum 10 positions de movement possiblent pour le joueur
    for (unsigned int i = 0; i < (4 * p[0].board->num_edges + 10); i++)
        moves[i].t = NO_TYPE;
    int moves_size = 0; // les dix premières cases sont pour les mouvements
    int move_score = -(p[0].board->num_edges + 1);
    int tmp_score = 0;
    vertex_t v1;
    vertex_t v2;
    enum dir_t d = FIRST_DIR;

    // testing all wall placement move possible
    if (p[my_id].num_wall) {
        for (unsigned int i = 0; i < p[0].board->num_vertices; i++) {
            d = FIRST_DIR;
            while (d % (LAST_DIR + 1) != NO_EDGE) {
                v1 = graph_neighbor_direction(p[0].board, i, d % (LAST_DIR + 1));
                v2 = graph_neighbor_direction(p[0].board, i, d % LAST_DIR + 1);
                if (v1 != p[0].board->num_vertices && v2 != p[0].board->num_vertices) {
                    graph_remove_edge(p[0].board, (struct edge_t) { i, v1 });
                    graph_remove_edge(p[0].board, (struct edge_t) { i, v2 });
                    if (objectives_are_reachable()) {
                        for (unsigned int j = 0; j < NUM_PLAYERS; j++)
                            dist_after_move[j] = bfs(p[0].board, p[j].pos, dest[j]);
                        tmp_score = (dist_after_move[compute_next_player(my_id)] - dist_after_move[my_id]) - (dist[compute_next_player(my_id)] - dist[my_id]); // calcul pour deux joueurs, à adapté pour plus
                        if (tmp_score > move_score) {
                            move_score = tmp_score;
                            moves_size = 0;
                        }
                        if (tmp_score >= move_score) {
                            moves[moves_size].t = WALL;
                            moves[moves_size].e[0] = (struct edge_t) { i, v1 };
                            moves[moves_size].e[1] = (struct edge_t) { i, v2 };
                            moves_size = moves_size + 1;
                        }
                    }
                    graph_add_edge(p[0].board, i, v1, d % (LAST_DIR + 1));
                    graph_add_edge(p[0].board, i, v2, d % LAST_DIR + 1);
                }
                d = d + 1;
            }
        }
    }

    // testing all moving move possible
    vertex_t v = 0;
    vertex_t v_bis = 0; // is used in case the opponent is on the final vertex of the movement studied
    enum dir_t dirs[10];
    int dirs_size = 0;
    int nbr_of_wall_moves = moves_size;
    dist_after_move[compute_next_player(my_id)] = bfs(p[0].board, p[compute_next_player(my_id)].pos, dest[compute_next_player(my_id)]);
    for (unsigned int i = 1; i < NUM_DIRS + 1; i++) {
        v = graph_neighbor_direction(p[0].board, p[my_id].pos, i);
        if (v != p[0].board->num_vertices) {
            if (v == p[compute_next_player(my_id)].pos) {
                for (unsigned int j = 1; j < NUM_DIRS + 1; j++) {
                    v_bis = graph_neighbor_direction(p[0].board, v, j);
                    if (v_bis != p[0].board->num_vertices && v_bis != p[my_id].pos) {
                        dist_after_move[my_id] = bfs(p[0].board, v_bis, dest[my_id]);
                        tmp_score = (dist_after_move[compute_next_player(my_id)] - dist_after_move[my_id]) - (dist[compute_next_player(my_id)] - dist[my_id]);
                        if (tmp_score > move_score) {
                            move_score = tmp_score;
                            moves_size = nbr_of_wall_moves;
                            dirs_size = 0;
                        }
                        if (tmp_score >= move_score) {
                            moves[moves_size].t = MOVE;
                            moves[moves_size].m = v_bis;
                            dirs[dirs_size] = i;
                            moves_size = moves_size + 1;
                            dirs_size = dirs_size + 1;
                        }
                    }
                }
            } else {
                dist_after_move[my_id] = bfs(p[0].board, v, dest[my_id]);
                tmp_score = (dist_after_move[compute_next_player(my_id)] - dist_after_move[my_id]) - (dist[compute_next_player(my_id)] - dist[my_id]);
                if (tmp_score > move_score) {
                    move_score = tmp_score;
                    moves_size = nbr_of_wall_moves;
                    dirs_size = 0;
                }
                if (tmp_score >= move_score) {
                    moves[moves_size].t = MOVE;
                    moves[moves_size].m = v;
                    dirs[dirs_size] = i;
                    moves_size = moves_size + 1;
                    dirs_size = dirs_size + 1;
                }
            }
            if (next_dir_inv(p[my_id].dir) == i || i == p[my_id].dir || i == next_dir(p[my_id].dir)) {
                v = graph_neighbor_direction(p[0].board, v, i);
                if (v != p[0].board->num_vertices) {
                    if (v != p[compute_next_player(my_id)].pos) {
                        dist_after_move[my_id] = bfs(p[0].board, v, dest[my_id]);
                        tmp_score = (dist_after_move[compute_next_player(my_id)] - dist_after_move[my_id]) - (dist[compute_next_player(my_id)] - dist[my_id]);
                        if (tmp_score > move_score) {
                            move_score = tmp_score;
                            moves_size = nbr_of_wall_moves;
                            dirs_size = 0;
                        }
                        if (tmp_score >= move_score) {
                            moves[moves_size].t = MOVE;
                            moves[moves_size].m = v;
                            dirs[dirs_size] = i;
                            moves_size = moves_size + 1;
                            dirs_size = dirs_size + 1;
                        }
                        if (i == p[my_id].dir) {
                            v = graph_neighbor_direction(p[0].board, v, i);
                            if (v != p[0].board->num_vertices) {
                                if (v != p[compute_next_player(my_id)].pos) {
                                    dist_after_move[my_id] = bfs(p[0].board, v, dest[my_id]);
                                    tmp_score = (dist_after_move[compute_next_player(my_id)] - dist_after_move[my_id]) - (dist[compute_next_player(my_id)] - dist[my_id]);
                                    if (tmp_score > move_score) {
                                        move_score = tmp_score;
                                        moves_size = nbr_of_wall_moves;
                                        dirs_size = 0;
                                    }
                                    if (tmp_score >= move_score) {
                                        moves[moves_size].t = MOVE;
                                        moves[moves_size].m = v;
                                        dirs[dirs_size] = i;
                                        moves_size = moves_size + 1;
                                        dirs_size = dirs_size + 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /*
    printf("\n::::::::::::::::::::::::::::::::");
    printf("dest0: %d, dest1: %d\n", dest[0], dest[1]);
    printf("\nvalue : %d\n", move_score);
    for (int i = 0; i < moves_size; i++) {
        printf("%d ", i);
        if (moves[i].t == MOVE)
            printf("dir: %d\n", dirs[i - nbr_of_wall_moves]);
        move_print(moves[i]);
    }
    printf("::::::::::::::::::::::::::::::::::\n");
    printf("\nT : direction : %d\ndirs :", p[my_id].dir);
    for (int i = 0; i < dirs_size; i++)
        printf("%d", dirs[i]);
    */

    int r = rand() % (moves_size);
    struct move_t final_move = moves[r];
    final_move.c = my_id;
    if (final_move.t == WALL) {
        final_move.m = p[0].board->num_vertices + 1;
        graph_remove_edge(p[0].board, final_move.e[0]);
        graph_remove_edge(p[0].board, final_move.e[1]);
        p[my_id].num_wall = p[my_id].num_wall - 1;
    } else if (final_move.t == MOVE) {
        p[my_id].pos = final_move.m;
        p[my_id].dir = dirs[r - nbr_of_wall_moves];
        final_move.e[0] = (struct edge_t) { p[0].board->num_vertices + 1, p[0].board->num_vertices + 2 };
        final_move.e[1] = (struct edge_t) { p[0].board->num_vertices + 3, p[0].board->num_vertices + 4 };
    }

    // move_print(final_move);

    for (unsigned int j = 0; j < p[0].board->num_objectives; j++)
        if (p[my_id].pos == p[0].board->objectives[j])
            p[my_id].objectives_reached[j] = 1;

    return final_move;
}

void finalize()
{
    graph_free(&(p[0].board));
    for (int i = 0; i < NUM_PLAYERS; i++) {
        free(p[i].objectives_reached);
    }
    return;
}
