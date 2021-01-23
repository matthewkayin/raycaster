#pragma once

#include "vector.h"

#include <stdbool.h>

static const int SPRITE_OBJECT = 0;
static const int SPRITE_PROJECTILE = 1;

// Represents any 2d image rendered in the world
typedef struct sprite{

    int image;
    int type;
    vector position;
    vector velocity;
} sprite;

typedef struct State{

    vector player_position;
    vector player_move_dir;
    vector player_direction;
    vector player_camera;
    float player_rotate_dir;

    int* map;
    int* map_ceil;
    int* map_floor;
    int map_width;
    int map_height;

    sprite* sprites;
    int sprite_count;
    int sprite_capacity;
} State;

State* state_init();
void state_update(State* state, float delta);

void player_shoot(State* state);

void sprite_create(State* state, sprite to_create);
void sprite_delete(State* state, int index);

void raycast(State* state, vector origin, vector ray, float* wall_dist, int* texture_x, bool* x_sided, int* texture);
