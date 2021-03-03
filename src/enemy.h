#pragma once

#include "vector.h"

#include <stdbool.h>

typedef struct enemy_data{
    char name[32];

    int move_frames;
    float move_duration;

    int attack_frames;
    float attack_duration;
    int attack_danger_frame;
    int attack_safe_frame;

    float speed;
    float attack_speed;
    float attack_radius;
} enemy_data;

typedef enum enemy_name{
    ENEMY_SLIME,
    NUM_ENEMIES
} enemy_name;

typedef enum enemy_state{
    ENEMY_STATE_IDLE,
    ENEMY_STATE_MOVING,
    ENEMY_STATE_ATTACKING,
    ENEMY_STATE_KNOCKBACK,
    ENEMY_STATE_FROZEN
} enemy_state;

extern enemy_data* enemy_info;
void enemy_data_init();

typedef struct enemy{
    enemy_name name;

    enemy_state state;
    int current_frame;
    float animation_timer;
    vector position;
    vector velocity;
} enemy;

void enemy_animation_update(enemy* the_enemy, float delta);
bool enemy_has_hurtbox(enemy* the_enemy);
void enemy_freeze(enemy* the_enemy);
