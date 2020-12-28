#include "state.h"

#include <stdlib.h>

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
    new_state->player_rotate_dir = 0;

    return new_state;
}

void state_update(State* state, float delta){

    vector player_velocity = vector_scale(state->player_move_dir, PLAYER_SPEED);
    state->player_position = vector_sum(state->player_position, vector_mult(player_velocity, delta));
    state->player_direction = vector_rotate(state->player_direction, PLAYER_ROTATE_SPEED * state->player_rotate_dir);
}
