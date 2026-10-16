#include "board.h"
#include "move.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
struct i_player_t;
// les joueurs imaginaires doivent être vu comme une représentation interne de l'imaginaire du joueur...
struct i_player_t* create_i_player(void* real_player);

void free_i_player(struct i_player_t* i_p);

int is_winner(struct i_player_t* p);

struct i_player_t* if_I_play(struct i_player_t* i_p, struct move_t* mv);

struct graph_t *get_board_i_player(struct i_player_t *i_p);

vertex_t get_pos_i_player(struct i_player_t *i_p);

unsigned int get_id_i_player(struct i_player_t *i_p);

enum dir_t get_last_dir_i_p(struct i_player_t *i_p);

int *get_obj_i_p(struct i_player_t *i_p);

int get_num_wall_i_p(struct i_player_t *i_p);

struct move_t get_last_move_i_p(struct i_player_t *i_p);
