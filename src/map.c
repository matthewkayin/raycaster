#include "map.h"

#include "vector_array.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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
    int entities_offset = 0;
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
                new_map->entities = malloc(sizeof(int) * map_size);
                new_map->collidemap = malloc(sizeof(bool) * map_size);

            }else if(starts_with(line_buffer, "<tileset")){

                read_property(line_buffer, "firstgid", value_buffer);
                int firstgid = atoi(value_buffer);
                read_property(line_buffer, "source", value_buffer);
                if(strcmp(value_buffer, "tileset.tsx") == 0){

                    tileset_offset = firstgid - 1;

                }else if(strcmp(value_buffer, "objects.tsx") == 0){

                    objects_offset = firstgid - 1;

                }else if(strcmp(value_buffer, "entities.tsx") == 0){

                    entities_offset = firstgid - 1;
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

                }else if(strcmp(value_buffer, "entities") == 0){

                    tile_array = new_map->entities;
                    current_offset = entities_offset;
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

    map_generate_collidemap(new_map);

    return new_map;
}

void map_generate_collidemap(map* the_map){

    int map_size = the_map->width * the_map->height;

    for(int i = 0; i < map_size; i++){

        the_map->collidemap[i] = the_map->wall[i] != 0 || the_map->objects[i] != 0;
    }
}

bool map_square_occupied(map* the_map, vector square){

    // First check if in bounds
    if(square.x < 0 || square.x >= the_map->width || square.y < 0 || square.y >= the_map->height){

        return true;
    }

    // Then cast to int and check the static collidemap
    // This function doesn't check living entities, it's really used more for pathfinding around walls and objects
    return the_map->collidemap[(int)square.x + ((int)square.y * the_map->width)];
}

bool map_pathfind(map* the_map, vector start, vector goal, vector* solution){

    typedef struct{
        vector position;
        int direction;
        int path_length;
        int score;
    } node;

    static const vector direction_vectors[8] = {
        (vector){ .x = 0, .y = -1 }, // up
        (vector){ .x = 1, .y = -1 }, // up right
        (vector){ .x = 1, .y = 0 }, // right
        (vector){ .x = 1, .y = 1 }, // down right
        (vector){ .x = 0, .y = 1 }, // down
        (vector){ .x = -1, .y = 1 }, // down left
        (vector){ .x = -1, .y = 0 }, // left
        (vector){ .x = -1, .y = -1 } // up left
    };

    vector goal_square = (vector){ .x = (int)goal.x, .y = (int)goal.y };
    vector start_square = (vector){ .x = (int)start.x, .y = (int)start.y };

    int frontier_size = 0;
    int frontier_capacity = 16;
    node* frontier = malloc(sizeof(node) * frontier_capacity);
    int explored_size = 0;
    int explored_capacity = 16;
    node* explored = malloc(sizeof(node) * explored_capacity);

    node start_node = (node){
        .position = start_square,
        .direction = -1,
        .path_length = 0,
        .score = abs((int)(goal_square.x - start_square.x)) + abs((int)(goal_square.y - start_square.y))
    };
    vector_array_push((void**)&frontier, &start_node, &frontier_size, &frontier_capacity, sizeof(node));

    while(true){

        if(frontier_size == 0){

            printf("Pathfinding failed!\n");
            false;
        }

        // Find the smallest cost node
        int smallest_index = 0;
        for(int i = 1; i < frontier_size; i++){

            if(frontier[i].score < frontier[smallest_index].score){

                smallest_index = i;
            }
        }

        // Remove it from the frontier
        node smallest = frontier[smallest_index];
        vector_array_delete(frontier, smallest_index, &frontier_size, sizeof(node));

        // Check if it's the solution
        if(smallest.position.x == goal_square.x && smallest.position.y == goal_square.y){

            (*solution) = vector_sum(start_square, direction_vectors[smallest.direction]);
            return true;
        }

        // Add it to explored
        vector_array_push((void**)&explored, &smallest, &explored_size, &explored_capacity, sizeof(node));

        // Expand out all possible paths based on the one we've chosen
        for(int direction = 0; direction < 8; direction++){

            vector child_pos = vector_sum(smallest.position, direction_vectors[direction]);

            // If the path leads to an invalid square, ignore it
            if(map_square_occupied(the_map, child_pos)){

                continue;
            }

            // If moving diagonally, make sure this movement isn't taking us through a wall corner
            bool direction_is_diagonal = direction % 2 == 1;
            if(direction_is_diagonal){

                vector child_pos_x_component = vector_sum(smallest.position, (vector){ .x = direction_vectors[direction].x, .y = 0 });
                vector child_pos_y_component = vector_sum(smallest.position, (vector){ .x = 0, .y = direction_vectors[direction].y });
                if(map_square_occupied(the_map, child_pos_x_component) || map_square_occupied(the_map, child_pos_y_component)){

                    continue;
                }
            }

            // Create the child node
            int first_direction = smallest.direction;
            if(first_direction == -1){

                first_direction = direction;
            }
            int path_length = smallest.path_length + 1;
            int score = path_length + abs((int)(goal_square.x - child_pos.x)) + abs((int)(goal_square.y - child_pos.y));
            node child = (node){
                .position = child_pos,
                .direction = first_direction,
                .path_length = path_length,
                .score = score
            };

            // Ignore this child if in explored
            bool child_in_explored = false;
            for(int i = 0; i < explored_size; i++){

                if(child.position.x == explored[i].position.x && child.position.y == explored[i].position.y){

                    child_in_explored = true;
                    break;
                }
            }
            if(child_in_explored){

                continue;
            }

            // Ignore this child if in frontier
            bool child_in_frontier = false;
            int frontier_index = -1;
            for(int i = 0; i < frontier_size; i++){

                if(child.position.x == frontier[i].position.x && child.position.y == frontier[i].position.y){

                    child_in_frontier = true;
                    frontier_index = i;
                    break;
                }
            }

            if(child_in_frontier){

                // If child is in frontier but with a smaller cost, replace the frontier version with the child
                if(child.score < frontier[frontier_index].score){

                    frontier[frontier_index] = child;
                }
                continue;
            }

            // Finally, if child is neither in frontier nor explored, add it to the frontier
            vector_array_push((void**)&frontier, &child, &frontier_size, &frontier_capacity, sizeof(node));
        } // End for each direction

    } // End while true

    free(frontier);
    free(explored);
}
