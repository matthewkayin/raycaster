#include "engine.h"
#include "state.h"
#include "enemy.h"

#include <SDL2/SDL.h>

void loop_game();

int main(){

    enemy_data_init(); // init first since needed by engine

    bool success = engine_init();
    if(!success){

        return 0;
    }

    State* state = state_init();
    bool input_held[4] = {false, false, false, false};

    bool running = true;
    engine_clock_init();
    while(running){

        SDL_Event e;
        while(SDL_PollEvent(&e)){

            if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){

                running = false;

            }else if(e.type == SDL_KEYDOWN){

                int key = e.key.keysym.sym;
                if(key == SDLK_F11){

                    engine_toggle_fullscreen();

                }if(key == SDLK_w){

                    state->player_move_dir.y = -1;
                    input_held[0] = true;

                }else if(key == SDLK_d){

                    state->player_move_dir.x = 1;
                    input_held[1] = true;

                }else if(key == SDLK_s){

                    state->player_move_dir.y = 1;
                    input_held[2] = true;

                }else if(key == SDLK_a){

                    state->player_move_dir.x = -1;
                    input_held[3] = true;
                }

            }else if(e.type == SDL_KEYUP){

                int key = e.key.keysym.sym;
                if(key == SDLK_w){

                    state->player_move_dir.y = 1 * (int)(input_held[2]);
                    input_held[0] = false;

                }else if(key == SDLK_d){

                    state->player_move_dir.x = -1 * (int)(input_held[3]);
                    input_held[1] = false;

                }else if(key == SDLK_s){

                    state->player_move_dir.y = -1 * (int)(input_held[0]);
                    input_held[2] = false;

                }else if(key == SDLK_a){

                    state->player_move_dir.x = 1 * (int)(input_held[1]);
                    input_held[3] = false;
                }

            }else if(e.type == SDL_MOUSEMOTION){

                state->player_rotate_dir = e.motion.xrel / 100.0;

            }else if(e.type == SDL_MOUSEBUTTONDOWN){

                if(e.button.button == SDL_BUTTON_LEFT){

                    if(!player_is_spellcasting(state)){

                        player_cast_start(state, 0);
                    }

                }else if(e.button.button == SDL_BUTTON_RIGHT){

                    if(!player_is_spellcasting(state)){

                        player_cast_start(state, 1);
                    }
                }
            }
        }

        float delta = engine_clock_tick();
        state_update(state, delta);
        engine_render_state(state);
    }

    free(state);

    engine_quit();
    return 0;
}
