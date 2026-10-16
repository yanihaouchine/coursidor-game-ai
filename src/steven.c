#include <assert.h>

#include "board.h"
#include "graph.h"
#include "move.h"
#include "player.h"

struct player_t {
    const char* name;
    unsigned int id;
    struct graph_t* board;
    enum dir_t dir;
    vertex_t pos;
    vertex_t pos_opponent;
};

struct player_t p;

char const* get_player_name()
{
    return "Steven";
}

void initialize(unsigned int player_id, struct graph_t* graph)
{
    assert(player_id == 0 || player_id == 1);
    p.name = get_player_name();
    p.id = player_id;
    p.dir = NO_EDGE;
    p.pos = graph->start[player_id];
    p.pos_opponent = graph->start[(player_id + 1) % NUM_PLAYERS];
    p.board = graph;
}

struct move_t play(const struct move_t previous_move)
{
    if (previous_move.t == WALL) {
        graph_remove_edge(p.board, previous_move.e[0]);
        graph_remove_edge(p.board, previous_move.e[1]);
    }
    if (previous_move.t == MOVE) {
    }

    struct move_t new_move;
    new_move.c = p.id;
    new_move.t = MOVE;
    new_move.m = p.board->num_vertices;
    vertex_t v = graph_neighbor_direction(p.board, p.pos, E);
    if (v != p.board->num_vertices && previous_move.m != v) {
        new_move.m = v;
        p.pos = v;
        p.dir = E;
    } else {
        v = graph_neighbor_direction(p.board, p.pos, W);
        if (v != p.board->num_vertices && previous_move.m != v) {
            new_move.m = v;
            p.pos = v;
            p.dir = W;
        } else {
            for (int i = 1; i < NUM_DIRS + 1; i++) {
                v = graph_neighbor_direction(p.board, p.pos, i);
                if (v != p.board->num_vertices && previous_move.m != v) {
                    new_move.m = v;
                    p.pos = v;
                    p.dir = i;
                    break;
                }
            }
        }
    }

    if (new_move.m == p.board->num_vertices) {
        new_move.m = graph_neighbor_direction(p.board, previous_move.m, graph_exist_edge(p.board, (struct edge_t) { p.pos, previous_move.m }));
    }

    return new_move;
}

void finalize()
{
    graph_free(&(p.board));
    return;
}
