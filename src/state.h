#pragma once

#include "vector.h"

#include <stdbool.h>

typedef struct State{

    vector player_position;
    vector player_move_dir;
    vector player_direction;
    vector player_camera;
    float player_rotate_dir;

    int* map;
    int map_width;
    int map_height;
} State;

State* state_init();
void state_update(State* state, float delta);

void raycast(State* state, vector origin, vector ray, float* wall_dist, int* texture_x, bool* x_sided, int* texture);
