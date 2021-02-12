#include "engine.h"
#include "enemy.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <stdio.h>
#include <stdint.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 360;

float z_buffer[640];

uint32_t* screen_buffer;
SDL_Texture* screen_buffer_texture;
SDL_Surface* screen_surface;

typedef struct spritesheet{
    int sprite_count;
    uint32_t** sprites;
} spritesheet;

const int TEXTURE_SIZE = 64;
spritesheet* texture_sprites;
spritesheet* object_sprites;
spritesheet* projectile_sprites;
spritesheet** enemy_move_sprites;
spritesheet** enemy_attack_sprites;

typedef struct anim_texture{
    SDL_Texture* texture;
    SDL_Rect* frame_rects;
    int frame_count;
} anim_texture;

anim_texture* player_hand_anim;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
bool is_fullscreen = false;

TTF_Font* font_small;

uint32_t COLOR_TRANSPARENT;
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

SDL_Texture* engine_texture_load(const char* path){

    SDL_Surface* loaded_surface = IMG_Load(path);
    if(loaded_surface == NULL){

        printf("Unable to load texture image! SDL Error %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture* new_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if(new_texture == NULL){

        printf("Unable to create texture! SDL Error: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(loaded_surface);
    return new_texture;
}

anim_texture* engine_anim_texture_load(const char* path, int frame_width, int frame_height, int frame_count){

    anim_texture* new_texture = malloc(sizeof(anim_texture));

    SDL_Surface* loaded_surface = IMG_Load(path);
    if(loaded_surface == NULL){

        printf("Unable to load texture image! SDL Error %s\n", IMG_GetError());
        return NULL;
    }

    new_texture->texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if(new_texture->texture == NULL){

        printf("Unable to create texture! SDL Error: %s\n", SDL_GetError());
        return NULL;
    }

    new_texture->frame_count = frame_count;
    new_texture->frame_rects = malloc(sizeof(SDL_Rect) * frame_count);
    int frame_x = 0;
    int frame_y = 0;
    for(int i = 0; i < frame_count; i++){

        new_texture->frame_rects[i] = (SDL_Rect){
            .x = frame_x,
            .y = frame_y,
            .w = frame_width,
            .h = frame_height
        };

        frame_x += frame_width;
        if(frame_x >= loaded_surface->w){

            frame_x = 0;
            frame_y += frame_height;
        }
    }

    SDL_FreeSurface(loaded_surface);

    return new_texture;
}

void engine_anim_texture_free(anim_texture* anim){

    SDL_DestroyTexture(anim->texture);
    free(anim->frame_rects);
    free(anim);
}

spritesheet* engine_spritesheet_load(const char* path){

    SDL_Surface* loaded_surface = IMG_Load(path);
    if(loaded_surface == NULL){

        printf("Unable to load spritesheet image! SDL Error: %s\n", IMG_GetError());
        return NULL;
    }

    uint32_t* loaded_surface_pixels = loaded_surface->pixels;
    int sprite_count_width = loaded_surface->w / TEXTURE_SIZE;
    int sprite_count_height = loaded_surface->h / TEXTURE_SIZE;

    spritesheet* sheet = malloc(sizeof(spritesheet));
    sheet->sprite_count = sprite_count_width * sprite_count_height;
    sheet->sprites = malloc(sizeof(uint32_t*) * sheet->sprite_count);

    for(int x = 0; x < sprite_count_width; x++){

        for(int y = 0; y < sprite_count_height; y++){

            int sprite_index = x + (y * sprite_count_width);
            sheet->sprites[sprite_index] = malloc(sizeof(uint32_t) * TEXTURE_SIZE * TEXTURE_SIZE);

            int source_base_x = x * TEXTURE_SIZE;
            int source_base_y = y * TEXTURE_SIZE;

            for(int tx = 0; tx < TEXTURE_SIZE; tx++){
                for(int ty = 0; ty < TEXTURE_SIZE; ty++){

                    int source_index = source_base_x + tx + ((source_base_y + ty) * loaded_surface->w);
                    int dest_index = tx + (ty * TEXTURE_SIZE);

                    uint8_t r, g, b, a;
                    SDL_GetRGBA(loaded_surface_pixels[source_index], loaded_surface->format, &r, &g, &b, &a);
                    sheet->sprites[sprite_index][dest_index] = a == 0 ? 0 : SDL_MapRGBA(screen_surface->format, r, g, b, a);
                }
            }
        }
    }

    SDL_FreeSurface(loaded_surface);

    return sheet;
}

void engine_spritesheet_free(spritesheet* sheet){

    free(sheet->sprites);
    free(sheet);
}

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

    engine_set_resolution(1280, 720);
    screen_surface = SDL_GetWindowSurface(window);
    screen_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    COLOR_TRANSPARENT = 0;

    player_hand_anim = engine_anim_texture_load("./res/hand.png", 128, 128, 4);

    texture_sprites = engine_spritesheet_load("./res/textures.png");
    object_sprites = engine_spritesheet_load("./res/sprites.png");
    projectile_sprites = engine_spritesheet_load("./res/projectiles.png");

    enemy_move_sprites = malloc(sizeof(spritesheet*) * NUM_ENEMIES);
    enemy_attack_sprites = malloc(sizeof(spritesheet*) * NUM_ENEMIES);
    for(int i = 0; i < NUM_ENEMIES; i++){

        char move_path[128] = "./res/";
        strcat(move_path, enemy_info[i].name);
        strcat(move_path, "_move.png");
        enemy_move_sprites[i] = engine_spritesheet_load(move_path);

        char attack_path[128] = "./res/";
        strcat(attack_path, enemy_info[i].name);
        strcat(attack_path, "_attack.png");
        enemy_attack_sprites[i] = engine_spritesheet_load(attack_path);
    }

    return true;
}

void engine_quit(){

    engine_anim_texture_free(player_hand_anim);

    engine_spritesheet_free(texture_sprites);
    engine_spritesheet_free(object_sprites);
    engine_spritesheet_free(projectile_sprites);

    for(int i = 0; i < NUM_ENEMIES; i++){

        engine_spritesheet_free(enemy_move_sprites[i]);
        engine_spritesheet_free(enemy_attack_sprites[i]);
    }
    free(enemy_move_sprites);
    free(enemy_attack_sprites);

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

void engine_render_anim_texture(anim_texture* texture, int frame, int x, int y){

    SDL_Rect dest_rect = (SDL_Rect){ .x = x, .y = y, .w = texture->frame_rects[frame].w, .h = texture->frame_rects[frame].h };
    SDL_RenderCopy(renderer, texture->texture, &(texture->frame_rects[frame]), &dest_rect);
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
            if(cell.x >= 0 && cell.x <= state->map->width - 1 && cell.y >= 0 && cell.y <= state->map->height - 1){

                int floor_index = cell.x + (cell.y * state->map->width);
                int source_index = texture_x + (texture_y * TEXTURE_SIZE);
                int dest_index = x + (y * SCREEN_WIDTH);
                screen_buffer[dest_index] = (texture_sprites->sprites[state->map->floor[floor_index] - 1][source_index] >> 1) & 8355711;
                dest_index = x + ((SCREEN_HEIGHT - y - 1) * SCREEN_WIDTH);
                screen_buffer[dest_index] = (texture_sprites->sprites[state->map->ceil[floor_index] - 1][source_index] >> 1) & 8355711;
            }
        }
    }

    // Wall casting
    for(int x = 0; x < SCREEN_WIDTH; x++){

        float camera_x = ((2 * x) / (float)SCREEN_WIDTH) - 1;
        vector ray = vector_sum(state->player_direction, vector_mult(state->player_camera, camera_x));
        float wall_dist;
        int texture_x;
        bool x_sided;
        int texture;
        render_raycast(state, state->player_position, ray, &wall_dist, &texture_x, &x_sided, &texture);
        z_buffer[x] = wall_dist;

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
            screen_buffer[dest_index] = x_sided ? texture_sprites->sprites[texture - 1][source_index] : (texture_sprites->sprites[texture - 1][source_index] >> 1) & 8355711;
        }
    }

    // Sprite casting

    // First collect sprite info from all the different kinds of sprite arrays
    // This is done because it's easier from a game-logic perspective to store the sprites in separate arrays rather than carrying a tag on each sprite
    int sprite_count = state->object_count + state->projectile_count + state->enemy_count;
    vector** sprite_positions = malloc(sizeof(vector*) * sprite_count);
    uint32_t** sprite_images = malloc(sizeof(uint32_t*) * sprite_count);
    float** sprite_distances = (float**)malloc(sizeof(float*) * sprite_count);
    for(int i = 0; i < state->object_count; i++){

        sprite_positions[i] = &(state->objects[i].position);
        sprite_images[i] = object_sprites->sprites[state->objects[i].image];
    }
    int base_index = state->object_count;
    for(int i = 0; i < state->projectile_count; i++){

        sprite_positions[i + base_index] = &(state->projectiles[i].position);
        sprite_images[i + base_index] = projectile_sprites->sprites[state->projectiles[i].image];
    }
    base_index += state->projectile_count;
    for(int i = 0; i < state->enemy_count; i++){

        sprite_positions[i + base_index] = &(state->enemies[i].position);
        if(state->enemies[i].state == ENEMY_STATE_ATTACKING){

            sprite_images[i + base_index] = enemy_attack_sprites[state->enemies[i].name]->sprites[state->enemies[i].current_frame];

        }else{

            sprite_images[i + base_index] = enemy_move_sprites[state->enemies[i].name]->sprites[state->enemies[i].current_frame];
        }
    }
    for(int i = 0; i < sprite_count; i++){

        sprite_distances[i] = malloc(sizeof(float) * 2);
        sprite_distances[i][0] = i;
        sprite_distances[i][1] = vector_distance(state->player_position, *(sprite_positions[i]));
    }

    // Now sort all the collected sprites by distance
    quicksort(sprite_distances, 0, sprite_count - 1);

    vector minus_player_pos = vector_mult(state->player_position, -1);
    // Lastly render the sprites in order from farthest to nearest
    for(int i = sprite_count - 1; i >= 0; i--){

        vector sprite_render_pos = vector_sum(*(sprite_positions[(int)sprite_distances[i][0]]), minus_player_pos);
        float inverse_determinate = 1.0 / ((state->player_camera.x * state->player_direction.y) - (state->player_direction.x * state->player_camera.y));
        vector transform = (vector){ .x = (state->player_direction.y * sprite_render_pos.x) - (state->player_direction.x * sprite_render_pos.y), .y = (-state->player_camera.y * sprite_render_pos.x) + (state->player_camera.x * sprite_render_pos.y) };
        transform = vector_mult(transform, inverse_determinate);

        int sprite_screen_x = (int)((SCREEN_WIDTH / 2) * (1 + (transform.x / transform.y)));
        int sprite_height = abs((int)(SCREEN_HEIGHT / transform.y));
        int sprite_start_y = (SCREEN_HEIGHT / 2) - (sprite_height / 2);
        int sprite_end_y = (SCREEN_HEIGHT / 2) + (sprite_height / 2);
        if(sprite_start_y < 0){

            sprite_start_y = 0;
        }
        if(sprite_end_y >= SCREEN_HEIGHT){

            sprite_end_y = SCREEN_HEIGHT - 1;
        }

        int sprite_width = abs((int)(SCREEN_HEIGHT / transform.y));
        int sprite_start_x = sprite_screen_x - (sprite_width / 2);
        int sprite_end_x = sprite_screen_x + (sprite_width / 2);
        if(sprite_start_x < 0){

            sprite_start_x = 0;
        }
        if(sprite_end_x >= SCREEN_WIDTH){

            sprite_end_x = SCREEN_WIDTH - 1;
        }

        uint32_t* sprite_image = sprite_images[(int)sprite_distances[i][0]];
        // SDL_Rect region = object_sprite_regions[sprite_index];
        for(int stripe = sprite_start_x; stripe < sprite_end_x; stripe++){

            int texture_x = (int)((stripe - (sprite_screen_x - (sprite_width / 2))) * TEXTURE_SIZE / sprite_width);
            if(transform.y > 0 && stripe > 0 && stripe < SCREEN_WIDTH && transform.y < z_buffer[stripe]){

                for(int y = sprite_start_y; y < sprite_end_y; y++){

                    int d = y - (SCREEN_HEIGHT / 2) + (sprite_height / 2);
                    int texture_y = (int)((d * TEXTURE_SIZE) / sprite_height);
                    int source_index = texture_x + (texture_y * TEXTURE_SIZE);
                    int dest_index = stripe + (y * SCREEN_WIDTH);
                    uint32_t color = sprite_image[source_index];
                    if(color != COLOR_TRANSPARENT){

                        screen_buffer[dest_index] = color;
                    }
                } // End for each sprite y
            }
        } // End for each stripe
    } // End for each sprite

    // Clean memory from sprite casting
    free(sprite_positions);
    free(sprite_images);
    for(int i = 0; i < sprite_count; i++){

        free(sprite_distances[i]);
    }
    free(sprite_distances);

    engine_render_buffer();

    // Render UI
    engine_render_anim_texture(player_hand_anim, 0, SCREEN_WIDTH - 160 + get_player_animation_offset_x(state), SCREEN_HEIGHT - 128 + get_player_animation_offset_y(state));

    engine_render_fps();
    SDL_RenderPresent(renderer);
}
