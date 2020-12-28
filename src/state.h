#pragma once

#include "vector.h"

#include <stdbool.h>

typedef struct State{

    vector player_position;
    vector player_move_dir;
    vector player_direction;
    int player_rotate_dir;

    bool* map;
    int map_width;
    int map_height;
} State;

State* state_init();
void state_update(State* state, float delta);
