#pragma once

typedef struct map{

    int* wall;
    int* ceil;
    int* floor;
    int* objects;
    int* entities;
    int width;
    int height;
} map;

map* map_init(int width, int height);

map* map_load_from_tmx(const char* path);
