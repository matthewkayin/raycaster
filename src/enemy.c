#include "enemy.h"

#include <stdlib.h>

enemy_data* enemy_info;

void enemy_data_init(){

    enemy_info = malloc(sizeof(enemy_data) * (int)NUM_ENEMIES);

    enemy_info[ENEMY_SLIME] = (enemy_data){

        .name = "slime",
        .move_frames = 5,
        .move_duration = 6.0,
        .attack_frames = 21,
        .attack_danger_frame = 8,
        .attack_safe_frame = 15,
        .attack_duration = 0.25,
        .speed = 0.02,
        .attack_radius = 2.0
    };
}
