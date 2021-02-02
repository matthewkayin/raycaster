#pragma once

#include "vector.h"
#include "map.h"

#include <stdbool.h>
#include <stdlib.h>

// Represents any 2d image rendered in the world
typedef struct sprite{

    int image;
    vector position;
} sprite;

typedef struct animation{

    int low_frame;
    int high_frame;
    float duration;
    float timer;
} animation;

typedef struct projectile{

    sprite image;
    vector velocity;
} projectile;

typedef struct enemy{

    sprite image;
    vector velocity;
    animation anim;
} enemy;

typedef struct State{

    vector player_position;
    vector player_move_dir;
    vector player_direction;
    vector player_camera;
    float player_rotate_dir;

    map* map;

    sprite* objects;
    int object_count;
    int object_capacity;

    projectile* projectiles;
    int projectile_count;
    int projectile_capacity;

    enemy* enemies;
    int enemy_count;
    int enemy_capacity;
} State;

State* state_init();
void state_update(State* state, float delta);

void player_shoot(State* state);

void vector_push(void* vector, void* to_push, int* count, int* capacity, size_t unit_size);
void vector_delete(void* vector, int index, int* count, size_t unit_size);

void raycast_line_of_sight(State* state, vector origin, vector target, vector* hit);
void render_raycast(State* state, vector origin, vector ray, float* wall_dist, int* texture_x, bool* x_sided, int* texture);
