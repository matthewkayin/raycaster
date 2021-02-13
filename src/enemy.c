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
        .attack_duration = 5.0,
        .speed = 0.02,
        .attack_radius = 2.0
    };
}

void enemy_animation_update(enemy* the_enemy, float delta){

    if(the_enemy->state == ENEMY_STATE_IDLE){

        the_enemy->animation_timer = 0;
        the_enemy->current_frame = 0;
        return;
    }

    enemy_data* the_enemy_info = &enemy_info[the_enemy->name];
    float target_duration;
    int max_frames;
    if(the_enemy->state == ENEMY_STATE_MOVING){

        target_duration = the_enemy_info->move_duration;
        max_frames = the_enemy_info->move_frames;

    }else{

        target_duration = the_enemy_info->attack_duration;
        max_frames = the_enemy_info->attack_frames;
    }
    max_frames--;

    the_enemy->animation_timer += delta;
    if(the_enemy->animation_timer >= target_duration){

        if(the_enemy->current_frame == max_frames && the_enemy->state == ENEMY_STATE_ATTACKING){

            the_enemy->state = ENEMY_STATE_IDLE;
            the_enemy->velocity = ZERO_VECTOR;
            the_enemy->current_frame = 0;
            the_enemy->animation_timer = 0;

        }else{

            the_enemy->current_frame = the_enemy->current_frame == max_frames ? 0 : the_enemy->current_frame + 1;
            the_enemy->animation_timer -= target_duration;
        }
    }
}

bool enemy_has_hurtbox(enemy* the_enemy){

    enemy_data* the_enemy_info = &enemy_info[the_enemy->name];
    return the_enemy->state == ENEMY_STATE_ATTACKING && the_enemy->current_frame >= the_enemy_info->attack_danger_frame && the_enemy->current_frame < the_enemy_info->attack_safe_frame;
}
