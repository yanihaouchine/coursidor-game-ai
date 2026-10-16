#include "board.h"
#include "graph.h"
#include "move.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int bfs(struct graph_t* graph, vertex_t start, vertex_t end);

// Structure contenant les informations des joueurs et les pipes pour communiquer
struct player_func {
    pid_t pid;                       // PID du processus du joueur
    int to_player[2];                // Pipe pour envoyer des données au joueur
    int from_player[2];              // Pipe pour recevoir des données du joueur
    char name[256];                  // Nom du joueur
    struct graph_t* graph_player;    // Graphe du joueur
    vertex_t initial_position;       // Position initiale
    vertex_t current_position;       // Position actuelle
    int* objectives_reached;         // Objectifs atteints
    enum dir_t previous_dir;         // Direction précédente
    int num_wall;                    // Nombre de murs restants
    int lost;                        // Joueur a perdu
    int num_wins;                    // Nombre de victoires
};

struct player_func players[NUM_PLAYERS];

// Tue les processus des joueurs
void kill_players() {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (players[i].pid > 0) {
            kill(players[i].pid, SIGTERM);
        }
    }
}

// Routine d'erreur - ferme les pipes et quitte
void exit_if_error(int condition, const char* message) {
    if (condition) {
        fprintf(stderr, "%s\n", message);
        kill_players();
        exit(EXIT_FAILURE);
    }
}

// Nettoie les ressources avant de quitter
void cleanup() {
    kill_players();
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (players[i].to_player[0] >= 0) close(players[i].to_player[0]);
        if (players[i].to_player[1] >= 0) close(players[i].to_player[1]);
        if (players[i].from_player[0] >= 0) close(players[i].from_player[0]);
        if (players[i].from_player[1] >= 0) close(players[i].from_player[1]);
    }
}

// Donne le joueur suivant étant donné le joueur actuel
int compute_next_player(int current_player) {
    return (current_player + 1) % NUM_PLAYERS;
}

/* Une fonction pour déterminer la direction suivante dans l'ordre horaire */
static inline enum dir_t next_dir_inv(enum dir_t d) {
    return (d == 0) ? 0 : (d == 6) ? FIRST_DIR : (d + 1);
}

/* Renvoie un booléen qui indique si les objectifs sont toujours accessibles par le joueur donné */
int objectives_are_reachable(struct graph_t* g, struct player_func player) {
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

// Teste si un mouvement est invalide
// Renvoie un booléen
int is_invalid(struct move_t current_move, struct player_func* player, struct player_func players[], struct graph_t* g, enum player_color_t current_player) {
    //return 0;
    if (current_move.t == MOVE && current_move.c == current_player) { // si le type de mouvement est MOVE et la couleur du mouvement correspond au joueur actuel
        // Si le sommet où le joueur veut aller n'est pas directement connecté à la position actuelle
        if (graph_exist_edge(g, (struct edge_t) { player->current_position, current_move.m }) == NO_EDGE) {
            vertex_t current_vertex_1 = player->current_position;
            vertex_t current_vertex_2 = player->current_position;
            // Vérifie si le sommet où le joueur veut aller est à une distance maximale de 3 et est dans la même direction que la direction précédente
            for (int k = 0; k < 3; k++) {
                vertex_t v = graph_neighbor_direction(g, current_vertex_1, player->previous_dir);
                if (v == g->num_vertices)
                    break;
                else if (v == current_move.m)
                    return 0;
                current_vertex_1 = v;
            }
            // Vérifie si le sommet où le joueur veut aller est à une distance maximale de 2 et est à un angle de 30° par rapport à la direction précédente dans l'ordre antihoraire et horaire
            unsigned int dir = 1;
            while (dir < NUM_DIRS + 1) {
                current_vertex_2 = player->current_position;
                for (int j = 0; j < 2; j++) {
                    vertex_t v = graph_neighbor_direction(g, current_vertex_2, dir);
                    if (v == g->num_vertices)
                        break;
                    int there_is_opponent = 0;
                    for (int i = 0; i < NUM_PLAYERS; i++)
                        if (current_vertex_2 == players[i].current_position)
                            there_is_opponent = 1;
                    if (v != g->num_vertices && v == current_move.m && (dir == next_dir(player->previous_dir) || dir == next_dir_inv(player->previous_dir) || there_is_opponent)) {
                        player->previous_dir = dir;
                        return 0;
                    }
                    if (v != g->num_vertices)
                        current_vertex_2 = v;
                }
                dir = dir + 1;
            }
            return 1;
        } else {
            // Vérifie si la place où le joueur veut aller n'est pas occupée
            for (int i = 0; i < NUM_PLAYERS; i++)
                if (current_move.m == players[i].current_position)
                    return 1;
            player->previous_dir = graph_exist_edge(g, (struct edge_t) { player->current_position, current_move.m });
            return 0;
        }
    } else if (current_move.t == WALL && current_move.c == current_player && players[current_player].num_wall) { // Si le type de mouvement est WALL
        for (int i = 0; i < NUM_PLAYERS; i++)
            if (!objectives_are_reachable(g, players[i]))
                return 1;
        return 0;
    } else {
        if (!players[current_player].num_wall)
            printf("Player %d placed too many walls\n", current_player);
        return 1;
    }
    return 0;
}

int game_over(struct move_t m, struct graph_t* g, struct player_func players[], int in_game, enum player_color_t current_player) {
    // S'il n'y a plus qu'un joueur
    if (in_game == 1) {
        // Recherche du joueur restant dans players
        for (enum player_color_t c = 0; c < NUM_PLAYERS; c = compute_next_player(c)) {
            if (!players[c].lost) {
                players[c].num_wins = players[c].num_wins + 1;
                printf("\033[1;31m%s won\033[0m\n", players[c].name);
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
    // Si le joueur actuel a atteint tous les objectifs et est de retour à sa position initiale
    if (count == (int)g->num_objectives && m.m == g->start[current_player]) {
        players[current_player].num_wins = players[current_player].num_wins + 1;
        printf("\033[1;31m%s won\033[0m\n", players[current_player].name);
        return 1;
    }
    return 0;
}

// Initialise la communication avec les joueurs
void initialize_player(int player_id, struct graph_t* g) {
    struct player_func* player = &players[player_id];
    
    // Envoie l'ID du joueur
    write(player->to_player[1], &player_id, sizeof(int));
    
    // Sérialisation et envoi du graphe
    write(player->to_player[1], &g->num_vertices, sizeof(unsigned int));
    write(player->to_player[1], &g->num_edges, sizeof(unsigned int));
    write(player->to_player[1], &g->num_directions, sizeof(unsigned int));
    write(player->to_player[1], &g->num_objectives, sizeof(unsigned int));
    
    // Envoie des arêtes
    for (unsigned int i = 0; i < g->num_vertices; i++) {
        for (unsigned int j = 0; j < g->num_directions; j++) {
            vertex_t neighbor = g->t[i][j];
            write(player->to_player[1], &neighbor, sizeof(vertex_t));
        }
    }
    
    // Envoie des positions de départ et objectifs
    for (int i = 0; i < NUM_PLAYERS; i++) {
        write(player->to_player[1], &g->start[i], sizeof(vertex_t));
    }
    for (unsigned int i = 0; i < g->num_objectives; i++) {
        write(player->to_player[1], &g->objectives[i], sizeof(vertex_t));
    }
    
    // Lecture du nom du joueur
    int name_len;
    read(player->from_player[0], &name_len, sizeof(int));
    read(player->from_player[0], player->name, name_len);
    player->name[name_len] = '\0';
}

// Fait jouer le joueur actuel
struct move_t play_move(int player_id, struct move_t previous_move) {
    struct player_func* player = &players[player_id];
    struct move_t current_move;
    
    // Envoie le mouvement précédent
    write(player->to_player[1], &previous_move, sizeof(struct move_t));
    
    // Attend et lit le nouveau mouvement
    read(player->from_player[0], &current_move, sizeof(struct move_t));
    
    return current_move;
}

// Finalise le joueur
void finalize_player(int player_id) {
    struct player_func* player = &players[player_id];
    int command = -1; // Code pour finaliser
    
    // Envoie la commande de finalisation
    write(player->to_player[1], &command, sizeof(int));
    
    // Ferme les pipes
    close(player->to_player[1]);
    close(player->from_player[0]);
    
    // Attend la fin du processus
    int status;
    waitpid(player->pid, &status, 0);
}

void game(int graph_size, int max_turn, char graph_type, enum player_color_t first_player, int print) {
    enum player_color_t current_player = first_player;
    struct graph_t* g;
    if (graph_type == 'T') {
        g = graph_make_triangular(graph_size);
    }
    int max_wall = (g->num_edges / 16) + ((g->num_edges % 16) > 0);
    
    // Initialisation des joueurs
    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].graph_player = malloc(sizeof(struct graph_t));
        graph_cpy(players[i].graph_player, g);
        players[i].initial_position = g->start[i];
        players[i].current_position = players[i].initial_position;
        printf("Initial position player %d: %d \n", i, g->start[i]);
        players[i].previous_dir = NO_EDGE;
        initialize_player(i, players[i].graph_player);
        players[i].num_wall = max_wall;
        players[i].lost = 0;
        players[i].objectives_reached = malloc(g->num_objectives * sizeof(int));
        for (int k = 0; k < (int)g->num_objectives; k++)
            players[i].objectives_reached[k] = 0;
        printf("Name player %d : %s\n", i, players[i].name);
    }
    
    struct move_t current_move = { .t = NO_TYPE, .c = compute_next_player(current_player), .m = players[compute_next_player(current_player)].current_position };
    int turn_count = 0;
    int in_game = NUM_PLAYERS;
    struct edge_t removed[g->num_edges];
    int removed_size = 0;
    graph_dot(g, removed, removed_size, players[0].current_position, players[1].current_position, "game.dot", 0);
    
    while (turn_count < NUM_PLAYERS * max_turn) {
        // printf("Player %d plays\n", current_player);
        // struct move_t previous_move = current_move;
        current_move = play_move(current_player, current_move);
        
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
        finalize_player(i);
    }
    
    graph_free(&g);
}

int main(int argc, char** argv) {
    int graph_size = 6;
    int max_turn = 100;
    char graph_type = 'T';
    int print = 0;
    int num_games = 1;
    int opt;
    
    while ((opt = getopt(argc, argv, "m:t:M:pg:")) != -1) {
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
        default: // Option inconnue
            fprintf(stderr, "Usage: %s [-m cycle size] [-t graph type] [-M max move number] joueur1 joueur2\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    // Vérification des arguments
    if (graph_type == 'T') {
        if (graph_size < 2) {
            fprintf(stderr, "For a triangular graph, m has to be >=2\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Vérifier si nous avons assez d'arguments pour les joueurs
    if (argc - optind < NUM_PLAYERS) {
        fprintf(stderr, "Not enough player executables provided\n");
        exit(EXIT_FAILURE);
    }
    
    // Initialiser les pipes et lancer les processus des joueurs
    srand(time(NULL));
    enum player_color_t current_player = rand() % NUM_PLAYERS;
    
    for (int i = 0; i < NUM_PLAYERS; i++) {
        // Initialisation des pipes
        pipe(players[i].to_player);
        pipe(players[i].from_player);
        
        // Création du processus fils
        players[i].pid = fork();
        
        if (players[i].pid == -1) {
            perror("fork");
            cleanup();
            exit(EXIT_FAILURE);
        } else if (players[i].pid == 0) {
            // Code du processus fils
            
            // Fermer les extrémités non utilisées des pipes
            close(players[i].to_player[1]);  // Ferme l'écriture vers le joueur
            close(players[i].from_player[0]); // Ferme la lecture depuis le joueur
            
            // Rediriger stdin vers to_player[0]
            dup2(players[i].to_player[0], STDIN_FILENO);
            close(players[i].to_player[0]);
            
            // Rediriger stdout vers from_player[1]
            dup2(players[i].from_player[1], STDOUT_FILENO);
            close(players[i].from_player[1]);
            
            // Exécuter le programme du joueur
            execl(argv[optind + (current_player + i) % NUM_PLAYERS], argv[optind + (current_player + i) % NUM_PLAYERS], NULL);
            
            // Si execl échoue
            perror("execl");
            exit(EXIT_FAILURE);
        } else {
            // Code du processus parent
            
            // Fermer les extrémités non utilisées des pipes
            close(players[i].to_player[0]);  // Ferme la lecture vers le joueur
            close(players[i].from_player[1]); // Ferme l'écriture depuis le joueur
            
            // Rendre les descripteurs de fichier non bloquants
            fcntl(players[i].from_player[0], F_SETFL, O_NONBLOCK);
        }
    }
    
    // Configuration du handler de signal pour nettoyer avant de quitter
    atexit(cleanup);
    
    // Jouer les parties
    current_player = BLACK;
    for (int i = 0; i < num_games; i++) {
        game(graph_size, max_turn, graph_type, current_player, print);
    }
    
    // Affichage des résultats
    printf("\n\tWins on %d games:\n", num_games);
    for (int i = 0; i < NUM_PLAYERS; i++) {
        printf("\t   %s: %d\n", players[i].name, players[i].num_wins);
    }
    
    return 0;
}
