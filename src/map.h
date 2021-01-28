#pragma once

typedef struct map{

    int* wall;
    int* ceil;
    int* floor;
    int width;
    int height;
} map;

map* map_init(int width, int height);
void map_resize(map* old_map, int new_width, int new_height);

map* map_load_from_tmx(const char* path);
