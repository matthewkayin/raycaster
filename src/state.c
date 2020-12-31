#include "state.h"

#include <stdlib.h>
#include <math.h>

const float PLAYER_SPEED = 0.05;
const float PLAYER_ROTATE_SPEED = 0.05;

State* state_init(){

    State* new_state = (State*)malloc(sizeof(State));

    new_state->map_width = 6;
    new_state->map_height = 6;
    new_state->map = (bool*)malloc(sizeof(bool) * (new_state->map_width * new_state->map_height));

    for(int i = 0; i < new_state->map_width; i++){

        for(int j = 0; j < new_state->map_height; j++){

            int index = i + (j * new_state->map_width);
            if(i == 0 || i == new_state->map_width - 1 || j == 0 || j == new_state->map_height - 1){

                new_state->map[index] = true;

            }else{

                new_state->map[index] = false;
            }
        }
    }

    new_state->map[1 + new_state->map_width] = true;

    new_state->player_position = (vector){ .x = 2.5, .y = 2.5 };
    new_state->player_move_dir = ZERO_VECTOR;
    new_state->player_direction = (vector){ .x = 0, .y = -1 };
    new_state->player_camera = (vector){ .x = 0.66, .y = 0 };
    new_state->player_rotate_dir = 0;

    return new_state;
}

void state_update(State* state, float delta){

    float rotation_amount = PLAYER_ROTATE_SPEED * state->player_rotate_dir * delta;
    state->player_direction = vector_rotate(state->player_direction, rotation_amount);
    state->player_camera = vector_rotate(state->player_camera, rotation_amount);

    if(state->player_move_dir.x != 0 || state->player_move_dir.y != 0){

        float angle = atan2(-state->player_move_dir.y, -state->player_move_dir.x) - (PI / 2);
        vector player_velocity = vector_scale(vector_rotate(state->player_direction, angle), PLAYER_SPEED);
        state->player_position = vector_sum(state->player_position, vector_mult(player_velocity, delta));
    }
}

bool hits_wall(State* state, vector v){

    if(v.x == 0 || v.x == state->map_width || v.y == 0 || v.y == state->map_height){

        return true;
    }

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

                return true;
            }
        }
    }

    return false;
}

vector raycast(State* state, vector origin, vector ray){

    vector current = origin;

    while(!hits_wall(state, current)){

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
    } // End while

    return current;
}

float raycast_dda(State* state, vector origin, vector ray){

    vector start = (vector){ .x = (int)origin.x, .y = (int)origin.y };
    vector side_dist;
    vector delta_dist = (vector){ .x = ray.x == 0 ? 0 : fabs(1 / ray.x), .y = ray.y == 0 ? 0 : fabs(1/ ray.y) };
    float perp_wall_dist;

    vector step;
    bool wall_hit = false;
    bool x_sided;

    if(ray.x < 0){

        step.x = -1;
        side_dist.x = (origin.x - start.x) * delta_dist.x;

    }else{

        step.x = 1;
        side_dist.x = (start.x + 1 - origin.x) * delta_dist.x;
    }
    if(ray.y < 0){

        step.y = -1;
        side_dist.y = (origin.y - start.y) * delta_dist.y;

    }else{

        step.y = 1;
        side_dist.y = (start.y + 1 - origin.y) * delta_dist.y;
    }

    while(!wall_hit){

        if(side_dist.x < side_dist.y){

            side_dist.x += delta_dist.x;
            start.x += step.x;
            x_sided = true;

        }else{

            side_dist.y += delta_dist.y;
            start.y += step.y;
            x_sided = false;
        }

        int index = (int)start.x + ((int)start.y * state->map_width);
        wall_hit = state->map[index] || start.x == 0 || start.x == state->map_width || start.y == 0 || start.y == state->map_height;
    }

    if(x_sided){

        perp_wall_dist = (start.x - origin.x + ((1 - step.x) / 2)) / ray.x;

    }else{

        perp_wall_dist = (start.y - origin.y + ((1 - step.y) / 2)) / ray.y;
    }

    return perp_wall_dist;
}

float raycast_get_walldist(State* state, vector origin, vector ray){

    vector wall_point = raycast(state, origin, ray);
    vector dist = (vector){ .x = wall_point.x - origin.x, .y = wall_point.y - origin.y };
    return wall_point.x == (int)wall_point.x ? dist.x / ray.x : dist.y / ray.y;
}
