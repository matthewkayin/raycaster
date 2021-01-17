#include "state.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

const float PLAYER_SPEED = 0.05;
const float PLAYER_ROTATE_SPEED = 0.5;

State* state_init(){

    State* new_state = (State*)malloc(sizeof(State));

    new_state->map = NULL;
    new_state->map_ceil = NULL;
    new_state->map_floor = NULL;
    new_state->map_width = 0;
    new_state->map_height = 0;

    new_state->player_position = (vector){ .x = 2.5, .y = 2.5 };
    new_state->player_move_dir = ZERO_VECTOR;
    new_state->player_direction = (vector){ .x = 0, .y = -1 };
    new_state->player_camera = (vector){ .x = 0.66, .y = 0 };
    new_state->player_rotate_dir = 0;

    new_state->sprites = (sprite*)malloc(sizeof(sprite) * 3);
    new_state->sprite_count = 3;
    new_state->sprites[0] = (sprite){ .image = 9, .position = (vector){ .x = 2.5, .y = 6.5 }};
    new_state->sprites[1] = (sprite){ .image = 9, .position = (vector){ .x = 6.5, .y = 5.5 }};
    new_state->sprites[2] = (sprite){ .image = 9, .position = (vector){ .x = 8.5, .y = 4.5 }};

    return new_state;
}

bool in_wall(State* state, vector v){

    int index = (int)v.x + ((int)v.y * state->map_width);
    return state->map[index] != 0;
}

void state_update(State* state, float delta){

    float rotation_amount = PLAYER_ROTATE_SPEED * state->player_rotate_dir * delta;
    state->player_rotate_dir = 0; // Always reset each frame otherwise they will keep rotating
    state->player_direction = vector_rotate(state->player_direction, rotation_amount);
    state->player_camera = vector_rotate(state->player_camera, rotation_amount);

    if(state->player_move_dir.x != 0 || state->player_move_dir.y != 0){

        float angle = atan2(-state->player_move_dir.y, -state->player_move_dir.x) - (PI / 2);
        vector player_velocity = vector_scale(vector_rotate(state->player_direction, angle), PLAYER_SPEED);
        vector player_last_pos = state->player_position;
        state->player_position = vector_sum(state->player_position, vector_mult(player_velocity, delta));

        if(in_wall(state, state->player_position)){

            bool x_caused = in_wall(state, vector_sum(player_last_pos, (vector){ .x = player_velocity.x, .y = 0 }));
            bool y_caused = in_wall(state, vector_sum(player_last_pos, (vector){ .x = 0, .y = player_velocity.y }));

            if(x_caused){

                state->player_position.x = player_last_pos.x;
            }
            if(y_caused){

                state->player_position.y = player_last_pos.y;
            }
        }
    }
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

            int index = points[i].x + (points[i].y * state->map_width);
            if(state->map[index]){

                return state->map[index];
            }
        }
    }

    return -1;
}

void raycast(State* state, vector origin, vector ray, float* wall_dist, int* texture_x, bool* x_sided, int* texture){

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
