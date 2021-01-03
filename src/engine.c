#include "engine.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <stdio.h>
#include <stdint.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 360;

uint32_t* screen_buffer;
SDL_Texture* screen_buffer_texture;
SDL_Surface* screen_surface;

const int TEXTURE_SIZE = 64;
int texture_count;
uint32_t* texture_banner;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
bool is_fullscreen = false;

TTF_Font* font_small;

const SDL_Color COLOR_WHITE = (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 };

const unsigned long SECOND = 1000;
const float FRAME_TIME = SECOND / 60.0;
const float UPDATE_TIME = SECOND / 60.0;
unsigned long second_before_time;
unsigned long frame_before_time;
unsigned long last_update_time;
float deltas = 0;
int frames = 0;
int fps = 0;
int ups = 0;

bool engine_init(){

    if(SDL_Init(SDL_INIT_VIDEO) < 0){

        printf("Unable to initialize SDL! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int img_flags = IMG_INIT_PNG;

    if(!(IMG_Init(img_flags) & img_flags)){

        printf("Unable to initialize SDL_image! SDL Error: %s\n", IMG_GetError());
        return false;
    }

    if(TTF_Init() == -1){

        printf("Unable to initialize SDL_ttf! SDL Error: %s\n", TTF_GetError());
        return false;
    }

    if(!window || !renderer){

        printf("Unable to initialize engine!\n");
        return false;
    }

    font_small = TTF_OpenFont("./res/hack.ttf", 10);
    if(font_small == NULL){

        printf("Unable to initialize font_small! SDL Error: %s\n", TTF_GetError());
        return false;
    }

    engine_set_resolution(SCREEN_WIDTH, SCREEN_HEIGHT);
    screen_surface = SDL_GetWindowSurface(window);
    screen_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_Surface* loaded_surface = IMG_Load("./res/wolf_banner.png");
    uint32_t* loaded_surface_pixels = loaded_surface->pixels;
    texture_banner = (uint32_t*)malloc(sizeof(uint32_t) * TEXTURE_SIZE * TEXTURE_SIZE);
    for(int i = 0; i < TEXTURE_SIZE * TEXTURE_SIZE; i++){

        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
        SDL_GetRGBA(loaded_surface_pixels[i], loaded_surface->format, &r, &g, &b, &a);
        texture_banner[i] = SDL_MapRGBA(screen_surface->format, r, g, b, a);
    }

    return true;
}

void engine_quit(){

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void engine_set_resolution(int width, int height){

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetWindowSize(window, width, height);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void engine_toggle_fullscreen(){

    if(is_fullscreen){

        SDL_SetWindowFullscreen(window, 0);

    }else{

        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }

    is_fullscreen = !is_fullscreen;
}

void engine_clock_init(){

    second_before_time = SDL_GetTicks();
    frame_before_time = second_before_time;
    last_update_time = second_before_time;
}

float engine_clock_tick(){

    frames++;
    unsigned long current_time = SDL_GetTicks();

    if(current_time - second_before_time >= SECOND){

        fps = frames;
        ups = (int)deltas;
        frames = 0;
        deltas -= ups;
        second_before_time += SECOND;
    }

    float delta = (current_time - last_update_time) / UPDATE_TIME;
    deltas += delta;
    last_update_time = current_time;

    if(current_time - frame_before_time < FRAME_TIME){

        unsigned long delay_time = FRAME_TIME - (current_time - frame_before_time);
        SDL_Delay(delay_time);
    }

    return delta;
}

void engine_render_text(const char* text, SDL_Color color, int x, int y){

    SDL_Surface* text_surface = TTF_RenderText_Solid(font_small, text, color);
    if(text_surface == NULL){

        printf("Unable to render text to surface! SDL Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    if(text_texture == NULL){

        printf("Unable to reate texture! SDL Error: %s\n", SDL_GetError());
        return;
    }

    SDL_Rect source_rect = (SDL_Rect){ .x = 0, .y = 0, .w = text_surface->w, .h = text_surface->h };
    SDL_Rect dest_rect = (SDL_Rect){ .x = x, .y = y, .w = text_surface->w, .h = text_surface->h };
    SDL_RenderCopy(renderer, text_texture, &source_rect, &dest_rect);

    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

void engine_render_fps(){

    char fps_text[10];
    sprintf(fps_text, "FPS: %i", fps);
    engine_render_text(fps_text, COLOR_WHITE, 0, 0);
    char ups_text[10];
    sprintf(ups_text, "UPS: %i", ups);
    engine_render_text(ups_text, COLOR_WHITE, 0, 10);
}

void engine_put_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b){

    int index = x + (y * SCREEN_WIDTH);
    screen_buffer[index] = SDL_MapRGBA(screen_surface->format, r, g, b, 255);
}

void engine_unlock_buffer(){

    void* buffer_pixels;
    int buffer_pitch;
    SDL_LockTexture(screen_buffer_texture, NULL, &buffer_pixels, &buffer_pitch);
    screen_buffer = (uint32_t*)buffer_pixels;

    for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++){

        screen_buffer[i] = SDL_MapRGBA(screen_surface->format, 0, 0, 0, 255);
    }
}

void engine_render_buffer(){

    SDL_UnlockTexture(screen_buffer_texture);
    SDL_RenderCopy(renderer, screen_buffer_texture, &(SDL_Rect){ .x = 0, .y = 0, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT }, &(SDL_Rect){ .x = 0, .y = 0, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT });
}

void engine_render_preview(State* state){

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for(int x = 0; x < state->map_width; x++){

        for(int y = 0; y < state->map_height; y++){

            int index = x + (y * state->map_width);
            if(state->map[index]){

                SDL_RenderFillRect(renderer, &(SDL_Rect){ .x = x * 20, .y = y * 20, .w = 20, .h = 20 });
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &(SDL_Rect){ .x = (int)(state->player_position.x * 20) - 2, .y = (int)(state->player_position.y * 20) - 2, .w = 4, .h = 4 });

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    vector dir = vector_sum(state->player_position, state->player_direction);
    vector cam = vector_sum(dir, state->player_camera);
    SDL_RenderDrawLine(renderer, (int)(state->player_position.x * 20), (int)(state->player_position.y * 20), (int)(dir.x * 20), (int)(dir.y * 20));
    SDL_RenderDrawLine(renderer, (int)(dir.x * 20), (int)(dir.y * 20), (int)(cam.x * 20), (int)(cam.y * 20));

    engine_render_fps();

    SDL_RenderPresent(renderer);
}

void engine_render_state(State* state){

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    engine_unlock_buffer();

    // Floor casting
    for(int y = 0; y < SCREEN_HEIGHT; y++){

        vector ray_dir0 = vector_sum(state->player_direction, vector_mult(state->player_camera, -1));
        vector ray_dir1 = vector_sum(state->player_direction, state->player_camera);

        int p = y - (SCREEN_HEIGHT / 2);
        float z_pos = 0.5 * SCREEN_HEIGHT;
        float row_dist = z_pos / p;

        vector floor_step = vector_mult(vector_sum(ray_dir1, vector_mult(ray_dir0, -1)), row_dist / SCREEN_WIDTH);
        vector floor = vector_sum(state->player_position, vector_mult(ray_dir0, row_dist));

        for(int x = 0; x < SCREEN_WIDTH; ++x){

            vector cell = (vector){ .x = (int)floor.x, .y = (int)floor.y };
            int texture_x = (int)(TEXTURE_SIZE * (floor.x - cell.x)) & (TEXTURE_SIZE - 1);
            int texture_y = (int)(TEXTURE_SIZE * (floor.y - cell.y)) & (TEXTURE_SIZE - 1);

            floor = vector_sum(floor, floor_step);
            int source_index = texture_x + (texture_y * TEXTURE_SIZE);
            int dest_index = x + (y * SCREEN_WIDTH);
            screen_buffer[dest_index] = (texture_banner[source_index] >> 1) & 8355711;
            dest_index = x + ((SCREEN_HEIGHT - y - 1) * SCREEN_WIDTH);
            screen_buffer[dest_index] = (texture_banner[source_index] >> 1) & 8355711;
        }
    }

    // Wall casting
    for(int x = 0; x < SCREEN_WIDTH; x++){

        float camera_x = ((2 * x) / (float)SCREEN_WIDTH) - 1;
        vector ray = vector_sum(state->player_direction, vector_mult(state->player_camera, camera_x));
        float wall_dist;
        int texture_x;
        bool x_sided;
        raycast_get_info(state, state->player_position, ray, &wall_dist, &texture_x, &x_sided);

        int line_height = (int)(SCREEN_HEIGHT / wall_dist);
        int line_start = (SCREEN_HEIGHT / 2) - (line_height / 2);
        int line_end = (SCREEN_HEIGHT / 2) + (line_height / 2);
        if(line_start < 0){

            line_start = 0;
        }
        if(line_end >= SCREEN_HEIGHT){

            line_end = SCREEN_HEIGHT - 1;
        }

        float step = (1.0 * TEXTURE_SIZE) / line_height;
        float texture_pos = (line_start - (SCREEN_HEIGHT / 2) + (line_height / 2)) * step;
        for(int y = line_start; y < line_end; y++){

            int texture_y = (int)texture_pos & (TEXTURE_SIZE - 1);
            texture_pos += step;
            int source_index = texture_x + (texture_y * TEXTURE_SIZE);
            int dest_index = x + (y * SCREEN_WIDTH);
            screen_buffer[dest_index] = x_sided ? texture_banner[source_index] : (texture_banner[source_index] >> 1) & 8355711;
        }
    }

    engine_render_buffer();

    engine_render_fps();
    SDL_RenderPresent(renderer);
}
