#pragma once

#include "vector.h"
#include "map.h"
#include "enemy.h"

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

    int image;
    vector position;
    vector velocity;
} projectile;

typedef struct State{

    vector player_position;
    vector player_move_dir;
    vector player_direction;
    vector player_camera;
    float player_rotate_dir;

    int player_animation_frame;
    float player_animation_timer;

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
void check_wall_collisions(State* state, vector* mover_position, vector mover_last_pos, vector velocity);
void check_sprite_collision(vector* mover_position, vector mover_last_pos, vector velocity, vector object);

int get_player_animation_offset_x(State* state);
int get_player_animation_offset_y(State* state);

void player_shoot(State* state);

bool ray_intersects(State* state, vector origin, vector ray, vector target);
void render_raycast(State* state, vector origin, vector ray, float* wall_dist, int* texture_x, bool* x_sided, int* texture);
