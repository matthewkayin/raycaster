#include "state.h"
#include "vector_array.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

// Player movement constants
const float PLAYER_SPEED = 0.05;
const float PLAYER_ROTATE_SPEED = 0.5;

// Player animation states
const int PLAYER_ANIMATION_STATE_IDLE = 0;
const int PLAYER_ANIMATION_STATE_WALK = 1;
const int PLAYER_ANIMATION_STATE_SPELLCAST = 2;

// Player animation constants
const float PLAYER_ANIMATION_WALK_DURATION = 30.0;
const float PLAYER_ANIMATION_SPELLCAST_DURATION = 3.0;
const int PLAYER_ANIMATION_SPELLCAST_FRAMES = 4;
const int PLAYER_OFFSET_X_MAX = 4;
const int PLAYER_OFFSET_Y_MAX = 4;

// Init
State* state_init(){

    State* new_state = (State*)malloc(sizeof(State));

    // new_state->map = map_init(20, 15);
    new_state->map = map_load_from_tmx("./tiled/test.tmx");

    new_state->player_position = (vector){ .x = 2.5, .y = 2.5 };
    new_state->player_velocity = ZERO_VECTOR;
    new_state->player_move_dir = ZERO_VECTOR;
    new_state->player_direction = (vector){ .x = 0, .y = -1 };
    new_state->player_camera = (vector){ .x = 0.66, .y = 0 };
    new_state->player_rotate_dir = 0;

    new_state->player_knockback_timer = 0;

    new_state->player_animation_state = PLAYER_ANIMATION_STATE_IDLE;
    new_state->player_animation_frame = 0;
    new_state->player_animation_timer = 0.0;

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
                vector_array_push((void**)&(new_state->objects), &to_push, &new_state->object_count, &new_state->object_capacity, sizeof(sprite));
            }

            int entity = new_state->map->entities[x + (y * new_state->map->width)];
            if(entity == 1){

                new_state->player_position = (vector){ .x = x + 0.5, .y = y + 0.5 };

            }else if(entity == 2){

                enemy to_push = (enemy){
                    .name = ENEMY_SLIME,
                    .state = ENEMY_STATE_IDLE,
                    .current_frame = 0,
                    .animation_timer = 0,
                    .position = (vector){ .x = x + 0.5, .y = y + 0.5 },
                    .velocity = ZERO_VECTOR
                };
                vector_array_push((void**)&(new_state->enemies), &to_push, &new_state->enemy_count, &new_state->enemy_capacity, sizeof(enemy));
            }
        }
    }

    return new_state;
}

// Updates

void state_update(State* state, float delta){

    // Rotate player and player camera
    float rotation_amount = PLAYER_ROTATE_SPEED * state->player_rotate_dir * delta;
    state->player_rotate_dir = 0; // Always reset each frame otherwise they will keep rotating
    state->player_direction = vector_rotate(state->player_direction, rotation_amount);
    state->player_camera = vector_rotate(state->player_camera, rotation_amount);

    bool player_inputs_movement = state->player_move_dir.x != 0 || state->player_move_dir.y != 0;

    // Player movement
    vector player_last_pos = state->player_position;
    if(state->player_knockback_timer != 0){

        state->player_knockback_timer -= delta;
        if(state->player_knockback_timer <= 0){

            state->player_knockback_timer = 0;
            state->player_velocity = ZERO_VECTOR;
        }

    }else if(player_inputs_movement){

        // Move player
        float move_angle = atan2(-state->player_move_dir.y, -state->player_move_dir.x) - (PI / 2);
        state->player_velocity = vector_mult(vector_scale(vector_rotate(state->player_direction, move_angle), PLAYER_SPEED), delta);

    }else{

        state->player_velocity = ZERO_VECTOR;
    }

    state->player_position = vector_sum(state->player_position, state->player_velocity);

    // Collisions
    if(state->player_knockback_timer != 0 && in_wall(state, state->player_position)){

        state->player_knockback_timer = 0;
    }
    check_wall_collisions(state, &(state->player_position), player_last_pos, state->player_velocity);

    for(int i = 0; i < state->object_count; i++){

        check_sprite_collision(&(state->player_position), player_last_pos, state->player_velocity, state->objects[i].position, 0.2);
    }
    for(int i = 0; i < state->enemy_count; i++){

        check_sprite_collision(&(state->player_position), player_last_pos, state->player_velocity, state->enemies[i].position, 0.2);
    }

    // Player animation update
    if(state->player_animation_state != PLAYER_ANIMATION_STATE_SPELLCAST){

        state->player_animation_state = player_inputs_movement ? PLAYER_ANIMATION_STATE_WALK : PLAYER_ANIMATION_STATE_IDLE;
    }
    if(state->player_animation_state == PLAYER_ANIMATION_STATE_IDLE){

        state->player_animation_timer = 0;

    }else if(state->player_animation_state == PLAYER_ANIMATION_STATE_WALK){

        state->player_animation_timer += delta;
        if(state->player_animation_timer >= PLAYER_ANIMATION_WALK_DURATION){

            state->player_animation_timer -= PLAYER_ANIMATION_WALK_DURATION;
        }

    }else if(state->player_animation_state == PLAYER_ANIMATION_STATE_SPELLCAST){

        state->player_animation_timer += delta;
        if(state->player_animation_timer >= PLAYER_ANIMATION_SPELLCAST_DURATION){

            state->player_animation_frame++;
            if(state->player_animation_frame == PLAYER_ANIMATION_SPELLCAST_FRAMES){

                state->player_animation_state = PLAYER_ANIMATION_STATE_IDLE;
                state->player_animation_frame = 0;
                state->player_animation_timer = 0;
                player_cast_kinetic(state);

            }else{

                state->player_animation_timer -= PLAYER_ANIMATION_SPELLCAST_DURATION;
            }
        }
    }

    // Projectile movement
    // We go last to first element so that deletion logic is cleaner
    for(int i = state->projectile_count - 1; i >= 0; i--){

        if(state->projectiles[i].velocity.x != 0 || state->projectiles[i].velocity.y != 0){

            state->projectiles[i].position = vector_sum(state->projectiles[i].position, state->projectiles[i].velocity);
            if(in_wall(state, state->projectiles[i].position)){

                vector_array_delete(state->projectiles, i, &state->projectile_count, sizeof(projectile));
            }
        }
    }

    // Enemy update
    for(int i = state->enemy_count - 1; i >= 0; i--){

        enemy_update(state, i, delta);
    }
}

void enemy_update(State* state, int index, float delta){

    enemy* current_enemy = &state->enemies[index];
    enemy_data* current_info = &(enemy_info[state->enemies[index].name]);

    // Movement
    vector enemy_last_pos = current_enemy->position;

    if(current_enemy->state != ENEMY_STATE_ATTACKING && current_enemy->state != ENEMY_STATE_KNOCKBACK && vector_distance(state->player_position, current_enemy->position) <= current_info->attack_radius){

        if(state->enemies[index].state != ENEMY_STATE_ATTACKING){

            current_enemy->state = ENEMY_STATE_ATTACKING;
            current_enemy->velocity = ZERO_VECTOR;
        }
    }

    if(current_enemy->state == ENEMY_STATE_KNOCKBACK){

        current_enemy->animation_timer -= delta;
        if(current_enemy->animation_timer <= 0){

            current_enemy->state = ENEMY_STATE_IDLE;
            current_enemy->animation_timer = 0;
            current_enemy->velocity = ZERO_VECTOR;
        }

    }else if(current_enemy->state == ENEMY_STATE_ATTACKING){

        if(enemy_has_hurtbox(current_enemy)){

            if(current_enemy->velocity.x == 0 && current_enemy->velocity.y == 0){

                current_enemy->velocity = vector_scale(vector_sub(state->player_position, current_enemy->position), current_info->attack_speed);
            }

        }else{

            current_enemy->velocity = ZERO_VECTOR;
        }

    }else{

        vector enemy_target;
        bool success = map_pathfind(state->map, current_enemy->position, state->player_position, &enemy_target);
        if(success){

            current_enemy->state = ENEMY_STATE_MOVING;

            enemy_target.x += 0.5;
            enemy_target.y += 0.5;
            vector enemy_direction = vector_scale(vector_sub(enemy_target, current_enemy->position), 1);

            current_enemy->velocity = vector_scale(enemy_direction, current_info->speed);

        }else{

            current_enemy->state = ENEMY_STATE_IDLE;
            current_enemy->velocity = ZERO_VECTOR;
        }
    }

    current_enemy->position = vector_sum(current_enemy->position, current_enemy->velocity);
    check_rect_wall_collisions(state, &(current_enemy->position), enemy_last_pos, current_enemy->velocity, 0.5);

    // Check for collisions with other enemies
    for(int j = 0; j < state->enemy_count; j++){

        if(index == j){

            continue;
        }

        check_sprite_collision(&(current_enemy->position), enemy_last_pos, current_enemy->velocity, state->enemies[j].position, 0.5);
    }

    // After checking for collisions, check if hurt player
    if(enemy_has_hurtbox(current_enemy) && vector_distance(current_enemy->position, state->player_position) <= 0.2){

        player_knockback(state, current_enemy->velocity);
        current_enemy->state = ENEMY_STATE_IDLE;
        current_enemy->velocity = ZERO_VECTOR;
    }

    // Animation
    enemy_animation_update(current_enemy, delta);
}

// Collision helpers / handlers

bool in_wall(State* state, vector v){

    int index = (int)v.x + ((int)v.y * state->map->width);
    return state->map->wall[index] != 0;
}

bool rect_in_wall(State* state, vector rect_pos, vector rect_dim){

    int indeces[4] = {
        (int)rect_pos.x + ((int)rect_pos.y * state->map->width),
        (int)(rect_pos.x + rect_dim.x) + ((int)rect_pos.y * state->map->width),
        (int)rect_pos.x + ((int)(rect_pos.y + rect_dim.y) * state->map->width),
        (int)(rect_pos.x + rect_dim.x) + ((int)(rect_pos.y + rect_dim.y) * state->map->width)
    };

    bool is_in_wall = false;
    for(int i = 0; i < 4; i++){

        is_in_wall |= state->map->wall[indeces[i]] != 0;
    }

    return is_in_wall;
}

void check_wall_collisions(State* state, vector* mover_position, vector mover_last_pos, vector velocity){

    if(in_wall(state, *mover_position)){

        bool x_caused = in_wall(state, vector_sum(mover_last_pos, (vector){ .x = velocity.x, .y = 0 }));
        bool y_caused = in_wall(state, vector_sum(mover_last_pos, (vector){ .x = 0, .y = velocity.y }));

        if(x_caused){

            mover_position->x = mover_last_pos.x;
        }
        if(y_caused){

            mover_position->y = mover_last_pos.y;
        }
    }
}

void check_rect_wall_collisions(State* state, vector* mover_position, vector mover_last_pos, vector velocity, float rect_size){

    vector rect_pos = (vector){ .x = mover_position->x - (rect_size / 2), .y = mover_position->y - (rect_size / 2) };
    vector rect_last_pos = (vector){ .x = mover_last_pos.x - (rect_size / 2), .y = mover_last_pos.y - (rect_size / 2) };
    vector rect_dim = (vector){ .x = rect_size, .y = rect_size };
    if(rect_in_wall(state, rect_pos, rect_dim)){

        bool x_caused = rect_in_wall(state, vector_sum(rect_last_pos, (vector){ .x = velocity.x, .y = 0 }), rect_dim);
        bool y_caused = rect_in_wall(state, vector_sum(rect_last_pos, (vector){ .x = 0, .y = velocity.y }), rect_dim);

        if(x_caused){

            mover_position->x = mover_last_pos.x;
        }
        if(y_caused){

            mover_position->y = mover_last_pos.y;
        }
    }
}

void check_sprite_collision(vector* mover_position, vector mover_last_pos, vector velocity, vector object, float collision_dist){

    if(vector_distance(*mover_position, object) <= collision_dist){

        bool x_caused = vector_distance(vector_sum(mover_last_pos, (vector){ .x = velocity.x, .y = 0 }), object) <= collision_dist;
        bool y_caused = vector_distance(vector_sum(mover_last_pos, (vector){ .x = 0, .y = velocity.y }), object) <= collision_dist;

        if(x_caused){

            (*mover_position).x = mover_last_pos.x;
        }
        if(y_caused){

            (*mover_position).y = mover_last_pos.y;
        }
    }
}

// Player

vector player_get_animation_offset(State* state){

    if(state->player_animation_state == PLAYER_ANIMATION_STATE_SPELLCAST){

        return ZERO_VECTOR;

    }else{

        vector offset;

        // compute x offset
        float half_time = PLAYER_ANIMATION_WALK_DURATION / 2;
        float fmod_time = state->player_animation_timer;
        bool second_half = false;
        if(fmod_time > half_time){

            fmod_time -= half_time;
            second_half = true;
        }
        float percentage = fmod_time / half_time;

        offset.x = (second_half ? 1 - percentage : percentage) * PLAYER_OFFSET_X_MAX;

        // compute y offset
        float quarter_time = PLAYER_ANIMATION_WALK_DURATION / 4;
        fmod_time = state->player_animation_timer;
        int which_quarter = 0;
        while(fmod_time > quarter_time){

            fmod_time -= quarter_time;
            which_quarter++;
        }
        percentage = fmod_time / quarter_time;

        offset.y = (which_quarter % 2 == 0 ? percentage : 1 - percentage) * PLAYER_OFFSET_Y_MAX;

        return offset;
    }
}

void player_knockback(State* state, vector impact_vector){

    // Knockback player
    state->player_velocity = impact_vector;
    state->player_knockback_timer = 20.0;

    // Reset animation / interrupt current spellcast
    state->player_animation_state = PLAYER_ANIMATION_STATE_IDLE;
    state->player_animation_frame = 0;
    state->player_animation_timer = 0;
}

void player_cast_start(State* state){

    state->player_animation_state = PLAYER_ANIMATION_STATE_SPELLCAST;
    state->player_animation_frame = 0;
    state->player_animation_timer = 0;
}

void player_cast_kinetic(State* state){

    vector player_look_point = vector_sum(state->player_position, state->player_direction);
    float player_angle = atan2(player_look_point.y - state->player_position.y, player_look_point.x - state->player_position.x) * (180 / PI);

    for(int i = 0; i < state->enemy_count; i++){

        enemy* current_enemy = &(state->enemies[i]);

        if(vector_distance(current_enemy->position, state->player_position) > 2.5){

            continue;
        }

        vector difference_vector = (vector){ .x = current_enemy->position.x - state->player_position.x, .y = current_enemy->position.y - state->player_position.y };
        float angle = (atan2(difference_vector.y, difference_vector.x) * (180 / PI));
        if(fabs(player_angle - angle) <= 50.0){

            current_enemy->state = ENEMY_STATE_KNOCKBACK;
            current_enemy->velocity = vector_scale(difference_vector, 0.1);
            current_enemy->animation_timer = 20.0;
            current_enemy->current_frame = 0;
        }
    }
}

// Raycasting

int hits_wall(State* state, vector v){

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

bool hit_tile(vector v, vector tile){

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

        if(check_point[i] && points[i].x == tile.x && points[i].y == tile.y){

            return true;
        }
    }

    return false;
}

bool ray_intersects(State* state, vector origin, vector ray, vector target){

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

    float wall_distance = vector_distance(origin, current);
    float target_distance = vector_distance(origin, target);

    return target_distance <= wall_distance;
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
