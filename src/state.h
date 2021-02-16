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
    vector player_velocity;
    vector player_move_dir;
    vector player_direction;
    vector player_camera;
    float player_rotate_dir;

    float player_knockback_timer;

    int player_animation_state;
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

// Init
State* state_init();

// Updates
void state_update(State* state, float delta);
void enemy_update(State* state, int index, float delta);

// Collision helpers / handlers
bool in_wall(State* state, vector v);
bool rect_in_wall(State* state, vector rect_pos, vector rect_dim);
void check_wall_collisions(State* state, vector* mover_position, vector mover_last_pos, vector velocity);
void check_rect_wall_collisions(State* state, vector* mover_position, vector mover_last_pos, vector velocity, float rect_size);
void check_sprite_collision(vector* mover_position, vector mover_last_pos, vector velocity, vector object, float collision_dist);

// Player
vector player_get_animation_offset(State* state); // returns the offset of the player animation, used to create the head bobbing effect when walking
void player_knockback(State* state, vector impact_vector); // knocks back the player with the force of the impact vector
void player_cast_start(State* state);
void player_cast_kinetic(State* state); // causes player to cast the "kinetic" spell

// Raycasting
int hits_wall(State* state, vector v); // returns true if point touches a wall on the map
bool hit_tile(vector v, vector tile); // returns true if point touches an edge of a given tile
bool ray_intersects(State* state, vector origin, vector ray, vector target); // casts a ray and returns true if it intersects with the target vector
void render_raycast(State* state, vector origin, vector ray, float* wall_dist, int* texture_x, bool* x_sided, int* texture); // casts a ray and returns info needed for rendering
