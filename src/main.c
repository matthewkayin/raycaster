#include "engine.h"
#include "state.h"

#include <SDL2/SDL.h>

int main(){

    bool success = engine_init();
    if(!success){

        return 0;
    }

    State* state = state_init();
    bool input_held[6] = {false, false, false, false, false, false};
    bool render_preview = false;

    bool running = true;
    engine_clock_init();
    while(running){

        SDL_Event e;
        while(SDL_PollEvent(&e)){

            if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){

                running = false;

            }else if(e.type == SDL_KEYDOWN){

                int key = e.key.keysym.sym;
                if(key == SDLK_w){

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

                }else if(key == SDLK_q){

                    state->player_rotate_dir = -1;
                    input_held[4] = true;

                }else if(key == SDLK_e){

                    state->player_rotate_dir = 1;
                    input_held[5] = true;

                }else if(key == SDLK_F1){

                    render_preview = !render_preview;
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

                }else if(key == SDLK_q){

                    state->player_rotate_dir = 1 * (int)(input_held[5]);
                    input_held[4] = false;

                }else if(key == SDLK_e){

                    state->player_rotate_dir = -1 * (int)(input_held[4]);
                    input_held[5] = false;
                }

            }else if(e.type == SDL_MOUSEMOTION){

                state->player_rotate_dir = e.motion.xrel / 100.0;
            }
        }

        float delta = engine_clock_tick();
        state_update(state, delta);
        if(render_preview){

            engine_render_preview(state);

        }else{

            engine_render_state(state);
        }
    }


    engine_quit();

    return 0;
}
