#include "map.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

map* map_init(int width, int height){

    map* new_map = (map*)malloc(sizeof(map));
    new_map->width = width;
    new_map->height = height;

    int tile_count = width * height;
    new_map->wall = (int*)malloc(sizeof(int) * tile_count);
    new_map->ceil = (int*)malloc(sizeof(int) * tile_count);
    new_map->floor = (int*)malloc(sizeof(int) * tile_count);

    for(int i = 0; i < tile_count; i++){

        int x = i % width;
        int y = (int)(i / width);

        new_map->wall[i] = (x == 0 || y == 0 || x == width - 1 || y == height - 1) ? 1 : 0;
        new_map->ceil[i] = 1;
        new_map->floor[i] = 1;
    }

    return new_map;
}

void map_resize(map* old_map, int new_width, int new_height){

    int old_width = old_map->width;
    int old_height = old_map->height;
    int* old_wall = (int*)malloc(sizeof(int) * old_width * old_height);
    int* old_ceil = (int*)malloc(sizeof(int) * old_width * old_height);
    int* old_floor = (int*)malloc(sizeof(int) * old_width * old_height);

    old_map->width = new_width;
    old_map->height = new_height;
    old_map->wall = (int*)malloc(sizeof(int) * new_width * new_height);
    old_map->ceil = (int*)malloc(sizeof(int) * new_width * new_height);
    old_map->floor = (int*)malloc(sizeof(int) * new_width * new_height);

    for(int i = 0; i < new_width * new_height; i++){

        int x = i % new_width;
        int y = (int)(i / new_width);

        if(x < old_width && y < old_height){

            int old_index = x + (y * old_width);
            old_map->wall[i] = old_wall[old_index];
            old_map->ceil[i] = old_ceil[old_index];
            old_map->floor[i] = old_floor[old_index];

        }else{

            old_map->wall[i] = (x == 0 || y == 0 || x == new_width - 1 || y == new_height - 1) ? 1 : 0;
            old_map->ceil[i] = 1;
            old_map->floor[i] = 1;
        }
    }

    free(old_wall);
    free(old_ceil);
    free(old_floor);
}

void trim_leading_whitespace(char* str){

    int start_index = 0;
    while(str[start_index] == ' ' || str[start_index] == '\t'){

        start_index++;
    }
    if(start_index != 0){

        int i = 0;
        while(str[i + start_index] != '\0'){

            str[i] = str[i + start_index];
            i++;
        }
        str[i] = '\0';
    }
}

bool starts_with(const char* str, const char* prefix){

    size_t str_length = strlen(str);
    size_t prefix_length = strlen(prefix);
    return str_length < prefix_length ? false : memcmp(prefix, str, prefix_length) == 0;
}

void read_property(const char* line_buffer, const char* property, char* value){

    // Find the XML definition of the property
    char* property_start = strstr(line_buffer, property);

    // Navigate to the first of the double quotes that contain the property value
    int scan_index = 0;
    while(property_start[scan_index] != '\"'){

        scan_index++;
    }
    scan_index++;

    // Read the value into 'value' until anther double quote is found
    int value_length = 0;
    while(property_start[scan_index] != '\"'){

        value[value_length] = property_start[scan_index];
        value_length++;
        scan_index++;
    }

    // Lastly, terminate the string
    value[value_length] = '\0';
}

void read_next_value(const char* line_buffer, int* scan_pos, char* value){

    int length = 0;
    while(line_buffer[*scan_pos] != ',' && line_buffer[*scan_pos] != '\n'){

        value[length] = line_buffer[*scan_pos];
        length++;
        (*scan_pos)++;
    }

    (*scan_pos)++;
    value[length] = '\0';
}

map* map_load_from_tmx(const char* path){

    // Open the file
    FILE* file = fopen(path, "r");
    if(file == NULL){

        printf("Error opening mapfile!\n");
        return NULL;
    }

    map* new_map = malloc(sizeof(map));

    char line_buffer[1024];
    char value_buffer[32];

    bool reading_tiles = false;
    bool fill_blanks = false;
    int* tile_array;
    int current_offset = 0;
    int y = 0;

    int tileset_offset = 0;
    int objects_offset = 0;
    while(fgets(line_buffer, sizeof(line_buffer), file)){

        trim_leading_whitespace(line_buffer);
        if(!reading_tiles){

            if(starts_with(line_buffer, "<map")){

                read_property(line_buffer, "width", value_buffer);
                new_map->width = atoi(value_buffer);
                read_property(line_buffer, "height", value_buffer);
                new_map->height = atoi(value_buffer);

                int map_size = new_map->width * new_map->height;

                new_map->wall = malloc(sizeof(int) * map_size);
                new_map->floor = malloc(sizeof(int) * map_size);
                new_map->ceil = malloc(sizeof(int) * map_size);
                new_map->objects = malloc(sizeof(int) * map_size);

            }else if(starts_with(line_buffer, "<tileset")){

                read_property(line_buffer, "firstgid", value_buffer);
                int firstgid = atoi(value_buffer);
                read_property(line_buffer, "source", value_buffer);
                if(strcmp(value_buffer, "wolftiles.tsx") == 0){

                    tileset_offset = firstgid - 1;

                }else if(strcmp(value_buffer, "objects.tsx") == 0){

                    objects_offset = firstgid - 1;
                }

            }else if(starts_with(line_buffer, "<layer")){

                read_property(line_buffer, "name", value_buffer);
                if(strcmp(value_buffer, "wall") == 0){

                    tile_array = new_map->wall;
                    current_offset = tileset_offset;

                }else if(strcmp(value_buffer, "floor") == 0){

                    tile_array = new_map->floor;
                    fill_blanks = true;
                    current_offset = tileset_offset;

                }else if(strcmp(value_buffer, "ceil") == 0){

                    tile_array = new_map->ceil;
                    fill_blanks = true;
                    current_offset = tileset_offset;

                }else if(strcmp(value_buffer, "objects") == 0){

                    tile_array = new_map->objects;
                    current_offset = objects_offset;
                }
                reading_tiles = true;
                y = 0;
            }

        }else{

            if(starts_with(line_buffer, "</data>")){

                reading_tiles = false;
                fill_blanks = false;
                current_offset = 0;

            }else if(!starts_with(line_buffer, "<data")){

                int scan_index = 0;
                for(int x = 0; x < new_map->width; x++){

                    read_next_value(line_buffer, &scan_index, value_buffer);
                    tile_array[x + (y * new_map->width)] = atoi(value_buffer);
                    if(tile_array[x + (y * new_map->width)] != 0){

                        tile_array[x + (y * new_map->width)] -= current_offset;
                    }
                    if(fill_blanks && tile_array[x + (y * new_map->width)] == 0){

                        tile_array[x + (y * new_map->width)] = 1;
                    }
                }
                y++;
            }
        }

    } // End while

    fclose(file);

    return new_map;
}
