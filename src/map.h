#pragma once

#include "vector.h"
#include <stdbool.h>

typedef struct map{

    int* wall;
    int* ceil;
    int* floor;
    int* objects;
    int* entities;
    bool* collidemap;
    int width;
    int height;
} map;

map* map_load_from_tmx(const char* path);
void map_generate_collidemap(map* the_map);
bool map_square_occupied(map* the_map, vector square);
bool map_pathfind(map* the_map, vector start, vector goal, vector* solution);
