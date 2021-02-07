#pragma once

#include "vector.h"

typedef struct enemy_data{
    char name[32];
    int move_frames;
    float move_duration;
    int attack_frames;
    int attack_danger_frame;
    int attack_safe_frame;
    float attack_duration;
    float speed;
    float attack_radius;
} enemy_data;

typedef enum enemy_name{
    ENEMY_SLIME,
    NUM_ENEMIES
} enemy_name;

extern enemy_data* enemy_info;
void enemy_data_init();

typedef struct enemy{
    enemy_name name;
    int current_frame;
    float animation_timer;
    vector position;
} enemy;
