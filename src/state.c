#include "state.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

const float PLAYER_SPEED = 0.05;
const float PLAYER_ROTATE_SPEED = 0.5;

State* state_init(){

    State* new_state = (State*)malloc(sizeof(State));

    // new_state->map = map_init(20, 15);
    new_state->map = map_load_from_tmx("./tiled/test.tmx");

    new_state->player_position = (vector){ .x = 2.5, .y = 2.5 };
    new_state->player_move_dir = ZERO_VECTOR;
    new_state->player_direction = (vector){ .x = 0, .y = -1 };
    new_state->player_camera = (vector){ .x = 0.66, .y = 0 };
    new_state->player_rotate_dir = 0;

    new_state->projectile_capacity = 10;
    new_state->projectile_count = 0;
    new_state->projectiles = malloc(sizeof(projectile) * new_state->projectile_capacity);

    new_state->object_capacity = 10;
    new_state->object_count = 0;
    new_state->objects = malloc(sizeof(sprite) * new_state->object_capacity);

    new_state->enemy_capacity = 10;
    new_state->enemy_count = 0;
    new_state->enemies = malloc(sizeof(enemy) * new_state->enemy_capacity);

    for(int x = 0; x < new_state->map->width; x++){

        for(int y = 0; y < new_state->map->height; y++){

            int obj = new_state->map->objects[x + (y * new_state->map->width)];
            if(obj != 0){

                sprite to_push = (sprite){
                    .image = obj - 1,
                    .position = (vector){ .x = x + 0.5, .y = y + 0.5 }
                };
                vector_push(new_state->objects, &to_push, &new_state->object_count, &new_state->object_capacity, sizeof(sprite));
            }

            int entity = new_state->map->entities[x + (y * new_state->map->width)];
            if(entity == 1){

                new_state->player_position = (vector){ .x = x + 0.5, .y = y + 0.5 };

            }else if(entity == 2){

                enemy to_push = (enemy){
                    .image = (sprite){
                        .image = 1,
                        .position = (vector){ .x = x + 0.5, .y = y + 0.5 }
                    },
                    .velocity = ZERO_VECTOR,
                    .anim = (animation){
                        .low_frame = 0,
                        .high_frame = 4,
                        .duration = 6.0,
                        .timer = 0.0
                    }
                };
                vector_push(new_state->enemies, &to_push, &new_state->enemy_count, &new_state->enemy_capacity, sizeof(enemy));
            }
        }
    }

    return new_state;
}

bool in_wall(State* state, vector v){

    int index = (int)v.x + ((int)v.y * state->map->width);
    return state->map->wall[index] != 0;
}

void state_update(State* state, float delta){

    // Rotate player and player camera
    float rotation_amount = PLAYER_ROTATE_SPEED * state->player_rotate_dir * delta;
    state->player_rotate_dir = 0; // Always reset each frame otherwise they will keep rotating
    state->player_direction = vector_rotate(state->player_direction, rotation_amount);
    state->player_camera = vector_rotate(state->player_camera, rotation_amount);

    // Player movement
    if(state->player_move_dir.x != 0 || state->player_move_dir.y != 0){

        // Move player
        float move_angle = atan2(-state->player_move_dir.y, -state->player_move_dir.x) - (PI / 2);
        vector player_velocity = vector_scale(vector_rotate(state->player_direction, move_angle), PLAYER_SPEED);
        vector player_last_pos = state->player_position;
        state->player_position = vector_sum(state->player_position, vector_mult(player_velocity, delta));

        // Collisions
        vector player_velocity_x_component = (vector){ .x = player_velocity.x, .y = 0 };
        vector player_velocity_y_component = (vector){ .x = 0, .y = player_velocity.y };

        if(in_wall(state, state->player_position)){

            bool x_caused = in_wall(state, vector_sum(player_last_pos, player_velocity_x_component));
            bool y_caused = in_wall(state, vector_sum(player_last_pos, player_velocity_y_component));

            if(x_caused){

                state->player_position.x = player_last_pos.x;
            }
            if(y_caused){

                state->player_position.y = player_last_pos.y;
            }
        }

        // Check player sprite collisions
        int sprite_collider_count = state->object_count + state->enemy_count;
        vector** sprite_colliders = malloc(sizeof(vector*) * sprite_collider_count);
        for(int i = 0; i < state->object_count; i++){

            sprite_colliders[i] = &(state->objects[i].position);
        }
        int base_index = state->object_count;
        for(int i = 0; i < state->enemy_count; i++){

            sprite_colliders[i + base_index] = &(state->enemies[i].image.position);
        }
        for(int i = 0; i < sprite_collider_count; i++){

            if(vector_distance(state->player_position, *sprite_colliders[i]) <= 0.2){

                bool x_caused = vector_distance(vector_sum(player_last_pos, player_velocity_x_component), *sprite_colliders[i]) <= 0.2;
                bool y_caused = vector_distance(vector_sum(player_last_pos, player_velocity_y_component), *sprite_colliders[i]) <= 0.2;

                if(x_caused){

                    state->player_position.x = player_last_pos.x;
                }
                if(y_caused){

                    state->player_position.y = player_last_pos.y;
                }
            }
        }
        free(sprite_colliders);
    }

    // Projectile movement
    // We go last to first element so that deletion logic is cleaner
    for(int i = state->projectile_count - 1; i >= 0; i--){

        if(state->projectiles[i].velocity.x != 0 || state->projectiles[i].velocity.y != 0){

            state->projectiles[i].image.position = vector_sum(state->projectiles[i].image.position, state->projectiles[i].velocity);
            if(in_wall(state, state->projectiles[i].image.position)){

                vector_delete(state->projectiles, i, &state->projectile_count, sizeof(projectile));
            }
        }
    }

    // Enemy update
    for(int i = state->enemy_count - 1; i >= 0; i--){

        // Movement
        vector hit;
        raycast_line_of_sight(state, state->enemies[i].image.position, state->player_position, &hit);
        if(hit.x != -1){

            vector enemy_dist = vector_sum(hit, vector_mult(state->enemies[i].image.position, -1));
            if(vector_magnitude(enemy_dist) > 0.5){

                vector enemy_velocity = vector_scale(enemy_dist, 0.04);
                state->enemies[i].image.position = vector_sum(state->enemies[i].image.position, enemy_velocity);
            }
        }

        // Animation
        animation* anim = &(state->enemies[i].anim);
        anim->timer += delta;
        if(anim->timer >= anim->duration){

            state->enemies[i].image.image = state->enemies[i].image.image == anim->high_frame ? anim->low_frame : state->enemies[i].image.image + 1;
            anim->timer -= anim->duration;
        }
    }
}

void player_shoot(State* state){

    projectile to_create = (projectile){
        .image = (sprite){
            .image = 0,
            .position = vector_sum(state->player_position, vector_scale(state->player_direction, 0.3))
        },
        .velocity = vector_scale(state->player_direction, 0.1)
    };
    vector_push(state->projectiles, &to_create, &state->projectile_count, &state->projectile_capacity, sizeof(projectile));
}

void vector_push(void* vector, void* to_push, int* count, int* capacity, size_t unit_size){

    if(*count == *capacity){

        *capacity *= 2;
        vector = realloc(vector, unit_size * *capacity);
    }

    memcpy(vector + (unit_size * *count), to_push, unit_size);
    (*count)++;
}

void vector_delete(void* vector, int index, int* count, size_t unit_size){

    if(index >= *count){

        printf("Error! Tried to delete vector item index %i out of size %i; index out of bounds\n", index, *count);
    }

    for(int i = index; i < (*count) - 1; i++){

        memcpy(vector + (unit_size * i), vector + (unit_size * (i + 1)), unit_size);
    }
    (*count)--;
}

int hits_wall(State* state, vector v){

    /*
    if(v.x == 0 || v.x == state->map_width || v.y == 0 || v.y == state->map_height){

        return true;
    }
    */

    bool x_is_int = v.x == (int)v.x;
    bool y_is_int = v.y == (int)v.y;

    vector points[4] = {
        (vector){ .x = (int)v.x, .y = (int)v.y },
        (vector){ .x = ((int)v.x) - 1, .y = (int)v.y },
        (vector){ .x = (int)v.x, .y = ((int)v.y) - 1 },
        (vector){ .x = ((int)v.x) - 1, .y = ((int)v.y) - 1 }
    };
    bool check_point[4] = {
        true,
        x_is_int,
        y_is_int,
        x_is_int && y_is_int
    };

    for(int i = 0; i < 4; i++){

        if(check_point[i]){

            int index = points[i].x + (points[i].y * state->map->width);
            if(state->map->wall[index]){

                return state->map->wall[index];
            }
        }
    }

    return -1;
}

void raycast_line_of_sight(State* state, vector origin, vector target, vector* hit){

    vector current = origin;
    vector ray = (vector){ .x = target.x - origin.x, .y = target.y - origin.y };
    float dist = vector_magnitude(ray);
    int wall_hit = hits_wall(state, current);
    while(wall_hit == -1){

        // Check if we've hit the target before we hit a wall
        if(vector_magnitude((vector){ .x = current.x - origin.x, .y = current.y - origin.y }) >= dist){

            *hit = target;
            return;
        }

        vector x_step = ZERO_VECTOR;
        vector y_step = ZERO_VECTOR;

        if(ray.x != 0){

            if(current.x == (int)current.x){

                x_step.x = ray.x > 0 ? current.x + 1 : current.x - 1;

            }else{

                x_step.x = ray.x > 0 ? (int)(current.x + 1) : (int)current.x;
            }

            x_step.x = x_step.x - current.x;
            float scale = ray.x / x_step.x;
            x_step.y = ray.y / scale;
        }

        if(ray.y != 0){

            if(current.y == (int)current.y){

                y_step.y = ray.y > 0 ? current.y + 1 : current.y - 1;

            }else{

                y_step.y = ray.y > 0 ? (int)(current.y + 1) : (int)current.y;
            }

            y_step.y = y_step.y - current.y;
            float scale = ray.y / y_step.y;
            y_step.x = ray.x / scale;
        }

        if(ray.x == 0){

            current = vector_sum(current, y_step);

        }else if(ray.y == 0){

            current = vector_sum(current, x_step);

        }else{

            float x_dist = vector_magnitude(x_step);
            float y_dist = vector_magnitude(y_step);
            vector step = x_dist <= y_dist ? x_step : y_step;
            current = vector_sum(current, step);
        }

        wall_hit = hits_wall(state, current);
    } // End while

    // Indicates hit wall before hit target
    *hit = (vector){ .x = -1, .y = -1 };
}

void render_raycast(State* state, vector origin, vector ray, float* wall_dist, int* texture_x, bool* x_sided, int* texture){

    vector current = origin;

    int wall_hit = hits_wall(state, current);
    while(wall_hit == -1){

        vector x_step = ZERO_VECTOR;
        vector y_step = ZERO_VECTOR;

        if(ray.x != 0){

            if(current.x == (int)current.x){

                x_step.x = ray.x > 0 ? current.x + 1 : current.x - 1;

            }else{

                x_step.x = ray.x > 0 ? (int)(current.x + 1) : (int)current.x;
            }

            x_step.x = x_step.x - current.x;
            float scale = ray.x / x_step.x;
            x_step.y = ray.y / scale;
        }

        if(ray.y != 0){

            if(current.y == (int)current.y){

                y_step.y = ray.y > 0 ? current.y + 1 : current.y - 1;

            }else{

                y_step.y = ray.y > 0 ? (int)(current.y + 1) : (int)current.y;
            }

            y_step.y = y_step.y - current.y;
            float scale = ray.y / y_step.y;
            y_step.x = ray.x / scale;
        }

        if(ray.x == 0){

            current = vector_sum(current, y_step);

        }else if(ray.y == 0){

            current = vector_sum(current, x_step);

        }else{

            float x_dist = vector_magnitude(x_step);
            float y_dist = vector_magnitude(y_step);
            vector step = x_dist <= y_dist ? x_step : y_step;
            current = vector_sum(current, step);
        }

        wall_hit = hits_wall(state, current);
    } // End while

    *texture = wall_hit;
    *x_sided = current.x == (int)current.x;

    vector dist = (vector){ .x = current.x - origin.x, .y = current.y - origin.y };
    *wall_dist = *x_sided ? dist.x / ray.x : dist.y / ray.y;

    int wall_x = (int)((*x_sided ? current.y - (int)current.y : current.x - (int)current.x) * 64.0);
    *texture_x = (*x_sided && ray.x > 0) || (!*x_sided && ray.y < 0) ? 64 - wall_x - 1 : wall_x;
}
