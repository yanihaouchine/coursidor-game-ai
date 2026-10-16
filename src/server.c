#include "board.h"
#include "graph.h"
#include "move.h"
#include <dlfcn.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int bfs(struct graph_t* graph, vertex_t start, vertex_t end);

// Structure containing the pointers of the players functions
struct player_func {
    void* player;
    char* (*get_player_name)(void);
    void (*initialize)(unsigned int, struct graph_t*);
    struct move_t (*play)(const struct move_t);
    void (*finalize)(void);
    int num_wins;
    struct graph_t* graph_player;
    vertex_t initial_position;
    vertex_t current_position;
    int* objectives_reached;
    enum dir_t previous_dir;
    int num_wall;
    int lost;
};

struct player_func players[NUM_PLAYERS];

void _exit_if_error(void* p)
{
    char* error;
    if ((error = dlerror()) != NULL) {
        fputs(error, stderr);
        dlclose(p);
        exit(1);
    }
}

// Returns a function obtained with dlsym and tests if there is no error
void* _get_func(void* p, char* s)
{
    void* func = dlsym(p, s);
    _exit_if_error(p);
    return func;
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

/* Returns a boolean that shows if the objectives are still reachable by the player given*/
int objectives_are_reachable(struct graph_t* g, struct player_func player)
{
    int bool_res = 1;
    unsigned int i = 0;
    while (bool_res && i < g->num_objectives) {
        if (!player.objectives_reached[i])
            bool_res = bool_res && (bfs(g, player.current_position, g->objectives[i]) != -1);
        i = i + 1;
    }
    bool_res = bool_res && (bfs(g, player.current_position, player.initial_position) != -1);
    return bool_res;
}

// Tests if a move is invalid
// Returns a boolean
int is_invalid(struct move_t current_move, struct player_func* player, struct player_func players[], struct graph_t* g, enum player_color_t current_player)
{
  //return 0;
    if (current_move.t == MOVE && current_move.c == current_player) // if move type is MOVE and the color of the move correponds to current_player
    {
        // Checks if the place the player wants to go is not occupied
        for (int i = 0; i < NUM_PLAYERS; i++)
            if (current_move.m == players[i].current_position)
                return 1;
        // If the vertex the player wants to go is not directly connected to the current position
        if (graph_exist_edge(g, (struct edge_t) { player->current_position, current_move.m }) == NO_EDGE) {
            vertex_t current_vertex_1 = player->current_position;
            vertex_t current_vertex_2 = player->current_position;
            vertex_t current_vertex_3 = player->current_position;
            // Checks if the vertex the player wants to go is at a distance max of 3 and is in the same direction than the previous direction taken
            for (int k = 0; k < 3; k++) {
                vertex_t v = graph_neighbor_direction(g, current_vertex_1, player->previous_dir);
                if (v == g->num_vertices)
                    break;
                else if (v == current_move.m)
                    return 0;

                current_vertex_1 = v;
            }
            // Checks if the vertex the player wants to go is at a distance max of 2 and is at a 30° angle from the previous direction in the counterclockwise  and in the clockwise order
            for (int j = 0; j < 2; j++) {
                vertex_t v1 = graph_neighbor_direction(g, current_vertex_2, next_dir(player->previous_dir));
                vertex_t v2 = graph_neighbor_direction(g, current_vertex_3, next_dir_inv(player->previous_dir));
                if (v1 != g->num_vertices && v1 == current_move.m) {
                    player->previous_dir = next_dir(player->previous_dir);
                    return 0;
                }
                if (v2 != g->num_vertices && v2 == current_move.m) {
                    player->previous_dir = next_dir_inv(player->previous_dir);
                    return 0;
                }

                if (v1 != g->num_vertices)
                    current_vertex_2 = v1;
                if (v2 != g->num_vertices)
                    current_vertex_3 = v2;
            }
            // Checks if the vertex the player wants to go is at a distance 2 and is behind a another player
            for (unsigned int d = 1; d < NUM_DIRS + 1; d++) {
                vertex_t v = graph_neighbor_direction(g, player->current_position, d);
                if (v != g->num_vertices) {
                    int is_occupied = 0;
                    for (int i = 0; i < NUM_PLAYERS; i++)
                        if (v == players[i].current_position)
                            is_occupied = 1;
                    if (is_occupied) {
                        for (unsigned int dir = 1; dir < NUM_DIRS + 1; dir++) {
                            if (graph_neighbor_direction(g, v, dir) == current_move.m)
                                return 0;
                        }
                    }
                }
            }
            return 1;
        } else {
            player->previous_dir = graph_exist_edge(g, (struct edge_t) { player->current_position, current_move.m });
            return 0;
        }
    } else if (current_move.t == WALL && current_move.c == current_player && players[current_player].num_wall) { // If move type is WALL
        for (int i = 0; i < NUM_PLAYERS; i++)
            if (!objectives_are_reachable(g, players[i]))
                return 1;
        return 0;
    } else {
        if (!players[current_player].num_wall)
            printf("Player %d placed to many walls\n", current_player);
        return 1;
    }
    return 0;
}

int game_over(struct move_t m, struct graph_t* g, struct player_func players[], int in_game, enum player_color_t current_player)
{
    // If there is only one player left
    if (in_game == 1) {
        // Research of the remaining player in players
        for (enum player_color_t c = 0; c < NUM_PLAYERS; c = compute_next_player(c)) {
            if (!players[c].lost) {
                players[c].num_wins = players[c].num_wins + 1;
                printf("\033[1;31m%s won\033[0m\n", players[c].get_player_name());
                return 1;
            }
        }
    }
    int count = 0;
    for (unsigned int i = 0; i < g->num_objectives; i++) {
        if (m.m == g->objectives[i]) {
            players[current_player].objectives_reached[i] = 1;
        }
        count = count + players[current_player].objectives_reached[i];
    }

    // If the current player reached all the objectives and is back to its initial position
    if (count == (int)g->num_objectives && m.m == g->start[current_player]) {
        players[current_player].num_wins = players[current_player].num_wins + 1;
        printf("\033[1;31m%s won\033[0m\n", players[current_player].get_player_name());
        return 1;
    }
    return 0;
}

void game(int graph_size, int max_turn, char graph_type, enum player_color_t first_player, int print)
{
    enum player_color_t current_player = first_player;
    struct graph_t* g;
    if (graph_type == 'T') {
        g = graph_make_triangular(graph_size);
    } else if (graph_type == 'C') {
        g = graph_make_circular(graph_size);
    } else if (graph_type == 'H') {
        g = graph_make_holey(graph_size);
    }

    int max_wall = (g->num_edges / 16) + ((g->num_edges % 16) > 0);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].graph_player = malloc(sizeof(struct graph_t));
        graph_cpy(players[i].graph_player, g);
        players[i].initial_position = g->start[i];
        players[i].current_position = players[i].initial_position;
        printf("Initial position player %d: %d \n", i, g->start[i]);
        players[i].previous_dir = NO_EDGE;
        players[i].initialize(i, players[i].graph_player);
        players[i].num_wall = max_wall;
        players[i].lost = 0;

        players[i].objectives_reached = malloc(g->num_objectives * sizeof(int));
        for (int k = 0; k < (int)g->num_objectives; k++)
            players[i].objectives_reached[k] = 0;

        printf("Name player %d : %s\n", i, players[i].get_player_name());
    }

    struct move_t current_move = { .t = NO_TYPE, .c = compute_next_player(current_player), .m = players[compute_next_player(current_player)].current_position };

    int turn_count = 0;
    int in_game = NUM_PLAYERS;
    struct edge_t removed[g->num_edges];
    int removed_size = 0;

    graph_dot(g, removed, removed_size, players[0].current_position, players[1].current_position, "game.dot", 0);
    while (turn_count < NUM_PLAYERS * max_turn) {
        // printf("Player %d plays\n", current_player);
        //  struct move_t previous_move = current_move;
        current_move = players[current_player].play(current_move);
        if (print) {
            printf("\n");
            move_print(current_move);
        }
        if (is_invalid(current_move, &players[current_player], players, g, current_player)) {
            printf("Player %d made an invalid move\n", current_player);
            printf("Player %d lost\n", current_player);
            players[current_player].lost = 1;
            in_game = in_game - 1;
        }
        if (game_over(current_move, g, players, in_game, current_player)) {
            break;
        }
        if (current_move.t == WALL) {
            graph_remove_edge(g, current_move.e[0]);
            graph_remove_edge(g, current_move.e[1]);
            removed[removed_size] = current_move.e[0];
            removed[removed_size + 1] = current_move.e[1];
            removed_size = removed_size + 2;
            players[current_player].num_wall = players[current_player].num_wall - 1;
        } else if (current_move.t == MOVE) {
            players[current_player].current_position = current_move.m;
        }
        current_player = compute_next_player(current_player);
        graph_dot(g, removed, removed_size, players[0].current_position, players[1].current_position, "game.dot", 1);
        turn_count = turn_count + 1;
    }

    for (int i = 0; i < NUM_PLAYERS; i++) {
        free(players[i].objectives_reached);
        players[i].finalize();
    }

    graph_free(&g);
}

int main(int argc, char** argv)
{
    int graph_size = 6;
    int max_turn = 100;
    char graph_type = 'T';
    int print = 0;
    int num_games = 1;
    int seed = time(NULL);

    int opt;
    while ((opt = getopt(argc, argv, "m:t:M:pg:s:")) != -1) {
        switch (opt) {
        case 'm':
            graph_size = atoi(optarg);
            break;
        case 't':
            if (strlen(optarg) != 1 || (optarg[0] != 'T' && optarg[0] != 'C' && optarg[0] != 'H')) {
                fprintf(stderr, "Graph type can only be T, C or H\n");
                exit(EXIT_FAILURE);
            }
            graph_type = optarg[0];
            break;
        case 'M':
            max_turn = atoi(optarg);
            break;
        case 'p':
            print = 1;
            break;
        case 'g':
            num_games = atoi(optarg);
            break;
        case 's':
            seed = atoi(optarg);
            break;
        default: // Unknown option
            fprintf(stderr, "Usage: %s [-m cycle size] [-t graph type] [-M max move number]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Creation of the graph according to the type given
    if (graph_type == 'T') {
        if (graph_size < 2) {
            fprintf(stderr, "For a triangular graph, m has to be >=2\n");
            exit(EXIT_FAILURE);
        }
    } else if (graph_type == 'C') {
        if (graph_size < 3) {
            fprintf(stderr, "For a circular graph, m has to be >=3");
            exit(EXIT_FAILURE);
        }
    } else if (graph_type == 'H') {
        if (graph_size < 6 || graph_size % 3 != 0) {
            fprintf(stderr, "For a holey graph, m has to be >=6 and has to be a multiple of 3");
            exit(EXIT_FAILURE);
        }
    }

    srand(seed);
    enum player_color_t current_player = rand() % NUM_PLAYERS;

    int count = 0;
    for (int i = optind; i < argc; i++) {
        if (count < NUM_PLAYERS) {
            players[count].player = dlopen(argv[optind + current_player], RTLD_LAZY); // Storage of the pointers to the libraries in players
            if (!players[count].player) {
                fputs(dlerror(), stderr);
                exit(1);
            }
            count++;
            current_player = compute_next_player(current_player);
        }
    }

    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].get_player_name = _get_func(players[i].player, "get_player_name");
        players[i].initialize = _get_func(players[i].player, "initialize");
        players[i].play = _get_func(players[i].player, "play");
        players[i].finalize = _get_func(players[i].player, "finalize");
        players[i].num_wins = 0;
    }

    for (int i = 0; i < num_games; i++) {
        printf("\tGame %d\n", i + 1);
        game(graph_size, max_turn, graph_type, current_player, print);
    }
    printf("\n\tWins on %d games:\n", num_games);
    for (int i = 0; i < NUM_PLAYERS; i++)
        printf("\t   %s: %d\n", players[i].get_player_name(), players[i].num_wins);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        dlclose(players[i].player);
    }

    return 0;
}
